/* dbLoadPartitions - get information about partitions to process */

/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#ifndef DBLOADPARTITIONS_H
#define DBLOADPARTITIONS_H
struct gbIndex;
struct dbLoadOptions;

struct gbSelect* dbLoadPartitionsGet(struct dbLoadOptions* options,
                                     struct gbIndex* index);
/* build a list of partitions to load based on the command line and
 * conf file options and whats in the index */

boolean dbLoadNonCoding(char *db, struct gbSelect* select);
/* determine if non-protein coding sequences should be loaded for this
 * partition */

#endif
