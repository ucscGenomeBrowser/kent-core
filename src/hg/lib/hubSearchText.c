/* hubSearchText.c was originally generated by the autoSql program, which also 
 * generated hubSearchText.h and hubSearchText.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rangeTree.h"
#include "hubSearchText.h"
#include "hubPublic.h"
#include "hgConfig.h"


char *hubSearchTextCommaSepFieldNames = "hubUrl,db,track,label,textLength,text,parents,parentTypes";

/* definitions for textLength column */
static char *values_textLength[] = {"Short", "Long", "Meta", NULL};
static struct hash *valhash_textLength = NULL;

void hubSearchTextStaticLoadWithNull(char **row, struct hubSearchText *ret)
/* Load a row from hubSearchText table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->hubUrl = row[0];
ret->db = row[1];
ret->track = row[2];
ret->label = row[3];
ret->textLength = sqlEnumParse(row[4], values_textLength, &valhash_textLength);
ret->text = row[5];
ret->parents = row[6];
ret->parentTypes = row[7];
}

struct hubSearchText *hubSearchTextLoadWithNull(char **row)
/* Load a hubSearchText from row fetched with select * from hubSearchText
 * from database.  Dispose of this with hubSearchTextFree(). */
{
struct hubSearchText *ret;

AllocVar(ret);
ret->hubUrl = cloneString(row[0]);
ret->db = cloneString(row[1]);
ret->track = cloneString(row[2]);
ret->label = cloneString(row[3]);
ret->textLength = sqlEnumParse(row[4], values_textLength, &valhash_textLength);
ret->text = cloneString(row[5]);
ret->parents = cloneString(row[6]);
ret->parentTypes = cloneString(row[7]);
return ret;
}

struct hubSearchText *hubSearchTextLoadAll(char *fileName) 
/* Load all hubSearchText from a whitespace-separated file.
 * Dispose of this with hubSearchTextFreeList(). */
{
struct hubSearchText *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileRow(lf, row))
    {
    el = hubSearchTextLoadWithNull(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hubSearchText *hubSearchTextLoadAllByChar(char *fileName, char chopper) 
/* Load all hubSearchText from a chopper separated file.
 * Dispose of this with hubSearchTextFreeList(). */
{
struct hubSearchText *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[8];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = hubSearchTextLoadWithNull(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hubSearchText *hubSearchTextCommaIn(char **pS, struct hubSearchText *ret)
/* Create a hubSearchText out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hubSearchText */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->hubUrl = sqlStringComma(&s);
ret->db = sqlStringComma(&s);
ret->track = sqlStringComma(&s);
ret->label = sqlStringComma(&s);
ret->textLength = sqlEnumComma(&s, values_textLength, &valhash_textLength);
ret->text = sqlStringComma(&s);
ret->parents = sqlStringComma(&s);
ret->parentTypes = sqlStringComma(&s);
*pS = s;
return ret;
}

void hubSearchTextFree(struct hubSearchText **pEl)
/* Free a single dynamically allocated hubSearchText such as created
 * with hubSearchTextLoad(). */
{
struct hubSearchText *el;

if ((el = *pEl) == NULL) return;
freeMem(el->hubUrl);
freeMem(el->db);
freeMem(el->track);
freeMem(el->label);
freeMem(el->text);
freeMem(el->parents);
freeMem(el->parentTypes);
freez(pEl);
}

void hubSearchTextFreeList(struct hubSearchText **pList)
/* Free a list of dynamically allocated hubSearchText's */
{
struct hubSearchText *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    hubSearchTextFree(&el);
    }
*pList = NULL;
}

void hubSearchTextOutput(struct hubSearchText *el, FILE *f, char sep, char lastSep) 
/* Print out hubSearchText.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hubUrl);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->db);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->track);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->label);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
sqlEnumPrint(f, el->textLength, values_textLength);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->text);
if (sep == ',') fputc('"',f);
fputc(sep, f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->parents);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->parentTypes);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


/* Restrictions on the size of context chunks returned by getTextContext() */
#define HST_MAXCONTEXTLENGTH 300
#define HST_CONTEXTRADIUS 70

static char *getTextContext(char *text, char *searchTerms)
/* Look for instances of words from searchTerms in the supplied text, grab some surrounding
 * context, and stitch them together into a string that combines overlapping regions. */
{
struct dyString *contextString = dyStringNew(0);
struct rbTree *contextTree = rangeTreeNew();
char *splitTerms[1024];
char *cloneTerms = cloneString(searchTerms);
if (isNotEmpty(cloneTerms))
    strLower(cloneTerms);
char *lowText = cloneString(text);
if (isNotEmpty(lowText))
    strLower(lowText);
int termCount = chopByWhiteRespectDoubleQuotes(cloneTerms, splitTerms, 1024);
int i;
for (i=0; i<termCount; i++)
    {
    char *thisTerm = splitTerms[i];
    if (thisTerm[0] == '"')
        {
        thisTerm++;
        trimLastChar(thisTerm);
        }
    char *position = stringIn(thisTerm, lowText);
    if (position == NULL)
        continue;
    int termStart = position - HST_CONTEXTRADIUS - lowText;
    if (termStart < 0)
        termStart = 0;
    int termEnd = position + strlen(thisTerm) + HST_CONTEXTRADIUS - lowText;
    if (termEnd > strlen(lowText))
        termEnd = strlen(lowText);
    rangeTreeAdd(contextTree, termStart, termEnd);
    }
struct range *rangeResult = rangeTreeList(contextTree);
int cumulativeSize = 0;
if (rangeResult != NULL)
    dyStringPrintf(contextString, "...");
while (rangeResult != NULL)
    {
    char contextChunk[HST_MAXCONTEXTLENGTH+1];
    int copySize = rangeResult->end - rangeResult->start;
    if (copySize > HST_MAXCONTEXTLENGTH)
        copySize = HST_MAXCONTEXTLENGTH;
    safencpy(contextChunk, sizeof(contextChunk), text+rangeResult->start, copySize);
    dyStringPrintf(contextString, " %s ...", contextChunk);
    cumulativeSize += copySize;
    if (cumulativeSize >= HST_MAXCONTEXTLENGTH)
        break;
    rangeResult = rangeResult->next;
    }
return dyStringCannibalize(&contextString);
}


struct hubSearchText *hubSearchTextLoadWithNullGiveContext(char **row, char *searchTerms)
/* Load a hubSearchText from row fetched with select * from hubSearchText
 * from database, but instead of loading the entire text field for long text results,
 * only load the pieces that provide context for the supplied searchTerms.
 * Dispose of this with hubSearchTextFree(). */
{
struct hubSearchText *hst = hubSearchTextLoadWithNull(row);
if (hst->textLength == hubSearchTextLong)
    {
    char *longTextContext = getTextContext(hst->text, searchTerms);
    freeMem(hst->text);
    hst->text = longTextContext;
    }
return hst;
}

char *modifyTermsForHubSearch(char *hubSearchTerms, bool isStrictSearch)
/* This won't exactly be pretty.  MySQL treats any sequence of alphanumerics and underscores
 * as a word, and single apostrophes are allowed as long as they don't come back-to-back.
 * Cut down to those characters, then add initial + (for requiring word) and * (for word
 * expansion) as appropriate. */
{
char *cloneTerms = cloneString(hubSearchTerms);
struct dyString *modifiedTerms = dyStringNew(0);
if (isNotEmpty(cloneTerms))
    {
    int i;
    for (i=0; i<strlen(cloneTerms); i++)
        {
        // allowed punctuation is underscore and apostrophe, and we'll do special handling for hyphen
        if (!isalnum(cloneTerms[i]) && cloneTerms[i] != '_' && cloneTerms[i] != '\'' &&
                cloneTerms[i] != '-')
            cloneTerms[i] = ' ';
        }
    char *splitTerms[1024];
    int termCount = chopByWhite(cloneTerms, splitTerms, sizeof(splitTerms));
    for (i=0; i<termCount; i++)
        {
        char *hyphenatedTerms[1024];
        int hyphenTerms = chopString(splitTerms[i], "-", hyphenatedTerms, sizeof(hyphenatedTerms));
        int j;
        for (j=0; j<hyphenTerms-1; j++)
            {
            dyStringPrintf(modifiedTerms, "+%s ", hyphenatedTerms[j]);
            }
        if (isStrictSearch)
            dyStringPrintf(modifiedTerms, "+%s ", hyphenatedTerms[j]);
        else
            {
            dyStringPrintf(modifiedTerms, "+%s* ", hyphenatedTerms[j]);
            }
        }
    }
return dyStringCannibalize(&modifiedTerms);
}

void getHubSearchResults(struct sqlConnection *conn, char *hubSearchTableName,
        char *hubSearchTerms, bool checkLongText, char *dbFilter, struct hash *hubLookup,
        struct hash **searchResultHashRet, struct slName **hubsToPrintRet, char *extra)
/* Find hubs, genomes, and tracks that match the provided search terms.
 * Return all hits that satisfy the (optional) supplied assembly filter.
 * if checkLongText is FALSE, skip searching within the long description text entries
 * the string 'extra' is used for additional mysql filter(s) by the caller and
 * must be escaped by them */
{
char *cleanSearchTerms = cloneString(hubSearchTerms);
if (isNotEmpty(cleanSearchTerms))
    tolowers(cleanSearchTerms);
bool isStrictSearch = FALSE;
char *modifiedSearchTerms = modifyTermsForHubSearch(cleanSearchTerms, isStrictSearch);
struct hubSearchText *hubSearchResultsList = NULL;
struct dyString *query = dyStringNew(100);

sqlDyStringPrintf(query, "select * from %s where ", hubSearchTableName);
if (isNotEmpty(extra))
    {
    sqlDyStringPrintf(query, "%-s ", extra);
    if (!checkLongText || isNotEmpty(modifiedSearchTerms))
        sqlDyStringPrintf(query, "and ");
    }
if (!checkLongText)
    {
    sqlDyStringPrintf(query, "textLength = 'Short'"); 
    if (isNotEmpty(modifiedSearchTerms))
       sqlDyStringPrintf(query, "and ");
    }

if (isNotEmpty(modifiedSearchTerms))
    {
    sqlDyStringPrintf(query, "match(text) against ('%s' in boolean mode)"
        " order by match(text) against ('%s' in boolean mode)",
        modifiedSearchTerms, modifiedSearchTerms);
    }
struct sqlResult *sr = sqlGetResult(conn, dyStringContents(query));
char **row;
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct hubSearchText *hst = hubSearchTextLoadWithNullGiveContext(row, cleanSearchTerms);
    // Skip rows where the long text matched the more lax MySQL search (punctuation just
    // splits terms into two words, so "rna-seq" finds "rna" and "seq" separately, but
    // not the more strict rules used to find context for the search terms.
    if ((hst->textLength == hubSearchTextLong) && isEmpty(hst->text))
        continue;
    char *hubUrl = hst->hubUrl;
    struct hubEntry *hubInfo = hashFindVal(hubLookup, hubUrl);
    if (hubInfo == NULL)
        continue; // Search table evidently includes a hub that's not on this server.  Skip it.
    char *db = cloneString(hst->db);
    tolowers(db);
    if (isNotEmpty(dbFilter))
        {
        if (isNotEmpty(db))
            {
            if (stringIn(dbFilter, db) == NULL)
                continue;
            }
        else
            {
            // no db in the hubSearchText means this is a top-level hub hit.
            // filter by the db list associated with the hub instead
            char *dbList = cloneString(hubInfo->dbList);
            tolowers(dbList);
            if (stringIn(dbFilter, dbList) == NULL)
                continue;
            }
        }
    // Add hst to the list to be returned
    slAddHead(&hubSearchResultsList, hst);
    }
slReverse(&hubSearchResultsList);
struct slName *hubsToPrint = NULL;
struct hash *searchResultHash = hashNew(0);
struct hubSearchText *hst = hubSearchResultsList;
while (hst != NULL)
    {
    struct hubSearchText *nextHst = hst->next;
    hst->next = NULL;
    struct hashEl *hubHashEnt = hashLookup(searchResultHash, hst->hubUrl);
    if (hubHashEnt == NULL)
        {
        slNameAddHead(&hubsToPrint, hst->hubUrl);
        hashAdd(searchResultHash, hst->hubUrl, hst);
        }
    else
        slAddHead(&(hubHashEnt->val), hst);
    hst = nextHst;
    }
struct hashEl *hel;
struct hashCookie cookie = hashFirst(searchResultHash);
while ((hel = hashNext(&cookie)) != NULL)
    slReverse(&(hel->val));
*searchResultHashRet = searchResultHash;
*hubsToPrintRet = hubsToPrint;
}

char *hubSearchTextTableName()
{
return cfgOptionDefault("hubSearchTextTable", "hubSearchText");
}
