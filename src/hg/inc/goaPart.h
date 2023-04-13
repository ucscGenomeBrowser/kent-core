/* goaPart.h was originally generated by the autoSql program, which also 
 * generated goaPart.c and goaPart.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2003 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef GOAPART_H
#define GOAPART_H

#define GOAPART_NUM_COLS 5

struct goaPart
/* Partial GO Association. Useful subset of goa */
    {
    struct goaPart *next;  /* Next in singly linked list. */
    char *dbObjectId;	/* Database accession - like 'Q13448' */
    char *dbObjectSymbol;	/* Name - like 'CIA1_HUMAN' */
    char *notId;	/* (Optional) If 'NOT'. Indicates object isn't goId */
    char *goId;	/* GO ID - like 'GO:0015888' */
    char *aspect;	/*  P (process), F (function) or C (cellular component) */
    };

void goaPartStaticLoad(char **row, struct goaPart *ret);
/* Load a row from goaPart table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct goaPart *goaPartLoad(char **row);
/* Load a goaPart from row fetched with select * from goaPart
 * from database.  Dispose of this with goaPartFree(). */

struct goaPart *goaPartLoadAll(char *fileName);
/* Load all goaPart from whitespace-separated file.
 * Dispose of this with goaPartFreeList(). */

struct goaPart *goaPartLoadAllByChar(char *fileName, char chopper);
/* Load all goaPart from chopper separated file.
 * Dispose of this with goaPartFreeList(). */

#define goaPartLoadAllByTab(a) goaPartLoadAllByChar(a, '\t');
/* Load all goaPart from tab separated file.
 * Dispose of this with goaPartFreeList(). */

struct goaPart *goaPartCommaIn(char **pS, struct goaPart *ret);
/* Create a goaPart out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new goaPart */

void goaPartFree(struct goaPart **pEl);
/* Free a single dynamically allocated goaPart such as created
 * with goaPartLoad(). */

void goaPartFreeList(struct goaPart **pList);
/* Free a list of dynamically allocated goaPart's */

void goaPartOutput(struct goaPart *el, FILE *f, char sep, char lastSep);
/* Print out goaPart.  Separate fields with sep. Follow last field with lastSep. */

#define goaPartTabOut(el,f) goaPartOutput(el,f,'\t','\n');
/* Print out goaPart as a line in a tab-separated file. */

#define goaPartCommaOut(el,f) goaPartOutput(el,f,',',',');
/* Print out goaPart as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* GOAPART_H */

