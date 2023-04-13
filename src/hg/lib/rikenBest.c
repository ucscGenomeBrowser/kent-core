/* rikenBest.c was originally generated by the autoSql program, which also 
 * generated rikenBest.h and rikenBest.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "rikenBest.h"


void rikenBestStaticLoad(char **row, struct rikenBest *ret)
/* Load a row from rikenBest table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->name = row[0];
ret->orfScore = atof(row[1]);
strcpy(ret->orfStrand, row[2]);
ret->intronOrientation = sqlSigned(row[3]);
ret->position = row[4];
ret->rikenCount = sqlSigned(row[5]);
ret->genBankCount = sqlSigned(row[6]);
ret->refSeqCount = sqlSigned(row[7]);
ret->clusterId = row[8];
}

struct rikenBest *rikenBestLoad(char **row)
/* Load a rikenBest from row fetched with select * from rikenBest
 * from database.  Dispose of this with rikenBestFree(). */
{
struct rikenBest *ret;

AllocVar(ret);
ret->name = cloneString(row[0]);
ret->orfScore = atof(row[1]);
strcpy(ret->orfStrand, row[2]);
ret->intronOrientation = sqlSigned(row[3]);
ret->position = cloneString(row[4]);
ret->rikenCount = sqlSigned(row[5]);
ret->genBankCount = sqlSigned(row[6]);
ret->refSeqCount = sqlSigned(row[7]);
ret->clusterId = cloneString(row[8]);
return ret;
}

struct rikenBest *rikenBestLoadAll(char *fileName) 
/* Load all rikenBest from a tab-separated file.
 * Dispose of this with rikenBestFreeList(). */
{
struct rikenBest *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[9];

while (lineFileRow(lf, row))
    {
    el = rikenBestLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct rikenBest *rikenBestLoadWhere(struct sqlConnection *conn, char *table, char *where)
/* Load all rikenBest from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with rikenBestFreeList(). */
{
struct rikenBest *list = NULL, *el;
struct dyString *query = dyStringNew(256);
struct sqlResult *sr;
char **row;

sqlDyStringPrintf(query, "select * from %s", table);
if (where != NULL)
    sqlDyStringPrintf(query, " where %-s", where);
sr = sqlGetResult(conn, query->string);
while ((row = sqlNextRow(sr)) != NULL)
    {
    el = rikenBestLoad(row);
    slAddHead(&list, el);
    }
slReverse(&list);
sqlFreeResult(&sr);
dyStringFree(&query);
return list;
}

struct rikenBest *rikenBestCommaIn(char **pS, struct rikenBest *ret)
/* Create a rikenBest out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new rikenBest */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->name = sqlStringComma(&s);
ret->orfScore = sqlFloatComma(&s);
sqlFixedStringComma(&s, ret->orfStrand, sizeof(ret->orfStrand));
ret->intronOrientation = sqlSignedComma(&s);
ret->position = sqlStringComma(&s);
ret->rikenCount = sqlSignedComma(&s);
ret->genBankCount = sqlSignedComma(&s);
ret->refSeqCount = sqlSignedComma(&s);
ret->clusterId = sqlStringComma(&s);
*pS = s;
return ret;
}

void rikenBestFree(struct rikenBest **pEl)
/* Free a single dynamically allocated rikenBest such as created
 * with rikenBestLoad(). */
{
struct rikenBest *el;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->position);
freeMem(el->clusterId);
freez(pEl);
}

void rikenBestFreeList(struct rikenBest **pList)
/* Free a list of dynamically allocated rikenBest's */
{
struct rikenBest *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    rikenBestFree(&el);
    }
*pList = NULL;
}

void rikenBestOutput(struct rikenBest *el, FILE *f, char sep, char lastSep) 
/* Print out rikenBest.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->name);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%f", el->orfScore);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->orfStrand);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->intronOrientation);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->position);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%d", el->rikenCount);
fputc(sep,f);
fprintf(f, "%d", el->genBankCount);
fputc(sep,f);
fprintf(f, "%d", el->refSeqCount);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->clusterId);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

