/* orthoAlleles.h was originally generated by the autoSql program, which also 
 * generated orthoAlleles.c and orthoAlleles.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef ORTHOALLELES_H
#define ORTHOALLELES_H

#define ORTHOALLELES_NUM_COLS 22

struct orthoAlleles
/* SNP data with orthologous alleles */
    {
    struct orthoAlleles *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Name of SNP - rsId or temporary name */
    unsigned score;	/* 0 */
    char strand;	/* + or - */
    char allele1;	/* A, C, G, or T */
    float allele1Freq;	/* From 0.0 to 1.0 */
    unsigned allele1Count;	/* From 1 to sample size */
    char allele2;	/* A, C, G, or T, different from allele1 */
    float allele2Freq;	/* 1.0 - allele1Freq */
    unsigned allele2Count;	/* sample size - allele1Count */
    char *ortho1Chrom;	/* Chromosome of orthologous species 1 */
    unsigned ortho1ChromStart;	/* Start position in species 1 */
    unsigned ortho1ChromEnd;	/* End position in species 1 */
    char ortho1Strand;	/* + or - or . */
    char *ortho1State;	/* A, C, G, T, a, c, g, t, or . */
    char *ortho2Chrom;	/* Chromosome of orthologous species 2 */
    unsigned ortho2ChromStart;	/* Start position in species 2 */
    unsigned ortho2ChromEnd;	/* End position in species 2 */
    char ortho2Strand;	/* + or - or . */
    char *ortho2State;	/* A, C, G, T, a, c, g, t, or . */
    };

void orthoAllelesStaticLoad(char **row, struct orthoAlleles *ret);
/* Load a row from orthoAlleles table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct orthoAlleles *orthoAllelesLoad(char **row);
/* Load a orthoAlleles from row fetched with select * from orthoAlleles
 * from database.  Dispose of this with orthoAllelesFree(). */

struct orthoAlleles *orthoAllelesLoadAll(char *fileName);
/* Load all orthoAlleles from whitespace-separated file.
 * Dispose of this with orthoAllelesFreeList(). */

struct orthoAlleles *orthoAllelesLoadAllByChar(char *fileName, char chopper);
/* Load all orthoAlleles from chopper separated file.
 * Dispose of this with orthoAllelesFreeList(). */

#define orthoAllelesLoadAllByTab(a) orthoAllelesLoadAllByChar(a, '\t');
/* Load all orthoAlleles from tab separated file.
 * Dispose of this with orthoAllelesFreeList(). */

struct orthoAlleles *orthoAllelesCommaIn(char **pS, struct orthoAlleles *ret);
/* Create a orthoAlleles out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new orthoAlleles */

void orthoAllelesFree(struct orthoAlleles **pEl);
/* Free a single dynamically allocated orthoAlleles such as created
 * with orthoAllelesLoad(). */

void orthoAllelesFreeList(struct orthoAlleles **pList);
/* Free a list of dynamically allocated orthoAlleles's */

void orthoAllelesOutput(struct orthoAlleles *el, FILE *f, char sep, char lastSep);
/* Print out orthoAlleles.  Separate fields with sep. Follow last field with lastSep. */

#define orthoAllelesTabOut(el,f) orthoAllelesOutput(el,f,'\t','\n');
/* Print out orthoAlleles as a line in a tab-separated file. */

#define orthoAllelesCommaOut(el,f) orthoAllelesOutput(el,f,',',',');
/* Print out orthoAlleles as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* ORTHOALLELES_H */

