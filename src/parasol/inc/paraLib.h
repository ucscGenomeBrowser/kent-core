/* paraLib - some misc. routines used by multiple parasol 
 * associated programs. */

#ifndef PARALIB_H
#define PARALIB_H

extern char *cuserid(char *__s);  /* Linux should define this but... */

extern int paraHubPort;		      /* Port for hub. */
extern char *paraHubPortStr;	      /* Port for hub as a string. */
extern int paraNodePort;	      /* Port for nodes. */
extern char *paraNodePortStr;	      /* Port for nodes as a string. */

boolean sendWithSig(int fd, char *string);
/* Send a string with the signature prepended.  Warn 
 * but don't abort if there's a problem.  Return TRUE
 * on success. */

void mustSendWithSig(int fd, char *string);
/* Send a string with the signature prepended. 
 * Abort on failure. */

char *getMachine(void);
/* Return host machine name. */

char *getUser(void);
/* Get user name */

int forkOrDie(void);
/* Fork, aborting if it fails. */

struct runJobMessage
/* Parsed out runJob message as paraNode sees it. */
    {
    char *jobIdString;	   /* Unique ID - ascii number */
    char *reserved;	   /* Reserved for future expansion, always "0" */
    char *user;		   /* User associated with job. */
    char *dir;		   /* Directory to run job in. */
    char *in;              /* File for stdin. */
    char *out;		   /* File for stdout. */
    char *err;             /* File for stderr. */
    float cpus;            /* Number of CPUs used by job */
    long long ram;         /* Number of bytes of RAM used by job */
    char *command;	   /* Command line for job. */
    };

boolean parseRunJobMessage(char *line, struct runJobMessage *rjm);
/* Parse runJobMessage as paraNodes sees it. */

void fillInErrFile(char errFile[512], int jobId, char *tempDir);
/* Fill in error file name */

extern time_t now;	/* Time when started processing current message */

void findNow(void);
/* Just set now to current time. */

char* paraFormatIp(struct in6_addr *ip);
/* format a binary IP added into dotted quad format.  ip should be
 * in host byte order. Warning: static return. */

void paraDaemonize(char *progName);
/* daemonize parasol server process, closing open file descriptors and
 * starting logging based on the -logFacility and -log command line options .
 * if -debug is supplied , don't fork. */

long long paraParseRam(char *ram);
/* Parse RAM expression like 2000000, 2t, 2g, 2m, 2k
 * Returns long long number of bytes, or -1 for error
 * The value of input variable ram may be modified. */


#define NORMAL_PRIORITY 10   /* 1 is highest priority */
#define MAX_PRIORITY 10000   /* 10000 is the lowest priority */

#endif /* PARALIB_H */

