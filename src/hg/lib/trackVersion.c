/* trackVersion.c was originally generated by the autoSql program, which also 
 * generated trackVersion.h and trackVersion.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hdb.h"
#include "trackVersion.h"
#include "trackHub.h"


void trackVersionStaticLoad(char **row, struct trackVersion *ret)
/* Load a row from trackVersion table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->ix = sqlSigned(row[0]);
ret->db = row[1];
ret->name = row[2];
ret->who = row[3];
ret->version = row[4];
ret->updateTime = row[5];
ret->comment = row[6];
ret->source = row[7];
ret->dateReference = row[8];
}

struct trackVersion *trackVersionLoad(char **row)
/* Load a trackVersion from row fetched with select * from trackVersion
 * from database.  Dispose of this with trackVersionFree(). */
{
struct trackVersion *ret;

AllocVar(ret);
ret->ix = sqlSigned(row[0]);
ret->db = cloneString(row[1]);
ret->name = cloneString(row[2]);
ret->who = cloneString(row[3]);
ret->version = cloneString(row[4]);
ret->updateTime = cloneString(row[5]);
ret->comment = cloneString(row[6]);
ret->source = cloneString(row[7]);
ret->dateReference = cloneString(row[8]);
return ret;
}

struct trackVersion *trackVersionLoadAll(char *fileName) 
/* Load all trackVersion from a whitespace-separated file.
 * Dispose of this with trackVersionFreeList(). */
{
struct trackVersion *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = trackVersionLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct trackVersion *trackVersionLoadAllByChar(char *fileName, char chopper) 
/* Load all trackVersion from a chopper separated file.
 * Dispose of this with trackVersionFreeList(). */
{
struct trackVersion *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = trackVersionLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct trackVersion *trackVersionLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all trackVersion from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with trackVersionFreeList(). */
{
struct trackVersion *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = trackVersionLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void trackVersionSaveToDb(struct sqlConnection *conn, struct trackVersion *el, char *tableName, int updateSize)
/* Save trackVersion as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */
{
struct dyString *update = dyStringNew(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( %d,'%s','%s','%s','%s','%s','%s','%s','%s')", 
	tableName,  el->ix,  el->db,  el->name,  el->who,  el->version,  el->updateTime,  el->comment,  el->source,  el->dateReference);
sqlUpdate(conn, update->string);
dyStringFree(&update);
}


struct trackVersion *trackVersionCommaIn(char **pS, struct trackVersion *ret)
/* Create a trackVersion out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new trackVersion */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->ix = sqlSignedComma(&s);
ret->db = sqlStringComma(&s);
ret->name = sqlStringComma(&s);
ret->who = sqlStringComma(&s);
ret->version = sqlStringComma(&s);
ret->updateTime = sqlStringComma(&s);
ret->comment = sqlStringComma(&s);
ret->source = sqlStringComma(&s);
ret->dateReference = sqlStringComma(&s);
*pS = s;
return ret;
}

void trackVersionFree(struct trackVersion **pEl)
/* Free a single dynamically allocated trackVersion such as created
 * with trackVersionLoad(). */
{
struct trackVersion *el;

if ((el = *pEl) == NULL) return;
freeMem(el->db);
freeMem(el->name);
freeMem(el->who);
freeMem(el->version);
freeMem(el->updateTime);
freeMem(el->comment);
freeMem(el->source);
freeMem(el->dateReference);
freez(pEl);
}

void trackVersionFreeList(struct trackVersion **pList)
/* Free a list of dynamically allocated trackVersion's */
{
struct trackVersion *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    trackVersionFree(&el);
    }
*pList = NULL;
}

void trackVersionOutput(struct trackVersion *el, FILE *f, char sep, char lastSep) 
/* Print out trackVersion.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%d", el->ix);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->db);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->who);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->version);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->updateTime);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->comment);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->source);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dateReference);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

struct trackVersion *getTrackVersion(char *database, char *track)
// Get most recent trackVersion for given track in given database
{
boolean trackVersionExists = !trackHubDatabase(database) && hTableExists("hgFixed", "trackVersion");
struct trackVersion *trackVersion = NULL;
if (trackVersionExists)
    {
    char query[256];
    struct sqlConnection *conn = hAllocConn(database);

    sqlSafef(query, sizeof(query), "select * from hgFixed.trackVersion where db = '%s' AND name = '%s' order by updateTime DESC limit 1", database, track);
    struct sqlResult *sr = sqlGetResult(conn, query);
    char **row;
    if ((row = sqlNextRow(sr)) != NULL)
        trackVersion = trackVersionLoad(row);
    sqlFreeResult(&sr);
    hFreeConn(&conn);
    }
return trackVersion;
}
