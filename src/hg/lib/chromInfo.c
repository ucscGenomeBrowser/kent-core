/* chromInfo.c was originally generated by the autoSql program, which also 
 * generated chromInfo.h and chromInfo.sql.  This module links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "jksql.h"
#include "hdb.h"
#include "chromInfo.h"
#include "net.h"

void chromInfoStaticLoad(char **row, struct chromInfo *ret)
/* Load a row from chromInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */
{

ret->chrom = row[0];
ret->size = sqlUnsigned(row[1]);
ret->fileName = row[2];
}

struct chromInfo *chromInfoLoad(char **row)
/* Load a chromInfo from row fetched with select * from chromInfo
 * from database.  Dispose of this with chromInfoFree(). */
{
struct chromInfo *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->size = sqlUnsigned(row[1]);
ret->fileName = cloneString(row[2]);
return ret;
}

struct chromInfo *chromInfoLoadAll(char *fileName) 
/* Load all chromInfo from a whitespace-separated file.
 * Dispose of this with chromInfoFreeList(). */
{
struct chromInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileRow(lf, row))
    {
    el = chromInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct chromInfo *chromInfoLoadAllByChar(char *fileName, char chopper) 
/* Load all chromInfo from a chopper separated file.
 * Dispose of this with chromInfoFreeList(). */
{
struct chromInfo *list = NULL, *el;
struct lineFile *lf = lineFileOpen(fileName, TRUE);
char *row[3];

while (lineFileNextCharRow(lf, chopper, row, ArraySize(row)))
    {
    el = chromInfoLoad(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}

struct chromInfo *chromInfoCommaIn(char **pS, struct chromInfo *ret)
/* Create a chromInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new chromInfo */
{
char *s = *pS;

if (ret == NULL)
    AllocVar(ret);
ret->chrom = sqlStringComma(&s);
ret->size = sqlUnsignedComma(&s);
ret->fileName = sqlStringComma(&s);
*pS = s;
return ret;
}

void chromInfoFree(struct chromInfo **pEl)
/* Free a single dynamically allocated chromInfo such as created
 * with chromInfoLoad(). */
{
struct chromInfo *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freeMem(el->fileName);
freez(pEl);
}

void chromInfoFreeList(struct chromInfo **pList)
/* Free a list of dynamically allocated chromInfo's */
{
struct chromInfo *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    chromInfoFree(&el);
    }
*pList = NULL;
}

void chromInfoOutput(struct chromInfo *el, FILE *f, char sep, char lastSep) 
/* Print out chromInfo.  Separate fields with sep. Follow last field with lastSep. */
{
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->chrom);
if (sep == ',') fputc('"',f);
fputc(sep,f);
fprintf(f, "%u", el->size);
fputc(sep,f);
if (sep == ',') fputc('"',f);
fprintf(f, "%s", el->fileName);
if (sep == ',') fputc('"',f);
fputc(lastSep,f);
}

/* -------------------------------- End autoSql Generated Code -------------------------------- */

boolean chromSeqFileExists(char *db, char *chrom)
/* Check whether chromInfo exists for a database, find the path of the */
/* sequence file for this chromosome and check if the file exists. */
{
char seqFile[512];
struct sqlConnection *conn = sqlConnect(db);
char query[256];
char *res = NULL;
boolean exists = FALSE;

/* if the database exists (which it must since we opened the connection above), check for the chromInfo table */
if (sqlDatabaseExists(db) && sqlTableExists(conn, "chromInfo"))
    {
    /* the database and chromInfo table exist, look to see if it has our chrom. */
    sqlSafef(query, sizeof(query), "select fileName from chromInfo where chrom = '%s'", chrom);
    res = sqlQuickQuery(conn, query, seqFile, 512);
    sqlDisconnect(&conn);
    }

/* if there is not table or no information in the table or if the table */
/* exists but the file can not be opened return false, otherwise sequence */
/* file exists and return true */
if (res != NULL)
    {
    char *seqFile2 = hReplaceGbdb(seqFile);
    exists = udcExists(seqFile2);
    freeMem(seqFile2);
    }
return exists;
}


struct chromInfo *createChromInfoList(char *name, char *database)
/* Load up chromosome information for chrom 'name'.
 * If name is NULL or "all" then load all chroms.
 * Similar to featureBits.c - could be moved to library */
{
struct sqlConnection *conn = hAllocConn(database);
struct sqlResult *sr = NULL;
char **row;
int loaded=0;
struct chromInfo *ret = NULL;
unsigned totalSize = 0;
/* do the query */
if (!name || sameWord(name, "all"))
    {
    char query[1024];
    sqlSafef(query, sizeof query, "select * from chromInfo");
    sr = sqlGetResult(conn, query);
    }
else
    {
    char select[256];
    sqlSafef(select, ArraySize(select), "select * from chromInfo where chrom='%s'", name);
    sr = sqlGetResult(conn, select);
    }
/* read the rows and build the chromInfo list */
while ((row = sqlNextRow(sr)) != NULL)
    {
    struct chromInfo *el;
    struct chromInfo *ci;
    AllocVar(ci);
    el = chromInfoLoad(row);
    ci->chrom = cloneString(el->chrom);
    ci->size = el->size;
    totalSize += el->size;
    slAddHead(&ret, ci);
    ++loaded;
    }
if (loaded < 1)
    errAbort("ERROR: can not find chrom name: '%s'\n", name ? name : "NULL");
slReverse(&ret);
if (!name || sameWord(name, "all"))
    verbose(2, "#\tloaded size info for %d chroms, total size: %u\n",
        loaded, totalSize);
sqlFreeResult(&sr);
hFreeConn(&conn);
return ret;
}

struct hash *chromNameAndSizeHashFromList(struct chromInfo *ci)
/* Return a hash table of chrom key=name, val=size */
{
unsigned *size;
struct hash *h = newHash(0);
for ( ; ci ; ci = ci->next )
    {
    AllocVar(size);
    *size = ci->size;
    hashAdd(h, ci->chrom, size);
    }
return h;
}

struct hash *chromHashFromDatabase(char *db) 
/* read chrom info from database and return hash of name and size */
{
struct hash *chromSizeHash = NULL;
struct chromInfo *list = NULL;
list = createChromInfoList(NULL, db);
if (!list)
    errAbort("could not load chromInfo from DB %s\n", db);
chromSizeHash = chromNameAndSizeHashFromList(list);
chromInfoFree(&list);
return chromSizeHash;
}

struct chromInfo *chromInfoAddOneFromFile(char **row)
/* Load a chromInfo from row fetched from file.
 * Dispose of this with chromInfoFree(). */
{
struct chromInfo *ret;

AllocVar(ret);
ret->chrom = cloneString(row[0]);
ret->size = sqlUnsigned(row[1]);
ret->fileName = NULL;  // note only name and size supported
return ret;
}

struct chromInfo *chromInfoListFromFile(char *fileName) 
/* read chrom info from file and return list of name and size */
{
struct chromInfo *list = NULL, *el;
struct lineFile *lf = NULL;
if (udcIsLocal(fileName))
    lf = lineFileOpen(fileName, TRUE);
else
    lf = netLineFileOpen(fileName);
char *row[2];

while (lineFileRow(lf, row))
    {
    el = chromInfoAddOneFromFile(row);
    slAddHead(&list, el);
    }
lineFileClose(&lf);
slReverse(&list);
return list;
}


struct hash *chromHashFromFile(char *fileName) 
/* read chrom info from file and return hash of name and size */
{
struct hash *chromSizeHash = NULL;
struct chromInfo *list = NULL;
list = chromInfoListFromFile(fileName);
if (!list)
    errAbort("could not load chromInfo file %s\n", fileName);
chromSizeHash = chromNameAndSizeHashFromList(list);
chromInfoFree(&list);
return chromSizeHash;
}
