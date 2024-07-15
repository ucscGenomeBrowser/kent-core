/* genomePriority.c was originally generated by the autoSql program, which also 
 * generated genomePriority.h and genomePriority.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "genomePriority.h"



char *genomePriorityCommaSepFieldNames = "name,priority,commonName,scientificName,taxId,description";

void genomePriorityStaticLoad(char **row, struct genomePriority *ret)
/* Load a row from genomePriority table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->priority = sqlUnsigned(row[1]);
ret->commonName = row[2];
ret->scientificName = row[3];
ret->taxId = sqlUnsigned(row[4]);
ret->description = row[5];
}

struct genomePriority *genomePriorityLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all genomePriority from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with genomePriorityFreeList(). */
{
struct genomePriority *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = genomePriorityLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void genomePrioritySaveToDb(struct sqlConnection *conn, struct genomePriority *el, char *tableName, int updateSize)
/* Save genomePriority as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. This function automatically escapes quoted strings for mysql. */
{
struct dyString *update = dyStringNew(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( '%s',%u,'%s','%s',%u,'%s')", 
	tableName,  el->name,  el->priority,  el->commonName,  el->scientificName,  el->taxId,  el->description);
sqlUpdate(conn, update->string);
dyStringFree(&update);
}

struct genomePriority *genomePriorityLoad(char **row)
/* Load a genomePriority from row fetched with select * from genomePriority
 * from database.  Dispose of this with genomePriorityFree(). */
{
struct genomePriority *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->priority = sqlUnsigned(row[1]);
ret->commonName = cloneString(row[2]);
ret->scientificName = cloneString(row[3]);
ret->taxId = sqlUnsigned(row[4]);
ret->description = cloneString(row[5]);
return ret;
}

struct genomePriority *genomePriorityLoadAll(char *fileName) 
/* Load all genomePriority from a whitespace-separated file.
 * Dispose of this with genomePriorityFreeList(). */
{
struct genomePriority *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileRow(lf, row))
    {
    el = genomePriorityLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct genomePriority *genomePriorityLoadAllByChar(char *fileName, char chopper) 
/* Load all genomePriority from a chopper separated file.
 * Dispose of this with genomePriorityFreeList(). */
{
struct genomePriority *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[6];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = genomePriorityLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct genomePriority *genomePriorityCommaIn(char **pS, struct genomePriority *ret)
/* Create a genomePriority out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new genomePriority */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->priority = sqlUnsignedComma(&s);
ret->commonName = sqlStringComma(&s);
ret->scientificName = sqlStringComma(&s);
ret->taxId = sqlUnsignedComma(&s);
ret->description = sqlStringComma(&s);
*pS = s;
return ret;
}

void genomePriorityFree(struct genomePriority **pEl)
/* Free a single dynamically allocated genomePriority such as created
 * with genomePriorityLoad(). */
{
struct genomePriority *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->commonName);
freeMem(el->scientificName);
freeMem(el->description);
freez(pEl);
}

void genomePriorityFreeList(struct genomePriority **pList)
/* Free a list of dynamically allocated genomePriority's */
{
struct genomePriority *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    genomePriorityFree(&el);
    }
*pList = NULL;
}

void genomePriorityOutput(struct genomePriority *el, FILE *f, char sep, char lastSep) 
/* Print out genomePriority.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->priority);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->commonName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->scientificName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->taxId);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->description);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

void genomePriorityJsonOutput(struct genomePriority *el, FILE *f) 
/* Print out genomePriority in JSON format. */
{
fputc('{',f);
fputc('"',f);
fprintf(f,"name");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->name);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"priority");
fputc('"',f);
fputc(':',f);
fprintf(f, "%u", el->priority);
fputc(',',f);
fputc('"',f);
fprintf(f,"commonName");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->commonName);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"scientificName");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->scientificName);
fputc('"',f);
fputc(',',f);
fputc('"',f);
fprintf(f,"taxId");
fputc('"',f);
fputc(':',f);
fprintf(f, "%u", el->taxId);
fputc(',',f);
fputc('"',f);
fprintf(f,"description");
fputc('"',f);
fputc(':',f);
fputc('"',f);
fprintf(f, "%s", el->description);
fputc('"',f);
fputc('}',f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

