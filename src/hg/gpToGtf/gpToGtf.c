/* gpToGtf - Convert gp table to GTF. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"
#include "hdb.h"
#include "genePred.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "gpToGtf - Convert gp table to GTF\n"
  "usage:\n"
  "   gpToGtf database table output.gtf\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

void gtfFromTable(char *database, char *table, FILE *f)
/* Convert table to GTF file. */
{
int rowOffset = hIsBinned(database, table);
char query[256];
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr;
char **row;
struct genePred *gp;
int i;

sqlSafef(query, sizeof(query), "select * from %s", table);
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    gp = genePredLoad(row+rowOffset);
    fprintf(f, "# name %s, exons %d\n", gp->name, gp->exonCount);
    for (i=0; i<gp->exonCount; ++i)
        {
	int s,e;
	s = gp->exonStarts[i];
	e = gp->exonEnds[i];
	fprintf(f, "%s\t%s\texon\t%d\t%d\t.\t%s\t.\tgene_id \"%s\"; transcript_id \"%s\"\n",
		gp->chrom, table, s+1, e, gp->strand, gp->name, gp->name);
	if (rangeIntersection(s, e, gp->cdsStart, gp->cdsEnd) > 0)
	    {
	    if (s < gp->cdsStart) s = gp->cdsStart;
	    if (e > gp->cdsEnd) e = gp->cdsEnd;
	    fprintf(f, "%s\t%s\tCDS\t%d\t%d\t.\t%s\t.\tgene_id \"%s\"; transcript_id \"%s\"\n",
		    gp->chrom, table, s+1, e, gp->strand, gp->name, gp->name);
	    }
	}
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
}

void gpToGtf(char *database, char *table, char *fileName)
/* gpToGtf - Convert gp table to GTF. */
{
FILE *f = mustOpen(fileName, "w");
time_t now = time(NULL);
if (!hTableExists(database, table))
    errAbort("%s doesn't exist in %s", table, database);
fprintf(f, "# GTF Conversion of UCSC table %s.%s, %s.\n", database, table, ctime(&now));
gtfFromTable(database, table, f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionHash(&argc, argv);
if (argc != 4)
    usage();
gpToGtf(argv[1], argv[2], argv[3]);
return 0;
}
