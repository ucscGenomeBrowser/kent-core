/* bigRmskAlignBed.c was originally generated by the autoSql program, which also 
 * generated bigRmskAlignBed.h and bigRmskAlignBed.sql.  This module links the database and
 * the RAM representation of objects. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "bigRmskAlignBed.h"



char *bigRmskAlignBedCommaSepFieldNames = "chrom,chromStart,chromEnd,chromRemain,score,percSubst,percDel,percIns,strand,repName,repType,repSubtype,repStart,repEnd,repRemain,id,calignData";

void bigRmskAlignBedStaticLoad(char **row, struct bigRmskAlignBed *ret)
/* Load a row from bigRmskAlignBed table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->chromRemain = sqlUnsigned(row[3]);
ret->score = sqlFloat(row[4]);
ret->percSubst = sqlFloat(row[5]);
ret->percDel = sqlFloat(row[6]);
ret->percIns = sqlFloat(row[7]);
safecpy(ret->strand, sizeof(ret->strand), row[8]);
ret->repName = row[9];
ret->repType = row[10];
ret->repSubtype = row[11];
ret->repStart = sqlUnsigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repRemain = sqlUnsigned(row[14]);
ret->id = sqlUnsigned(row[15]);
ret->calignData = row[16];
}

struct bigRmskAlignBed *bigRmskAlignBedLoad(char **row)
/* Load a bigRmskAlignBed from row fetched with select * from bigRmskAlignBed
 * from database.  Dispose of this with bigRmskAlignBedFree(). */
{
struct bigRmskAlignBed *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->chromStart = sqlUnsigned(row[1]);
ret->chromEnd = sqlUnsigned(row[2]);
ret->chromRemain = sqlUnsigned(row[3]);
ret->score = sqlFloat(row[4]);
ret->percSubst = sqlFloat(row[5]);
ret->percDel = sqlFloat(row[6]);
ret->percIns = sqlFloat(row[7]);
safecpy(ret->strand, sizeof(ret->strand), row[8]);
ret->repName = cloneString(row[9]);
ret->repType = cloneString(row[10]);
ret->repSubtype = cloneString(row[11]);
ret->repStart = sqlUnsigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repRemain = sqlUnsigned(row[14]);
ret->id = sqlUnsigned(row[15]);
ret->calignData = cloneString(row[16]);
return ret;
}

struct bigRmskAlignBed *bigRmskAlignBedLoadAll(char *fileName) 
/* Load all bigRmskAlignBed from a whitespace-separated file.
 * Dispose of this with bigRmskAlignBedFreeList(). */
{
struct bigRmskAlignBed *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileRow(lf, row))
    {
    el = bigRmskAlignBedLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bigRmskAlignBed *bigRmskAlignBedLoadAllByChar(char *fileName, char chopper) 
/* Load all bigRmskAlignBed from a chopper separated file.
 * Dispose of this with bigRmskAlignBedFreeList(). */
{
struct bigRmskAlignBed *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[17];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = bigRmskAlignBedLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct bigRmskAlignBed *bigRmskAlignBedCommaIn(char **pS, struct bigRmskAlignBed *ret)
/* Create a bigRmskAlignBed out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bigRmskAlignBed */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->chromStart = sqlUnsignedComma(&s);
ret->chromEnd = sqlUnsignedComma(&s);
ret->chromRemain = sqlUnsignedComma(&s);
ret->score = sqlFloatComma(&s);
ret->percSubst = sqlFloatComma(&s);
ret->percDel = sqlFloatComma(&s);
ret->percIns = sqlFloatComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->repName = sqlStringComma(&s);
ret->repType = sqlStringComma(&s);
ret->repSubtype = sqlStringComma(&s);
ret->repStart = sqlUnsignedComma(&s);
ret->repEnd = sqlUnsignedComma(&s);
ret->repRemain = sqlUnsignedComma(&s);
ret->id = sqlUnsignedComma(&s);
ret->calignData = sqlStringComma(&s);
*pS = s;
return ret;
}

void bigRmskAlignBedFree(struct bigRmskAlignBed **pEl)
/* Free a single dynamically allocated bigRmskAlignBed such as created
 * with bigRmskAlignBedLoad(). */
{
struct bigRmskAlignBed *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->repName);
freeMem(el->repType);
freeMem(el->repSubtype);
freeMem(el->calignData);
freez(pEl);
}

void bigRmskAlignBedFreeList(struct bigRmskAlignBed **pList)
/* Free a list of dynamically allocated bigRmskAlignBed's */
{
struct bigRmskAlignBed *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    bigRmskAlignBedFree(&el);
    }
*pList = NULL;
}

void bigRmskAlignBedOutput(struct bigRmskAlignBed *el, FILE *f, char sep, char lastSep) 
/* Print out bigRmskAlignBed.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->chromStart);
fputc(sep,f);
fprintf(f, "%u", el->chromEnd);
fputc(sep,f);
fprintf(f, "%u", el->chromRemain);
fputc(sep,f);
fprintf(f, "%g", el->score);
fputc(sep,f);
fprintf(f, "%g", el->percSubst);
fputc(sep,f);
fprintf(f, "%g", el->percDel);
fputc(sep,f);
fprintf(f, "%g", el->percIns);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repType);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repSubtype);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->repStart);
fputc(sep,f);
fprintf(f, "%u", el->repEnd);
fputc(sep,f);
fprintf(f, "%u", el->repRemain);
fputc(sep,f);
fprintf(f, "%u", el->id);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->calignData);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */
