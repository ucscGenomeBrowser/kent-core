/* col gisaidSubjInfo - columns having to do with subjInfo and closely related tables. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "dystring.h"
#include "jksql.h"

#include "gisaidTable.h"


struct subjInfo *readAllSubjInfo(struct sqlConnection *conn, struct column *columns)
/* Get all main table gisaidSubjInfo columns in use. */
{
struct sqlResult *sr;
char **row;
struct subjInfo *siList = NULL, *si;
struct column *column = NULL;
int colCount = 0;
struct dyString *query = dyStringNew(256);
boolean firstTime = TRUE;
//if (!sameString(columns->name,"subjId"))
    //errAbort("subjId must be the first column in columnDb.ra");
if (!sameString(columns->name,"EPI_ISOLATE_ID"))
    {
    errAbort("EPI_ISOLATE_ID must be the first column in columnDb.ra %s", columns->name);
    }
sqlDyStringPrintf(query,"select ");
for (column=columns;column;column=column->next)
    {
    // skip non-main table columns which have "query" setting
    // automatically add first column (subjId) whether it's on or not.
    if ((column->on || column->filterOn || column==columns) && (!column->query))  
	{
	if (firstTime)
	    firstTime = FALSE;
	else
	    sqlDyStringPrintf(query,",");
	sqlDyStringPrintf(query,"%s\n", column->name);
	column->colNo = colCount++;
	}
    }
sqlDyStringPrintf(query," from gisaidSubjInfo\n");

sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    int i=0;
    AllocVar(si);
    si->fields = needMem(sizeof(char *)*colCount);
    for(i=0;i<colCount;++i)
	si->fields[i] = cloneString(row[i]);
    slAddHead(&siList, si);
    }
sqlFreeResult(&sr);
dyStringFree(&query);
return siList;
}





