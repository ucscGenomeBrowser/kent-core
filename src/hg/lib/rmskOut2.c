/* rmskOut2.c was originally generated by the autoSql program, which also 
 * generated rmskOut2.h and rmskOut2.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rmskOut.h"
#include "rmskOut2.h"
#include "obscure.h"
#include "binRange.h"



void rmskOut2StaticLoad(char **row, struct rmskOut2 *ret)
/* Load a row from rmskOut2 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->swScore = sqlUnsigned(row[0]);
ret->milliDiv = sqlUnsigned(row[1]);
ret->milliDel = sqlUnsigned(row[2]);
ret->milliIns = sqlUnsigned(row[3]);
ret->genoName = row[4];
ret->genoStart = sqlUnsigned(row[5]);
ret->genoEnd = sqlUnsigned(row[6]);
ret->genoLeft = sqlSigned(row[7]);
safecpy(ret->strand, sizeof(ret->strand), row[8]);
ret->repName = row[9];
ret->repClass = row[10];
ret->repFamily = row[11];
ret->repStart = sqlSigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repLeft = sqlSigned(row[14]);
ret->id = sqlUnsigned(row[15]);
}

struct rmskOut2 *rmskOut2Load(char **row)
/* Load a rmskOut2 from row fetched with select * from rmskOut2
 * from database.  Dispose of this with rmskOut2Free(). */
{
struct rmskOut2 *ret;

AllocVar(ret);
ret->swScore = sqlUnsigned(row[0]);
ret->milliDiv = sqlUnsigned(row[1]);
ret->milliDel = sqlUnsigned(row[2]);
ret->milliIns = sqlUnsigned(row[3]);
ret->genoName = cloneString(row[4]);
ret->genoStart = sqlUnsigned(row[5]);
ret->genoEnd = sqlUnsigned(row[6]);
ret->genoLeft = sqlSigned(row[7]);
safecpy(ret->strand, sizeof(ret->strand), row[8]);
ret->repName = cloneString(row[9]);
ret->repClass = cloneString(row[10]);
ret->repFamily = cloneString(row[11]);
ret->repStart = sqlSigned(row[12]);
ret->repEnd = sqlUnsigned(row[13]);
ret->repLeft = sqlSigned(row[14]);
ret->id = sqlUnsigned(row[15]);
return ret;
}

struct rmskOut2 *rmskOut2LoadAll(char *fileName) 
/* Load all rmskOut2 from a whitespace-separated file.
 * Dispose of this with rmskOut2FreeList(). */
{
struct rmskOut2 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[16];

while (lineFileRow(lf, row))
    {
    el = rmskOut2Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rmskOut2 *rmskOut2LoadAllByChar(char *fileName, char chopper) 
/* Load all rmskOut2 from a chopper separated file.
 * Dispose of this with rmskOut2FreeList(). */
{
struct rmskOut2 *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[16];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = rmskOut2Load(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rmskOut2 *rmskOut2CommaIn(char **pS, struct rmskOut2 *ret)
/* Create a rmskOut2 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rmskOut2 */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->swScore = sqlUnsignedComma(&s);
ret->milliDiv = sqlUnsignedComma(&s);
ret->milliDel = sqlUnsignedComma(&s);
ret->milliIns = sqlUnsignedComma(&s);
ret->genoName = sqlStringComma(&s);
ret->genoStart = sqlUnsignedComma(&s);
ret->genoEnd = sqlUnsignedComma(&s);
ret->genoLeft = sqlSignedComma(&s);
sqlFixedStringComma(&s, ret->strand, sizeof(ret->strand));
ret->repName = sqlStringComma(&s);
ret->repClass = sqlStringComma(&s);
ret->repFamily = sqlStringComma(&s);
ret->repStart = sqlSignedComma(&s);
ret->repEnd = sqlUnsignedComma(&s);
ret->repLeft = sqlSignedComma(&s);
ret->id = sqlUnsignedComma(&s);
*pS = s;
return ret;
}

void rmskOut2Free(struct rmskOut2 **pEl)
/* Free a single dynamically allocated rmskOut2 such as created
 * with rmskOut2Load(). */
{
struct rmskOut2 *el;

if ((el = *pEl) == NULL) return;
freeMem(el->genoName);
freeMem(el->repName);
freeMem(el->repClass);
freeMem(el->repFamily);
freez(pEl);
}

void rmskOut2FreeList(struct rmskOut2 **pList)
/* Free a list of dynamically allocated rmskOut2's */
{
struct rmskOut2 *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rmskOut2Free(&el);
    }
*pList = NULL;
}

void rmskOut2Output(struct rmskOut2 *el, FILE *f, char sep, char lastSep) 
/* Print out rmskOut2.  Separate fields with sep. Follow last field with lastSep. */
{
fprintf(f, "%u", el->swScore);
fputc(sep,f);
fprintf(f, "%u", el->milliDiv);
fputc(sep,f);
fprintf(f, "%u", el->milliDel);
fputc(sep,f);
fprintf(f, "%u", el->milliIns);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->genoName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->genoStart);
fputc(sep,f);
fprintf(f, "%u", el->genoEnd);
fputc(sep,f);
fprintf(f, "%d", el->genoLeft);
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
fprintf(f, "%s", el->repClass);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->repFamily);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->repStart);
fputc(sep,f);
fprintf(f, "%u", el->repEnd);
fputc(sep,f);
fprintf(f, "%d", el->repLeft);
fputc(sep,f);
fprintf(f, "%u", el->id);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */


void rmskOut2OpenVerify(char *fileName, struct lineFile **retFile, boolean *retEmpty)
/* Open repeat masker .out file and verify that it is good.
 * Set retEmpty if it has header characteristic of an empty file. */
{
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *line;
int lineSize;

lineFileNeedNext(lf, &line, &lineSize);
if (startsWith("There were no", line))
    *retEmpty = TRUE;
line = skipLeadingSpaces(line);
if (! ( startsWith("SW", line) || startsWith("bit", line) ) )
  errAbort("%s doesn't seem to be a RepeatMasker .out file", fileName);
lineFileSkip(lf, 2);
*retEmpty = FALSE;
*retFile = lf;
}

static int negParenNum2(struct lineFile *lf, char *s)
/* Return number where negative is shown by parenthization. */
{
boolean hasParen = FALSE;
int result;
if (*s == '(')
   {
   hasParen = TRUE;
   ++s;
   }
if (!isdigit(s[0]) && s[0] != '-')
   errAbort("Expecting digit line %d of %s got %s\n",
        lf->lineIx, lf->fileName, s);
result = atoi(s);
if (hasParen)
    result = -result;
return result;
}

static void parseClassAndFamily(char *s, char **retClass, char **retFamily)
/* Separate repeatMasker class/family .*/
{
char *e = strchr(s, '/');
if (e == NULL)
    *retClass = *retFamily = s;
else
    {
    *e++ = 0;
    *retClass = s;
    *retFamily = e;
    }
}

struct rmskOut2 *rmskOut2ReadNext(struct lineFile *lf)
/* Read next record from repeat masker file.  Return NULL at EOF. */
{
char *words[32];
int wordCount;
struct rmskOut2 *ret;
char *class, *family;

if ((wordCount = lineFileChop(lf, words)) == 0)
    return NULL;
if (wordCount != 15 )
    errAbort("Expecting 15 words - line %d of %s", lf->lineIx, lf->fileName);

AllocVar(ret);
ret->swScore = lineFileNeedNum(lf, words, 0);
ret->milliDiv = round(10.0*atof(words[1]));
ret->milliDel = round(10.0*atof(words[2]));
ret->milliIns = round(10.0*atof(words[3]));
ret->genoName = cloneString(words[4]);
ret->genoStart = lineFileNeedNum(lf, words, 5)-1;
ret->genoEnd = lineFileNeedNum(lf, words, 6);
ret->genoLeft = -negParenNum2(lf, words[7]);
if (sameString(words[8], "C"))
    ret->strand[0] = '-';
else if (sameString(words[8], "+"))
    ret->strand[0] = '+';
else
    errAbort("Unexpected strand char line %d of %s", lf->lineIx, lf->fileName);
ret->repName = cloneString(words[9]);
parseClassAndFamily(words[10], &class, &family);
ret->repClass = cloneString(class);
ret->repFamily = cloneString(family);
if (sameString(words[8], "C"))
{
    ret->repStart = negParenNum2(lf, words[11])-1;
    ret->repEnd = sqlSigned(words[12]);
    ret->repLeft = -negParenNum2(lf, words[13]);
}else
{
    ret->repLeft = -negParenNum2(lf, words[11]);
    ret->repEnd = sqlSigned(words[12]);
    ret->repStart = negParenNum2(lf, words[13])-1;
}
return ret;
}

struct rmskOut2 *rmskOut2Read(char *fileName)
/* Read all records in .out file and return as list. */
{
struct lineFile *lf;
boolean isEmpty;
struct rmskOut2 *list = NULL, *el;

rmskOut2OpenVerify(fileName, &lf, &isEmpty);
if (!isEmpty)
    {
    while ((el = rmskOut2ReadNext(lf)) != NULL)
       {
       slAddHead(&list, el);
       }
    slReverse(&list);
    }
lineFileClose(&lf);
return list;
}

void rmskOut2WriteHead(FILE *f)
/* Write out rmsk header lines. */
{
fprintf(f,
"   SW  perc perc perc  query      position in query           matching       repeat              position in  repeat\n"
"score  div. del. ins.  sequence    begin     end    (left)    repeat         class/family         begin  end (left)   ID\n"
"\n");
}

static void parenNeg2(int num, char *s, size_t sSize)
/* Write number to s, parenthesizing if negative. */
{
if (num <= 0)
   safef(s, sSize, "(%d)", -num);
else
   safef(s, sSize, "%d", num);
}

void rmskOut2WriteOneOut(struct rmskOut2 *rmsk, FILE *f)
/* Write one rmsk in .out format to file. */
{
char genoLeft[24], repStart[24], repLeft[24];
char classFam[128];

parenNeg2(-rmsk->genoLeft, genoLeft, sizeof(genoLeft));
parenNeg2(rmsk->repStart+1, repStart, sizeof(repStart));
parenNeg2(-rmsk->repLeft, repLeft, sizeof(repLeft));
if (sameString(rmsk->repClass, rmsk->repFamily))
    safef(classFam, sizeof(classFam), "%s", rmsk->repClass);
else
    safef(classFam, sizeof(classFam), "%s/%s",
	  rmsk->repClass, rmsk->repFamily);
if ( rmsk->strand[0] == '+' )
  fprintf(f, 
    "%5d %5.1f %4.1f %4.1f  %-9s %7d %7d %9s %1s  %-14s %-19s %6s %4d %6s %6d\n",
    rmsk->swScore, 0.1*rmsk->milliDiv, 0.1*rmsk->milliDel, 0.1*rmsk->milliIns, 
    rmsk->genoName, rmsk->genoStart+1, rmsk->genoEnd, genoLeft, "+", 
    rmsk->repName, classFam, repStart, rmsk->repEnd, repLeft, rmsk->id);
else
  fprintf(f, 
    "%5d %5.1f %4.1f %4.1f  %-9s %7d %7d %9s %1s  %-14s %-19s %6s %4d %6s %6d\n",
    rmsk->swScore, 0.1*rmsk->milliDiv, 0.1*rmsk->milliDel, 0.1*rmsk->milliIns, 
    rmsk->genoName, rmsk->genoStart+1, rmsk->genoEnd, genoLeft, "C",
    rmsk->repName, classFam, repLeft, rmsk->repEnd, repStart, rmsk->id);

}

void rmskOut2WriteAllOut(char *fileName, struct rmskOut2 *rmskList)
/* Write .out format file containing all in rmskList. */
{
FILE *f = mustOpen(fileName, "w");
struct rmskOut2 *rmsk;

rmskOut2WriteHead(f);
for (rmsk = rmskList; rmsk != NULL; rmsk = rmsk->next)
    rmskOut2WriteOneOut(rmsk, f);
fclose(f);
}

struct binKeeper *readRepeats2(char *chrom, char *rmskFileName, struct hash *tSizeHash)
/* read all repeats for a chromosome of size size, returns results in binKeeper structure for fast query*/
{
    boolean rmskRet;
    struct lineFile *rmskF = NULL;
    struct rmskOut2 *rmsk;
    struct binKeeper *bk; 
    int size;

    size = hashIntVal(tSizeHash, chrom);
    bk = binKeeperNew(0, size);
    assert(size > 1);
    rmskOut2OpenVerify(rmskFileName ,&rmskF , &rmskRet);
    while ((rmsk = rmskOut2ReadNext(rmskF)) != NULL)
        {
        binKeeperAdd(bk, rmsk->genoStart, rmsk->genoEnd, rmsk);
        }
    lineFileClose(&rmskF);
    return bk;
}

struct hash *readRepeatsAll2(char *sizeFileName, char *rmskDir)
/* read all repeats for a all chromosomes getting sizes from sizeFileNmae , returns results in hash of binKeeper structure for fast query*/
{
boolean rmskRet;
struct binKeeper *bk; 
struct lineFile *rmskF = NULL;
struct rmskOut2 *rmsk;
struct lineFile *lf = lineFileOpen(sizeFileName, TRUE);
struct hash *hash = newHash(0);
char *row[2];
char rmskFileName[256];

while (lineFileRow(lf, row))
    {
    char *name = row[0];
    int size = lineFileNeedNum(lf, row, 1);

    if (hashLookup(hash, name) != NULL)
        warn("Duplicate %s, ignoring all but first\n", name);
    else
        {
        bk = binKeeperNew(0, size);
        assert(size > 1);
        safef(rmskFileName, sizeof(rmskFileName), "%s/%s.fa.out",rmskDir,name);
        rmskOut2OpenVerify(rmskFileName ,&rmskF , &rmskRet);
        while ((rmsk = rmskOut2ReadNext(rmskF)) != NULL)
            {
            binKeeperAdd(bk, rmsk->genoStart, rmsk->genoEnd, rmsk);
            }
        lineFileClose(&rmskF);
	hashAdd(hash, name, bk);
        }
    }
lineFileClose(&lf);
return hash;
}
