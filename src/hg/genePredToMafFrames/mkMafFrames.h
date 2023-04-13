/* mkMafFrames - build mafFrames objects for exons */

/* Copyright (C) 2006 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#ifndef MKMAFFRAMES_H
#define MKMAFFRAMES_H
struct orgGenes;

void mkMafFramesForMaf(char *targetDb, struct orgGenes *orgs,
                       char *mafFilePath);
/* create mafFrames objects from an MAF file */
#endif
