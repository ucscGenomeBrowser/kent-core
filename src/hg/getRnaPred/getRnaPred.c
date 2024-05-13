/* getRnaPred - Get RNA for gene predictions. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "genePred.h"
#include "psl.h"
#include "fa.h"
#include "jksql.h"
#include "hdb.h"
#include "nibTwo.h"
#include "dnautil.h"
#include "options.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "getRnaPred - Get genome putative RNA for gene predictions\n"
  "usage:\n"
  "   getRnaPred [options] database table chromosome output.fa\n"
  "table can be a table or a file.  Specify chromosome of 'all' to\n"
  "to process all chromosome\n"
  "\n"
  "options:\n"
  "   -weird - only get ones with weird splice sites\n"
  "   -cdsUpper - output CDS in upper case\n"
  "   -cdsOnly - only output CDS\n"
  "   -cdsOut=file - write CDS to this tab-separated file, in the form\n"
  "      acc  start  end\n"
  "    where start..end are genbank style, one-based coordinates\n"
  "   -keepMasking - un/masked in upper/lower case.\n"
  "   -pslOut=psl - output a PSLs for the genomic mRNAs.  Allows \n"
  "    mRNA to be analyzed by tools that work on PSLs\n"
  "   -suffix=suf - append suffix to each id to avoid confusion with mRNAs\n"
  "    use to define the genes.\n"
  "   -peptides - out the translation of the CDS to a peptide sequence.\n"
  "    The newer program genePredToProt maybe produce better results in cases\n"
  "    were there are frame-shifting indels in the CDS.\n"
  "   -exonIndices - output indices of exon boundaries after sequence name,\n"
  "    e.g., \"103 243 290\" says positions 1-103 are from the first exon,\n"
  "    positions 104-243 are from the second exon, etc. \n"
  "   -maxSize=size - output a maximum of size characters.  Useful when\n"
  "    testing gene predictions by RT-PCR.\n"
  "   -genomeSeqs=spec - get genome sequences from the specified nib directory\n"
  "    or 2bit file instead of going though the path found in chromInfo.\n"
  "   -includeCoords - include the genomic coordinates as a comment in the\n"
  "    fasta header.  This is necessary when there are multiple genePreds\n"
  "    with the same name.\n"
  "   -genePredExt - (for use with -peptides) use extended genePred format,\n"
  "    and consider frame information when translating (Warning: only\n"
  "    considers offset at 5' end, not frameshifts between blocks)\n"
  );
}

static struct optionSpec options[] = {
   {"weird", OPTION_BOOLEAN},
   {"cdsUpper", OPTION_BOOLEAN},
   {"cdsOut", OPTION_STRING},
   {"cdsOnly", OPTION_BOOLEAN},
   {"keepMasking", OPTION_BOOLEAN},
   {"pslOut", OPTION_STRING},
   {"suffix", OPTION_STRING},
   {"peptides", OPTION_BOOLEAN},
   {"includeCoords", OPTION_BOOLEAN},
   {"exonIndices", OPTION_BOOLEAN},
   {"maxSize", OPTION_INT},
   {"genePredExt", OPTION_BOOLEAN},
   {"genomeSeqs", OPTION_STRING},
   {NULL, 0}
};


/* parsed from command line */
boolean weird, cdsUpper, cdsOnly, peptides, keepMasking, includeCoords, exonIndices,
  genePredExt;
char *cdsOut = NULL;
char *pslOut = NULL;
char *suffix = "";
int maxSize = -1;
char *genomeSeqs = NULL;

struct nibTwoCache *getNibTwoCacheFromDb(char *db)
/* get the nib or two-bit cache from database */
{
struct sqlConnection *conn = hAllocConn(db);
char nibTwoPath[PATH_LEN];
struct nibTwoCache *nibTwoCache;

/* grab the first chromsome file name, if it's a nib, convert to
 * directory name */
char query[1024];
sqlSafef(query, sizeof query, "select fileName from chromInfo limit 1");
sqlNeedQuickQuery(conn, query,
                  nibTwoPath, sizeof(nibTwoPath));
if (nibIsFile(nibTwoPath))
    {
    char *p = strrchr(nibTwoPath, '/');
    if (p != NULL)
        *p = '\0';
    else
        strcpy(nibTwoPath, "."); 
    }
nibTwoCache = nibTwoCacheNew(nibTwoPath);
hFreeConn(&conn);
return nibTwoCache;
}

struct nibTwoCache *getNibTwoCache(char *db)
/* get the nib or two-bit cache */
{
if (genomeSeqs != NULL)
    return nibTwoCacheNew(genomeSeqs);
else
    return getNibTwoCacheFromDb(db);
}

struct dnaSeq *fetchDna(char *db, char *seqName, int start, int end, enum dnaCase dnaCase)
/* Fetch DNA sequence, with caching of opens */
{
static struct nibTwoCache *nibTwoCache = NULL;
struct dnaSeq *dna;
if (nibTwoCache == NULL)
    nibTwoCache = getNibTwoCache(db);
dna = nibTwoCacheSeqPart(nibTwoCache, seqName, start, (end-start), NULL);
if (dnaCase == dnaLower)
    tolowers(dna->dna);
return dna;
}

boolean hasWeirdSplice(char *db, struct genePred *gp)
/* see if a gene has weird splice sites */
{
int i;
boolean gotOdd = FALSE;
for (i=1; (i<gp->exonCount) && !gotOdd; ++i)
    {
    int start = gp->exonEnds[i-1];
    int end = gp->exonStarts[i];
    int size = end - start;
    if (size > 0)
        {
        struct dnaSeq *seq = fetchDna(db, gp->chrom, start, end, dnaLower);
        DNA *s = seq->dna;
        DNA *e = seq->dna + seq->size;
        uglyf("%s %c%c/%c%c\n", gp->name, s[0], s[1], e[-2], e[-1]);
        if (gp->strand[0] == '-')
            reverseComplement(seq->dna, seq->size);
        if (s[0] != 'g' || (s[1] != 't' && s[1] != 'c') || e[-2] != 'a' || e[-1] != 'g')
            gotOdd = TRUE;
        freeDnaSeq(&seq);
        }
    }
return gotOdd;
}

int findGeneOff(struct genePred *gp, int chrPos)
/* find the mrna offset containing the specified position.*/
{
int iRna = 0, iExon;

for (iExon = 0; iExon < gp->exonCount; iExon++)
    {
    if (chrPos < gp->exonStarts[iExon])
        {
        /* intron before this exon */
        return iRna;
        }
    if (chrPos < gp->exonEnds[iExon])
        {
        return iRna + (chrPos - gp->exonStarts[iExon]);
        }
    iRna += (gp->exonEnds[iExon] - gp->exonStarts[iExon]);
    }
return iRna-1;
}

void processCds(struct genePred *gp, struct dyString *dnaBuf, struct dyString *cdsBuf,
                FILE* cdsFh)
/* find the CDS bounds in the mRNA defined by the annotation.
 * output and/or update as requested.  If CDS buf is not NULL,
 * copy CDS to it.*/
{
int cdsStart = findGeneOff(gp, gp->cdsStart);
int cdsEnd = findGeneOff(gp, gp->cdsEnd-1)+1;
int rnaSize = genePredBases(gp);
if (gp->strand[0] == '-')
    reverseIntRange(&cdsStart, &cdsEnd, rnaSize);
assert(cdsStart <= cdsEnd);
assert(cdsEnd <= rnaSize);

if (cdsUpper)
    toUpperN(dnaBuf->string+cdsStart, (cdsEnd-cdsStart));

if (cdsBuf != NULL)
    {
    dyStringClear(cdsBuf);
    dyStringAppendN(cdsBuf, dnaBuf->string+cdsStart, (cdsEnd-cdsStart));
    }

if (cdsFh != NULL)
    fprintf(cdsFh,"%s\t%d\t%d\n", gp->name, cdsStart+1, cdsEnd);
}

void writePsl(char *db, struct genePred *gp, FILE* pslFh)
/* create a PSL for the genomic mRNA */
{
int rnaSize = genePredBases(gp);
int qStart = 0, iExon;
struct psl psl;
ZeroVar(&psl);

psl.match = rnaSize;
psl.tNumInsert = gp->exonCount-1;
psl.strand[0] = gp->strand[0];
psl.qName = gp->name;
psl.qSize = rnaSize;
psl.qStart = 0;
psl.qEnd = rnaSize;
psl.tName = gp->chrom;
psl.tSize =  hChromSize(db, gp->chrom);
psl.tStart = gp->txStart;
psl.tEnd = gp->txEnd;

/* fill in blocks in chrom order */
AllocArray(psl.blockSizes, 3 * gp->exonCount);
psl.qStarts = psl.blockSizes + gp->exonCount;
psl.tStarts = psl.qStarts + gp->exonCount;

for (iExon = 0; iExon < gp->exonCount; iExon++)
    {
    int exonSize = gp->exonEnds[iExon] - gp->exonStarts[iExon];
    int qEnd = qStart + exonSize;
    psl.blockSizes[iExon] = exonSize;
    psl.qStarts[iExon] = qStart;
    psl.tStarts[iExon] = gp->exonStarts[iExon];
    if (iExon > 0)
        {
        /* count intron as bases inserted */
        psl.tBaseInsert += gp->exonStarts[iExon]-gp->exonEnds[iExon-1];
        }
    psl.blockCount++;
    qStart = qEnd;
    }
assert(qStart == rnaSize);

if (pslCheck("from genePred", stderr, &psl) > 0)
    {
    fprintf(stderr, "psl: ");
    pslTabOut(&psl, stderr);
    errAbort("invalid psl created");
    }
pslTabOut(&psl, pslFh);
freeMem(psl.blockSizes);
}

void outputPeptide(struct genePred *gp, char *name, struct dyString *cdsBuf, FILE* faFh)
/* output the peptide sequence */
{
int offset = 0;

/* get frame offset, if available and needed */
if (gp->exonFrames != NULL) 
{
    if (gp->strand[0] == '+' && gp->cdsStartStat != cdsComplete)
        offset = (3 - gp->exonFrames[0]) % 3;
    else if (gp->strand[0] == '-' && gp->cdsEndStat != cdsComplete)
        offset = (3 - gp->exonFrames[gp->exonCount-1]) % 3;
}
/* NOTE: this fix will not handle the case in which frame is shifted
 * internally or at multiple exons, as when frame-shift gaps occur in
 * an alignment of an mRNA to the genome.  */

/* just overwrite the buffer with the peptide, which will stop at end of DNA
 * if no stop codon. Buffer size must allow for stop codon. */
int ir, ip;
boolean isChrM = isMito(gp->chrom);
for (ir = offset, ip = 0; ir < cdsBuf->stringSize; ir += 3, ip++)
    {
    cdsBuf->string[ip] = (isChrM ? lookupMitoCodon(cdsBuf->string+ir)
                          : lookupCodon(cdsBuf->string+ir));
    }
cdsBuf->string[ip] = '\0';
  
faWriteNext(faFh, name, cdsBuf->string, strlen(cdsBuf->string));
}

void processGenePred(char *db, struct genePred *gp, struct dyString *dnaBuf, 
                     struct dyString *cdsBuf, struct dyString *nameBuf,
                     FILE* faFh, FILE* cdsFh, FILE* pslFh)
/* output genePred DNA, check for weird splice sites if requested */
{
int i;
int index = 0;

/* Load exons one by one into dna string. */
dyStringClear(dnaBuf);
dyStringClear(nameBuf);
dyStringAppend(nameBuf, gp->name);
dyStringAppend(nameBuf, suffix);
if (exonIndices || includeCoords)
    dyStringPrintf(nameBuf, " %s", db);  /* we'll also include the db */
if (includeCoords)
    dyStringPrintf(nameBuf, " %s:%d-%d", gp->chrom, gp->txStart, gp->txEnd);
for (i=0; i<gp->exonCount; ++i)
    {
    int start = gp->exonStarts[i];
    int end = gp->exonEnds[i];
    int size = end - start;
    if (size < 0)
        warn("%d sized exon in %s\n", size, gp->name);
    else
        {
        struct dnaSeq *seq = fetchDna(db, gp->chrom, start, end, (keepMasking ? dnaMixed : dnaLower));
        dyStringAppendN(dnaBuf, seq->dna, size);
        freeDnaSeq(&seq);
        }
    }

/* Reverse complement if necessary */
if (gp->strand[0] == '-')
    reverseComplement(dnaBuf->string, dnaBuf->stringSize);

/* create list of exon indices, if necessary */
if (exonIndices) 
    {
    for (i = (gp->strand[0] == '-' ? gp->exonCount-1 : 0); 
         i >= 0 && i < gp->exonCount; 
         i += (gp->strand[0] == '-' ? -1 : 1))
                                /* use forward order if plus strand
                                   and reverse order if minus
                                   strand */
        {
        index += gp->exonEnds[i] - gp->exonStarts[i];
        if (maxSize != -1 && index > maxSize) index = maxSize;
        dyStringPrintf(nameBuf, " %d", index);
        if (index == maxSize) break; /* can only happen if maxSize != -1 */
        }
    } 

if ((gp->cdsStart < gp->cdsEnd)
    && (cdsUpper || cdsOnly || peptides || (cdsFh != NULL)))
    processCds(gp, dnaBuf, cdsBuf, cdsFh);

if (cdsOnly)
    faWriteNext(faFh, nameBuf->string, cdsBuf->string, 
                (maxSize != -1 && cdsBuf->stringSize > maxSize) ? maxSize : 
                cdsBuf->stringSize);
else if (peptides)
    {
    if (gp->cdsStart < gp->cdsEnd)
	outputPeptide(gp, nameBuf->string, cdsBuf, faFh);
    }
else
    faWriteNext(faFh, nameBuf->string, dnaBuf->string, 
                (maxSize != -1 && dnaBuf->stringSize > maxSize) ? maxSize : 
                dnaBuf->stringSize);

if (pslFh != NULL)
    writePsl(db, gp, pslFh);
}

void getRnaForTable(char *db, char *table, char *chrom, struct dyString *dnaBuf, 
                    struct dyString *cdsBuf, struct dyString *nameBuf,
                    FILE *faFh, FILE *cdsFh, FILE* pslFh)
/* get RNA for a genePred table */
{
int rowOffset;
char **row;
struct sqlConnection *conn = hAllocConn(db);
struct sqlResult *sr;

if (!sqlTableExists(conn, table))
    errAbort("table or file \"%s\" does not exists", table);

sr = hChromQuery(conn, table, chrom, NULL, &rowOffset);
while ((row = sqlNextRow(sr)) != NULL)
    {
    /* Load gene prediction from database. */
    struct genePred *gp;
    if (genePredExt)
      gp = genePredExtLoad(row+rowOffset, GENEPREDX_NUM_COLS);
    else
      gp = genePredLoad(row+rowOffset);
    if ((!weird) || hasWeirdSplice(db, gp))
        processGenePred(db, gp, dnaBuf, cdsBuf, nameBuf, faFh, cdsFh, pslFh);
    genePredFree(&gp);
    }
sqlFreeResult(&sr);
hFreeConn(&conn);
}

void getRnaForTables(char *db, char *table, char *chrom, struct dyString *dnaBuf, 
                     struct dyString *cdsBuf, struct dyString *nameBuf,
                     FILE *faFh, FILE *cdsFh, FILE* pslFh)
/* get RNA for one for possibly splite genePred table */
{
struct slName* chroms = NULL, *chr;
if (sameString(chrom, "all"))
    chroms = hAllChromNames(db);
else
    chroms = slNameNew(chrom);
for (chr = chroms; chr != NULL; chr = chr->next)
    getRnaForTable(db, table, chr->name, dnaBuf, cdsBuf, nameBuf, faFh, cdsFh, pslFh);
}

void getRnaForFile(char *db, char *table, char *chrom, struct dyString *dnaBuf, 
                   struct dyString *cdsBuf, struct dyString *nameBuf,
                   FILE *faFh, FILE* cdsFh, FILE* pslFh)
/* get RNA for a genePred file */
{
boolean all = sameString(chrom, "all");
struct lineFile *lf = lineFileOpen(table, TRUE);
char *row[GENEPREDX_NUM_COLS];
while (lineFileNextRowTab(lf, row, genePredExt ? 
                          GENEPREDX_NUM_COLS : GENEPRED_NUM_COLS))
    {
    struct genePred *gp;
    if (genePredExt) 
      gp = genePredExtLoad(row, GENEPREDX_NUM_COLS);
    else 
      gp = genePredLoad(row);
    if (all || sameString(gp->chrom, chrom))
        processGenePred(db, gp, dnaBuf, cdsBuf, nameBuf, faFh, cdsFh, pslFh);
    genePredFree(&gp);
    }
lineFileClose(&lf); 
}

void getRnaPred(char *db, char *table, char *chrom, char *faOut)
/* getRna - Get RNA for gene predictions and write to file. */
{
struct dyString *dnaBuf = dyStringNew(16*1024);
struct dyString *cdsBuf = NULL;
struct dyString *nameBuf = dyStringNew(64);
FILE *faFh = mustOpen(faOut, "w");
FILE *cdsFh = NULL;
FILE *pslFh = NULL;
if (cdsOnly || peptides)
    cdsBuf = dyStringNew(16*1024);
dyStringNew(16*1024);
if (cdsOut != NULL)
    cdsFh = mustOpen(cdsOut, "w");
if (pslOut != NULL)
    pslFh = mustOpen(pslOut, "w");
if (exonIndices)
   nameBuf = dyStringNew(512);

if (fileExists(table))
    getRnaForFile(db, table, chrom, dnaBuf, cdsBuf, nameBuf, faFh, cdsFh, pslFh);
else
    getRnaForTables(db, table, chrom, dnaBuf, cdsBuf, nameBuf, faFh, cdsFh, pslFh);

dyStringFree(&dnaBuf);
dyStringFree(&cdsBuf);
dyStringFree(&nameBuf);
carefulClose(&pslFh);
carefulClose(&cdsFh);
carefulClose(&faFh);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 5)
    usage();
dnaUtilOpen();
weird = optionExists("weird");
cdsUpper = optionExists("cdsUpper");
cdsOnly = optionExists("cdsOnly");
cdsOut = optionVal("cdsOut", NULL); 
keepMasking = optionExists("keepMasking"); 
pslOut = optionVal("pslOut", NULL); 
suffix = optionVal("suffix", suffix); 
peptides = optionExists("peptides");
includeCoords = optionExists("includeCoords");
exonIndices = optionExists("exonIndices");
maxSize = optionInt("maxSize", -1);
genePredExt = optionExists("genePredExt");
if (cdsOnly && peptides)
    errAbort("can't specify both -cdsOnly and -peptides");
if (cdsUpper && keepMasking)
    errAbort("can't specify both -cdsUpper and -keepMasking");
if (exonIndices && (cdsOnly || peptides))
    errAbort("can't specify -exonIndices with -cdsOnly or -peptides");
if (maxSize != -1 && peptides)
    errAbort("can't specify -maxSize with -peptides");
genomeSeqs = optionVal("genomeSeqs", NULL);
getRnaPred(argv[1], argv[2], argv[3], argv[4]);
return 0;
}
