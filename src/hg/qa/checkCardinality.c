/* checkCardinality - check for NULL cardinality in indexes. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "hdb.h"

char *database = NULL;

struct table
{
    struct table *next;
    int rowCount;
    char *name;
};
		
struct table *tableList = NULL;

int tableCmp(const void *va, const void *vb)
{
const struct table *a = *((struct table **)va);
const struct table *b = *((struct table **)vb);
int dif;
dif = a->rowCount - b->rowCount;
return dif;
}

void usage()
/* Explain usage and exit. */
{
errAbort(
    "reviewIndexes - check indexes\n"
    "usage:\n"
    "    reviewIndexes database \n");
}

struct table *getTables()
/* Get results from 'show tables' */
{
char query[512];
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr;
char **row;
int count = 0;
struct table *table, *list = NULL;

verbose(2, "show tables...\n");
sqlSafef(query, sizeof(query), "show tables");
sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    count++;
    // short-circuit
    // if (count == 100) return list;
    AllocVar(table);
    table->name = cloneString(row[0]);
    table->next = list;
    list = table;
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
slReverse(&list);  /* could possibly skip if it made much difference in speed. */
verbose(1, "%d tables found\n", count);
return list;
}

void addRowcount()
/* get the size of each table */
{
struct table *table1 = NULL;
struct sqlConnection *conn = hAllocConn(database);

for (table1 = tableList; table1 != NULL; table1 = table1->next)
    {
    table1->rowCount = sqlTableSize(conn, table1->name);
    }
verbose(1, "done with rowCount lookup\n");
}

void reviewIndexes()
/* reviewIndexes - look at index for each table. */
{
struct table *table1 = NULL;
char query[512];
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr;
char **row;

verbose(2, "checking....\n");

for (table1 = tableList; table1 != NULL; table1 = table1->next)
    {
    /* check for bin index */
    sqlSafef(query, sizeof(query), "show index from %s", table1->name);
    sr = sqlGetResult(conn, query);
    while ((row = sqlNextRow(sr)) != NULL)
        {  
	if (row[6] == NULL)
             printf("%s %s\n", row[2], table1->name);
	} 
    sqlFreeResult(&sr);
    }

// freeList(&tableList);

}


int main(int argc, char *argv[])
/* Check args and call reviewIndexes. */
{
if (argc != 2)
    usage();
database = argv[1];
tableList = getTables();
addRowcount();
slSort(&tableList, tableCmp);
reviewIndexes();
return 0;
}
