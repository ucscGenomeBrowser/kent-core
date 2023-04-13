/* kgName - builds association table between knownPep and gene common name. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "hCommon.h"
#include "hdb.h"
#include "spDb.h"

char *database = "hg16";
void usage()
/* Explain usage and exit. */
{
errAbort(
  "kgName - builds association table between knownPep and gene common name\n"
  "usage:\n"
  "   kgName database reference.psl outAssoc \n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

char *protDbName = NULL;

char* lookupName(  struct sqlConnection *conn , char *id)
{
char query[256];
char *name = id;

if (hTableExists("kgXref"))
    {
    struct sqlResult *sr;
    char **row;

    sqlSafef(query, sizeof query, "select geneSymbol from kgXref where mRNA = '%s'", id);
    sr = sqlGetResult(conn, query);
    row = sqlNextRow(sr);
    if (row != NULL)
	{
	name = cloneString(row[0]);
	}
    sqlFreeResult(&sr);
    }
#ifdef NOTNOW
if (hTableExists("refLink") && hTableExists("knownGeneLink"))
    {
    struct sqlResult *sr;
    char **row;

    sqlSafef(cond_str, sizeof cond_str, "name='%s' and seqType='g'", id);
    seqType = sqlGetField(conn, database, "knownGeneLink", "seqType", cond_str);

    if (seqType != NULL)
	{
	// special processing for RefSeq DNA based genes
	qlSafefFrag(cond_str, sizeof cond_str, "mrnaAcc = '%s'", id);
	refSeqName = sqlGetField(conn, database, "refLink", "name", cond_str);
	if (refSeqName != NULL)
	    {
	    name=  cloneString(refSeqName);
	    }
	}
    else if (protDbName != NULL)
	{
	sqlSafef(cond_str, sizeof cond_str, "mrnaID='%s'", id);
	proteinID = sqlGetField(conn, database, "spMrna", "spID", cond_str);

	sqlSafef(cond_str, sizeof cond_str, "displayID = '%s'", proteinID);
	hugoID = sqlGetField(conn, protDbName, "spXref3", "hugoSymbol", cond_str);
	if (!((hugoID == NULL) || (*hugoID == '\0')) )
	    {
	    name= cloneString(hugoID);
	    }
	else
	    {
	    sqlSafef(query, sizeof query,"select refseq from %s.mrnaRefseq where mrna = '%s';",  
		    database, id);

	    sr = sqlGetResult(conn, query);
	    row = sqlNextRow(sr);
	    if (row != NULL)
		{
		sqlSafef(query, sizeof query, "select * from refLink where mrnaAcc = '%s'", row[0]);
		sqlFreeResult(&sr);
		sr = sqlGetResult(conn, query); 
		if ((row = sqlNextRow(sr)) != NULL)
		    {
		    if (strlen(row[0]) > 0)
			name= cloneString(row[0]);
		    }
		sqlFreeResult(&sr);
		}
	    else
		{
		sqlFreeResult(&sr);
		}
	    }
	}
    }
#endif
return name;
}

char *getSwiss( struct sqlConnection *conn , char *id)
{
char query[256];
char *name = NULL;
if (hTableExists("kgXref"))
    {
    struct sqlResult *sr;
    char **row;

    sqlSafef(query, sizeof query, "select spID from kgXref where mRNA = '%s'", id);
    sr = sqlGetResult(conn, query);
    row = sqlNextRow(sr);
    if (row != NULL)
	{
	name = cloneString(row[0]);
	sqlFreeResult(&sr);
	return name;
	}
	sqlFreeResult(&sr);
    }
errAbort("no kgXref");
}

void kgName(char *database, char *protDb, char *refPsl,  char *outAssoc)
/* kgName - builds association table between knownPep and gene common name. */
{
struct sqlConnection *conn = sqlConnect(database);
//struct sqlConnection *conn2 = sqlConnect("uniProt");
FILE *f = mustOpen(outAssoc, "w");
struct lineFile *pslLf = pslFileOpen(refPsl);
struct psl *psl;

while ((psl = pslNext(pslLf)) != NULL)
    fprintf(f,"%s\t%s\t%s:%d-%d\t%s\n",psl->qName, lookupName(conn,psl->qName), 
	psl->tName, psl->tStart, psl->tEnd, getSwiss(conn, psl->qName));
/*
while (lineFileRow(lf, words))
    {
    fprintf(f,"%s\t%s\t%s\n",words[0], lookupName(conn,words[0]), getSwiss(conn, words[0]));
    }
    */
//hFreeConn(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 4)
    usage();
database=argv[1];
hSetDb(argv[1]);
protDbName = hPdbFromGdb(database);
kgName(argv[1], protDbName, argv[2], argv[3]);
return 0;
}
