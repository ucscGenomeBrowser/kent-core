/* das - Distributed Annotation System server.
 * Needs to be called from a Web server generally.
 * You can spoof it from the command line by giving
 * it a command argument such as:
 *      das dsn
 *      das hg6/types segment=chr22
 */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "cheapcgi.h"
#include "htmshell.h"
#include "psl.h"
#include "jksql.h"
#include "hdb.h"
#include "chromInfo.h"
#include "bed.h"
#include "genePred.h"
#include "trackTable.h"
#include "botDelay.h"


/* Including the count in the types response can be very slow for large
 * regions and is optional.  Inclusion of count if controlled by this compile-
 * time option. */
#define TYPES_RETURN_COUNT 0

static char *version = "1.00";
static char *database = NULL;	

/* DAS response codes */
#define DAS_OK                     200
#define DAS_BAD_COMMAND            400
#define DAS_BAD_DATA_SOURCE        401
#define DAS_BAD_COMMAND_ARGS       402
#define DAS_BAD_REFERENCE_OBJECT   403
#define DAS_BAD_STYLESHEET         404
#define DAS_COORDINATE_ERROR       405
#define DAS_SERVER_ERROR           500
#define DAS_UNIMPLEMENTED_FEATURE  501

static void dasHead(int code, boolean justText)
/* Write out very start of DAS header */
{
printf("X-DAS-Version: DAS/0.95\n");
printf("X-DAS-Status: %d\n", code);
if (justText)
    printf("Content-Type:text\n");
else
    printf("Content-Type:text/xml\n");
// these allow access from javascript, see http://www.w3.org/TR/cors/
printf("Access-Control-Allow-Origin: *\n");
printf("Access-Control-Expose-Headers: X-DAS-Version X-DAS-Status X-DAS-Capabilities\n");
printf("\n");
}

static void dasHeader(int code)
/* Write out DAS header */
{
dasHead(code, FALSE);
if (code != DAS_OK)
    exit(-1);
printf("<?xml version=\"1.0\" standalone=\"no\"?>\n");
}

static char dasStrand(char strand)
/* convert a strand to a valid DAS strand (+,-,0) */
{
if ((strand == '+') || (strand == '-'))
    return strand;
else
    return '0';
}


void sendHogMessage(char *hogHost)
{
dasHead(DAS_OK, TRUE);
printf("Your host, %s, has been sending too many requests lately and is "
       "unfairly loading our site, impacting performance for other users. "
       "Please contact genome-www@soe.ucsc.edu to ask that your site "
       "be reenabled.  Also, please consider downloading sequence and/or "
       "annotations in bulk -- see http://genome.ucsc.edu/downloads.html.",
       hogHost);
exit(0);
}


void blockHog(char *hogHost, char *hogAddr)
/* Compare host/addr to those of an abusive client that we want to block. */
{
char *rhost = getenv("REMOTE_HOST");
char *raddr = getenv("REMOTE_ADDR");
if ((rhost != NULL && sameWord(rhost, hogHost)) ||
    (raddr != NULL && sameWord(raddr, hogAddr)))
    {
    sendHogMessage(hogHost);
    }
}

static void dasHelp(char *s)
/* Put up some hopefully helpful information. */
{
dasHead(DAS_OK, TRUE);
puts(s);
exit(0);
}

static void dasAbout()
/* Print a little info when they just hit cgi-bin/das. */
{
dasHead(DAS_OK, TRUE);
dasHelp("UCSC DAS Server.\n"
    "See http://www.biodas.org for more info on DAS.\n"
    "Try http://genome.ucsc.edu/cgi-bin/das/dsn for a list of databases.\n"
    "See our DAS FAQ (http://genome.ucsc.edu/FAQ/FAQdownloads#download23)\n"
    "for more information.  Alternatively, we also provide query capability\n"
    "through our MySQL server; please see our FAQ for details\n"
    "(http://genome.ucsc.edu/FAQ/FAQdownloads#download29).\n\n"
    "Note that DAS is an inefficient protocol which does not support\n"
    "all types of annotation in our database.  We recommend you\n"
    "1) use our API at https://api.genome.ucsc.edu/\n"
    "2) access the UCSC database by downloading the tab-separated files in\n"
    "the downloads section (http://hgdownload.soe.ucsc.edu/downloads.html)\n"
    "3) use the Table Browser (http://genome.ucsc.edu/cgi-bin/hgTables)\n"
    "instead of DAS in most circumstances.\n"
    "4) alternatively, use our command line tools. They can all load data directly from a URL,\n"
    "e.g. \n"
    "  wget https://hgdownload.cse.ucsc.edu/admin/exe/linux.x86_64/twoBitToFa\n"
    "  chmod a+x twoBitToFa\n"
    "  twoBitToFa https://hgdownload.soe.ucsc.edu/goldenPath/hg38/bigZips/latest/hg38.2bit -bed=test.bed out.fa\n"
    "See also the 'Data Access' section of any genome browser track with details on how to\n"
    "access the annotation data using our tools or contact us by email so we can suggest the fastest way forward..\n"
    );
exit(0);
}

static void normalHeader()
/* Write normal (non-error) header. */
{
dasHeader(DAS_OK);
}

static void earlyError(int errCode)
/* Return error in early processing (before writing header) */
{
dasHeader(errCode);
}

static char *currentUrl()
/* Query environment to get current URL.  WARNING: static return */
{
static char url[1024];
safef(url, sizeof(url), "http://%s%s%s", getenv("SERVER_NAME"), getenv("SCRIPT_NAME"), getenv("PATH_INFO"));
return url;
}

struct tableDef
/* A table definition. */
    {
    struct tableDef *next;	/* Next in list. */
    char *name;			/* Name of table. */
    struct slName *splitTables;	/* Names of subTables.  Only used if isSplit. */
    char *chromField;		/* Name of field chromosome is stored in. */
    char *startField;		/* Name of field chromosome start is in. */
    char *endField;		/* Name of field chromosome end is in. */
    char *category;		/* Category type. */
    char *method;		/* Category type. */
    boolean hasBin;		/* Has bin field. */
    };

static boolean hasLogicalChromName(char *name)
/* Return TRUE if name begins with "chr"*/
{
return (startsWith("chr", name));
}

static boolean tableIsSplit(char *table)
/* Return TRUE if table is split. */
{
if (!hasLogicalChromName(table))
    return FALSE;
if (strchr(table, '_') == NULL)
    return FALSE;
return TRUE;
}

static char *skipOverChrom(char *table)
/* Skip over chrN_ or chrN_random_. */
{
char *e = strchr(table, '_');
if (e != NULL)
    {
    ++e;
    if (startsWith("random_", e))
	e += 7;
    table = e;
    }
return table;
}

static char *chromNumberToName(char *seqName)
/* If seqName is a chrom number, prepend "chr". */
{
char *official = NULL;
char chrName[128];
safef(chrName, sizeof(chrName), "chr%s", seqName);
official = hgOfficialChromName(database, chrName);
if (official == NULL)
    official = seqName;
return(official);
}


static boolean dasableType(char *type)
/* Return TRUE if we can handle type. */
{
char *dupe = cloneString(type);
char *line = dupe, *word;
boolean ok = TRUE;
word = nextWord(&line);
if (word != NULL)
    {
    if (sameString("chain", word))
         ok = FALSE;
    else if (sameString("net", word))
         ok = FALSE;
    else if (sameString("netAlign", word))
         ok = FALSE;
    }
freeMem(dupe);
return ok;
}

static boolean disabledViaSettings(char *settings)
// Return TRUE if this table is disabled via "tableBrowser off" in trackDb settings string.
// We modify settings string (pass in a copy if you don't want it modified).
{
char *line;
boolean disabled = FALSE;
struct lineFile *lf = lineFileOnString("settings", TRUE, settings);

while (!disabled && lineFileNext(lf, &line, NULL))
    disabled = startsWith("tableBrowser off", line);
lineFileClose(&lf);
return disabled;
}

static struct hash *mkTrackTypeHash()
/* build a hash of track name to type */
{
struct sqlConnection *conn = hAllocConn(database);
struct hash *hash = hashNew(10);
struct slName *trackDb, *trackDbs = hTrackDbList();
for (trackDb = trackDbs; trackDb != NULL; trackDb = trackDb->next)
    {
    if (sqlTableExists(conn, trackDb->name))
        {
        char query[128];
        sqlSafef(query, sizeof(query), "select tableName,type,settings from %s", trackDb->name);
        struct sqlResult *sr = sqlGetResult(conn, query);
        char **row;
        while ((row = sqlNextRow(sr)) != NULL)
            {
            if (dasableType(row[1]) && !disabledViaSettings(row[2]) && (hashLookup(hash, row[0]) == NULL))
                hashAdd(hash, row[0], NULL);
            }
        sqlFreeResult(&sr);
        }
    }
slFreeList(&trackDbs);
hFreeConn(&conn);
return hash;
}

static boolean dasableTrack(char *name)
/* Return TRUE if track can be put into DAS format. */
{
static struct hash *types = NULL;
if (types == NULL)
    types = mkTrackTypeHash();
return hashLookup(types, name) != NULL;
}

static struct hash *mkSkipTableHash()
/* build hash of big tables that we don't actually want to serve. */

{
struct hash *skips = hashNew(0);
hashAdd(skips, "all_est", NULL);
hashAdd(skips, "all_mrna", NULL);
hashAdd(skips, "refFlat", NULL);
hashAdd(skips, "simpleRepeat", NULL);
hashAdd(skips, "ctgPos", NULL);
hashAdd(skips, "gold", NULL);
hashAdd(skips, "clonePos", NULL);
hashAdd(skips, "gap", NULL);
hashAdd(skips, "rmsk", NULL);
hashAdd(skips, "estPair", NULL);
hashAdd(skips, "altGraphX", NULL);
return skips;
}

static boolean skipTable(char *name)
/* should big tables be skipped. */
{
static struct hash *skips = NULL;
if (skips == NULL)
    skips = mkSkipTableHash();
return hashLookup(skips, name) != NULL;
}

static struct tableDef *getTables()
/* Get all tables. */
{
struct sqlConnection *conn = hAllocConn(database);
struct hash *hash = newHash(0);
struct tableDef *tdList = NULL, *td;
struct sqlResult *sr;
char **row;
char *table, *root;
boolean isSplit, hasBin;
char chromField[32], startField[32], endField[32];

char query[1024];
sqlSafef(query, sizeof query, "show tables"); 
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    table = root = row[0];
    if (hFindFieldsAndBin(database, table, chromField, startField, endField, &hasBin))
	{
	isSplit = tableIsSplit(table);
	if (isSplit)
	    root = skipOverChrom(table);
	if (!skipTable(root) && dasableTrack(root))
	    {
	    if ((td = hashFindVal(hash, root)) == NULL)
		{
		AllocVar(td);
		slAddHead(&tdList, td);
		hashAdd(hash, root, td);
		td->name = cloneString(root);
		td->chromField = cloneString(chromField);
		td->startField = cloneString(startField);
		td->endField = cloneString(endField);
		td->hasBin = hasBin;
		if (stringIn("snp", root) || stringIn("Snp", root))
		     {
		     td->category = "variation";
		     }
		else if (stringIn("est", root) || stringIn("Est", root) 
		    || stringIn("mrna", root) || stringIn("Mrna", root))
		    {
		    td->category = "transcription";
		    td->method = "BLAT";
		    }
		else if (sameString("txStart", startField) || stringIn("rnaGene", root))
		    {
		    td->category = "transcription";
		    }
		else if (stringIn("mouse", root) || stringIn("Mouse", root) ||
		    stringIn("fish", root) || stringIn("Fish", root))
		    {
		    td->category = "similarity";
		    if (startsWith("blat", root))
		         td->method = "BLAT";
		    else if (sameString("exoMouse", root))
		         td->method = "Exonerate";
		    else if (sameString("exoMouse", root))
		         td->method = "Exofish";
		    }
		else if (sameString("gap", root) || sameString("gold", root) ||
			sameString("gl", root) || sameString("clonePos", root))
		    {
		    td->category = "structural";
		    td->method = "GigAssembler";
		    }
		else
		    td->category = "other";
		}
	    if (isSplit)
	        {
		struct slName *sln = newSlName(table);
		slAddHead(&td->splitTables, sln);
		}
	    }
	}
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
hashFree(&hash);
slReverse(&tdList);
return tdList;
}

static struct hash *hashOfTracks()
/* Get list of tracks and put into hash keyed by tableName*/
{
struct hash *trackHash = newHash(7);
struct trackTable *ttList = hGetTracks(database), *tt;
for (tt = ttList; tt != NULL; tt = tt->next)
    hashAdd(trackHash, tt->tableName, tt);
return trackHash;
}

struct segment
/* A subset of a sequence. */
    {
    struct segment *next;	/* Next in list. */
    char *seq;		/* Name of sequence, often a chromosome. */
    int start;		/* Zero based start. */
    int end;		/* Base after end. */
    boolean wholeThing;	/* TRUE if user requested whole sequence. */
    char *seqName;      /* Name of sequence in external DAS world */
    };

static struct segment *segmentNew(char *seq, int start, int end, boolean wholeThing,
                                  char *seqName)
/* Make a new segment. */
{
struct segment *segment;
AllocVar(segment);
segment->seq = cloneString(seq);
segment->start = start;
segment->end = end;
segment->wholeThing = wholeThing;
segment->seqName = cloneString(seqName);
return segment;
}

static struct segment *dasSegmentList(boolean mustExist)
/* Get a DAS segment list from either segment or ref parameters. 
 * Call this before you write out header so it can return errors
 * properly if parameters are malformed. */
{
struct slName *segList, *seg;
struct segment *segmentList = NULL, *segment;
char *seq, *seqName;
int start=0, end=0;
boolean wholeThing;

segList = cgiStringList("segment");
if (segList != NULL)
    {
    /* Handle new format (post 0.995 spec) lists. */
    for (seg = segList; seg != NULL; seg = seg->next)
	{
	char *parts[3];
	int partCount;

	wholeThing = FALSE;
	partCount = chopString(seg->name, ":,", parts, ArraySize(parts));
	seqName = parts[0];
	seq = chromNumberToName(seqName);
	if (partCount == 1)
	    {
	    start = 0;
	    end = hChromSize(database, seq);
	    wholeThing = TRUE;
	    }
	else if (partCount == 3)
	    {
	    if (!isdigit(parts[1][0]) || !isdigit(parts[2][0]))
		earlyError(DAS_BAD_COMMAND_ARGS);
	    start = atoi(parts[1])-1;
	    end = atoi(parts[2]);
	    if (start > end)
	        earlyError(DAS_COORDINATE_ERROR);
	    }
	else
	    {
	    earlyError(DAS_BAD_COMMAND_ARGS);
	    }
	segment = segmentNew(seq, start, end, wholeThing, seqName);
	slAddHead(&segmentList, segment);
	}
    slReverse(&segmentList);
    }
else
    {
    /* Handle old format (pre 0.995 spec) lists. */
    seqName = cgiOptionalString("ref");
    wholeThing = TRUE;
    if (seqName == NULL)
	{
	if (mustExist)
	    earlyError(DAS_BAD_COMMAND_ARGS);
	else
	    {
	    return NULL;
	    }
	}
    seq = chromNumberToName(seqName);
    if (cgiVarExists("start"))
	{
        start = cgiInt("start")-1;
	wholeThing = FALSE;
	}
    else
        start = 0;
    if (cgiVarExists("stop"))
	{
        end = cgiInt("stop");
	wholeThing = FALSE;
	}
    else
        end = hChromSize(database, seq);
    if (start > end)
	earlyError(DAS_COORDINATE_ERROR);
    segmentList = segmentNew(seq, start, end, wholeThing, seqName);
    }
/* Check all segments are chromosomes. */
for (segment = segmentList; segment != NULL; segment = segment->next)
    {
    if (hgOfficialChromName(database, segment->seq) == NULL)
        earlyError(DAS_BAD_REFERENCE_OBJECT);
    }
return segmentList;
}

struct filters
/* hash tables of filter values. */
{
    struct hash *type;      // type filters, or NULL if no filters
    struct hash *category;  // category filters, or NULL if no filters
};

static struct hash *filtersFromCgiVar(char *cgiVar)
/* build hash of filters from a cgiVar name */
{
struct slName *nList = cgiStringList(cgiVar), *n;
if (nList == NULL)
    return NULL;  // no filters
struct hash *filters = hashNew(10);
for (n = nList; n != NULL; n = n->next)
    hashStore(filters, n->name);
return filters;
}

static struct filters *filtersNew()
/* construct filters for type and category */
{
struct filters *filters;
AllocVar(filters);
filters->type = filtersFromCgiVar("type");
filters->category = filtersFromCgiVar("category");
return filters;
}

static boolean catTypeFilter(struct filters *filters, char *cat, char *type)
/* Combined category/type filter. */
{
/* spec doesn't says if both category and type results a AND or OR, so we
 * treat it as an AND, as this seems more usefully. */
if ((filters->category != NULL) && (filters->type != NULL))
    return (hashLookup(filters->category,cat) != NULL) && (hashLookup(filters->type, type) != NULL);
if (filters->category != NULL)
    return (hashLookup(filters->category,cat) != NULL);
if (filters->type != NULL)
    return (hashLookup(filters->type, type) != NULL);
return TRUE;
}

static boolean tableDefFilter(struct tableDef *td)
/* determine if a tableDef has the required information to use the track */
{
return isNotEmpty(td->chromField) && isNotEmpty(td->startField)
    && isNotEmpty(td->endField);
}

static boolean trackFilter(struct filters *filters, struct tableDef *td)
/* do track filtering. */
{
return tableDefFilter(td) && catTypeFilter(filters, td->category, td->name);
}

static void doDsn(struct slName *dbList)
/* dsn - DSN Server for DAS. */
{
struct slName *db;

normalHeader();
printf(
"<!DOCTYPE DASDSN SYSTEM \"http://www.biodas.org/dtd/dasdsn.dtd\">\n"
"<DASDSN>\n");
for (db = dbList; db != NULL; db = db->next)
    {
    char *freeze = hFreezeFromDb(db->name);
    char *organism = hOrganism(db->name);
    if (! hDbIsActive(db->name))
	continue;
    printf("   <DSN>\n");
    printf("     <SOURCE id=\"%s\" version=\"%s\">%s at UCSC</SOURCE>\n", 
    	db->name, version, freeze);
    printf("     <DESCRIPTION>%s %s Genome at UCSC</DESCRIPTION>\n", organism, freeze);
    printf("     <MAPMASTER>http://genome.ucsc.edu:80/cgi-bin/das/%s</MAPMASTER>\n",
        db->name);
    printf("   </DSN>\n");
    }
printf("</DASDSN>\n");
}

static int countFeatures(struct tableDef *td, struct segment *segmentList)
/* Count all the features in a given segment. */
{
struct segment *segment;
int acc = 0;
struct sqlConnection *conn = hAllocConn(database);
char chrTable[256];
char query[512];
struct slName *n;

if (segmentList == NULL)
    {
    if (td->splitTables == NULL)
        {
	sqlSafef(query, sizeof query, "select count(*) from %s", td->name);
	acc = sqlQuickNum(conn, query);
	}
    else
        {
	for (n = td->splitTables; n != NULL; n = n->next)
	    {
	    sqlSafef(query, sizeof query, "select count(*) from %s", n->name);
	    acc += sqlQuickNum(conn, query);
	    }
	}
    }
else
    {
    for (segment = segmentList; segment != NULL; segment = segment->next)
	{
	if (segment->wholeThing)
	    {
	    if (td->splitTables == NULL)
	        {
		sqlSafef(query, sizeof query, "select count(*) from %s where %s = '%s'", 
			td->name, td->chromField, segment->seq);
		acc += sqlQuickNum(conn, query);
		}
	    else
	        {
		sprintf(chrTable, "%s_%s", segment->seq, td->name);
		if (sqlTableExists(conn, chrTable))
		    {
		    sqlSafef(query, sizeof query, "select count(*) from %s", 
			    chrTable);
		    acc += sqlQuickNum(conn, query);
		    }
		}
	    }
	else
	    {
	    if (td->splitTables == NULL)
	        {
		sqlSafef(query, sizeof query, "select count(*) from %s where %s = '%s' and %s < %d and %s > %d",
		     td->name, td->chromField, segment->seq,
		     td->startField, segment->end, td->endField, segment->start);
		acc += sqlQuickNum(conn, query);
		}
	    else
	        {
		sprintf(chrTable, "%s_%s", segment->seq, td->name);
		if (sqlTableExists(conn, chrTable))
		    {
		    sqlSafef(query, sizeof query, "select count(*) from %s where %s < %d and %s > %d", chrTable, 
			 td->startField, segment->end, td->endField, segment->start);
		    acc += sqlQuickNum(conn, query);
		    }
		}
	    }
	}
    }
hFreeConn(&conn);
return acc;
}
 

static void doTypes()
/* Handle a types request. */
{
struct segment *segment, *segmentList = dasSegmentList(FALSE);
struct tableDef *tdList = getTables(), *td;
struct filters *filters = filtersNew();

normalHeader();
printf("<!DOCTYPE DASTYPES SYSTEM \"http://www.biodas.org/dtd/dastypes.dtd\">\n");
printf("<DASTYPES>\n");
printf("<GFF version=\"1.2\" href=\"%s\">\n", currentUrl());
for (segment = segmentList;;)
    {
    if (segment == NULL)
        printf("<SEGMENT version=\"%s\">\n", version);
    else
	printf("<SEGMENT id=\"%s\" start=\"%d\" stop=\"%d\" version=\"%s\">\n", 
	    segment->seqName, segment->start+1, segment->end, version);
    for (td = tdList; td != NULL; td = td->next)
	{
	if (trackFilter(filters, td))
	    {
	    printf("<TYPE id=\"%s\" category=\"%s\"", td->name, td->category);
	    if (td->method != NULL)
		printf(" method=\"%s\"", td->method);
            if (TYPES_RETURN_COUNT)
                printf(">%d</TYPE>\n", countFeatures(td, segment));
            else
                printf("/>\n");
                
	    }
	}
    printf("</SEGMENT>\n");
    if (segment == NULL || segment->next == NULL)
        break;
    segment = segment->next;
    }
printf("</GFF>\n");
printf("</DASTYPES>\n");
}

static void dasPrintType(struct tableDef *td, struct trackTable *tt)
/* Print out from <TYPE> to </TYPE> inside a feature. */
{
char *description = (tt != NULL ? tt->shortLabel : td->name);
printf(" <TYPE id=\"%s\" category=\"%s\" reference=\"no\">%s</TYPE>\n", td->name, td->category, description);
}

static void dasOutGp(struct genePred *gp, struct tableDef *td, struct trackTable *tt)
/* Write out DAS info on a gene prediction. */
{
int i;
for (i=0; i<gp->exonCount; ++i)
    {
    int start = gp->exonStarts[i];
    int end =  gp->exonEnds[i];
    printf(
    "<FEATURE id=\"%s.%s.%d.%d\" label=\"%s\">\n", gp->name, gp->chrom, gp->txStart, i, gp->name);
    dasPrintType(td, tt);
    if (td->method != NULL)
	printf(" <METHOD id=\"id\">%s</METHOD>\n", td->method);
    else
	printf(" <METHOD></METHOD>\n");
    printf(" <START>%d</START>\n", start+1);
    printf(" <END>%d</END>\n", end);
    printf(" <SCORE>-</SCORE>\n");
    printf(" <ORIENTATION>%c</ORIENTATION>\n", dasStrand(gp->strand[0]));
    printf(" <PHASE>-</PHASE>\n");
    printf(" <GROUP id=\"%s.%s.%d\">\n", gp->name, gp->chrom, gp->txStart);
    printf("  <LINK href=\"http://genome.ucsc.edu/cgi-bin/hgTracks?position=%s:%d-%d&amp;db=%s\">Link to UCSC Browser</LINK>\n", 
	gp->chrom, gp->txStart, gp->txEnd, database);
    printf(" </GROUP>\n");
    printf("</FEATURE>\n");
    }
}

static void dasOutGpSegment(struct sqlResult *sr, int rowOffset, struct tableDef *td, struct trackTable *tt)
/* output genePreds resulting from query */
{
char **row;
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct genePred *gp = genePredLoad(row+rowOffset);
    dasOutGp(gp, td, tt);
    genePredFree(&gp);
    }
}

static void dasOutPsl(struct psl *psl, struct tableDef *td, struct trackTable *tt)
/* Write out DAS info on a psl alignment. */
{
int i;
int score = 1000 - pslCalcMilliBad(psl, TRUE);
for (i=0; i<psl->blockCount; ++i)
    {
    int s = psl->tStarts[i];
    int e = s + psl->blockSizes[i];
    int qStart, qEnd;
    int start,end;
    if (psl->strand[1] == '-')
        {
	start = psl->tSize - e;
	end = psl->tSize - s;
	}
    else
        {
	start = s;
	end = e;
	}
    s = psl->qStarts[i];
    e = s + psl->blockSizes[i];
    if (psl->strand[0] == '-')
        {
	qStart = psl->qSize - e;
	qEnd = psl->qSize - s;
	}
    else
        {
	qStart = s;
	qEnd = e;
	}
    printf(
    "<FEATURE id=\"%s.%s.%d.%d\" label=\"%s\">\n", psl->qName, psl->tName, psl->tStart, i, psl->qName);
    dasPrintType(td, tt);
    if (td->method != NULL)
	printf(" <METHOD id=\"id\">%s</METHOD>\n", td->method);
    else
	printf(" <METHOD></METHOD>\n");
    printf(" <START>%d</START>\n", start+1);
    printf(" <END>%d</END>\n", end);
    printf(" <SCORE>%d</SCORE>\n", score);
    printf(" <ORIENTATION>%c</ORIENTATION>\n", dasStrand(psl->strand[0]));
    printf(" <PHASE>-</PHASE>\n");
    printf(" <GROUP id=\"%s.%s.%d\">\n", psl->qName, psl->tName, psl->tStart);
    printf("  <LINK href=\"http://genome.ucsc.edu/cgi-bin/hgTracks?position=%s:%d-%d&amp;db=%s\">Link to UCSC Browser</LINK>\n", 
	psl->tName, psl->tStart, psl->tEnd, database);
    printf("  <TARGET id=\"%s\" start=\"%d\" stop=\"%d\">%s</TARGET>\n",
        psl->qName, qStart+1, qEnd, psl->qName);
    printf(" </GROUP>\n");
    printf("</FEATURE>\n");
    }
}

static void dasOutPslSegment(struct sqlResult *sr, int rowOffset, struct tableDef *td, struct trackTable *tt)
/* output psls resulting from query */
{
char **row;
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct psl *psl = pslLoad(row+rowOffset);
    dasOutPsl(psl, td, tt);
    pslFree(&psl);
    }
}

static void dasOutBed(char *chrom, int start, int end, 
	char *name, char *score,  char *strand,
	struct tableDef *td, struct trackTable *tt)
/* Write out a generic one. */
{
printf("<FEATURE id=\"%s.%s.%d\" label=\"%s\">\n", name, chrom, start, name);
dasPrintType(td, tt);
if (td->method != NULL)
    printf(" <METHOD id=\"id\">%s</METHOD>\n", td->method);
else
    printf(" <METHOD></METHOD>\n");
printf(" <START>%d</START>\n", start+1);
printf(" <END>%d</END>\n", end);
printf(" <SCORE>%s</SCORE>\n", score);
printf(" <ORIENTATION>%c</ORIENTATION>\n", dasStrand(strand[0]));
printf(" <PHASE>-</PHASE>\n");
printf(" <GROUP id=\"%s.%s.%d\">\n", name, chrom, start);
printf("  <LINK href=\"http://genome.ucsc.edu/cgi-bin/hgTracks?position=%s:%d-%d&amp;db=%s\">Link to UCSC Browser</LINK>\n", 
    chrom, start, end, database);
printf(" </GROUP>\n");
printf("</FEATURE>\n");
}

static void dasOutBedSegment(struct sqlResult *sr, char *table, struct tableDef *td,
                             struct trackTable *tt)
/* output BEDs and BED like tables resulting from query */
{
char **row;
int chromIx = sqlFieldColumn(sr, "chrom");
int chromStartIx = sqlFieldColumn(sr, "chromStart");
int chromEndIx = sqlFieldColumn(sr, "chromEnd");
int nameIx = sqlFieldColumn(sr, "name");
int scoreIx = sqlFieldColumn(sr, "score");
int strandIx = sqlFieldColumn(sr, "strand");

if (nameIx == -1)
    nameIx = sqlFieldColumn(sr, "contig");
if (scoreIx == -1)
    scoreIx = sqlFieldColumn(sr, "gcPpt");
if (scoreIx == -1)
    scoreIx = sqlFieldColumn(sr, "dataValue");  // bedGraph
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *chrom = (chromIx >= 0) ? row[chromIx] : "-";
    char *chromStart = (chromStartIx >= 0) ? row[chromStartIx] : "0";
    char *chromEnd = (chromEndIx >= 0) ? row[chromEndIx] : "0";
    char *name = (nameIx >= 0 ? row[nameIx] : td->name);
    char *score = (scoreIx >= 0 ? row[scoreIx] : "-");
    char *strand = (strandIx >= 0 ? row[strandIx] : "0");
    dasOutBed(chrom, sqlUnsigned(chromStart), sqlUnsigned(chromEnd), name, score, strand, td, tt);
    }
}

static void writeSegmentFeaturesTable(struct segment *segment,
                                      struct tableDef *td,
                                      struct hash *trackHash,
                                      struct sqlConnection *conn)
/* write segments features for a table */
{
int rowOffset;
boolean hasBin;
char table[HDB_MAX_TABLE_STRING];

verbose(2, "track %s\n", td->name);
if (!hFindSplitTable(database, segment->seq, td->name, table, sizeof table, &hasBin))
    errAbort("track %s not found", td->name);
// horrible hack because hFindSplitTable and hRangeQuery don't really seem to
// handle est->all_est/mrna->all_mrna right, but markd fears modifying that
// code
if (sameString(td->name, "mrna"))
    safecpy(table, sizeof(table), "all_mrna");
else if (sameString(td->name, "est"))
    safecpy(table, sizeof(table), "all_est");
struct trackTable *tt = hashFindVal(trackHash, td->name);
struct sqlResult *sr = hRangeQuery(conn, table, segment->seq, segment->start, segment->end, NULL, &rowOffset);
// FIXME: should use trackDb to determine type, as field names are
// not always unique.
if (sameString(td->startField, "tStart") && (sqlFieldColumn(sr, "qStart") >= 0))
    {
    dasOutPslSegment(sr, rowOffset, td, tt);
    }
else if (sameString(td->startField, "txStart"))
    {
    dasOutGpSegment(sr, rowOffset, td, tt);
    }
else if (sameString(td->startField, "chromStart"))
    {
    dasOutBedSegment(sr, table, td, tt);
    }
sqlFreeResult(&sr);
}

static void writeSegmentFeatures(struct segment *segment,
                                 struct filters *filters,
                                 struct tableDef *tdList,
                                 struct hash *trackHash,
                                 struct sqlConnection *conn)
/* write features for a segment */
{
/* Print segment header. */
printf(
"<SEGMENT id=\"%s\" start=\"%d\" stop=\"%d\" version=\"%s\" label=\"%s\">\n",
       segment->seqName, segment->start+1, segment->end, version, segment->seqName);

/* Query database and output features. */
struct tableDef *td;
for (td = tdList; td != NULL; td = td->next)
    {
    if (trackFilter(filters, td))
        writeSegmentFeaturesTable(segment, td, trackHash, conn);
    }
printf("</SEGMENT>\n");
}

static void doFeatures()
/* features - DAS Annotation Feature Server. */
{
struct segment *segmentList = dasSegmentList(TRUE), *segment;
struct hash *trackHash = hashOfTracks();
struct tableDef *tdList = getTables();
struct filters *filters = filtersNew();
struct sqlConnection *conn = hAllocConn(database);

/* Write out DAS features header. */
normalHeader();
printf(
"<!DOCTYPE DASGFF SYSTEM \"http://www.biodas.org/dtd/dasgff.dtd\">\n"
"<DASGFF>\n"
"<GFF version=\"1.0\" href=\"%s\">\n", currentUrl());

for (segment = segmentList; segment != NULL; segment = segment->next)
    writeSegmentFeatures(segment, filters, tdList, trackHash, conn);

/* Write out DAS footer. */
printf("</GFF></DASGFF>\n");

/* Clean up. */
freeHash(&trackHash);
hFreeConn(&conn);
}


static void doEntryPoints()
/* Handle entry points request. */
{
struct sqlConnection *conn;
struct sqlResult *sr;
char **row;
struct chromInfo *ci;

normalHeader();
conn = hAllocConn(database);
printf("<!DOCTYPE DASEP SYSTEM \"http://www.biodas.org/dtd/dasep.dtd\">\n");
printf("<DASEP>\n");
printf("<ENTRY_POINTS href=\"%s\" version=\"7.00\">\n",
	currentUrl());

char query[1024];
sqlSafef(query, sizeof query, "select * from chromInfo");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    ci = chromInfoLoad(row);
    /* "chr"-less chromosome ID for clients such as Ensembl: */
    if (startsWith("chr", ci->chrom))
	printf(" <SEGMENT id=\"%s\" start=\"%d\" stop=\"%d\" orientation=\"+\" subparts=\"no\">%s</SEGMENT>\n", ci->chrom+3, 1, ci->size, ci->chrom+3);
    else
	printf(" <SEGMENT id=\"%s\" start=\"%d\" stop=\"%d\" orientation=\"+\" subparts=\"no\">%s</SEGMENT>\n", ci->chrom, 1, ci->size, ci->chrom);
    chromInfoFree(&ci);
    }

printf("</ENTRY_POINTS>\n");
printf("</DASEP>\n");
}

static void doDna()
/* Handle request for DNA. */
{
struct segment *segmentList = dasSegmentList(TRUE), *segment;
int size = 0;
struct dnaSeq *seq;
int i, oneSize, lineSize = 50;

/* Write header. */
normalHeader();
printf("<!DOCTYPE DASDNA SYSTEM \"http://www.biodas.org/dtd/dasdna.dtd\">\n");
printf("<DASDNA>\n");

/* Write each sequence. */
for (segment = segmentList; segment != NULL; segment = segment->next)
    {
    printf("<SEQUENCE id=\"%s\" start=\"%d\" stop=\"%d\" version=\"%s\">\n",
	    segment->seqName, segment->start+1, segment->end, version);
    printf("<DNA length=\"%d\">\n", segment->end - segment->start);

    /* Write out DNA. */
    seq = hDnaFromSeq(database, segment->seq, segment->start, segment->end, dnaLower);
    if (seq == NULL)
        errAbort("Couldn't fetch %s\n", segment->seq);
    size = seq->size;
    for (i=0; i<size; i += oneSize)
	{
	oneSize = size - i;
	if (oneSize > lineSize) oneSize = lineSize;
	mustWrite(stdout, seq->dna+i, oneSize);
	fputc('\n', stdout);
	}
    printf("</DNA>\n");
    printf("</SEQUENCE>\n");
    }

/* Write footer. */
printf("</DASDNA>\n");
}

static void dispatch(char *dataSource, char *command)
/* Dispatch a dase command. */
{
struct slName *dbList = hDbList();
if (sameString(dataSource, "dsn"))
    {
    doDsn(dbList);
    }
else if (slNameFind(dbList, dataSource) != NULL && hDbIsActive(dataSource))
    {
    database = dataSource;
    if (sameString(command, "entry_points"))
        doEntryPoints();
    else if (sameString(command, "dna"))
        doDna();
    else if (sameString(command, "types"))
        doTypes();
    else if (sameString(command, "features"))
        doFeatures();
    else
        earlyError(DAS_UNIMPLEMENTED_FEATURE);
    }
else 
    earlyError(DAS_BAD_DATA_SOURCE);
}

static void das(char *pathInfo)
/* das - Das Server. */
{
static char *parts[3];
int partCount;
if (pathInfo == NULL)
    dasAbout();
pathInfo = cloneString(pathInfo);
partCount = chopString(pathInfo, "/", parts, ArraySize(parts));
if (partCount < 1)
    dasAbout();
if (partCount == 1 && !sameString(parts[0], "dsn"))
    {
    dasHelp("Expecting dsn or database/command in the URL after das.\n"
            "Try das/dsn for a list of databases.\n"
	    "Try das/database/types for a list of available annotations.\n");
    }
dispatch(parts[0], parts[1]);
}

int main(int argc, char *argv[])
/* Process command line. */
{
char *path = getenv("PATH_INFO");

cgiSpoof(&argc, argv);
if (cgiVarExists("verbose"))
    verboseSetLevel(cgiInt("verbose"));
if (argc == 2)
    path = argv[1];

int delay = hgBotDelayTimeFrac(0.03);
if (delay > 2000)
    {
    char *hogHost = getenv("REMOTE_ADDR");
    sendHogMessage(hogHost);
    }
sleep1000(delay);
das(path);
return 0;
}
