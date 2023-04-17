/* vntr.h was originally generated by the autoSql program, which also 
 * generated vntr.c and vntr.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2004 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef VNTR_H
#define VNTR_H

#define VNTR_NUM_COLS 10

struct vntr
/* Microsatellites from Gerome Breen's VNTR program (bed 4+) */
    {
    struct vntr *next;  /* Next in singly linked list. */
    char *chrom;	/* chrom */
    unsigned chromStart;	/* Start position in chromosome */
    unsigned chromEnd;	/* End position in chromosome */
    char *name;	/* Name of item (Repeat unit) */
    float repeatCount;	/* Number of perfect repeats */
    int distanceToLast;	/* Distance to previous microsat. repeat */
    int distanceToNext;	/* Distance to next microsat. repeat */
    char *forwardPrimer;	/* Forward PCR primer sequence (or Design_Failed) */
    char *reversePrimer;	/* Reverse PCR primer sequence (or Design_Failed) */
    char *pcrLength;	/* PCR product length (or Design_Failed) */
    };

void vntrStaticLoad(char **row, struct vntr *ret);
/* Load a row from vntr table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct vntr *vntrLoad(char **row);
/* Load a vntr from row fetched with select * from vntr
 * from database.  Dispose of this with vntrFree(). */

struct vntr *vntrLoadAll(char *fileName);
/* Load all vntr from whitespace-separated file.
 * Dispose of this with vntrFreeList(). */

struct vntr *vntrLoadAllByChar(char *fileName, char chopper);
/* Load all vntr from chopper separated file.
 * Dispose of this with vntrFreeList(). */

#define vntrLoadAllByTab(a) vntrLoadAllByChar(a, '\t');
/* Load all vntr from tab separated file.
 * Dispose of this with vntrFreeList(). */

struct vntr *vntrCommaIn(char **pS, struct vntr *ret);
/* Create a vntr out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new vntr */

void vntrFree(struct vntr **pEl);
/* Free a single dynamically allocated vntr such as created
 * with vntrLoad(). */

void vntrFreeList(struct vntr **pList);
/* Free a list of dynamically allocated vntr's */

void vntrOutput(struct vntr *el, FILE *f, char sep, char lastSep);
/* Print out vntr.  Separate fields with sep. Follow last field with lastSep. */

#define vntrTabOut(el,f) vntrOutput(el,f,'\t','\n');
/* Print out vntr as a line in a tab-separated file. */

#define vntrCommaOut(el,f) vntrOutput(el,f,',',',');
/* Print out vntr as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* VNTR_H */
