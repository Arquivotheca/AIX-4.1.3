static char sccsid[] = "@(#)00	1.18.1.1  src/bos/usr/lib/sendmail/daemon.c, cmdsend, bos411, 9428A410j 3/16/93 10:37:52";
/* 
 * COMPONENT_NAME: CMDSEND daemon.c
 * 
 * FUNCTIONS: MSGSTR, clrdaemon, getrequests, makeconnection, 
 *            maphostname, myhostname, proc_src, src_reply, src_stats 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
**
*/


#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

# include <errno.h>
# include <stdio.h>
# include <ctype.h>
# include <string.h>
# include <memory.h>
# include <sys/types.h>

# include "conf.h"
# include "useful.h"

# include <sys/uio.h>
# include <sys/socket.h>
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"

# include <netdb.h>
# include <signal.h>

#include <sys/select.h>
#include <spc.h>

#include <arpa/nameser.h>
#include <resolv.h>
            
extern char *errstring();  /* gets text string from errno */
extern struct hostent *gethostbyaddr();
extern struct hostent *gethostbyname();
extern char *inet_ntoa();
extern void initconf();
extern int intsig();
extern char  *xalloc ();
extern ENVELOPE *newenvelope ();
extern void exit ();

extern int  Sid;

/*
**  DAEMON.C -- routines to use when running as a daemon.
**
**	This entire file is highly dependent on the 4.2 BSD
**	interprocess communication primitives.  No attempt has
**	been made to make this file portable to Version 7,
**	Version 6, MPX files, etc.  If you should try such a
**	thing yourself, I recommend chucking the entire file
**	and starting from scratch.  Basic semantics are:
**
**	getrequests()
**		Opens a port and initiates a connection.
**		Returns in a child.  Must set InChannel and
**		OutChannel appropriately.
**	clrdaemon()
**		Close any open files associated with getting
**		the connection; this is used when running the queue,
**		etc., to avoid having extra file descriptors during
**		the queue run and to avoid confusing the network
**		code (if it cares).
**	makeconnection(host, port, outfile, infile)
**		Make a connection to the named host on the given
**		port.  Set *outfile and *infile to the files
**		appropriate for communication.  Returns zero on
**		success, else an exit status describing the
**		error.
**	maphostname(hbuf, hbufsize)
**		Convert the entry in hbuf into a canonical form.  It
**		may not be larger than hbufsize.
*/
/*
**  GETREQUESTS -- open mail IPC port and get requests.
**
**	Environment:
**		SIGCLD handling must be set to SIG_IGN.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Waits until some interesting activity occurs.  When
**		it does, a child is created to process it, and the
**		parent waits for completion.  Return from this
**		routine is always in the child.  The file pointers
**		"InChannel" and "OutChannel" should be set to point
**		to the communication channel.
*/

struct sockaddr_in	SendmailAddress;/* internet address of sendmail */

int	DaemonSocket	= -1;		/* fd describing socket */

static char progname[] = "sendmail";  /* name of our src "object" */
static char mailstats[] = "/usr/lib/mailstats 2>&1";  /* mailstats cmd line */
static struct srcreq srcmsg;  /* for receiving src msgs */
static struct srcrep srcreply;  /* for replying to src msgs */
static struct srchdr *srchdr;  /* global header ptr for replies */
static void proc_src();
extern int Src_fd;  /* src file descriptor, opened for us by main() */

getrequests()
{
	int t;
	register struct servent *sp;
	int on = 1;
	struct sigvec sigin;
	fd_set read_list;  /* bit mask for select() */
	int mask_size;

	/*
	 *  We are fixing to touch the queue.  Indicate our intentions by
	 *  pulling a count from the semaphore.  We may wait here if a
	 *  clean is in progress.
	 *
	 *  All exceptions cause a syserr, but don't otherwise interfere
	 *  with sendmail operation.  We assume that if it fails here, it
	 *  fails in orderq and elsewhere, and no clean will be attempted.
	 */
	(void) semwait (Sid, 1, 0, 0);

#ifdef DEBUG
	if (tTd (15, 111))		/* sema4 test */
	    exit (99);
#endif DEBUG

	dropenvelope(CurEnv);

	/*
	**  Set up the address for the mailer.
	*/

	sp = getservbyname("smtp", "tcp");
	if (sp == NULL)
	{
		syserr(MSGSTR(DM_ESVR, "server \"smtp\" unknown"));
# ifdef LOG
		syslog (LOG_ALERT, MSGSTR(DM_SMTP, "server SMTP unknown"));
# endif LOG
		finis();
	}

	SendmailAddress.sin_family = AF_INET;
	SendmailAddress.sin_addr.s_addr = INADDR_ANY;
	SendmailAddress.sin_port = sp->s_port;

	/*
	**  Try to actually open the connection.
	*/

# ifdef DEBUG
	if (tTd(15, 1))
	    (void) printf("getrequests: port 0x%x\n", SendmailAddress.sin_port);
# endif DEBUG

	/* get a socket for the SMTP connection */
	DaemonSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (DaemonSocket < 0)
	{
		/* probably another daemon already */
		syserr(MSGSTR(DM_ESOCK, "getrequests: can't create socket")); 
# ifdef LOG
		syslog(LOG_ALERT, MSGSTR(DM_ESOCK2, "cannot create socket")); 
# endif LOG
		finis();
	}

#ifdef DEBUG
	/* turn on network debugging? */
	if (tTd(15, 15))
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, 
							(char *)&on, sizeof on);
#endif DEBUG

	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_REUSEADDR, 
							(char *)&on, sizeof on);
	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_KEEPALIVE, 
							(char *)&on, sizeof on);

	if (bind(DaemonSocket, &SendmailAddress, sizeof SendmailAddress) < 0)
	{
	    syserr(MSGSTR(DM_EBIND,
		"getrequests: cannot bind socket to port %d"),
		SendmailAddress.sin_port);
	    (void) close(DaemonSocket);
# ifdef LOG
	    syslog(LOG_ALERT, MSGSTR(DM_BIND2, "cannot bind socket to port %d"),
		SendmailAddress.sin_port);
# endif LOG
	    finis();
	}

	if (listen(DaemonSocket, 10) < 0)
	{
		syserr(MSGSTR(DM_LISTEN, "getrequests: cannot listen")); 
		(void) close(DaemonSocket);
# ifdef LOG
		syslog(LOG_ALERT, MSGSTR(DM_LISTEN2,
		    "cannot listen on connection"));
# endif LOG
		finis();
	}

# ifdef DEBUG
	if (tTd(15, 1))
	    (void) printf("getrequests: %d\n", DaemonSocket);
# endif DEBUG

	/*
	 *  Indicate that we are out of the queue, till mail
	 *  comes in.
	 *
	 *  All exceptions cause a syserr, but don't otherwise interfere
	 *  with sendmail operation.  If it fails here we assume that it
	 *  fails in orderq and elsewhere, and no queue clean will be
	 *  attempted.
	 */
	(void) semsig (Sid, 1, 0);

	/* init number of file descriptors that select looks at in read_list */
	mask_size = (DaemonSocket > Src_fd ? DaemonSocket : Src_fd) + 1;
	FD_ZERO(&read_list);

	/* set up static fields in srcrep */
	strcpy(srcreply.svrreply.objname, progname);
	srcreply.svrreply.objtype = SUBSYSTEM;

	for (;;)
	{
		register int pid;
		int lotherend;

# ifdef DEBUG
		if (tTd(15, 2))
		    (void) printf("getrequests: loop\n");
# endif DEBUG

		/* see if we are rejecting connections */
		while (getla() > RefuseLA)
			sleep(15);

		/* wait for a connection or an src message (if we were invoked
		   by src): we select on both sockets, and check for an smtp
		   connection first; on errors not caused by EINTR, we print
		   an error message and wait a bit to allow it to clear */

		/* set up select bits for src and daemon sockets */
		FD_SET(DaemonSocket, &read_list);
		if (Src_fd)  /* only set it if we were invoked by src */
		    FD_SET(Src_fd, &read_list);

		/* wait until we can read from one of the sockets */
		t = select(mask_size, &read_list, 0, 0, (struct timeval *) 0);
		if (t == -1) {
		    if (errno != EINTR) {  /* notify of error */
			syserr(MSGSTR(DM_SEL,
			    "getrequests: select"));
			sleep(15);  /* wait a bit */
		    }
		    continue;  /* and try again */
		}

		/* check first for an smtp connection */
		if (FD_ISSET(DaemonSocket, &read_list)) {

		    /* got one: try to accept it */
		    lotherend = sizeof RealHostAddr;
		    errno = 0;
		    t = accept(DaemonSocket, &RealHostAddr, &lotherend);
		    if (t == -1) {
			if (errno != EINTR) {  /* notify of error */
			    syserr(MSGSTR(DM_ACC,
				"getrequests: accept"));
			    sleep(15);  /* wait a bit */
			}

		    } else {  /* accept succeeded */
		

			/*
			**  got an smtp connection:
			**  create a subprocess to process the mail.
			*/

# ifdef DEBUG
			if (tTd(15, 2))
			    (void) printf("getrequests: forking (fd = %d)\n", t);
# endif DEBUG

			/*
			 *  Create child to handle this connection request.
			 *  Since this is the daemon, it must be set to prevent
			 *  zombies.
			 *  The children just fend for themselves and are
			 *  unmonitored by the parent.
			 *
			 *  Failure to fork is nonfatal and does not cause loss
			 *  of data.  The connection is closed and presumably
			 *  retried later.  A time delay is introduced to
			 *  prevent excessive activity.
			 *  This is probably a kloodge, since I don't know how
			 *  many connection attempts per minute can be
			 *  dispatched to sendmail.
			 */
			pid = fork();
			if (pid < 0)
			{
				syserr(MSGSTR(DM_EFORK, "daemon: cannot fork"));

				/*
				 *  Wait awhile for supposed lack of system
				 *  resources to clear up.
				 */
				sleep(30);

				/*
				 *  Go close the connection as if finished.  The
				 *  other end will probably try again.
				 */
			}

			if (pid == 0)
			{
				register struct hostent *hp;
				char buf[MAXNAME];

				/*
				**  CHILD -- speak SMTP to other end of link.
				**	Collect verified idea of sending host.
				**	Verify calling user id if possible here.
				*/

#ifdef DEBUG
				if (tTd (15, 110))		/* sema4 test */
				    exit (99);
#endif DEBUG
				/*
				 *  Indicate our intentions to enter the queue.
				 *  We may delay here if a clean is in progres.
				 *  Process exit will signal the sema4.
				 *
				 *  All exceptions cause a syserr, but don't
				 *  otherwise interfere with sendmail operation.
				 *  We assume that if it fails here it fails
				 *  in orderq and elsewhere
				 *  and no queue clean will be attempted.
				 */
				(void) semwait (Sid, 1, 0, 0);
#ifdef DEBUG
				if (tTd (15, 109))		/* sema4 test */
				    exit (99);
#endif DEBUG
				sigin.sv_handler = SIG_DFL;
				sigin.sv_mask = 0;
				sigin.sv_onstack = 0;
				/* make zombies */
				if(sigvec(SIGCLD, &sigin, (struct sigvec *) 0))
				   perror(MSGSTR(DM_SIG,
				    "getrequest:sigvec:SIGCLD"));

				/* determine host name */
				hp = gethostbyaddr ((char *)
				    &RealHostAddr.sin_addr,
				    sizeof RealHostAddr.sin_addr, AF_INET);
				if (hp != NULL)
					(void) strcpy(buf, hp->h_name);
				else
				{
					/* produce a dotted quad */
					(void) sprintf(buf, "[%s]",
					    inet_ntoa(RealHostAddr.sin_addr));
				}

				RealHostName = (char *)newstr(buf);

				(void) close(DaemonSocket);
				InChannel = fdopen(t, "r");
				OutChannel = fdopen(dup(t), "w");
# ifdef LOG
				if (LogLevel > 11)
					syslog(LOG_DEBUG, MSGSTR(DM_CONN,
					    "connected, pid=%d"), getpid());
# endif LOG
				OpMode = MD_SMTP;	/* now talk SMTP */
				(void) newenvelope(CurEnv);
				openxscript(CurEnv);
				smtp ();		/* never returns */

			/*NOTREACHED*/
			}

#ifdef DEBUG
			if (tTd(4, 5)) {  /* single-threaded: wait for child */

			    int st, wpid;

				syslog(LOG_DEBUG,
				    "daemon: parent: waiting on %d", pid);
				if ((wpid = wait(&st)) == -1)
				    syslog(LOG_DEBUG,
					"daemon: parent: wait error %d", errno);
				else
				    syslog(LOG_DEBUG,
					"daemon: child %d returned %#x",
					wpid, st);
			}
#endif DEBUG

			/*
			 *  If we are a successful parent, just close the file
			 *  descriptor since the child is handling it.  If we
			 *  are an unsuccessful parent, close it anyway and
			 *  reject communication on it.  The other side will
			 *  probably try to connect again.
			 */
			(void) close(t);

		    }  /* end of successfule accept */
		}  /* end of smtp connection */

		/* check for an src message */
		if (Src_fd && FD_ISSET(Src_fd, &read_list)) {
		    /* get the src msg */
		    if (recvfrom(Src_fd, &srcmsg, sizeof(srcmsg), 0, 0, 0)
			== -1) {
			    if (errno != EINTR)  /* notify of error */
				syserr(MSGSTR(DM_RECV,
				    "getrequests: recvfrom"));

		    } else {  /* set up the header and process the msg */
			srchdr = srcrrqs(&srcmsg);
			proc_src(&srcmsg);
		    }
		}
	}  /* end of forever loop */
	/*NOTREACHED*/
}
/*
**  CLRDAEMON -- reset the daemon connection
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		releases any resources used by the passive daemon.
*/

clrdaemon()
{
	if (DaemonSocket >= 0)
		(void) close(DaemonSocket);
	DaemonSocket = -1;
}
/*
**  MAKECONNECTION -- make a connection to an SMTP socket on another machine.
**
**	Parameters:
**		host -- the name of the host.
**		port -- the port number to connect to.
**		outfile -- a pointer to a place to put the outfile
**			descriptor.
**		infile -- ditto for infile.
**
**	Returns:
**		An exit code telling whether the connection could be
**			made and if not why not.
**
**	Side Effects:
**		none.
*/

makeconnection(host, port, outfile, infile)
	char *host;
	unsigned short port;
	FILE **outfile;
	FILE **infile;
{
	register int i, s;
	register struct hostent *hp = (struct hostent *)NULL;
	extern char *inet_ntoa();
	int sav_errno;
	extern int h_errno;

# ifdef DEBUG
	if (tTd(15, 1))
	    (void) printf("makeconnection: host %s, port %d\n", host, port);
# endif DEBUG

	/*
	**  Set up the address for the mailer.
	**	Accept "[a.b.c.d]" syntax for host name.
	*/

	h_errno = 0;
	errno = 0;

	if (host[0] == '[')
	{
		long hid;
		register char *p;

# ifdef DEBUG
	if (tTd(15, 1))
	    (void) printf("makeconnection: processing [] type host addr\n");
# endif DEBUG

		p = strchr(host, ']');

		if (p != NULL)
		{
			*p = '\0';
			hid = inet_addr(&host[1]);
			*p = ']';
		}
		if (p == NULL || hid == -1)
		{
			usrerr(MSGSTR(DM_EDOMAIN, "Invalid numeric domain spec \"%s\""), host); 
			return (EX_NOHOST);
		}
		SendmailAddress.sin_addr.s_addr = hid;
	}
	else
	{
		char  hbuf[MAXNAME];

# ifdef DEBUG
	if (tTd(15, 1))
	    (void) printf(MSGSTR(DM_MCONN, "makeconnection: perform gethostbyname\n")); 
# endif DEBUG

		errno = 0;              /* will still be zero on EOF    */
		(void) unquotestr (hbuf, host, MAXNAME);
		hp = gethostbyname(hbuf);

		if (hp == NULL)         /* error or EOF                 */
		{

# ifdef DEBUG
			if (tTd(15, 1))
			    (void) printf(
				"makeconnection: gethostbyname failed:\n\
    errno=%d, h_errno=%d, UseNameServer=%d\n",
				errno, h_errno, UseNameServer);
# endif DEBUG

			if (errno == ETIMEDOUT || h_errno == TRY_AGAIN)
				return (EX_TEMPFAIL);

			/* if name server is specified, assume temp fail */
			if (errno == ECONNREFUSED && UseNameServer)
				return (EX_TEMPFAIL);
			/*
			**  XXX Should look for mail forwarder record here
			**  XXX if (h_errno == NO_ADDRESS).
			*/

			return (EX_NOHOST);
		}
		(void) memcpy((char *) &SendmailAddress.sin_addr, hp->h_addr, hp->h_length);
		i = 1;
	}

	/*
	**  Determine the port number.
	*/

	if (port != 0)
		SendmailAddress.sin_port = htons(port);
	else
	{
		register struct servent *sp;

# ifdef DEBUG
	if (tTd(15, 1))
	    (void) printf("makeconnection: perform getservbyname\n");
# endif DEBUG

		sp = getservbyname("smtp", "tcp");

		if (sp == NULL)
		{
			syserr(MSGSTR(DM_UNKN, "makeconnection: server \"smtp\" unknown")); 
			return (EX_OSFILE);
		}
		SendmailAddress.sin_port = sp->s_port;
	}

	/*
	**  Try to actually open the connection.
	*/

again:
# ifdef DEBUG
	if (tTd(16, 1))
	    (void) printf("makeconnection: to %s [%s]\n", host,
		    inet_ntoa(SendmailAddress.sin_addr.s_addr));
# endif DEBUG

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		syserr(MSGSTR(DM_ESOCK3, "makeconnection: no socket")); 
		sav_errno = errno;
		goto failure;
	}

# ifdef DEBUG
	if (tTd(16, 1))
	    (void) printf("makeconnection: to socket %d\n", s);

	/* turn on network debugging? */
	if (tTd(16, 14))
	{
		int on = 1;
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);
	}
# endif DEBUG

	if (CurEnv->e_xfp != NULL)
		(void) fflush(CurEnv->e_xfp);		/* for debugging */
	errno = 0;					/* for debugging */
	SendmailAddress.sin_family = AF_INET;
	if (connect(s, &SendmailAddress, sizeof SendmailAddress) < 0)
	{
# ifdef DEBUG
		if (tTd(16, 1))
		{
		    (void) printf("makeconnection: connect failed\n");
			perror ("makeconnection");
		}
# endif DEBUG
		sav_errno = errno;
		(void) close(s);
		if (hp && hp->h_addr_list[i] && (!AltAddrLimit || AltAddrLimit > i))  /* try next address in list */
		{
			(void) memcpy((char *) &SendmailAddress.sin_addr,
			    hp->h_addr_list[i++], hp->h_length);
			goto again;
		}

		/* failure, decide if temporary or not */
	failure:
		switch (sav_errno)
		{
		  case EISCONN:
		  case ETIMEDOUT:
		  case EINPROGRESS:
		  case EALREADY:
		  case EADDRINUSE:
		  case EHOSTDOWN:
		  case ENETDOWN:
		  case ENETRESET:
		  case ENOBUFS:
		  case ECONNREFUSED:
		  case ECONNRESET:
		  case EHOSTUNREACH:
		  case ENETUNREACH:
			/* there are others, I'm sure..... */
			return (EX_TEMPFAIL);

		  case EPERM:
			/* why is this happening? */
			syserr(MSGSTR(DM_FUNNY, "makeconnection: funny failure, addr=%lx, port=%x"), SendmailAddress.sin_addr.s_addr, SendmailAddress.sin_port); 
			return (EX_TEMPFAIL);

		  default:
			message(Arpa_Info, "%s", errstring(sav_errno));
			return (EX_UNAVAILABLE);
		}
	}

# ifdef DEBUG
	if (tTd(16, 1))
	    (void) printf("makeconnection: connect OK\n");
# endif DEBUG

	/* connection ok, put it into canonical form */
	*outfile = fdopen(s, "w");
	*infile = fdopen(dup(s), "r");
	return (EX_OK);
}
/*
**  MYHOSTNAME -- return the name of this host.
**
**	Parameters:
**		hostbuf -- a place to return the name of this host.
**		size -- the size of hostbuf.
**
**	Returns:
**		A list of aliases for this host.
**
**	Side Effects:
**		none.
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	struct hostent *hp;

	if (gethostname(hostbuf, size) < 0)
	{
		(void) strcpy(hostbuf, "localhost");
	}

	hp = gethostbyname(hostbuf);
	if (hp != NULL)
	{
		(void) strcpy(hostbuf, hp->h_name);
		return (hp->h_aliases);
	}
	else
		return (NULL);
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	register struct hostent *hp;
	u_long in_addr;
	char ptr[MAXNAME], *cp;
	struct hostent *gethostbyaddr();
	long oldopts;

	/*
	**  If first character is a bracket, then it is an address
	**  lookup.  Address is copied into a temporary buffer to
	**  strip the brackets and to preserve hbuf if address is
	**  unknown.
	*/

	if (*hbuf != '[') {
		/*
		**  We first do a gethostbyname() to fetch any A records;
		**  assuming that there shouldn't be a CNAME and A record
		**  for the same hostname, if we succeed here we don't need
		**  to do the CNAME/ANY lookup in getcanonname(). And
		**  this allows us to do the DEFNAMES logic to tack on
		**  the local domain to non-fully-qualified hostnames.
		**  (If we aren't using a nameserver then getcanonname()
		**  will do a duplicate lookup.)
		*/
		(void) unquotestr (hbuf, hbuf, hbsize);
		oldopts = _res.options;
		_res.options |= RES_DEFNAMES;	/* just to make sure */
		if (! (hp = gethostbyname(hbuf)))
			getcanonname(hbuf, hbsize);
		_res.options = oldopts;
	} else {
		if ((cp = strchr((char*)strcpy(ptr, hbuf), ']')) == NULL)
			return;
		*cp = '\0';
		in_addr = inet_addr(&ptr[1]);
		hp = gethostbyaddr((char *)&in_addr, sizeof(struct in_addr),
			 AF_INET);
	}
	if (hp == NULL)
		return;
	if (strlen(hp->h_name) >= hbsize)
		hp->h_name[hbsize - 1] = '\0';
	(void)strcpy(hbuf, hp->h_name);
}

/*
**  SRC_REPLY -- send a reply to the src process
**
**	Parameters:
**		code -- return code: should be either SRC_OK or SRC_SUBMSG
**		buf -- ptr to a string to send, NULL if none
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.  NOTE that we assume the global srchdr ptr has been
**		properly initalized after reception of the src message.
*/

static void src_reply(code, buf)
int code;
char *buf;

{

register int len, offset;

    /* set up the global srcreply struct with the code and buf; NOTE that
       we assume the svrreply.objname and svrreply.objtype members
       have already been initialized */

    /* get size of header */
    offset = sizeof(srcreply) - sizeof(srcreply.svrreply.rtnmsg);

    if (buf) {
	if ((len = strlen(buf)) > sizeof(srcreply.svrreply.rtnmsg))
	    len = sizeof(srcreply.svrreply.rtnmsg);  /* get size of data */
	strncpy(srcreply.svrreply.rtnmsg, buf, len);
	len += offset;  /* adjust for header */

    } else  /* no msg */
	len = offset;

    srcreply.svrreply.rtncode = code;

# ifdef DEBUG
    if (tTd(15, 20))
	syslog(LOG_DEBUG, "src_reply: sending %d msg, %d len = '%s'",
	    srcreply.svrreply.rtncode, len, srcreply.svrreply.rtnmsg);
# endif DEBUG

    /* send the reply to src */

    srcsrpy(srchdr, &srcreply, len, END);
}

/*
**  SRC_STATS -- send mailstats to src
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.  NOTE that we assume the global srchdr ptr has been
**		properly initalized after reception of the src message.
*/

static void src_stats()

{

register char *cp;
register FILE *fp;
int pid;
char buf[BUFSIZ];


    /* fork a child to do this, so that we don't hang the daemon if
       mailstats misbehaves */
    
    if ((pid = fork()) == -1) {  /* error: notify src */
	sprintf(buf, MSGSTR(DM_FORK, "%s: cannot fork: %s\n"),
	    progname, errstring(errno));
	src_reply(SRC_SUBMSG, buf);
	return;
    }

    if (! pid) {  /* child: do our thing */
       
	/* close the smtp socket and set our process name */

	close(DaemonSocket);
	/* this messes up popen(): why????
	setproctitle(MSGSTR(DM_STAT, "%s SRC status"), progname);
	*/

	/* open a pipe to the mailstats program */
	
	if (! (fp = popen(mailstats, "r"))) {
	    sprintf(buf, MSGSTR(DM_POPEN, "%s: cannot popen: %s\n"),
		progname, errstring(errno));
	    src_reply(SRC_SUBMSG, buf);
	    return;
	}

	/* set up size of header data */

	cp = srcreply.svrreply.rtnmsg;  /* point to msg buf */
	srcreply.svrreply.rtncode = SRC_SUBMSG;

	/* read from mailstats and send its output off to src */

	while (fgets(cp, sizeof(srcreply.svrreply.rtnmsg), fp)) {

		/* send the data as a text reply to be continued */

		*(cp + strlen(cp) - 1) = '\0';  /* strip newline */
# ifdef DEBUG
		if (tTd(15, 20))
		    syslog(LOG_DEBUG, "src_stats: sending '%s'", cp);
# endif DEBUG
		srcsrpy(srchdr, &srcreply, sizeof(srcreply), CONTINUED);
	}

	/* check for read errors */

	if (ferror(fp)) {
	    sprintf(buf, MSGSTR(DM_FREAD,
		"\n%s: error reading from mailstats: %s\n"),
		progname, errstring(errno));
	    src_reply(SRC_SUBMSG, buf);

	} else {  /* finish with zero-length END packet */
# ifdef DEBUG
	    if (tTd(15, 20))
		syslog(LOG_DEBUG, "src_stats: END packet");
# endif DEBUG
	    srcreply.svrreply.rtncode = 0;  /* so it doesn't print this */
	    srcsrpy(srchdr, &srcreply, sizeof(srcreply), END);
	}

	fclose(fp);
	exit(0);  /* all done */
    }
}
/*
**  PROC_SRC -- process the src message
**
**	Parameters:
**		msg -- ptr to an srcreq struct containing the msg from src
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

static void proc_src(msg)
struct srcreq *msg;

{

int val;
static char not_supp[] = "unsupported option";

# ifdef DEBUG
    if (tTd(15, 20))
	syslog(LOG_DEBUG, "proc_src: action = %d", msg->subreq.action);
# endif DEBUG

    switch (msg->subreq.action) {

	case START :  /* not supported */

	    src_reply(SRC_SUBMSG, MSGSTR(DM_NOTSUP, not_supp));
	    break;

	case STOP :  /* terminate ourselves with varying degrees of prejudice */

	    if (msg->subreq.object == SUBSYSTEM) {
		src_reply(SRC_OK, NULL);  /* confirm ok */
		if (msg->subreq.parm1 != FORCED)  /* do it with dignity */
		    finis();
		intsig();  /* don't mess around: just like SIGTERM */

	    } else  /* not supported */
		src_reply(SRC_SUBMSG, MSGSTR(DM_NOTSUP, not_supp));

	    break;

	case STATUS :  /* send status a la mailstats */

	    if (msg->subreq.object == SUBSYSTEM)
		src_stats();
	    else  /* not supported */
		src_reply(SRC_SUBMSG, MSGSTR(DM_NOTSUP, not_supp));
	    break;

	case TRACE :  /* turn on or off SO_DEBUG on daemon's socket */

	    if (msg->subreq.object == SUBSYSTEM) {
		val = (msg->subreq.parm2 == TRACEON);  /* boolean on or off */
		if (setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, &val,
		    sizeof(val)) == -1) {
			syserr(MSGSTR(DM_SOPT, "daemon: setsockopt"));
			return;
		}
		src_reply(SRC_OK, NULL);  /* confirm ok */

	    } else  /* not supported */
		src_reply(SRC_SUBMSG, MSGSTR(DM_NOTSUP, not_supp));
	    break;

	case REFRESH :  /* re-read and freeze config file */

	    initconf();
	    src_reply(SRC_OK, NULL);  /* confirm ok */
	    break;

	default :  /* illegal cmd */
	    src_reply(SRC_SUBICMD, NULL);
	    break;
    }
}
