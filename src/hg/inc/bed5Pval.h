/* bed5Pval.h was originally generated by the autoSql program, which also 
 * generated bed5Pval.c and bed5Pval.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef BED5PVAL_H
#define BED5PVAL_H

#define BED5PVAL_NUM_COLS 6

struct bed5Pval
/* Scored BED with P-value floating point values and integer scores (0-1000) for display. */
    {
    struct bed5Pval *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item */
    int score;	/* 0-1000 score for useScore shading */
    float pValue;	/* -log10 P-value */
    };

void bed5PvalStaticLoad(char **row, struct bed5Pval *ret);
/* Load a row from bed5Pval table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct bed5Pval *bed5PvalLoad(char **row);
/* Load a bed5Pval from row fetched with select * from bed5Pval
 * from database.  Dispose of this with bed5PvalFree(). */

struct bed5Pval *bed5PvalLoadAll(char *fileName);
/* Load all bed5Pval from whitespace-separated file.
 * Dispose of this with bed5PvalFreeList(). */

struct bed5Pval *bed5PvalLoadAllByChar(char *fileName, char chopper);
/* Load all bed5Pval from chopper separated file.
 * Dispose of this with bed5PvalFreeList(). */

#define bed5PvalLoadAllByTab(a) bed5PvalLoadAllByChar(a, '\t');
/* Load all bed5Pval from tab separated file.
 * Dispose of this with bed5PvalFreeList(). */

struct bed5Pval *bed5PvalCommaIn(char **pS, struct bed5Pval *ret);
/* Create a bed5Pval out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bed5Pval */

void bed5PvalFree(struct bed5Pval **pEl);
/* Free a single dynamically allocated bed5Pval such as created
 * with bed5PvalLoad(). */

void bed5PvalFreeList(struct bed5Pval **pList);
/* Free a list of dynamically allocated bed5Pval's */

void bed5PvalOutput(struct bed5Pval *el, FILE *f, char sep, char lastSep);
/* Print out bed5Pval.  Separate fields with sep. Follow last field with lastSep. */

#define bed5PvalTabOut(el,f) bed5PvalOutput(el,f,'\t','\n');
/* Print out bed5Pval as a line in a tab-separated file. */

#define bed5PvalCommaOut(el,f) bed5PvalOutput(el,f,',',',');
/* Print out bed5Pval as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* BED5PVAL_H */

