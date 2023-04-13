/* wgEncodeRegGenRa - Help generate some of the wgEncodeReg.ra file.. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "jksql.h"
#include "bigWig.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "wgEncodeRegGenRa - Help generate some of the wgEncodeReg.ra file.\n"
  "usage:\n"
  "   wgEncodeRegGenRa now\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

struct markInfo
/* Info on a histone mark or the like. */
    {
    char *part;		/* Part as used in file and track names */
    char *name;		/* Human readable name */
    char *blurb;	/* Short description */
    char *vis;		/* Visibility */
    int yMax;		/* Top before clipping */
    };


struct markInfo marks[] = {
    { "H3k4me1", "H3K4Me1", "Often Found Near Regulatory Elements", "full", 150},
    { "H3k4me3", "H3K4Me3", "Often Found Near Promoters", "full", 250},
    { "H3k9ac", "H3K9Ac", "Often Found Near Regulatory Elements", "hide", 200},
    { "H3k27ac", "H3K27Ac", "Often Found Near Regulatory Elements", "hide", 200},
    { "H3k27me3", "H3K27Me3", "Polycomb silenced", "hide", 150},
    { "H3k36me3", "H3K36Me3", "Marks Areas of RNA PolII Elongation", "hide", 100},
    { "H4k20me1", "H3K30Me1", "Associated with Active Accessible Regions", "hide", 150},
    { "Ctcf", "CTCF", "Insulator/DNA Looping", "hide", 300},
    { "Control", "Input Control", "Input Control - used to filter peaks", "hide", 150},
};

struct cellInfo
/* Info on a cell line */
    {
    char *part;		/* Part as used in file and track names */
    char *name;		/* Human readable name */
    int r,g,b;		/* Color */
    };

struct cellInfo cells[] = {
    { "Gm12878", "GM12878", 255,128,128, },
    { "H1hesc", "H1-hESC", 255,212,128, },
    { "Helas3", "HeLa-S3", 227, 255, 128,},
    { "Hepg2", "HepG2", 128,255,149,},
    { "Hsmm", "HSMM", 128,255,242, },
    { "Huvec", "HUVEC", 128,199,255, },
    { "K562", "K562", 149,128,255, },
    { "Nhek", "NHEK", 227,128,255, },
    { "Nhlf", "NHLF", 255,128,212, },
};

void wgEncodeRegGenRa(char *now)
/* wgEncodeRegGenRa - Help generate some of the wgEncodeReg.ra file. */
{
struct sqlConnection *conn = sqlConnect("hg19");
int i;
for (i=0; i<ArraySize(marks); ++i)
    {
    struct markInfo *mark = &marks[i];
    printf("    track wgEncodeRegMarkNorm%s\n", mark->part);
    printf("    container multiWig\n");
    printf("    noInherit on\n");
    printf("    type bigWig 0 10000\n");
    printf("    superTrack wgEncodeReg full\n");
    printf("    shortLabel Normaliz %s\n", mark->name);
    printf("    longLabel %s Normalized (%s) on 9 cell lines from ENCODE\n", mark->name, mark->blurb);
    printf("    visibility %s\n", mark->vis);
    printf("    viewLimits 0:%d\n", mark->yMax);
    printf("    maxHeightPixels 100:50:11\n");
    printf("    aggregate transparentOverlay\n");
    printf("    showSubtrackColorOnUi on\n");
    printf("    priority 2.%d\n", i+2);
    printf("    dragAndDrop subtracks\n");
    printf("\n");

    int j;
    for (j=0; j<ArraySize(cells); ++j)
        {
	struct cellInfo *cell = &cells[j];

	/* Get table, and see if it exists.  If so get info from it. */
	boolean tableExists = FALSE;
	int wigMax = 10000;
	char table[256];
	safef(table, sizeof(table), "wgEncodeBroadHistone%s%sNormSig", cell->part, mark->part);
	if (sqlTableExists(conn, table))
	    {
	    tableExists = TRUE;
	    char query[512];
	    sqlSafef(query, sizeof(query), "select fileName from %s", table);
	    char *fileName = sqlQuickString(conn, query);
	    struct bbiFile *bbi = bigWigFileOpen(fileName);
	    struct bbiSummaryElement sum = bbiTotalSummary(bbi);
	    wigMax = sum.maxVal;
	    bbiFileClose(&bbi);
	    }
	printf("\ttrack wgEncodeRegMarkNorm%s%s\n", mark->part, cell->part);
	printf("\ttable %s\n", table);
	if (!tableExists)
	    printf("\t#%s table doesn't exist\n", table);
	printf("\tshortLabel %s\n", cell->name);
	printf("\tlongLabel %s Normalized (%s) on %s Cells from ENCODE\n",
		mark->name, mark->blurb, cell->name);
	printf("\tparent wgEncodeRegMarkNorm%s\n", mark->part);
	printf("\tcolor %d,%d,%d\n", cell->r, cell->g, cell->b);
	printf("\ttype bigWig 0 %d\n", wigMax);
	printf("\n");
	}
    }
sqlDisconnect(&conn);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 2)
    usage();
wgEncodeRegGenRa(argv[1]);
return 0;
}
