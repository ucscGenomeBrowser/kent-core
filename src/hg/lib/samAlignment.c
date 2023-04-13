/* samAlignment.c was originally generated by the autoSql program, which also
 * generated samAlignment.h and samAlignment.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "samAlignment.h"


void samAlignmentStaticLoad(char **row, struct samAlignment *ret)
/* Load a row from samAlignment table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->qName = row[0];
ret->flag = sqlUnsigned(row[1]);
ret->rName = row[2];
ret->pos = sqlUnsigned(row[3]);
ret->mapQ = sqlUnsigned(row[4]);
ret->cigar = row[5];
ret->rNext = row[6];
ret->pNext = sqlSigned(row[7]);
ret->tLen = sqlSigned(row[8]);
ret->seq = row[9];
ret->qual = row[10];
ret->tagTypeVals = row[11];
}

struct samAlignment *samAlignmentLoad(char **row)
/* Load a samAlignment from row fetched with select * from samAlignment
 * from database.  Dispose of this with samAlignmentFree(). */
{
struct samAlignment *ret;

AllocVar(ret);
ret->qName = cloneString(row[0]);
ret->flag = sqlUnsigned(row[1]);
ret->rName = cloneString(row[2]);
ret->pos = sqlUnsigned(row[3]);
ret->mapQ = sqlUnsigned(row[4]);
ret->cigar = cloneString(row[5]);
ret->rNext = cloneString(row[6]);
ret->pNext = sqlSigned(row[7]);
ret->tLen = sqlSigned(row[8]);
ret->seq = cloneString(row[9]);
ret->qual = cloneString(row[10]);
ret->tagTypeVals = cloneString(row[11]);
return ret;
}

struct samAlignment *samAlignmentLoadAll(char *fileName)
/* Load all samAlignment from a whitespace-separated file.
 * Dispose of this with samAlignmentFreeList(). */
{
struct samAlignment *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

while (lineFileRow(lf, row))
    {
    el = samAlignmentLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct samAlignment *samAlignmentLoadAllByChar(char *fileName, char chopper)
/* Load all samAlignment from a chopper separated file.
 * Dispose of this with samAlignmentFreeList(). */
{
struct samAlignment *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[12];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = samAlignmentLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct samAlignment *samAlignmentCommaIn(char **pS, struct samAlignment *ret)
/* Create a samAlignment out of a comma separated string.
 * This will fill in ret if non-null, otherwise will
 * return a new samAlignment */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->qName = sqlStringComma(&s);
ret->flag = sqlUnsignedComma(&s);
ret->rName = sqlStringComma(&s);
ret->pos = sqlUnsignedComma(&s);
ret->mapQ = sqlUnsignedComma(&s);
ret->cigar = sqlStringComma(&s);
ret->rNext = sqlStringComma(&s);
ret->pNext = sqlSignedComma(&s);
ret->tLen = sqlSignedComma(&s);
ret->seq = sqlStringComma(&s);
ret->qual = sqlStringComma(&s);
ret->tagTypeVals = sqlStringComma(&s);
*pS = s;
return ret;
}

void samAlignmentFree(struct samAlignment **pEl)
/* Free a single dynamically allocated samAlignment such as created
 * with samAlignmentLoad(). */
{
struct samAlignment *el;

if ((el = *pEl) == NULL) return;
freeMem(el->qName);
freeMem(el->rName);
freeMem(el->cigar);
freeMem(el->rNext);
freeMem(el->seq);
freeMem(el->qual);
freeMem(el->tagTypeVals);
freez(pEl);
}

void samAlignmentFreeList(struct samAlignment **pList)
/* Free a list of dynamically allocated samAlignment's */
{
struct samAlignment *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    samAlignmentFree(&el);
    }
*pList = NULL;
}

void samAlignmentOutput(struct samAlignment *el, FILE *f, char sep, char lastSep)
/* Print out samAlignment.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->flag);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rName);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->pos);
fputc(sep,f);
fprintf(f, "%u", el->mapQ);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->cigar);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->rNext);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->pNext);
fputc(sep,f);
fprintf(f, "%d", el->tLen);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->seq);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->qual);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->tagTypeVals);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */
static char *samAlignmentAutoSqlString =
"table samAlignment\n"
"\"The fields of a SAM short read alignment, the text version of BAM.\"\n"
    "(\n"
    "string qName;	\"Query template name - name of a read\"\n"
    "ushort flag;	\"Flags.  0x10 set for reverse complement.  See SAM docs for others.\"\n"
    "string rName;	\"Reference sequence name (often a chromosome)\"\n"
    "uint pos;		\"1 based position\"\n"
    "ubyte mapQ;		\"Mapping quality 0-255, 255 is best\"\n"
    "string cigar;	\"CIGAR encoded alignment string.\"\n"
    "string rNext;	\"Ref sequence for next (mate) read. '=' if same as rName, '*' if no mate\"\n"
    "int pNext;		\"Position (1-based) of next (mate) sequence. May be -1 or 0 if no mate\"\n"
    "int tLen;	        \"Size of DNA template for mated pairs.  -size for one of mate pairs\"\n"
    "string seq;		\"Query template sequence\"\n"
    "string qual;	\"ASCII of Phred-scaled base QUALity+33.  Just '*' if no quality scores\"\n"
    "string tagTypeVals; \"Tab-delimited list of tag:type:value optional extra fields\"\n"
    ")\n";

struct asObject *samAsObj()
// Return asObject describing fields of SAM/BAM
{
return asParseText(samAlignmentAutoSqlString);
}

