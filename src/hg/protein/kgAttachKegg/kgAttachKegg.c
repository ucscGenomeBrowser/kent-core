/* kgAttachKegg - Attach UCSC genes to KEGG pathways via locusLink IDs. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "obscure.h"
#include "jksql.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "kgAttachKegg - Attach UCSC genes to KEGG pathways via locusLink IDs\n"
  "usage:\n"
  "   kgAttachKegg database locusLinkToPathway.tab knownToKegg.tab\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

void kgAttachKegg(char *database, char *locusLinkToPathway, char *knownToKegg)
/* kgAttachKegg - Attach UCSC genes to KEGG pathways via locusLink IDs. */
{
/* Build up hash keyed by locus link ID with KEGG pathway id's as value. */
struct hash *llToKegg = hashTwoColumnFile(locusLinkToPathway);
verbose(1, "Got %d items in %s\n", llToKegg->elCount, locusLinkToPathway);

/* Build up hash keyed by refSeq accession (without version) with UCSC known gene values. */
struct sqlConnection *conn = sqlConnect(database);
struct hash *ucscToRef = hashNew(16);
char query[1024];
sqlSafef(query, sizeof query, "select * from knownToRefSeq");
struct sqlResult *sr = sqlGetResult(conn, query);
char **row;
while ((row = sqlNextRow(sr)) != NULL)
    hashAdd(ucscToRef, row[0], cloneString(row[1]));
verbose(1, "Got %d items in %s.knownToRefSeq\n", ucscToRef->elCount, database);
sqlFreeResult(&sr);

/* Build up hash keyed by refSeq accessions with locus link values. */
struct hash *refToLl = hashNew(16);
sqlSafef(query, sizeof query, "select mrnaAcc,locusLinkId from refLink");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    hashAdd(refToLl, row[0], cloneString(row[1]));
sqlFreeResult(&sr);
verbose(1, "Got %d items in %s.refLink\n", refToLl->elCount, database);

/* Stream through kgTxInfo table getting ones that are _primarily_ refSeq. */
sqlSafef(query, sizeof query, "select name from kgTxInfo where isRefSeq=1");
sr = sqlGetResult(conn, query);
FILE *f = mustOpen(knownToKegg, "w");
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *ucsc = row[0];
    char *refSeq = hashFindVal(ucscToRef, ucsc);
    if (refSeq)
        {
	char *ll = hashFindVal(refToLl, refSeq);
	if (ll)
	    {
	    char *kegg = hashFindVal(llToKegg, ll);
	    if (kegg)
		fprintf(f, "%s\t%s\t%s\n", ucsc, ll, kegg);
	    }
	}
    }
sqlFreeResult(&sr);
carefulClose(&f);
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 4)
    usage();
kgAttachKegg(argv[1], argv[2], argv[3]);
return 0;
}
