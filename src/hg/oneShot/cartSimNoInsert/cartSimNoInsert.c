/* cartSimNoInsert - simulates N users accessing cart at regular intervals
 * where cart data is read and then written back unchanged */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "cheapcgi.h"
#include "jksql.h"
#include "portable.h"


int userCount = 10;
int randSeed = 0;
boolean readOnly = FALSE;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "cartSimNoInsert - simulates N users accessing cart at regular intervals\n"
  "   where cart data is read and then written back unchanged\n"
  "usage:\n"
  "   cartSimNoInsert host user password database milliDelay iterations\n"
  "where:\n"
  "   host is the MySQL host name - example mysqlbeta\n"
  "   user is the MySQL user name - example hgcentuser\n"
  "   password is the MySQL password\n"
  "   database is the MySQL database - example hgcentralbeta\n"
  "   milliDelay is the number of milliseconds to delay between requests\n"
  "   iterations is the number of cart read/write accesses to do\n"
  "options:\n"
  "   -userCount=N number of users simulating.  Default %d\n"
  "   -randSeed=N random number generator seed.  Defaults to pid\n"
  "   -readOnly - don't write to cart\n"
  , userCount
  );
}

int *getSomeInts(struct sqlConnection *conn, char *table, char *field, int limit)
/* Return an array of ints from field in table. */
{
int *result, i;
char query[512];
sqlSafef(query, sizeof(query), "select %s from %s limit %d", field, table, limit);
struct sqlResult *sr = sqlGetResult(conn, query);
AllocArray(result, limit);
for (i=0; i<limit; ++i)
    {
    char **row = sqlNextRow(sr);
    if (row == NULL)
        errAbort("Less than %d rows in %s", limit, table);
    result[i] = sqlSigned(row[0]);
    }
sqlFreeResult(&sr);
return result;
}

void cartSimNoInsert(char *host, char *user, char *password, char *database, char *milliDelayString,
	char *iterationString)
/* cartSimNoInsert - simulates N users accessing cart at regular intervals
 * where cart data is read and then written back unchanged */
{
int milliDelay = sqlUnsigned(milliDelayString);
int iterations = sqlUnsigned(iterationString);

/* Figure out size of tables. */
struct sqlConnection *conn = sqlConnectRemote(host, user, password, database);
char query2[1024];
sqlSafef(query2, sizeof query2, "select count(*) from userDb");
int userDbSize = sqlQuickNum(conn, query2);
sqlSafef(query2, sizeof query2, "select count(*) from sessionDb");
int sessionDbSize = sqlQuickNum(conn, query2);
int sampleSize = min(userDbSize, sessionDbSize);
int maxSampleSize = 8*1024;
sampleSize = min(sampleSize, maxSampleSize);
verbose(2, "# userDb has %d rows,  sessionDb has %d rows, sampling %d\n"
	, userDbSize, sessionDbSize, sampleSize);

/* Get sample of user id's. */
int *userIds = getSomeInts(conn, "userDb", "id", sampleSize);
int *sessionIds = getSomeInts(conn, "sessionDb", "id", sampleSize);

/* Get userCount random indexes. */
int *randomIxArray, ix;
AllocArray(randomIxArray, userCount);
verbose(2, "random user ix:\n");
for (ix=0; ix<userCount; ++ix)
    {
    randomIxArray[ix] = rand() % sampleSize;
    verbose(2, "%d ", randomIxArray[ix]);
    }
verbose(2, "\n");

sqlDisconnect(&conn);

int iteration = 0;
int querySize = 1024*1024*16;
char *query = needLargeMem(querySize);
for (;;)
    {
    for (ix = 0; ix < userCount; ++ix)
	{
	int randomIx = randomIxArray[ix];
	long startTime = clock1000();
	struct sqlConnection *conn = sqlConnectRemote(host, user, password, database);
	long connectTime = clock1000();

	sqlSafef(query, querySize, "select contents from userDb where id=%d", 
		userIds[randomIx]);
	char *userContents = sqlQuickString(conn, query);
	long userReadTime = clock1000();

	sqlSafef(query, querySize, "select contents from sessionDb where id=%d", 
		sessionIds[randomIx]);
	char *sessionContents = sqlQuickString(conn, query);
	long sessionReadTime = clock1000();

	sqlSafef(query, querySize, "update userDb set contents='%s' where id=%d",
		userContents, userIds[randomIx]);
	if (!readOnly)
	    sqlUpdate(conn, query);
	long userWriteTime = clock1000();

	sqlSafef(query, querySize, "update sessionDb set contents='%s' where id=%d",
		sessionContents, sessionIds[randomIx]);
	if (!readOnly)
	    sqlUpdate(conn, query);
	long sessionWriteTime = clock1000();

	sqlDisconnect(&conn);
	long disconnectTime = clock1000();

	printf("%ld total, %ld size, %ld connect, %ld userRead, %ld sessionRead, %ld userWrite, %ld sessionWrite\n",
		disconnectTime - startTime,
		(long) strlen(userContents) + strlen(sessionContents),
		connectTime - startTime,
		userReadTime - connectTime,
		sessionReadTime - userReadTime,
		userWriteTime - sessionReadTime,
		sessionWriteTime - userReadTime);

	freez(&userContents);
	freez(&sessionContents);

	sleep1000(milliDelay);
	if (++iteration >= iterations)
	    return;
	}
    }
}

int main(int argc, char *argv[])
/* Process command line. */
{
cgiSpoof(&argc, argv);
userCount = cgiOptionalInt("userCount", userCount);
randSeed = cgiOptionalInt("randSeed", (int)getpid());
verboseSetLevel(cgiOptionalInt("verbose", 1));
readOnly = cgiVarExists("readOnly");
srand(randSeed);
if (argc != 7)
    usage();
cartSimNoInsert(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
return 0;
}
