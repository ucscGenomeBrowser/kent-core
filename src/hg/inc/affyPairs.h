/* affyPairs.h was originally generated by the autoSql program, which also 
 * generated affyPairs.c and affyPairs.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2002 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef AFFYPAIRS_H
#define AFFYPAIRS_H

struct affyPairs
/* Representation of the 'pairs' file format from the Affymetrix transcriptome data */
    {
    struct affyPairs *next;  /* Next in singly linked list. */
    unsigned x;	/* X, the x-coordinate of the perfect-complement oligo on the chip. */
    unsigned y;	/* Y, the y-coordinate of the perfect-complement oligo on the chip. */
    char *probeSet;	/* set, the probe set. */
    char *method;	/* Method (not informative) */
    unsigned tBase;	/* tBase, the target base (base in target at central position of 25-mer probe). */
    unsigned pos;	/* Pos, the position in the probeset of the central base of the probe. */
    float pm;	/* PM, the perfect-complement probe intensity. */
    float pms;	/* PM.s, the standard deviation of the pixels in the perfect-complement feature. */
    float pmp;	/* PM.p, the number of pixel used in the perfect-complement feature. */
    float mm;	/* MM, the mismatch-complement probe intensity. */
    float mms;	/* MM.s, the standard deviation of the pixels in the mismatch-complement feature. */
    float mmp;	/* MM.p, the number of pixel used in the mismatch-complement feature.  */
    };

void affyPairsStaticLoad(char **row, struct affyPairs *ret);
/* Load a row from affyPairs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct affyPairs *affyPairsLoad(char **row);
/* Load a affyPairs from row fetched with select * from affyPairs
 * from database.  Dispose of this with affyPairsFree(). */

struct affyPairs *affyPairsLoadAll(char *fileName);
/* Load all affyPairs from a tab-separated file.
 * Dispose of this with affyPairsFreeList(). */

struct affyPairs *affyPairsCommaIn(char **pS, struct affyPairs *ret);
/* Create a affyPairs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new affyPairs */

void affyPairsFree(struct affyPairs **pEl);
/* Free a single dynamically allocated affyPairs such as created
 * with affyPairsLoad(). */

void affyPairsFreeList(struct affyPairs **pList);
/* Free a list of dynamically allocated affyPairs's */

void affyPairsOutput(struct affyPairs *el, FILE *f, char sep, char lastSep);
/* Print out affyPairs.  Separate fields with sep. Follow last field with lastSep. */

#define affyPairsTabOut(el,f) affyPairsOutput(el,f,'\t','\n');
/* Print out affyPairs as a line in a tab-separated file. */

#define affyPairsCommaOut(el,f) affyPairsOutput(el,f,',',',');
/* Print out affyPairs as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* AFFYPAIRS_H */
