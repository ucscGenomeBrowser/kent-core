/* net.c some stuff to wrap around net communications. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

#include "common.h"
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "errAbort.h"
#include "hash.h"
#include "net.h"
#include "linefile.h"
#include "base64.h"
#include "cheapcgi.h"
#include "htmlPage.h"
#include "https.h"
#include "sqlNum.h"
#include "obscure.h"
#include "errCatch.h"

char *timeoutErrorMessage = "";

/* Brought errno in to get more useful error messages */
extern int errno;

static int netStreamSocketFromAddrInfo(struct addrinfo *address)
/* Create a socket from addrinfo structure.  
 * Complain and return something negative if can't. */
{
int sd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
if (sd < 0)
    warn("Couldn't make %s socket.", familyToString(address->ai_family));

return sd;
}

static int setSocketNonBlocking(int sd, boolean set)
/* Use socket control flags to set O_NONBLOCK if set==TRUE,
 * or clear it if set==FALSE.
 * Return -1 if there are any errors, 0 if successful. */
{
long fcntlFlags;
// Set or clear non-blocking
if ((fcntlFlags = fcntl(sd, F_GETFL, NULL)) < 0) 
    {
    warn("Error fcntl(..., F_GETFL) (%s)", strerror(errno));
    return -1;
    }
if (set)
    fcntlFlags |= O_NONBLOCK;
else
    fcntlFlags &= (~O_NONBLOCK);
if (fcntl(sd, F_SETFL, fcntlFlags) < 0) 
    {
    warn("Error fcntl(..., F_SETFL) (%s)", strerror(errno));
    return -1;
    }
return 0;
}

int setReadWriteTimeouts(int sd, int seconds)
/* Set read and write timeouts on socket sd 
 * Return -1 if there are any errors, 0 if successful. */
{
struct timeval timeout;      
timeout.tv_sec = seconds;
timeout.tv_usec = 0;

if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
    warn("setsockopt failed setting socket receive timeout\n");
    return -1;
    }

if (setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
    warn("setsockopt failed setting socket send timeout\n");
    return -1;
    }
return 0;
}


static struct timeval tvMinus(struct timeval a, struct timeval b)
/* Return the result of a - b; this handles wrapping of milliseconds. 
 * result.tv_usec will always be positive. 
 * result.tv_sec will be negative if b > a. */
{
// subtract b from a.
if (a.tv_usec < b.tv_usec)
    {
    a.tv_usec += 1000000;
    a.tv_sec--;
    }
a.tv_usec -= b.tv_usec;
a.tv_sec -= b.tv_sec;
return a;
}

int netConnectWithTimeoutOneAddr(int sd, struct addrinfo *address, long msTimeout, char *hostName, int port, struct dyString *dy)
/* Try to connect one address with timeout or return success == 0, failure == -1 */
{
// Set socket to nonblocking so we can manage our own timeout time.
if (setSocketNonBlocking(sd, TRUE) < 0)
    {
    return -1;
    }

// Trying to connect with timeout
int res;
res = connect(sd, address->ai_addr, address->ai_addrlen);
char ipStr[NI_MAXHOST];
getAddrAsString6n4((struct sockaddr_storage *)address->ai_addr, ipStr, sizeof ipStr);
if (res < 0)
    {
    if (errno == EINPROGRESS)
	{
	struct timeval startTime;
	gettimeofday(&startTime, NULL);
	struct timeval remainingTime;
	remainingTime.tv_sec = (long) (msTimeout/1000);
	remainingTime.tv_usec = (long) (((msTimeout/1000)-remainingTime.tv_sec)*1000000);
	while (1) 
	    {
	    fd_set mySet, exceptSet;
	    FD_ZERO(&mySet);
	    FD_SET(sd, &mySet);
            exceptSet = mySet;
	    // use tempTime (instead of using remainingTime directly) because on some platforms select() may modify the time val.
	    struct timeval tempTime = remainingTime;
	    res = select(sd+1, NULL, &mySet, &exceptSet, &tempTime);  
	    if (res < 0) 
		{
		if (errno == EINTR)  // Ignore the interrupt 
		    {
		    // Subtract the elapsed time from remaining time since some platforms need this.
		    struct timeval newTime;
		    gettimeofday(&newTime, NULL);
		    struct timeval elapsedTime = tvMinus(newTime, startTime);
		    remainingTime = tvMinus(remainingTime, elapsedTime);
		    if (remainingTime.tv_sec < 0)  // means our timeout has more than expired
			{
			remainingTime.tv_sec = 0;
			remainingTime.tv_usec = 0;
			}
		    startTime = newTime;
		    }
		else
		    {
		    dyStringPrintf(dy, "Error in select() during TCP non-blocking connect %d - %s\n", errno, strerror(errno));
		    return -1;
		    }
		}
	    else if (res > 0)
		{
		// Socket selected for write when it is ready
		int valOpt;
		socklen_t lon;
		// But check the socket for any errors
		lon = sizeof(valOpt);
		if (getsockopt(sd, SOL_SOCKET, SO_ERROR, (void*) (&valOpt), &lon) < 0)
		    {
		    warn("Error in getsockopt() %d - %s", errno, strerror(errno));
		    return -1;
		    }
		// Check the value returned...
		if (valOpt)
		    {
		    dyStringPrintf(dy, "Error in TCP non-blocking connect() %d - %s. Host %s IP %s port %d.\n", valOpt, strerror(valOpt), hostName, ipStr, port);
		    return -1;
		    }
		break;  // OK
		}
	    else
		{
		dyStringPrintf(dy, "TCP non-blocking connect() to %s IP %s timed-out in select() after %ld milliseconds. %s", hostName, ipStr, msTimeout, timeoutErrorMessage);
		return -1;
		}
	    }
	}
    else
	{
	dyStringPrintf(dy, "TCP non-blocking connect() error %d - %s", errno, strerror(errno));
	return -1;
	}
    }
return 0; // OK
}


int netConnectWithTimeout(char *hostName, int port, long msTimeout)
/* In order to avoid a very long default timeout (several minutes) for hosts that will
* not answer the port, we are forced to connect non-blocking.
* After the connection has been established, we return to blocking mode.
* Also closes sd if error. */
{
int sd;
struct addrinfo *addressList=NULL, *address;
char portStr[8];
safef(portStr, sizeof portStr, "%d", port);

if (hostName == NULL)
    {
    warn("NULL hostName in netConnect");
    return -1;
    }
if (!internetGetAddrInfo6n4(hostName, portStr, &addressList))
    return -1;

struct dyString *errMsg = dyStringNew(256);
for (address = addressList; address; address = address->ai_next)
    {
    if ((sd = netStreamSocketFromAddrInfo(address)) < 0)
	continue;

    if (netConnectWithTimeoutOneAddr(sd, address, msTimeout, hostName, port, errMsg) == 0)
	break;

    close(sd);
    }
boolean connected = (address != NULL); // one of the addresses connected successfully
freeaddrinfo(addressList);

if (!connected) 
    {
    if (!sameString(errMsg->string, ""))
	{
	warn("%s", errMsg->string);
	}
    }
dyStringFree(&errMsg);
if (!connected)
    return -1;

// Set to blocking mode again
if (setSocketNonBlocking(sd, FALSE) < 0)
    {
    close(sd);
    return -1;
    }

if (setReadWriteTimeouts(sd, DEFAULTREADWRITETTIMEOUTSEC) < 0)
    {
    close(sd);
    return -1;
    }

return sd;

}


int netConnect(char *hostName, int port)
/* Start connection with a server. */
{
return netConnectWithTimeout(hostName, port, DEFAULTCONNECTTIMEOUTMSEC); // 10 seconds connect timeout
}

int netMustConnect(char *hostName, int port)
/* Start connection with server or die. */
{
int sd = netConnect(hostName, port);
if (sd < 0)
   noWarnAbort();
return sd;
}

int netMustConnectTo(char *hostName, char *portName)
/* Start connection with a server and a port that needs to be converted to integer */
{
if (!isdigit(portName[0]))
    errAbort("netConnectTo: ports must be numerical, not %s", portName);
return netMustConnect(hostName, atoi(portName));
}

int netAcceptingSocket4Only(int port, int queueSize)
/* Create an IPV4 socket that can accept connections from 
 * only IPV4 clients on the current machine. Useful for systems with ipv6 disabled. */
{
struct sockaddr_in serverAddr;
int sd;

netBlockBrokenPipes();

// ipv4 listening socket accepts ipv4 connections.
if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
    errAbort("socket() failed");
    }

// Allow local address reuse when server is restarted without waiting.
int on = -1;
if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on,sizeof(on)) < 0)
    {
    errAbort("setsockopt(SO_REUSEADDR) failed");
    }

ZeroVar(&serverAddr);
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(port);
serverAddr.sin_addr.s_addr = INADDR_ANY;

if (bind(sd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
    errAbort("Couldn't bind socket to %d: %s", port, strerror(errno));
    }

if (listen(sd, queueSize) < 0)
    {
    errAbort("listen() failed");
    }

return sd;
}


int netAcceptingSocket6n4(int port, int queueSize)
/* Create an IPV6 socket that can accept connections from 
 * both IPV4 and IPV6 clients on the current machine. */
{
struct sockaddr_in6 serverAddr;
int sd;

netBlockBrokenPipes();

// Hybrid dual stack ipv6 listening socket accepts ipv6 and ipv4 mapped connections.
if ((sd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
    {
    errAbort("socket() failed");
    }

// Allow local address reuse when server is restarted without waiting.
int on = -1;
if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on,sizeof(on)) < 0)
    {
    errAbort("setsockopt(SO_REUSEADDR) failed");
    }

// Explicitly turn off IPV6_V6ONLY which is needed on non-Linux platforms like NetBSD and Darwin.
// This means we allow ipv4 socket connections that can have ipv4-mapped ipv6 IPs.
int off = 0;
if (setsockopt(sd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&off, sizeof(off)) < 0)
    {
    errAbort("setsockopt IPV6_V6ONLY off failed.");
    }

ZeroVar(&serverAddr);
serverAddr.sin6_family = AF_INET6;
serverAddr.sin6_port   = htons(port);
serverAddr.sin6_addr   = in6addr_any;
if (bind(sd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
    errAbort("Couldn't bind socket to %d: %s", port, strerror(errno));
    }

if (listen(sd, queueSize) < 0)
    {
    errAbort("listen() failed");
    }

return sd;
}


int netAcceptingSocket(int port, int queueSize)
/* Create an IPV6 socket that can accept connections from 
 * both IPV4 and IPV6 clients on the current machine.
 * OR Failover to making IPv4 Only socket if IPv6 is disabled. */
{
int sd = -1;
struct errCatch *errCatch = errCatchNew();
if (errCatchStart(errCatch))
    {
    sd = netAcceptingSocket6n4(port, queueSize);
    }
errCatchEnd(errCatch);
if (errCatch->gotError)
    {
    // if ipv6 is disabled, fall back to trying ipv4 only listen socket
    warn("%s", errCatch->message->string);
    warn("Retrying listen socket using ipv4 only.");
    sd = netAcceptingSocket4Only(port, queueSize);
    }
errCatchFree(&errCatch);
if (sd == -1)
  errAbort("unable to open listening socket");
return sd;
}

int netAccept(int sd)
/* Accept incoming connection from socket descriptor. */
{
socklen_t fromLen;
return accept(sd, NULL, &fromLen);
}

int netAcceptFrom(int acceptor, struct cidr *subnet)
/* Wait for incoming connection from socket descriptor
 * from IP address in subnet.  Subnet is something
 * returned from internetParseSubnetCidr. 
 * Subnet may be NULL. */
{
for (;;)
    {
    int sd = accept(acceptor, NULL, NULL);
    if (sd >= 0)
	{
	if (subnet == NULL)
	    return sd;
	else
	    {
	    socklen_t len;
	    struct sockaddr_storage addr;
	    char ipStr[INET6_ADDRSTRLEN];

	    len = sizeof addr;
	    getpeername(sd, (struct sockaddr*)&addr, &len);

            getAddrAsString6n4(&addr, ipStr, sizeof ipStr);

	    if (!strchr(ipStr, ':'))   // convert ipv4 to ipv6-mapped
		{
		// prepend "::ffff:" to ipStr
	        char temp[INET6_ADDRSTRLEN];
		safef(temp, sizeof temp, "::ffff:%s", ipStr);
		safecpy(ipStr, sizeof ipStr, temp);
		}
	    // convert back to ipv6 address.
	    struct sockaddr_in6 clientAddr;
	    internetIpStringToIp6(ipStr, &clientAddr.sin6_addr);

            // see if it is in the allowed subnet
	    if (internetIpInSubnetCidr(&clientAddr.sin6_addr, subnet))
		{
		return sd;
		}
	    else
		{
		close(sd);
		}
	    }
	}
    }
}

FILE *netFileFromSocket(int socket)
/* Wrap a FILE around socket.  This should be fclose'd
 * and separately the socket close'd. */
{
FILE *f;
if ((socket = dup(socket)) < 0)
   errnoAbort("Couldn't dupe socket in netFileFromSocket");
f = fdopen(socket, "r+");
if (f == NULL)
   errnoAbort("Couldn't fdopen socket in netFileFromSocket");
return f;
}

static boolean plumberInstalled = FALSE;

void netBlockBrokenPipes()
/* Make it so a broken pipe doesn't kill us. */
{
if (!plumberInstalled)
    {
    signal(SIGPIPE, SIG_IGN);       /* Block broken pipe signals. */
    plumberInstalled = TRUE;
    }
}

ssize_t netReadAll(int sd, void *vBuf, ssize_t size)
/* Read given number of bytes into buffer.
 * Don't give up on first read! */
{
char *buf = vBuf;
ssize_t totalRead = 0;
int oneRead;

if (!plumberInstalled)
    netBlockBrokenPipes();
while (totalRead < size)
    {
    oneRead = read(sd, buf + totalRead, size - totalRead);
    if (oneRead < 0)
	return oneRead;
    if (oneRead == 0)
        break;
    totalRead += oneRead;
    }
return totalRead;
}

ssize_t netMustReadAll(int sd, void *vBuf, ssize_t size)
/* Read given number of bytes into buffer or die.
 * Don't give up if first read is short! */
{
ssize_t ret = netReadAll(sd, vBuf, size);
if (ret < 0)
    errnoAbort("Couldn't finish netReadAll");
return ret;
}


static void parseByteRange(char *url, ssize_t *rangeStart, ssize_t *rangeEnd, boolean terminateAtByteRange)
/* parse the byte range information from url */
{
char *x;
/* default to no byte range specified */
*rangeStart = -1;
*rangeEnd = -1;
x = strrchr(url, ';');
if (x)
    {
    if (startsWith(";byterange=", x))
	{
	char *y=strchr(x, '=');
	++y;
	char *z=strchr(y, '-');
	if (z)
	    {
	    ++z;
	    if (terminateAtByteRange)
		*x = 0;
	    *rangeStart = atoll(y); 
	    if (z[0] != '\0')
		*rangeEnd = atoll(z);
	    }    
	}
    }

}

void netHandleHostForIpv6(struct netParsedUrl *npu, struct dyString *dy)
/* if needed, add brackets around the host name, 
 * for raw ipv6 address so it is not confused with colon port that may follow. */
{
boolean hostIsIpv6 = FALSE;
if (strchr(npu->host, ':')) // Is the host really an IPV6 address?
    hostIsIpv6 = TRUE;
if (hostIsIpv6) // surround ipv6 host with brakets []
    dyStringAppendC(dy, '[');
dyStringAppend(dy, npu->host);
if (hostIsIpv6) // surround ipv6 host with brakets []
    dyStringAppendC(dy, ']');
}

void netParseUrl(char *url, struct netParsedUrl *parsed)
/* Parse a URL into components.   A full URL is made up as so:
 *   http://user:password@hostName:port/file;byterange=0-499
 * User and password may be cgi-encoded.
 * This is set up so that the http:// and the port are optional. 
 */
{
char *s, *t, *u, *v, *w, *br, *bl;
char buf[MAXURLSIZE];

/* Make local copy of URL. */
if (strlen(url) >= sizeof(buf))
    errAbort("Url too long: '%s'", url);
strcpy(buf, url);
url = buf;

/* Find out protocol - default to http. */
s = trimSpaces(url);
s = stringIn("://", url);
if (s == NULL)
    {
    strcpy(parsed->protocol, "http");
    s = url;
    }
else
    {
    *s = 0;
    tolowers(url);
    safecpy(parsed->protocol, sizeof(parsed->protocol), url);
    s += 3;
    }

/* Split off file part. */
parsed->byteRangeStart = -1;  /* default to no byte range specified */
parsed->byteRangeEnd = -1;
u = strchr(s, '/');
if (u == NULL)
    strcpy(parsed->file, "/");
else
    {

    parseByteRange(u, &parsed->byteRangeStart, &parsed->byteRangeEnd, TRUE);

    if (sameWord(parsed->protocol,"http") ||
        sameWord(parsed->protocol,"https"))
	{
	// http servers expect the URL request to be URL-encoded already.
	/* need to encode spaces, but not ! other characters */
	char *t=replaceChars(u," ","%20");
	safecpy(parsed->file, sizeof(parsed->file), t);
	freeMem(t);
	}

    *u = 0; // terminate the host:port string

    if (sameWord(parsed->protocol,"ftp"))
	{
	++u; // that first slash is not considered part of the ftp path 
	// decode now because the FTP server does NOT expect URL-encoding.
	cgiDecodeFull(u,parsed->file,strlen(u));  // decodes %FF but not +
	}

    }

/* Split off user part */
v = strchr(s, '@');
if (v == NULL)
    {
    if (sameWord(parsed->protocol,"http") ||
        sameWord(parsed->protocol,"https"))
	{
	strcpy(parsed->user, "");
	strcpy(parsed->password, "");
	}
    if (sameWord(parsed->protocol,"ftp"))
	{
	strcpy(parsed->user, "anonymous");
	strcpy(parsed->password, "x@genome.ucsc.edu");
	}
    }
else
    {
    *v = 0;
    /* split off password part */
    w = strchr(s, ':');
    if (w == NULL)
	{
	safecpy(parsed->user, sizeof(parsed->user), s);
	strcpy(parsed->password, "");
	}
    else
	{
	*w = 0;
	safecpy(parsed->user, sizeof(parsed->user), s);
	safecpy(parsed->password, sizeof(parsed->password), w+1);
	}
    
    cgiDecode(parsed->user,parsed->user,strlen(parsed->user));
    cgiDecode(parsed->password,parsed->password,strlen(parsed->password));
    s = v+1;
    }


// Whenever IPv6 address : port are provided,
// the address MUST be surrounded by square brackets like [IPv6-address]:port
// because without the square brackets, we cannot tell if the trailing bit
// is end end of an IPv6 address, or port number.

int blCount = countChars(s, '[');
int brCount = countChars(s, ']');

// double-check any stray brackets
if ((brCount != blCount) || (brCount > 1))
    errAbort("badly formed url, stray square brackets in IPv6 address");

/* Save port if it's there.  If not default to 80. */
bl = strchr(s, '['); // IPV6 address in url surrounded by brackets []
br = strrchr(s, ']'); // IPV6 address in url surrounded by brackets []

if (!br != !bl)  // logical XOR
    errAbort("badly formed url, unbalanced square brackets around IPv6 address.");

if (!br && isIpv6Address(s))  // host looks like IPv6 address but no brackets.
    errAbort("badly formed url, should be protocol://[IPv6-address]:port/. Put square brackets around literal IPv6 address.");

// trim off the brackets around the ipv6 host name
if (br)
    {
    // expecting *s == [
    if (*s != '[')
	errAbort("badly formed url %s, expected [ at start of ipv6 address", s);
    ++s;    // skip [
    *br = 0; // erase ]
    t = br+1;
    char c = *t;
    if (c == 0)
	t = NULL;
    else if (c != ':')
	errAbort("badly formed url %s, stray characters after ] at end of ipv6 address", s);
    }
else
    {
    t = strrchr(s, ':');
    }

if (br && !isIpv6Address(s))  // host has brackets but does not look like IPv6 address.
    errAbort("badly formed url, brackets found, but not valid literal IPv6 address.");

if (t) // the port was explicitly provided
    {
    *t++ = 0;
    if (!isdigit(t[0]))
	errAbort("Non-numeric port name %s", t);
    safecpy(parsed->port, sizeof(parsed->port), t);
    }
else // get default port for each protocol
    {  
    if (sameWord(parsed->protocol,"http"))
	strcpy(parsed->port, "80");
    if (sameWord(parsed->protocol,"https"))
	strcpy(parsed->port, "443");
    if (sameWord(parsed->protocol,"ftp"))
	strcpy(parsed->port, "21");
    }

/* What's left is the host. */
safecpy(parsed->host, sizeof(parsed->host), s);
}

char *urlFromNetParsedUrl(struct netParsedUrl *npu)
/* Build URL from netParsedUrl structure */
{
struct dyString *dy = dyStringNew(512);

dyStringAppend(dy, npu->protocol);
dyStringAppend(dy, "://");
if (npu->user[0] != 0)
    {
    char *encUser = cgiEncode(npu->user);
    dyStringAppend(dy, encUser);
    freeMem(encUser);
    if (npu->password[0] != 0)
	{
	dyStringAppend(dy, ":");
	char *encPassword = cgiEncode(npu->password);
	dyStringAppend(dy, encPassword);
	freeMem(encPassword);
	}
    dyStringAppend(dy, "@");
    }

netHandleHostForIpv6(npu, dy);

/* do not include port if it is the default */
if (!(
 (sameString(npu->protocol, "ftp"  ) && sameString("21", npu->port)) ||
 (sameString(npu->protocol, "http" ) && sameString("80", npu->port)) ||
 (sameString(npu->protocol, "https") && sameString("443",npu->port))
    ))
    {
    dyStringAppend(dy, ":");
    dyStringAppend(dy, npu->port);
    }
dyStringAppend(dy, npu->file);
if (npu->byteRangeStart != -1)
    {
    dyStringPrintf(dy, ";byterange=%lld-", (long long)npu->byteRangeStart);
    if (npu->byteRangeEnd != -1)
	dyStringPrintf(dy, "%lld", (long long)npu->byteRangeEnd);
    }

/* Clean up and return handle. */
return dyStringCannibalize(&dy);
}

int netWaitForData(int sd, int microseconds)
/* Wait for descriptor to have some data to read, up to given number of
 * microseconds.  Returns amount of data there or zero if timed out. */
{
struct timeval tv;
fd_set set;
int readyCount;

for (;;)
    {
    if (microseconds >= 1000000)
	{
	tv.tv_sec = microseconds/1000000;
	tv.tv_usec = microseconds%1000000;
	}
    else
	{
	tv.tv_sec = 0;
	tv.tv_usec = microseconds;
	}
    FD_ZERO(&set);
    FD_SET(sd, &set);
    readyCount = select(sd+1, &set, NULL, NULL, &tv);
    if (readyCount < 0) 
	{
	if (errno == EINTR)	/* Select interrupted, not timed out. */
	    continue;
    	else 
    	    warn("select failure %s", strerror(errno));
    	}
    else
	{
    	return readyCount;	/* Zero readyCount indicates time out */
	}
    }
}

static boolean readReadyWait(int sd, int microseconds)
/* Wait for descriptor to have some data to read, up to given number of
 * number of microseconds.  Returns true if there is data, false if timed out. */
{
int readyCount = netWaitForData(sd, microseconds);
return readyCount > 0;	/* Zero readyCount indicates time out */
}

static void sendFtpCommandOnly(int sd, char *cmd)
/* send command to ftp server */
{   
mustWriteFd(sd, cmd, strlen(cmd));
}

#define NET_FTP_TIMEOUT 1000000

static boolean receiveFtpReply(int sd, char *cmd, struct dyString **retReply, int *retCode)
/* send command to ftp server and check resulting reply code, 
 * warn and return FALSE if not desired reply.  If retReply is non-NULL, store reply text there. */
{
char *startLastLine = NULL;
struct dyString *rs = dyStringNew(4*1024);
while (1)
    {
    int readSize = 0;
    while (1)
	{
	char buf[4*1024];
	if (!readReadyWait(sd, NET_FTP_TIMEOUT))
	    {
	    warn("ftp server response timed out > %d microsec", NET_FTP_TIMEOUT);
	    return FALSE;
	    }
	if ((readSize = read(sd, buf, sizeof(buf))) == 0)
	    break;

	dyStringAppendN(rs, buf, readSize);
	if (endsWith(rs->string,"\n"))
	    break;
	}
	
    /* find the start of the last line in the buffer */
    startLastLine = rs->string+strlen(rs->string)-1;
    if (startLastLine >= rs->string)
	if (*startLastLine == '\n') 
	    --startLastLine;
    while ((startLastLine >= rs->string) && (*startLastLine != '\n'))
	--startLastLine;
    ++startLastLine;
	
    if (strlen(startLastLine)>4)
      if (
	isdigit(startLastLine[0]) &&
	isdigit(startLastLine[1]) &&
	isdigit(startLastLine[2]) &&
	startLastLine[3]==' ')
	break;
	
    if (readSize == 0)
	break;  // EOF
    /* must be some text info we can't use, ignore it till we get status code */
    }

int reply = atoi(startLastLine);
if (retCode)
    *retCode = reply;
if ((reply < 200) || (reply > 399))
    {
    if (!(sameString(cmd,"PASV\r\n") && reply==501))
	warn("ftp server error on cmd=[%s] response=[%s]",cmd,rs->string);
    return FALSE;
    }
    
if (retReply)
    *retReply = rs;
else
    dyStringFree(&rs);
return TRUE;
}

static boolean sendFtpCommand(int sd, char *cmd, struct dyString **retReply, int *retCode)
/* send command to ftp server and check resulting reply code, 
 * warn and return FALSE if not desired reply.  If retReply is non-NULL, store reply text there. */
{   
sendFtpCommandOnly(sd, cmd);
return receiveFtpReply(sd, cmd, retReply, retCode);
}

static int parsePasvPort(char *rs)
/* parse PASV reply to get the port and return it */
{
char *words[7];
int wordCount;
char *rsStart = strchr(rs,'(');
char *rsEnd = strchr(rs,')');
int result = 0;
rsStart++;
*rsEnd=0;
wordCount = chopString(rsStart, ",", words, ArraySize(words));
if (wordCount != 6)
    errAbort("PASV reply does not parse correctly");
result = atoi(words[4])*256+atoi(words[5]);    
return result;
}    

static int parseEpsvPort(char *rs)
/* parse EPSV reply to get the port and return it */
{
char *words[6];
int wordCount;
char *rsStart = strchr(rs,'(');
char *rsEnd = strchr(rs,')');
int result = 0;
rsStart++;
*rsEnd=0;
wordCount = chopString(rsStart, "|", words, ArraySize(words));
if (wordCount != 1)
    errAbort("EPSV reply does not parse correctly");
result = atoi(words[0]); // multiple separators treated as one.   
return result;
}    


static long long parseFtpSIZE(char *rs)
/* parse reply to SIZE and return it */
{
char *words[3];
int wordCount;
char *rsStart = rs;
long long result = 0;
wordCount = chopString(rsStart, " ", words, ArraySize(words));
if (wordCount != 2)
    errAbort("SIZE reply does not parse correctly");
result = atoll(words[1]);    
return result;
}    


static time_t parseFtpMDTM(char *rs)
/* parse reply to MDTM and return it
 * 200 YYYYMMDDhhmmss */
{
char *words[3];
int wordCount = chopString(rs, " ", words, ArraySize(words));
if (wordCount != 2)
    errAbort("MDTM reply should have 2 words but has %d", wordCount);
#define EXPECTED_MDTM_FORMAT "YYYYmmddHHMMSS"
if (strlen(words[1]) < strlen(EXPECTED_MDTM_FORMAT))
    errAbort("MDTM reply word [%s] shorter than " EXPECTED_MDTM_FORMAT, words[1]);
// man strptime ->
// "There should be whitespace or other alphanumeric characters between any two field descriptors."
// There are no separators in the MDTM timestamp, so make a spread-out version for strptime:
char spread[] = "YYYY mm dd HH MM SS";
char *from = words[1];
char *to = spread;
*to++ = *from++;
*to++ = *from++;
*to++ = *from++;
*to++ = *from++;
*to++ = '-';
*to++ = *from++;
*to++ = *from++;
*to++ = '-';
*to++ = *from++;
*to++ = *from++;
*to++ = ' ';
*to++ = *from++;
*to++ = *from++;
*to++ = ':';
*to++ = *from++;
*to++ = *from++;
*to++ = ':';
*to++ = *from++;
*to++ = *from++;
*to++ = 0;
struct tm tm;
if (strptime(spread, "%Y-%m-%d %H:%M:%S", &tm) == NULL)
    errAbort("unable to parse MDTM string [%s]", spread);
// Not set by strptime(); tells mktime() to determine whether daylight saving time is in effect:
tm.tm_isdst = -1;
time_t t = mktime(&tm);
if (t == -1)
    errAbort("mktime failed while parsing strptime'd MDTM string [%s]", words[1]);
return t;
}    


static int openFtpControlSocket(char *host, int port, char *user, char *password)
/* Open a socket to host,port; authenticate anonymous ftp; set type to I; 
 * return socket desc or -1 if there was an error. */
{
int sd = netConnect(host, port);
if (sd < 0)
    return -1;

/* First read the welcome msg */
if (readReadyWait(sd, NET_FTP_TIMEOUT))
    sendFtpCommand(sd, "", NULL, NULL);

char cmd[256];
int retCode = 0;
safef(cmd,sizeof(cmd),"USER %s\r\n", user);
if (!sendFtpCommand(sd, cmd, NULL, &retCode))
    {
    close(sd);
    return -1;
    }

if (retCode == 331)
    {
    safef(cmd,sizeof(cmd),"PASS %s\r\n", password);
    if (!sendFtpCommand(sd, cmd, NULL, NULL))
	{
	close(sd);
	return -1;
	}
    }

if (!sendFtpCommand(sd, "TYPE I\r\n", NULL, NULL))
    {
    close(sd);
    return -1;
    }
/* 200 Type set to I */
/* (send the data as binary, so can support compressed files) */
return sd;
}

boolean netGetFtpInfo(char *url, long long *retSize, time_t *retTime)
/* Return date in UTC and size of ftp url file */
{
/* Parse the URL and connect. */
struct netParsedUrl npu;
netParseUrl(url, &npu);
if (!sameString(npu.protocol, "ftp"))
    errAbort("netGetFtpInfo: url (%s) is not for ftp.", url);

if (sameString(npu.file,"/"))
    {
    *retSize = 0;
    *retTime = time(NULL);
    return TRUE;
    }

int sd = openFtpControlSocket(npu.host, atoi(npu.port), npu.user, npu.password);
if (sd < 0)
    return FALSE;
char cmd[256];
safef(cmd,sizeof(cmd),"SIZE %s\r\n", npu.file);
struct dyString *rs = NULL;
if (!sendFtpCommand(sd, cmd, &rs, NULL))
    {
    close(sd);
    return FALSE;
    }
*retSize = parseFtpSIZE(rs->string);
/* 200 12345 */
dyStringFree(&rs);

safef(cmd,sizeof(cmd),"MDTM %s\r\n", npu.file);
if (!sendFtpCommand(sd, cmd, &rs, NULL))
    {
    close(sd);
    return FALSE;
    }
*retTime = parseFtpMDTM(rs->string);
/* 200 YYYYMMDDhhmmss */
dyStringFree(&rs);
close(sd);   
return TRUE;
}

struct netConnectFtpParams
/* params to pass to thread */
{
pthread_t thread;
int pipefd[2];
int sd;
int sdata; 
struct netParsedUrl npu;
};

static void *sendFtpDataToPipeThread(void *threadParams)
/* This is to be executed by the child process after the fork in netGetOpenFtpSockets.
 * It keeps the ftp control socket alive while reading from the ftp data socket
 * and writing to the pipe to the parent process, which closes the ftp sockets
 * and reads from the pipe. */
{

struct netConnectFtpParams *params = threadParams;

pthread_detach(params->thread);  // this thread will never join back with it's progenitor

char buf[32768];
int rd = 0;
long long dataPos = 0; 
if (params->npu.byteRangeStart != -1)
    dataPos = params->npu.byteRangeStart;
while((rd = read(params->sdata, buf, 32768)) > 0) 
    {
    if (params->npu.byteRangeEnd != -1 && (dataPos + rd) > params->npu.byteRangeEnd)
	rd = params->npu.byteRangeEnd - dataPos + 1;
    int wt = write(params->pipefd[1], buf, rd);
    if (wt == -1 && params->npu.byteRangeEnd != -1)
	{
	// errAbort in child process is messy; let reader complain if
	// trouble.  If byterange was open-ended, we will hit this point
	// when the parent stops reading and closes the pipe.
	errnoWarn("error writing ftp data to pipe");
	break;
	}
    dataPos += rd;
    if (params->npu.byteRangeEnd != -1 && dataPos >= params->npu.byteRangeEnd)
	break;	    
    }
if (rd == -1)
    // Again, avoid abort in child process.
    errnoWarn("error reading ftp socket");
close(params->pipefd[1]);  /* we are done with it */
close(params->sd);
close(params->sdata);
return NULL;
}

static int netGetOpenFtpSockets(char *url, int *retCtrlSd)
/* Return a socket descriptor for url data (url can end in ";byterange:start-end)",
 * or -1 if error.
 * If retCtrlSd is non-null, keep the control socket alive and set *retCtrlSd.
 * Otherwise, create a pipe and fork to keep control socket alive in the child 
 * process until we are done fetching data. */
{
char cmd[256];

/* Parse the URL and connect. */
struct netParsedUrl npu;
struct netParsedUrl pxy;
netParseUrl(url, &npu);
if (!sameString(npu.protocol, "ftp"))
    errAbort("netGetOpenFtpSockets: url (%s) is not for ftp.", url);

boolean noProxy = checkNoProxy(npu.host);
char *proxyUrl = getenv("ftp_proxy");
if (noProxy)
    proxyUrl = NULL;

int sd = -1;
if (proxyUrl)
    {
    netParseUrl(proxyUrl, &pxy);
    if (!sameString(pxy.protocol, "ftp"))
        errAbort("Unknown proxy protocol %s in %s. Should be ftp.", pxy.protocol, proxyUrl);
    char proxyUser[4096];
    safef(proxyUser, sizeof proxyUser, "%s@%s:%s", npu.user, npu.host, npu.port);
    sd = openFtpControlSocket(pxy.host, atoi(pxy.port), proxyUser, npu.password);
    char *logProxy = getenv("log_proxy");
    if (sameOk(logProxy,"on"))
	verbose(1, "%s as %s via proxy %s\n", url, proxyUser, proxyUrl);
    }
else
    {
    sd = openFtpControlSocket(npu.host, atoi(npu.port), npu.user, npu.password);
    }

if (sd == -1)
    return -1;

int retCode = 0;
struct dyString *rs = NULL;
sendFtpCommand(sd, "PASV\r\n", &rs, &retCode);
/* 227 Entering Passive Mode (128,231,210,81,222,250) */  
boolean isIpv6 = FALSE;
if (retCode == 501)
    {

    if (!sendFtpCommand(sd, "EPSV\r\n", &rs, NULL))
    /* 229 Entering Extended Passive Mode (|||44022|) */
	{
	close(sd);
	return -1;
	}
    isIpv6 = TRUE;

    }
else if (retCode != 227)
    {
    close(sd);
    return -1;
    }

if (npu.byteRangeStart != -1)
    {
    safef(cmd,sizeof(cmd),"REST %lld\r\n", (long long) npu.byteRangeStart);
    if (!sendFtpCommand(sd, cmd, NULL, NULL))
	{
	dyStringFree(&rs);
	close(sd);
	return -1;
	}
    }

/* RETR for files, LIST for directories ending in / */
safef(cmd,sizeof(cmd),"%s %s\r\n",((npu.file[strlen(npu.file)-1]) == '/') ? "LIST" : "RETR", npu.file);
sendFtpCommandOnly(sd, cmd);

int sdata = -1;
sdata = netConnect(proxyUrl ? pxy.host : npu.host, isIpv6 ? parseEpsvPort(rs->string) : parsePasvPort(rs->string));
dyStringFree(&rs);
if (sdata < 0)
    {
    close(sd);
    return -1;
    }

int secondsWaited = 0;
while (TRUE)
    {
    if (secondsWaited >= 10)
	{
	warn("ftp server error on cmd=[%s] timed-out waiting for data or error", cmd);
	close(sd);
	close(sdata);
	return -1;
	}
    if (readReadyWait(sdata, NET_FTP_TIMEOUT))
	break;   // we have some data
    if (readReadyWait(sd, 0)) /* wait in microsec */
	{
	// this can see an error like bad filename
	if (!receiveFtpReply(sd, cmd, NULL, NULL))
	    {
	    close(sd);
	    close(sdata);
	    return -1;
	    }
	}
    ++secondsWaited;
    }

if (retCtrlSd != NULL)
    {
    *retCtrlSd = sd;
    return sdata;
    }
else
    {
    /* Because some FTP servers will kill the data connection
     * as soon as the control connection closes,
     * we have to develop a workaround using a partner thread. */
    fflush(stdin);
    fflush(stdout);
    fflush(stderr);

    struct netConnectFtpParams *params;
    AllocVar(params);
    params->sd = sd;
    params->sdata = sdata;
    params->npu = npu;
    /* make a pipe (fds go in pipefd[0] and pipefd[1])  */
    if (pipe(params->pipefd) != 0)
	errAbort("netGetOpenFtpSockets: failed to create pipe: %s", strerror(errno));
    int rc;
    rc = pthread_create(&params->thread, NULL, sendFtpDataToPipeThread, (void *)params);
    if (rc)
	{
	errAbort("Unexpected error %d from pthread_create(): %s",rc,strerror(rc));
	}

    return params->pipefd[0];
    }
}


int connectNpu(struct netParsedUrl npu, char *url, boolean noProxy, char *httpProtocol)
/* Connect using NetParsedUrl. */
{
int sd = -1;
if (sameString(npu.protocol, "http"))
    {
    sd = netConnect(npu.host, atoi(npu.port));
    }
else if (sameString(npu.protocol, "https"))
    {
    sd = netConnectHttps(npu.host, atoi(npu.port), noProxy, httpProtocol);
    }
else
    {
    errAbort("netHttpConnect: url (%s) is not for http.", url);
    return -1;  /* never gets here, fixes compiler complaint */
    }
return sd;
}

void setAuthorization(struct netParsedUrl npu, char *authHeader, struct dyString *dy)
/* Set the specified authorization header with BASIC auth base64-encoded user and password */
{
if (!sameString(npu.user,""))
    {
    char up[256];
    char *b64up = NULL;
    safef(up, sizeof(up), "%s:%s", npu.user, npu.password);
    b64up = base64Encode(up, strlen(up));
    dyStringPrintf(dy, "%s: Basic %s\r\n", authHeader, b64up);
    freez(&b64up);
    }
}

boolean checkNoProxy(char *host)
/* See if host endsWith element on no_proxy list. Elements are comma-separated. */
{
char *list = cloneString(getenv("no_proxy"));
if (!list)
    return FALSE;
replaceChar(list, ',', ' ');
char *word;
while((word=nextWord(&list)))
    {
    if (endsWith(host, word))
	return TRUE;
    }
return FALSE;
}

int netHttpConnect(char *url, char *method, char *protocol, char *agent, char *optionalHeader)
/* Parse URL, connect to associated server on port, and send most of
 * the request to the server.  If specified in the url send user name
 * and password too.  Typically the "method" will be "GET" or "POST"
 * and the agent will be the name of your program or
 * library. optionalHeader may be NULL or contain additional header
 * lines such as cookie info. 
 * Proxy support via hg.conf httpProxy or env var http_proxy
 * Cert verification control via hg.conf httpsCertCheck or env var https_cert_check
 * Cert verify domains exception white-list via hg.conf httpsCertCheckDomainExceptions or env var https_cert_check_domain_exceptions
 * Return data socket, or -1 if error.*/
{
struct netParsedUrl npu;
struct netParsedUrl pxy;
struct dyString *dy = dyStringNew(512);
int sd = -1;
/* Parse the URL and connect. */
netParseUrl(url, &npu);



boolean noProxy = checkNoProxy(npu.host);
char *proxyUrl = getenv("http_proxy");
if (sameString(npu.protocol, "https"))
    proxyUrl = NULL;
if (noProxy)
    proxyUrl = NULL;
if (proxyUrl)
    {
    netParseUrl(proxyUrl, &pxy);
    if (!sameString(pxy.protocol, "http"))
	errAbort("Unknown proxy protocol %s in %s.", pxy.protocol, proxyUrl);
    sd = connectNpu(pxy, url, noProxy, protocol);
    char *logProxy = getenv("log_proxy");
    if (sameOk(logProxy,"on"))
	verbose(1, "%s via proxy %s\n", url, proxyUrl);
    }
else
    {
    sd = connectNpu(npu, url, noProxy, protocol);
    }
if (sd < 0)
    return -1;

/* Ask remote server for a file. */
char *urlForProxy = NULL;
if (proxyUrl)
    {
    /* trim off the byterange part at the end of url because proxy does not understand it. */
    urlForProxy = cloneString(url);
    char *x = strrchr(urlForProxy, ';');
    if (x && startsWith(";byterange=", x))
	*x = 0;
    }
dyStringPrintf(dy, "%s %s %s\r\n", method, proxyUrl ? urlForProxy : npu.file, protocol);
freeMem(urlForProxy);
if (sameString(npu.host, "www.dropbox.com") || endsWith(npu.host, ".dl.dropboxusercontent.com"))
    dyStringPrintf(dy, "User-Agent: %s\r\n", "curl/8");
else 
    dyStringPrintf(dy, "User-Agent: %s\r\n", agent);

dyStringPrintf(dy, "Host: ");
netHandleHostForIpv6(&npu, dy);

boolean portIsDefault = FALSE;
/* do not need the 80 since it is the default */
if (sameString(npu.protocol, "http" ) && sameString("80", npu.port))
    portIsDefault = TRUE;
if (sameString(npu.protocol, "https" ) && sameString("443", npu.port))
    portIsDefault = TRUE;
if (!portIsDefault)
    {
    dyStringAppendC(dy, ':');
    dyStringAppend(dy, npu.port);
    }
dyStringPrintf(dy, "\r\n");

setAuthorization(npu, "Authorization", dy);
if (proxyUrl)
    setAuthorization(pxy, "Proxy-Authorization", dy);
dyStringAppend(dy, "Accept: */*\r\n");
if (npu.byteRangeStart != -1)
    {
    if (npu.byteRangeEnd != -1)
	dyStringPrintf(dy, "Range: bytes=%lld-%lld\r\n"
		       , (long long)npu.byteRangeStart
		       , (long long)npu.byteRangeEnd);
    else
	dyStringPrintf(dy, "Range: bytes=%lld-\r\n"
		       , (long long)npu.byteRangeStart);
    }

if (optionalHeader)
    dyStringAppend(dy, optionalHeader);
if (sameString(protocol, "HTTP/1.1"))
    dyStringAppend(dy, "Connection: close\r\n");  // NON-persistent HTTP 1.1 connection

/* finish off the header with final blank line */
dyStringAppend(dy, "\r\n");

mustWriteFd(sd, dy->string, dy->stringSize);


/* Clean up and return handle. */
dyStringFree(&dy);
return sd;
}


int netOpenHttpExt(char *url, char *method, char *optionalHeader)
/* Return a file handle that will read the url.  optionalHeader
 * may by NULL or may contain cookies and other info.  */
{
return netHttpConnect(url, method, "HTTP/1.0", "genome.ucsc.edu/net.c", optionalHeader);
}

static int netGetOpenHttp(char *url)
/* Return a file handle that will read the url.  */
{
return netOpenHttpExt(url, "GET", NULL);
}

int netUrlHeadExt(char *url, char *method, struct hash *hash)
/* Go get head and return status.  Return negative number if
 * can't get head. If hash is non-null, fill it with header
 * lines with upper cased keywords for case-insensitive lookup, 
 * including hopefully CONTENT-TYPE: . */
{
int sd = netOpenHttpExt(url, method, NULL);
int status = EIO;
if (sd >= 0)
    {
    char *line, *word;
    struct lineFile *lf = lineFileAttach(url, TRUE, sd);

    if (lineFileNext(lf, &line, NULL))
	{
	if (startsWith("HTTP/", line))
	    {
	    word = nextWord(&line);
	    word = nextWord(&line);
	    if (word != NULL && isdigit(word[0]))
	        {
		status = atoi(word);
		if (hash != NULL)
		    {
		    while (lineFileNext(lf, &line, NULL))
		        {
			word = nextWord(&line);
			if (word == NULL)
			    break;
			hashAdd(hash, strUpper(word), cloneString(skipLeadingSpaces(line)));
			}
		    }
		}
	    }
	}
    lineFileClose(&lf);
    }
else
    status = errno;
return status;
}


int netUrlHead(char *url, struct hash *hash)
/* Go get head and return status.  Return negative number if
 * can't get head. If hash is non-null, fill it with header
 * lines with upper cased keywords for case-insensitive lookup, 
 * including hopefully CONTENT-TYPE: . */
{
return netUrlHeadExt(url, "HEAD", hash);
}


int netUrlFakeHeadByGet(char *url, struct hash *hash)
/* Use GET with byteRange as an alternate method to HEAD. 
 * Return status. */
{
char rangeUrl[MAXURLSIZE];
safef(rangeUrl, sizeof(rangeUrl), "%s;byterange=0-0", url);
int status = netUrlHeadExt(rangeUrl, "GET", hash);
return status;
}


int netUrlOpenSockets(char *url, int *retCtrlSocket)
/* Return socket descriptor (low-level file handle) for read()ing url data,
 * or -1 if error. 
 * If retCtrlSocket is non-NULL and url is FTP, set *retCtrlSocket
 * to the FTP control socket which is left open for a persistent connection.
 * close(result) (and close(*retCtrlSocket) if applicable) when done. 
 * If url is missing :// then it's just treated as a file. */
{
if (stringIn("://", url) == NULL)
    return open(url, O_RDONLY);
else
    {
    if (startsWith("http://",url) || startsWith("https://",url))
	return netGetOpenHttp(url);
    else if (startsWith("ftp://",url))
	return netGetOpenFtpSockets(url, retCtrlSocket);
    else    
	errAbort("Sorry, can only netUrlOpen http, https and ftp currently, not '%s'", url);
    }
return -1;    
}

int netUrlOpen(char *url)
/* Return socket descriptor (low-level file handle) for read()ing url data,
 * or -1 if error.  Just close(result) when done. */
{
return netUrlOpenSockets(url, NULL);
}

struct dyString *netSlurpFile(int sd)
/* Slurp file into dynamic string and return. */
{
char buf[4*1024];
int readSize;
struct dyString *dy = dyStringNew(4*1024);

/* Slurp file into dy and return. */
while ((readSize = read(sd, buf, sizeof(buf))) > 0)
    dyStringAppendN(dy, buf, readSize);
return dy;
}

struct dyString *netSlurpUrl(char *url)
/* Go grab all of URL and return it as dynamic string. */
{
int sd = netUrlOpen(url);
if (sd < 0)
    errAbort("netSlurpUrl: failed to open socket for [%s]", url);
struct dyString *dy = netSlurpFile(sd);
close(sd);
return dy;
}

static void parseContentRange(char *x, ssize_t *rangeStart, ssize_t *rangeEnd)
/* parse the content range information from response header value 
	"bytes 763400000-763400112/763400113"
 */
{
/* default to no byte range specified */
*rangeStart = -1;
*rangeEnd = -1;
if (startsWith("bytes ", x))
    {
    char *y=strchr(x, ' ');
    ++y;
    char *z=strchr(y, '-');
    if (z)
	{
	++z;
	*rangeStart = atoll(y); 
	if (z[0] != '\0')
	    *rangeEnd = atoll(z);
	}    
    }

}


boolean netSkipHttpHeaderLinesWithRedirect(int sd, char *url, char **redirectedUrl)
/* Skip http header lines. Return FALSE if there's a problem.
 * The input is a standard sd or fd descriptor.
 * This is meant to be able work even with a re-passable stream handle,
 * e.g. can pass it to the pipes routines, which means we can't
 * attach a linefile since filling its buffer reads in more than just the http header.
 * Handles 301, 302, 307, 308 http redirects by setting *redirectedUrl to
 * the new location. */
{
char buf[8192];
char *line = buf;
int maxbuf = sizeof(buf);
int i=0;
char c = ' ';
int nread = 0;
char *sep = NULL;
char *headerName = NULL;
char *headerVal = NULL;
boolean redirect = FALSE;
boolean byteRangeUsed = (strstr(url,";byterange=") != NULL);
ssize_t byteRangeStart = -1;
ssize_t byteRangeEnd = -1;
boolean foundContentRange = FALSE;
ssize_t contentRangeStart = -1;
ssize_t contentRangeEnd = -1;

boolean mustUseProxy = FALSE;  /* User must use proxy 305 error*/
char *proxyLocation = NULL;
boolean mustUseProxyAuth = FALSE;  /* User must use proxy authentication 407 error*/

if (byteRangeUsed)
    {
    parseByteRange(url, &byteRangeStart, &byteRangeEnd, FALSE);
    }

while(TRUE)
    {
    i = 0;
    while (TRUE)
	{
	nread = read(sd, &c, 1);  /* one char at a time, but http headers are small */
	if (nread != 1)
	    {
	    if (nread == -1)
		{
		if (errno == EINTR)
		    continue;
    		warn("Error (%s) reading http header on %s", strerror(errno), url);
		}
	    else if (nread == 0)
    		warn("Error unexpected end of input reading http header on %s", url);
	    else
    		warn("Error reading http header on %s", url);
	    return FALSE;  /* err reading descriptor */
	    }
	if (c == 10)
	    break;
	if (c != 13)
    	    buf[i++] = c;
	if (i >= maxbuf)
	    {
	    warn("http header line too long > %d chars.",maxbuf);
	    return FALSE;
	    }
	}
    buf[i] = 0;  /* add string terminator */

    if (sameString(line,""))
	{
	break; /* End of Header found */
	}
    if (startsWith("HTTP/", line))
        {
	char *code;
	nextWord(&line);  // version
	code = nextWord(&line);
	if (code == NULL)
	    {
	    warn("Strange http header on %s", url);
	    return FALSE;
	    }
	if (sameString(code, "301")
	 || sameString(code, "302")
	 || sameString(code, "307")
	 || sameString(code, "308")
	   )
	    {
	    redirect = TRUE;
	    }
	else if (sameString(code, "305"))
	    {
	    mustUseProxy = TRUE;
	    }
	else if (sameString(code, "407"))
	    {
	    mustUseProxyAuth = TRUE;
	    }
	else if (byteRangeUsed 
	    /* hack for Apache bug 2.2.20 and 2.2.21 2011-10-21 should be OK to remove after one year. */
		&& !(sameString(code, "200") && byteRangeStart == 0 && byteRangeEnd == -1))  
	    {
	    if (!sameString(code, "206"))
		{
		if (sameString(code, "200"))
		    warn("Byte-range request was ignored by server. ");
		warn("Expected Partial Content 206. %s: %s %s. rangeStart=%lld rangeEnd=%lld", 
		    url, code, line, (long long)byteRangeStart, (long long)byteRangeEnd);
		return FALSE;
		}
	    }
	else if (sameString(code, "404"))
	    {
	    warn("404 file not found on %s", url);
	    return FALSE;
	    }
	else if (!sameString(code, "200"))
	    {
	    warn("Expected 200 %s: %s %s", url, code, line);
	    return FALSE;
	    }
	line = buf;  /* restore it */
	}
    headerName = line;
    sep = strchr(line,':');
    if (sep)
	{
	*sep = 0;
	headerVal = skipLeadingSpaces(++sep);
	}
    else
	{
	headerVal = NULL;
	}
    if (sameWord(headerName,"Location"))
	{
	if (redirect)
	    *redirectedUrl = cloneString(headerVal);
	if (mustUseProxy)
	    proxyLocation = cloneString(headerVal);
	}
    if (sameWord(headerName,"Content-Range"))
	{
	if (byteRangeUsed)
	    {
	    foundContentRange = TRUE;
	    parseContentRange(headerVal, &contentRangeStart, &contentRangeEnd);	
    	    if ((contentRangeStart != byteRangeStart) ||
		(byteRangeEnd != -1 && (contentRangeEnd != byteRangeEnd)))
		{
		char bre[256];
		safef(bre, sizeof bre, "%lld", (long long)byteRangeEnd);
		if (byteRangeEnd == -1)
		    bre[0] = 0;
		warn("Found Content-Range: %s. Expected bytes %lld-%s. Improper caching of 206 reponse byte-ranges?",
		    headerVal, (long long) byteRangeStart, bre);
    		return FALSE;
		}
	    }
	}
    }
if (mustUseProxy ||  mustUseProxyAuth)
    {
    warn("%s: %s error. Use Proxy%s. Location = %s", url, 
	mustUseProxy ? "" : " Authentication", 
	mustUseProxy ? "305" : "407", 
	proxyLocation ? proxyLocation : "not given");
    return FALSE;
    }
if (byteRangeUsed && !foundContentRange && !redirect
	    /* hack for Apache bug 2.2.20 and 2.2.21 2011-10-21 should be OK to remove after one year. */
		&& !(byteRangeStart == 0 && byteRangeEnd == -1))  
    {
    char bre[256];
    safef(bre, sizeof bre, "%lld", (long long)byteRangeEnd);
    if (byteRangeEnd == -1)
	bre[0] = 0;
    warn("Expected response header Content-Range: %lld-%s", (long long) byteRangeStart, bre);
    return FALSE;
    }

return TRUE;
}

char *transferParamsToRedirectedUrl(char *url, char *newUrl)
/* Transfer password, byteRange, and any other parameters from url to newUrl and return result.
 * freeMem result. */
{
struct netParsedUrl npu, newNpu;
/* Parse the old URL to make parts available for graft onto the redirected url. */
/* This makes redirection work with byterange urls and user:password@ */
netParseUrl(url, &npu);
netParseUrl(newUrl, &newNpu);
if (npu.byteRangeStart != -1)
    {
    newNpu.byteRangeStart = npu.byteRangeStart;
    newNpu.byteRangeEnd = npu.byteRangeEnd;
    }
if ((npu.user[0] != 0) && (newNpu.user[0] == 0))
    {
    safecpy(newNpu.user,     sizeof newNpu.user,     npu.user);
    safecpy(newNpu.password, sizeof newNpu.password, npu.password);
    }
return urlFromNetParsedUrl(&newNpu);
}

boolean netSkipHttpHeaderLinesHandlingRedirect(int sd, char *url, int *redirectedSd, char **redirectedUrl)
/* Skip http headers lines, returning FALSE if there is a problem.  Generally called as
 *    netSkipHttpHeaderLine(sd, url, &sd, &url);
 * where sd is a socket (file) opened with netUrlOpen(url), and url is in dynamic memory.
 * If the http header indicates that the file has moved, then it will update the *redirectedSd and
 * *redirectedUrl with the new socket and URL, first closing sd.
 * If for some reason you want to detect whether the forwarding has occurred you could
 * call this as:
 *    char *newUrl = NULL;
 *    int newSd = 0;
 *    netSkipHttpHeaderLine(sd, url, &newSd, &newUrl);
 *    if (newUrl != NULL)
 *          // Update sd with newSd, free url if appropriate and replace it with newUrl, etc.
 *          //  free newUrl when finished.
 * This routine handles up to 5 steps of redirection.
 * The logic to this routine is also complicated a little to make it work in a pipe, which means we
 * can't attach a lineFile since filling the lineFile buffer reads in more than just the http header. */
{
int redirectCount = 0;
while (TRUE)
    {
    /* url needed for err msgs, and to return redirect location */
    char *newUrl = NULL;
    boolean success = netSkipHttpHeaderLinesWithRedirect(sd, url, &newUrl);

    // removing any hashkey from redirect location
    if (newUrl)
	{
	char *hashKey = strchr(newUrl, '#');  // truncate at hashkey
	if (hashKey)
	    {
	    *hashKey = 0;
	    }
	}

    if (success && !newUrl) /* success after 0 to 5 redirects */
        {
	if (redirectCount > 0)
	    {
	    *redirectedSd = sd;
	    *redirectedUrl = url;
	    }
	else
	    {
	    *redirectedSd = -1;
	    *redirectedUrl = NULL;
	    }
	return TRUE;
	}
    close(sd);
    if (success)
	{
	/* we have a new url to try */
	++redirectCount;
	if (redirectCount > 5)
	    {
	    warn("code 30x redirects: exceeded limit of 5 redirects, %s", newUrl);
	    success = FALSE;
	    }
	else 
	    {
	    // path may be relative
	    if (!hasProtocol(newUrl))
		{
		char *newUrl2 = expandUrlOnBase(url, newUrl);
		freeMem(newUrl);
		newUrl = newUrl2;
		}
	    if (!startsWith("http://",newUrl) 
              && !startsWith("https://",newUrl))
		{
		warn("redirected to non-http(s): %s", newUrl);
		success = FALSE;
		}
	    else
		{
		// transfer password and byteranges if any
		newUrl = transferParamsToRedirectedUrl(url, newUrl);
		sd = netUrlOpen(newUrl);
		if (sd < 0)
		    {
		    warn("Couldn't open %s", newUrl);
		    success = FALSE;
		    }
		}
	    }
	}
    if (redirectCount > 1)
	freeMem(url);
    if (!success)
	{  /* failure after 0 to 5 redirects */
	if (redirectCount > 0)
	    freeMem(newUrl);
	return FALSE;
	}
    url = newUrl;
    }
return FALSE;
}

struct lineFile *netLineFileMayOpen(char *url)
/* Return a lineFile attached to url. http skips header.
 * Supports some compression formats.  Prints warning message, but
 * does not abort, just returning NULL if there's a problem. */
{
int sd = netUrlOpen(url);
if (sd < 0)
    {
    warn("Couldn't open %s", url);
    return NULL;
    }
else
    {
    struct lineFile *lf = NULL;
    char *newUrl = NULL;
    int newSd = 0;
    if (startsWith("http://",url) || startsWith("https://",url))
	{  
	if (!netSkipHttpHeaderLinesHandlingRedirect(sd, url, &newSd, &newUrl))
	    {
	    return NULL;
	    }
	if (newUrl != NULL)
	    {
    	    /*  Update sd with newSd, replace it with newUrl, etc. */
	    sd = newSd;
	    url = newUrl;
	    }
	}
    char *urlDecoded = cloneString(url);
    cgiDecode(url, urlDecoded, strlen(url));
    boolean isCompressed =
	(endsWith(urlDecoded,".gz") ||
   	 endsWith(urlDecoded,".Z")  ||
	 endsWith(urlDecoded,".bz2"));
    freeMem(urlDecoded);
    if (isCompressed)
	{
	lf = lineFileDecompressFd(url, TRUE, sd);
           /* url needed only for compress type determination */
	}
    else
	{
	lf = lineFileAttach(url, TRUE, sd);
	}
    if (newUrl) 
	freeMem(newUrl); 
    return lf;
    }
}

int netUrlMustOpenPastHeader(char *url)
/* Get socket descriptor for URL.  Process header, handling any forwarding and
 * the like.  Do errAbort if there's a problem, which includes anything but a 200
 * return from http after forwarding. */
{
int sd = netUrlOpen(url);
if (sd < 0)
    noWarnAbort();
int newSd = 0;
if (startsWith("http://",url) || startsWith("https://",url))
    {  
    char *newUrl = NULL;
    if (!netSkipHttpHeaderLinesHandlingRedirect(sd, url, &newSd, &newUrl))
	noWarnAbort();
    if (newUrl != NULL)
	{
	sd = newSd;
	freeMem(newUrl); 
	}
    }
return sd;
}

struct lineFile *netLineFileSilentOpen(char *url)
/* Open a lineFile on a URL.  Just return NULL without any user
 * visible warning message if there's a problem. */
{
pushSilentWarnHandler();
struct lineFile *lf = netLineFileMayOpen(url);
popWarnHandler();
return lf;
}

char *netReadTextFileIfExists(char *url)
/* Read entire URL and return it as a string.  URL should be text (embedded zeros will be
 * interpreted as end of string).  If the url doesn't exist or has other problems,
 * returns NULL. */
{
struct lineFile *lf = netLineFileSilentOpen(url);
if (lf == NULL)
    return NULL;
char *text = lineFileReadAll(lf);
lineFileClose(&lf);
return text;
}

struct lineFile *netLineFileOpen(char *url)
/* Return a lineFile attached to url.  This one
 * will skip any headers.   Free this with
 * lineFileClose(). */
{
struct lineFile *lf = netLineFileMayOpen(url);
if (lf == NULL)
    noWarnAbort();
return lf;
}

boolean netSendString(int sd, char *s)
/* Send a string down a socket - length byte first. */
{
int length = strlen(s);
UBYTE len;

if (length > 255)
    errAbort("Trying to send a string longer than 255 bytes (%d bytes)", length);
len = length;
if (write(sd, &len, 1)<0)
    {
    warn("Couldn't send string to socket");
    return FALSE;
    }
if (write(sd, s, length)<0)
    {
    warn("Couldn't send string to socket");
    return FALSE;
    }
return TRUE;
}

boolean netSendLongString(int sd, char *s)
/* Send a long string down socket: two bytes for length. */
{
unsigned length = strlen(s);
UBYTE b[2];

if (length >= 64*1024)
    {
    warn("Trying to send a string longer than 64k bytes (%d bytes)", length);
    return FALSE;
    }
b[0] = (length>>8);
b[1] = (length&0xff);
if (write(sd, b, 2) < 0)
    {
    warn("Couldn't send long string to socket");
    return FALSE;
    }
if (write(sd, s, length)<0)
    {
    warn("Couldn't send long string to socket");
    return FALSE;
    }
return TRUE;
}

boolean netSendHugeString(int sd, char *s)
/* Send a long string down socket: four bytes for length. */
{
unsigned long length = strlen(s);
unsigned long l = length;
UBYTE b[4];
int i;
for (i=3; i>=0; --i)
    {
    b[i] = l & 0xff;
    l >>= 8;
    }
if (write(sd, b, 4) < 0)
    {
    warn("Couldn't send huge string to socket");
    return FALSE;
    }
if (write(sd, s, length) < 0)
    {
    warn("Couldn't send huge string to socket");
    return FALSE;
    }
return TRUE;
}


char *netGetString(int sd, char buf[256])
/* Read string into buf and return it.  If buf is NULL
 * an internal buffer will be used. Print warning message
 * and return NULL if any problem. */
{
static char sbuf[256];
UBYTE len = 0;
int length;
int sz;
if (buf == NULL) buf = sbuf;
sz = netReadAll(sd, &len, 1);
if (sz == 0)
    return NULL;
if (sz < 0)
    {
    warn("Couldn't read string length");
    return NULL;
    }
length = len;
if (length > 0)
    if (netReadAll(sd, buf, length) < 0)
	{
	warn("Couldn't read string body");
	return NULL;
	}
buf[length] = 0;
return buf;
}

char *netGetLongString(int sd)
/* Read string and return it.  freeMem
 * the result when done. */
{
UBYTE b[2];
char *s = NULL;
int length = 0;
int sz;
b[0] = b[1] = 0;
sz = netReadAll(sd, b, 2);
if (sz == 0)
    return NULL;
if (sz < 0)
    {
    warn("Couldn't read long string length");
    return NULL;
    }
length = (b[0]<<8) + b[1];
s = needMem(length+1);
if (length > 0)
    if (netReadAll(sd, s, length) < 0)
	{
	warn("Couldn't read long string body");
	return NULL;
	}
s[length] = 0;
return s;
}

char *netGetHugeString(int sd)
/* Read string and return it.  freeMem
 * the result when done. */
{
UBYTE b[4];
char *s = NULL;
unsigned long length = 0;
int sz, i;
sz = netReadAll(sd, b, 4);
if (sz == 0)
    return NULL;
if (sz < 4)
    {
    warn("Couldn't read huge string length");
    return NULL;
    }
for (i=0; i<4; ++i)
    {
    length <<= 8;
    length += b[i];
    }
s = needMem(length+1);
if (length > 0)
    {
    if (netReadAll(sd, s, length) < 0)
	{
	warn("Couldn't read huge string body");
	return NULL;
	}
    }
s[length] = 0;
return s;
}


char *netRecieveString(int sd, char buf[256])
/* Read string into buf and return it.  If buf is NULL
 * an internal buffer will be used. Abort if any problem. */
{
char *s = netGetString(sd, buf);
if (s == NULL)
     noWarnAbort();   
return s;
}

char *netRecieveLongString(int sd)
/* Read string and return it.  freeMem
 * the result when done. Abort if any problem*/
{
char *s = netGetLongString(sd);
if (s == NULL)
     noWarnAbort();   
return s;
}

char *netRecieveHugeString(int sd)
/* Read string and return it.  freeMem
 * the result when done. Abort if any problem*/
{
char *s = netGetHugeString(sd);
if (s == NULL)
     noWarnAbort();   
return s;
}


struct lineFile *netHttpLineFileMayOpen(char *url, struct netParsedUrl **npu)
/* Parse URL and open an HTTP socket for it but don't send a request yet. */
{
int sd;
struct lineFile *lf;

/* Parse the URL and try to connect. */
AllocVar(*npu);
netParseUrl(url, *npu);
if (!sameString((*npu)->protocol, "http"))
    errAbort("netHttpLineFileMayOpen: url (%s) is not for http.", url);
sd = netConnect((*npu)->host, atoi((*npu)->port));
if (sd < 0)
    return NULL;

/* Return handle. */
lf = lineFileAttach(url, TRUE, sd);
return lf;
} /* netHttpLineFileMayOpen */


void netHttpGet(struct lineFile *lf, struct netParsedUrl *npu,
		boolean keepAlive)
/* Send a GET request, possibly with Keep-Alive. */
{
struct dyString *dy = dyStringNew(512);

/* Ask remote server for the file/query. */
dyStringPrintf(dy, "GET %s HTTP/1.1\r\n", npu->file);
dyStringPrintf(dy, "User-Agent: genome.ucsc.edu/net.c\r\n");

dyStringPrintf(dy, "Host: ");
netHandleHostForIpv6(npu, dy);
dyStringAppendC(dy, ':');
dyStringAppend(dy, npu->port);
dyStringPrintf(dy, "\r\n");

if (!sameString(npu->user,""))
    {
    char up[256];
    char *b64up = NULL;
    safef(up,sizeof(up), "%s:%s", npu->user, npu->password);
    b64up = base64Encode(up, strlen(up));
    dyStringPrintf(dy, "Authorization: Basic %s\r\n", b64up);
    freez(&b64up);
    }
dyStringAppend(dy, "Accept: */*\r\n");
if (keepAlive)
  {
    dyStringAppend(dy, "Connection: Keep-Alive\r\n");
    dyStringAppend(dy, "Connection: Persist\r\n");
  }
else
    dyStringAppend(dy, "Connection: close\r\n");
dyStringAppend(dy, "\r\n");
mustWriteFd(lf->fd, dy->string, dy->stringSize);
/* Clean up. */
dyStringFree(&dy);
} /* netHttpGet */

int netHttpGetMultiple(char *url, struct slName *queries, void *userData,
		       void (*responseCB)(void *userData, char *req,
					  char *hdr, struct dyString *body))
/* Given an URL which is the base of all requests to be made, and a 
 * linked list of queries to be appended to that base and sent in as 
 * requests, send the requests as a batch and read the HTTP response 
 * headers and bodies.  If not all the requests get responses (i.e. if 
 * the server is ignoring Keep-Alive or is imposing a limit), try again 
 * until we can't connect or until all requests have been served. 
 * For each HTTP response, do a callback. */
{
  struct slName *qStart;
  struct slName *qPtr;
  struct lineFile *lf;
  struct netParsedUrl *npu;
  struct dyString *dyQ    = dyStringNew(512);
  struct dyString *body;
  char *base;
  char *hdr;
  int qCount;
  int qTotal;
  int numParseFailures;
  int contentLength;
  boolean chunked;
  boolean done;
  boolean keepAlive;

  /* Find out how many queries we'll need to do so we know how many times 
   * it's OK to run into end of file in case server ignores Keep-Alive. */
  qTotal = 0;
  for (qPtr = queries;  qPtr != NULL;  qPtr = qPtr->next)
    {
      qTotal++;
    }

  done = FALSE;
  qCount = 0;
  numParseFailures = 0;
  qStart = queries;
  while ((! done) && (qStart != NULL))
    {
      lf = netHttpLineFileMayOpen(url, &npu);
      if (lf == NULL)
	{
	  done = TRUE;
	  break;
	}
      base = cloneString(npu->file);
      /* Send all remaining requests with keep-alive. */
      for (qPtr = qStart;  qPtr != NULL;  qPtr = qPtr->next)
	{
	  dyStringClear(dyQ);
	  dyStringAppend(dyQ, base);
	  dyStringAppend(dyQ, qPtr->name);
	  strcpy(npu->file, dyQ->string);
	  keepAlive = (qPtr->next == NULL) ? FALSE : TRUE;
	  netHttpGet(lf, npu, keepAlive);
	}
      /* Get as many responses as we can; call responseCB() and 
       * advance qStart for each. */
      for (qPtr = qStart;  qPtr != NULL;  qPtr = qPtr->next)
        {
	  if (lineFileParseHttpHeader(lf, &hdr, &chunked, &contentLength))
	    {
	      body = lineFileSlurpHttpBody(lf, chunked, contentLength);
	      dyStringClear(dyQ);
	      dyStringAppend(dyQ, base);
	      dyStringAppend(dyQ, qPtr->name);
	      responseCB(userData, dyQ->string, hdr, body);
	      qStart = qStart->next;
	      qCount++;
	    }
	  else
	    {
	      if (numParseFailures++ > qTotal) {
		done = TRUE;
	      }
	      break;
	    }
	}
    }

  return qCount;
} /* netHttpMultipleQueries */

boolean hasProtocol(char *urlOrPath)
/* Return TRUE if it looks like it has http://, ftp:// etc. */
{
return stringIn("://", urlOrPath) != NULL;
}

void netSetTimeoutErrorMsg(char *msg) {
    timeoutErrorMessage = msg;
}
