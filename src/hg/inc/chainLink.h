/* chainLink.h was originally generated by the autoSql program, which also 
 * generated chainLink.c and chainLink.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2002 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef CHAINLINK_H
#define CHAINLINK_H

#ifndef CHAIN_H
#include "chain.h"
#endif

struct chainLink
/* alignment block in chain */
    {
    struct chainLink *next;  /* Next in singly linked list. */
    char *tName;	/* Target sequence name */
    unsigned tStart;	/* Alignment start position in target */
    unsigned tEnd;	/* Alignment end position in target */
    unsigned qStart;	/* start in query */
    unsigned chainId;	/* chain id in chain table */
    };

void chainLinkStaticLoad(char **row, struct chainLink *ret);
/* Load a row from chainLink table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct chainLink *chainLinkLoad(char **row);
/* Load a chainLink from row fetched with select * from chainLink
 * from database.  Dispose of this with chainLinkFree(). */

struct chainLink *chainLinkLoadAll(char *fileName);
/* Load all chainLink from a tab-separated file.
 * Dispose of this with chainLinkFreeList(). */

struct chainLink *chainLinkCommaIn(char **pS, struct chainLink *ret);
/* Create a chainLink out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new chainLink */

void chainLinkFree(struct chainLink **pEl);
/* Free a single dynamically allocated chainLink such as created
 * with chainLinkLoad(). */

void chainLinkFreeList(struct chainLink **pList);
/* Free a list of dynamically allocated chainLink's */

void chainLinkOutput(struct chainLink *el, FILE *f, char sep, char lastSep);
/* Print out chainLink.  Separate fields with sep. Follow last field with lastSep. */

#define chainLinkTabOut(el,f) chainLinkOutput(el,f,'\t','\n');
/* Print out chainLink as a line in a tab-separated file. */

#define chainLinkCommaOut(el,f) chainLinkOutput(el,f,',',',');
/* Print out chainLink as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* CHAINLINK_H */

