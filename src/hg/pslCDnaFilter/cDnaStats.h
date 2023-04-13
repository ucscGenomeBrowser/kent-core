/* cDnaStats - collect statistics on alignment filtering */

/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#ifndef CDNASTATS_H
#define CDNASTATS_H

struct cDnaCnts
/* counts kept for alignments */
{
    unsigned queries;        /* count of queries; */
    unsigned aligns;         /* count of alignments */
    unsigned prevAligns;     /* previous aligns count, used to update queires */
};

struct cDnaStats
/* all statistics collected on cDNA filtering */
{
    struct cDnaCnts totalCnts;             /* number read */
    struct cDnaCnts keptCnts;              /* number of kept */
    struct cDnaCnts weirdOverCnts;         /* number of weird overlapping PSLs */
    struct cDnaCnts weirdKeptCnts;         /* weird overlapping PSLs not dropped  */
    struct cDnaCnts badDropCnts;           /* number bad that were dropped */ 
    struct cDnaCnts minQSizeDropCnts;      /* number below min size dropped */ 
    struct cDnaCnts weirdDropCnts;         /* weird overlapping PSLs dropped because of weird overlap  */
    struct cDnaCnts overlapDropCnts;       /* number dropped due to overlap */
    struct cDnaCnts minIdDropCnts;         /* number dropped less that min id */
    struct cDnaCnts minCoverDropCnts;      /* number dropped less that min cover */
    struct cDnaCnts minAlnSizeDropCnts;    /* number dropped due minAlnSize */
    struct cDnaCnts minNonRepSizeDropCnts; /* number dropped due minNonRepSize */
    struct cDnaCnts maxRepMatchDropCnts;   /* number dropped due maxRepMatch */
    struct cDnaCnts maxAlignsDropCnts;     /* number dropped due to over max */
    struct cDnaCnts localBestDropCnts;     /* number dropped due to local near best */
    struct cDnaCnts globalBestDropCnts;    /* number dropped due to global near best */
    struct cDnaCnts minSpanDropCnts;       /* number dropped due to under minSpan */
    struct cDnaCnts nonUniqueMap;          /* number dropped due to non-unique mappings */
    struct cDnaCnts blackListCnts;         /* number dropped due to black list*/
};

void cDnaStatsUpdate(struct cDnaStats *stats);
/* update counters after processing a one cDNAs worth of alignments */

void cDnaStatsPrint(struct cDnaStats *stats, FILE* fh);
/* print filter stats to file  */

#endif
