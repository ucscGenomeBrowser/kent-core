/* repMask.h was originally generated by the autoSql program, which also 
 * generated repMask.c and repMask.sql.  This header links the database and the RAM 
 * representation of objects. */

#ifndef REPMASK_H
#define REPMASK_H

struct repeatMaskOut
/* Repeat Masker out format */
    {
    struct repeatMaskOut *next;  /* Next in singly linked list. */
    unsigned score;	/* Smith-Waterman score. */
    float percDiv;	/* Percentage base divergence. */
    float percDel;	/* Percentage deletions. */
    float percInc;	/* Percentage inserts. */
    char *qName;	/* Name of query. */
    int qStart;	/* Start query position. */
    int qEnd;	/* End query position. */
    char *qLeft;	/* Bases left in query. */
    char strand[2];	/* Query strand (+ or C) */
    char *rName;	/* Repeat name */
    char *rFamily;	/* Repeat name */
    char *rStart;	/* Start position in repeat. */
    unsigned rEnd;	/* End position in repeat. */
    char *rLeft;	/* Bases left in repeat. */
    };

void repeatMaskOutStaticLoad(char **row, struct repeatMaskOut *ret);
/* Load a row from repeatMaskOut table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct repeatMaskOut *repeatMaskOutLoad(char **row);
/* Load a repeatMaskOut from row fetched with select * from repeatMaskOut
 * from database.  Dispose of this with repeatMaskOutFree(). */

struct repeatMaskOut *repeatMaskOutCommaIn(char **pS);
/* Create a repeatMaskOut out of a comma separated string. */

void repeatMaskOutFree(struct repeatMaskOut **pEl);
/* Free a single dynamically allocated repeatMaskOut such as created
 * with repeatMaskOutLoad(). */

void repeatMaskOutFreeList(struct repeatMaskOut **pList);
/* Free a list of dynamically allocated repeatMaskOut's */

void repeatMaskOutOutput(struct repeatMaskOut *el, FILE *f, char sep, char lastSep);
/* Print out repeatMaskOut.  Separate fields with sep. Follow last field with lastSep. */

#define repeatMaskOutTabOut(el,f) repeatMaskOutOutput(el,f,'\t','\n');
/* Print out repeatMaskOut as a line in a tab-separated file. */

#define repeatMaskOutCommaOut(el,f) repeatMaskOutOutput(el,f,',',',');
/* Print out repeatMaskOut as a comma separated list including final comma. */

struct lineFile *rmskLineFileOpen(char *fileName);
/* open a repeat masker .out file or die trying */

#endif /* REPMASK_H */

