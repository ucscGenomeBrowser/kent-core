/* bigDbSnp.h was originally generated by the autoSql program, which also 
 * generated bigDbSnp.c and bigDbSnp.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef BIGDBSNP_H
#define BIGDBSNP_H

#define BIGDBSNP_NUM_COLS 17

extern char *bigDbSnpCommaSepFieldNames;

enum bigDbSnpClass
    {
    bigDbSnpSnv = 0,
    bigDbSnpMnv = 1,
    bigDbSnpIns = 2,
    bigDbSnpDel = 3,
    bigDbSnpDelins = 4,
    bigDbSnpIdentity = 5,
    };
struct bigDbSnp
/* Variant summary data extracted from dbSNP, 2019 and later */
    {
    struct bigDbSnp *next;  /* Next in singly linked list. */
    char *chrom;	/* Reference sequence chromosome or scaffold */
    unsigned chromStart;	/* Start position in chrom */
    unsigned chromEnd;	/* End position in chrom */
    char *name;	/* dbSNP Reference SNP (rs) identifier */
    char *ref;	/* Reference allele; usually major allele, but may be minor allele */
    int altCount;	/* Number of alternate alleles (usually 1) */
    char **alts;	/* Alternate (non-reference) alleles; may include major allele */
    unsigned shiftBases;	/* Bases by which an indel placement could be shifted left or right (display shows thin line over uncertain region, shifts minimal representation right) */
    int freqSourceCount;	/* Number of projects reporting frequencies in current dbSNP build */
    double *minorAlleleFreq;	/* Minor allele frequency, i.e. second highest allele frequency, from each frequency source; NaN if no data from project */
    char **majorAllele;	/* Allele most frequently observed by each source */
    char **minorAllele;	/* Allele second most frequently observed by each source */
    unsigned maxFuncImpact;	/* Sequence Ontology (SO) ID number for greatest functional impact on gene; 0 if no SO terms are annotated */
    enum bigDbSnpClass class;	/* Variation class/type */
    char *ucscNotes;	/* Interesting or anomalous properties noted by UCSC */
    long long _dataOffset;	/* Offset into bigDbSnpDetails file for line with more info */
    int _dataLen;	/* Length of line in bigDbSnpDetails */
    };

struct bigDbSnp *bigDbSnpLoad(char **row);
/* Load a bigDbSnp from row fetched with select * from bigDbSnp
 * from database.  Dispose of this with bigDbSnpFree(). */

struct bigDbSnp *bigDbSnpLoadAll(char *fileName);
/* Load all bigDbSnp from whitespace-separated file.
 * Dispose of this with bigDbSnpFreeList(). */

struct bigDbSnp *bigDbSnpLoadAllByChar(char *fileName, char chopper);
/* Load all bigDbSnp from chopper separated file.
 * Dispose of this with bigDbSnpFreeList(). */

#define bigDbSnpLoadAllByTab(a) bigDbSnpLoadAllByChar(a, '\t');
/* Load all bigDbSnp from tab separated file.
 * Dispose of this with bigDbSnpFreeList(). */

struct bigDbSnp *bigDbSnpCommaIn(char **pS, struct bigDbSnp *ret);
/* Create a bigDbSnp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new bigDbSnp */

void bigDbSnpFree(struct bigDbSnp **pEl);
/* Free a single dynamically allocated bigDbSnp such as created
 * with bigDbSnpLoad(). */

void bigDbSnpFreeList(struct bigDbSnp **pList);
/* Free a list of dynamically allocated bigDbSnp's */

void bigDbSnpOutput(struct bigDbSnp *el, FILE *f, char sep, char lastSep);
/* Print out bigDbSnp.  Separate fields with sep. Follow last field with lastSep. */

#define bigDbSnpTabOut(el,f) bigDbSnpOutput(el,f,'\t','\n');
/* Print out bigDbSnp as a line in a tab-separated file. */

#define bigDbSnpCommaOut(el,f) bigDbSnpOutput(el,f,',',',');
/* Print out bigDbSnp as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

// ucscNotes is a comma-separated list of these values.  Developer: if you add a new one here,
// then also update the descriptions used by bigDbSnpDescribeUcscNote in bigDbSnp.c.
#define bdsAltIsAmbiguous "altIsAmbiguous"
#define bdsClassMismatch "classMismatch"
#define bdsClinvar "clinvar"
#define bdsClinvarBenign "clinvarBenign"
#define bdsClinvarConflicting "clinvarConflicting"
#define bdsClinvarPathogenic "clinvarPathogenic"
#define bdsClusterError "clusterError"
#define bdsCommonAll "commonAll"
#define bdsCommonSome "commonSome"
#define bdsDiffMajor "diffMajor"
#define bdsFreqIncomplete "freqIncomplete"
#define bdsFreqIsAmbiguous "freqIsAmbiguous"
#define bdsFreqNotMapped "freqNotMapped"
#define bdsFreqNotRefAlt "freqNotRefAlt"
#define bdsMultiMap "multiMap"
#define bdsOtherMapErr "otherMapErr"
#define bdsOverlapDiffClass "overlapDiffClass"
#define bdsOverlapSameClass "overlapSameClass"
#define bdsRareAll "rareAll"
#define bdsRareSome "rareSome"
#define bdsRefIsAmbiguous "refIsAmbiguous"
#define bdsRefIsMinor "refIsMinor"
#define bdsRefIsRare "refIsRare"
#define bdsRefIsSingleton "refIsSingleton"
#define bdsRefMismatch "refMismatch"
#define bdsRevStrand "revStrand"

char *bigDbSnpDescribeUcscNote(char *ucscNote);
/* Return a string describing ucscNote, unless it is unrecognized in which case return NULL.
 * Do not free returned value. */

char *bigDbSnpClassToString(enum bigDbSnpClass class);
/* Return the string version of enum bigDbSnpClass.  Do not free result. */

char *bigDbSnpAbbrevAllele(char *allele, char *buf, size_t bufLen);
/* If allele can be abbreviated to something shorter than itself that fits in buf,
 * and doesn't end up with a tiny bit of abbreviation followed by a bunch of unabbreviated
 * sequence, then put the abbreviation in buf and return buf; otherwise return allele.
 * If allele is the empty string, returns "-" (in buf). */

#endif /* BIGDBSNP_H */
