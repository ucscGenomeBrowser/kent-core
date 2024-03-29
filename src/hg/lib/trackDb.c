/* trackDb.c was originally generated by the autoSql program, which also
 * generated trackDb.h and trackDb.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "trackDb.h"
#include "hash.h"


struct trackDb *trackDbLoad(char **row)
/* Load a trackDb from row fetched with select * from trackDb
 * from database.  Dispose of this with trackDbFree(). */
{
struct trackDb *ret;
int sizeOne;

AllocVar(ret);
ret->restrictCount = sqlSigned(row[14]);
ret->track = cloneString(row[0]);
ret->shortLabel = cloneString(row[1]);
ret->type = cloneString(row[2]);
ret->longLabel = cloneString(row[3]);
ret->visibility = sqlUnsigned(row[4]);
ret->priority = atof(row[5]);
ret->colorR = sqlUnsigned(row[6]);
ret->colorG = sqlUnsigned(row[7]);
ret->colorB = sqlUnsigned(row[8]);
ret->altColorR = sqlUnsigned(row[9]);
ret->altColorG = sqlUnsigned(row[10]);
ret->altColorB = sqlUnsigned(row[11]);
ret->useScore = sqlUnsigned(row[12]);
ret->private = sqlUnsigned(row[13]);
sqlStringDynamicArray(row[15], &ret->restrictList, &sizeOne);
assert(sizeOne == ret->restrictCount);
ret->url = cloneString(row[16]);
ret->html = cloneString(row[17]);
ret->grp = cloneString(row[18]);
ret->canPack = sqlUnsigned(row[19]);
ret->settings = cloneString(row[20]);
return ret;
}

struct trackDb *trackDbLoadAll(char *fileName)
/* Load all trackDb from a whitespace-separated file.
 * Dispose of this with trackDbFreeList(). */
{
struct trackDb *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[21];

while (lineFileRow(lf, row))
    {
    el = trackDbLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct trackDb *trackDbLoadWhere(struct sqlConnection *conn, char *table,
	char *where)
/* Load all trackDb from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with trackDbFreeList(). */
{
struct trackDb *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

sqlDyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    sqlDyStringPrintf(query, " where %-s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = trackDbLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct trackDb *trackDbLoadAllByChar(char *fileName, char chopper)
/* Load all trackDb from a chopper separated file.
 * Dispose of this with trackDbFreeList(). */
{
struct trackDb *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[21];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = trackDbLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct trackDb *trackDbCommaIn(char **pS, struct trackDb *ret)
/* Create a trackDb out of a comma separated string.
 * This will fill in ret if non-null, otherwise will
 * return a new trackDb */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->track = sqlStringComma(&s);
ret->shortLabel = sqlStringComma(&s);
ret->type = sqlStringComma(&s);
ret->longLabel = sqlStringComma(&s);
ret->visibility = sqlUnsignedComma(&s);
ret->priority = sqlFloatComma(&s);
ret->colorR = sqlUnsignedComma(&s);
ret->colorG = sqlUnsignedComma(&s);
ret->colorB = sqlUnsignedComma(&s);
ret->altColorR = sqlUnsignedComma(&s);
ret->altColorG = sqlUnsignedComma(&s);
ret->altColorB = sqlUnsignedComma(&s);
ret->useScore = sqlUnsignedComma(&s);
ret->private = sqlUnsignedComma(&s);
ret->restrictCount = sqlSignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->restrictList, ret->restrictCount);
for (i=0; i<ret->restrictCount; ++i)
    {
    ret->restrictList[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->url = sqlStringComma(&s);
ret->html = sqlStringComma(&s);
ret->grp = sqlStringComma(&s);
ret->canPack = sqlUnsignedComma(&s);
ret->settings = sqlStringComma(&s);
*pS = s;
return ret;
}

void trackDbFree(struct trackDb **pEl)
/* Free a single dynamically allocated trackDb such as created
 * with trackDbLoad(). */
{
return;
// #ifdef NOT
struct trackDb *el;

if ((el = *pEl) == NULL) return;
freeMem(el->track);
freeMem(el->shortLabel);
freeMem(el->type);
freeMem(el->longLabel);
/* All strings in restrictList are allocated at once, so only need to free first. */
if (el->restrictList != NULL)
    freeMem(el->restrictList[0]);
freeMem(el->restrictList);
freeMem(el->url);
freeMem(el->html);
freeMem(el->grp);
freeMem(el->settings);
hashFree(&el->settingsHash);
hashFree(&el->overrides);
tdbExtrasFree(&el->tdbExtras);
freez(pEl);
// #endif /* NOT */
}

void trackDbFreeList(struct trackDb **pList)
/* Free a list of dynamically allocated trackDb's */
{
return;
// #ifdef NOT
struct trackDb *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    trackDbFree(&el);
    }
*pList = NULL;
// #endif /* NOT */
}

void trackDbOutput(struct trackDb *el, FILE *f, char sep, char lastSep)
/* Print out trackDb.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->track);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->shortLabel);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->type);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->longLabel);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->visibility);
fputc(sep,f);
fprintf(f, "%f", el->priority);
fputc(sep,f);
fprintf(f, "%u", el->colorR);
fputc(sep,f);
fprintf(f, "%u", el->colorG);
fputc(sep,f);
fprintf(f, "%u", el->colorB);
fputc(sep,f);
fprintf(f, "%u", el->altColorR);
fputc(sep,f);
fprintf(f, "%u", el->altColorG);
fputc(sep,f);
fprintf(f, "%u", el->altColorB);
fputc(sep,f);
fprintf(f, "%u", el->useScore);
fputc(sep,f);
fprintf(f, "%u", el->private);
fputc(sep,f);
fprintf(f, "%d", el->restrictCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->restrictCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->restrictList[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->url);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->html);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->grp);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->canPack);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->settings);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

