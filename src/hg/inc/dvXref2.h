/* dvXref2.h was originally generated by the autoSql program, which also 
 * generated dvXref2.c and dvXref2.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2005 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef DVXREF2_H
#define DVXREF2_H

#define DVXREF2_NUM_COLS 3

struct dvXref2
/* A xref table between SWISS-PROT variant ids and OMIM. */
    {
    struct dvXref2 *next;  /* Next in singly linked list. */
    char *varId;	/* SWISS-PROT variant */
    char *extSrc;	/* external source (MIM for example) */
    char *extAcc;	/* external accession */
    };

void dvXref2StaticLoad(char **row, struct dvXref2 *ret);
/* Load a row from dvXref2 table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct dvXref2 *dvXref2Load(char **row);
/* Load a dvXref2 from row fetched with select * from dvXref2
 * from database.  Dispose of this with dvXref2Free(). */

struct dvXref2 *dvXref2LoadAll(char *fileName);
/* Load all dvXref2 from whitespace-separated file.
 * Dispose of this with dvXref2FreeList(). */

struct dvXref2 *dvXref2LoadAllByChar(char *fileName, char chopper);
/* Load all dvXref2 from chopper separated file.
 * Dispose of this with dvXref2FreeList(). */

#define dvXref2LoadAllByTab(a) dvXref2LoadAllByChar(a, '\t');
/* Load all dvXref2 from tab separated file.
 * Dispose of this with dvXref2FreeList(). */

struct dvXref2 *dvXref2CommaIn(char **pS, struct dvXref2 *ret);
/* Create a dvXref2 out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dvXref2 */

void dvXref2Free(struct dvXref2 **pEl);
/* Free a single dynamically allocated dvXref2 such as created
 * with dvXref2Load(). */

void dvXref2FreeList(struct dvXref2 **pList);
/* Free a list of dynamically allocated dvXref2's */

void dvXref2Output(struct dvXref2 *el, FILE *f, char sep, char lastSep);
/* Print out dvXref2.  Separate fields with sep. Follow last field with lastSep. */

#define dvXref2TabOut(el,f) dvXref2Output(el,f,'\t','\n');
/* Print out dvXref2 as a line in a tab-separated file. */

#define dvXref2CommaOut(el,f) dvXref2Output(el,f,',',',');
/* Print out dvXref2 as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* DVXREF2_H */

