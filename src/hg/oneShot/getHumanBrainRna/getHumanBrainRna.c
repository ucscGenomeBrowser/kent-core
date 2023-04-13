/* getHumanBrainRna - Get list of human brain RNAs. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "getHumanBrainRna - Get list of human brain RNAs\n"
  "usage:\n"
  "   getHumanBrainRna db output.psl\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

struct hash *getBadLibs(struct sqlConnection *conn)
/* Get hash of IDs of libs that we know are bad
 * (Orestes, etc) */
{
struct hash *hash = hashNew(0);
char query[1024];
sqlSafef(query, sizeof query, 
    "select id from library where "
    "name like 'Athersys RAGE%%' or name like '%%ORESTES%%'");
struct sqlResult *sr = sqlGetResult(conn, query);
char **row;
while ((row = sqlNextRow(sr)) != NULL)
    {
    hashAdd(hash, row[0], NULL);
    }
sqlFreeResult(&sr);
return hash;
}

struct hash *getGoodTissues(struct sqlConnection *conn)
/* Get hash of IDs of good tissues. */
{
struct hash *hash = hashNew(0);
char query[1024];
sqlSafef(query, sizeof query, 
    "select id from tissue where "
    "name like '%%brain%%' or name like '%%amygdala%%' or "
    "name like '%%hippocampus%%' or name like '%%cortex%%' or "
    "name like '%%straitum%%'");
struct sqlResult *sr = sqlGetResult(conn, query);
char **row;
while ((row = sqlNextRow(sr)) != NULL)
    {
    hashAdd(hash, row[0], NULL);
    }
sqlFreeResult(&sr);
return hash;
}

struct hash *getGoodAccs(struct sqlConnection *conn, 
	struct hash *goodTissueHash, struct hash *badLibHash,
	int orgId)
/* Get accessions of mRNA and ESTs that are of given organism,
 * given tissues, and not of given libraries. */
{
char query[256];
sqlSafef(query, sizeof(query), 
   "select acc,library,tissue from gbCdnaInfo where organism = %d ",
   orgId);
struct sqlResult *sr = sqlGetResult(conn, query);
char **row;
struct hash *hash = hashNew(22);
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *acc = row[0];
    char *library = row[1];
    char *tissue = row[2];
    if (hashLookup(goodTissueHash, tissue) && !hashLookup(badLibHash, library))
	{
        hashAdd(hash, acc, NULL);
	}
    }
sqlFreeResult(&sr);
return hash;
}

void saveRow(char **row, int rowSize, FILE *f)
/* Save row to tab-separated file. */
{
int i;
int end = rowSize-1;
for (i=0; i<end; ++i)
    fprintf(f, "%s\t", row[i]);
fprintf(f, "%s\n", row[end]);
}

void writeMatchingPsl(struct sqlConnection *conn, char *table,
	struct hash *qHash, FILE *f)
/* Write all psls in table where qName is in qHash to f */
{
char query[256];
sqlSafef(query, sizeof(query), "select * from %s", table);
struct sqlResult *sr = sqlGetResult(conn, query);
char **row;
int count = 0;
while ((row = sqlNextRow(sr)) != NULL)
    {
    char *qName = row[10];
    if (hashLookup(qHash, qName))
        {
	saveRow(row+1, 21, f);
	++count;
	}
    }
sqlFreeResult(&sr);
verbose(1, "wrote %d %s\n", count, table);
}

void savePsl(struct sqlConnection *conn, struct hash *accHash, char *fileName)
/* Save EST and mRNAs aligments that are in hash table to file */
{
FILE *f = mustOpen(fileName, "w");
struct slName *table, *splicedEstList;
char query[1024];
sqlSafef(query, sizeof query, "show tables like 'chr%%_intronEst'");
splicedEstList = sqlQuickList(conn, query);
writeMatchingPsl(conn, "all_mrna", accHash, f);
for (table = splicedEstList; table != NULL; table = table->next)
    writeMatchingPsl(conn, table->name, accHash, f);
// writeMatchingPsl(conn, "all_est", accHash, f);

carefulClose(&f);
}

void getHumanBrainRna(char *db, char *outFile)
/* getHumanBrainRna - Get list of human brain RNAs. */
{
struct sqlConnection *conn = sqlConnect(db);
struct hash *badLibHash = getBadLibs(conn);
verbose(1, "Got %d bad libraries\n", badLibHash->elCount);
struct hash *goodTissueHash = getGoodTissues(conn);
verbose(1, "Got %d good tissues\n", goodTissueHash->elCount);
char query[1024];
sqlSafef(query, sizeof query, 
	"select id from organism where name = 'Homo sapiens'");
int humanId = sqlQuickNum(conn, query);
struct hash *goodAccHash = getGoodAccs(conn, goodTissueHash, 
	badLibHash, humanId);
verbose(1, "Got %d good accessions\n", goodAccHash->elCount);
savePsl(conn, goodAccHash, outFile);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
getHumanBrainRna(argv[1], argv[2]);
return 0;
}
