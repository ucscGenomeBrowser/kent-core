/* spaceSaver - routines that help layout 2-D objects into a
 * minimum number of rows so that no two objects overlap
 * within the same row. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include "spaceSaver.h"



struct spaceSaver *spaceSaverMaxCellsNew(int winStart, int winEnd, int maxRows, int maxCells)
/* Create a new space saver around the given window.   */
{
struct spaceSaver *ss;
float winWidth;

AllocVar(ss);
ss->winStart = winStart;
ss->winEnd = winEnd;
ss->maxRows = maxRows;
winWidth = winEnd - winStart;
ss->cellsInRow = winWidth;
while (ss->cellsInRow > maxCells)
    ss->cellsInRow /= 2;
ss->scale = ss->cellsInRow/winWidth;
return ss;
}

struct spaceSaver *spaceSaverNew(int winStart, int winEnd, int maxRows)
/* Create a new space saver around the given window.   */
{
return spaceSaverMaxCellsNew(winStart, winEnd, maxRows, 800);
}

void spaceSaverFree(struct spaceSaver **pSs)
/* Free up a space saver. */
{
struct spaceSaver *ss = *pSs;
if (ss != NULL)
    {
    struct spaceRowTracker *srt;
    for (srt = ss->rowList; srt != NULL; srt = srt->next)
	freeMem(srt->used);
    slFreeList(&ss->rowList);
    slFreeList(&ss->nodeList);
    freez(pSs);
    }
}

static boolean allClear(bool *b, int count)
/* Return TRUE if count bools starting at b are all 0 */
{
int i;
for (i=0; i<count; ++i)
    if (b[i])
	return FALSE;
return TRUE;
}

struct spaceNode *spaceSaverAddOverflowMultiOptionalPadding(struct spaceSaver *ss, 
	                struct spaceRange *rangeList, struct spaceNode *nodeList, 
					boolean allowOverflow, boolean doPadding)
/* Add new nodes for multiple windows to space saver. Returns NULL if can't fit item in
 * and allowOverflow == FALSE. If allowOverflow == TRUE then put items
 * that won't fit in first row (ends up being last row after
 * spaceSaverFinish()). Allow caller to suppress padding between items 
 * (show adjacent items on single row) */
{
struct spaceRowTracker *startSrt, *freeSrt = NULL;
int rowIx = 0;
struct spaceNode *sn;

if (ss->isFull)
    return NULL;

struct spaceRange *range;
struct spaceRange *cellRanges = NULL, *cellRange;
int maxHeight = 0;
for (range = rangeList; range; range = range->next)
    {
    AllocVar(cellRange);
    int start = range->start;
    int end   = range->end;
    if ((start -= ss->winStart) < 0)
    	start = 0;
    end -= ss->winStart;	/* We'll clip this in cell coordinates. */
    cellRange->start = round(start * ss->scale);
    int padding = doPadding ? 1 : 0;
    cellRange->end = round(end * ss->scale) + padding;
    if (cellRange->end > ss->cellsInRow)
	cellRange->end = ss->cellsInRow;
    cellRange->width = cellRange->end - cellRange->start;
    if (range->height < 1)
        errAbort ("Attempting to insert item with 0 height into spaceSaver");
    cellRange->height = range->height;
    if (range->height > maxHeight)
        maxHeight = range->height;
    slAddHead(&cellRanges, cellRange);
    }
slReverse(&cellRanges);


if (ss->vis == 2) // tvFull for BEDLIKE tracks does not pack, so force a new line
    {
    rowIx = ss->rowCount;
    }
else
    {
    /* Find free row. */
    for (startSrt = ss->rowList; startSrt != NULL; startSrt = startSrt->next)
	{
	bool freeFound = TRUE;
        // Check if we can pack each of the cells into this row
	for (cellRange = cellRanges; cellRange; cellRange = cellRange->next)
	    { // TODO possibly can just calculate cellRange->width instead of storing it?
            struct spaceRowTracker *srt = startSrt; // for packing something that fills multiple rows
            int j;
            for (j=0; j<cellRange->height; j++)
                {
                if (srt == NULL)  // can trivially pack into rows that aren't yet allocated
                    break;
                if (!allClear(srt->used + cellRange->start, cellRange->width))
                    {
                    // Nope, at least one cell won't fit in this row
                    freeFound = FALSE;
                    break;
                    }
                srt = srt->next;
                }
                if (!freeFound)
                    // No point testing the rest of the cells; we've already found one that won't fit here
                    break;
	    }
	if (freeFound)
	    {
	    freeSrt = startSrt;
	    break;
	    }
	++rowIx;
	}
    }

/* If we need to make new row(s), do so up to the limit of either what's needed to draw the item or maxRows rows.
 * If we successfully added any needed rows, freeSrt should point to the start row.  Otherwise it's NULL.
 */
if (rowIx + maxHeight > ss->rowCount)
    {
    struct spaceRowTracker *newSrt = NULL;
    for (;ss->rowCount < rowIx + maxHeight; ss->rowCount++)
        {
        if (ss->rowCount >= ss->maxRows)
            {
            // this just became an overflow item
            freeSrt = NULL;
            rowIx = ss->rowCount;
            break;
            }
        AllocVar(newSrt);
        newSrt->used = needMem(ss->cellsInRow);
        slAddTail(&ss->rowList, newSrt);
        if (freeSrt == NULL)
            freeSrt = newSrt; // This item was assigned to go on all new rows, so set freeSrt to the first new row
        }
    }

if (freeSrt == NULL)
    {
    if (ss->rowCount + maxHeight > ss->maxRows)
	{
	/* Abort if too many rows and no overflow allowed. */
	if(!allowOverflow) 
	    {
	    ss->isFull = TRUE;
	    return NULL;
	    }
        ss->hasOverflowRow = TRUE;
	}
    }

/* Mark that part of row used (except in overflow case). */
if(freeSrt != NULL)
    {
    for (cellRange = cellRanges; cellRange; cellRange = cellRange->next)
	{
        struct spaceRowTracker *srt = freeSrt;
        int j;
        for (j=0; j<cellRange->height; j++)
            {
            memset(srt->used + cellRange->start, 1, cellRange->width);
            srt = srt->next;
            }
	}
    }

// Instead of allocating nodes, just shift them
// from the input to parent window's list while setting the row
//
struct spaceNode *snNext;
for (sn=nodeList; sn; sn=snNext)
    {
    /* Make a space node. If allowing overflow it will
     all end up in the last row. */
    sn->row = rowIx;
    snNext = sn->next;
    slAddHead(&sn->parentSs->nodeList, sn);
    }
return nodeList;
}

struct spaceNode *spaceSaverAddOverflowMulti(struct spaceSaver *ss, 
	struct spaceRange *rangeList, struct spaceNode *nodeList, 
					boolean allowOverflow)
/* Add new nodes for multiple windows to space saver. Returns NULL if can't fit item in
 * and allowOverflow == FALSE. If allowOverflow == TRUE then put items
 * that won't fit in first row (ends up being last row after
 * spaceSaverFinish()). */
{
return spaceSaverAddOverflowMultiOptionalPadding(ss, rangeList, nodeList, allowOverflow, TRUE);
}


struct spaceNode *spaceSaverAddOverflowExtended(struct spaceSaver *ss, int start, int end, int height,
					void *val, boolean allowOverflow, struct spaceSaver *parentSs, bool noLabel)
/* Add a new node to space saver. Returns NULL if can't fit item in
 * and allowOverflow == FALSE. If allowOverflow == TRUE then put items
 * that won't fit in last row. parentSs allows specification of destination for node from alternate window.*/
{
struct spaceRange *range;
AllocVar(range);
range->start = start;
range->end = end;
range->height = height;
struct spaceNode *node;
AllocVar(node);
node->val = val;
node->parentSs = parentSs;
node->noLabel = noLabel;
return spaceSaverAddOverflowMulti(ss, range, node, allowOverflow);
}

struct spaceNode *spaceSaverAddOverflow(struct spaceSaver *ss, int start, int end, 
					void *val, boolean allowOverflow)
/* Add a new node to space saver. Returns NULL if can't fit item in
 * and allowOverflow == FALSE. If allowOverflow == TRUE then put items
 * that won't fit in last row. */
{
return spaceSaverAddOverflowExtended(ss, start, end, 1, val, allowOverflow, ss, FALSE);
}

struct spaceNode *spaceSaverAdd(struct spaceSaver *ss, 
	int start, int end, void *val)
/* Add a new node to space saver. Returns NULL if can't fit
 * item in. */
{
return spaceSaverAddOverflow(ss, start, end, val, FALSE);
}

struct spaceNode *spaceSaverAddMulti(struct spaceSaver *ss, 
	struct spaceRange *rangeList, struct spaceNode *nodeList)
/* Add new nodes for multiple windows to space saver. */
{
return spaceSaverAddOverflowMulti(ss, rangeList, nodeList, FALSE);
}

int spaceSaverGetRowHeightsTotal(struct spaceSaver *ss)
/* Return height of all rows. Used by tracks with variable height items */
{
assert(ss != NULL);
if (ss->rowSizes == NULL)
    return 0;
int height = 0;
int i;
for (i=0; i<ss->rowCount; i++)
    {
    height += ss->rowSizes[i];
    }
return height;
}

void spaceSaverSetRowHeights(struct spaceSaver *ss, struct spaceSaver *holdSs, int (*itemHeight)(void *item))
/* Determine maximum height of items in a row. Return total height.
   Used by tracks with variable height items */
{
assert(ss != NULL);
assert(holdSs != NULL);
if (!holdSs->rowSizes)
    AllocArray(holdSs->rowSizes, ss->rowCount);
struct spaceNode *sn;
for (sn = ss->nodeList; sn != NULL; sn = sn->next)
    {
    holdSs->rowSizes[sn->row] = max(holdSs->rowSizes[sn->row], itemHeight(sn->val));
    }
}

void spaceSaverFinish(struct spaceSaver *ss)
/* Tell spaceSaver done adding nodes. */
{
slReverse(&ss->nodeList);
}
