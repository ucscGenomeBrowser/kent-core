/* hgChroms - print chromosomes for a genome. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "hdb.h"
#include "options.h"


void usage()
/* Explain usage and exit. */
{
errAbort(
    "hgChroms - print chromosomes for a genome.\n"
    "\n"
    "usage:\n"
    "   hgChroms [options] db\n"
    "\n"
    "Options:\n"
    "   -noRandom - omit random and Un chromsomes\n"
    "   -noHap - omit _hap|_alt chromsomes\n"
    "   -noPrefix - omit \"chr\" prefix\n"
  );
}


/* command line */
static struct optionSpec optionSpec[] = {
    {"noRandom", OPTION_BOOLEAN},
    {"noHap", OPTION_BOOLEAN},
    {"noPrefix", OPTION_BOOLEAN},
    {NULL, 0}
};

boolean noRandom;
boolean noHap;
boolean noPrefix;

bool inclChrom(struct slName *chrom)
/* check if a chromosome should be included */
{
return  !((noRandom && (endsWith(chrom->name, "_random")
                        || startsWith("chrUn", chrom->name)))
          || (noHap && haplotype(chrom->name)));
}

void hgChroms(char *db)
/* hgChroms - print chromosomes for a genome. */
{
struct slName *chrom, *chroms = hAllChromNames(db);
for (chrom = chroms; chrom != NULL; chrom = chrom->next)
    {
    if (inclChrom(chrom))
        {
        if (noPrefix && startsWith("chr", chrom->name))
            {
            printf("%s\n", chrom->name + strlen("chr"));
            }
        else
            {
            printf("%s\n", chrom->name);
            }
        }
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpec);
if (argc != 2)
    usage();
noRandom = optionExists("noRandom");
noHap = optionExists("noHap");
noPrefix = optionExists("noPrefix");
hgChroms(argv[1]);
return 0;
}
