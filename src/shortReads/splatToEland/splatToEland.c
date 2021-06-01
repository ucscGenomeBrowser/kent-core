/* splatToEland - Convert from splat to eland format.. */
/* This file is copyright 2008 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "splatAli.h"
#include "verbose.h"


boolean multi = FALSE;
char *reads = NULL;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "splatToEland - Convert from splat to eland format.\n"
  "usage:\n"
  "   splatToEland in.splat out.vmf\n"
  "options:\n"
  "   -multi - produce multiple mapping format rather than omitting multiply-mapped\n"
  "   -reads=reads.fa - read sequences, used if you want to put out information on\n"
  "                     reads that _don't_ map.\n"
  );
}

static struct optionSpec options[] = {
   {"multi", OPTION_BOOLEAN},
   {"reads", OPTION_STRING},
   {NULL, 0},
};

boolean needSort(struct splatAli *list)
/* Return TRUE if list needs to be sorted. */
{
struct splatAli *el, *next;
if ((el = list) == NULL)
    return FALSE;
if ((next = el->next) == NULL)
    return FALSE;
while (next != NULL)
    {
    if (strcmp(el->readName, next->readName) > 0)
       return TRUE;
    el = next;
    next = next->next;
    }
return FALSE;
}

void findNextDifferent(struct splatAli *list, struct splatAli **retNext, int *retSameCount)
/* Find next element of list that is from a different read. Return it in retNext, and
 * the number of items that are the same in retSameCount. */
{
char *readName = list->readName;
int sameCount = 1;
struct splatAli *el;
for (el = list->next; el != NULL; el = el->next)
    {
    if (differentString(el->readName, readName))
        break;
    ++sameCount;
    }
*retNext = el;
*retSameCount = sameCount;
}

struct splatAli *findFirstGood(struct splatAli *start, struct splatAli *end, int minScore)
/* Return first alignment that is minScore or better. */
{
struct splatAli *el;
for (el = start; el != end; el = el->next)
    if (splatAliScore(el->alignedBases) >= minScore)
        return el;
return NULL;
}

char *splatAliBasesOnly(char *aligned)
/* Strip out ^ and - chars, and upper case everything.  FreeMem result when done. */
{
char *bases = cloneString(aligned);
stripChar(bases, '-');
stripChar(bases, '^');
touppers(bases);
return bases;
}

int minSubs(int subCounts[3])
/* Return minimum number of substitutions. */
{
int i;
for (i=0; i<3; ++i)
    if (subCounts[i])
        return i;
internalErr();
return -1;
}

void elandSingleOutput(struct splatAli *start, struct splatAli *end,
	int bestScore, int bestCount, int subCounts[3], FILE *f)
/* Output information on read in single mapping format. Multiple
 * mappings will get swallowed. */
{
static int lineId = 0;
fprintf(f, "%d\t", ++lineId);
fprintf(f, ">%s\t", start->readName);
char *bases = splatAliBasesOnly(start->alignedBases);
int baseCount = strlen(bases);
fprintf(f, "%s\t", bases);
freeMem(bases);
if (bestCount == 1)
   {
   struct splatAli *splat = findFirstGood(start, end, bestScore);
   fprintf(f, "U%d\t", minSubs(subCounts));
   int i;
   for (i=0; i<3; ++i)
       fprintf(f, "%d\t", subCounts[i]);
   fprintf(f, "%s\t", splat->chrom);
   fprintf(f, "%d\t", splat->chromStart+1);
   fprintf(f, "%c\t", (splat->strand[0] == '-' ? 'R' : 'F'));
   fprintf(f, "..");
   int aliSize = strlen(splat->alignedBases);
   int baseOffset = 0;
   int subCount = 0;
   for (i=0; i<aliSize; ++i)
       {
       char c = splat->alignedBases[i];
       if (c == '^')  // We can't really output these, lower case char follows though
           ;
       else 
           {
	   if (c == '-' || islower(c))
	       {
	       if (splat->strand[0] == '+')
		   fprintf(f, "\t%d", baseOffset+1);
	       else
	           fprintf(f, "\t%d", baseCount - baseOffset);
	       if (++subCount >= 2)
	           break;
	       }
	   if (c != '-')
	       ++baseOffset;
	   }
       }
   fprintf(f, "\n");
   }
else
   {
   fprintf(f, "R%d", minSubs(subCounts));
   int i;
   for (i=0; i<3; ++i)
       fprintf(f, "\t%d", subCounts[i]);
   fprintf(f, "\n");
   }
}

int countSubs(char *seq)
/* Count substitutions in seq.  Indels count as a substitution. */
{
int count = 0;
char c;
while ((c = *seq++) != 0)
    {
    if (islower(c) || c == '-')
       ++count;
    }
return count;
}

void countSubsInList(struct splatAli *start, struct splatAli *end, int subCounts[0])
/* Count up number of alignments with zero, one, or two substitutions.  Indels count
 * as a substitution.  More than 2 subs count as 2. */
{
struct splatAli *el;
subCounts[0] = subCounts[1] = subCounts[2] = 0;
for (el = start; el != end; el = el->next)
    {
    int subs = countSubs(el->alignedBases);
    if (subs > 2)
        subs = 2;
    subCounts[subs] += 1;
    }
}

int cmpSplatRefChromStart(const void *va, const void *vb)
/* Compare to sort based on query start. */
{
struct slRef *aRef = *((struct slRef **)va);
struct slRef *bRef = *((struct slRef **)vb);
struct splatAli *a = aRef->val;
struct splatAli *b = bRef->val;
int dif;
dif = strcmp(a->chrom, b->chrom);
if (dif == 0)
    dif = a->chromStart - b->chromStart;
return dif;
}

void elandMultiOutput(struct splatAli *start, struct splatAli *end,
	int bestScore, int bestCount, int subCounts[3], FILE *f)
/* Output information on read in multiple mapping format. */
{
fprintf(f, ">%s\t", start->readName);
char *bases = splatAliBasesOnly(start->alignedBases);
fprintf(f, "%s\t", bases);
freeMem(bases);
fprintf(f, "%d:%d:%d\t", subCounts[0], subCounts[1], subCounts[2]);

/* Create a reference list of mappings that are in the best slot for substitutions,
 * sorted by chromosome location. */
int targetSub = minSubs(subCounts);
struct slRef *refList = NULL, *ref;
struct splatAli *splat;
for (splat = start; splat != end; splat = splat->next)
    {
    if (countSubs(splat->alignedBases) == targetSub)
       refAdd(&refList, splat);
    }
slSort(&refList, cmpSplatRefChromStart);

/* Output comma separated list of mappings. */
char *chrom = "";
for (ref = refList; ref != NULL; ref = ref->next)
    {
    splat = ref->val;
    if (!sameString(chrom, splat->chrom))
	{
	chrom = splat->chrom;
        fprintf(f, "%s:", chrom);
	}
    fprintf(f, "%d", splat->chromStart+1);
    fprintf(f, "%c", (splat->strand[0] == '-' ? 'R' : 'F'));
    fprintf(f, "%d", targetSub);
    if (ref->next != NULL)
        fprintf(f, ",");
    }
fprintf(f, "\n");
}


void splatToEland(char *inName, char *outName)
/* splatToEland - Convert from splat to eland format.. */
{
verboseTimeInit();
struct splatAli *list = splatAliLoadAll(inName);
verboseTime(1, "Loaded %d sequences from %s", slCount(list), inName);
if (needSort(list))
    {
    slSort(&list, splatAliCmpReadName);
    verboseTime(1, "Sorted by read name");
    }
FILE *f = mustOpen(outName, "w");
struct splatAli *el, *next;
for (el = list; el != NULL; el = next)
    {
    int sameCount;
    findNextDifferent(el, &next, &sameCount);
    int bestScore, bestCount;
    splatAliLookForBest(el, next, &bestScore, &bestCount);
    int subCounts[3];
    countSubsInList(el, next, subCounts);
    if (multi)
        elandMultiOutput(el, next, bestScore, bestCount, subCounts, f);
    else
        elandSingleOutput(el, next, bestScore, bestCount, subCounts, f);
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
multi = optionExists("multi");
reads = optionVal("reads", reads);
splatToEland(argv[1], argv[2]);
return 0;
}
