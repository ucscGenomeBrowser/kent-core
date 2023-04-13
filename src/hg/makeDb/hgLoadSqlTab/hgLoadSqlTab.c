/* hgLoadSqlTab - Load table into database from SQL and text files. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "dystring.h"
#include "jksql.h"
#include "options.h"
#include "obscure.h"
#include "hgRelate.h"
#include "hdb.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
"hgLoadSqlTab - Load table into database from SQL and text files.\n"
"usage:\n"
"   hgLoadSqlTab database table file.sql file(s).tab\n"
"file.sql contains a SQL create statement for table\n"
"file.tab contains tab-separated text (rows of table)\n"
"The actual table name will come from the command line, not the sql file.\n"
"options:\n"
"  -warn - warn instead of abort on mysql errors or warnings\n"
"  -notOnServer - file is *not* in a directory that the mysql server can see\n"
"  -oldTable|-append - add to existing table\n"
"\n"
"To load bed 3+ sorted tab files as hgLoadBed would do automatically\n"
"sort the input file:\n"
"  sort -k1,1 -k2,2n file(s).tab | hgLoadSqlTab database table file.sql stdin\n"
  );
}

static struct optionSpec options[] = {
    {"warn", OPTION_BOOLEAN},
    {"notOnServer", OPTION_BOOLEAN},
    {"oldTable", OPTION_BOOLEAN},
    {"append", OPTION_BOOLEAN},
    {NULL, 0},
};

struct dyString *readAndReplaceTableName(char *fileName, char *table)
/* Read file into string.  While doing so strip any leading comments
 * and insist that the first non-comment line contain the words
 * "create table" followed by a table name.  Replace the table name,
 * and copy the rest of the file verbatem. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
struct dyString *dy = dyStringNew(0);
char *line, *word;
if (!lineFileNextReal(lf, &line))
    errAbort("No real lines in %s\n", fileName);
word = nextWord(&line);
if (!sameWord(word, "create"))
    errAbort("Expecting first word in file to be CREATE. Got %s", word);
word = nextWord(&line);
if (word == NULL || !sameWord(word, "table"))
    errAbort("Expecting second word in file to be table. Got %s", emptyForNull(word));
word = nextWord(&line);
if (word == NULL)
    errAbort("Expecting table name on same line as CREATE TABLE");
dyStringPrintf(dy, "CREATE TABLE %s ", table);   // trust .sql file off disk.
if (line != NULL)
    dyStringAppend(dy, line);
dyStringAppendC(dy, '\n');
while (lineFileNext(lf, &line, NULL))
    {
    dyStringAppend(dy, line);
    dyStringAppendC(dy, '\n');
    }
lineFileClose(&lf);
return dy;
}

void hgLoadSqlTab(char *database, char *table, char *createFile,
		  int inCount, char *inNames[])
/* hgLoadSqlTab - Load table into database from SQL and text files. */
{
struct sqlConnection *conn = sqlConnect(database);

int loadOptions = 0;
int i=0;
boolean oldTable = optionExists("oldTable") || optionExists("append");
if (optionExists("warn"))
    loadOptions |= SQL_TAB_FILE_WARN_ON_ERROR;

if (! optionExists("notOnServer"))
    loadOptions |= SQL_TAB_FILE_ON_SERVER;

if (! oldTable)
    {
    sqlCkId(table);   
    struct dyString *dy = readAndReplaceTableName(createFile, table);
    // trust sql loaded from disk file
    struct dyString *dyTrust = dyStringNew(1024);
    sqlDyStringPrintf(dyTrust, dy->string, NULL);
    sqlRemakeTable(conn, table, dyTrust->string);
    dyStringFree(&dy);
    dyStringFree(&dyTrust);
    }
verbose(1, "Scanning through %d files\n", inCount);
for (i=0;  i < inCount;  i++)
    {
    verbose(2, "Loading file %s into table %s\n", inNames[i], table);
    if (sameString("stdin", inNames[i]))
	sqlLoadTabFile(conn, "/dev/stdin", table,
		       (loadOptions & ~SQL_TAB_FILE_ON_SERVER));
    else
	sqlLoadTabFile(conn, inNames[i], table, loadOptions);
    }
if (oldTable)
    hgHistoryComment(conn, "Add contents of %d text file(s) to table %s.",
		     inCount, table);
else
    hgHistoryComment(conn, "Load table %s directly from .sql and %d text file(s).",
		     table, inCount);
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc < 5)
    usage();
hgLoadSqlTab(argv[1], argv[2], argv[3], argc-4, argv+4);
return 0;
}
