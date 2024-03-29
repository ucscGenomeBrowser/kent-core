/* stsInfo2.c was originally generated by the autoSql program, which also 
 * generated stsInfo2.h and stsInfo2.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "stsInfo2.h"


struct stsInfo2 *stsInfo2Load(char **row)
/* Load a stsInfo2 from row fetched with select * from stsInfo2
 * from database.  Dispose of this with stsInfo2Free(). */
{
struct stsInfo2 *ret;
int sizeOne;

AllocVar(ret);
ret->gbCount = sqlUnsigned(row[2]);
ret->gdbCount = sqlUnsigned(row[4]);
ret->nameCount = sqlUnsigned(row[6]);
ret->otherDbstsCount = sqlUnsigned(row[9]);
ret->otherUCSCcount = sqlUnsigned(row[16]);
ret->mergeUCSCcount = sqlUnsigned(row[18]);
ret->identNo = sqlUnsigned(row[0]);
ret->name = cloneString(row[1]);
sqlStringDynamicArray(row[3], &ret->genbank, &sizeOne);
assert(sizeOne == ret->gbCount);
sqlStringDynamicArray(row[5], &ret->gdb, &sizeOne);
assert(sizeOne == ret->gdbCount);
sqlStringDynamicArray(row[7], &ret->otherNames, &sizeOne);
assert(sizeOne == ret->nameCount);
ret->dbSTSid = sqlUnsigned(row[8]);
sqlUnsignedDynamicArray(row[10], &ret->otherDbSTS, &sizeOne);
assert(sizeOne == ret->otherDbstsCount);
ret->leftPrimer = cloneString(row[11]);
ret->rightPrimer = cloneString(row[12]);
ret->distance = cloneString(row[13]);
ret->organism = cloneString(row[14]);
ret->sequence = sqlUnsigned(row[15]);
sqlUnsignedDynamicArray(row[17], &ret->otherUCSC, &sizeOne);
assert(sizeOne == ret->otherUCSCcount);
sqlUnsignedDynamicArray(row[19], &ret->mergeUCSC, &sizeOne);
assert(sizeOne == ret->mergeUCSCcount);
ret->genethonName = cloneString(row[20]);
ret->genethonChr = cloneString(row[21]);
ret->genethonPos = atof(row[22]);
ret->genethonLOD = atof(row[23]);
ret->marshfieldName = cloneString(row[24]);
ret->marshfieldChr = cloneString(row[25]);
ret->marshfieldPos = atof(row[26]);
ret->marshfieldLOD = atof(row[27]);
ret->wiyacName = cloneString(row[28]);
ret->wiyacChr = cloneString(row[29]);
ret->wiyacPos = atof(row[30]);
ret->wiyacLOD = atof(row[31]);
ret->wirhName = cloneString(row[32]);
ret->wirhChr = cloneString(row[33]);
ret->wirhPos = atof(row[34]);
ret->wirhLOD = atof(row[35]);
ret->gm99gb4Name = cloneString(row[36]);
ret->gm99gb4Chr = cloneString(row[37]);
ret->gm99gb4Pos = atof(row[38]);
ret->gm99gb4LOD = atof(row[39]);
ret->gm99g3Name = cloneString(row[40]);
ret->gm99g3Chr = cloneString(row[41]);
ret->gm99g3Pos = atof(row[42]);
ret->gm99g3LOD = atof(row[43]);
ret->tngName = cloneString(row[44]);
ret->tngChr = cloneString(row[45]);
ret->tngPos = atof(row[46]);
ret->tngLOD = atof(row[47]);
ret->decodeName = cloneString(row[48]);
ret->decodeChr = cloneString(row[49]);
ret->decodePos = atof(row[50]);
ret->decodeLOD = atof(row[51]);
return ret;
}

struct stsInfo2 *stsInfo2LoadAll(char *fileName) 
/* Load all stsInfo2 from a tab-separated file.
 * Dispose of this with stsInfo2FreeList(). */
{
struct stsInfo2 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[52];

while (lineFileRow(lf, row))
    {
    el = stsInfo2Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct stsInfo2 *stsInfo2LoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all stsInfo2 from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with stsInfo2FreeList(). */
{
struct stsInfo2 *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

sqlDyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    sqlDyStringPrintf(query, " where %-s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = stsInfo2Load(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct stsInfo2 *stsInfo2CommaIn(char **pS, struct stsInfo2 *ret)
/* Create a stsInfo2 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new stsInfo2 */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->identNo = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->gbCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->genbank, ret->gbCount);
for (i=0; i<ret->gbCount; ++i)
    {
    ret->genbank[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->gdbCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->gdb, ret->gdbCount);
for (i=0; i<ret->gdbCount; ++i)
    {
    ret->gdb[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->nameCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->otherNames, ret->nameCount);
for (i=0; i<ret->nameCount; ++i)
    {
    ret->otherNames[i] = sqlStringComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->dbSTSid = sqlUnsignedComma(&s);
ret->otherDbstsCount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->otherDbSTS, ret->otherDbstsCount);
for (i=0; i<ret->otherDbstsCount; ++i)
    {
    ret->otherDbSTS[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->leftPrimer = sqlStringComma(&s);
ret->rightPrimer = sqlStringComma(&s);
ret->distance = sqlStringComma(&s);
ret->organism = sqlStringComma(&s);
ret->sequence = sqlUnsignedComma(&s);
ret->otherUCSCcount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->otherUCSC, ret->otherUCSCcount);
for (i=0; i<ret->otherUCSCcount; ++i)
    {
    ret->otherUCSC[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->mergeUCSCcount = sqlUnsignedComma(&s);
s = sqlEatChar(s, '{');
AllocArray(ret->mergeUCSC, ret->mergeUCSCcount);
for (i=0; i<ret->mergeUCSCcount; ++i)
    {
    ret->mergeUCSC[i] = sqlUnsignedComma(&s);
    }
s = sqlEatChar(s, '}');
s = sqlEatChar(s, ',');
ret->genethonName = sqlStringComma(&s);
ret->genethonChr = sqlStringComma(&s);
ret->genethonPos = sqlFloatComma(&s);
ret->genethonLOD = sqlFloatComma(&s);
ret->marshfieldName = sqlStringComma(&s);
ret->marshfieldChr = sqlStringComma(&s);
ret->marshfieldPos = sqlFloatComma(&s);
ret->marshfieldLOD = sqlFloatComma(&s);
ret->wiyacName = sqlStringComma(&s);
ret->wiyacChr = sqlStringComma(&s);
ret->wiyacPos = sqlFloatComma(&s);
ret->wiyacLOD = sqlFloatComma(&s);
ret->wirhName = sqlStringComma(&s);
ret->wirhChr = sqlStringComma(&s);
ret->wirhPos = sqlFloatComma(&s);
ret->wirhLOD = sqlFloatComma(&s);
ret->gm99gb4Name = sqlStringComma(&s);
ret->gm99gb4Chr = sqlStringComma(&s);
ret->gm99gb4Pos = sqlFloatComma(&s);
ret->gm99gb4LOD = sqlFloatComma(&s);
ret->gm99g3Name = sqlStringComma(&s);
ret->gm99g3Chr = sqlStringComma(&s);
ret->gm99g3Pos = sqlFloatComma(&s);
ret->gm99g3LOD = sqlFloatComma(&s);
ret->tngName = sqlStringComma(&s);
ret->tngChr = sqlStringComma(&s);
ret->tngPos = sqlFloatComma(&s);
ret->tngLOD = sqlFloatComma(&s);
ret->decodeName = sqlStringComma(&s);
ret->decodeChr = sqlStringComma(&s);
ret->decodePos = sqlFloatComma(&s);
ret->decodeLOD = sqlFloatComma(&s);
*pS = s;
return ret;
}

void stsInfo2Free(struct stsInfo2 **pEl)
/* Free a single dynamically allocated stsInfo2 such as created
 * with stsInfo2Load(). */
{
struct stsInfo2 *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
/* All strings in genbank are allocated at once, so only need to free first. */
if (el->genbank != NULL)
    freeMem(el->genbank[0]);
freeMem(el->genbank);
/* All strings in gdb are allocated at once, so only need to free first. */
if (el->gdb != NULL)
    freeMem(el->gdb[0]);
freeMem(el->gdb);
/* All strings in otherNames are allocated at once, so only need to free first. */
if (el->otherNames != NULL)
    freeMem(el->otherNames[0]);
freeMem(el->otherNames);
freeMem(el->otherDbSTS);
freeMem(el->leftPrimer);
freeMem(el->rightPrimer);
freeMem(el->distance);
freeMem(el->organism);
freeMem(el->otherUCSC);
freeMem(el->mergeUCSC);
freeMem(el->genethonName);
freeMem(el->genethonChr);
freeMem(el->marshfieldName);
freeMem(el->marshfieldChr);
freeMem(el->wiyacName);
freeMem(el->wiyacChr);
freeMem(el->wirhName);
freeMem(el->wirhChr);
freeMem(el->gm99gb4Name);
freeMem(el->gm99gb4Chr);
freeMem(el->gm99g3Name);
freeMem(el->gm99g3Chr);
freeMem(el->tngName);
freeMem(el->tngChr);
freeMem(el->decodeName);
freeMem(el->decodeChr);
freez(pEl);
}

void stsInfo2FreeList(struct stsInfo2 **pList)
/* Free a list of dynamically allocated stsInfo2's */
{
struct stsInfo2 *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    stsInfo2Free(&el);
    }
*pList = NULL;
}

void stsInfo2Output(struct stsInfo2 *el, FILE *f, char sep, char lastSep) 
/* Print out stsInfo2.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
fprintf(f, "%u", el->identNo);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->gbCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->gbCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->genbank[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->gdbCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->gdbCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->gdb[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->nameCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->nameCount; ++i)
    {
    if (sep == ',') fputc('"',f);
    fprintf(f, "%s", el->otherNames[i]);
    if (sep == ',') fputc('"',f);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->dbSTSid);
fputc(sep,f);
fprintf(f, "%u", el->otherDbstsCount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->otherDbstsCount; ++i)
    {
    fprintf(f, "%u", el->otherDbSTS[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->leftPrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rightPrimer);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->distance);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->organism);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->sequence);
fputc(sep,f);
fprintf(f, "%u", el->otherUCSCcount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->otherUCSCcount; ++i)
    {
    fprintf(f, "%u", el->otherUCSC[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
fprintf(f, "%u", el->mergeUCSCcount);
fputc(sep,f);
if (sep == ',') fputc('{',f);
for (i=0; i<el->mergeUCSCcount; ++i)
    {
    fprintf(f, "%u", el->mergeUCSC[i]);
    fputc(',', f);
    }
if (sep == ',') fputc('}',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genethonName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genethonChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->genethonPos);
fputc(sep,f);
fprintf(f, "%f", el->genethonLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->marshfieldName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->marshfieldChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->marshfieldPos);
fputc(sep,f);
fprintf(f, "%f", el->marshfieldLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wiyacName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wiyacChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->wiyacPos);
fputc(sep,f);
fprintf(f, "%f", el->wiyacLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wirhName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->wirhChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->wirhPos);
fputc(sep,f);
fprintf(f, "%f", el->wirhLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99gb4Name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99gb4Chr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->gm99gb4Pos);
fputc(sep,f);
fprintf(f, "%f", el->gm99gb4LOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99g3Name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->gm99g3Chr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->gm99g3Pos);
fputc(sep,f);
fprintf(f, "%f", el->gm99g3LOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tngName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tngChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->tngPos);
fputc(sep,f);
fprintf(f, "%f", el->tngLOD);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->decodeName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->decodeChr);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->decodePos);
fputc(sep,f);
fprintf(f, "%f", el->decodeLOD);
fputc(lastSep,f);
}

