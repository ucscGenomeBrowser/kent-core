/* quickLiftChain.c was originally generated by the autoSql program, which also 
 * generated quickLiftChain.h and quickLiftChain.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "quickLiftChain.h"



char *quickLiftChainCommaSepFieldNames = "id,fromDb,toDb,path";

void quickLiftChainStaticLoad(char **row, struct quickLiftChain *ret)
/* Load a row from quickLiftChain table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->id = sqlUnsigned(row[0]);
ret->fromDb = row[1];
ret->toDb = row[2];
ret->path = row[3];
}

struct quickLiftChain *quickLiftChainLoad(char **row)
/* Load a quickLiftChain from row fetched with select * from quickLiftChain
 * from database.  Dispose of this with quickLiftChainFree(). */
{
struct quickLiftChain *ret;

AllocVar(ret);
ret->id = sqlUnsigned(row[0]);
ret->fromDb = cloneString(row[1]);
ret->toDb = cloneString(row[2]);
ret->path = cloneString(row[3]);
return ret;
}

struct quickLiftChain *quickLiftChainLoadAll(char *fileName) 
/* Load all quickLiftChain from a whitespace-separated file.
 * Dispose of this with quickLiftChainFreeList(). */
{
struct quickLiftChain *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileRow(lf, row))
    {
    el = quickLiftChainLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct quickLiftChain *quickLiftChainLoadAllByChar(char *fileName, char chopper) 
/* Load all quickLiftChain from a chopper separated file.
 * Dispose of this with quickLiftChainFreeList(). */
{
struct quickLiftChain *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[4];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = quickLiftChainLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct quickLiftChain *quickLiftChainCommaIn(char **pS, struct quickLiftChain *ret)
/* Create a quickLiftChain out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new quickLiftChain */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->id = sqlUnsignedComma(&s);
ret->fromDb = sqlStringComma(&s);
ret->toDb = sqlStringComma(&s);
ret->path = sqlStringComma(&s);
*pS = s;
return ret;
}

void quickLiftChainFree(struct quickLiftChain **pEl)
/* Free a single dynamically allocated quickLiftChain such as created
 * with quickLiftChainLoad(). */
{
struct quickLiftChain *el;

if ((el = *pEl) == NULL) return;
freeMem(el->fromDb);
freeMem(el->toDb);
freeMem(el->path);
freez(pEl);
}

void quickLiftChainFreeList(struct quickLiftChain **pList)
/* Free a list of dynamically allocated quickLiftChain's */
{
struct quickLiftChain *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    quickLiftChainFree(&el);
    }
*pList = NULL;
}

void quickLiftChainOutput(struct quickLiftChain *el, FILE *f, char sep, char lastSep) 
/* Print out quickLiftChain.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->fromDb);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->toDb);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->path);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

