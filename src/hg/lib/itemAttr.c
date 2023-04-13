/* itemAttr.c was originally generated by the autoSql program, which also 
 * generated itemAttr.h and itemAttr.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "itemAttr.h"
#include "hdb.h"


void itemAttrStaticLoad(char **row, struct itemAttr *ret)
/* Load a row from itemAttr table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->chrom = row[1];
ret->chromStart = sqlUnsigned(row[2]);
ret->chromEnd = sqlUnsigned(row[3]);
ret->colorR = sqlUnsigned(row[4]);
ret->colorG = sqlUnsigned(row[5]);
ret->colorB = sqlUnsigned(row[6]);
}

struct itemAttr *itemAttrLoad(char **row)
/* Load a itemAttr from row fetched with select * from itemAttr
 * from database.  Dispose of this with itemAttrFree(). */
{
struct itemAttr *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->chrom = cloneString(row[1]);
ret->chromStart = sqlUnsigned(row[2]);
ret->chromEnd = sqlUnsigned(row[3]);
ret->colorR = sqlUnsigned(row[4]);
ret->colorG = sqlUnsigned(row[5]);
ret->colorB = sqlUnsigned(row[6]);
return ret;
}

struct itemAttr *itemAttrLoadAll(char *fileName) 
/* Load all itemAttr from a whitespace-separated file.
 * Dispose of this with itemAttrFreeList(). */
{
struct itemAttr *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileRow(lf, row))
    {
    el = itemAttrLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct itemAttr *itemAttrLoadAllByChar(char *fileName, char chopper) 
/* Load all itemAttr from a chopper separated file.
 * Dispose of this with itemAttrFreeList(). */
{
struct itemAttr *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[7];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = itemAttrLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct itemAttr *itemAttrCommaIn(char **pS, struct itemAttr *ret)
/* Create a itemAttr out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new itemAttr */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->colorR = sqlUnsignedComma(&s);
ret->colorG = sqlUnsignedComma(&s);
ret->colorB = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void itemAttrFree(struct itemAttr **pEl)
/* Free a single dynamically allocated itemAttr such as created
 * with itemAttrLoad(). */
{
struct itemAttr *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->chrom);
freez(pEl);
}

void itemAttrFreeList(struct itemAttr **pList)
/* Free a list of dynamically allocated itemAttr's */
{
struct itemAttr *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    itemAttrFree(&el);
    }
*pList = NULL;
}

void itemAttrOutput(struct itemAttr *el, FILE *f, char sep, char lastSep) 
/* Print out itemAttr.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
fprintf(f, "%u", el->colorR);
fputc(sep,f);
fprintf(f, "%u", el->colorG);
fputc(sep,f);
fprintf(f, "%u", el->colorB);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


struct itemAttrTbl
/* object holding itemAttr from a range query */
{
    char *table;
    struct hash *nameMap;  /* hash of items, indexed by name, multiple
                            * entries per name allowed. */
};

struct itemAttrTbl *itemAttrTblNew(char *table)
/* Create a new itemAttr object. This saves the table name, but
 * doesn't load the data. */
{
struct itemAttrTbl *iat;
AllocVar(iat);
iat->table = cloneString(table);
return iat;    
}

void itemAttrTblLoad(struct itemAttrTbl *iat,
                     struct sqlConnection *conn,
                     char *chrom, int start, int end)
/* load itemAttrs for the specified chrom range */
{
int rowOffset;
struct sqlResult *result;
char **row;

if (iat->nameMap == NULL)
    iat->nameMap = hashNew(18);

result = hRangeQuery(conn, iat->table, chrom, start, end,
                     NULL, &rowOffset);

while ((row = sqlNextRow(result)) != NULL)
    {
    struct itemAttr* ia = itemAttrLoad(row+rowOffset);
    hashAdd(iat->nameMap, ia->name, ia);
    }
sqlFreeResult(&result);
}

struct itemAttr *itemAttrTblGet(struct itemAttrTbl *iat, char* name,
                                char *chrom, int chromStart, int chromEnd)
/* lookup an itemAttr by name and location */
{
struct hashEl *hel = hashLookup(iat->nameMap, name);

/* search for matching location */
while (hel != NULL)
    {
    struct itemAttr* ia = hel->val;
    if ((ia->chromStart == chromStart) && (ia->chromEnd == chromEnd)
        && sameString(ia->chrom, chrom))
        return ia;
    hel = hashLookupNext(hel);
    }
return NULL; /* not found */
}

void itemAttrTblFree(struct itemAttrTbl **iatPtr)
/* free an itemAttrTbl */
{
struct itemAttrTbl *iat = *iatPtr;
if (iat != NULL)
    {
    struct hashCookie cookie = hashFirst(iat->nameMap);
    struct hashEl *hel;
    while((hel = hashNext(&cookie)) != NULL)
        itemAttrFree((struct itemAttr**)&hel->val);
    hashFree(&iat->nameMap);
    freez(iatPtr);
    }
}
