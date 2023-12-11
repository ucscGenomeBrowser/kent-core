/* hgTrackDb - Create trackDb table from text files. */


/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "sqlList.h"
#include "jksql.h"
#include "trackDb.h"
#include "hui.h"
#include "hdb.h"
#include "hVarSubst.h"
#include "obscure.h"
#include "portable.h"
#include "dystring.h"
#include "regexHelper.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgTrackDb - Create trackDb table from text files.\n\n"
  "Note that the browser supports multiple trackDb tables, usually\n"
  "in the form: trackDb_YourUserName. Which particular trackDb\n"
  "table the browser uses is specified in the hg.conf file found\n"
  "either in your home directory file '.hg.conf' or in the web server's\n"
  "cgi-bin/hg.conf configuration file with the setting: db.trackDb=trackDb\n"
  "see also: src/product/ex.hg.conf discussion of this setting.\n"
  "usage:\n"
  "   hgTrackDb [options] org database trackDb trackDb.sql hgRoot\n"
  "\n"
  "Options:\n"
  "  org - a directory name with a hierarchy of trackDb.ra files to examine\n"
  "      - in the case of a single directory with a single trackDb.ra file use .\n"
  "  database - name of database to create the trackDb table in\n"
  "  trackDb  - name of table to create, usually trackDb, or trackDb_${USER}\n"
  "  trackDb.sql  - SQL definition of the table to create, typically from\n"
  "               - the source tree file: src/hg/lib/trackDb.sql\n"
  "               - the table name in the CREATE statement is replaced by the\n"
  "               - table name specified on this command line.\n"
  "  hgRoot - a directory name to prepend to org to locate the hierarchy:\n"
  "           hgRoot/trackDb.ra - top level trackDb.ra file processed first\n"
  "           hgRoot/org/trackDb.ra - second level file processed second\n"
  "           hgRoot/org/database/trackDb.ra - third level file processed last\n"
  "         - for no directory hierarchy use .\n"
  "  -strict - only include tables that exist (and complain about missing html files).\n"
  "  -raName=trackDb.ra - Specify a file name to use other than trackDb.ra\n"
  "   for the ra files.\n"
  "  -release=alpha|beta|public - Include trackDb entries with this release tag only.\n"
  "  -settings - for trackDb scanning, output table name, type line,\n"
  "            -  and settings hash to stderr while loading everything.\n"
  "  -gbdbList=list - list of files to confirm existance of bigDataUrl files\n"
  "  -addVersion - add cartVersion pseudo-table\n"
  "  -noHtmlCheck - don't check for HTML even if strict is set\n"
  );
}

static struct optionSpec optionSpecs[] = {
    {"raName", OPTION_STRING},
    {"strict", OPTION_BOOLEAN},
    {"release", OPTION_STRING},
    {"settings", OPTION_BOOLEAN},
    {"gbdbList", OPTION_STRING},
    {"addVersion", OPTION_BOOLEAN},
    {"noHtmlCheck", OPTION_BOOLEAN},
    {NULL,      0}
};

static char *raName = "trackDb.ra";

static char *release = "alpha";

static char *gbdbList = NULL;
static struct hash *gbdbHash = NULL;
static boolean addVersion = FALSE;
static boolean noHtmlCheck = FALSE;

// release tags
#define RELEASE_ALPHA  (1 << 0)
#define RELEASE_BETA   (1 << 1)
#define RELEASE_PUBLIC (1 << 2)

unsigned releaseBit = RELEASE_ALPHA;

static bool showSettings = FALSE;

static boolean hasNonAsciiChars(char *text)
/* Check if text has any non-printing or non-ascii characters */
{
char *c;
for (c = text; *c != '\0'; c++)
    {
    if ((*c <= 7) || ((14 <= *c) && (*c <= 31)) || (*c >= 127))
        {
        return TRUE;
        }
    }
return FALSE;
}


static struct trackDb *trackDbListFromHash(struct hash *tdHash)
/* Build a list of the tracks in hash.  List will be threaded through
 * the entries. */
{
struct trackDb *tdbList = NULL;
struct hashCookie cookie = hashFirst(tdHash);
struct hashEl *hel;
while ((hel = hashNext(&cookie)) != NULL)
    {
    struct trackDb *tdb = hel->val;
    slSafeAddHead(&tdbList, tdb);
    }
return tdbList;
}

static struct trackDb *pruneStrict(struct trackDb *tdbList, char *db)
/* Remove tracks without data.  For parent tracks data in any child is sufficient to keep
 * them alive. */
{
struct trackDb *newList = NULL, *tdb, *next;
for (tdb = tdbList; tdb != NULL; tdb = next)
    {
    next = tdb->next;
    verbose(3,"pruneStrict checking track: '%s'\n", tdb->track);
    if (tdb->subtracks != NULL)
	{
	tdb->subtracks = pruneStrict(tdb->subtracks, db);
	}
    if (tdb->subtracks != NULL)
        {
	slAddHead(&newList, tdb);
	}
    else if (tdbIsDownloadsOnly(tdb) || trackDataAccessibleHash(db, tdb, gbdbHash)) 
        {
        slAddHead(&newList, tdb);
        }
    // We will allow these at some point, but currently(10/10/2011) the table browser still chokes.
    //else if (trackDbSetting(tdb, "bigDataUrl") != NULL)
    //    {
    //    slAddHead(&newList, tdb);
    //    }
    else
	verbose(3,"pruneStrict removing track: '%s' no table %s\n", tdb->track, tdb->table);
    }
slReverse(&newList);
return newList;
}


static struct trackDb *pruneEmptyContainers(struct trackDb *tdbList)
/* Remove container tracks (views and compositeTracks) with no children. */
{
struct trackDb *newList = NULL, *tdb, *next;
for (tdb = tdbList; tdb != NULL; tdb = next)
    {
    next = tdb->next;
    if (trackDbLocalSetting(tdb, "compositeTrack") || trackDbLocalSetting(tdb, "view"))
	{
	tdb->subtracks = pruneEmptyContainers(tdb->subtracks);
	if (tdb->subtracks != NULL)
	    slAddHead(&newList, tdb);
	else
	    verbose(3,"pruneEmptyContainers: empty track: '%s'\n",
		tdb->track);
	}
    else
        {
	slAddHead(&newList, tdb);
	}
    }
slReverse(&newList);
return newList;
}

unsigned buildReleaseBits(char *rel, char *track)
/* unpack the comma separated list of possible release tags */
{
if (rel == NULL)
    return RELEASE_ALPHA |  RELEASE_BETA |  RELEASE_PUBLIC;

char *oldString = cloneString(rel);
unsigned bits = 0;
while(rel)
    {
    char *end = strchr(rel, ',');

    if (end)
	*end++ = 0;
    rel = trimSpaces(rel);

    if (sameString(rel, "alpha"))
	bits |= RELEASE_ALPHA;
    else if (sameString(rel, "beta"))
	bits |= RELEASE_BETA;
    else if (sameString(rel, "public"))
	bits |= RELEASE_PUBLIC;
    else
	errAbort("track %s with release %s must have a release combination of alpha, beta, and public", 
                        track, oldString);

    rel = end;
    }

return bits;
}

static struct trackDb * pruneRelease(struct trackDb *tdbList)
/* Prune out alternate track entries for another release */
{
/* Build up list that only includes things in this release.  Release
 * can be inherited from parents. */
struct trackDb *tdb;
struct trackDb *relList = NULL;
struct hash *haveHash = hashNew(3);

while ((tdb = slPopHead(&tdbList)) != NULL)
    {
    char *rel = trackDbSetting(tdb, "release");
    unsigned trackRelBits = buildReleaseBits(rel, tdb->track);

    if (trackRelBits & releaseBit)
	{
	/* we want to include this track, check to see if we already have it */
	struct hashEl *hel;
        verbose(3,"pruneRelease: adding '%s', release: '%s'\n", tdb->track, rel);
	if ((hel = hashLookup(haveHash, tdb->track)) != NULL)
	    errAbort("found two copies of table %s: one with release %s, the other %s\n",
		tdb->track, (char *)hel->val, release);
	hashAdd(haveHash, tdb->track, rel);
	hashRemove(tdb->settingsHash, "release");
	slAddHead(&relList, tdb);
	}
    else
	verbose(3,"pruneRelease: removing '%s', release: '%s' != '%s'\n",
	    tdb->track, rel, release);
    }

freeHash(&haveHash);
return relList;
}

static struct trackDb * pruneOrphans(struct trackDb *tdbList, struct hash *trackHash)
/* Prune out orphans with no parents of the right release */
{
boolean done = FALSE;
struct trackDb *tdb;
struct trackDb *nonOrphanList = NULL;
while (!done)
    {
    done = TRUE;
    nonOrphanList = NULL;
    while ((tdb = slPopHead(&tdbList)) != NULL)
	{
	char *parentName = cloneFirstWord(trackDbLocalSetting(tdb, "parent"));
	struct hashEl *hel = NULL;
	if (parentName != NULL)
	    {
	    hel = hashLookup(trackHash, parentName);
	    }
	if (parentName == NULL || hel != NULL)
	    {
	    slAddHead(&nonOrphanList, tdb);
	    }
	else
	    {
	    verbose(3,"pruneOrphans: removing '%s'\n",
		tdb->track);
	    hashRemove(trackHash, tdb->track);
	    done = FALSE;
	    }
	freeMem(parentName);
	}
    tdbList = nonOrphanList;
    }
return tdbList;
}

static void applyOverride(struct hash *trackHash,
                          struct trackDb *overTd)
/* Apply a trackDb override to a track, if it exists */
{
struct trackDb *tdb = hashFindVal(trackHash, overTd->track);
if (tdb != NULL)
    trackDbOverride(tdb, overTd);
}

static void addVersionRa(boolean strict, char *database, char *dirName, char *raName,
                         struct hash *trackHash)
/* Read in tracks from raName and add them to table, pruning as required. Call
 * top-down so that track override will work. */
{
struct trackDb *tdbList = trackDbFromRa(raName, NULL, NULL), *tdb;
/* prune records of the incorrect release */
tdbList= pruneRelease(tdbList);

/* load tracks, replacing higher-level ones with lower-level and
 * applying overrides*/
while ((tdb = slPopHead(&tdbList)) != NULL)
    {
    if (tdb->overrides != NULL)
	{
        verbose(3,"# override '%s'\n", tdb->track);
	applyOverride(trackHash, tdb);
	}
    else
	{
        verbose(3,"# track '%s'\n", tdb->track);
	hashStore(trackHash, tdb->track)->val = tdb;
	}
    }
}

void updateBigTextField(struct sqlConnection *conn, char *table,
     char *whereField, char *whereVal,
     char *textField, char *textVal)
/* Generate sql code to update a big text field that may include
 * newlines and stuff. */
{
struct dyString *dy = dyStringNew(4096);
sqlDyStringPrintf(dy, "update %s set %s='%s'", table, textField, textVal);
sqlDyStringPrintf(dy, " where %s = '%s'", whereField, whereVal);
sqlUpdate(conn, dy->string);
dyStringFree(&dy);
}

char *substituteTrackName(char *create, char *tableName)
/* Substitute tableName for whatever is between CREATE TABLE  and  first '('
   freez()'s create passed in. */
{
char newCreate[strlen(create) + strlen(tableName) + 10];
char *front = NULL;
char *rear = NULL;
int length = (strlen(create) + strlen(tableName) + 10);
/* try to find "TABLE" or "table" in create */
front = strstr(create, "TABLE");
if(front == NULL)
    front = strstr(create, "table");
if(front == NULL)
    errAbort("hgTrackDb::substituteTrackName() - Can't find 'TABLE' in %s", create);

/* try to find first "(" in string */
rear = strstr(create, "(");
if(rear == NULL)
    errAbort("hgTrackDb::substituteTrackName() - Can't find '(' in %s", create);

/* set to NULL character after "TABLE" */
front += 5;
*front = '\0';

safef(newCreate, length , "%s %s %s", create, tableName, rear);
return cloneString(newCreate);
}

void layerOnRa(boolean strict, char *database, char *dir, struct hash *trackHash,
               boolean raMustExist)
/* Read trackDb.ra from directory and layer them on top of whatever is in trackHash.
 * Must call top-down (root->org->assembly) */
{
char raFile[512];
if (raName[0] != '/')
    safef(raFile, sizeof(raFile), "%s/%s", dir, raName);
else
    safef(raFile, sizeof(raFile), "%s", raName);
if (fileExists(raFile))
    {
    verbose(3,"# reading '%s'\n", raFile);
    addVersionRa(strict, database, dir, raFile, trackHash);
    }
else
    {
    if (raMustExist)
        errAbort("%s doesn't exist!", raFile);
    }
}

// This regular expression matches an HTML comment that contains a
// server-side-include-like syntax that tells us to replace the HTML
// comment with the contents of some other HTML file (relative path)
// [which itself may include such HTML comments].
// This regex has substrs:
// * substrs[0] is the entire match
// * substrs[1] is an optional '#if (db==xxxYyy1)' before #insert
// * substrs[2] is the database from the #if, or empty if substrs[1] is empty
// * substrs[3] is the relative path to pull in.
const static char *insertHtmlRegex =
    "<!--"
    "([[:space:]]*#if[[:space:]]*\\(db[[:space:]]*==[[:space:]]*([[:alnum:]]+)[[:space:]]*\\))?"
    "[[:space:]]*#insert[[:space:]]+file[[:space:]]*=[[:space:]]*"
    "\"([^/][^\"]+)\"[[:space:]]*-->";

static char *readHtmlRecursive(char *fileName, char *database, struct trackDb *tdb)
/* Slurp in an html file.  Wherever it contains insertHtmlRegex, recursively slurp that in
 * and replace insertHtmlRegex with the contents. */
{
char *html;
readInGulp(fileName, &html, NULL);
if (isEmpty(html))
    return html;
regmatch_t substrs[4];
while (regexMatchSubstr(html, insertHtmlRegex, substrs, ArraySize(substrs)))
    {
    struct dyString *dy = dyStringNew(0);
    // All text before the regex match:
    dyStringAppendN(dy, html, substrs[0].rm_so);
    // Is there an #if before the #insert ?
    boolean doInsert = TRUE;
    if (substrs[1].rm_so != -1 &&
	(! sameStringN(database, html+substrs[2].rm_so, (substrs[2].rm_eo - substrs[2].rm_so))))
	doInsert = FALSE;
    if (doInsert)
	{
	// Recursively pull in inserted file contents from relative path, replacing regex match:
	char dir[PATH_LEN];
	splitPath(fileName, dir, NULL, NULL);
	char insertFileName[PATH_LEN+FILENAME_LEN];
	safecpy(insertFileName, sizeof(insertFileName), dir);
	safencat(insertFileName, sizeof(insertFileName), html+substrs[3].rm_so,
		 (substrs[3].rm_eo - substrs[3].rm_so));
        char *varSubFilename = hVarSubst("readHtmlRecursive: var substitution error",
                                                tdb, database, insertFileName);
        if (varSubFilename)
            safecpy(insertFileName, sizeof(insertFileName), varSubFilename);
        if (differentString(fileName, insertFileName)) // protect against infinite loop
            {
            if (!fileExists(insertFileName))
                errAbort("readHtmlRecursive: relative path '%s' (#insert'ed in %s) not found",
                         insertFileName, fileName);
            char *insertedText = readHtmlRecursive(insertFileName, database, tdb);
            dyStringAppend(dy, insertedText);
            freez(&insertedText);
            }
	}
    // All text after the regex match:
    dyStringAppend(dy, html+substrs[0].rm_eo);
    freez(&html);
    html = dyStringCannibalize(&dy);
    }
return html;
}

static void layerOnHtml(char *dirName, struct trackDb *tdbList, char *database)
/* Read in track HTML call bottom-up. */
{
char fileName[512];
struct trackDb *td;
for (td = tdbList; td != NULL; td = td->next)
    {
    if (isEmpty(td->html))
        {
        char *htmlName = trackDbSetting(td, "html");
        if (htmlName == NULL)
            htmlName = td->track;
	safef(fileName, sizeof(fileName), "%s/%s.html", dirName, htmlName);
	if (!fileExists(fileName))
            safef(fileName, sizeof(fileName), "%s/%s", dirName, htmlName);
	if (fileExists(fileName) && !isDirectory(fileName))
            {
	    td->html = readHtmlRecursive(fileName, database, td);
            // Check for note ASCII characters at higher levels of verboseness.
            // Normally, these are acceptable ISO-8859-1 characters
            if  ((verboseLevel() >= 2) && hasNonAsciiChars(td->html))
                verbose(2, "Note: non-printing or non-ASCII characters in %s\n", fileName);
            }
        }
    }
}

static char *subsituteVariables(struct hashEl *el, char *database)
/* substitute variables where supported */
{
char* val = (char*)el->val;
char *name = el->name;
/* Only some attribute support variable substitution, at least for now
 * Just leak memory when doing substitution.
 */
if (trackSettingIsFile(name))
    {
    val = replaceChars(val, "$D", database);
    }

return val;
}

static char *settingsFromHash(struct hash *hash, char* database)
/* Create settings string from settings hash. */
{
if (hash == NULL)
    return cloneString("");
else
    {
    struct dyString *dy = dyStringNew(1024);
    char *ret;
    struct hashEl *el, *list = hashElListHash(hash);
    slSort(&list, hashElCmp);
    for (el = list; el != NULL; el = el->next)
	dyStringPrintf(dy, "%s %s\n", el->name, subsituteVariables(el, database));
    slFreeList(&list);
    ret = cloneString(dy->string);
    dyStringFree(&dy);
    return ret;
    }
}

struct subGroupData
/* composite group definitions */
{
struct hash *nameHash;   /* hash of subGroup names */
struct trackDb *compositeTdb;  /* tdb of composite parent */
};

static void validateTag(char *database,struct trackDb *tdb,char *type,char *tag,
                        boolean firstIsDigitWarning)
// Determines if tag is conforming to rules:
// 1) must exist
// 2) cannot be all numeric.
// 3) Contains only letters, numbers, '_', '-'
// 4) Begins with a letter
// Aborts on 1-3 and optionally warns on 4
{
char *c = tag;
if (*c == '\0')
    errAbort("Track %s.%s has no '%s' tag",database,tdb->track,type);

if (!isalpha(*c))
    {
    if (countLeadingDigits(tag) == strlen(tag))
        errAbort("Track %s.%s has all numeric '%s' tag '%s'",database,tdb->track,type,tag);
    if (!firstIsDigitWarning)
        return;
    warn("Track %s.%s has non-conforming '%s' tag '%s' (begins with digit)",
         database,tdb->track,type,tag);
    }
for (c++;*c != '\0';c++)
    {
    if (!isalnum(*c) && *c != '_' && *c != '-')
        errAbort("Track %s.%s has non-conforming '%s' tag '%s' (contains unsupported characters)",
                database,tdb->track,type,tag);
    }
}

static struct hash *buildCompositeHash(char *database,struct trackDb *tdbList)
/* Create a hash of all composite tracks.  This is keyed by their name with
 * subGroupData values. */
{
struct hash *compositeHash = newHash(8);

/* process all composite tracks */
struct trackDb *td, *tdNext;
for (td = tdbList; td != NULL; td = tdNext)
    {
    tdNext = td->next;
    if (trackDbLocalSetting(td, "compositeTrack"))
	{
	int i;
	struct subGroupData *sgd;
        char subGroupName[256];
        AllocVar(sgd);
	sgd->compositeTdb = td;

	verbose(3,"getting subgroups for %s\n", td->track);
	for(i=1; ; i++)
	    {
	    safef(subGroupName, sizeof(subGroupName), "subGroup%d", i);
	    char *sgSetting = trackDbSetting(td, subGroupName);
	    if (!sgSetting)
		break;
            sgSetting = cloneString(sgSetting);
            char *sgWord = sgSetting;
            char *sgName = nextWord(&sgWord);
            validateTag(database,td,subGroupName,sgName,TRUE);
            nextWord(&sgWord);  /* skip word not used */
            struct hash *subGroupHash = newHash(3);
            struct slPair *slPair, *slPairList = slPairListFromString(sgWord,TRUE); // respect ""
            for (slPair = slPairList; slPair; slPair = slPair->next)
                {
                validateTag(database,td,sgName,slPair->name,TRUE);
                hashAdd(subGroupHash, slPair->name, slPair->val);
                }
            if (sgd->nameHash == NULL)
		sgd->nameHash = newHash(3);
	    verbose(3,"   adding group %s\n", sgName);
	    hashAdd(sgd->nameHash, sgName, subGroupHash);
	    }
	hashAdd(compositeHash, td->track, sgd);
	}
    }
return compositeHash;
}

static void checkOneSubGroups(char *compositeName,
    struct trackDb *td, struct subGroupData *sgd)
{
char *subGroups = trackDbSetting(td, "subGroups");

verbose(2, "   checkOne %s %s\n", compositeName, td->track);
if (subGroups && (sgd->nameHash == NULL))
    {
    errAbort("subTrack %s has groups not defined in parent %s\n",
	     td->track, compositeName);
    }
else if (!subGroups && (sgd->nameHash != NULL))
    {
    errAbort("subtrack %s is missing subGroups defined in parent %s\n",
	td->track, compositeName);
    }

if (!subGroups && (sgd->nameHash == NULL))
    return; /* nothing to do */

assert(sgd->nameHash != NULL);

struct slPair *slPair, *slPairList = slPairFromString(subGroups);
struct hash *foundHash = newHash(3);
for (slPair = slPairList; slPair; slPair = slPair->next)
    {
    struct hashEl *hel = hashLookup( sgd->nameHash, slPair->name);
    if (hel == NULL)
	errAbort("subtrack %s has subGroup (%s) not found in parent\n",
	    td->track, slPair->name);

    struct hash *subHash = hel->val;
    hashStore(foundHash, slPair->name);

    if (hashLookup(subHash, slPair->val) == NULL)
	{
	errAbort("subtrack %s has subGroup (%s) with value (%s) not found in parent\n",
	    td->track, slPair->name, (char *)slPair->val);
	}
    }

struct hashCookie cookie = hashFirst(sgd->nameHash);
struct hashEl *hel;
while ((hel = hashNext(&cookie)) != NULL)
    {
    if (subgroupRequired(hel->name) && hashLookup(foundHash, hel->name) == NULL)
        errAbort("subtrack %s is missing required subGroup %s\n",
	    td->track, hel->name);
    }

}

static void verifySubTracks(struct trackDb *tdbList, struct hash *compositeHash)
/* Verify subGroup relelated settings in subtracks, assisted by compositeHash */
{
struct trackDb *tdb;

for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    struct slRef *child;
    if (trackDbLocalSetting(tdb, "compositeTrack"))
	{
	struct slRef *childList = trackDbListGetRefsToDescendantLeaves(
	    tdb->subtracks);

	verbose(2,"verifying %s child %p\n",tdb->track, childList);
	for (child = childList; child != NULL; child = child->next)
	    {
	    struct trackDb *childTdb = child->val;
	    verbose(2, "  checking child %s\n",childTdb->track);

	    /* Look for some ancestor in composite hash and set sgd to it. */
	    struct subGroupData *sgd =  hashFindVal(compositeHash, tdb->track);
	    verbose(2, "looking for %s in compositeHash got %p\n",
		tdb->track, sgd);
	    assert(sgd != NULL);
	    checkOneSubGroups(tdb->track, childTdb, sgd);
	    }
        }
    }
}

static void checkSubGroups(char *database,struct trackDb *tdbList, boolean strict)
/* Check integrity of subGroup clauses */
{
struct hash *compositeHash = buildCompositeHash(database,tdbList);

verifySubTracks(tdbList, compositeHash);
}

static void rSetTrackDbFields(struct trackDb *tdbList)
/* Recursively fill in non-settings fields from settings. */
{
struct trackDb *tdb;
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    trackDbFieldsFromSettings(tdb);
    rSetTrackDbFields(tdb->subtracks);
    }
}


static void rPolish(struct trackDb *tdbList)
/* Call polisher on list and all children of list. */
{
struct trackDb *tdb;
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    trackDbPolish(tdb);
    rPolish(tdb->subtracks);
    }
}

static void polishSupers(struct trackDb *tdbList)
/* Run polish on supertracks. */
{
struct trackDb *tdb;
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    struct trackDb *parent = tdb->parent;
    if (parent != NULL)
	{
	trackDbFieldsFromSettings(parent);
	trackDbPolish(parent);
	}
    }
}

static struct trackDb *buildTrackDb(char *org, char *database, char *hgRoot, boolean strict)
/* Build trackDb objects from files. */
{
struct hash *trackHash = newHash(0);
char rootDir[PATH_LEN], orgDir[PATH_LEN], asmDir[PATH_LEN];

/* Create track list from hgRoot and hgRoot/org and hgRoot/org/assembly
 * ra format database. */

safef(rootDir, sizeof(rootDir), "%s", hgRoot);
safef(orgDir, sizeof(orgDir), "%s/%s", hgRoot, org);
safef(asmDir, sizeof(asmDir), "%s/%s/%s", hgRoot, org, database);

/* Must call these top-down.
 * Also prunes things not in our release. */
layerOnRa(strict, database, rootDir, trackHash, TRUE);
layerOnRa(strict, database, orgDir, trackHash, FALSE);
layerOnRa(strict, database, asmDir, trackHash, FALSE);

/* Represent hash as list */
struct trackDb *tdbList = trackDbListFromHash(trackHash);
trackDbAddTableField(tdbList);

/* Get rid of orphans with no parent of the correct release. */
tdbList = pruneOrphans(tdbList, trackHash);

/* After this the hash is no longer needed, so get rid of it. */
hashFree(&trackHash);

/* Read in HTML bits onto what remains. */
layerOnHtml(asmDir, tdbList, database);
layerOnHtml(orgDir, tdbList, database);
layerOnHtml(rootDir, tdbList, database);

/* Set up parent/subtracks pointers. */
tdbList = trackDbLinkUpGenerations(tdbList);

rSetTrackDbFields(tdbList);

/* Fill in any additional missing info from defaults. */
rPolish(tdbList);
polishSupers(tdbList);

/* Optionally check for tables/tracks that actually exist and get rid of ones that don't. */
if (strict)
    tdbList = pruneStrict(tdbList, database);

tdbList = pruneEmptyContainers(tdbList);
checkSubGroups(database,tdbList,strict);
trackDbPrioritizeContainerItems(tdbList);

return tdbList;
}

static struct trackDb *flatten(struct trackDb *tdbForest)
/* Convert our peculiar forest back to a list.
 * This for now rescues superTracks from the heavens. */
{
struct hash *superTrackHash = hashNew(0);
struct slRef *ref, *refList = trackDbListGetRefsToDescendants(tdbForest);

struct trackDb *tdbList = NULL;
for (ref = refList; ref != NULL; ref = ref->next)
    {
    struct trackDb *tdb = ref->val;
    struct trackDb *parent = tdb->parent;
    if (parent != NULL && tdbIsSuperTrack(parent))
	{
	/* The supertrack may appear as a 'floating' parent for multiple tracks.
	 * Only put it on the list once. */
	if (!hashLookup(superTrackHash, parent->track))
	    {
	    hashAdd(superTrackHash, parent->track, parent);
	    slAddHead(&tdbList, parent);
	    }
	}
    slAddHead(&tdbList, tdb);
    }

slFreeList(&refList);
hashFree(&superTrackHash);
slReverse(&tdbList);
return tdbList;
}

static int findMaxCartVersion(struct trackDb *tdbList)
/* Search the track list for the maximum cartVersion. */
{
struct trackDb *tdb;
int maxVal = 0;
for (tdb = tdbList; tdb != NULL; tdb = tdb->next)
    {
    char *value;
    if ((value = hashFindVal(tdb->settingsHash, "cartVersion")) != NULL)
        {
        int check = sqlUnsigned(value);
        if (check > maxVal)
            maxVal = check;
        }
    }
return maxVal;
}

static struct trackDb *makeCartVersionTrack(struct trackDb *tdbList)
/* Build a trackDb entry for the cartVersion pseudo track that keeps track of the
 * highest cartVersion used in this trackDb list.  
 */
{
struct trackDb *cartVerTdb;

AllocVar(cartVerTdb);

/* we negate cartVersion so the priority puts it first on the list. */
cartVerTdb->priority = -findMaxCartVersion(tdbList);

cartVerTdb->track = cloneString("cartVersion");
cartVerTdb->shortLabel = cloneString("cartVersion");
cartVerTdb->longLabel = cloneString("cartVersion");
cartVerTdb->html = cloneString("cartVersion");
cartVerTdb->type = cloneString("cartVersion");
cartVerTdb->url = cloneString("cartVersion");
cartVerTdb->grp = cloneString("cartVersion");

char buffer[1024];
safef(buffer, sizeof buffer, "cartVersion %d", (int)-cartVerTdb->priority);
cartVerTdb->settings = cloneString(buffer);

return cartVerTdb;
}

void hgTrackDb(char *org, char *database, char *trackDbName, char *sqlFile, char *hgRoot,
               boolean strict)
/* hgTrackDb - Create trackDb table from text files. */
{
struct trackDb *td;
char *tab = rTempName(getTempDir(), trackDbName, ".tab");

struct trackDb *tdbList = buildTrackDb(org, database, hgRoot, strict);
tdbList = flatten(tdbList);
slSort(&tdbList, trackDbCmp);

if (addVersion)
    slAddHead(&tdbList, makeCartVersionTrack(tdbList));
verbose(1, "Loaded %d track descriptions total\n", slCount(tdbList));

/* Write to tab-separated file; hold off on html, since it must be encoded */
    {
    verbose(2, "Starting write of tabs to %s\n", tab);
    FILE *f = mustOpen(tab, "w");
    for (td = tdbList; td != NULL; td = td->next)
        {
        hVarSubstTrackDb(td, database);
        char *hold = td->html;
        td->html = "";
	subChar(td->type, '\t', ' ');	/* Tabs confuse things. */
	subChar(td->shortLabel, '\t', ' ');	/* Tabs confuse things. */
	subChar(td->longLabel, '\t', ' ');	/* Tabs confuse things. */
	trackDbTabOut(td, f);
        td->html = hold;
        // be sure to change the settings hash with the updated substituted settings
        if (td->settingsHash)
            {
            hashReplace(td->settingsHash, "shortLabel", cloneString(td->shortLabel));
            hashReplace(td->settingsHash, "longLabel", cloneString(td->longLabel));
            }
        }
    carefulClose(&f);
    verbose(2, "Wrote tab representation to %s\n", tab);
    }

/* Update database */
    {
    char *create, *end;
    char query[2048];
    struct sqlConnection *conn = sqlConnect(database);

    /* Load in table definition. */
    readInGulp(sqlFile, &create, NULL);
    create = trimSpaces(create);
    create = substituteTrackName(create, trackDbName);
    end = create + strlen(create)-1;
    if (*end == ';') *end = 0;
    sqlSafef(query, sizeof(query), create, NULL);
    sqlRemakeTable(conn, trackDbName, query);

    /* Load in regular fields. */
    sqlSafef(query, sizeof(query), "load data local infile '%s' into table %s", tab, trackDbName);
    verbose(2, "sending mysql \"%s\"\n", query);
    sqlUpdate(conn, query);
    verbose(2, "done tab file load");

    /* Load in html and settings fields. */
    for (td = tdbList; td != NULL; td = td->next)
	{
        if (isEmpty(td->html))
	    {
	    if (strict && !noHtmlCheck && !trackDbLocalSetting(td, "parent") && !trackDbLocalSetting(td, "superTrack") &&
	        !sameString(td->track,"cytoBandIdeo"))
		{
		fprintf(stderr, "Warning: html missing for %s %s %s '%s'\n",org, database, td->track, td->shortLabel);
		}
	    }
	else
	    {
	    updateBigTextField(conn,  trackDbName, "tableName", td->track, "html", td->html);
	    }
	if (td->settingsHash != NULL)
	    {
	    char *settings = settingsFromHash(td->settingsHash, database);
	    updateBigTextField(conn, trackDbName, "tableName", td->track,
	        "settings", settings);
	    if (showSettings)
		{
		verbose(1, "%s: type='%s';", td->track, td->type);
		if (isNotEmpty(settings))
		    {
		    char *oneLine = replaceChars(settings, "\n", "; ");
		    eraseTrailingSpaces(oneLine);
		    verbose(1, " %s", oneLine);
		    freeMem(oneLine);
		    }
		verbose(1, "\n");
		}
	    freeMem(settings);
	    }
	}

    sqlSafef(query, sizeof(query), "flush tables");
    sqlUpdate(conn, query);
    sqlDisconnect(&conn);
    verbose(1, "Loaded database %s\n", database);
    }
    unlink(tab);
}

unsigned getReleaseBit(char *release)
/* make sure that the tag is a legal release */
{
if (sameString(release, "alpha"))
    return RELEASE_ALPHA;

if (sameString(release, "beta"))
    return RELEASE_BETA;

if (sameString(release, "public"))
    return RELEASE_PUBLIC;

errAbort("release must be alpha, beta, or public");

return 0;  /* make compiler happy */
}

struct hash *hashLines(char *fileName)
/* Read all lines in file and put them in a hash. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[1];
struct hash *hash = newHash(0);
while (lineFileRow(lf, row))
    hashAdd(hash, row[0], NULL);
lineFileClose(&lf);
return hash;
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc != 6)
    usage();
raName = optionVal("raName", raName);
showSettings = optionExists("settings");
if (strchr(raName, '/') != NULL)
    errAbort("-raName value should be a file name without directories");
release = optionVal("release", release);
releaseBit = getReleaseBit(release);
gbdbList = optionVal("gbdbList", gbdbList);
addVersion = optionExists("addVersion");
noHtmlCheck = optionExists("noHtmlCheck");

if (gbdbList)
    gbdbHash = hashLines(gbdbList);

hgTrackDb(argv[1], argv[2], argv[3], argv[4], argv[5], optionExists("strict"));
return 0;
}
