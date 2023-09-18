/* bedToBigBed - Convert bed to bigBed.. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "dystring.h"
#include "obscure.h"
#include "asParse.h"
#include "basicBed.h"
#include "memalloc.h"
#include "sig.h"
#include "rangeTree.h"
#include "zlibFace.h"
#include "sqlNum.h"
#include "bPlusTree.h"
#include "bigBed.h"
#include "bbiAlias.h"
#include "twoBit.h"
#include "portable.h"

char *version = "2.9";   // when changing, change in bedToBigBed, bedGraphToBigWig, and wigToBigWig
/* Version history from 2.6 on at least -
 *   2.9 - ability to specify chromAlias bigBed as chromSizes file
 *   2.8 - Various changes where developer didn't increment version id
 *   2.7 - Added check for duplicate field names in asParse.c
 *   2.6 - Made it not crash on empty input.  
 *   */

/* Things set directly or indirectly by command lne in main() routine. */
int blockSize = 256;
int itemsPerSlot = 512;
char *extraIndex = NULL;
int bedN = 0;   /* number of standard bed fields */
int bedP = 0;   /* number of bed plus fields */
char *asFile = NULL;
char *asText = NULL;
char *udcDir = NULL;
static boolean doCompress = FALSE;
static boolean tabSep = FALSE;
static boolean sizesIs2Bit = FALSE;
static boolean sizesIsChromAliasBb = FALSE;
static boolean allow1bpOverlap = FALSE;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "bedToBigBed v. %s - Convert bed file to bigBed. (bbi version: %d)\n"
  "usage:\n"
  "   bedToBigBed in.bed chrom.sizes out.bb\n"
  "Where in.bed is in one of the ascii bed formats, but not including track lines\n"
  "and chrom.sizes is a two-column file/URL: <chromosome name> <size in bases>\n"
  "and out.bb is the output indexed big bed file.\n"
  "\n"
  "If the assembly <db> is hosted by UCSC, chrom.sizes can be a URL like\n"
  "  http://hgdownload.soe.ucsc.edu/goldenPath/<db>/bigZips/<db>.chrom.sizes\n"
  "or you may use the script fetchChromSizes to download the chrom.sizes file.\n"
  "If you have bed annotations on patch sequences from NCBI, a more inclusive\n"
  "chrom.sizes file can be found using a URL like\n"
  "  http://hgdownload.soe.ucsc.edu/goldenPath/<db>/database/chromInfo.txt.gz\n"
  "If not hosted by UCSC, a chrom.sizes file can be generated by running\n"
  "twoBitInfo on the assembly .2bit file or the 2bit file or used directly\n"
  "if the -sizesIs2Bit option is specified.\n"
  "\n"
  "The chrom.sizes file may also be a chromAlias bigBed file, or a URL to\n"
  "such a file, by specifying the -sizesIsChromAliasBb option.  When using\n"
  "a chromAlias bigBed file, the input BED file may have chromosome names\n"
  "matching any of the sequence name aliases in the chromAlias file.\n"
  "\n"
  "For UCSC provided genomes, the chromAlias files can be found under:\n"
  "    https://hgdownload.soe.ucsc.edu/goldenPath/<db>/bigZips/<db>.chromAlias.bb\n"
  "For UCSC GenArk assembly hubs, the chrom aliases are namedd in the form:\n"
  "    https://hgdownload.soe.ucsc.edu/hubs/GCF/006/542/625/GCF_006542625.1/GCF_006542625.1.chromAlias.bb\n"
  "For a description of generating chromAlias files for your own assembly hub, see:\n"
  "      http://genomewiki.ucsc.edu/index.php/Chrom_Alias\n"
  "\n"
  "The in.bed file must be sorted by chromosome,start,\n"
  "  to sort a bed file, use the unix sort command:\n"
  "     LC_ALL=C sort -k1,1 -k2,2n unsorted.bed > sorted.bed\n"
  "Sorting must be set to skip Unicode mapping to make it case-sensitive (LC_ALL=C).\n"
  "\n"
  "options:\n"
  "   -type=bedN[+[P]] : \n"
  "                      N is between 3 and 15, \n"
  "                      optional (+) if extra \"bedPlus\" fields, \n"
  "                      optional P specifies the number of extra fields. Not required, but preferred.\n"
  "                      Examples: -type=bed6 or -type=bed6+ or -type=bed6+3 \n"
  "                      (see http://genome.ucsc.edu/FAQ/FAQformat.html#format1)\n"
  "   -as=fields.as - If you have non-standard \"bedPlus\" fields, it's great to put a definition\n"
  "                   of each field in a row in AutoSql format here.\n"
  "   -blockSize=N - Number of items to bundle in r-tree.  Default %d\n"
  "   -itemsPerSlot=N - Number of data points bundled at lowest level. Default %d\n"
  "   -unc - If set, do not use compression.\n"
  "   -tab - If set, expect fields to be tab separated, normally\n"
  "           expects white space separator.\n"
  "   -extraIndex=fieldList - If set, make an index on each field in a comma separated list\n"
  "           extraIndex=name and extraIndex=name,id are commonly used.\n"
  "   -sizesIs2Bit  -- If set, the chrom.sizes file is assumed to be a 2bit file.\n"
  "   -sizesIsChromAliasBb -- If set, then chrom.sizes file is assumed to be a chromAlias\n"
  "    bigBed file or a URL to a such a file (see above).\n"
  "   -sizesIsBb  -- Obsolete name for -sizesIsChromAliasBb.\n"
  "   -udcDir=/path/to/udcCacheDir  -- sets the UDC cache dir for caching of remote files.\n"
  "   -allow1bpOverlap  -- allow exons to overlap by at most one base pair\n"
  "   -maxAlloc=N -- Set the maximum memory allocation size to N bytes\n"
  , version, bbiCurrentVersion, blockSize, itemsPerSlot
  );
}

static struct optionSpec options[] = {
   {"blockSize", OPTION_INT},
   {"itemsPerSlot", OPTION_INT},
   {"type", OPTION_STRING},
   {"as", OPTION_STRING},
   {"unc", OPTION_BOOLEAN},
   {"tab", OPTION_BOOLEAN},
   {"sizesIs2Bit", OPTION_BOOLEAN},
   {"sizesIsChromAliasBb", OPTION_BOOLEAN},
   {"sizesIsBb", OPTION_BOOLEAN},
   {"extraIndex", OPTION_STRING},
   {"udcDir", OPTION_STRING},
   {"allow1bpOverlap", OPTION_BOOLEAN},
   {"maxAlloc", OPTION_LONG_LONG},
   {NULL, 0},
};

int bbNamedFileChunkCmpByName(const void *va, const void *vb)
/* Compare two named offset object to facilitate qsorting by name. */
{
const struct bbNamedFileChunk *a = va, *b = vb;
return strcmp(a->name, b->name);
}

static int maxBedNameSize;

void bbNamedFileChunkKey(const void *va, char *keyBuf)
/* Copy name to keyBuf for bPlusTree maker */
{
const struct bbNamedFileChunk *item = va;
strncpy(keyBuf,item->name, maxBedNameSize);
}

void *bbNamedFileChunkVal(const void *va)
/* Return pointer to val for bPlusTree maker. */
{
const struct bbNamedFileChunk *item = va;
return (void *)&item->offset;
}

void bbExIndexMakerAddKeysFromRow(struct bbExIndexMaker *eim, char **row, int recordIx)
/* Save the keys that are being indexed by row in eim. */
{
int i;
for (i=0; i < eim->indexCount; ++i)
    {
    int rowIx = eim->indexFields[i];
    eim->chunkArrayArray[i][recordIx].name = cloneString(row[rowIx]);
    }
}

void bbExIndexMakerAddOffsetSize(struct bbExIndexMaker *eim, bits64 offset, bits64 size,
    long startIx, long endIx)
/* Update offset and size fields of all file chunks between startIx and endIx */
{
int i;
for (i=0; i < eim->indexCount; ++i)
    {
    struct bbNamedFileChunk *chunks = eim->chunkArrayArray[i];
    long j;
    for (j = startIx; j < endIx; ++j)
        {
	struct bbNamedFileChunk *chunk = chunks + j;
	chunk->offset = offset;
	chunk->size = size;
	}
    }
}

static void writeBlocks(struct bbiChromUsage *usageList, struct lineFile *lf, struct asObject *as, 
	int itemsPerSlot, struct bbiBoundsArray *bounds, 
	int sectionCount, boolean doCompress, FILE *f, 
	int resTryCount, int resScales[], int resSizes[], 
	struct bbExIndexMaker *eim,  int bedCount,
	bits16 fieldCount, bits32 *retMaxBlockSize)
/* Read through lf, writing it in f.  Save starting points of blocks (every itemsPerSlot)
 * to boundsArray */
{
int maxBlockSize = 0;
struct bbiChromUsage *usage = usageList;
char *line, *row[fieldCount+1];
int lastField = fieldCount-1;
int itemIx = 0, sectionIx = 0;
bits64 blockStartOffset = 0;
int startPos = 0, endPos = 0;
bits32 chromId = 0;
struct dyString *stream = dyStringNew(0);

/* Will keep track of some things that help us determine how much to reduce. */
bits32 resEnds[resTryCount];
int resTry;
for (resTry = 0; resTry < resTryCount; ++resTry)
    resEnds[resTry] = 0;
boolean atEnd = FALSE, sameChrom = FALSE;
bits32 start = 0, end = 0;
char *chrom = NULL;
struct bed *bed;
AllocVar(bed);

/* Help keep track of which beds are in current chunk so as to write out
 * namedChunks to eim if need be. */
long sectionStartIx = 0, sectionEndIx = 0;

for (;;)
    {
    /* Get next line of input if any. */
    if (lineFileNextReal(lf, &line))
	{
	/* Chop up line and make sure the word count is right. */
	int wordCount;
	if (tabSep)
	    wordCount = chopTabs(line, row);
	else
	    wordCount = chopLine(line, row);
	lineFileExpectWordsMesg(lf, fieldCount, wordCount, "If the input is a tab-sep file, do not forget to use the -tab option");

	loadAndValidateBedExt(row, bedN, fieldCount, lf, bed, as, FALSE, allow1bpOverlap);

	chrom = bed->chrom;
	start = bed->chromStart;
	end = bed->chromEnd;

	sameChrom = sameString(chrom, usage->name);
	}
    else  /* No next line */
	{
	atEnd = TRUE;
	}


    /* Check conditions that would end block and save block info and advance to next if need be. */
    if (atEnd || !sameChrom || itemIx >= itemsPerSlot)
        {
	/* Save stream to file, compressing if need be. */
	if (stream->stringSize > maxBlockSize)
	    maxBlockSize = stream->stringSize;
	if (doCompress)
            {
	    size_t maxCompSize = zCompBufSize(stream->stringSize);

            // keep around an area of scratch memory
            static int compBufSize = 0;
            static char *compBuf = NULL;
            // check to see if buffer needed for compression is big enough
            if (compBufSize < maxCompSize)
                {
                // free up the old not-big-enough piece
                freez(&compBuf); // freez knows bout NULL

                // get new scratch area
                compBufSize = maxCompSize;
                compBuf = needLargeMem(compBufSize);
                }

	    int compSize = zCompress(stream->string, stream->stringSize, compBuf, maxCompSize);
	    mustWrite(f, compBuf, compSize);
	    }
	else
	    mustWrite(f, stream->string, stream->stringSize);
	dyStringClear(stream);

	/* Save block offset and size for all named chunks in this section. */
	if (eim != NULL)
	    {
	    bits64 blockEndOffset = ftell(f);
	    bbExIndexMakerAddOffsetSize(eim, blockStartOffset, blockEndOffset-blockStartOffset,
		sectionStartIx, sectionEndIx);
	    sectionStartIx = sectionEndIx;
	    }

	/* Save info on existing block. */
	struct bbiBoundsArray *b = &bounds[sectionIx];
	b->offset = blockStartOffset;
	b->range.chromIx = chromId;
	b->range.start = startPos;
	b->range.end = endPos;
	++sectionIx;
	itemIx = 0;

	if (atEnd)
	    break;
	}

    /* Advance to next chromosome if need be and get chromosome id. */
    if (!sameChrom)
        {
	usage = usage->next;
	assert(usage != NULL);
	assert(sameString(chrom, usage->name));
	for (resTry = 0; resTry < resTryCount; ++resTry)
	    resEnds[resTry] = 0;
	}
    chromId = usage->id;

    /* At start of block we save a lot of info. */
    if (itemIx == 0)
        {
	blockStartOffset = ftell(f);
	startPos = start;
	endPos = end;
	}
    /* Otherwise just update end. */
        {
	if (endPos < end)
	    endPos = end;
	/* No need to update startPos since list is sorted. */
	}

    /* Save name into namedOffset if need be. */
    if (eim != NULL)
	{
	bbExIndexMakerAddKeysFromRow(eim, row, sectionEndIx);
	sectionEndIx += 1;
	}

    /* Write out data. */
    dyStringWriteOne(stream, chromId);
    dyStringWriteOne(stream, start);
    dyStringWriteOne(stream, end);
    if (fieldCount > 3)
        {
	int i;
	/* Write 3rd through next to last field and a tab separator. */
	for (i=3; i<lastField; ++i)
	    {
	    char *s = row[i];
	    dyStringAppend(stream, s);
	    dyStringAppendC(stream, '\t');
	    }
	/* Write last field and terminal zero */
	char *s = row[lastField];
	dyStringAppend(stream, s);
	}
    dyStringAppendC(stream, 0);

    itemIx += 1;

    /* Do zoom counting. */
    for (resTry = 0; resTry < resTryCount; ++resTry)
        {
	bits32 resEnd = resEnds[resTry];
	if (start >= resEnd && resEnd < usage->size)
	    {
	    resSizes[resTry] += 1;
	    resEnds[resTry] = resEnd = start + resScales[resTry];
	    }
	while (end > resEnd)
	    {
	    resSizes[resTry] += 1;
	    resEnds[resTry] = resEnd = resEnd + resScales[resTry];
	    }
	}
    }
assert(sectionIx == sectionCount);
freez(&bed);
*retMaxBlockSize = maxBlockSize;
}

struct rbTree *rangeTreeForBedChrom(struct lineFile *lf, char *chrom)
/* Read lines from bed file as long as they match chrom.  Return a rangeTree that
 * corresponds to the coverage. */
{
struct rbTree *tree = rangeTreeNew();
char *line;
while (lineFileNextReal(lf, &line))
    {
    if (!startsWithWord(chrom, line))
        {
	lineFileReuse(lf);
	break;
	}
    char *row[3];
    chopLine(line, row);
    unsigned start = sqlUnsigned(row[1]);
    unsigned end = sqlUnsigned(row[2]);
    rangeTreeAddToCoverageDepth(tree, start, end);
    }
return tree;
}

static struct bbiSummary *bedWriteReducedOnceReturnReducedTwice(struct bbiChromUsage *usageList, 
	int fieldCount, struct lineFile *lf, bits32 initialReduction, bits32 initialReductionCount, 
	int zoomIncrement, int blockSize, int itemsPerSlot, boolean doCompress,
	struct lm *lm, FILE *f, bits64 *retDataStart, bits64 *retIndexStart,
	struct bbiSummaryElement *totalSum)
/* Write out data reduced by factor of initialReduction.  Also calculate and keep in memory
 * next reduction level.  This is more work than some ways, but it keeps us from having to
 * keep the first reduction entirely in memory. */
{
struct bbiSummary *twiceReducedList = NULL;
bits32 doubleReductionSize = initialReduction * zoomIncrement;
struct bbiChromUsage *usage = usageList;
struct bbiBoundsArray *boundsArray, *boundsPt, *boundsEnd;
boundsPt = AllocArray(boundsArray, initialReductionCount);
boundsEnd = boundsPt + initialReductionCount;

*retDataStart = ftell(f);
writeOne(f, initialReductionCount);

/* This gets a little complicated I'm afraid.  The strategy is to:
 *   1) Build up a range tree that represents coverage depth on that chromosome
 *      This also has the nice side effect of getting rid of overlaps.
 *   2) Stream through the range tree, outputting the initial summary level and
 *      further reducing. 
 */
boolean firstTime = TRUE;
struct bbiSumOutStream *stream = bbiSumOutStreamOpen(itemsPerSlot, f, doCompress);
for (usage = usageList; usage != NULL; usage = usage->next)
    {
    struct bbiSummary oneSummary, *sum = NULL;
    struct rbTree *rangeTree = rangeTreeForBedChrom(lf, usage->name);
    struct range *range, *rangeList = rangeTreeList(rangeTree);
    for (range = rangeList; range != NULL; range = range->next)
        {
	/* Grab values we want from range. */
	double val = ptToInt(range->val);
	int start = range->start;
	int end = range->end;
	bits32 size = end - start;

        // we want to make sure we count zero size elements
        if (size == 0)
            size = 1;

	/* Add to total summary. */
	if (firstTime)
	    {
	    totalSum->validCount = size;
	    totalSum->minVal = totalSum->maxVal = val;
	    totalSum->sumData = val*size;
	    totalSum->sumSquares = val*val*size;
	    firstTime = FALSE;
	    }
	else
	    {
	    totalSum->validCount += size;
	    if (val < totalSum->minVal) totalSum->minVal = val;
	    if (val > totalSum->maxVal) totalSum->maxVal = val;
	    totalSum->sumData += val*size;
	    totalSum->sumSquares += val*val*size;
	    }

	/* If start past existing block then output it. */
	if (sum != NULL && sum->end <= start && sum->end < usage->size)
	    {
	    bbiOutputOneSummaryFurtherReduce(sum, &twiceReducedList, doubleReductionSize, 
		&boundsPt, boundsEnd, lm, stream);
	    sum = NULL;
	    }
	/* If don't have a summary we're working on now, make one. */
	if (sum == NULL)
	    {
	    oneSummary.chromId = usage->id;
	    oneSummary.start = start;
	    oneSummary.end = start + initialReduction;
	    if (oneSummary.end > usage->size) oneSummary.end = usage->size;
	    oneSummary.minVal = oneSummary.maxVal = val;
	    oneSummary.sumData = oneSummary.sumSquares = 0.0;
	    oneSummary.validCount = 0;
	    sum = &oneSummary;
	    }
	/* Deal with case where might have to split an item between multiple summaries.  This
	 * loop handles all but the final affected summary in that case. */
	while (end > sum->end)
	    {
	    /* Fold in bits that overlap with existing summary and output. */
	    int overlap = rangeIntersection(start, end, sum->start, sum->end);
	    assert(overlap > 0);
	    verbose(3, "Splitting size %d at %d, overlap %d\n", end - start, sum->end, overlap);
	    sum->validCount += overlap;
	    if (sum->minVal > val) sum->minVal = val;
	    if (sum->maxVal < val) sum->maxVal = val;
	    sum->sumData += val * overlap;
	    sum->sumSquares += val*val * overlap;
	    bbiOutputOneSummaryFurtherReduce(sum, &twiceReducedList, doubleReductionSize, 
		    &boundsPt, boundsEnd, lm, stream);
	    size -= overlap;

	    /* Move summary to next part. */
	    sum->start = start = sum->end;
	    sum->end = start + initialReduction;
	    if (sum->end > usage->size) sum->end = usage->size;
	    sum->minVal = sum->maxVal = val;
	    sum->sumData = sum->sumSquares = 0.0;
	    sum->validCount = 0;
	    }

	/* Add to summary. */
	sum->validCount += size;
	if (sum->minVal > val) sum->minVal = val;
	if (sum->maxVal < val) sum->maxVal = val;
	sum->sumData += val * size;
	sum->sumSquares += val*val * size;
	}
    if (sum != NULL)
	{
	bbiOutputOneSummaryFurtherReduce(sum, &twiceReducedList, doubleReductionSize, 
	    &boundsPt, boundsEnd, lm, stream);
	}
    rangeTreeFree(&rangeTree);
    }
bbiSumOutStreamClose(&stream);

/* Write out 1st zoom index. */
int indexOffset = *retIndexStart = ftell(f);
assert(boundsPt == boundsEnd);
cirTreeFileBulkIndexToOpenFile(boundsArray, sizeof(boundsArray[0]), initialReductionCount,
    blockSize, itemsPerSlot, NULL, bbiBoundsArrayFetchKey, bbiBoundsArrayFetchOffset, 
    indexOffset, f);

freez(&boundsArray);
slReverse(&twiceReducedList);
return twiceReducedList;
}

struct bbExIndexMaker *bbExIndexMakerNew(struct slName *extraIndexList, struct asObject *as)
/* Return an index maker corresponding to extraIndexList. Checks that all fields
 * mentioned are in autoSql definition, and for now that they are all text fields. */
{
/* Fill in scalar fields and return quickly if no extra indexes. */
struct bbExIndexMaker *eim;
AllocVar(eim);
eim->indexCount = slCount(extraIndexList);
if (eim->indexCount == 0)
     return eim;	// Not much to do in this case

/* Allocate arrays according field count. */
AllocArray(eim->indexFields, eim->indexCount);
AllocArray(eim->maxFieldSize, eim->indexCount);
AllocArray(eim->chunkArrayArray, eim->indexCount);
AllocArray(eim->fileOffsets, eim->indexCount);

/* Loop through each field checking that it is indeed something we can index
 * and if so saving information about it */
int indexIx = 0;
struct slName *name;
for (name = extraIndexList; name != NULL; name = name->next)
    {
    struct asColumn *col = asColumnFind(as, name->name);
    if (col == NULL)
        errAbort("extraIndex field %s not a standard bed field or found in 'as' file.",
	    name->name);
    if (!sameString(col->lowType->name, "string"))
        errAbort("Sorry for now can only index string fields.");
    eim->indexFields[indexIx] = slIxFromElement(as->columnList, col);
    ++indexIx;
    }
return eim;
}

void bbExIndexMakerAllocChunkArrays(struct bbExIndexMaker *eim, int recordCount)
/* Allocate the big part of the extra index maker - the part that holds which
 * chunk is used for each record. */
{
eim->recordCount = recordCount;
int i;
for (i=0; i < eim->indexCount; ++i)
    AllocArray(eim->chunkArrayArray[i], recordCount);
}

void bbExIndexMakerFree(struct bbExIndexMaker **pEim)
/* Free up memory associated with bbExIndexMaker */
{
struct bbExIndexMaker *eim = *pEim;
if (eim != NULL)
    {
    if (eim->chunkArrayArray != NULL)
	{
	int i;
	for (i=0; i < eim->indexCount; ++i)
	    freeMem(eim->chunkArrayArray[i]);
	}
    freeMem(eim->indexFields);
    freeMem(eim->maxFieldSize);
    freeMem(eim->chunkArrayArray);
    freeMem(eim->fileOffsets);
    freez(pEim);
    }
}

void bbFileCreate(
	char *inName, 	  /* Input file in a tabular bed format <chrom><start><end> + whatever. */
	char *chromSizes, /* Two column tab-separated file: <chromosome> <size>. */
	int blockSize,	  /* Number of items to bundle in r-tree.  1024 is good. */
	int itemsPerSlot, /* Number of items in lowest level of tree.  64 is good. */
	char *asText,	  /* Field definitions in a string */
	struct asObject *as,  /* Field definitions parsed out */
	boolean doCompress, /* If TRUE then compress data. */
	struct slName *extraIndexList,	/* List of extra indexes to add */
	char *outName)    /* BigBed output file name. */
/* Convert tab-separated bed file to binary indexed, zoomed bigBed version. */
{
/* Set up timing measures. */
verboseTimeInit();
struct lineFile *lf = lineFileOpen(inName, TRUE);

bits16 fieldCount = slCount(as->columnList);
bits16 extraIndexCount = slCount(extraIndexList);

struct bbExIndexMaker *eim = NULL;
if (extraIndexList != NULL)
    eim = bbExIndexMakerNew(extraIndexList, as);

/* Do first pass, mostly just scanning file and counting hits per chromosome. */
int minDiff = 0;
double aveSize = 0;
bits64 bedCount = 0;
bits32 uncompressBufSize = 0;
struct bbiChromUsage *usageList = NULL;
if (sizesIsChromAliasBb)
    usageList = bbiChromUsageFromBedFileAlias(lf, chromSizes, eim, &minDiff, &aveSize, &bedCount, tabSep);
else
    {
    struct hash *chromSizesHash = NULL;
    if (sizesIs2Bit)
        chromSizesHash = twoBitChromHash(chromSizes);
    else
        chromSizesHash = bbiChromSizesFromFile(chromSizes);
    verbose(2, "Read %d chromosomes and sizes from %s\n",  chromSizesHash->elCount, chromSizes);
    usageList = bbiChromUsageFromBedFile(lf, chromSizesHash, eim, &minDiff, &aveSize, &bedCount, tabSep);
    freeHash(&chromSizesHash);
    }
verboseTime(1, "pass1 - making usageList (%d chroms)", slCount(usageList));
verbose(2, "%d chroms in %s. Average span of beds %f\n", slCount(usageList), inName, aveSize);

/* Open output file and write dummy header. */
FILE *f = mustOpen(outName, "wb");
bbiWriteDummyHeader(f);
bbiWriteDummyZooms(f);

/* Write out autoSql string */
bits64 asOffset = ftell(f);
mustWrite(f, asText, strlen(asText) + 1);
verbose(2, "as definition has %d columns\n", fieldCount);

/* Write out dummy total summary. */
struct bbiSummaryElement totalSum;
ZeroVar(&totalSum);
bits64 totalSummaryOffset = ftell(f);
bbiSummaryElementWrite(f, &totalSum);

/* Write out dummy header extension */
bits64 extHeaderOffset = ftell(f);
bits16 extHeaderSize = 64;
repeatCharOut(f, 0, extHeaderSize);

/* Write out extra index stuff if need be. */
bits64 extraIndexListOffset = 0;
bits64 extraIndexListEndOffset = 0;
if (extraIndexList != NULL)
    {
    extraIndexListOffset = ftell(f);
    int extraIndexSize = 16 + 4*1;   // Fixed record size 16, plus 1 times field size of 4 
    repeatCharOut(f, 0, extraIndexSize*extraIndexCount);
    extraIndexListEndOffset = ftell(f);
    }

/* Write out chromosome/size database. */
bits64 chromTreeOffset = ftell(f);
bbiWriteChromInfo(usageList, blockSize, f);

/* Set up to keep track of possible initial reduction levels. */
int resScales[bbiMaxZoomLevels], resSizes[bbiMaxZoomLevels];
int resTryCount = bbiCalcResScalesAndSizes(aveSize, resScales, resSizes);

/* Write out primary full resolution data in sections, collect stats to use for reductions. */
bits64 dataOffset = ftell(f);
bits32 blockCount = 0;
bits32 maxBlockSize = 0;
struct bbiBoundsArray *boundsArray = NULL;
writeOne(f, bedCount);
if (bedCount > 0)
    {
    blockCount = bbiCountSectionsNeeded(usageList, itemsPerSlot);
    AllocArray(boundsArray, blockCount);
    lineFileRewind(lf);
    if (eim)
	bbExIndexMakerAllocChunkArrays(eim, bedCount);
    writeBlocks(usageList, lf, as, itemsPerSlot, boundsArray, blockCount, doCompress,
	    f, resTryCount, resScales, resSizes, eim, bedCount, fieldCount, &maxBlockSize);
    }
verboseTime(1, "pass2 - checking and writing primary data (%lld records, %d fields)", 
	(long long)bedCount, fieldCount);

/* Write out primary data index. */
bits64 indexOffset = ftell(f);
cirTreeFileBulkIndexToOpenFile(boundsArray, sizeof(boundsArray[0]), blockCount,
    blockSize, 1, NULL, bbiBoundsArrayFetchKey, bbiBoundsArrayFetchOffset, 
    indexOffset, f);
freez(&boundsArray);
verboseTime(2, "index write");

/* Declare arrays and vars that track the zoom levels we actually output. */
bits32 zoomAmounts[bbiMaxZoomLevels];
bits64 zoomDataOffsets[bbiMaxZoomLevels];
bits64 zoomIndexOffsets[bbiMaxZoomLevels];

/* Call monster zoom maker library function that bedGraphToBigWig also uses. */
int zoomLevels = 0;
if (bedCount > 0)
    {
    zoomLevels = bbiWriteZoomLevels(lf, f, blockSize, itemsPerSlot,
	bedWriteReducedOnceReturnReducedTwice, fieldCount,
	doCompress, indexOffset - dataOffset, 
	usageList, resTryCount, resScales, resSizes, 
	zoomAmounts, zoomDataOffsets, zoomIndexOffsets, &totalSum);
    }

/* Write out extra indexes if need be. */
if (eim)
    {
    int i;
    for (i=0; i < eim->indexCount; ++i)
        {
	eim->fileOffsets[i] = ftell(f);
	maxBedNameSize = eim->maxFieldSize[i];
	qsort(eim->chunkArrayArray[i], bedCount, 
	    sizeof(struct bbNamedFileChunk), bbNamedFileChunkCmpByName);
	assert(sizeof(struct bbNamedFileChunk) == sizeof(eim->chunkArrayArray[i][0]));
	bptFileBulkIndexToOpenFile(eim->chunkArrayArray[i], sizeof(eim->chunkArrayArray[i][0]), 
	    bedCount, blockSize, bbNamedFileChunkKey, maxBedNameSize, bbNamedFileChunkVal, 
	    sizeof(bits64) + sizeof(bits64), f);
	verboseTime(1, "Sorting and writing extra index %d", i);
	}
    }

/* Figure out buffer size needed for uncompression if need be. */
if (doCompress)
    {
    int maxZoomUncompSize = itemsPerSlot * sizeof(struct bbiSummaryOnDisk);
    uncompressBufSize = max(maxBlockSize, maxZoomUncompSize);
    }

/* Go back and rewrite header. */
rewind(f);
bits32 sig = bigBedSig;
bits16 version = bbiCurrentVersion;
bits16 summaryCount = zoomLevels;
bits32 reserved32 = 0;
bits64 reserved64 = 0;

bits16 definedFieldCount = bedN;

/* Write fixed header */
writeOne(f, sig);
writeOne(f, version);
writeOne(f, summaryCount);
writeOne(f, chromTreeOffset);
writeOne(f, dataOffset);
writeOne(f, indexOffset);
writeOne(f, fieldCount);
writeOne(f, definedFieldCount);
writeOne(f, asOffset);
writeOne(f, totalSummaryOffset);
writeOne(f, uncompressBufSize);
writeOne(f, extHeaderOffset);
assert(ftell(f) == 64);

/* Write summary headers with data. */
int i;
verbose(2, "Writing %d levels of zoom\n", zoomLevels);
for (i=0; i<zoomLevels; ++i)
    {
    verbose(3, "zoomAmounts[%d] = %d\n", i, (int)zoomAmounts[i]);
    writeOne(f, zoomAmounts[i]);
    writeOne(f, reserved32);
    writeOne(f, zoomDataOffsets[i]);
    writeOne(f, zoomIndexOffsets[i]);
    }
/* Write rest of summary headers with no data. */
for (i=zoomLevels; i<bbiMaxZoomLevels; ++i)
    {
    writeOne(f, reserved32);
    writeOne(f, reserved32);
    writeOne(f, reserved64);
    writeOne(f, reserved64);
    }

/* Write total summary. */
fseek(f, totalSummaryOffset, SEEK_SET);
bbiSummaryElementWrite(f, &totalSum);

/* Write extended header */
fseek(f, extHeaderOffset, SEEK_SET);
writeOne(f, extHeaderSize);
writeOne(f, extraIndexCount);
writeOne(f, extraIndexListOffset);
repeatCharOut(f, 0, 52);    // reserved
assert(ftell(f) - extHeaderOffset == extHeaderSize);

/* Write extra index offsets if need be. */
if (extraIndexCount != 0)
    {
    fseek(f, extraIndexListOffset, SEEK_SET);
    int i;
    for (i=0; i<extraIndexCount; ++i)
        {
	// Write out fixed part of index info
	bits16 type = 0;    // bPlusTree type
	bits16 indexFieldCount = 1;
	writeOne(f, type);
	writeOne(f, indexFieldCount);
	writeOne(f, eim->fileOffsets[i]);
	repeatCharOut(f, 0, 4);  // reserved

	// Write out field list - easy this time because for now always only one field.
	bits16 fieldId = eim->indexFields[i];
	writeOne(f, fieldId);
	repeatCharOut(f, 0, 2); // reserved
	}
    assert(ftell(f) == extraIndexListEndOffset);
    }

/* Write end signature. */
fseek(f, 0L, SEEK_END);
writeOne(f, sig);


/* Clean up. */
lineFileClose(&lf);
carefulClose(&f);
bbiChromUsageFreeList(&usageList);
asObjectFreeList(&as);
}

void bedToBigBed(char *inName, char *chromSizes, char *outName)
/* bedToBigBed - Convert bed file to bigBed.. */
{
struct slName *extraIndexList = slNameListFromString(extraIndex, ',');
struct asObject *as = asParseText(asText);
if (as == NULL)
    errAbort("AutoSql file (%s) not in legal format.", asFile);
asCompareObjAgainstStandardBed(as, bedN, TRUE); // abort if bedN columns are not standard
bbFileCreate(inName, chromSizes, blockSize, itemsPerSlot, asText, as, 
	doCompress, extraIndexList, outName);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
blockSize = optionInt("blockSize", blockSize);
itemsPerSlot = optionInt("itemsPerSlot", itemsPerSlot);
asFile = optionVal("as", asFile);
doCompress = !optionExists("unc");
sizesIs2Bit = optionExists("sizesIs2Bit");
sizesIsChromAliasBb = optionExists("sizesIsChromAliasBb") || optionExists("sizesIsBb");
if (sizesIs2Bit && sizesIsChromAliasBb)
    errAbort("can't specify both -sizesIs2Bit and -sizesIsChromAliasBb");
extraIndex = optionVal("extraIndex", NULL);
tabSep = optionExists("tab");
allow1bpOverlap = optionExists("allow1bpOverlap");
udcDir = optionVal("udcDir", udcDefaultDir());
size_t maxAlloc = optionLongLong("maxAlloc", 0);
if (argc != 4)
    usage();

char *bedFileName = argv[1];

mustBeReadableAndRegularFile(bedFileName);

udcSetDefaultDir(udcDir);
if (maxAlloc > 0)
    setMaxAlloc(maxAlloc);

if (optionExists("type"))
    {
    // parse type
    char *btype = cloneString(optionVal("type", ""));
    char *plus = strchr(btype, '+');
    if (plus)
	{
	*plus++ = 0;
	if (isdigit(*plus))
	    bedP = sqlUnsigned(plus);
	}
    if (!startsWith("bed", btype))
	errAbort("type must begin with \"bed\"");
    btype +=3;
    bedN = sqlUnsigned(btype);
    if (bedN < 3)
	errAbort("Bed must be 3 or higher, found %d\n", bedN);
    if (bedN > 15)
	errAbort("Bed must be 15 or lower, found %d\n", bedN);
    }
else
    {
    if (asText)
	errAbort("If you specify the .as file, you must specify the -type as well so that\n"
	         "the number of standard BED columns is known.");
    }

/* If the haven't set bedN and bedP from the type var in the command line, then we sniff it
 * out from file. */
if (bedN == 0)
    {
    /* Just read in single line and count fields. */
    struct lineFile *lf = lineFileOpen(bedFileName, TRUE);
    char *line;
    if (!lineFileNextReal(lf, &line))
        errAbort("%s is empty", lf->fileName);
    int fieldCount;
    if (tabSep)
	fieldCount = chopByChar(line, '\t', NULL, 0); // Do not use chopString, see GOTCHA
    else
	fieldCount = chopByWhite(line, NULL, 0);
    if (fieldCount > 256)
        errAbort("Too many columns in %s, you sure it's a bed file?", lf->fileName);
    lineFileClose(&lf);

    /* Set up so that it looks like we are straight up bed for that many fields,
     * or if more than or maximum defined fields, then for bed15+ */
    bedN = fieldCount;
    if (bedN > bedKnownFields)
        {
	bedP = bedN - bedKnownFields;
	bedN = bedKnownFields;
	}
    }
   
/* Make sure that fields are defined, from bed spec if nowhere else. */
if (asFile)
    readInGulp(asFile, &asText, NULL);
else
    asText = bedAsDef(bedN,  bedN + bedP);

bedToBigBed(bedFileName, argv[2], argv[3]);
optionFree();
if (verboseLevel() > 1)
    printVmPeak();
return 0;
}
