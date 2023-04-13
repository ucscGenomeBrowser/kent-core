/* wgEncodeGencodeGeneSource.h was originally generated by the autoSql program, which also 
 * generated wgEncodeGencodeGeneSource.c and wgEncodeGencodeGeneSource.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef WGENCODEGENCODEGENESOURCE_H
#define WGENCODEGENCODEGENESOURCE_H

#define WGENCODEGENCODEGENESOURCE_NUM_COLS 2

struct wgEncodeGencodeGeneSource
/* The source of Gencode gene annotation */
    {
    struct wgEncodeGencodeGeneSource *next;  /* Next in singly linked list. */
    char *geneId;	/* GENCODE gene identifier */
    char *source;	/* Source of gene */
    };

void wgEncodeGencodeGeneSourceStaticLoad(char **row, struct wgEncodeGencodeGeneSource *ret);
/* Load a row from wgEncodeGencodeGeneSource table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct wgEncodeGencodeGeneSource *wgEncodeGencodeGeneSourceLoad(char **row);
/* Load a wgEncodeGencodeGeneSource from row fetched with select * from wgEncodeGencodeGeneSource
 * from database.  Dispose of this with wgEncodeGencodeGeneSourceFree(). */

struct wgEncodeGencodeGeneSource *wgEncodeGencodeGeneSourceLoadAll(char *fileName);
/* Load all wgEncodeGencodeGeneSource from whitespace-separated file.
 * Dispose of this with wgEncodeGencodeGeneSourceFreeList(). */

struct wgEncodeGencodeGeneSource *wgEncodeGencodeGeneSourceLoadAllByChar(char *fileName, char chopper);
/* Load all wgEncodeGencodeGeneSource from chopper separated file.
 * Dispose of this with wgEncodeGencodeGeneSourceFreeList(). */

#define wgEncodeGencodeGeneSourceLoadAllByTab(a) wgEncodeGencodeGeneSourceLoadAllByChar(a, '\t');
/* Load all wgEncodeGencodeGeneSource from tab separated file.
 * Dispose of this with wgEncodeGencodeGeneSourceFreeList(). */

struct wgEncodeGencodeGeneSource *wgEncodeGencodeGeneSourceCommaIn(char **pS, struct wgEncodeGencodeGeneSource *ret);
/* Create a wgEncodeGencodeGeneSource out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new wgEncodeGencodeGeneSource */

void wgEncodeGencodeGeneSourceFree(struct wgEncodeGencodeGeneSource **pEl);
/* Free a single dynamically allocated wgEncodeGencodeGeneSource such as created
 * with wgEncodeGencodeGeneSourceLoad(). */

void wgEncodeGencodeGeneSourceFreeList(struct wgEncodeGencodeGeneSource **pList);
/* Free a list of dynamically allocated wgEncodeGencodeGeneSource's */

void wgEncodeGencodeGeneSourceOutput(struct wgEncodeGencodeGeneSource *el, FILE *f, char sep, char lastSep);
/* Print out wgEncodeGencodeGeneSource.  Separate fields with sep. Follow last field with lastSep. */

#define wgEncodeGencodeGeneSourceTabOut(el,f) wgEncodeGencodeGeneSourceOutput(el,f,'\t','\n');
/* Print out wgEncodeGencodeGeneSource as a line in a tab-separated file. */

#define wgEncodeGencodeGeneSourceCommaOut(el,f) wgEncodeGencodeGeneSourceOutput(el,f,',',',');
/* Print out wgEncodeGencodeGeneSource as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* WGENCODEGENCODEGENESOURCE_H */

