/* exportedDataHubs.h was originally generated by the autoSql program, which also 
 * generated exportedDataHubs.c and exportedDataHubs.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef EXPORTEDDATAHUBS_H
#define EXPORTEDDATAHUBS_H

#define EXPORTEDDATAHUBS_NUM_COLS 5

extern char *exportedDataHubsCommaSepFieldNames;

struct exportedDataHubs
/* registered data hubs for quickLift */
    {
    struct exportedDataHubs *next;  /* Next in singly linked list. */
    unsigned id;	/*  Auto-incrementing ID */
    char *db;	/* Native assembly */
    char *label;	/* Label */
    char *description;	/* Description */
    char *path;	/* URL to data hub */
    };

void exportedDataHubsStaticLoad(char **row, struct exportedDataHubs *ret);
/* Load a row from exportedDataHubs table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct exportedDataHubs *exportedDataHubsLoad(char **row);
/* Load a exportedDataHubs from row fetched with select * from exportedDataHubs
 * from database.  Dispose of this with exportedDataHubsFree(). */

struct exportedDataHubs *exportedDataHubsLoadAll(char *fileName);
/* Load all exportedDataHubs from whitespace-separated file.
 * Dispose of this with exportedDataHubsFreeList(). */

struct exportedDataHubs *exportedDataHubsLoadAllByChar(char *fileName, char chopper);
/* Load all exportedDataHubs from chopper separated file.
 * Dispose of this with exportedDataHubsFreeList(). */

#define exportedDataHubsLoadAllByTab(a) exportedDataHubsLoadAllByChar(a, '\t');
/* Load all exportedDataHubs from tab separated file.
 * Dispose of this with exportedDataHubsFreeList(). */

struct exportedDataHubs *exportedDataHubsCommaIn(char **pS, struct exportedDataHubs *ret);
/* Create a exportedDataHubs out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new exportedDataHubs */

void exportedDataHubsFree(struct exportedDataHubs **pEl);
/* Free a single dynamically allocated exportedDataHubs such as created
 * with exportedDataHubsLoad(). */

void exportedDataHubsFreeList(struct exportedDataHubs **pList);
/* Free a list of dynamically allocated exportedDataHubs's */

void exportedDataHubsOutput(struct exportedDataHubs *el, FILE *f, char sep, char lastSep);
/* Print out exportedDataHubs.  Separate fields with sep. Follow last field with lastSep. */

#define exportedDataHubsTabOut(el,f) exportedDataHubsOutput(el,f,'\t','\n');
/* Print out exportedDataHubs as a line in a tab-separated file. */

#define exportedDataHubsCommaOut(el,f) exportedDataHubsOutput(el,f,',',',');
/* Print out exportedDataHubs as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

boolean exportedDataHubsEnabled();
/* Return TRUE if feature is available */

void printExportedDataHubs(char *db);
/* Fill out exported data hubs popup. */

unsigned registerExportedDataHub(char *db, char *hubUrl);
/* Add a hub to the exportedDataHubs table. */
#endif /* EXPORTEDDATAHUBS_H */

