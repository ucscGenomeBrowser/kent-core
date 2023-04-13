/* pgPhenoAssoc.h was originally generated by the autoSql program, which also 
 * generated pgPhenoAssoc.c and pgPhenoAssoc.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2010 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef PGPHENOASSOC_H
#define PGPHENOASSOC_H

#define PGPHENOASSOC_NUM_COLS 5

struct pgPhenoAssoc
/* phenotypes from various databases for pgSnp tracks */
    {
    struct pgPhenoAssoc *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Phenotype */
    char *srcUrl;	/* link back to source database */
    };

void pgPhenoAssocStaticLoad(char **row, struct pgPhenoAssoc *ret);
/* Load a row from pgPhenoAssoc table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct pgPhenoAssoc *pgPhenoAssocLoad(char **row);
/* Load a pgPhenoAssoc from row fetched with select * from pgPhenoAssoc
 * from database.  Dispose of this with pgPhenoAssocFree(). */

struct pgPhenoAssoc *pgPhenoAssocLoadAll(char *fileName);
/* Load all pgPhenoAssoc from whitespace-separated file.
 * Dispose of this with pgPhenoAssocFreeList(). */

struct pgPhenoAssoc *pgPhenoAssocLoadAllByChar(char *fileName, char chopper);
/* Load all pgPhenoAssoc from chopper separated file.
 * Dispose of this with pgPhenoAssocFreeList(). */

#define pgPhenoAssocLoadAllByTab(a) pgPhenoAssocLoadAllByChar(a, '\t');
/* Load all pgPhenoAssoc from tab separated file.
 * Dispose of this with pgPhenoAssocFreeList(). */

struct pgPhenoAssoc *pgPhenoAssocCommaIn(char **pS, struct pgPhenoAssoc *ret);
/* Create a pgPhenoAssoc out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new pgPhenoAssoc */

void pgPhenoAssocFree(struct pgPhenoAssoc **pEl);
/* Free a single dynamically allocated pgPhenoAssoc such as created
 * with pgPhenoAssocLoad(). */

void pgPhenoAssocFreeList(struct pgPhenoAssoc **pList);
/* Free a list of dynamically allocated pgPhenoAssoc's */

void pgPhenoAssocOutput(struct pgPhenoAssoc *el, FILE *f, char sep, char lastSep);
/* Print out pgPhenoAssoc.  Separate fields with sep. Follow last field with lastSep. */

#define pgPhenoAssocTabOut(el,f) pgPhenoAssocOutput(el,f,'\t','\n');
/* Print out pgPhenoAssoc as a line in a tab-separated file. */

#define pgPhenoAssocCommaOut(el,f) pgPhenoAssocOutput(el,f,',',',');
/* Print out pgPhenoAssoc as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* PGPHENOASSOC_H */

