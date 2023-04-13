/* dbLoadPartitions - get information about partitions to process */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "dbLoadPartitions.h"
#include "dbLoadOptions.h"
#include "gbIndex.h"
#include "gbDefs.h"
#include "gbRelease.h"
#include "portable.h"

static void getSelPartitions(struct dbLoadOptions* options,
                             struct gbIndex* index,
                             unsigned srcDb,
                             unsigned type,
                             struct gbSelect** selectList)
/* find selected partitions based on attributes and options */
{
unsigned orgCats = 0;
if (dbLoadOptionsGetAttr(options, srcDb, type, GB_NATIVE)->load)
    orgCats |= GB_NATIVE;
if (dbLoadOptionsGetAttr(options, srcDb, type, GB_XENO)->load)
    orgCats |= GB_XENO;

if (orgCats)
    {
    struct gbSelect* select
        = gbIndexGetPartitions(index, GB_ALIGNED, srcDb,
                               options->relRestrict, type, orgCats,
                               options->accPrefixRestrict);
    if ((select != NULL) && (options->flags & DBLOAD_ADD_VERSION))
	select->hasVersion = TRUE;
    *selectList = slCat(*selectList, select);
    }
}

struct gbSelect* dbLoadPartitionsGet(struct dbLoadOptions* options,
                                     struct gbIndex* index)
/* build a list of partitions to load based on the command line and
 * conf file options and whats in the index */
{
struct gbSelect* selectList = NULL;
getSelPartitions(options, index, GB_GENBANK, GB_MRNA, &selectList);
getSelPartitions(options, index, GB_GENBANK, GB_EST, &selectList);
getSelPartitions(options, index, GB_REFSEQ, GB_MRNA, &selectList);
return selectList;
}


boolean dbLoadNonCoding(char *db, struct gbSelect* select)
/* determine if non-protein coding sequences should be loaded for this
 * partition */
{
return TRUE;
}
