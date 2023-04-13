/* spToSpXref2- Create tab delimited data file for spXref2 table in proteinsxxxx database */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "hCommon.h"
#include "hdb.h"
#include "spDb.h"
#include "options.h"

#define MAX_EXTDB 500

void usage()
/* Explain usage and exit. */
{
errAbort(
  "spToSpXref2- Create tab delimited data files for the spXref2 table in uniProt (spxxxxxx) database.\n"
  "usage:\n"
  "   spToSpXref2 swissProt-yymmdd proteome-yymmdd\n"
  "      yymmdd is the release date of SWISS-PROT data\n"
  "Example: spToSpXref2 080707 120323\n");
}

int main(int argc, char *argv[])
{
struct sqlConnection *conn, *conn2;
char query[256], query2[256];
struct sqlResult *sr, *sr2;
char **row, **row2;
FILE *o1;
char *proteomeDataDate;
char *uniprotDataDate;
char *bioDatabase, *bioentryID;
char *displayID;
char *accession;
char *extAC;
char *extDb;
char *division;
int iii;
char *extDbName[MAX_EXTDB];
int icnt;
int id;
char *val;

optionHash(&argc, argv);
if (argc != 3) usage();

uniprotDataDate = argv[1];
proteomeDataDate = argv[2];

o1 = mustOpen("temp_spXref2.dat", "w");

conn  = hAllocConn(hDefaultDb());
conn2 = hAllocConn(hDefaultDb());

icnt =0;

/* first read in extDb table */
sqlSafef(query, sizeof(query), "select * from sp%s.extDb", uniprotDataDate);
sr = sqlMustGetResult(conn, query);
row = sqlNextRow(sr);
while (row != NULL)
    {
    icnt++;

    if (icnt >= MAX_EXTDB)
	{
        errAbort("Too many extDBs - please set MAX_EXTDB to be more than %d\n", MAX_EXTDB);
	}
    id  = atoi(row[0]);
    val = row[1];
    extDbName[id] = strdup(val);
       
    row = sqlNextRow(sr);
    }
sqlFreeResult(&sr);

iii=0;

/* Reuse the first 5 fields from the spXref3 table */
sqlSafef(query2, sizeof(query2), 
      "select accession, displayID, division, bioentryID, biodatabaseID from proteins%s.spXref3;",
      proteomeDataDate);
sr2 = sqlMustGetResult(conn2, query2);
row2 = sqlNextRow(sr2);
iii=0;
while (row2 != NULL)
    {
    accession		= row2[0];   
    displayID           = row2[1];
    division            = row2[2];
    bioentryID          = row2[3];
    bioDatabase         = row2[4];
  
    sqlSafef(query, sizeof(query),
	  "select extAcc1, extDb.id from sp%s.extDb, sp%s.extDbRef where extDbRef.acc='%s' %s",
	  uniprotDataDate, uniprotDataDate, accession, "and extDb.id = extDbRef.extDb;"); 
    sr = sqlMustGetResult(conn, query);
    row = sqlNextRow(sr);
    while (row != NULL)
    	{
    	extAC = row[0];
	extDb = extDbName[atoi(row[1])];

	printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n", accession, displayID, division,
		extDb, extAC, bioentryID, bioDatabase);
 	iii++; 
        
	if ((iii%100000) == 0)
	fprintf(stderr, "Processed %d records\n", iii);fflush(stderr);
        
	row = sqlNextRow(sr);
	}
    sqlFreeResult(&sr);
    row2 = sqlNextRow(sr2);
    }

fclose(o1);

sqlFreeResult(&sr2);
hFreeConn(&conn);
hFreeConn(&conn2);

return(0);
}
