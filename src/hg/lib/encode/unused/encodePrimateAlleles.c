/* encodePrimateAlleles.c was originally generated by the autoSql program, which also 
 * generated encodePrimateAlleles.h and encodePrimateAlleles.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "encode/encodePrimateAlleles.h"


void encodePrimateAllelesStaticLoad(char **row, struct encodePrimateAlleles *ret)
/* Load a row from encodePrimateAlleles table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = row[3];
ret->score = sqlUnsigned(row[4]);
ret->strand = row[5][0];
ret->refAllele = row[6][0];
ret->otherAllele = row[7][0];
ret->chimpChrom = row[8];
ret->chimpPos = sqlUnsigned(row[9]);
ret->chimpStrand = row[10][0];
ret->chimpAllele = row[11];
ret->chimpQual = sqlUnsigned(row[12]);
ret->rhesusChrom = row[13];
ret->rhesusPos = sqlUnsigned(row[14]);
ret->rhesusStrand = row[15][0];
ret->rhesusAllele = row[16];
ret->rhesusQual = sqlUnsigned(row[17]);
}

struct encodePrimateAlleles *encodePrimateAllelesLoad(char **row)
/* Load a encodePrimateAlleles from row fetched with select * from encodePrimateAlleles
 * from database.  Dispose of this with encodePrimateAllelesFree(). */
{
struct encodePrimateAlleles *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->name = cloneString(row[3]);
ret->score = sqlUnsigned(row[4]);
ret->strand = row[5][0];
ret->refAllele = row[6][0];
ret->otherAllele = row[7][0];
ret->chimpChrom = cloneString(row[8]);
ret->chimpPos = sqlUnsigned(row[9]);
ret->chimpStrand = row[10][0];
ret->chimpAllele = cloneString(row[11]);
ret->chimpQual = sqlUnsigned(row[12]);
ret->rhesusChrom = cloneString(row[13]);
ret->rhesusPos = sqlUnsigned(row[14]);
ret->rhesusStrand = row[15][0];
ret->rhesusAllele = cloneString(row[16]);
ret->rhesusQual = sqlUnsigned(row[17]);
return ret;
}

struct encodePrimateAlleles *encodePrimateAllelesLoadAll(char *fileName) 
/* Load all encodePrimateAlleles from a whitespace-separated file.
 * Dispose of this with encodePrimateAllelesFreeList(). */
{
struct encodePrimateAlleles *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[18];

while (lineFileRow(lf, row))
    {
    el = encodePrimateAllelesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodePrimateAlleles *encodePrimateAllelesLoadAllByChar(char *fileName, char chopper) 
/* Load all encodePrimateAlleles from a chopper separated file.
 * Dispose of this with encodePrimateAllelesFreeList(). */
{
struct encodePrimateAlleles *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[18];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = encodePrimateAllelesLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct encodePrimateAlleles *encodePrimateAllelesLoadByQuery(struct sqlConnection *conn, char *query)
/* Load all encodePrimateAlleles from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with encodePrimateAllelesFreeList(). */
{
struct encodePrimateAlleles *list = NULL, *el;
struct sqlResult *sr;
char **row;

sr = sqlGetResult(conn, query);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = encodePrimateAllelesLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
return list;
}

void encodePrimateAllelesSaveToDb(struct sqlConnection *conn, struct encodePrimateAlleles *el, char *tableName, int updateSize)
/* Save encodePrimateAlleles as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */
{
struct dyString *update = dyStringNew(updateSize);
sqlDyStringPrintf(update, "insert into %s values ( '%s',%u,%u,'%s',%u,'%c','%c','%c','%s',%u,'%c','%s',%u,'%s',%u,'%c','%s',%u)", 
	tableName,  el->chrom,  el->chromStart,  el->chromEnd,  el->name,  el->score,  el->strand,  el->refAllele,  el->otherAllele,  el->chimpChrom,  el->chimpPos,  el->chimpStrand,  el->chimpAllele,  el->chimpQual,  el->rhesusChrom,  el->rhesusPos,  el->rhesusStrand,  el->rhesusAllele,  el->rhesusQual);
sqlUpdate(conn, update->string);
dyStringFree(&update);
}


struct encodePrimateAlleles *encodePrimateAllelesCommaIn(char **pS, struct encodePrimateAlleles *ret)
/* Create a encodePrimateAlleles out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new encodePrimateAlleles */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->name = sqlStringComma(&s);
ret->score = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->strand), sizeof(ret->strand));
sqlFixedStringComma(&s, &(ret->refAllele), sizeof(ret->refAllele));
sqlFixedStringComma(&s, &(ret->otherAllele), sizeof(ret->otherAllele));
ret->chimpChrom = sqlStringComma(&s);
ret->chimpPos = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->chimpStrand), sizeof(ret->chimpStrand));
ret->chimpAllele = sqlStringComma(&s);
ret->chimpQual = sqlUnsignedComma(&s);
ret->rhesusChrom = sqlStringComma(&s);
ret->rhesusPos = sqlUnsignedComma(&s);
sqlFixedStringComma(&s, &(ret->rhesusStrand), sizeof(ret->rhesusStrand));
ret->rhesusAllele = sqlStringComma(&s);
ret->rhesusQual = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void encodePrimateAllelesFree(struct encodePrimateAlleles **pEl)
/* Free a single dynamically allocated encodePrimateAlleles such as created
 * with encodePrimateAllelesLoad(). */
{
struct encodePrimateAlleles *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->name);
freeMem(el->chimpChrom);
freeMem(el->chimpAllele);
freeMem(el->rhesusChrom);
freeMem(el->rhesusAllele);
freez(pEl);
}

void encodePrimateAllelesFreeList(struct encodePrimateAlleles **pList)
/* Free a list of dynamically allocated encodePrimateAlleles's */
{
struct encodePrimateAlleles *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    encodePrimateAllelesFree(&el);
    }
*pList = NULL;
}

void encodePrimateAllelesOutput(struct encodePrimateAlleles *el, FILE *f, char sep, char lastSep) 
/* Print out encodePrimateAlleles.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->score);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->refAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->otherAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chimpChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chimpPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->chimpStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chimpAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chimpQual);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rhesusChrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->rhesusPos);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%c", el->rhesusStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rhesusAllele);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->rhesusQual);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

