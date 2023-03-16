/* bigBedInfo - Show information about a bigBed file.. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "udc.h"
#include "bPlusTree.h"
#include "bbiFile.h"
#include "bigBed.h"
#include "obscure.h"
#include "hmmstats.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "bigBedInfo - Show information about a bigBed file.\n"
  "usage:\n"
  "   bigBedInfo file.bb\n"
  "options:\n"
  "   -udcDir=/dir/to/cache - place to put cache for remote bigBed/bigWigs\n"
  "   -chroms - list all chromosomes and their sizes\n"
  "   -zooms - list all zoom levels and their sizes\n"
  "   -as - get autoSql spec\n"
  "   -extraIndex - list all the extra indexes\n"
  );
}

static struct optionSpec options[] = {
   {"udcDir", OPTION_STRING},
   {"chroms", OPTION_BOOLEAN},
   {"zooms", OPTION_BOOLEAN},
   {"as", OPTION_BOOLEAN},
   {"extraIndex", OPTION_BOOLEAN},
   {NULL, 0},
};

void printLabelAndLongNumber(char *label, long long l)
/* Print label: 1,234,567 format number */
{
printf("%s: ", label);
printLongWithCommas(stdout, l);
printf("\n");
}

void bigBedInfo(char *fileName)
/* bigBedInfo - Show information about a bigBed file.. */
{
struct bbiFile *bbi = bigBedFileOpen(fileName);
printf("version: %d\n", bbi->version);
printf("fieldCount: %d\n", bbi->fieldCount);
printf("hasHeaderExtension: %s\n", (bbi->extensionOffset != 0 ? "yes" : "no"));
printf("isCompressed: %s\n", (bbi->uncompressBufSize > 0 ? "yes" : "no"));
printf("isSwapped: %d\n", bbi->isSwapped);
printf("extraIndexCount: %d\n", bbi->extraIndexCount);
if (optionExists("extraIndex"))
    {
    struct slName *el, *list = bigBedListExtraIndexes(bbi);
    for (el = list; el != NULL; el = el->next)
	{
	int fieldIx = 0;
	struct bptFile *bpt = bigBedOpenExtraIndex(bbi, el->name, &fieldIx);
        printf("    %s (field %d) with %lld items\n", el->name, fieldIx, (long long)bpt->itemCount);
	}
    }
printLabelAndLongNumber("itemCount", bigBedItemCount(bbi));
printLabelAndLongNumber("primaryDataSize", bbi->unzoomedIndexOffset - bbi->unzoomedDataOffset);
if (bbi->levelList != NULL)
    {
    long long indexEnd = bbi->levelList->dataOffset;
    printLabelAndLongNumber("primaryIndexSize", indexEnd - bbi->unzoomedIndexOffset);
    }
struct bbiChromInfo *chrom, *chromList = bbiChromList(bbi);
printf("zoomLevels: %d\n", bbi->zoomLevels);
if (optionExists("zooms"))
    {
    struct bbiZoomLevel *zoom;
    for (zoom = bbi->levelList; zoom != NULL; zoom = zoom->next)
	{
	printf("\t%d\t%d\n", zoom->reductionLevel, (int)(zoom->indexOffset - zoom->dataOffset));
	for (chrom=chromList; chrom != NULL; chrom = chrom->next)
	    {
	    struct bbiSummary *sum, *sumList = bbiSummariesInRegion(zoom, bbi,   chrom->id,0, chrom->size);
	     for (sum = sumList; sum != NULL; sum = sum->next)
	             {
		     printf("\t\t%s:%d-%d\n",chrom->name, sum->start, sum->end);
		     }
				       
	    }

	}
    }
printf("chromCount: %d\n", slCount(chromList));
if (optionExists("chroms"))
    for (chrom=chromList; chrom != NULL; chrom = chrom->next)
	printf("\t%s %d %d\n", chrom->name, chrom->id, chrom->size);
if (optionExists("as"))
    {
    char *asText = bigBedAutoSqlText(bbi);
    if (asText == NULL)
        printf("as:  n/a\n");
    else
	{
	printf("as:\n");
	printf("%s", asText);
	}
    }
struct bbiSummaryElement sum = bbiTotalSummary(bbi);
printLabelAndLongNumber("basesCovered", sum.validCount);
double meanDepth = 0, depthStd = 0;
if (sum.validCount > 0)
    {
    meanDepth = sum.sumData/sum.validCount;
    depthStd = calcStdFromSums(sum.sumData, sum.sumSquares, sum.validCount);
    }
printf("meanDepth (of bases covered): %f\n", meanDepth);
printf("minDepth: %f\n", sum.minVal);
printf("maxDepth: %f\n", sum.maxVal);
printf("std of depth: %f\n", depthStd);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 2)
    usage();
udcSetDefaultDir(optionVal("udcDir", udcDefaultDir()));
bigBedInfo(argv[1]);
if (verboseLevel() > 1)
    printVmPeak();
return 0;
}
