/* snp.h was originally generated by the autoSql program, which also 
 * generated snp.c and snp.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef SNP_H
#define SNP_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define SNP_NUM_COLS 16

struct snp
/* Polymorphism data from dbSnp XML files or genotyping arrays */
    {
    struct snp *next;  /* Next in singly linked list. */
    char *chrom;	/* Chromosome */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* Reference SNP identifier or Affy SNP name */
    unsigned score;	/* Not used */
    char strand[2];	/* Which DNA strand contains the observed alleles */
    char *observed;	/* The sequences of the observed alleles */
    char *molType;	/* Sample type from exemplar ss */
    char *class;	/* The class of variant */
    char *valid;	/* The validation status of the SNP */
    float avHet;	/* The average heterozygosity from all observations */
    float avHetSE;	/* The Standard Error for the average heterozygosity */
    char *func;	/* The functional category of the SNP */
    char *locType;	/* How the variant affects the reference sequence */
    char *source;	/* Source of the data - dbSnp, Affymetrix, ... */
    char *exception;	/* List of exceptionIds for 'invariant' conditions */
    };

void snpStaticLoad(char **row, struct snp *ret);
/* Load a row from snp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct snp *snpLoad(char **row);
/* Load a snp from row fetched with select * from snp
 * from database.  Dispose of this with snpFree(). */

struct snp *snpLoadAll(char *fileName);
/* Load all snp from whitespace-separated file.
 * Dispose of this with snpFreeList(). */

struct snp *snpLoadAllByChar(char *fileName, char chopper);
/* Load all snp from chopper separated file.
 * Dispose of this with snpFreeList(). */

#define snpLoadAllByTab(a) snpLoadAllByChar(a, '\t');
/* Load all snp from tab separated file.
 * Dispose of this with snpFreeList(). */

struct snp *snpLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all snp from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with snpFreeList(). */

void snpSaveToDb(struct sqlConnection *conn, struct snp *el, char *tableName, int updateSize);
/* Save snp as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */

struct snp *snpCommaIn(char **pS, struct snp *ret);
/* Create a snp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new snp */

void snpFree(struct snp **pEl);
/* Free a single dynamically allocated snp such as created
 * with snpLoad(). */

void snpFreeList(struct snp **pList);
/* Free a list of dynamically allocated snp's */

void snpOutput(struct snp *el, FILE *f, char sep, char lastSep);
/* Print out snp.  Separate fields with sep. Follow last field with lastSep. */

#define snpTabOut(el,f) snpOutput(el,f,'\t','\n');
/* Print out snp as a line in a tab-separated file. */

#define snpCommaOut(el,f) snpOutput(el,f,',',',');
/* Print out snp as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* SNP_H */
