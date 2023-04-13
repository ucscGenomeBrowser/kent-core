/* columnInfo.c was originally generated by the autoSql program, which also 
 * generated columnInfo.h and columnInfo.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "columnInfo.h"


void columnInfoStaticLoad(char **row, struct columnInfo *ret)
/* Load a row from columnInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->type = row[1];
ret->nullAllowed = row[2];
ret->key = row[3];
ret->defaultVal = row[4];
ret->extra = row[5];
}

struct columnInfo *columnInfoLoad(char **row)
/* Load a columnInfo from row fetched with select * from columnInfo
 * from database.  Dispose of this with columnInfoFree(). */
{
struct columnInfo *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->type = cloneString(row[1]);
ret->nullAllowed = cloneString(row[2]);
ret->key = cloneString(row[3]);
ret->defaultVal = cloneString(row[4]);
ret->extra = cloneString(row[5]);
return ret;
}

struct columnInfo *columnInfoLoadAll(char *fileName) 
/* Load all columnInfo from a tab-separated file.
 * Dispose of this with columnInfoFreeList(). */
{
struct columnInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = columnInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct columnInfo *columnInfoLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all columnInfo from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with columnInfoFreeList(). */
{
struct columnInfo *list = NULL, *el;
struct sqlResult *sr;
char **row;
int i = 0;
int colCount = 0; 
sr = sqlGetResult(conn, query);
colCount = sqlCountColumns(sr);

while ((row = sqlNextRow(sr)) != NULL)
    {
    /* The colCount check makes sure that we are compatible
       with mysql 4.x where they have added a collation field. Would 
       like to write a query that specifies the colums. Really should
       write the query to be column specific but the describe and show columns
       commands appear to be all there is. */
    if(colCount > 6 && stringIn("DESCRIBE", query))
	for(i=2; i<colCount-1; i++)
	    row[i] = row[i+1];
    el = columnInfoLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void columnInfoSaveToDb(struct sqlConnection *conn, struct columnInfo *el, char *tableName, int updateSize)
/* Save columnInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */
{
struct dyString *update = dyStringNew(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( '%s','%s','%s','%s','%s','%s')", 
	tableName,  el->name,  el->type,  el->nullAllowed,  el->key,  el->defaultVal,  el->extra);
sqlUpdate(conn, update->string);
dyStringFree(&update);
}

struct columnInfo *columnInfoCommaIn(char **pS, struct columnInfo *ret)
/* Create a columnInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new columnInfo */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->type = sqlStringComma(&s);
ret->nullAllowed = sqlStringComma(&s);
ret->key = sqlStringComma(&s);
ret->defaultVal = sqlStringComma(&s);
ret->extra = sqlStringComma(&s);
*pS = s;
return ret;
}

void columnInfoFree(struct columnInfo **pEl)
/* Free a single dynamically allocated columnInfo such as created
 * with columnInfoLoad(). */
{
struct columnInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->type);
freeMem(el->nullAllowed);
freeMem(el->key);
freeMem(el->defaultVal);
freeMem(el->extra);
freez(pEl);
}

void columnInfoFreeList(struct columnInfo **pList)
/* Free a list of dynamically allocated columnInfo's */
{
struct columnInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    columnInfoFree(&el);
    }
*pList = NULL;
}

void columnInfoOutput(struct columnInfo *el, FILE *f, char sep, char lastSep) 
/* Print out columnInfo.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->nullAllowed);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->key);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->defaultVal);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->extra);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

