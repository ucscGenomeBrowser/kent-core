/* pslCDnaGenomeMatch - check if retroGene aligns better to parent or retroGene */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "memalloc.h"
#include "psl.h"
#include "hdb.h"
#include "axt.h"
#include "dnautil.h"
#include "dnaseq.h"
#include "twoBit.h"
#include "fa.h"
#include "dlist.h"
#include "dystring.h"
#include "binRange.h"
#include "options.h"
#include "genePred.h"
#include "genePredReader.h"
#include "dystring.h"
#include "pseudoGeneLink.h"
#include "verbose.h"
#include "nibTwo.h"
#include "bed.h"
#include "snp125.h"
#include "pipeline.h"
#define MINDIFF 5
#define MAXLOCI 2048
#define NOVALUE 10000  /* loci index when there is no genome base for that mrna position */
#include "mrnaMisMatch.h"
#define NOTALIGNEDFACTOR 8 /* unaligned bases divided by this factor reduce score */
static char na[3] = "NA";
struct axtScoreScheme *ss = NULL; /* blastz scoring matrix */
struct hash *snpHash = NULL, *mrnaHash = NULL, *faHash = NULL, *tHash = NULL, *species1Hash = NULL, *species2Hash = NULL;
int maxGap = 100;
int minDiff = MINDIFF;
int notAlignPenalty = NOTALIGNEDFACTOR;
int aliCount = 0; /* number of alignments read */
int mrnaCount = 0; /* number of mrna read */
int filterCount = 0; /* number of mrna where best hit can be determined  */
int outputCount = 0; /* number of alignments written */
int verbosity = 1;
int lociCounter = 0;
bool passthru = FALSE;
bool computeSS = FALSE;
struct dlList *fileCache = NULL;
FILE *outFile = NULL; /* output tab sep file */
FILE *bedFile = NULL; /* output bed file */
FILE *scoreFile = NULL; /* output score file */
struct twoBitFile *cdnaSeqFile = NULL;
struct twoBitFile *genomeSeqFile = NULL;
char *species1 = NULL;
char *species2 = NULL;
char *nibdir1 = NULL;
char *nibdir2 = NULL;
char *mrna1 = NULL;
char *mrna2 = NULL;
char *bedOut = NULL;
char *scoreOut = NULL;
char *snpFile = NULL; /* snp tab file (browser format)*/
int histogram[256][256];       /* histogram of counts for suff statistics, index is a,c,g,t,-,. */
float  histoNorm[256][256]; /* normalized histogram for suff statistics, index is a,c,g,t,-,. */

/* command line */
static struct optionSpec optionSpecs[] = {
    {"species1", OPTION_STRING},
    {"species2", OPTION_STRING},
    {"nibdir1", OPTION_STRING},
    {"nibdir2", OPTION_STRING},
    {"mrna1", OPTION_STRING},
    {"mrna2", OPTION_STRING},
    {"bedOut", OPTION_STRING},
    {"score", OPTION_STRING},
    {"snp", OPTION_STRING},
    {"minDiff", OPTION_INT},
    {"notAlignPenalty", OPTION_INT},
    {"passthru", OPTION_BOOLEAN},
    {"computeSS", OPTION_BOOLEAN},
    {NULL, 0}
};

struct loci
    {
    struct loci *next;
    char *chrom ;
    int chromStart;
    int chromEnd;
    int index ;
    int score ;     /* score for the loci - count of mismatches relative to other loci*/ 
    struct psl *psl; /* alignment to mrna for this loci */
    };

struct misMatch
    {
/* list of mismatches between genome and mrna */
    struct misMatch *next;      /* next in list. */
    char *name;                 /* mrna acc */
    int mrnaLoc;                /* offset in mrna of mismatch */
    char *chrom;                /* position in genome of misMatch */
    int chromStart;             /* position in genome of misMatch */
    char strand;
    int type;                   /* type of mismatch , 0=normal , 1=snp */
    char mrnaBase;              /* base in mRNA */
    char genomeBase;            /* base in genome (strand relative to mrna) */
    int loci;                   /* index number of loci */
    int snpCount;               /* count of snps overlapping this mismatch */
    char **snpObs;              /* snp observed bases */
    char **snps;                /* snp ids */
    /* not used */
    int retroLoc;               /* genomic location of mismatch in retroGene */
    int parentLoc;              /* genomic location of mismatch in parent Gene */
    char retroBase;             /* contents of base in retroGene */
    char parentBase;            /* contents of base in parent gene */
    };

struct alignment
/* alignment with context info */
    {
    struct alignment *next;
    struct psl *psl;        /* alignment of mrna for this species */
    char *db;                   /* name of database of species */
    struct twoBitFile *twoBitSeqFile;           /* open two bit file */
    char *mrnaPath;             /* path to location of mrna sequence */
    struct dnaSeq *tSeq;        /* sequence in genome */
    struct dnaSeq *qSeq;        /* mrna sequence */
    char **qSequence;  /* query sequence for each block */
    char **tSequence;  /* target sequence for each block */
    };

struct cachedFile
/* File in cache. */
    {
    struct cachedFile *next;	/* next in list. */
    char *name;		/* File name (allocated here) */
    FILE *f;		/* Open file. */
    };

void usage()
/* Print usage instructions and exit. */
{
errAbort(
    "pslCDnaGenomeMatch - check if retroGene aligns better to parent or retroGene \n"
    "usage:\n"
    "    pslCDnaGenomeMatch input.psl chrom.sizes cdna.2bit genome.2bit output.psl\n"
    "input.psl contains input mRNA/EST alignment psl file SORTED by qName\n"
    "chrom.sizes is a list of chromosome followed by size\n"
    "cdna.2bit contains fasta sequences of all mrnas/EST in 2bit format\n"
    "genome.2bit contains fasta genome sequences in 2bit format\n"
    "output.psl contains filtered alignments for best matches and cases where no filtering occurred.\n\n"
    "Options: \n"
    "    -score=output.tab  is output containing mismatch info\n" 
    "    -computeSS compute sufficient statistics instead of filtering\n"
    "    -passthru  if best hit cannot be decided, then pass through all alignments\n"
    "    -minDiff=N minimum difference in score to filter out 2nd best hit (default %d)\n"
    "    -notAlignPenalty=N score non-aligning bases with 1/N (default %d)\n"
    "    -bedOut=bed output file of mismatches.\n"
    "    -species1=psl file with alignment of mrna/EST to other species.\n"
    "    -species2=psl file with alignment of mrna/EST to other species.\n"
    "    -snp=snp.tab.gz contains snps with or without bin field in snp125 format\n"
    "    -nibdir1=sequence of species 1 \n"
    "    -nibdir2=sequence of species 2 \n"
    "    -mrna1=fasta file with sequence of mrna/EST used in alignment 1\n"
    "    -mrna2=fasta file with sequence of mrna/EST used in alignment 2\n",
    minDiff, notAlignPenalty);
}

void alignFree(struct alignment  **pEl)
/* Free a single dynamically allocated alignment */
{
struct alignment *el;
struct psl *p;
struct dnaSeq *tSeq;
struct dnaSeq *qSeq;

if ((el = *pEl) == NULL) return;
p = el->psl;
pslFree(&p);
tSeq = el->tSeq;
qSeq = el->qSeq;
freeDnaSeq(&tSeq);
freeDnaSeq(&qSeq);
freez(pEl);
}

void alignFreeList(struct alignment **pList)
/* Free a list of dynamically allocated alignment's */
{
struct alignment *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    alignFree(&el);
    }
*pList = NULL;
}

bool purine(char a)
{
if (toupper(a) == 'A' || toupper(a) == 'G')
    return(TRUE);
else
    return(FALSE);
}

bool transversion(char a, char b)
{
if (purine(a) != purine(b))
    return TRUE;
else
    return FALSE;
}

bool transition(char a, char b)
{
if (!transversion(a,b))
    return TRUE;
else
    return FALSE;
}

struct dnaSeq *twoBitGetSeqForStrand(struct twoBitFile *tbf, char *seqName, int start, int end,
	char strand)
/* Load in a mixed case sequence from from a twoBit file, from reverse strand if
 * strand is '-'. */
{
struct dnaSeq *seq;
int size = end - start;
int seqSize = twoBitSeqSize(tbf, seqName);
assert(size >= 0);

if (strand == '-')
    {
    verbose(6,"before start %d end %d size %d %s\n",
            start, end, size, seqName);
    reverseIntRange(&start, &end, seqSize);
    verbose(6,"after start %d end %d size %d %s\n",
            start, end, size, seqName);
    assert(start >= 0);
    seq = twoBitReadSeqFrag(tbf, seqName, start, end);
    reverseComplement(seq->dna, seq->size);
    }
else
    {
    seq = twoBitReadSeqFrag(tbf, seqName, start, end);
    }
return seq;
}

void addLoci(struct loci **lociList, struct psl *psl)
    /* add a new loci from psl, if not already in list */
{
struct loci *loci = NULL;
bool found = FALSE;

if (!found)
    {
    AllocVar(loci);
    loci->chrom = cloneString(psl->tName);
    loci->chromStart = psl->tStart;
    loci->chromEnd = psl->tEnd;
    loci->index = lociCounter++;
    loci->psl = psl;
    slAddHead(lociList , loci);
    }
}

void lociFree(struct loci **pEl)
/* Free a single dynamically allocated lociList */
{
struct loci *el;

if ((el = *pEl) == NULL) return;
freeMem(el->chrom);
freez(pEl);
}

void lociFreeList (struct loci **pList)
/* Free a list of dynamically allocated loci's */
{
struct loci *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    lociFree(&el);
    }
*pList = NULL;
}
int getLoci(struct loci *lociList, char *chrom, int chromStart) 
{
struct loci *loci = NULL;
for (loci = lociList ; loci != NULL ; loci = loci->next)
    if (sameString(chrom, loci->chrom)  && chromStart == loci->chromStart )
        return loci->index;
return -1;
}

bool getLociPosition(struct loci *lociList, int index , char **chrom, int *chromStart, int *chromEnd, struct psl **psl)
{
struct loci *loci = NULL;
if (index < 0) 
    return FALSE;
for (loci = lociList ; loci != NULL ; loci = loci->next)
    if (index == loci->index)
        {
        *chrom = loci->chrom;
        *chromStart = loci->chromStart;
        *chromEnd = loci->chromEnd;
        *psl = loci->psl;
        }
        return TRUE; 
return FALSE;
}

struct hash *readSnpToBinKeeper(char *sizeFileName, char *snpFileName)
/* read a list of psls and return results in hash of binKeeper structure for fast query*/
{
struct binKeeper *bk; 
struct snp125 *snp;
struct lineFile *sf = lineFileOpen(sizeFileName, TRUE);
struct lineFile *pf = lineFileOpen(snpFileName , TRUE);
struct hash *hash = newHash(0);
char *chromRow[2];
char *row[SNP125_NUM_COLS+1] ;
int wordCount = 0;

while (lineFileRow(sf, chromRow))
    {
    char *name = chromRow[0];
    int size = lineFileNeedNum(sf, chromRow, 1);

    if (hashLookup(hash, name) != NULL)
        warn("Duplicate %s, ignoring all but first\n", name);
    else
        {
        bk = binKeeperNew(0, size);
        assert(size > 1);
	hashAdd(hash, name, bk);
        }
    }
while ((wordCount = lineFileChopNextTab(pf, row, ArraySize(row))))
    {
    int offset = wordCount-SNP125_NUM_COLS;
    snp = snp125Load(row+offset);
    bk = hashMustFindVal(hash, snp->chrom);
    binKeeperAdd(bk, snp->chromStart, snp->chromEnd, snp);
    }
lineFileClose(&pf);
lineFileClose(&sf);
return hash;
}

struct psl *readPslList(char *inName)
/* read in a list of psls from a file */
{
int lineSize;
char *line;
char *words[32];
int wordCount;
struct lineFile *in = pslFileOpen(inName);
struct psl *psl = NULL, *pslList = NULL;

while (lineFileNext(in, &line, &lineSize))
    {
    wordCount = chopTabs(line, words);
    if (wordCount == 21)
	psl = pslLoad(words);
    else if (wordCount == 23)
	psl = pslxLoad(words);
    else
	errAbort("Bad line %d of %s\n", in->lineIx, in->fileName);
    aliCount++;
    slAddHead(&pslList, psl);
    }
lineFileClose(&in);
slReverse(&pslList);
return pslList;
}

struct hash *readPslQnameToHash(char *pslFileName)
/* read a list of psls and return results in hash for fast query*/
{
struct psl *psl;
struct lineFile *pf = lineFileOpen(pslFileName , TRUE);
struct hash *hash = newHash(0);
char *row[21] ;

while (lineFileNextRow(pf, row, ArraySize(row)))
    {
    psl = pslLoad(row);
    hashAdd(hash, psl->qName, psl);
    }
lineFileClose(&pf);
return hash;
}

void printMisMatch(struct misMatch **misMatchList)
{
int i;
struct misMatch *mm;

for (mm = *misMatchList ; mm != NULL ; mm = mm->next)
    for (i = 0 ; i < mm->snpCount; i++)
        {
        verbose(4,"       [%d] print mmlist snp %s %s %s:%d %c mrnaLoc %d t %c loci %u\n",
            i, mm->snps[i], mm->name, mm->chrom, mm->chromStart, 
            mm->strand, mm->mrnaLoc,  mm->genomeBase, mm->loci);
        }
    }
void printMrnaMisMatch(struct mrnaMisMatch *mm)
{
int i;

for (i = 0 ; i < mm->misMatchCount; i++)
    {
    char *sp = mm->snps[i];
    unsigned int loci = mm->loci[i];
    verbose(4,"       [%d] print snp %s %s %s:%u %s mrnaLoc %d t %c loci %u\n",
        i, sp, mm->name, mm->chroms[i], mm->tStarts[i],  
        mm->strands, mm->mrnaLoc,  mm->bases[i], loci);
    }
}

struct snp125 *getSnpList(char *chrom, int chromStart, int chromEnd, char genomeStrand)
/* get list of snps from hash based on genome coordinates */
{
struct binKeeper *bk; 
struct binElement *elist = NULL, *el = NULL;
struct snp125 *snpList = NULL;
if (snpHash == NULL)
    return NULL;
bk = hashFindVal(snpHash, chrom);
verbose(6, "     filter snps %s %d %d from list \n", chrom, chromStart, chromEnd);
assert(chromEnd != 0);
elist = binKeeperFindSorted(bk, chromStart, chromEnd ) ;
for (el = elist; el != NULL ; el = el->next)
    {
    /* retrieve snp and complement to agree with strand of mrna  */
    struct snp125 *snp = el->val;
    char *snpDna = NULL;
    verbose(4, "%s %s:%d \n", snp->name, snp->chrom, snp->chromStart);
    snpDna = replaceChars(snp->observed, "/",".");
    if (genomeStrand != snp->strand[0])
        reverseComplement(snpDna, strlen(snp->observed));
    slAddHead(&snpList, snp);
    }
return snpList;
}

struct misMatch *newMisMatch(char *name, int offset  , char genomeBase, char mrnaBase, 
        char *chrom, int chromStart, int mrnaLoc, char strand, int index , 
        struct loci *lociList, struct snp125 *snpList)
/* add new mismatch to list, figure out which loci we are mapped to */
{
struct misMatch *misMatch = NULL;
AllocVar(misMatch);    

misMatch->name = cloneString(name);
misMatch->genomeBase = genomeBase;
misMatch->mrnaBase = mrnaBase;
misMatch->chromStart = chromStart;
misMatch->chrom = cloneString(chrom);
misMatch->strand = strand;
misMatch->mrnaLoc = mrnaLoc;
misMatch->loci = index ; 
if (index > 5000)
    assert(index < 5000);
misMatch->snpCount = 0;
if (snpList != NULL)
    {
    struct snp125 *snp = NULL;
    AllocArray(misMatch->snpObs, slCount(snpList));
    AllocArray(misMatch->snps, slCount(snpList));
    for (snp = snpList ; snp != NULL ; snp = snp->next)
        {
        if (sameString(snp->chrom, chrom) && chromStart == snp->chromStart)
            {
            misMatch->snpObs[misMatch->snpCount] = cloneString(snp->observed);
            misMatch->snps[misMatch->snpCount] = cloneString(snp->name);
            misMatch->snpCount++;
            verbose(5,"   yz mismatch %s %d %c/%c ",
                    name, offset ,genomeBase, mrnaBase);
            verbose(5," t %s:%d q %d %c loci %d index %d",
                    chrom, chromStart, mrnaLoc, strand, misMatch->loci, index);
            verbose(5, " snp %s count %d", snp->name, misMatch->snpCount);
            verbose(5, "\n");
            }
        }
    assert(misMatch->snpCount <= slCount(snpList));
    }
printMisMatch(&misMatch);
return misMatch;
}

void misMatchFree(struct misMatch **pEl)
/* Free a single dynamically allocated misMatch such as created
 * with misMatchLoad(). */
{
struct misMatch *el;
int i;

if ((el = *pEl) == NULL) return;
freeMem(el->name);
freeMem(el->chrom);
/* All strings in snps are allocated at once, so only need to free first. */
for (i = 0 ; i < el->snpCount; i++)
    if (el->snps[i] != NULL)
        freeMem(el->snps[i]);
freeMem(el->snps);
/* All strings in snpObs are allocated at once, so only need to free first. */
for (i = 0 ; i < el->snpCount; i++)
    if (el->snpObs[i] != NULL)
        freeMem(el->snpObs[i]);
freeMem(el->snpObs);
freez(pEl);
}

void misMatchFreeList(struct misMatch **pList)
/* Free a list of dynamically allocated misMatch's */
{
struct misMatch *el, *next;

for (el = *pList; el != NULL; el = next)
    {
    next = el->next;
    misMatchFree(&el);
    }
*pList = NULL;
}


void newMisMatches(struct misMatch **misMatchList, char *name, int offset  , char genomeBase, char mrnaBase, 
        char *chrom, int chromStart, int mrnaLoc, char strand, struct loci *lociList, struct snp125 *snpList)
/* add new mismatch to list, figure out which loci we are mapped to */
{
struct loci *l = NULL;
struct misMatch *misMatch = NULL;

for (l = lociList ; l != NULL ; l = l->next)
    {
    int index = getLoci(lociList, chrom, chromStart);
    if (index == l->index)
            misMatch = newMisMatch(name, offset, genomeBase, mrnaBase, chrom, chromStart, mrnaLoc, strand, 
                    l->index, lociList, snpList);
    else
            misMatch = newMisMatch(name, offset, '-', mrnaBase, na, -1, mrnaLoc, '.' , 
                    l->index, lociList, snpList);
    slAddHead(misMatchList, misMatch);
    }
}

struct mrnaMisMatch *tabulateMisMatches(struct misMatch *mm, int seqCount, struct loci *lociList)
/* get all mismatches for an mRNA and group all mismatches by position in mrna */
{
struct misMatch *mme = NULL;
struct mrnaMisMatch *mrnaMisMatch = NULL, *misMatchList = NULL;
int prevLoc = -99;

if (mm == NULL)
    return NULL;
for (mme = mm ; mme != NULL ; mme = mme->next)
    {
    int i = mme->loci;
    int j = 0;
    if (i >= seqCount)
        assert (i < seqCount);
    if (prevLoc != mme->mrnaLoc)
        {
        AllocVar(mrnaMisMatch);
        AllocArray(mrnaMisMatch->bases, seqCount+1);
        AllocArray(mrnaMisMatch->strands, seqCount+1);
        slAddHead(&misMatchList, mrnaMisMatch);
        mrnaMisMatch->tStarts = AllocArray(mrnaMisMatch->tStarts, seqCount);
        mrnaMisMatch->name = cloneString(mme->name);
        mrnaMisMatch->misMatchCount = seqCount;
        mrnaMisMatch->mrnaBase = mme->mrnaBase;
        mrnaMisMatch->mrnaLoc = mme->mrnaLoc;
        AllocArray(mrnaMisMatch->chroms, seqCount);
        AllocArray(mrnaMisMatch->loci, seqCount);
        for (j = 0 ; j<seqCount; j++)
            {
            /* default to empty alignment */
            mrnaMisMatch->bases[j] = '.';
            mrnaMisMatch->tStarts[j] = 0;
            mrnaMisMatch->chroms[j] = cloneString(na);
            mrnaMisMatch->loci[j] = j /* NOVALUE mme->loci*/ ;
            }
        mrnaMisMatch->bases[seqCount] = '\0';
        mrnaMisMatch->snpCount = 0;
        AllocArray(mrnaMisMatch->snps, seqCount);
        if (mme->snpCount > 0)
            {
            mrnaMisMatch->snps[i] = cloneString(mme->snps[0]);
            mrnaMisMatch->snpCount++;
            verbose(4,"mrnaloc %d mmsnp [%d] loci [%d] %s\n",
                    mrnaMisMatch->mrnaLoc,i, mme->loci, mrnaMisMatch->snps[i]);
            }
        }
    mrnaMisMatch->chroms[i] = cloneString(mme->chrom);
    mrnaMisMatch->tStarts[i] = mme->chromStart;
    mrnaMisMatch->bases[i] = mme->genomeBase;
    mrnaMisMatch->strands[i] = mme->strand;
    mrnaMisMatch->loci[i] = mme->loci;
    assert(mrnaMisMatch->loci[i] != NOVALUE);
    prevLoc = mme->mrnaLoc;
    }
slReverse(&misMatchList);
return misMatchList;
}

int misMatchCmpMrnaLoc(const void *va, const void *vb)
/* Compare to sort based on mrnaLoc. */
{
const struct misMatch *a = *((struct misMatch **)va);
const struct misMatch *b = *((struct misMatch **)vb);
return a->mrnaLoc - b->mrnaLoc;
}

int lociCmpSort(const void *va, const void *vb)
/* Compare to sort based on score desc. */
{
const struct loci *a = *((struct loci **)va);
const struct loci *b = *((struct loci **)vb);
return b->score - a->score;
}
bool compileOutput(char *name, struct misMatch *misMatchList, int seqCount, struct loci *lociList)
/* score predigested alignments  in misMatchList/lociList and output alignments to outFile 
   return false if list is empty , else true
   */
{
struct mrnaMisMatch *mrnaMm = NULL;
struct mrnaMisMatch *mrnaMisMatch = NULL;
struct bed *bed = NULL;
struct loci *l = NULL;
struct psl *psl = NULL;
int *missCount;             /* count of dings for each aligment of mrna to genome */
int *goodCount;             /* count of cases where alignment is better than others */
int *neither;             /* count of cases where the loci all have the same base */
int *gapCount;             /* count of cases where the loci are gaps*/
int *snpCount;             /* count of cases where the loci are snps*/
int indel = 0;             /* count of gaps in mrna alignment */
int maxScore = -99999;       /* max scoring alignment for this mrna */
int nextBestScore = -99999;       /* 2nd best scoring alignment for this mrna */
int maxHits = 0;       /* number of alignments with max score */
int qNameCount = 0;     /* count of filtered psls for this qName */
if (misMatchList == NULL) 
    return FALSE;
AllocArray(missCount,seqCount);
AllocArray(goodCount,seqCount);
AllocArray(gapCount,seqCount);
AllocArray(neither,seqCount);
AllocArray(snpCount,seqCount);
mrnaMm = tabulateMisMatches(misMatchList, seqCount, lociList);
for (mrnaMisMatch = mrnaMm ; mrnaMisMatch != NULL ; 
        mrnaMisMatch = mrnaMisMatch->next)
    {
    int i = 0;
    char b = toupper(mrnaMisMatch->bases[0]);
    bool lociDifferent = FALSE;
    bool atLeastOneMatch = FALSE;

    if (scoreFile != NULL)
        mrnaMisMatchTabOut(mrnaMisMatch,scoreFile) ;
   
    for (i = 0 ; i < seqCount; i++)
        {
        if (b == 'N')
            b = toupper(mrnaMisMatch->bases[i]);
        }
    /* check for mismatches between genomic loci */
    for (i = 0 ; i < mrnaMisMatch->misMatchCount ; i++)
        {
        char genomeBase = toupper(mrnaMisMatch->bases[i]) ;
        if (genomeBase != b && genomeBase != 'N' && b != 'N')
            lociDifferent = TRUE;
        if (mrnaMisMatch->bases[i] == mrnaMisMatch->mrnaBase)
            atLeastOneMatch = TRUE;
        }
    for (i = 0 ; i < mrnaMisMatch->misMatchCount ; i++)
        {
        char chrom[80];
        char name[80];
        unsigned index = mrnaMisMatch->loci[i];

        if (index == NOVALUE )
            {
            indel++;
            continue;
            }
        if (mrnaMisMatch->snpCount > 0 && mrnaMisMatch->snps[index] != NULL)
            {
            snpCount[index]++;
            verbose(4,"found snp %s count[%d] %d %s\n", mrnaMisMatch->snps[index], index, snpCount[index], mrnaMisMatch->chroms[index]);
            }
        if (mrnaMisMatch->bases[i] == '-' && atLeastOneMatch)
            indel++;
        if (!lociDifferent || !atLeastOneMatch)
            neither[index]++;
        if (lociDifferent && mrnaMisMatch->bases[index] == mrnaMisMatch->mrnaBase)
            goodCount[index]++;
        /* don't count indels as mismatches */
        if (atLeastOneMatch && lociDifferent && 
                mrnaMisMatch->bases[index] != mrnaMisMatch->mrnaBase && mrnaMisMatch->bases[i] != '-')
            {
            if (toupper(mrnaMisMatch->bases[index]) != 'N')
                {
                missCount[index]++;
                verbose(5,"missCount[%d]++ %d %c/%c\n",index, 
                        missCount[index], mrnaMisMatch->bases[index] ,mrnaMisMatch->mrnaBase );
                }
            else
                {
                gapCount[index]++;
                verbose(5,"gapCount[%d]++ %d\n",index, gapCount[index]);
                }
            }
        else
            verbose(9,"match not counted [%d] i=%d %c/%c/%c\n",
                    index, i, mrnaMisMatch->bases[index], mrnaMisMatch->mrnaBase, mrnaMisMatch->bases[i]);
        if (mrnaMisMatch->chroms[i] != NULL && differentString(mrnaMisMatch->chroms[i], "NA") &&
            (lociDifferent && mrnaMisMatch->bases[index] != mrnaMisMatch->mrnaBase))
            {
            safef(chrom, sizeof(chrom), "%s", mrnaMisMatch->chroms[i]);
            AllocVar(bed);
            bed->chrom = chrom;
            bed->chromStart = mrnaMisMatch->tStarts[i];
            bed->chromEnd = (bed->chromStart)+1;
            bed->strand[0] = mrnaMisMatch->strands[i];
            bed->strand[1] = (char)'\0';
            safef(name ,sizeof(name), "%s.%d.%c/%c", mrnaMisMatch->name, 
                mrnaMisMatch->mrnaLoc, mrnaMisMatch->mrnaBase, mrnaMisMatch->bases[i]);
            bed->name = name;
            if (bedFile != NULL)
                bedTabOutN(bed, 6, bedFile);
            }
            if (index != NOVALUE)
                {
                verbose(6,"snp index %d loc %d ", index, mrnaMisMatch->mrnaLoc); 
                verbose(6, " %s count[%d] %d %s\n", 
                        mrnaMisMatch->snps[index], index, snpCount[index], mrnaMisMatch->chroms[index]);
                }
            else
                verbose(6,"snp index %d loc %d \n", index, mrnaMisMatch->mrnaLoc);
        }
    }
mrnaMisMatchFreeList(&mrnaMm);

/* calculate scores for each loci*/
for (l = lociList ; l != NULL; l=l->next)
    {
    int z = l->index;
    psl = l->psl;
    int notAligned = psl->qSize-goodCount[z]-missCount[z]-indel-(psl->match);
    if (notAligned < 0) notAligned = 0;
    int score = goodCount[z]-missCount[z] - notAligned/notAlignPenalty;
    l->score = score;
    verbose(3, "%s %s:%d [%d] miss %d good %d neither %d indel %d total %d\
 gaps %d snp %d score %d unalg %d (-%d) match %d\n",
            name, l->chrom, l->chromStart, z, missCount[z], goodCount[z], neither[z], indel,
            missCount[z]+ goodCount[z]+ neither[z]+ indel, gapCount[z], snpCount[z],
            score, notAligned, notAligned/notAlignPenalty, psl->match);
    }

/* sort by score */
slSort(&lociList, lociCmpSort);

/* now grap the highest scoring alignment and output it if there are no others that score within minDiff points */
for (l = lociList ; l != NULL; l=l->next)
    {
    int score = l->score;
    if (maxScore == score)
        {
        maxHits ++;
        verbose(3,"%s score %d == maxScore %d next %d maxHits %d\n",
                name, score, maxScore, nextBestScore, maxHits);
        }
    else 
        {
        if (score > maxScore)
            {
            maxHits = 1;
            verbose(3,"%s score %d > maxScore %d nextBestScore %d \n",
                    name, score, maxScore, nextBestScore);
            }
        else if (score > nextBestScore ) 
            {
            nextBestScore = score;
            verbose(3,"%s score %d > nextBestScore %d (maxScore %d -score %d) > minDiff %d\n",
                    name, score, nextBestScore, maxScore, score, minDiff);
            }
    }
    maxScore = max(maxScore, score); 
    }

/* output results */
for (l = lociList ; l != NULL; l=l->next)
    {
    char *chrom;           /* best alignment */
    int chromStart;
    int chromEnd;
    int z = l->index;
    int spread = maxScore - nextBestScore;
    bool posOk = getLociPosition(lociList, l->index, &chrom, &chromStart, &chromEnd, &psl);
    int score = l->score;
    int diff =  score - nextBestScore;
    psl = l->psl;
    assert(psl != NULL);
    assert(l->index >= 0);
    if (scoreFile != NULL)
        fprintf(scoreFile, "##   %s score %d %s:%d [%d] mismatch %d good %d neither %d indel %d \
 gaps %d snpCount[%d] %d total %d nextBestScore %d diff %d index %d maxHITS %d posOk %d\n",
            name, score , l->chrom, l->chromStart, z, missCount[z], goodCount[z], neither[z], indel,
            missCount[z]+ goodCount[z]+ neither[z]+ indel, gapCount[z], z, snpCount[z],
            nextBestScore, diff ,l->index, maxHits, posOk);
    if (diff >= minDiff && spread >= minDiff /* z >= 0 *&& maxHits == 1 && */)
        {
        verbose(2, "%s bestHit score %d. diff %d and spread %d >= %d. %s:%d-%d [%d] mismatch %d good %d neither %d indel %d sum %d\
 gaps %d snps %d seqCnt-loci %d maxScore %d maxHITS %d 2nd best %d diff %d\n",
                name, score, diff , spread, minDiff , psl->tName , chromStart, chromEnd, z, 
                missCount[z], goodCount[z], neither[z], indel,
                missCount[z]+ goodCount[z]+ neither[z]+ indel, 
                gapCount[z], snpCount[z],
                seqCount - slCount(lociList), maxScore, maxHits, nextBestScore, diff);
        if (scoreFile)
            fprintf(scoreFile, "##>> %s score %d %s:%d-%d [%d] mismatch %d good %d neither %d indel %d \
 gaps %d snps %d total %d diff %d maxScore %d maxHITS %d 2nd best %d diff %d\n",
                name,  score, psl->tName , chromStart, chromEnd, z, 
                missCount[z], goodCount[z], neither[z], indel,
                missCount[z]+ goodCount[z]+ neither[z]+ indel, 
                gapCount[z], snpCount[z],
                seqCount - slCount(lociList), maxScore, maxHits, nextBestScore, diff);
        if (psl->strand[0] == '+' && psl->strand[1] == '-')
            pslRc(psl);
        if (psl->strand[1] == '+' || psl->strand[1] == '-') psl->strand[1] = '\0';
        pslTabOut(psl, outFile);
        outputCount++;
        filterCount++;
        qNameCount++;
        }
    else
        {
        verbose(2, "%s NO score %d diff %d spread %d max %d next %d maxHits %d index %d pos %s:%d diff %d < %d\n", 
                name, score , diff, spread,  maxScore, nextBestScore, maxHits, z, 
                psl->tName , psl->tStart, diff, minDiff);
        }
    }
freez(&missCount);
freez(&goodCount);
freez(&gapCount);
freez(&neither);
freez(&snpCount);
if (qNameCount > 0)
    return TRUE;
return FALSE;
}

bool isPureDna(char a)
/* return true of the base is a valid dna nt */
{
char b = tolower(a);
if (b == 'a' || b == 'c' || b == 'g' || b == 't')
    return TRUE;
else
    return FALSE;
}

void updateCount(char x1, char x2)
{
int a = tolower(x1);
int b = tolower(x2);
if (x1 == -1 )
    a = '.';
if (x2 == -1)
    b = '.';

if (isPureDna(a) && isPureDna(b))
    {
    histogram[a][b]++;
    }
else
    /* handle mismatches */
    {
    histogram[a][b]++;
    }
}

void normalizeSS()
{
unsigned int i, j;
int sum = 0;

for (i = 0 ; i < 256 ; i++)
    for (j = 0 ; j < 256 ; j++)
        sum += histogram[i][j] ;
for (i = 0 ; i < 256 ; i++)
    for (j = 0 ; j < 256 ; j++)
            histoNorm[i][j] = (float)histogram[i][j] / (float)sum ;
}

int getHistogram(char a, char b)
{
    return(histogram[(unsigned int)a][(unsigned int)b]);
}

float getHistoNorm(char a, char b)
{
    return(histoNorm[(unsigned int)a][(unsigned int)b]);
}

void initSS()
{
unsigned int i, j;

for (i = 0 ; i < 256 ; i++)
    for (j = 0 ; j < 256 ; j++)
        {
        histogram[i][j] = 0;
        histoNorm[i][j] = 0.0;
        }
}

void computeSuffStats(struct misMatch **misMatchList, struct alignment *align, struct loci *lociList)
/* compute sufficient statistics from each alignment */
{
int blockIx = 0;
struct psl *psl = align->psl;
static struct dnaSeq *tSeq = NULL;
int tStart = psl->tStart;
int tEnd   = psl->tEnd;
char genomeStrand = psl->strand[1] == '-' ? '-' : '+';
if (genomeStrand == '-')
    reverseIntRange(&tStart, &tEnd, psl->tSize);
for (blockIx=0; blockIx < psl->blockCount; ++blockIx)
    /* for each alignment block get sequence for both strands */
    {
    struct snp125 *snp = NULL, *snpList = NULL;
    if (hashFindVal(cdnaSeqFile->hash, psl->qName) == NULL)
        {
        verbose(5, "Skipping %s not found \n",psl->qName);
        return;
        }
    int size = twoBitSeqSize(cdnaSeqFile, psl->qName);
    int i = 0;
    int ts = psl->tStarts[blockIx];
    int te = psl->tStarts[blockIx]+(psl->blockSizes[blockIx]);
    int qe = psl->qStarts[blockIx]+(psl->blockSizes[blockIx]);
    struct dnaSeq *qSeq = NULL;
    if (qe > size)
        {
        verbose(5, "Skipping %s %d > %d \n",psl->qName, qe, size );
        return;
        }
    qSeq = twoBitReadSeqFrag(cdnaSeqFile, psl->qName, psl->qStarts[blockIx], 
            psl->qStarts[blockIx]+(psl->blockSizes[blockIx]));

    if (genomeStrand == '-')
        reverseIntRange(&ts, &te, psl->tSize);

    verbose(5,"  xyz %s t %s:%d-%d q %d-%d %s strand %c\n",
            psl->qName, psl->tName, ts, te, psl->qStarts[blockIx], 
            psl->qStarts[blockIx]+psl->blockSizes[blockIx], psl->strand, genomeStrand);
    tSeq = twoBitGetSeqForStrand(align->twoBitSeqFile, psl->tName,
            psl->tStarts[blockIx], 
            psl->tStarts[blockIx]+(psl->blockSizes[blockIx]), genomeStrand);

    verbose(6,"tSeq %s len %d %d\n",tSeq->dna, tSeq->size, (int)strlen(tSeq->dna));
    verbose(6,"qSeq %s\n",qSeq->dna);
    assert(psl->strand[0] == '+');

    /* count match and mismatches in each alignment block */
    for (i = 0 ; i < tSeq->size; i++)
        {
        char t = toupper(tSeq->dna[i]);
        char q = toupper(qSeq->dna[i]);
        int genomeStart = ts+i;

        if (q == 'n' || q == 'N')
            verbose(5, "N in sequence %s\n",psl->qName);
        updateCount(t,q);

        if (genomeStrand == '-')
            genomeStart = te-i-1;

        /* get overlapping snps */
        snpList = getSnpList(psl->tName, genomeStart, genomeStart+1, genomeStrand) ;
        for (snp = snpList ; snp != NULL ; snp = snp->next)
            {
            int offset = (snp->chromStart)-ts;
            char snpBase = ' ';
            assert(sameString(psl->tName, snp->chrom));
            if (genomeStrand == '-')
                offset = (tSeq->size - offset)-1;
            snpBase = toupper(tSeq->dna[offset]);
           if (snpBase != t)
               {
               verbose(4,"snp mismatch genome %c snpbase %c mrna %c ts %d snp %d valid %s\n",
                       t, snpBase, q, ts, snp->chromStart, snp->valid);
               }
           else
            verbose(4,"       [%d] snp match %s %s %s:%d %s %s offset %d gs %c ts %d genomic%c %c valid %s\n",
                    getLoci(lociList, psl->tName, psl->tStart), snp->name, psl->qName, snp->chrom, snp->chromStart, snp->observed, 
                    snp->strand, offset, genomeStrand, ts, genomeStrand, t, snp->valid);
            }
        }
    freeDnaSeq(&tSeq);
    freeDnaSeq(&qSeq);
    }
}

void addOtherAlignments( struct alignment *alignList, struct hash *speciesHash, char *name, struct twoBitFile *twoBitFile, char *mrnaPath)
{
/* add alignemnts from other species to the list */
struct hashEl *speciesList = NULL , *el = NULL;
struct alignment *align = NULL;

if (speciesHash != NULL)
    speciesList = hashLookup(speciesHash,name);
for (el = speciesList ; el != NULL ; el = el->next)
    {
    struct psl *psl = el->val;
    AllocVar(align);
    align->psl = psl;
    align->twoBitSeqFile = twoBitFile;
    align->mrnaPath = mrnaPath;
    slAddHead(&alignList, align);
    }
slReverse(&alignList);
}

void fillinMatches(struct misMatch **misMatchList, struct alignment *align, struct loci *lociList)
/* for each mismatch , lookup bases in other loci */
{
    int blockIx = 0;
    static struct dnaSeq *tSeq = NULL;

    struct psl *psl = align->psl;
    int tStart = psl->tStart;
    int tEnd   = psl->tEnd;
    char genomeStrand = psl->strand[1] == '-' ? '-' : '+';
    int index = getLoci(lociList, psl->tName, psl->tStart);


    if (genomeStrand == '-')
        reverseIntRange(&tStart, &tEnd, psl->tSize);
    for (blockIx=0; blockIx < psl->blockCount; ++blockIx)
        /* for each alignment block get sequence for both strands */
        {
        struct dnaSeq *qSeq = twoBitReadSeqFrag(cdnaSeqFile, psl->qName, psl->qStarts[blockIx], 
                psl->qStarts[blockIx]+(psl->blockSizes[blockIx]));
        int ts = psl->tStarts[blockIx];
        int te = psl->tStarts[blockIx]+(psl->blockSizes[blockIx]);
        int qs = psl->qStarts[blockIx];
        int qe = psl->qStarts[blockIx]+(psl->blockSizes[blockIx]);
        struct misMatch *mm = NULL;

        if (genomeStrand == '-')
            reverseIntRange(&ts, &te, psl->tSize);

        tSeq = twoBitGetSeqForStrand(align->twoBitSeqFile, psl->tName, psl->tStarts[blockIx], 
                psl->tStarts[blockIx]+(psl->blockSizes[blockIx]), genomeStrand);

        for (mm = *misMatchList ; mm != NULL ; mm = mm->next)
            {
            /* i = offset within the block */
            int i = mm->mrnaLoc - qs;
            int genomeStart = ts+i;
            if (genomeStrand == '-')
                genomeStart = te-i-1;
            /* if genomic chr not filled in yet (only mismatches are filled in ), and we are within the block, 
             * lookup genomic loc and base*/
            if (index == mm->loci)
                {
                if (sameString(mm->chrom , na) && 
                        mm->mrnaLoc >= qs && mm->mrnaLoc < qe && i >= 0)
                    {
                    char t = toupper(tSeq->dna[i]);
                    char q = toupper(qSeq->dna[i]);
                    if (q=='\0') q='-';
                    if (t=='\0') t='-';
                    if (i > 0 || i <= qe-qs) 
                        verbose(6," i %d qs %d qe %d\n",i,qs,qe);
                    assert (i > 0 || i <= qe-qs) ;
                    mm->chrom = cloneString(psl->tName);
                    mm->chromStart = genomeStart;
                    mm->genomeBase = t;
                    mm->strand = genomeStrand;
                    if (q!=(mm->mrnaBase) && q!='-')
                        {
                        verbose(2, "mismatch %s %s q %c != mmBase %c offset %d\n",psl->qName, psl->tName,q,(mm->mrnaBase), i );
                        assert(q==(mm->mrnaBase));
                        }
                    verbose(5,"   fM() FILL %s %c/%c t %s:%d q %d i %d qs %d qe %d %s %c loci %d BLK %d ",
                            psl->qName, t, q, psl->tName, 
                            genomeStart, mm->mrnaLoc, i, qs, qe ,psl->strand, genomeStrand, mm->loci, blockIx);
                    if (t==q)
                        verbose(5,"GOOD\n");
                    else
                        verbose(5,"MISMATCH\n");
                    }
                /* if we are outside the block, assume an indel */
                else if (mm->mrnaLoc >= qs && mm->mrnaLoc < qe)
                    {
                    mm->genomeBase = '-';
                    verbose(5,"   fM() INDEL %s t %s:%d mrnaLoc %d i %d not in %d-%d %s loci %d BLK %d\n",
                            psl->qName, psl->tName, 
                            psl->tStart, mm->mrnaLoc, i, qs, qe , psl->strand, mm->loci, blockIx);
                    }
                }
            }
        freeDnaSeq(&tSeq);
        freeDnaSeq(&qSeq);
        }
}

void buildMisMatches(struct misMatch **misMatchList, struct alignment *align, struct loci *lociList)
/* build list of mismatches for each alignment (loci) */
{
int blockIx = 0;
struct psl *psl = align->psl;
static struct dnaSeq *tSeq = NULL;
int tStart = psl->tStart;
int tEnd   = psl->tEnd;
int misMatchCount = 0;
int transitionCount = 0;
int transversionCount = 0;
char genomeStrand = psl->strand[1] == '-' ? '-' : '+';
int matchCount = 0;
for (blockIx=0; blockIx < psl->blockCount; ++blockIx)
    /* for each alignment block get sequence for both strands */
    {
    struct snp125 *snp = NULL, *snpList = NULL;
    struct dnaSeq *qSeq = twoBitReadSeqFrag(cdnaSeqFile, psl->qName, psl->qStarts[blockIx], 
            psl->qStarts[blockIx]+(psl->blockSizes[blockIx]));
    int i = 0;
    int ts = psl->tStarts[blockIx];
    int te = psl->tStarts[blockIx]+(psl->blockSizes[blockIx]);

    if (genomeStrand == '-')
        reverseIntRange(&ts, &te, psl->tSize);

    tSeq = twoBitGetSeqForStrand(align->twoBitSeqFile, psl->tName, psl->tStarts[blockIx], 
            psl->tStarts[blockIx]+(psl->blockSizes[blockIx]), genomeStrand);

    verbose(5,"  buildMisMatches for blk %s t %s:%d-%d q %d-%d %s strand %c\n",
            psl->qName, psl->tName, ts, te, psl->qStarts[blockIx], 
            psl->qStarts[blockIx]+psl->blockSizes[blockIx], psl->strand, genomeStrand);
    verbose(6,"tSeq %s len %d %d\n",tSeq->dna, tSeq->size, (int)strlen(tSeq->dna));
    verbose(6,"qSeq %s\n",qSeq->dna);
    assert(psl->strand[0] == '+');

    /* check for mismatches in exon (alignment block)*/
    for (i = 0 ; i < tSeq->size; i++)
        {
        char t = toupper(tSeq->dna[i]);
        char q = toupper(qSeq->dna[i]);
        
        if (t != q)
            /* add mismatch to list found between mRNA and genome */
            {
            int genomeStart = ts+i;
            int mrnaLoc = psl->qStarts[blockIx]+i;

            misMatchCount++;
            if (transition(t,q))
                transitionCount++;
            if (transversion(t,q))
                transversionCount++;

            if (genomeStrand == '-')
                genomeStart = te-i-1;

            /* get overlapping snps */
            snpList = getSnpList(psl->tName, genomeStart, genomeStart+1, genomeStrand) ;
            for (snp = snpList ; snp != NULL ; snp = snp->next)
                {
                int offset = (snp->chromStart)-ts;
                char snpBase = ' ';
                assert(sameString(psl->tName, snp->chrom));
                if (genomeStrand == '-')
                    offset = (tSeq->size - offset)-1;
                snpBase = toupper(tSeq->dna[offset]);
               if (snpBase != t)
                   {
                   verbose(4,"snp mismatch genome %c snpbase %c mrna %c ts %d snp %d valid %s\n",
                           t, snpBase, q, ts, snp->chromStart, snp->valid);
                   }
               else
                verbose(4,"       [%d] snp match %s %s %s:%d %s %s offset %d gs %c ts %d genomic%c %c valid %s\n",
                        getLoci(lociList, psl->tName, psl->tStart), snp->name, psl->qName, snp->chrom, snp->chromStart, snp->observed, 
                        snp->strand, offset, genomeStrand, ts, genomeStrand, t, snp->valid);
                }
           verbose(5,"     add mismatch genome %c mrna %c ts %d gn %d offset %d mrnaLoc %d\n",
                       t, q, ts, genomeStart, i, mrnaLoc);
            newMisMatches(misMatchList, psl->qName, i ,t, q, psl->tName, 
                    genomeStart, mrnaLoc, genomeStrand, lociList, snpList);
            }
        else
            {
            matchCount++;
            }
        }
    printMisMatch(misMatchList);
    freeDnaSeq(&tSeq);
    freeDnaSeq(&qSeq);
    }
    verbose(4,"buildMismatches final score %s:%d-%d %s %s #match %d misMatch %d \n",
            psl->tName, tStart, tEnd, psl->strand,
            psl->qName, matchCount, misMatchCount);
}

void doOneMrna(char *name, struct alignment *alignList)
/*  build list of mismatches based on all alignments (defined by locilist)
   of mrna to genome */ 
{
struct misMatch *misMatchList = NULL;
struct alignment *align = NULL;
int seqCount = slCount(alignList);
struct loci *lociList = NULL;

mrnaCount++;
lociCounter = 0;
if (seqCount == 1)
    {
    struct psl *psl = alignList->psl;
    if (psl == NULL)
        return;
    if (psl->strand[0] == '+' && psl->strand[1] == '-')
        pslRc(psl);
    if (psl->strand[1] == '+' || psl->strand[1] == '-') psl->strand[1] = '\0';
    pslTabOut(psl, outFile);
    outputCount++;
    filterCount++;
    return;
    }
/* one loci for each place the mrna alignments to the genome */
for (align = alignList ; align != NULL ; align= align->next)
    {
    struct psl *psl = align->psl;
    addLoci(&lociList, psl);
    verbose(5,"add loci for %s:%d-%d\n",
            psl->tName, psl->tStart, psl->tEnd);
    }

verbose(5, "name %s alignList %d loci %d \n",
        name, slCount(alignList), slCount(lociList));

if (alignList != NULL) 
    verbose(5,"buildMisMatch %s aList count %d \n",name, slCount(alignList));
for (align = alignList; align != NULL ; align = align->next)
    {
    /* for each alignment, build mismatch list */
    verbose(5,"buildMisMatch %s tName %s:%d-%d\n",
            name, align->psl->tName, align->psl->tStart, align->psl->tEnd);
    if (computeSS)
        computeSuffStats(&misMatchList, align, lociList);
    else
        buildMisMatches(&misMatchList, align, lociList);
    }
slReverse(&misMatchList);
if (!computeSS )
    {
    if (misMatchList != NULL)
        {
        /* sort list by mrna position */
        slSort(&misMatchList, misMatchCmpMrnaLoc);
        verbose(2, "sort on mrna loc compile %s mismatchList %d lociList %d of %d alist %d\n",
                name, slCount(misMatchList), seqCount, slCount(lociList), slCount(alignList));
        for (align = alignList; align != NULL ; align = align->next)
            {
            int index = getLoci(lociList, align->psl->tName, align->psl->tStart);
            /* check for matches or indels in other loci */
            verbose(6, "CALL FILLINMATCHES() %s:%d index %d\n",
                    align->psl->tName, align->psl->tStart, index);
            fillinMatches(&misMatchList, align, lociList);
            }
        }
    /* compute mismatches and output alignments,
       if best hit not found and passthru is true,  copy input to output */
    if (!compileOutput(name, misMatchList, seqCount, lociList) &&  passthru)
        {
        for (align = alignList; align != NULL ; align = align->next)
            {
            if (align->psl->strand[0] == '+' && align->psl->strand[1] == '-')
                pslRc(align->psl);
            if (align->psl->strand[1] == '+' || align->psl->strand[1] == '-') align->psl->strand[1] = '\0';
            pslTabOut(align->psl, outFile);
            outputCount++;
            }
        filterCount++;;
        }
    misMatchFreeList(&misMatchList);
    }
else if (computeSS)
    {
    }

lociFreeList(&lociList);
}

void pslCDnaGenomeMatch(char *inName)
{
struct psl *psl = NULL, *pslList = NULL ;
struct alignment *subList = NULL;
char lastName[512];
unsigned int i , j;

pslList = readPslList(inName);

/* rev compl alignments so mrna is always on + strand */
/* Note: this should flip the loci also */
safef(lastName,sizeof(lastName), "%s", pslList->qName);
psl = pslList;
while (psl != NULL )
    {
    bool rm = FALSE;
    struct psl *newPsl = NULL;
    struct alignment *align = NULL;
   
    if (psl->strand[0] == '-')
        {
        pslRc(psl);
        verbose(5, "REVERSE %s %s \n",psl->qName, psl->tName);
        }
    if (differentString(lastName, psl->qName) && subList != NULL && lastName != NULL)
	{
        slSort(&subList, pslCmpMatch);
        verbose(2, "sort on match pslList %d subList %d last %s\n",slCount(pslList), slCount(subList), lastName);
	doOneMrna(lastName, subList);
	alignFreeList(&subList);
        verbose(5, "3 pslList %d\n",slCount(pslList));
	safef(lastName, sizeof(lastName), "%s", psl->qName);
	}
    AllocVar(align);
    newPsl = psl;
    psl = psl->next;
    align->psl = newPsl; 
    align->db = "db";
    align->twoBitSeqFile = genomeSeqFile;
    align->mrnaPath = NULL;
    rm = slRemoveEl(&pslList, newPsl);
    if (rm)
        {
        slAddHead(&subList, align);
        verbose(5, "add %s %s\n",align->psl->qName, align->psl->tName);
        }
    }
verbose(5,"last doOneMrna\n");
slSort(&subList, pslCmpMatch);
doOneMrna(lastName, subList);
alignFreeList(&subList);
verbose(1,"Wrote %d alignments out of %d \n", outputCount, aliCount);
verbose(1,"%d out of %d mRNAs passed strict criteria (pass thru not counted)\n", filterCount, mrnaCount);
if (computeSS)
    {
    fprintf(outFile, "%8d %8d %8d %8d  %8d %8d %8d %8d  %8d %8d %8d %8d  %8d %8d %8d %8d \n",
            getHistogram('a','a'), getHistogram('a','c'), getHistogram('a','g'), getHistogram('a','t'), 
            getHistogram('c','a'), getHistogram('c','c'), getHistogram('c','g'), getHistogram('c','t'), 
            getHistogram('g','a'), getHistogram('g','c'), getHistogram('g','g'), getHistogram('g','t'), 
            getHistogram('t','a'), getHistogram('t','c'), getHistogram('t','g'), getHistogram('t','t') 
           );
    for (i = 0 ; i < 128 ; i++)
        for (j = 0 ; j < 128 ; j++)
            if (getHistogram(i,j) > 0)
                fprintf(outFile, "%d %d %c %c = %d \n",i, j, (char)i, (char)j, getHistogram(i,j)) ;
    normalizeSS();
    fprintf(outFile, "%5.5f %5.5f %5.5f %5.5f  %5.5f %5.5f %5.5f %5.5f  %5.5f %5.5f %5.5f %5.5f  %5.5f %5.5f %5.5f %5.5f \n",
            getHistoNorm('a','a'), getHistoNorm('a','c'), getHistoNorm('a','g'), getHistoNorm('a','t'), 
            getHistoNorm('c','a'), getHistoNorm('c','c'), getHistoNorm('c','g'), getHistoNorm('c','t'), 
            getHistoNorm('g','a'), getHistoNorm('g','c'), getHistoNorm('g','g'), getHistoNorm('g','t'), 
            getHistoNorm('t','a'), getHistoNorm('t','c'), getHistoNorm('t','g'), getHistoNorm('t','t') 
           );
    fprintf(outFile, "      A       C       G       T\nA %5.5f %5.5f %5.5f %5.5f\nC %5.5f %5.5f %5.5f %5.5f\nG %5.5f %5.5f %5.5f %5.5f\nT %5.5f %5.5f %5.5f %5.5f\n",
            getHistoNorm('a','a'), getHistoNorm('a','c'), getHistoNorm('a','g'), getHistoNorm('a','t'), 
            getHistoNorm('c','a'), getHistoNorm('c','c'), getHistoNorm('c','g'), getHistoNorm('c','t'), 
            getHistoNorm('g','a'), getHistoNorm('g','c'), getHistoNorm('g','g'), getHistoNorm('g','t'), 
            getHistoNorm('t','a'), getHistoNorm('t','c'), getHistoNorm('t','g'), getHistoNorm('t','t') 
           );
    fprintf(outFile,"Transitions %5.5f Transversions %5.5f Matches %5.5f\n",
            getHistoNorm('a','g')+ getHistoNorm('g','a')+ getHistoNorm('c','t')+ getHistoNorm('t','c'), 
            getHistoNorm('a','c')+ getHistoNorm('a','t')+ getHistoNorm('c','a')+ getHistoNorm('c','g')+ 
            getHistoNorm('g','c')+ getHistoNorm('g','t')+ getHistoNorm('t','a')+ getHistoNorm('t','g'), 
            getHistoNorm('a','a')+ getHistoNorm('c','c')+ getHistoNorm('g','g')+ getHistoNorm('t','t')
           );
    fprintf(outFile,"acgt N %f \n",
            getHistoNorm('a','n')+ getHistoNorm('c','n')+ getHistoNorm('g','n')+ getHistoNorm('t','n') );
    fprintf(outFile,"N acgt %f \n",
            getHistoNorm('n','a')+ getHistoNorm('n','c')+ getHistoNorm('n','g')+ getHistoNorm('n','t') );

    for (i = 0 ; i < 128 ; i++)
        for (j = 0 ; j < 128 ; j++)
            if (getHistogram(i,j) > 0)
                fprintf(outFile, "%c %c = %f \n",(char)i, (char)j, getHistoNorm(i,j)) ;
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc != 6)
    usage();
lociCounter = 0;
fileCache = newDlList();
verbosity = optionInt("verbose", verbosity);
verboseSetLogFile("stdout");
verboseSetLevel(verbosity);
ss = axtScoreSchemeDefault();
cdnaSeqFile = twoBitOpen(argv[3]);
genomeSeqFile = twoBitOpen(argv[4]);
outFile = fopen(argv[5],"w");

initSS();
minDiff = optionInt("minDiff", MINDIFF);
notAlignPenalty = optionInt("notAlignPenalty", NOTALIGNEDFACTOR);
snpFile = optionVal("snp", NULL);
if (snpFile != NULL)
    {
    verbose(1,"Reading snps from %s\n",snpFile);
    snpHash = readSnpToBinKeeper(argv[2],snpFile);
    }
scoreOut = optionVal("score", NULL);
if (scoreOut != NULL)
    scoreFile = fopen(scoreOut,"w");
bedOut = optionVal("bedOut", NULL);
if (bedOut != NULL)
    bedFile = fopen(bedOut,"w");
species1 = optionVal("species1", NULL);
if (species1 != NULL)
    species1Hash = readPslQnameToHash(species1);
species2 = optionVal("species2", NULL);
if (species2 != NULL)
    species2Hash = readPslQnameToHash(species2);
nibdir1 = optionVal("nibdir1", NULL);
nibdir2 = optionVal("nibdir2", NULL);
mrna1 = optionVal("mrna1", NULL);
mrna2 = optionVal("mrna2", NULL);
passthru = optionExists("passthru");
verbose(1,"Reading alignments from %s\n",argv[1]);
pslCDnaGenomeMatch(argv[1]);
fclose(outFile);
if (bedOut != NULL)
    fclose(bedFile);
if (scoreOut != NULL)
    fclose(scoreFile);
freeDlList(&fileCache);
twoBitClose(&cdnaSeqFile);
twoBitClose(&genomeSeqFile);
return(0);
}
