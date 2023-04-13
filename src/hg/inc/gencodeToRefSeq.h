/* gencodeToRefSeq.h was originally generated by the autoSql program, which also 
 * generated gencodeToRefSeq.c and gencodeToRefSeq.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef GENCODETOREFSEQ_H
#define GENCODETOREFSEQ_H

#define GENCODETOREFSEQ_NUM_COLS 3

extern char *gencodeToRefSeqCommaSepFieldNames;

struct gencodeToRefSeq
/* GENCODE transcript to RefSeq mRNA or non-coding RNA and peptide accession mapping */
    {
    struct gencodeToRefSeq *next;  /* Next in singly linked list. */
    char *transcriptId;	/* GENCODE transcript identifier */
    char *rnaAcc;	/* RefSeq mRNA or non-coding RNA accession */
    char *pepAcc;	/* RefSeq peptide identifier for coding accession */
    };

void gencodeToRefSeqStaticLoad(char **row, struct gencodeToRefSeq *ret);
/* Load a row from gencodeToRefSeq table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gencodeToRefSeq *gencodeToRefSeqLoad(char **row);
/* Load a gencodeToRefSeq from row fetched with select * from gencodeToRefSeq
 * from database.  Dispose of this with gencodeToRefSeqFree(). */

struct gencodeToRefSeq *gencodeToRefSeqLoadAll(char *fileName);
/* Load all gencodeToRefSeq from whitespace-separated file.
 * Dispose of this with gencodeToRefSeqFreeList(). */

struct gencodeToRefSeq *gencodeToRefSeqLoadAllByChar(char *fileName, char chopper);
/* Load all gencodeToRefSeq from chopper separated file.
 * Dispose of this with gencodeToRefSeqFreeList(). */

#define gencodeToRefSeqLoadAllByTab(a) gencodeToRefSeqLoadAllByChar(a, '\t');
/* Load all gencodeToRefSeq from tab separated file.
 * Dispose of this with gencodeToRefSeqFreeList(). */

struct gencodeToRefSeq *gencodeToRefSeqCommaIn(char **pS, struct gencodeToRefSeq *ret);
/* Create a gencodeToRefSeq out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gencodeToRefSeq */

void gencodeToRefSeqFree(struct gencodeToRefSeq **pEl);
/* Free a single dynamically allocated gencodeToRefSeq such as created
 * with gencodeToRefSeqLoad(). */

void gencodeToRefSeqFreeList(struct gencodeToRefSeq **pList);
/* Free a list of dynamically allocated gencodeToRefSeq's */

void gencodeToRefSeqOutput(struct gencodeToRefSeq *el, FILE *f, char sep, char lastSep);
/* Print out gencodeToRefSeq.  Separate fields with sep. Follow last field with lastSep. */

#define gencodeToRefSeqTabOut(el,f) gencodeToRefSeqOutput(el,f,'\t','\n');
/* Print out gencodeToRefSeq as a line in a tab-separated file. */

#define gencodeToRefSeqCommaOut(el,f) gencodeToRefSeqOutput(el,f,',',',');
/* Print out gencodeToRefSeq as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* GENCODETOREFSEQ_H */

