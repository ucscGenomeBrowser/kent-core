/* recombRate.h was originally generated by the autoSql program, which also 
 * generated recombRate.c and recombRate.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2002 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef RECOMBRATE_H
#define RECOMBRATE_H

struct recombRate
/* Describes the recombination rate in 1Mb intervals based on deCODE map */
    {
    struct recombRate *next;  /* Next in singly linked list. */
    char *chrom;	/* Human chromosome number */
    unsigned chromStart;	/* Start position in genoSeq */
    unsigned chromEnd;	/* End position in genoSeq */
    char *name;	/* Constant string recombRate */
    float decodeAvg;	/* Calculated deCODE recombination rate */
    float decodeFemale;	/* Calculated deCODE female recombination rate */
    float decodeMale;	/* Calculated deCODE male recombination rate */
    float marshfieldAvg;	/* Calculated Marshfield recombination rate */
    float marshfieldFemale;	/* Calculated Marshfield female recombination rate */
    float marshfieldMale;	/* Calculated Marshfield male recombination rate */
    float genethonAvg;	/* Calculated Genethon recombination rate */
    float genethonFemale;	/* Calculated Genethon female recombination rate */
    float genethonMale;	/* Calculated Genethon male recombination rate */
    };

void recombRateStaticLoad(char **row, struct recombRate *ret);
/* Load a row from recombRate table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct recombRate *recombRateLoad(char **row);
/* Load a recombRate from row fetched with select * from recombRate
 * from database.  Dispose of this with recombRateFree(). */

struct recombRate *recombRateLoadAll(char *fileName);
/* Load all recombRate from a tab-separated file.
 * Dispose of this with recombRateFreeList(). */

struct recombRate *recombRateLoadWhere(struct sqlConnection *conn, char *table, char *where);
/* Load all recombRate from table that satisfy where clause. The
 * where clause may be NULL in which case whole table is loaded
 * Dispose of this with recombRateFreeList(). */

struct recombRate *recombRateCommaIn(char **pS, struct recombRate *ret);
/* Create a recombRate out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new recombRate */

void recombRateFree(struct recombRate **pEl);
/* Free a single dynamically allocated recombRate such as created
 * with recombRateLoad(). */

void recombRateFreeList(struct recombRate **pList);
/* Free a list of dynamically allocated recombRate's */

void recombRateOutput(struct recombRate *el, FILE *f, char sep, char lastSep);
/* Print out recombRate.  Separate fields with sep. Follow last field with lastSep. */

#define recombRateTabOut(el,f) recombRateOutput(el,f,'\t','\n');
/* Print out recombRate as a line in a tab-separated file. */

#define recombRateCommaOut(el,f) recombRateOutput(el,f,',',',');
/* Print out recombRate as a comma separated list including final comma. */

#endif /* RECOMBRATE_H */

