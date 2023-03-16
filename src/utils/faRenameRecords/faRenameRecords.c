/* faRenameRecords - Rename all records in fasta file.. */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "dnaseq.h"
#include "fa.h"
#include "obscure.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
  "faRenameRecords - Rename all records in fasta file.\n"
  "usage:\n"
  "   faRenameRecords in.fa oldToNew.tab out.fa\n"
  "The oldToNew.tab is a two column file with the first column being the old name and\n"
  "the second column the new name.\n"
  "options:\n"
  "   -pass      Keep records from in.fa whose names are not found in oldToNew.tab\n"
  );
}

static struct optionSpec options[] = {
    {"pass", OPTION_BOOLEAN},
    {NULL, 0},
};

void faRenameRecords(char *inFa, char *oldToNew, char *outFa)
/* faRenameRecords - Rename all records in fasta file.. */
{
struct hash *subHash = hashTwoColumnFile(oldToNew);
struct lineFile *lf = lineFileOpen(inFa, TRUE);
FILE *f = mustOpen(outFa, "w");
boolean pass = optionExists("pass");
DNA *dna;
int size;
char *name;
while (faMixedSpeedReadNext(lf, &dna, &size, &name))
    {
    char *newName = hashFindVal(subHash, name);
    if (newName == NULL)
        {
        if (pass)
            newName = name;
        else
            errAbort("%s is in %s but not %s", name, inFa, oldToNew);
        }
    faWriteNext(f, newName, dna, size);
    }
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 4)
    usage();
faRenameRecords(argv[1], argv[2], argv[3]);
return 0;
}
