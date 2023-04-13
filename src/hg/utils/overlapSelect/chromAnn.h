/* chromAnn - chomosome annotations, generic object to store annotations from
 * other formats */

/* Copyright (C) 2010 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#ifndef CHROMANN_H
#define CHROMANN_H
struct rowReader;
struct coordCols;

enum chromAnnOpts
/* bit set of options */ 
{
    chromAnnCds        = 0x01,  /* use only CDS in blocks */
    chromAnnRange      = 0x02,  /* save entire range, not just blocks */
    chromAnnSaveLines  = 0x04,  /* save records as lines os they can be
                                 * outputted latter */
    chromAnnUseQSide   = 0x08   /* use query side of alignment */
};

struct chromAnn
/* chomosome annotations, generic object to store annotations from other
 * formats */
{
    struct chromAnn *next;
    char* name;      /* optional name of the item */
    char* chrom;     /* object coordinates */
    char strand;
    int start;       /* start of first block */
    int end;         /* end of last block */
    int totalSize;   /* size of all blocks */
    void *rec;       /* record that can be used to recreate the data */
    void (*recWrite)(struct chromAnn *ca, FILE *fh, char term); /* write record to file, with term character */
    void (*recFree)(struct chromAnn *ca);  /* free function for rec */
    struct chromAnnBlk *blocks;  /* ranges associated with this object */
    boolean used;    /* flag to indicated that this chromAnn has been used */
};

struct chromAnnRef
/* chromAnn reference list element */
{
    struct chromAnnRef *next;
    struct chromAnn *ref;
};

struct chromAnnBlk
/* specifies a range to select */
{
    struct chromAnnBlk *next;
    struct chromAnn *ca;  /* link back to chromAnn */
    int start;            /* block coordinates */
    int end;
};

struct chromAnnReader
/* interface object used to read chromAnn objects from various formats */
{
    struct chromAnn* (*caRead)(struct chromAnnReader *car);
    /* read the next object, returns NULL on eof */

    void (*carFree)(struct chromAnnReader **carPtr);
    /* function to free this object */

    unsigned opts;  /* options for reader */
    unsigned maxParsedColumns;  /* just drag along columns after this. */
    void *data;     /* data associated with this reader */
};

void chromAnnFree(struct chromAnn **caPtr);
/* free an object */

int chromAnnTotalBlockSize(struct chromAnn* ca);
/* count the total bases in the blocks of a chromAnn */

struct chromAnnReader *chromAnnBedReaderNew(char *fileName, unsigned opts,
                                            unsigned maxParsedCols);
/* construct a reader for a BED file */

struct chromAnnReader *chromAnnGenePredReaderNew(char *fileName, unsigned opts);
/* construct a reader for a genePred file */

struct chromAnnReader *chromAnnPslReaderNew(char *fileName, unsigned opts);
/* construct a reader for a PSL file */

struct chromAnnReader *chromAnnChainReaderNew(char *fileName, unsigned opts);
/* construct a reader for a chain file */

struct chromAnnReader *chromAnnTabReaderNew(char *fileName, struct coordCols* cols, unsigned opts);
/* construct a reader for an arbitrary tab file */

INLINE struct chromAnnRef *chromAnnRefNew(struct chromAnn *chromAnn)
/* construct a new chromAnnRef */
{
struct chromAnnRef *car;
AllocVar(car);
car->ref = chromAnn;
return car;
}

INLINE boolean chromAnnOnList(struct chromAnnRef *refList, struct chromAnn *chromAnn)
/* check in chromAnn is on list */
{
struct chromAnnRef *ref;
for (ref = refList; ref != NULL; ref = ref->next)
    if (ref->ref == chromAnn)
        return TRUE;
return FALSE;
}

INLINE void chromAnnRefAdd(struct chromAnnRef **refList, struct chromAnn *chromAnn)
/* add a chromAnn to the list, if it isn't already there  */
{
if (!chromAnnOnList(*refList, chromAnn))
    slSafeAddHead(refList, chromAnnRefNew(chromAnn));
}

int chromAnnRefLocCmp(const void *va, const void *vb);
/* Compare location of two chromAnnRef objects. */

#endif
