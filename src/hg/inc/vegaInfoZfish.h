/* vegaInfoZfish.h was originally generated by the autoSql program, which also 
 * generated vegaInfoZfish.c and vegaInfoZfish.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef VEGAINFOZFISH_H
#define VEGAINFOZFISH_H

#define VEGAINFOZFISH_NUM_COLS 8

struct vegaInfoZfish
/* Vega Genes track additional information */
    {
    struct vegaInfoZfish *next;  /* Next in singly linked list. */
    char *transcriptId;	/* Vega transcript ID */
    char *geneId;	/* Vega gene ID (OTTER ID) */
    char *sangerName;	/* Sanger gene name */
    char *zfinId;	/* ZFIN ID */
    char *zfinSymbol;	/* ZFIN gene symbol */
    char *method;	/* GTF method field */
    char *geneDesc;	/* Vega gene description */
    char *confidence;	/* Status (KNOWN, NOVEL, PUTATIVE, PREDICTED) */
    };

void vegaInfoZfishStaticLoad(char **row, struct vegaInfoZfish *ret);
/* Load a row from vegaInfoZfish table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct vegaInfoZfish *vegaInfoZfishLoad(char **row);
/* Load a vegaInfoZfish from row fetched with select * from vegaInfoZfish
 * from database.  Dispose of this with vegaInfoZfishFree(). */

struct vegaInfoZfish *vegaInfoZfishLoadAll(char *fileName);
/* Load all vegaInfoZfish from whitespace-separated file.
 * Dispose of this with vegaInfoZfishFreeList(). */

struct vegaInfoZfish *vegaInfoZfishLoadAllByChar(char *fileName, char chopper);
/* Load all vegaInfoZfish from chopper separated file.
 * Dispose of this with vegaInfoZfishFreeList(). */

#define vegaInfoZfishLoadAllByTab(a) vegaInfoZfishLoadAllByChar(a, '\t');
/* Load all vegaInfoZfish from tab separated file.
 * Dispose of this with vegaInfoZfishFreeList(). */

struct vegaInfoZfish *vegaInfoZfishCommaIn(char **pS, struct vegaInfoZfish *ret);
/* Create a vegaInfoZfish out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new vegaInfoZfish */

void vegaInfoZfishFree(struct vegaInfoZfish **pEl);
/* Free a single dynamically allocated vegaInfoZfish such as created
 * with vegaInfoZfishLoad(). */

void vegaInfoZfishFreeList(struct vegaInfoZfish **pList);
/* Free a list of dynamically allocated vegaInfoZfish's */

void vegaInfoZfishOutput(struct vegaInfoZfish *el, FILE *f, char sep, char lastSep);
/* Print out vegaInfoZfish.  Separate fields with sep. Follow last field with lastSep. */

#define vegaInfoZfishTabOut(el,f) vegaInfoZfishOutput(el,f,'\t','\n');
/* Print out vegaInfoZfish as a line in a tab-separated file. */

#define vegaInfoZfishCommaOut(el,f) vegaInfoZfishOutput(el,f,',',',');
/* Print out vegaInfoZfish as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* VEGAINFOZFISH_H */
