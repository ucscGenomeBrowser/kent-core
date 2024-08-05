/* grp.h was originally generated by the autoSql program, which also 
 * generated grp.c and grp.sql.  This header links the database and
 * the RAM representation of objects. */

/* Copyright (C) 2008 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */

#ifndef GRP_H
#define GRP_H

struct grp
/* This describes a group of annotation tracks. */
    {
    struct grp *next;  /* Next in singly linked list. */
    char *name;	/* Group name.  Connects with trackDb.grp */
    char *label;	/* Label to display to user */
    float priority;	/* 0 is top */
    boolean defaultIsClosed; /* true if group is closed by default in hgTracks (this column is optional, and may be missing in some installations. */
    char *errMessage;   /* if non-NULL contains an error message for the group. */
    };

void grpStaticLoad(char **row, struct grp *ret);
/* Load a row from grp table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct grp *grpLoadAll(char *fileName);
/* Load all grp from a tab-separated file.
 * Dispose of this with grpFreeList(). */

struct grp *grpCommaIn(char **pS, struct grp *ret);
/* Create a grp out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new grp */

struct grp *grpLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all grp from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with grpFreeList(). */

void grpFree(struct grp **pEl);
/* Free a single dynamically allocated grp such as created
 * with grpLoad(). */

void grpFreeList(struct grp **pList);
/* Free a list of dynamically allocated grp's */

void grpOutput(struct grp *el, FILE *f, char sep, char lastSep);
/* Print out grp.  Separate fields with sep. Follow last field with lastSep. */

#define grpTabOut(el,f) grpOutput(el,f,'\t','\n');
/* Print out grp as a line in a tab-separated file. */

#define grpCommaOut(el,f) grpOutput(el,f,',',',');
/* Print out grp as a comma separated list including final comma. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

int grpCmpPriorityLabel(const void *va, const void *vb);
/* Compare to sort based on priority and label. */

int grpCmpPriority(const void *va, const void *vb);
/* Compare to sort based on priority. */

int grpCmpName(const void *va, const void *vb);
/* Compare to sort based on name. */

void grpSuperimpose(struct grp **listA, struct grp **listB);
/* Replace all the grps in listA with the same names of those in
 * listB.  Ones in B that aren't the same get put in A too.  
 * The function returns with the new list A, and the empty list
 * B. */

struct grp *grpDup(struct grp *grp);
/* duplicate a grp structure */

#endif /* GRP_H */

