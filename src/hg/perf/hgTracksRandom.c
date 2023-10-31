/* hgTracksRandom - For a given organism, view hgTracks (default tracks) in random position.
                    Record display time. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "chromInfo.h"
#include "dystring.h"
#include "hash.h"
#include "hdb.h"
#include "htmlPage.h"
#include "linefile.h"
#include "jksql.h"
#include "options.h"
#include "portable.h"
#include <signal.h>


static char *database = NULL;
static boolean quiet = FALSE;
static struct hash *chromHash = NULL;

/* command line option specifications */
static struct optionSpec optionSpecs[] = {
    {"machines", OPTION_STRING},
    {"quiet", OPTION_BOOLEAN},
    {NULL, 0}
};

struct machine 
    {
    struct machine *next;
    char *name;
    };

struct machine *machineList;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgTracksRandom - Time default view for random position of default genome\n"
  "usage:\n"
  "  hgTracksRandom <machines> [options]\n"
  "options:\n"
  "   <machines> is a file listing the machines to test\n"
  "   -quiet - only outputs the timing numbers, no machine names\n");
}

/* this is the second instance of this, it needs to find its program
 * name to become a generic function in the libs
 */
/* if this Apoptosis function becomes more popular, it can go into the libs
 * there is a similar function in hg/lib/web.c cgiApoptosis()
 */
static unsigned long expireSeconds = 0;
/* to avoid long running processes on the RR */
static void selfApoptosis(int status)
/* signal handler for SIGALRM expiration */
{
if (expireSeconds > 0)
    {
    /* want to see this error message in the apache error_log also */
    fprintf(stderr, "hgTracksRandom selfApoptosis: %lu seconds\n", expireSeconds);
    /* this message may show up somewhere */
    errAbort("procedures have exceeded timeout: %lu seconds, function has ended.\n", expireSeconds);
    }
exit(0);
}

void getMachines(char *filename)
/* Read in list of machines to use. */
{
struct lineFile *lf = lineFileOpen(filename, TRUE);
char *line;
int lineSize;
struct machine *machine;

while (lineFileNext(lf, &line, &lineSize))
    {
    AllocVar(machine);
    // is this equivalent to slAddHead?
    machine->name = line;
    machine->next = machineList;
    machineList = machine;
    }
/* could reverse order here */
}

/* Copied from hgLoadWiggle. */
static struct hash *loadAllChromInfo()
/* Load up all chromosome infos. */
{
struct chromInfo *el;
struct sqlConnection *conn = sqlConnect(database);
struct sqlResult *sr = NULL;
struct hash *ret;
char **row;

ret = newHash(0);

char query[1024];
sqlSafef(query, sizeof query, "select * from chromInfo");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = chromInfoLoad(row);
    verbose(4, "Add hash %s value %u (%#lx)\n", el->chrom, el->size, (unsigned long)&el->size);
    hashAdd(ret, el->chrom, (void *)(& el->size));
    }
sqlFreeResult(&sr);
sqlDisconnect(&conn);
return ret;
}

/* also copied from hgLoadWiggle. */
static unsigned getChromSize(char *chrom)
/* Return size of chrom.  */
{
struct hashEl *el = hashLookup(chromHash,chrom);

if (el == NULL)
    errAbort("Couldn't find size of chrom %s", chrom);
return *(unsigned *)el->val;
}   



long hgTracksRandom(char *url)
/* hgTracksRandom - Time default view for random position. */
/* The URL can include position. */
{
long startTime, endTime;
struct htmlPage *rootPage;

startTime = clock1000();
rootPage = htmlPageGet(url);
endTime = clock1000();

htmlPageValidateOrAbort(rootPage);
if (rootPage->status->status != 200)
   errAbort("%s returned with status code %d", url, rootPage->status->status);

return endTime - startTime;
}

int getStartPos(int chromSize, int windowSize)
/* return a random start position */
{
if (windowSize >= chromSize)
    return 0;
return rand() % (chromSize - windowSize);
}

int main(int argc, char *argv[])
{
struct dyString *dy = NULL;
int startPos = 1;
char *chrom = "chr1";
int chromSize = 0;
int windowSize = 100000;
struct machine *machinePos;
time_t now;
char testTime[100];
char testDate[100];
long elapsedTime = 0;

optionInit(&argc, argv, optionSpecs);

expireSeconds = 300;	/* 5 minutes */
(void) signal(SIGALRM, selfApoptosis);
(void) alarm(expireSeconds);	/* CGI timeout */

quiet = optionExists("quiet");
now = time(NULL);
strftime(testTime, 100, "%H:%M", localtime(&now));
strftime(testDate, 100, "%B %d, %Y", localtime(&now));
if (!quiet)
    printf("%s %s\n", testDate, testTime);

if (argc != 2)
    usage();

srand( (unsigned) time(NULL) );

database = hDefaultDbForGenome(NULL); // default human db

chromHash = loadAllChromInfo();
chromSize = getChromSize(chrom);
startPos = getStartPos(chromSize, windowSize);
if (! quiet)
    printf("%s %s:%d-%d\n\n", database, chrom, startPos, startPos + windowSize);

getMachines(argv[1]);

for (machinePos = machineList; machinePos != NULL; machinePos = machinePos->next)
    {
    dy = dyStringNew(256);
    dyStringPrintf(dy, "%s/cgi-bin/hgTracks?db=%s&position=%s:%d-%d", machinePos->name, 
                   database, chrom, startPos, startPos + windowSize);
    elapsedTime = hgTracksRandom(dy->string);
    if (quiet)
	{
	printf("%ld\n", elapsedTime);
	}
    else
	{
	if (elapsedTime > 10000 || elapsedTime < 100)
	    printf("%s %ld <---\n", machinePos->name, elapsedTime);
	else
	    printf("%s %ld\n", machinePos->name, elapsedTime);
	}
    }
if (! quiet)
    printf("----------------------------------\n");

/* free machine list */
return 0;
}
