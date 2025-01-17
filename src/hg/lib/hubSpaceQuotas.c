/* hubSpaceQuotas.c was originally generated by the autoSql program, which also 
 * generated hubSpaceQuotas.h and hubSpaceQuotas.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hubSpaceQuotas.h"
#include "hdb.h"



char *hubSpaceQuotasCommaSepFieldNames = "userName,quota";

void hubSpaceQuotasStaticLoad(char **row, struct hubSpaceQuotas *ret)
/* Load a row from hubSpaceQuotas table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->userName = row[0];
ret->quota = sqlLongLong(row[1]);
}

struct hubSpaceQuotas *hubSpaceQuotasLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all hubSpaceQuotas from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hubSpaceQuotasFreeList(). */
{
struct hubSpaceQuotas *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = hubSpaceQuotasLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void hubSpaceQuotasSaveToDb(struct sqlConnection *conn, struct hubSpaceQuotas *el, char *tableName, int updateSize)
/* Save hubSpaceQuotas as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. This function automatically escapes quoted strings for mysql. */
{
struct dyString *update = dyStringNew(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( '%s',%lld)", 
	tableName,  el->userName,  el->quota);
sqlUpdate(conn, update->string);
dyStringFree(&update);
}

struct hubSpaceQuotas *hubSpaceQuotasLoad(char **row)
/* Load a hubSpaceQuotas from row fetched with select * from hubSpaceQuotas
 * from database.  Dispose of this with hubSpaceQuotasFree(). */
{
struct hubSpaceQuotas *ret;

AllocVar(ret);
ret->userName = cloneString(row[0]);
ret->quota = sqlLongLong(row[1]);
return ret;
}

struct hubSpaceQuotas *hubSpaceQuotasLoadAll(char *fileName) 
/* Load all hubSpaceQuotas from a whitespace-separated file.
 * Dispose of this with hubSpaceQuotasFreeList(). */
{
struct hubSpaceQuotas *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = hubSpaceQuotasLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hubSpaceQuotas *hubSpaceQuotasLoadAllByChar(char *fileName, char chopper) 
/* Load all hubSpaceQuotas from a chopper separated file.
 * Dispose of this with hubSpaceQuotasFreeList(). */
{
struct hubSpaceQuotas *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[2];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = hubSpaceQuotasLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct hubSpaceQuotas *hubSpaceQuotasCommaIn(char **pS, struct hubSpaceQuotas *ret)
/* Create a hubSpaceQuotas out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hubSpaceQuotas */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->userName = sqlStringComma(&s);
ret->quota = sqlLongLongComma(&s);
*pS = s;
return ret;
}

void hubSpaceQuotasFree(struct hubSpaceQuotas **pEl)
/* Free a single dynamically allocated hubSpaceQuotas such as created
 * with hubSpaceQuotasLoad(). */
{
struct hubSpaceQuotas *el;

if ((el = *pEl) == NULL) return;
freeMem(el->userName);
freez(pEl);
}

void hubSpaceQuotasFreeList(struct hubSpaceQuotas **pList)
/* Free a list of dynamically allocated hubSpaceQuotas's */
{
struct hubSpaceQuotas *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    hubSpaceQuotasFree(&el);
    }
*pList = NULL;
}

void hubSpaceQuotasOutput(struct hubSpaceQuotas *el, FILE *f, char sep, char lastSep) 
/* Print out hubSpaceQuotas.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->userName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%lld", el->quota);
fputc(lastSep,f);
}

void hubSpaceQuotasJsonOutput(struct hubSpaceQuotas *el, FILE *f) 
/* Print out hubSpaceQuotas in JSON format. */
{
fputc('{',f);
fputc('"',f);
fprintf(f,"userName");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->userName);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"quota");
fputc('"',f);
fputc(':',f);
fprintf(f, "%lld", el->quota);
fputc('}',f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

long long quotaForUserName(char *userName)
/* Return the quota for a given userName or 0 if not found */
{
long long ret = 0;
if (userName)
    {
    struct sqlConnection *conn = hConnectCentral();
    struct dyString *query = sqlDyStringCreate("select quota from hubSpaceQuotas where userName='%s'", userName);
    // the mysql value is a number of gigabytes so shift it over
    ret = sqlQuickLongLong(conn, dyStringCannibalize(&query)) * 1000000000;
    }
return ret;
}