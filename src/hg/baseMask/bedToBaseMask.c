/* bedToBaseMask - write bed file as baseMask */

/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#include "common.h"
#include "bed.h"
#include "genomeRangeTreeFile.h"
#include "options.h"

/* FIXME:
 * - would be nice to be able to specify ranges in the same manner
 *   as featureBits
 * - should keep header lines in files
 * - don't need to save if infile records if stats output
 */

static struct optionSpec optionSpecs[] = {
    {NULL, 0}
};


void bedToBaseMask(char *bedFile, char *obama)
/* read first 3 columns of bed and write as baseMask */
{
struct rbTree *rt;
struct genomeRangeTree *tree = genomeRangeTreeNew();
struct bed *b, *bList = bedLoadNAll(bedFile, 3);
/* loop over all beds adding to range tree */
for (b = bList ; b ; b = b->next)
    {
    rt = genomeRangeTreeFindOrAddRangeTree(tree, b->chrom);
    bedIntoRangeTree(b, rt);
    }
bedFreeList(&bList);
genomeRangeTreeWrite(tree, obama); /* write the tree out */
genomeRangeTreeFree(&tree);
}


void usage(char *msg)
/* usage message and abort */
{
static char *usageMsg =
#include "bedToBaseMaskUsage.msg"
    ;
errAbort("%s\n%s", msg, usageMsg);
}

/* entry */
int main(int argc, char** argv)
{
char *bedFile, *obama;
optionInit(&argc, argv, optionSpecs);
--argc;
++argv;
if (argc == 0)
    usage("");
if (argc != 2)
    usage("wrong # args");
bedFile = argv[0];
obama = argv[1];

bedToBaseMask(bedFile, obama);
return 0;
}
