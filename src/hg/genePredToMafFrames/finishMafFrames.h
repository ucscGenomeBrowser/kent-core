/* finishMafFrames - link mafFrames objects to deal with spliced codons */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#ifndef FINISHMAFFRAMES_H
#define FINISHMAFFRAMES_H
struct orgGenes;

void finishMafFrames(struct orgGenes *genes);
/* Finish mafFrames build, linking mafFrames prev/next fields */

#endif
