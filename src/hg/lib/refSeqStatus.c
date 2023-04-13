/* refSeqStatus.c was originally generated by the autoSql program, which also 
 * generated refSeqStatus.h and refSeqStatus.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "refSeqStatus.h"


/* definitions for status column */
static char *values_status[] = {"Unknown", "Reviewed", "Validated", "Provisional", "Predicted", "Inferred", NULL};
static struct hash *valhash_status = NULL;

/* definitions for mol column */
static char *values_mol[] = {"DNA", "RNA", "ds-RNA", "ds-mRNA", "ds-rRNA", "mRNA", "ms-DNA", "ms-RNA", "rRNA", "scRNA", "snRNA", "snoRNA", "ss-DNA", "ss-RNA", "ss-snoRNA", "tRNA", NULL};
static struct hash *valhash_mol = NULL;

void refSeqStatusStaticLoad(char **row, struct refSeqStatus *ret)
/* Load a row from refSeqStatus table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->mrnaAcc = row[0];
ret->status = sqlEnumParse(row[1], values_status, &valhash_status);
ret->mol = sqlEnumParse(row[2], values_mol, &valhash_mol);
}

struct refSeqStatus *refSeqStatusLoad(char **row)
/* Load a refSeqStatus from row fetched with select * from refSeqStatus
 * from database.  Dispose of this with refSeqStatusFree(). */
{
struct refSeqStatus *ret;

AllocVar(ret);
ret->mrnaAcc = cloneString(row[0]);
ret->status = sqlEnumParse(row[1], values_status, &valhash_status);
ret->mol = sqlEnumParse(row[2], values_mol, &valhash_mol);
return ret;
}

struct refSeqStatus *refSeqStatusLoadAll(char *fileName) 
/* Load all refSeqStatus from a whitespace-separated file.
 * Dispose of this with refSeqStatusFreeList(). */
{
struct refSeqStatus *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = refSeqStatusLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct refSeqStatus *refSeqStatusLoadAllByChar(char *fileName, char chopper) 
/* Load all refSeqStatus from a chopper separated file.
 * Dispose of this with refSeqStatusFreeList(). */
{
struct refSeqStatus *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = refSeqStatusLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct refSeqStatus *refSeqStatusCommaIn(char **pS, struct refSeqStatus *ret)
/* Create a refSeqStatus out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new refSeqStatus */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->mrnaAcc = sqlStringComma(&s);
ret->status = sqlEnumComma(&s, values_status, &valhash_status);
ret->mol = sqlEnumComma(&s, values_mol, &valhash_mol);
*pS = s;
return ret;
}

void refSeqStatusFree(struct refSeqStatus **pEl)
/* Free a single dynamically allocated refSeqStatus such as created
 * with refSeqStatusLoad(). */
{
struct refSeqStatus *el;

if ((el = *pEl) == NULL) return;
freeMem(el->mrnaAcc);
freez(pEl);
}

void refSeqStatusFreeList(struct refSeqStatus **pList)
/* Free a list of dynamically allocated refSeqStatus's */
{
struct refSeqStatus *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    refSeqStatusFree(&el);
    }
*pList = NULL;
}

void refSeqStatusOutput(struct refSeqStatus *el, FILE *f, char sep, char lastSep) 
/* Print out refSeqStatus.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->mrnaAcc);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
sqlEnumPrint(f, el->status, values_status);
if (sep == ',') fputc('"',f);
fputc(sep,f);
if (sep == ',') fputc('"',f);
sqlEnumPrint(f, el->mol, values_mol);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

