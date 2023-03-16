/* repMask.c was originally generated by the autoSql program, which also 
 * generated repMask.h and repMask.sql.  This module links the database and the RAM 
 * representation of objects. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "sqlList.h"
#include "sqlNum.h"
#include "repMask.h"
#include "net.h"
#include "linefile.h"


void repeatMaskOutStaticLoad(char **row, struct repeatMaskOut *ret)
/* Load a row from repeatMaskOut table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{
int i;

ret->score = sqlUnsigned(row[0]);
ret->percDiv = atof(row[1]);
ret->percDel = atof(row[2]);
ret->percInc = atof(row[3]);
ret->qName = row[4];
ret->qStart = sqlSigned(row[5]);
ret->qEnd = sqlSigned(row[6]);
ret->qLeft = row[7];
strcpy(ret->strand, row[8]);
ret->rName = row[9];
ret->rFamily = row[10];
ret->rStart = row[11];	/* sometimes this number is enclosed in (parens) */
/* This row[12] is interpreted as an Unsigned - but I can find no good reason
 * for it to be an error exit when there is a negative number here.
 * The only programs that use this routine are blat, psLayout and
 * maskOutFa and none of them even use this number for anything.
 * For data base loading of the rmsk tracks, these .out files are parsed
 * by hgLoadOut and none of this business is referenced.
 */
i = sqlSigned(row[12]);
if( i < 0 ) {
    warn("WARNING: negative rEnd: %d %s:%d-%d %s", i, ret->qName, ret->qStart, ret->qEnd, ret->rName);
    ret->rEnd = 0;
} else {
ret->rEnd = sqlUnsigned(row[12]);
}
ret->rLeft = row[13];
}

struct repeatMaskOut *repeatMaskOutLoad(char **row)
/* Load a repeatMaskOut from row fetched with select * from repeatMaskOut
 * from database.  Dispose of this with repeatMaskOutFree(). */
{
struct repeatMaskOut *ret;

AllocVar(ret);
ret->score = sqlUnsigned(row[0]);
ret->percDiv = atof(row[1]);
ret->percDel = atof(row[2]);
ret->percInc = atof(row[3]);
ret->qName = cloneString(row[4]);
ret->qStart = sqlSigned(row[5]);
ret->qEnd = sqlSigned(row[6]);
ret->qLeft = cloneString(row[7]);
strcpy(ret->strand, row[8]);
ret->rName = cloneString(row[9]);
ret->rFamily = cloneString(row[10]);
ret->rStart = cloneString(row[11]);
ret->rEnd = sqlUnsigned(row[12]);
ret->rLeft = cloneString(row[13]);
return ret;
}

struct repeatMaskOut *repeatMaskOutCommaIn(char **pS)
/* Create a repeatMaskOut out of a comma separated string. */
{
struct repeatMaskOut *ret;
char *s = *pS;

AllocVar(ret);
ret->score = sqlUnsignedComma(&s);
ret->percDiv = sqlSignedComma(&s);
ret->percDel = sqlSignedComma(&s);
ret->percInc = sqlSignedComma(&s);
ret->qName = sqlStringComma(&s);
ret->qStart = sqlSignedComma(&s);
ret->qEnd = sqlSignedComma(&s);
ret->qLeft = sqlStringComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->rName = sqlStringComma(&s);
ret->rFamily = sqlStringComma(&s);
ret->rStart = sqlStringComma(&s);
ret->rEnd = sqlUnsignedComma(&s);
ret->rLeft = sqlStringComma(&s);
*pS = s;
return ret;
}

void repeatMaskOutFree(struct repeatMaskOut **pEl)
/* Free a single dynamically allocated repeatMaskOut such as created
 * with repeatMaskOutLoad(). */
{
struct repeatMaskOut *el;

if ((el = *pEl) == NULL) return;
freeMem(el->qName);
freeMem(el->qLeft);
freeMem(el->rName);
freeMem(el->rFamily);
freeMem(el->rStart);
freeMem(el->rLeft);
freez(pEl);
}

void repeatMaskOutFreeList(struct repeatMaskOut **pList)
/* Free a list of dynamically allocated repeatMaskOut's */
{
struct repeatMaskOut *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    repeatMaskOutFree(&el);
    }
*pList = NULL;
}

void repeatMaskOutOutput(struct repeatMaskOut *el, FILE *f, char sep, char lastSep) 
/* Print out repeatMaskOut.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->score);
fputc(sep,f);
fprintf(f, "%f", el->percDiv);
fputc(sep,f);
fprintf(f, "%f", el->percDel);
fputc(sep,f);
fprintf(f, "%f", el->percInc);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->qStart);
fputc(sep,f);
fprintf(f, "%d", el->qEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qLeft);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->strand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rFamily);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rStart);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->rEnd);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rLeft);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

struct lineFile *rmskLineFileOpen(char *fileName)
/* open a repeat masker .out file or die trying */
{
struct lineFile *lf;
char *line;
int lineSize;

if (udcIsLocal(fileName))
  lf = lineFileOpen(fileName, TRUE);
else
  lf = netLineFileOpen(fileName);

if (!lineFileNext(lf, &line, &lineSize))
    errAbort("Empty %s", lf->fileName);

if (!(startsWith("   SW  perc perc", line) ||
      startsWith("   SW   perc perc", line) ||
      startsWith("    SW   perc perc", line) ||
      startsWith("  bit   perc perc", line)))
    {
    errAbort("%s doesn't seem to be a RepeatMasker .out file, first "
             "line seen:\n%s", lf->fileName, line);
    }
lineFileNext(lf, &line, &lineSize);
lineFileNext(lf, &line, &lineSize);

return lf;
}
