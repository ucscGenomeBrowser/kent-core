/* interaction.h was originally generated by the autoSql program, which also 
 * generated interaction.c and interaction.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef INTERACTION_H
#define INTERACTION_H

#ifndef JKSQL_H
#include "jksql.h"
#endif

#define INTERACTION_NUM_COLS 3

struct interaction
/* gene network interaction - weighted directed edge  */
    {
    struct interaction *next;  /* Next in singly linked list. */
    char *fromX;	/* from- gene, protein, node, etc. */
    char *toY;	/* to- gene, protein, node, etc. */
    float score;	/* weighted value for score */
    };

void interactionStaticLoad(char **row, struct interaction *ret);
/* Load a row from interaction table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct interaction *interactionLoad(char **row);
/* Load a interaction from row fetched with select * from interaction
 * from database.  Dispose of this with interactionFree(). */

struct interaction *interactionLoadAll(char *fileName);
/* Load all interaction from whitespace-separated file.
 * Dispose of this with interactionFreeList(). */

struct interaction *interactionLoadAllByChar(char *fileName, char chopper);
/* Load all interaction from chopper separated file.
 * Dispose of this with interactionFreeList(). */

#define interactionLoadAllByTab(a) interactionLoadAllByChar(a, '\t');
/* Load all interaction from tab separated file.
 * Dispose of this with interactionFreeList(). */

struct interaction *interactionLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all interaction from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with interactionFreeList(). */

void interactionSaveToDb(struct sqlConnection *conn, struct interaction *el, char *tableName, int updateSize);
/* Save interaction as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. Strings are automatically escaped to allow insertion into the database. */

struct interaction *interactionCommaIn(char **pS, struct interaction *ret);
/* Create a interaction out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new interaction */

void interactionFree(struct interaction **pEl);
/* Free a single dynamically allocated interaction such as created
 * with interactionLoad(). */

void interactionFreeList(struct interaction **pList);
/* Free a list of dynamically allocated interaction's */

void interactionOutput(struct interaction *el, FILE *f, char sep, char lastSep);
/* Print out interaction.  Separate fields with sep. Follow last field with lastSep. */

#define interactionTabOut(el,f) interactionOutput(el,f,'\t','\n');
/* Print out interaction as a line in a tab-separated file. */

#define interactionCommaOut(el,f) interactionOutput(el,f,',',',');
/* Print out interaction as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

#endif /* INTERACTION_H */

