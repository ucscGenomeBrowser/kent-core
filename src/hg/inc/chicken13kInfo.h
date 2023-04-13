/* chicken13kInfo.h was originally generated by the autoSql program, which also 
 * generated chicken13kInfo.c and chicken13kInfo.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef CHICKEN13KINFO_H
#define CHICKEN13KINFO_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define CHICKEN13KINFO_NUM_COLS 12

struct chicken13kInfo
/* Extra information for the chicken 13k probes. */
    {
    struct chicken13kInfo *next;  /* Next in singly linked list. */
    char *id;	/* ID of the probe */
    char *source;	/* Source of the probe */
    char pcr[2];	/* PCR Amp Code */
    char *library;	/* Library */
    char *clone;	/* Source Clone Name */
    char *gbkAcc;	/* Genbank accession */
    char *blat;	/* BLAT alignments */
    char *sourceAnnot;	/* Source assigned annotation */
    char *tigrTc;	/* TIGR assigned TC */
    char *tigrTcAnnot;	/* TIGR TC annotation */
    char *blastAnnot;	/* BLAST determined annotation */
    char *comment;	/* Comment */
    };

void chicken13kInfoStaticLoad(char **row, struct chicken13kInfo *ret);
/* Load a row from chicken13kInfo table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct chicken13kInfo *chicken13kInfoLoad(char **row);
/* Load a chicken13kInfo from row fetched with select * from chicken13kInfo
 * from database.  Dispose of this with chicken13kInfoFree(). */

struct chicken13kInfo *chicken13kInfoLoadAll(char *fileName);
/* Load all chicken13kInfo from whitespace-separated file.
 * Dispose of this with chicken13kInfoFreeList(). */

struct chicken13kInfo *chicken13kInfoLoadAllByChar(char *fileName, char chopper);
/* Load all chicken13kInfo from chopper separated file.
 * Dispose of this with chicken13kInfoFreeList(). */

#define chicken13kInfoLoadAllByTab(a) chicken13kInfoLoadAllByChar(a, '\t');
/* Load all chicken13kInfo from tab separated file.
 * Dispose of this with chicken13kInfoFreeList(). */

struct chicken13kInfo *chicken13kInfoLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all chicken13kInfo from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with chicken13kInfoFreeList(). */

void chicken13kInfoSaveToDb(struct sqlConnection *conn, struct chicken13kInfo *el, char *tableName, int updateSize);
/* Save chicken13kInfo as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */

struct chicken13kInfo *chicken13kInfoCommaIn(char **pS, struct chicken13kInfo *ret);
/* Create a chicken13kInfo out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new chicken13kInfo */

void chicken13kInfoFree(struct chicken13kInfo **pEl);
/* Free a single dynamically allocated chicken13kInfo such as created
 * with chicken13kInfoLoad(). */

void chicken13kInfoFreeList(struct chicken13kInfo **pList);
/* Free a list of dynamically allocated chicken13kInfo's */

void chicken13kInfoOutput(struct chicken13kInfo *el, FILE *f, char sep, char lastSep);
/* Print out chicken13kInfo.  Separate fields with sep. Follow last field with lastSep. */

#define chicken13kInfoTabOut(el,f) chicken13kInfoOutput(el,f,'\t','\n');
/* Print out chicken13kInfo as a line in a tab-separated file. */

#define chicken13kInfoCommaOut(el,f) chicken13kInfoOutput(el,f,',',',');
/* Print out chicken13kInfo as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CHICKEN13KINFO_H */

