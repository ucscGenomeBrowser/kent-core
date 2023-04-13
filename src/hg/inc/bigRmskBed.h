/* bigRmskBed.h was originally generated by the autoSql program, which also 
 * generated bigRmskBed.c and bigRmskBed.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef BIGRMSKBED_H
#define BIGRMSKBED_H

#define BIGRMSKBED_NUM_COLS 14

extern char *bigRmskBedCommaSepFieldNames;

struct bigRmskBed
/* Repetitive Element Annotation */
    {
    struct bigRmskBed *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position of visualization on chromosome */
    unsigned chromEnd;	/* End position of visualation on chromosome */
    char *name;	/* Name repeat, including the type/subtype suffix */
    unsigned score;	/* Divergence score */
    char strand[2];	/* + or - for strand */
    unsigned thickStart;	/* Start position of aligned sequence on chromosome */
    unsigned thickEnd;	/* End position of aligned sequence on chromosome */
    unsigned reserved;	/* Reserved */
    unsigned blockCount;	/* Count of sequence blocks */
    char *blockSizes;	/* A comma-separated list of the block sizes(+/-) */
    char *blockStarts;	/* A comma-separated list of the block starts(+/-) */
    unsigned id;	/* A unique identifier for the joined annotations in this record */
    char *description;	/* A comma separated list of technical annotation descriptions */
    };

void bigRmskBedStaticLoad(char **row, struct bigRmskBed *ret);
/* Load a row from bigRmskBed table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct bigRmskBed *bigRmskBedLoad(char **row);
/* Load a bigRmskBed from row fetched with select * from bigRmskBed
 * from database.  Dispose of this with bigRmskBedFree(). */

struct bigRmskBed *bigRmskBedLoadAll(char *fileName);
/* Load all bigRmskBed from whitespace-separated file.
 * Dispose of this with bigRmskBedFreeList(). */

struct bigRmskBed *bigRmskBedLoadAllByChar(char *fileName, char chopper);
/* Load all bigRmskBed from chopper separated file.
 * Dispose of this with bigRmskBedFreeList(). */

#define bigRmskBedLoadAllByTab(a) bigRmskBedLoadAllByChar(a, '\t');
/* Load all bigRmskBed from tab separated file.
 * Dispose of this with bigRmskBedFreeList(). */

struct bigRmskBed *bigRmskBedCommaIn(char **pS, struct bigRmskBed *ret);
/* Create a bigRmskBed out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bigRmskBed */

void bigRmskBedFree(struct bigRmskBed **pEl);
/* Free a single dynamically allocated bigRmskBed such as created
 * with bigRmskBedLoad(). */

void bigRmskBedFreeList(struct bigRmskBed **pList);
/* Free a list of dynamically allocated bigRmskBed's */

void bigRmskBedOutput(struct bigRmskBed *el, FILE *f, char sep, char lastSep);
/* Print out bigRmskBed.  Separate fields with sep. Follow last field with lastSep. */

#define bigRmskBedTabOut(el,f) bigRmskBedOutput(el,f,'\t','\n');
/* Print out bigRmskBed as a line in a tab-separated file. */

#define bigRmskBedCommaOut(el,f) bigRmskBedOutput(el,f,',',',');
/* Print out bigRmskBed as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* BIGRMSKBED_H */

