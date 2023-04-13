/* gbSeq.h was originally generated by the autoSql program, which also 
 * generated gbSeq.c and gbSeq.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2009 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef GBSEQ_H
#define GBSEQ_H

#define GBSEQ_NUM_COLS 9

enum gbSeqType
    {
    gbSeqEST = 0,
    gbSeqMRNA = 1,
    gbSeqPEP = 2,
    };
enum gbSeqSrcDb
    {
    gbSeqGenBank = 0,
    gbSeqRefSeq = 1,
    gbSeqOther = 2,
    };
struct gbSeq
/* Information about sequences contained in files described in gbExtFile */
    {
    struct gbSeq *next;  /* Next in singly linked list. */
    unsigned id;	/* ID/index */
    char *acc;	/* Accession of sequence */
    int version;	/* Genbank version */
    unsigned size;	/* Size of sequence (number of bases) */
    unsigned gbExtFile;	/* ID/index of file in gbExtFile */
    long long file_offset;	/* byte offset of sequence in file */
    unsigned file_size;	/* byte size of sequence in file */
    enum gbSeqType type;	/* Type of sequence */
    enum gbSeqSrcDb srcDb;	/* Source database */
    };

void gbSeqStaticLoad(char **row, struct gbSeq *ret);
/* Load a row from gbSeq table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct gbSeq *gbSeqLoad(char **row);
/* Load a gbSeq from row fetched with select * from gbSeq
 * from database.  Dispose of this with gbSeqFree(). */

struct gbSeq *gbSeqLoadAll(char *fileName);
/* Load all gbSeq from whitespace-separated file.
 * Dispose of this with gbSeqFreeList(). */

struct gbSeq *gbSeqLoadAllByChar(char *fileName, char chopper);
/* Load all gbSeq from chopper separated file.
 * Dispose of this with gbSeqFreeList(). */

#define gbSeqLoadAllByTab(a) gbSeqLoadAllByChar(a, '\t');
/* Load all gbSeq from tab separated file.
 * Dispose of this with gbSeqFreeList(). */

struct gbSeq *gbSeqCommaIn(char **pS, struct gbSeq *ret);
/* Create a gbSeq out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new gbSeq */

void gbSeqFree(struct gbSeq **pEl);
/* Free a single dynamically allocated gbSeq such as created
 * with gbSeqLoad(). */

void gbSeqFreeList(struct gbSeq **pList);
/* Free a list of dynamically allocated gbSeq's */

void gbSeqOutput(struct gbSeq *el, FILE *f, char sep, char lastSep);
/* Print out gbSeq.  Separate fields with sep. Follow last field with lastSep. */

#define gbSeqTabOut(el,f) gbSeqOutput(el,f,'\t','\n');
/* Print out gbSeq as a line in a tab-separated file. */

#define gbSeqCommaOut(el,f) gbSeqOutput(el,f,',',',');
/* Print out gbSeq as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* GBSEQ_H */

