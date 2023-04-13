/* hubPublic.c was originally generated by the autoSql program, which also 
 * generated hubPublic.h and hubPublic.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hubPublic.h"


char *hubPublicCommaSepFieldNames = "hubUrl,shortLabel,longLabel,registrationTime,dbCount,dbList,descriptionUrl";

void hubPublicStaticLoad(char **row, struct hubPublic *ret)
/* Load a row from hubPublic table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->hubUrl = row[0];
ret->shortLabel = row[1];
ret->longLabel = row[2];
ret->registrationTime = row[3];
ret->dbCount = sqlUnsigned(row[4]);
ret->dbList = row[5];
ret->descriptionUrl = row[6];
}

struct hubPublic *hubPublicLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all hubPublic from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hubPublicFreeList(). */
{
struct hubPublic *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = hubPublicLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void hubPublicSaveToDb(struct sqlConnection *conn, struct hubPublic *el, char *tableName, int updateSize)
/* Save hubPublic as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. This function automatically escapes quoted strings for mysql. */
{
struct dyString *update = dyStringNew(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( '%s','%s','%s','%s',%u,'%s','%s')", 
	tableName,  el->hubUrl,  el->shortLabel,  el->longLabel,  el->registrationTime,  el->dbCount,  el->dbList,  el->descriptionUrl);
sqlUpdate(conn, update->string);
dyStringFree(&update);
}

struct hubPublic *hubPublicLoad(char **row)
/* Load a hubPublic from row fetched with select * from hubPublic
 * from database.  Dispose of this with hubPublicFree(). */
{
struct hubPublic *ret;

AllocVar(ret);
ret->hubUrl = cloneString(row[0]);
ret->shortLabel = cloneString(row[1]);
ret->longLabel = cloneString(row[2]);
ret->registrationTime = cloneString(row[3]);
ret->dbCount = sqlUnsigned(row[4]);
ret->dbList = cloneString(row[5]);
ret->descriptionUrl = cloneString(row[6]);
return ret;
}

struct hubPublic *hubPublicLoadAll(char *fileName) 
/* Load all hubPublic from a whitespace-separated file.
 * Dispose of this with hubPublicFreeList(). */
{
struct hubPublic *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = hubPublicLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hubPublic *hubPublicLoadAllByChar(char *fileName, char chopper) 
/* Load all hubPublic from a chopper separated file.
 * Dispose of this with hubPublicFreeList(). */
{
struct hubPublic *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = hubPublicLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hubPublic *hubPublicCommaIn(char **pS, struct hubPublic *ret)
/* Create a hubPublic out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hubPublic */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->hubUrl = sqlStringComma(&s);
ret->shortLabel = sqlStringComma(&s);
ret->longLabel = sqlStringComma(&s);
ret->registrationTime = sqlStringComma(&s);
ret->dbCount = sqlUnsignedComma(&s);
ret->dbList = sqlStringComma(&s);
ret->descriptionUrl = sqlStringComma(&s);
*pS = s;
return ret;
}

void hubPublicFree(struct hubPublic **pEl)
/* Free a single dynamically allocated hubPublic such as created
 * with hubPublicLoad(). */
{
struct hubPublic *el;

if ((el = *pEl) == NULL) return;
freeMem(el->hubUrl);
freeMem(el->shortLabel);
freeMem(el->longLabel);
freeMem(el->registrationTime);
freeMem(el->dbList);
freeMem(el->descriptionUrl);
freez(pEl);
}

void hubPublicFreeList(struct hubPublic **pList)
/* Free a list of dynamically allocated hubPublic's */
{
struct hubPublic *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    hubPublicFree(&el);
    }
*pList = NULL;
}

void hubPublicOutput(struct hubPublic *el, FILE *f, char sep, char lastSep) 
/* Print out hubPublic.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hubUrl);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shortLabel);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->longLabel);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->registrationTime);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->dbCount);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->dbList);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->descriptionUrl);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

void hubPublicJsonOutput(struct hubPublic *el, FILE *f) 
/* Print out hubPublic in JSON format. */
{
fputc('{',f);
fputc('"',f);
fprintf(f,"hubUrl");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->hubUrl);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"shortLabel");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->shortLabel);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"longLabel");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->longLabel);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"registrationTime");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->registrationTime);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"dbCount");
fputc('"',f);
fputc(':',f);
fprintf(f, "%u", el->dbCount);
fputc(',',f);
fputc('"',f);
fprintf(f,"dbList");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->dbList);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"descriptionUrl");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->descriptionUrl);
fputc('"',f);
fputc('}',f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


struct hubEntry *hubEntryTextLoad(char **row, bool hasDescription)
{
struct hubEntry *ret;
AllocVar(ret);
ret->hubUrl = cloneString(row[0]);
ret->shortLabel = cloneString(row[1]);
ret->longLabel = cloneString(row[2]);
ret->dbList = cloneString(row[3]);
ret->errorMessage = cloneString(row[4]);
ret->id = sqlUnsigned(row[5]);
if (hasDescription)
    ret->descriptionUrl = cloneString(row[6]);
else
    ret->descriptionUrl = NULL;
return ret;
}
