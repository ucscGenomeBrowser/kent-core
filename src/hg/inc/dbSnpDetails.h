/* dbSnpDetails.h was originally generated by the autoSql program, which also 
 * generated dbSnpDetails.c and dbSnpDetails.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef DBSNPDETAILS_H
#define DBSNPDETAILS_H

#define DBSNPDETAILS_NUM_COLS 13

extern char *dbSnpDetailsCommaSepFieldNames;

struct dbSnpDetails
/* dbSNP annotations that are too lengthy to include in bigDbSnp file; for variant details page */
    {
    struct dbSnpDetails *next;  /* Next in singly linked list. */
    char *name;	/* rs# ID of variant */
    int freqSourceCount;	/* Number of frequency sources */
    char **alleleCounts;	/* Array of each source's |-sep list of allele:count */
    int *alleleTotals;	/* Array of each source's total number of chromosomes sampled; may be > sum of observed counts and differ across variants. */
    int soTermCount;	/* Number of distinct SO terms annotated on variant */
    int *soTerms;	/* SO term numbers annotated on RefSeq transcripts */
    int clinVarCount;	/* Number of ClinVar accessions associated with variant */
    char **clinVarAccs;	/* ClinVar accessions associated with variant */
    char **clinVarSigs;	/* ClinVar significance for each accession */
    int submitterCount;	/* Number of organizations/projects that reported variant */
    char **submitters;	/* dbSNP 'handles' of submitting orgs/projects */
    int pubMedIdCount;	/* Number of PubMed-indexed publications associated with variant */
    int *pubMedIds;	/* PMIDs of associated publications */
    };

struct dbSnpDetails *dbSnpDetailsLoad(char **row);
/* Load a dbSnpDetails from row fetched with select * from dbSnpDetails
 * from database.  Dispose of this with dbSnpDetailsFree(). */

struct dbSnpDetails *dbSnpDetailsLoadAll(char *fileName);
/* Load all dbSnpDetails from whitespace-separated file.
 * Dispose of this with dbSnpDetailsFreeList(). */

struct dbSnpDetails *dbSnpDetailsLoadAllByChar(char *fileName, char chopper);
/* Load all dbSnpDetails from chopper separated file.
 * Dispose of this with dbSnpDetailsFreeList(). */

#define dbSnpDetailsLoadAllByTab(a) dbSnpDetailsLoadAllByChar(a, '\t');
/* Load all dbSnpDetails from tab separated file.
 * Dispose of this with dbSnpDetailsFreeList(). */

struct dbSnpDetails *dbSnpDetailsCommaIn(char **pS, struct dbSnpDetails *ret);
/* Create a dbSnpDetails out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new dbSnpDetails */

void dbSnpDetailsFree(struct dbSnpDetails **pEl);
/* Free a single dynamically allocated dbSnpDetails such as created
 * with dbSnpDetailsLoad(). */

void dbSnpDetailsFreeList(struct dbSnpDetails **pList);
/* Free a list of dynamically allocated dbSnpDetails's */

void dbSnpDetailsOutput(struct dbSnpDetails *el, FILE *f, char sep, char lastSep);
/* Print out dbSnpDetails.  Separate fields with sep. Follow last field with lastSep. */

#define dbSnpDetailsTabOut(el,f) dbSnpDetailsOutput(el,f,'\t','\n');
/* Print out dbSnpDetails as a line in a tab-separated file. */

#define dbSnpDetailsCommaOut(el,f) dbSnpDetailsOutput(el,f,',',',');
/* Print out dbSnpDetails as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* DBSNPDETAILS_H */

