/* fakeChromGraph - Generate fake chrom graph data. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "fakeChromGraph - Generate fake chrom graph data\n"
  "usage:\n"
  "   fakeChromGraph chromSizes innerPeriod outerPeriod baseSpace output\n"
  "   fakeChromGraph XXX\n"
  "options:\n"
  "   -xxx=XXX\n"
  );
}

static struct optionSpec options[] = {
   {NULL, 0},
};

int needNum(char *s)
/* Get number or die trying */
{
int x = atoi(s);
if (x <= 0) usage();
return x;
}

#define PI 3.14159

void fakeChromGraph(char *chromFile, char *ascInner, char *ascOuter, 
	char *ascSpace, char *outName)
/* fakeBedGraph - Make a bed graph format file that contains fake data.. */
{
int inner = needNum(ascInner);
int outer = needNum(ascOuter);
int space = needNum(ascSpace);
struct lineFile *lf = lineFileOpen(chromFile, TRUE);
char *row[2];
FILE *f = mustOpen(outName, "w");
while (lineFileRow(lf, row))
    {
    char *chrom = row[0];
    int size = lineFileNeedNum(lf, row, 1);
    if (size > inner)
        {
	int pos;
	for (pos=0; pos<size; pos += space)
	    {
	    double innerAngle = 2*PI*pos/inner;
	    double outerAngle = 2*PI*pos/outer;
	    double val = 100.0 + 100.0*sin(innerAngle) * sin(outerAngle);
	    fprintf(f, "%s\t%d\t%f\n", chrom, pos, val);
	    }
	}
    }
}


int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 6)
    usage();
fakeChromGraph(argv[1], argv[2], argv[3], argv[4], argv[5]);
return 0;
}
