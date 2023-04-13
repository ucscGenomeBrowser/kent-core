/* Copyright (C) 2011 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef LOCALNEARBESTFILTER_H
#define LOCALNEARBESTFILTER_H
struct cDnaQuery;

void localNearBestFilter(struct cDnaQuery *cdna, float localNearBest, int minLocalBestCnt);
/* Local near best in genome filter. Algorithm is based on
 * pslReps.  This avoids dropping exons in drafty genomes.  See source for
 * detailed doc. */
#endif
