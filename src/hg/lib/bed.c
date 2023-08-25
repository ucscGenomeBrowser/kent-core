/* bed.c was originally generated by the autoSql program, which also 
 * generated bed.h and bed.sql.  This module links the database and the RAM 
 * representation of objects. */

/* Copyright (C) 2014 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */


#include "common.h"
#include "bed.h"
#include "minChromSize.h"
#include "hdb.h"


struct genePredExt *bedToGenePredExt(struct bed *bed)
/* Convert a single bed to a genePred structure. */
{
struct genePredExt *gp = NULL;
int i;
assert(bed);
AllocVar(gp);
gp->name = cloneString(bed->name);
gp->chrom = cloneString(bed->chrom);
//fails if strlen(bed->strand) == 2 as genepred has no space for zero terminator
//safef(gp->strand, sizeof(gp->strand), "%s", bed->strand);
gp->strand[0] = bed->strand[0];
gp->strand[1] = '\0';
assert(gp->strand[1] != '-');
gp->txStart = bed->chromStart;
gp->txEnd = bed->chromEnd;
gp->cdsStart = bed->thickStart;
gp->cdsEnd = bed->thickEnd;
gp->exonCount = bed->blockCount;
if(gp->exonCount != 0)
    {
    AllocArray(gp->exonStarts, gp->exonCount);
    AllocArray(gp->exonEnds, gp->exonCount);
    for(i=0; i<gp->exonCount; i++)
	{
	gp->exonStarts[i] = bed->chromStarts[i] + bed->chromStart;
	gp->exonEnds[i] = gp->exonStarts[i] + bed->blockSizes[i];
	}
    }
else 
    {
    gp->exonCount = 1;
    AllocArray(gp->exonStarts, gp->exonCount);
    AllocArray(gp->exonEnds, gp->exonCount);
    gp->exonStarts[0] = bed->chromStart;
    gp->exonEnds[0] = bed->chromEnd;
    }
return gp;
}

struct genePred *bedToGenePred(struct bed *bed)
/* Convert a single bed to a genePred structure. */
{
return (struct genePred *)bedToGenePredExt(bed);
}

struct bed *bedFromGenePred(struct genePred *genePred)
/* Convert a single genePred to a bed structure */
{
struct bed *bed;
int i, blockCount, *chromStarts, *blockSizes, chromStart;

/* A tiny bit of error checking on the genePred. */
if (genePred->txStart >= genePred->txEnd || genePred->cdsStart > genePred->cdsEnd)
    {
    errAbort("mangled genePred format for %s", genePred->name);
    }

/* Allocate bed and fill in from psl. */
AllocVar(bed);
bed->chrom = cloneString(genePred->chrom);
bed->chromStart = chromStart = genePred->txStart;
bed->chromEnd = genePred->txEnd;
bed->thickStart = genePred->cdsStart;
bed->thickEnd = genePred->cdsEnd;
bed->score = 0;
strncpy(bed->strand,  genePred->strand, sizeof(bed->strand));
bed->blockCount = blockCount = genePred->exonCount;
bed->blockSizes = blockSizes = (int *)cloneMem(genePred->exonEnds,(sizeof(int)*genePred->exonCount));
bed->chromStarts = chromStarts = (int *)cloneMem(genePred->exonStarts, (sizeof(int)*genePred->exonCount));
bed->name = cloneString(genePred->name);

/* Convert coordinates to relative and exnosEnds to blockSizes. */
for (i=0; i<blockCount; ++i)
    {
    blockSizes[i] -= chromStarts[i];
    chromStarts[i] -= chromStart;
    }
return bed;
}

boolean bedFilterChar(char value, enum charFilterType cft,
			  char *filterValues, boolean invert)
/* Return TRUE if value passes the filter. */
{
char thisVal;
if (filterValues == NULL)
    return(TRUE);
switch (cft)
    {
    case (cftIgnore):
	return(TRUE);
	break;
    case (cftSingleLiteral):
	return((value == *filterValues) ^ invert);
	break;
    case (cftMultiLiteral):
	while ((thisVal = *(filterValues++)) != 0)
	    {
	    if (value == thisVal)
		return(TRUE ^ invert);
	    }
	break;
    default:
	errAbort("illegal charFilterType: %d", cft);
	break;
    }
return(FALSE ^ invert);
}

boolean bedFilterString(char *value, enum stringFilterType sft, char **filterValues, boolean invert)
/* Return TRUE if value passes the filter. */
{
char *thisVal;

if (filterValues == NULL)
    return(TRUE);
switch (sft)
    {
    case (sftIgnore):
	return(TRUE);
	break;
    case (sftSingleLiteral):
	return(sameString(value, *filterValues) ^ invert);
	break;
    case (sftMultiLiteral):
	while ((thisVal = *(filterValues++)) != NULL)
	    if (sameString(value, thisVal))
		return(TRUE ^ invert);
	break;
    case (sftSingleRegexp):
	return(wildMatch(*filterValues, value) ^ invert);
	break;
    case (sftMultiRegexp):
	while ((thisVal = *(filterValues++)) != NULL)
	    if (wildMatch(thisVal, value))
		return(TRUE ^ invert);
	break;
    default:
	errAbort("illegal stringFilterType: %d", sft);
	break;
    }
return(FALSE ^ invert);
}

boolean bedFilterInt(int value, enum numericFilterType nft, int *filterValues)
/* Return TRUE if value passes the filter. */
/* This could probably be turned into a macro if performance is bad. */
{
if (filterValues == NULL)
    return(TRUE);
switch (nft)
    {
    case (nftIgnore):
	return(TRUE);
	break;
    case (nftLessThan):
	return(value < *filterValues);
	break;
    case (nftLTE):
	return(value <= *filterValues);
	break;
    case (nftEqual):
	return(value == *filterValues);
	break;
    case (nftNotEqual):
	return(value != *filterValues);
	break;
    case (nftGTE):
	return(value >= *filterValues);
	break;
    case (nftGreaterThan):
	return(value > *filterValues);
	break;
    case (nftInRange):
	return((value >= *filterValues) && (value < *(filterValues+1)));
	break;
    case (nftNotInRange):
	return(! ((value >= *filterValues) && (value < *(filterValues+1))));
	break;
    default:
	errAbort("illegal numericFilterType: %d", nft);
	break;
    }
return(FALSE);
}

boolean bedFilterDouble(double value, enum numericFilterType nft, double *filterValues)
/* Return TRUE if value passes the filter. */
/* This could probably be turned into a macro if performance is bad. */
{
if (filterValues == NULL)
    return(TRUE);
switch (nft)
    {
    case (nftIgnore):
	return(TRUE);
	break;
    case (nftLessThan):
	return(value < *filterValues);
	break;
    case (nftLTE):
	return(value <= *filterValues);
	break;
    case (nftEqual):
	return(value == *filterValues);
	break;
    case (nftNotEqual):
	return(value != *filterValues);
	break;
    case (nftGTE):
	return(value >= *filterValues);
	break;
    case (nftGreaterThan):
	return(value > *filterValues);
	break;
    case (nftInRange):
	return((value >= *filterValues) && (value < *(filterValues+1)));
	break;
    case (nftNotInRange):
	return(! ((value >= *filterValues) && (value < *(filterValues+1))));
	break;
    default:
	errAbort("illegal numericFilterType: %d", nft);
	break;
    }
return(FALSE);
}

boolean bedFilterLong(long long value, enum numericFilterType nft, long long *filterValues)
/* Return TRUE if value passes the filter. */
/* This could probably be turned into a macro if performance is bad. */
{
if (filterValues == NULL)
    return(TRUE);
switch (nft)
    {
    case (nftIgnore):
	return(TRUE);
	break;
    case (nftLessThan):
	return(value < *filterValues);
	break;
    case (nftLTE):
	return(value <= *filterValues);
	break;
    case (nftEqual):
	return(value == *filterValues);
	break;
    case (nftNotEqual):
	return(value != *filterValues);
	break;
    case (nftGTE):
	return(value >= *filterValues);
	break;
    case (nftGreaterThan):
	return(value > *filterValues);
	break;
    case (nftInRange):
	return((value >= *filterValues) && (value < *(filterValues+1)));
	break;
    case (nftNotInRange):
	return(! ((value >= *filterValues) && (value < *(filterValues+1))));
	break;
    default:
	errAbort("illegal numericFilterType: %d", nft);
	break;
    }
return(FALSE);
}

boolean bedFilterOne(struct bedFilter *bf, struct bed *bed)
/* Return TRUE if bed passes filter. */
{
int cmpValues[2];
if (bf == NULL)
    return TRUE;
if (!bedFilterString(bed->chrom, bf->chromFilter, bf->chromVals, bf->chromInvert))
    return FALSE;
if (!bedFilterInt(bed->chromStart, bf->chromStartFilter, bf->chromStartVals))
    return FALSE;
if (!bedFilterInt(bed->chromEnd, bf->chromEndFilter, bf->chromEndVals))
    return FALSE;
if (!bedFilterString(bed->name, bf->nameFilter, bf->nameVals, bf->nameInvert))
    return FALSE;
if (!bedFilterInt(bed->score, bf->scoreFilter, bf->scoreVals))
    return FALSE;
if (!bedFilterChar(bed->strand[0], bf->strandFilter, bf->strandVals,
		    bf->strandInvert))
    return FALSE;
if (!bedFilterInt(bed->thickStart, bf->thickStartFilter,
		    bf->thickStartVals))
    return FALSE;
if (!bedFilterInt(bed->thickEnd, bf->thickEndFilter,
		    bf->thickEndVals))
    return FALSE;
if (!bedFilterInt(bed->blockCount, bf->blockCountFilter,
		    bf->blockCountVals))
    return FALSE;
if (!bedFilterInt((bed->chromEnd - bed->chromStart),
		    bf->chromLengthFilter, bf->chromLengthVals))
    return FALSE;
if (!bedFilterInt((bed->thickEnd - bed->thickStart),
		    bf->thickLengthFilter, bf->thickLengthVals))
    return FALSE;
cmpValues[0] = cmpValues[1] = bed->thickStart;
if (!bedFilterInt(bed->chromStart, bf->compareStartsFilter,
		    cmpValues))
    return FALSE;
cmpValues[0] = cmpValues[1] = bed->thickEnd;
if (!bedFilterInt(bed->chromEnd, bf->compareEndsFilter, cmpValues))
    return FALSE;
return TRUE;
}

struct bed *bedFilterListInRange(struct bed *bedListIn, struct bedFilter *bf,
				 char *chrom, int winStart, int winEnd)
/* Given a bed list, a position range, and a bedFilter which specifies
 * constraints on bed fields, return the list of bed items that meet
 * the constraints.  If chrom is NULL, position range is ignored. */
{
struct bed *bedListOut = NULL, *bed;

for (bed=bedListIn;  bed != NULL;  bed=bed->next)
    {
    boolean passes = TRUE;
    if (chrom != NULL)
	{
	passes &= (sameString(bed->chrom, chrom) &&
		   (bed->chromStart < winEnd) &&
		   (bed->chromEnd   > winStart));
        }
    if (bf != NULL && passes)
	{
	passes &= bedFilterOne(bf, bed);
	}
    if (passes)
	{
	struct bed *newBed = cloneBed(bed);
	slAddHead(&bedListOut, newBed);
	}
    }
slReverse(&bedListOut);
return(bedListOut);
}

struct bed *bedFilterList(struct bed *bedListIn, struct bedFilter *bf)
/* Given a bed list and a bedFilter which specifies constraints on bed 
 * fields, return the list of bed items that meet the constraints. */
{
return bedFilterListInRange(bedListIn, bf, NULL, 0, 0);
}

struct bed *bedFilterByNameHash(struct bed *bedList, struct hash *nameHash)
/* Given a bed list and a hash of names to keep, return the list of bed 
 * items whose name is in nameHash. */
{
struct bed *bedListOut = NULL, *bed=NULL;

for (bed=bedList;  bed != NULL;  bed=bed->next)
    {
    if (bed->name == NULL)
	errAbort("bedFilterByNameHash: bed item at %s:%d-%d has no name.",
		 bed->chrom, bed->chromStart+1, bed->chromEnd);
    if (hashLookup(nameHash, bed->name) != NULL)
	{
	struct bed *newBed = cloneBed(bed);
	slAddHead(&bedListOut, newBed);
	}
    }

slReverse(&bedListOut);
return bedListOut;
}

struct bed *bedFilterByWildNames(struct bed *bedList, struct slName *wildNames)
/* Given a bed list and a list of names that may include wildcard characters,
 * return the list of bed items whose name matches at least one wildName. */
{
struct bed *bedListOut = NULL, *bed=NULL;
struct slName *wildName=NULL;

for (bed=bedList;  bed != NULL;  bed=bed->next)
    {
    for (wildName=wildNames;  wildName != NULL;  wildName=wildName->next)
	{
	if (bed->name == NULL)
	    errAbort("bedFilterByWildNames: bed item at %s:%d-%d has no name.",
		     bed->chrom, bed->chromStart+1, bed->chromEnd);
	if (wildMatch(wildName->name, bed->name))
	    {
	    struct bed *newBed = cloneBed(bed);
	    slAddHead(&bedListOut, newBed);
	    break;
	    }
	}
    }

slReverse(&bedListOut);
return bedListOut;
}

struct hash *bedsIntoKeeperHash(struct bed *bedList)
/* Create a hash full of bin keepers (one for each chromosome or contig.
 * The binKeepers are full of beds. */
{
struct hash *sizeHash = minChromSizeFromBeds(bedList);
struct hash *bkHash = minChromSizeKeeperHash(sizeHash);
struct bed *bed;
for (bed = bedList; bed != NULL; bed = bed->next)
    {
    struct binKeeper *bk = hashMustFindVal(bkHash, bed->chrom);
    binKeeperAdd(bk, bed->chromStart, bed->chromEnd, bed);
    }
hashFree(&sizeHash);
return bkHash;
}


