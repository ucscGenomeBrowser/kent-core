/* hgGeneBands - Find bands for all genes. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "options.h"
#include "jksql.h"
#include "hdb.h"
#include "refLink.h"
#include "genePred.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "hgGeneBands - Find bands for all genes\n"
  "usage:\n"
  "   hgGeneBands database outFile.tab\n"
  );
}

void printBands(char *database, struct refLink *rl, FILE *f)
/* Print name of genes and bands it occurs on. */
{
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr;
char **row;
struct genePred *gp;
char query[512];
int count = 0;
struct dyString *bands = dyStringNew(0);
char band[64];

sqlSafef(query, sizeof query, "select * from refGene where name = '%s'", rl->mrnaAcc);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    ++count;
    gp = genePredLoad(row);
    if (hChromBand(database, gp->chrom, (gp->txStart + gp->txEnd)/2, band))
        dyStringPrintf(bands, "%s,", band);
    else
        dyStringPrintf(bands, "n/a,");
    }
if (count > 0)
    fprintf(f, "%s\t%s\t%d\t%s\n", rl->name, rl->mrnaAcc, count, bands->string);

dyStringFree(&bands);
sqlFreeResult(&sr);
hFreeConn(&conn);
}

void hgGeneBands(char *database, char *outFile)
/* hgGeneBands - Find bands for all genes. */
{
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr;
char **row;
struct refLink rl;
FILE *f = mustOpen(outFile, "w");

char query[1024];
sqlSafef(query, sizeof query, "select * from refLink");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    refLinkStaticLoad(row, &rl);
    printBands(database, &rl, f);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc != 3)
    usage();
hgGeneBands(argv[1], argv[2]);
return 0;
}
