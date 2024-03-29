/* genotypeFreqs.c was originally generated by the autoSql program, which also 
 * generated genotypeFreqs.h and genotypeFreqs.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "genotypeFreqs.h"


void genotypeFreqsStaticLoad(char **row, struct genotypeFreqs *ret)
/* Load a row from genotypeFreqs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int sizeOne,i;
char *s;

ret->rsId = row[0];
ret->chrom = row[1];
ret->chromStart = sqlSigned(row[2]);
ret->strand = row[3][0];
ret->assembly = row[4];
ret->center = row[5];
ret->protLSID = row[6];
ret->assayLSID = row[7];
ret->panelLSID = row[8];
ret->majGt = row[9];
ret->majFreq = atof(row[10]);
ret->majCount = sqlSigned(row[11]);
ret->hetGt = row[12];
ret->hetFreq = atof(row[13]);
ret->hetCount = sqlSigned(row[14]);
ret->minGt = row[15];
ret->minFreq = atof(row[16]);
ret->minCount = sqlSigned(row[17]);
ret->totCount = sqlSigned(row[18]);
ret->minAlFreq = atof(row[19]);
}

struct genotypeFreqs *genotypeFreqsLoad(char **row)
/* Load a genotypeFreqs from row fetched with select * from genotypeFreqs
 * from database.  Dispose of this with genotypeFreqsFree(). */
{
struct genotypeFreqs *ret;
int sizeOne,i;
char *s;

AllocVar(ret);
ret->rsId = cloneString(row[0]);
ret->chrom = cloneString(row[1]);
ret->chromStart = sqlSigned(row[2]);
ret->strand = row[3][0];
ret->assembly = cloneString(row[4]);
ret->center = cloneString(row[5]);
ret->protLSID = cloneString(row[6]);
ret->assayLSID = cloneString(row[7]);
ret->panelLSID = cloneString(row[8]);
ret->majGt = cloneString(row[9]);
ret->majFreq = atof(row[10]);
ret->majCount = sqlSigned(row[11]);
ret->hetGt = cloneString(row[12]);
ret->hetFreq = atof(row[13]);
ret->hetCount = sqlSigned(row[14]);
ret->minGt = cloneString(row[15]);
ret->minFreq = atof(row[16]);
ret->minCount = sqlSigned(row[17]);
ret->totCount = sqlSigned(row[18]);
ret->minAlFreq = atof(row[19]);
return ret;
}

struct genotypeFreqs *genotypeFreqsLoadAll(char *fileName) 
/* Load all genotypeFreqs from a whitespace-separated file.
 * Dispose of this with genotypeFreqsFreeList(). */
{
struct genotypeFreqs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[20];

while (lineFileRow(lf, row))
    {
    el = genotypeFreqsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct genotypeFreqs *genotypeFreqsLoadAllByChar(char *fileName, char chopper) 
/* Load all genotypeFreqs from a chopper separated file.
 * Dispose of this with genotypeFreqsFreeList(). */
{
struct genotypeFreqs *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[20];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = genotypeFreqsLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct genotypeFreqs *genotypeFreqsLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all genotypeFreqs from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with genotypeFreqsFreeList(). */
{
struct genotypeFreqs *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = genotypeFreqsLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void genotypeFreqsSaveToDb(struct sqlConnection *conn, struct genotypeFreqs *el, char *tableName, int updateSize)
/* Save genotypeFreqs as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */
{
struct dyString *update = dyStringNew(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( '%s','%s',%d,'%c','%s','%s','%s','%s','%s','%s',%f,%d,'%s',%f,%d,'%s',%f,%d,%d,%f)", 
	tableName,  el->rsId,  el->chrom,  el->chromStart,  el->strand,  el->assembly,  el->center,  el->protLSID,  el->assayLSID,  el->panelLSID,  el->majGt,  el->majFreq,  el->majCount,  el->hetGt,  el->hetFreq,  el->hetCount,  el->minGt,  el->minFreq,  el->minCount,  el->totCount,  el->minAlFreq);
sqlUpdate(conn, update->string);
dyStringFree(&update);
}


struct genotypeFreqs *genotypeFreqsCommaIn(char **pS, struct genotypeFreqs *ret)
/* Create a genotypeFreqs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new genotypeFreqs */
{
char *s = *pS;
int i;

if (ret == NULL)
    AllocVar(ret);
ret->rsId = sqlStringComma(&s);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlSignedComma(&s);
sqlFixedStringComma(&s, &(ret->strand), sizeof(ret->strand));
ret->assembly = sqlStringComma(&s);
ret->center = sqlStringComma(&s);
ret->protLSID = sqlStringComma(&s);
ret->assayLSID = sqlStringComma(&s);
ret->panelLSID = sqlStringComma(&s);
ret->majGt = sqlStringComma(&s);
ret->majFreq = sqlFloatComma(&s);
ret->majCount = sqlSignedComma(&s);
ret->hetGt = sqlStringComma(&s);
ret->hetFreq = sqlFloatComma(&s);
ret->hetCount = sqlSignedComma(&s);
ret->minGt = sqlStringComma(&s);
ret->minFreq = sqlFloatComma(&s);
ret->minCount = sqlSignedComma(&s);
ret->totCount = sqlSignedComma(&s);
ret->minAlFreq = sqlFloatComma(&s);
*pS = s;
return ret;
}

void genotypeFreqsFree(struct genotypeFreqs **pEl)
/* Free a single dynamically allocated genotypeFreqs such as created
 * with genotypeFreqsLoad(). */
{
struct genotypeFreqs *el;

if ((el = *pEl) == NULL) return;
freeMem(el->rsId);
freeMem(el->chrom);
freeMem(el->assembly);
freeMem(el->center);
freeMem(el->protLSID);
freeMem(el->assayLSID);
freeMem(el->panelLSID);
freeMem(el->majGt);
freeMem(el->hetGt);
freeMem(el->minGt);
freez(pEl);
}

void genotypeFreqsFreeList(struct genotypeFreqs **pList)
/* Free a list of dynamically allocated genotypeFreqs's */
{
struct genotypeFreqs *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    genotypeFreqsFree(&el);
    }
*pList = NULL;
}

void genotypeFreqsOutput(struct genotypeFreqs *el, FILE *f, char sep, char lastSep) 
/* Print out genotypeFreqs.  Separate fields with sep. Follow last field with lastSep. */
{
int i;
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rsId);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->chromStart);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->assembly);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->center);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->protLSID);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->assayLSID);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->panelLSID);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->majGt);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->majFreq);
fputc(sep,f);
fprintf(f, "%d", el->majCount);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->hetGt);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->hetFreq);
fputc(sep,f);
fprintf(f, "%d", el->hetCount);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->minGt);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->minFreq);
fputc(sep,f);
fprintf(f, "%d", el->minCount);
fputc(sep,f);
fprintf(f, "%d", el->totCount);
fputc(sep,f);
fprintf(f, "%f", el->minAlFreq);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

