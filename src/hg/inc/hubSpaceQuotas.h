/* hubSpaceQuotas.h was originally generated by the autoSql program, which also 
 * generated hubSpaceQuotas.c and hubSpaceQuotas.sql.  This header links the database and
 * the RAM representation of objects. */

#ifndef HUBSPACEQUOTAS_H
#define HUBSPACEQUOTAS_H

#include "jksql.h"
#define HUBSPACEQUOTAS_NUM_COLS 2

extern char *hubSpaceQuotasCommaSepFieldNames;

struct hubSpaceQuotas
/* Table for per-user quotas that differ from the default quota */
    {
    struct hubSpaceQuotas *next;  /* Next in singly linked list. */
    char *userName;	/* userName in gbMembers */
    long long quota;	/* the users quota */
    };

void hubSpaceQuotasStaticLoad(char **row, struct hubSpaceQuotas *ret);
/* Load a row from hubSpaceQuotas table into ret.  The contents of ret will
 * be replaced at the next call to this function. */

struct hubSpaceQuotas *hubSpaceQuotasLoadByQuery(struct sqlConnection *conn, char *query);
/* Load all hubSpaceQuotas from table that satisfy the query given.  
 * Where query is of the form 'select * from example where something=something'
 * or 'select example.* from example, anotherTable where example.something = 
 * anotherTable.something'.
 * Dispose of this with hubSpaceQuotasFreeList(). */

void hubSpaceQuotasSaveToDb(struct sqlConnection *conn, struct hubSpaceQuotas *el, char *tableName, int updateSize);
/* Save hubSpaceQuotas as a row to the table specified by tableName. 
 * As blob fields may be arbitrary size updateSize specifies the approx size
 * of a string that would contain the entire query. Arrays of native types are
 * converted to comma separated strings and loaded as such, User defined types are
 * inserted as NULL. This function automatically escapes quoted strings for mysql. */

struct hubSpaceQuotas *hubSpaceQuotasLoad(char **row);
/* Load a hubSpaceQuotas from row fetched with select * from hubSpaceQuotas
 * from database.  Dispose of this with hubSpaceQuotasFree(). */

struct hubSpaceQuotas *hubSpaceQuotasLoadAll(char *fileName);
/* Load all hubSpaceQuotas from whitespace-separated file.
 * Dispose of this with hubSpaceQuotasFreeList(). */

struct hubSpaceQuotas *hubSpaceQuotasLoadAllByChar(char *fileName, char chopper);
/* Load all hubSpaceQuotas from chopper separated file.
 * Dispose of this with hubSpaceQuotasFreeList(). */

#define hubSpaceQuotasLoadAllByTab(a) hubSpaceQuotasLoadAllByChar(a, '\t');
/* Load all hubSpaceQuotas from tab separated file.
 * Dispose of this with hubSpaceQuotasFreeList(). */

struct hubSpaceQuotas *hubSpaceQuotasCommaIn(char **pS, struct hubSpaceQuotas *ret);
/* Create a hubSpaceQuotas out of a comma separated string. 
 * This will fill in ret if non-null, otherwise will
 * return a new hubSpaceQuotas */

void hubSpaceQuotasFree(struct hubSpaceQuotas **pEl);
/* Free a single dynamically allocated hubSpaceQuotas such as created
 * with hubSpaceQuotasLoad(). */

void hubSpaceQuotasFreeList(struct hubSpaceQuotas **pList);
/* Free a list of dynamically allocated hubSpaceQuotas's */

void hubSpaceQuotasOutput(struct hubSpaceQuotas *el, FILE *f, char sep, char lastSep);
/* Print out hubSpaceQuotas.  Separate fields with sep. Follow last field with lastSep. */

#define hubSpaceQuotasTabOut(el,f) hubSpaceQuotasOutput(el,f,'\t','\n');
/* Print out hubSpaceQuotas as a line in a tab-separated file. */

#define hubSpaceQuotasCommaOut(el,f) hubSpaceQuotasOutput(el,f,',',',');
/* Print out hubSpaceQuotas as a comma separated list including final comma. */

void hubSpaceQuotasJsonOutput(struct hubSpaceQuotas *el, FILE *f);
/* Print out hubSpaceQuotas in JSON format. */

/* -------------------------------- End autoSql Generated Code -------------------------------- */

long long quotaForUserName(char *userName);
/* Return the quota for a given userName or 0 if not found */

#endif /* HUBSPACEQUOTAS_H */

