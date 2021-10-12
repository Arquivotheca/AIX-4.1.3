static char sccsid[] = "@(#)02	1.23.1.3  src/bos/usr/lib/sendmail/deliver.c, cmdsend, bos41J, 9513A_all 3/17/95 17:50:59";
/* 
 * COMPONENT_NAME: CMDSEND deliver.c
 * 
 * FUNCTIONS: MSGSTR, deliver, endmailer, giveresponse, logdelivery, 
 *            mailfile, markfailure, openmailer, putbody, putfromline, 
 *            sendall, sendoff 
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
 *  Sendmail
 *  Copyright (c) 1983  Eric P. Allman
 *  Berkeley, California
 *
 *  Copyright (c) 1983 Regents of the University of California.
 *  All rights reserved.  The Berkeley software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
 */


# include <signal.h>
# include <errno.h>
# include <stdio.h>
# include <fcntl.h>
# include <ctype.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <sys/ioctl.h>
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"
# include <sys/stat.h>
# include <netdb.h>

# include <string.h>
# include <sys/lockf.h>

#include <arpa/nameser.h>
#include <resolv.h>

#ifdef _CSECURITY
#include <sys/audit.h>
#endif _CSECURITY

#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

extern long  OutputCount;

void  _exit ();
void  exit ();
ADDRESS *getctladdr();
char *remotename();
char *pintvl();
STAB *stab();
char *errstring();
ADDRESS *recipient();
char *aliaslookup();
FILE  *dfopen ();
char  *xalloc ();
long  time ();
#define cur_time	time ((long *) 0)

extern char *SysExMsg[];
int N_SysEx;
extern char SmtpError[];

extern int  Sid;			/* id of forking semaphore */
extern int h_errno;

static struct sigvec sigin = { 0, 0, 0};

static char *strcat_msc();
#define PERCENT 37			/* This is the "%" (percent sign) */

/*
**  DELIVER -- Deliver a message to a list of addresses.
**
**	This routine delivers to everyone on the same host as the
**	user on the head of the list.  It is clever about mailers
**	that don't handle multiple users.  It is NOT guaranteed
**	that it will deliver to all these addresses however -- so
**	deliver should be called once for each address on the
**	list.
**
**	Parameters:
**		e -- the envelope to deliver.
**		firstto -- head of the address list to deliver to.
**
**	Returns:
**		zero -- successfully delivered.
**		else -- some failure, see ExitStat for more info.
**
**	Side Effects:
**		The standard input is passed off to someone.
*/

deliver(e, firstto)
	register ENVELOPE *e;
	ADDRESS *firstto;
{
	char *host;			/* host being sent to */
	char *user;			/* user being sent to */
	char **pvp;
	register char **mvp;
	register char *p;
	register MAILER *m;		/* mailer for this recipient */
	ADDRESS *ctladdr;
	register ADDRESS *to = firstto;
	int clever = FALSE;		/* running user smtp to this mailer */
	ADDRESS *tochain = NULL;	/* chain of users in this mailer call */
	int rcode;			/* response code */
	char *pv[MAXPV+1];
	char tobuf[MAXLINE-50];		/* text line of to people */
	char buf[MAXNAME];
	char tfrombuf[MAXNAME];		/* translated from person */
	int btype;			/* body type being sent out */

	errno = 0;
	if (bitset(QDONTSEND, to->q_flags))
		return (0);

	/* unless interactive, try twice, over a minute */
	if (OpMode == MD_DAEMON || OpMode == MD_SMTP) {
		_res.retrans = 30;
		_res.retry = 2;
	}

	m = to->q_mailer;
	host = to->q_host;
	btype = to->q_btype;

# ifdef DEBUG
	if (tTd(10, 1))
	    (void) printf("deliver: mailer=%s host=\"%s\" first user=\"%s\"\n",
					m->m_name, host, to->q_user);
# endif DEBUG

	/*
	**  If this mailer is expensive, and if we don't want to make
	**  connections now, just mark these addresses and return.
	**	This is useful if we want to batch connections to
	**	reduce load.  This will cause the messages to be
	**	queued up, and a daemon will come along to send the
	**	messages later.
	**		This should be on a per-mailer basis.
	*/

	if (NoConnect && !QueueRun && bitnset(M_EXPENSIVE, m->m_flags) &&
	    !Verbose)
	{
		for (; to != NULL; to = to->q_next)
		{
			if (bitset(QDONTSEND, to->q_flags) || to->q_mailer != m)
				continue;
			to->q_flags |= QQUEUEUP|QDONTSEND;
			e->e_to = to->q_paddr;
			message(Arpa_Info, MSGSTR(DL_QUEUED, "queued")); /*MSG*/
			if (LogLevel > 4)
				logdelivery(MSGSTR(DL_QUEUED, "queued")); /*MSG*/
		}
		e->e_to = NULL;
		return (0);
	}

	/*
	**  Do initial argv setup.
	**	Insert the mailer name.  Notice that $x expansion is
	**	NOT done on the mailer name.  Then, if the mailer has
	**	a picky -f flag, we insert it as appropriate.  This
	**	code does not check for 'pv' overflow; this places a
	**	manifest lower limit of 4 for MAXPV.
	**		The from address rewrite is expected to make
	**		the address relative to the other end.
	*/

	/* rewrite from address, using rewriting rules */
	expand("\001f", buf, &buf[sizeof buf - 1], e);
	(void) strcpy(tfrombuf, remotename(buf, m, TRUE, TRUE));

	define('g', tfrombuf, e);		/* translated sender address */
	define('h', host, e);			/* to host */
	Errors = 0;
	pvp = pv;
	*pvp++ = m->m_argv[0];

	/* insert -f or -r flag as appropriate */
	if (FromFlag && (bitnset(M_FOPT, m->m_flags) || bitnset(M_ROPT, m->m_flags)))
	{
		if (bitnset(M_FOPT, m->m_flags))
			*pvp++ = "-f";
		else
			*pvp++ = "-r";
		expand("\001g", buf, &buf[sizeof buf - 1], e);
		*pvp++ = newstr(buf);
	}

	/*
	**  Append the other fixed parts of the argv.  These run
	**  up to the first entry containing "$u".  There can only
	**  be one of these, and there are only a few more slots
	**  in the pv after it.
	*/

	for (mvp = m->m_argv; (p = *++mvp) != NULL; )
	{
		while ((p = strchr(p, '\001')) != NULL)
			if (*++p == 'u')
				break;
		if (p != NULL)
			break;

		/* this entry is safe -- go ahead and process it */
		expand(*mvp, buf, &buf[sizeof buf - 1], e);
		*pvp++ = newstr(buf);
		if (pvp >= &pv[MAXPV - 3])
		{
			syserr(MSGSTR(DL_ENPARM, "Too many parameters to %s before $u"), pv[0]); /*MSG*/
			return (-1);
		}
	}

	/*
	**  If we have no substitution for the user name in the argument
	**  list, we know that we must supply the names otherwise -- and
	**  SMTP is the answer!!
	*/

	if (*mvp == NULL)
	{
		/* running SMTP */
		clever = TRUE;
		*pvp = NULL;
	}

	/*
	**  At this point *mvp points to the argument with $u.  We
	**  run through our address list and append all the addresses
	**  we can.  If we run out of space, do not fret!  We can
	**  always send another copy later.
	*/

	tobuf[0] = '\0';
	e->e_to = tobuf;
	ctladdr = NULL;
	for (; to != NULL; to = to->q_next)
	{
		/* avoid sending multiple recipients to dumb mailers */
		if (tobuf[0] != '\0' && !bitnset(M_MUSER, m->m_flags))
			break;

		/* if already sent or not for this host, don't send */
		if (bitset(QDONTSEND, to->q_flags) ||
		    strcmp(to->q_host, host) != 0 ||
		    to->q_mailer != firstto->q_mailer)
			continue;

		/* if a different body type, don't send */
		if (btype != to->q_btype && !NlEsc)
			continue;

		/* avoid overflowing tobuf */
		if (sizeof tobuf < (strlen(to->q_paddr) + strlen(tobuf) + 2))
			break;

# ifdef DEBUG
		if (tTd(10, 1))
		{
			(void) printf("send to ");
			printaddr(to, FALSE);
		}
# endif DEBUG

		/* compute effective uid/gid when sending */
		if (to->q_mailer == ProgMailer)
			ctladdr = getctladdr(to);

		user = to->q_user;
		e->e_to = to->q_paddr;
		to->q_flags |= QDONTSEND;

		/*
		**  Check to see that these people are allowed to
		**  talk to each other.
		*/

		if (m->m_maxsize != 0 && e->e_msgsize > m->m_maxsize)
		{
			usrerr(MSGSTR(DL_ELARGE, "Message is too large; %ld bytes max"), m->m_maxsize); /*MSG*/
			NoReturn = TRUE;
			giveresponse(EX_UNAVAILABLE, m, e);
			continue;
		}
		if (!checkcompat(to))
		{
			giveresponse(EX_UNAVAILABLE, m, e);
			continue;
		}

		/*
		**  Strip quote bits from names if the mailer is dumb
		**	about them.
		*/

		if (bitnset(M_STRIPQ, m->m_flags))
		{
			stripquotes(user, TRUE);
			stripquotes(host, TRUE);
		}
		else
		{
			stripquotes(user, FALSE);
			stripquotes(host, FALSE);
		}

		/* hack attack -- delivermail compatibility */
		if (m == ProgMailer && *user == '|')
			user++;

		/*
		**  If an error message has already been given, don't
		**	bother to send to this address.
		**
		**	>>>>>>>>>> This clause assumes that the local mailer
		**	>> NOTE >> cannot do any further aliasing; that
		**	>>>>>>>>>> function is subsumed by sendmail.
		*/

		if (bitset(QBADADDR|QQUEUEUP, to->q_flags))
			continue;

		/*
		**  See if this user name is "special".
		**	If the user name has a slash in it, assume that this
		**	is a file -- send it off without further ado.  Note
		**	that this type of addresses is not processed along
		**	with the others, so we fudge on the To person.
		*/

		if (m == LocalMailer)
		{
			if (user[0] == '/')
			{
				rcode = mailfile(user, getctladdr(to), btype);
				giveresponse(rcode, m, e);
#ifdef _CSECURITY
				auditwrite(FileEvent, (rcode == EX_OK ?
				    AUDIT_OK : AUDIT_FAIL), e->e_from.q_paddr,
				    strlen(e->e_from.q_paddr) + 1, user,
				    strlen(user) + 1, NULL);
#endif _CSECURITY
				continue;
			}
		}

		/*
		**  Address is verified -- add this user to mailer
		**  argv, and add it to the print list of recipients.
		*/

		/* link together the chain of recipients */
		to->q_tchain = tochain;
		tochain = to;

		/* create list of users for error messages */
		(void) strcat(tobuf, ",");
		(void) strcat(tobuf, to->q_paddr);
		define('u', user, e);		/* to user */
		define('z', to->q_home, e);	/* user's home */

		/*
		**  Expand out this user into argument list.
		*/

		if (!clever)
		{
			expand(*mvp, buf, &buf[sizeof buf - 1], e);
			*pvp++ = newstr(buf);
			if (pvp >= &pv[MAXPV - 2])
			{
				/* allow some space for trailing parms */
				break;
			}
		}
	}

	/* see if any addresses still exist */
	if (tobuf[0] == '\0')
	{
		define('g', (char *) NULL, e);
		return (0);
	}

	/* print out messages as full list */
	e->e_to = tobuf + 1;

	/*
	**  Fill out any parameters after the $u parameter.
	*/

	while (!clever && *++mvp != NULL)
	{
		expand(*mvp, buf, &buf[sizeof buf - 1], e);
		*pvp++ = newstr(buf);
		if (pvp >= &pv[MAXPV])
			syserr(MSGSTR(DL_OFLOW, "deliver: pv overflow after $u for %s"), pv[0]); /*MSG*/
	}
	*pvp++ = NULL;

	/*
	**  Call the mailer.
	**	The argument vector gets built, pipes
	**	are created as necessary, and we fork & exec as
	**	appropriate.
	**	If we are running SMTP, we just need to clean up.
	*/

	message(Arpa_Info, MSGSTR(DL_CONN, "Connecting to %s.%s..."), host, m->m_name); /*MSG*/

	if (ctladdr == NULL)
		ctladdr = &e->e_from;
	_res.options &= ~(RES_DEFNAMES | RES_DNSRCH);		/* XXX */
	if (clever)
	{
		expand("\001j", buf, &buf[sizeof(buf) - 1], e);
		rcode = EX_OK;

		/*
		** Do nameserver query for MX (mail exchanger) records
		** if configured for it and host isn't already an address
		*/

		if (host[0] != '[' && bitnset(T_MX, NameServOpt))
		{
			Nmx = getmxrr(host, MxHosts, buf, &rcode);
		}
		else
		{
			Nmx = 1;
			MxHosts[0] = host;
		}
		if (Nmx >= 0)
		{
			message(Arpa_Info, MSGSTR(DL_CONN2, "Connecting to %s (%s)..."), MxHosts[0], m->m_name);
			if ((rcode = smtpinit(m, pv)) == EX_OK) {
				register int i;

				/* send the recipient list */
				tobuf[0] = '\0';
				for (to = tochain; to; to = to->q_tchain) {
					e->e_to = to->q_paddr;
					if ((i = smtprcpt(to, m)) != EX_OK) {
						markfailure(e, to, i);
						giveresponse(i, m, e);
					}
					else {
						(void) strcat(tobuf, ",");
						(void) strcat(tobuf, to->q_paddr);
					}
				}

				/* now send the data */
				if (tobuf[0] == '\0')
					e->e_to = NULL;
				else {
					e->e_to = tobuf + 1;
					rcode = smtpdata(m, e, btype);
				}

				/* now close the connection */
				smtpquit(m);
			}
		}
	}
	else
	{
		message(Arpa_Info, MSGSTR(DL_CONN2, "Connecting to %s (%s)..."), host, m->m_name);
		rcode = sendoff(e, m, pv, ctladdr, btype);
	}
	_res.options |= RES_DEFNAMES | RES_DNSRCH;	/* XXX */

	/*
	**  Do final status disposal.
	**	We check for something in tobuf for the SMTP case.
	**	If we got a temporary failure, arrange to queue the
	**		addressees.
	*/

	if (tobuf[0] != '\0')
	{
		giveresponse(rcode, m, e);
	}

	if (rcode != EX_OK)
	{
		for (to = tochain; to != NULL; to = to->q_tchain)
			markfailure(e, to, rcode);
	}

	errno = 0;
	define('g', (char *) NULL, e);
	return (rcode);
}
/*
**  MARKFAILURE -- mark a failure on a specific address.
**
**	Parameters:
**		e -- the envelope we are sending.
**		q -- the address to mark.
**		rcode -- the code signifying the particular failure.
**
**	Returns:
**		none.
**
**	Side Effects:
**		marks the address (and possibly the envelope) with the
**			failure so that an error will be returned or
**			the message will be queued, as appropriate.
*/

markfailure(e, q, rcode)
	register ENVELOPE *e;
	register ADDRESS *q;
	int rcode;
{

#ifdef DEBUG
	if (tTd(13, 1))
		(void) printf("markfailure: rcode=%d, address=%x\n", rcode, q);
#endif DEBUG

	if (rcode == EX_OK)
		return;
	else if (rcode != EX_TEMPFAIL && rcode != EX_IOERR && rcode != EX_OSERR)
		q->q_flags |= QBADADDR;
	else if (cur_time > e->e_ctime + TimeOut)
	{
		char buf[MAXLINE];

		if (!bitset(EF_TIMEOUT, e->e_flags))
		{
			char *hlds;
			hlds = newstr(MSGSTR(DL_ESEND, "Cannot send message for %s")); /*MSG*/
			(void) sprintf(buf, hlds, pintvl(TimeOut, FALSE));
			free(hlds);
			if (e->e_message != NULL)
				free(e->e_message);
			e->e_message = newstr(buf);
			message(Arpa_Info, buf);
		}
		q->q_flags |= QBADADDR;
		e->e_flags |= EF_TIMEOUT;
	}
	else
		q->q_flags |= QQUEUEUP;
}

/*
**  SENDOFF -- send off call to mailer & collect response.  We allow zombie
**		creation to enable collection of mailer response.  Then we
**		disallow it again.  In case of accidental zombie retention
**		due to an unexpected function exit, no real problem ensues
**		since the parent at worst will be the current process 
**		working the queue.  When it is finished, it and its zombies
**		will go away.
**
**	Parameters:
**		e -- the envelope to mail.
**		m -- mailer descriptor.
**		pvp -- parameter vector to send to it.
**		ctladdr -- an address pointer controlling the
**			user/groupid etc. of the mailer.
**
**	Returns:
**		exit status of mailer.
**
**	Side Effects:
**		none.
*/

sendoff(e, m, pvp, ctladdr, btype)
	register ENVELOPE *e;
	MAILER *m;
	char **pvp;
	ADDRESS *ctladdr;
	int btype;
{
	FILE *mfile;
	FILE *rfile;
	long  saveOutputCount;
	register int i;
	int pid;

	/*
	 *  Create connection to mailer.
	 *  Set signal processing to allow zombie for endmailer.
	 */
	sigin.sv_handler = (void (*) (int)) SIG_DFL;
	if (sigvec(SIGCLD, &sigin, (struct sigvec *) 0))
	    perror(MSGSTR(DL_ESIGCLD, "sendoff: sigvec: SIGCLD"));

	pid = openmailer(m, pvp, ctladdr, FALSE, &mfile, &rfile);
	if (pid < 0)
		return (-1);

	/*
	**  Format and send message.
	*/

	OutputCount = 0;
	putfromline(mfile, m);
	(*e->e_puthdr)(mfile, m, e, btype);
	putline("\n", mfile, m);
	(*e->e_putbody)(mfile, m, e, btype);
	saveOutputCount = OutputCount;
	(void) fclose(mfile);
	if (rfile != NULL)
		(void) fclose(rfile);

	i = endmailer(pid, pvp[0]);

	if (i == 0)
	    markstats (TRUE, m->m_mno, saveOutputCount);

	/* arrange a return receipt if requested */
	if (e->e_receiptto != NULL && bitnset(M_LOCAL, m->m_flags))
	{
		e->e_flags |= EF_SENDRECEIPT;
		/* do we want to send back more info? */
	}

	sigin.sv_handler = (void (*) (int)) SIG_IGN;
	if (sigvec(SIGCLD, &sigin, (struct sigvec *)0))
	    perror(MSGSTR(DL_ESIGCLD, "sendoff: sigvec: SIGCLD"));	/* no more zombies	*/

	return (i);
}
/*
**  ENDMAILER -- Wait for mailer to terminate.
**
**	We should never get fatal errors (e.g., segmentation
**	violation), so we report those specially.  For other
**	errors, we choose a status message (into statmsg),
**	and if it represents an error, we print it.
**
**	Parameters:
**		pid -- pid of mailer.
**		name -- name of mailer (for error messages).
**
**	Returns:
**		exit code of mailer.
**
**	Side Effects:
**		none.
*/

endmailer(pid, name)
	int pid;
	char *name;
{
	int st;

# ifdef DEBUG
	if (tTd(11, 1))
		(void) printf("endmailer\n");
# endif DEBUG

	/* in the IPC case there is nothing to wait for */
	if (pid == 0)
		return (EX_OK);

	/* wait for the mailer process to die and collect status */
# ifdef DEBUG
	if (tTd(11, 1))
	{
		(void) printf("endmailer: wait for pid %d\n", pid);
		(void) fflush(stdout);
	}
# endif DEBUG
	st = waitfor(pid);
# ifdef DEBUG
	if (tTd(11, 1))
	{
		(void) printf("endmailer: waitfor status 0x%x\n", st);
		(void) fflush(stdout);
	}
# endif DEBUG
	if (st == -1)
	{
		syserr(MSGSTR(DL_WAIT, "endmailer %s: wait"), name); /*MSG*/
		return (EX_SOFTWARE);
	}

	/* see if it died a horrid death */
	if ((st & 0377) != 0)
	{
		syserr(MSGSTR(DL_MDIED, "mailer %s died with signal %o"), name, st); /*MSG*/
		ExitStat = EX_TEMPFAIL;
		return (EX_TEMPFAIL);
	}

	/* normal death -- return status */
	st = (st >> 8) & 0377;
# ifdef DEBUG
	if (tTd(11, 1))
	{
		(void) printf("endmailer: return st 0x%x\n", st);
		(void) fflush(stdout);
	}
# endif DEBUG
	return (st);
}
/*
**  OPENMAILER -- open connection to mailer.
**
**	Parameters:
**		m -- mailer descriptor.
**		pvp -- parameter vector to pass to mailer.
**		ctladdr -- controlling address for user.
**		clever -- create a full duplex connection.
**		pmfile -- pointer to mfile (to mailer) connection.
**		prfile -- pointer to rfile (from mailer) connection.
**
**	Environment:
**		SIGCLD handling should be SIG_DFL to allow catching of children
**		terminations.  This is assumed.
**
**	Returns:
**		pid of mailer ( > 0 ).
**		-1 on error.
**		zero on an IPC connection.
**
**	Side Effects:
**		forks to create a mailer in a subprocess.
*/

openmailer(m, pvp, ctladdr, clever, pmfile, prfile)
	MAILER *m;
	char **pvp;
	ADDRESS *ctladdr;
	int clever;
	FILE **pmfile;
	FILE **prfile;
{
	int pid, fd;
	int mpvect[2];
	int rpvect[2];
	FILE *mfile = NULL;
	FILE *rfile = NULL;

/* # ifdef DEBUG
	if (tTd(11, 1))
	{
		(void) printf("openmailer: mailer='%s', pvp=", m->m_mailer);
		printav(pvp);
	}
# endif DEBUG  */
	errno = 0;

	CurHostName = m->m_mailer;

	/*
	**  Deal with the special case of mail handled through an IPC
	**  connection.
	**	In this case we don't actually fork.  We must be
	**	running SMTP for this to work.  We will return a
	**	zero pid to indicate that we are running IPC.
	**  We also handle a debug version that just talks to stdin/out.
	*/

#ifdef DEBUG
	/* check for Local Person Communication -- not for mortals!!! */
	if (strcmp(m->m_mailer, "[LPC]") == 0)
	{
		*pmfile = stdout;
		*prfile = stdin;
		return (0);
	}
#endif DEBUG

	if (strcmp(m->m_mailer, "[IPC]") == 0)
	{
		register STAB *st;
		register int i, j;
		register unsigned short port;

		CurHostName = pvp[1];
		if (!clever)
			syserr(MSGSTR(DL_NOCLEVER, "non-clever IPC")); /*MSG*/
		if (pvp[2] != NULL)
			port = atoi(pvp[2]);
		else
			port = 0;
		for (j = 0; j < Nmx; j++)
		{
			CurHostName = MxHosts[j];
			/* see if we have already determined that this host
			   is fried */
			st = stab(MxHosts[j], ST_HOST, ST_FIND);
			if (st == NULL || st->s_host.ho_exitstat == EX_OK) {
				if (j > 1)
					message(Arpa_Info, MSGSTR(DL_CONN2, "Connecting to %s (%s)..."), MxHosts[j], m->m_name);
				i = makeconnection(MxHosts[j], port, pmfile, prfile);
			}
			else
			{
				i = st->s_host.ho_exitstat;
				errno = st->s_host.ho_errno;
			}
			if (i != EX_OK)
			{
				/* enter status of this host */
				if (st == NULL)
					st = stab(MxHosts[j], ST_HOST, ST_ENTER);
				st->s_host.ho_exitstat = i;
				st->s_host.ho_errno = errno;
				ExitStat = i;
				continue;
			}
			else
				return (0);
		}
		return (-1);
	}

	/* create a pipe to shove the mail through */
	if (pipe(mpvect) < 0)
	{
		syserr(MSGSTR(DL_EPIPE, "openmailer: pipe (to mailer)")); /*MSG*/
		return (-1);
	}

	/* if this mailer speaks smtp, create a return pipe */
	if (clever && pipe(rpvect) < 0)
	{
		syserr(MSGSTR(DL_EPIPE2, "openmailer: pipe (from mailer)")); /*MSG*/
		(void) close(mpvect[0]);
		(void) close(mpvect[1]);
		return (-1);
	}

	/*
	**  Actually fork the mailer process.
	*/

# ifdef DEBUG
	if (tTd(11, 1))
		(void) printf("openmailer: prepare to fork\n");
# endif DEBUG

	if (CurEnv->e_xfp != NULL)
		(void) fflush(CurEnv->e_xfp);		/* for debugging */
	(void) fflush(stdout);
	/*
	 *  BSD code tried to force fork here even though there's lots of code
	 *  to handle failure.  Can mail be dropped if fork fails.  Is there
	 *  a better way?
	 */
	pid = dfork (10);
	if (pid < 0)
	{
		/* failure */
# ifdef DEBUG
		if (tTd(11, 1))
		{
			(void) printf("openmailer: fork failed\n");
			if (CurEnv->e_xfp != NULL)
			       (void) fflush(CurEnv->e_xfp);
		       (void) fflush(stdout);
		}
# endif DEBUG
		syserr(MSGSTR(DL_EFORK, "openmailer: cannot fork")); /*MSG*/
		(void) close(mpvect[0]);
		(void) close(mpvect[1]);
		if (clever)
		{
			(void) close(rpvect[0]);
			(void) close(rpvect[1]);
		}
		return (-1);
	}
	else if (pid == 0)
	{
		int i;

		/*
		 *  We must immediately close the message catalog file
		 *  and zero its file descriptor so that we get our own
		 *  file buffer when we access messages (otherwise we
		 *  mess up the parent's).
		 */
		/* 
		 *  catgets() was fixed in version 3.2 such that it checks
		 *  the pid of it's invoker and if different, it reopens
		 *  the catalog. Therefore, we no longer need to close it.
		 *
		 * catclose(catd);
		 */

		/* child -- set up input & exec mailer */
		/* make diagnostic output be standard output */
		sigin.sv_handler = (void (*) (int)) SIG_IGN;
		if(sigvec(SIGINT, &sigin, (struct sigvec *) 0))
		    perror(MSGSTR(DL_OSIGINT, "openmailer: sigvec: SIGINT"));
		if(sigvec(SIGHUP, &sigin, (struct sigvec *) 0))
		    perror(MSGSTR(DL_OSIGHUP, "openmailer: sigvec: SIGHUP"));
		sigin.sv_handler = (void (*) (int)) SIG_DFL;
		if(sigvec(SIGTERM, &sigin, (struct sigvec *) 0))
		    perror(MSGSTR(DL_OSIGTERM, "openmailer: sigvec: SIGTERM"));

		/* arrange to filter standard & diag output of command */
		if (clever)
		{
			(void) close(rpvect[0]);
			(void) close(1);
			(void) dup(rpvect[1]);
			(void) close(rpvect[1]);
		}
		else if (OpMode == MD_SMTP || HoldErrs)
		{
			/* put mailer output in transcript */
			(void) close(1);
			(void) dup(fileno(CurEnv->e_xfp));
		}
		(void) close(2);
		(void) dup(1);

/* arrange to get standard input */
		(void) close(mpvect[1]);
		(void) close(0);
		if (dup(mpvect[0]) < 0)
		{
			syserr(MSGSTR(DL_EDUP, "Cannot dup to zero!")); /*MSG*/
			_exit(EX_OSERR);
		}
		(void) close(mpvect[0]);

		/*
		 * disassociate from the controlling tty -- defect 141515
		 */
		if((fd = open("/dev/tty", O_RDONLY)) >= 0)
		{
		    ioctl(fd, TIOCNOTTY, 0);
		    close(fd);
		}

		/*
		 *  We are effectively root.  However, if mailer need not
		 *  be root to execute, turn us into another user.
		 *
		 *  NOTE:  After we are another user, file access permissions
		 *  for syslog will probably not permit us to log an error to
		 *  to the log file in QueueDir!  In that case, syslog messages
		 *  will appear on /dev/console.  Syserr will log to the user
		 *  terminal if attached to the process (disconnect not called).
		 *  Syserr will also attempt a syslog.
		 */
		if (!bitnset(M_RESTR, m->m_flags))
		{
/**/			message (0, MSGSTR(DL_GID, "000 openmailer: DefUid %d, DefGid %d"),  DefUid, DefGid); /*MSG*/

			/*
			 *  If ctladdr not given or if receiver is root
			 *  (or unknown?), then set to default uid/gid.
			 *  else, set to receiver uid/gid.
			 */
			if (ctladdr == NULL || 
					!bitset (QGOODUID, ctladdr->q_flags))
			{
/**/				message (0, MSGSTR(DL_DGID, "000 openmailer: set default uid/gid = %d/%d"), DefUid, DefGid); /*MSG*/
				if (setgid(DefGid) < 0 || 
				    initgroups(DefUser, DefGid) ||
				    setuid (DefUid) < 0   )
				{
				    syserr (MSGSTR(DL_EGID, "Can't set gid/uid")); /*MSG*/
				    _exit (EX_OSERR);
				}
			}
			else
			{
/**/				message (0, MSGSTR(DL_CTLADDR, "000 openmailer: set ctladdr uid/gid %d/%d"),  ctladdr->q_uid, ctladdr->q_gid); /*MSG*/
				if (setgid (ctladdr->q_gid) < 0 || 
				    initgroups(ctladdr->q_ruser ?
					ctladdr->q_ruser: ctladdr->q_user,
					ctladdr->q_gid) ||
				    setuid (ctladdr->q_uid) < 0    )
				{
				    syserr (MSGSTR(DL_EGID, "Can't set gid/uid")); /*MSG*/
				    _exit (EX_OSERR);
				}
			}
		}

		/*
		 *  Set close-on-exec so that a successful exec will
		 *  close all the open files.  An unsuccessful exec
		 *  will leave them open for logging purposes.
		 *  NOTE:  closing them outright takes < 0.1 sec.
		 */
		/*
		 *  The syserr may or may not create a visible message to
		 *  the user.  However, it will also attempt a syslog.
		 *  The latter will either successfully log to the log file,
		 *  or it will appear on the console.
		 */
		for (i = 3; i < MAXFDESC; i++)
		{
		    if (fcntl (i, F_SETFD, 1) == -1)
		    {
			if (errno != EBADF)
			    syserr (MSGSTR(DL_EFCNTL, "fcntl error")); /* log only, not fatal */ /*MSG*/
		    }
		}

		/*
		 *  Load the mailer into this process.
		 *  If it's successful, start it;
		 *  else, continue.
		 */
/**/		message (0, MSGSTR(DL_EXECVE, "000 execve: uid = %d, gid = %d"), getuid (), getgid ()); /*MSG*/
		(void) execve(m->m_mailer, pvp, UserEnviron);

		/*
		 *  The syserr may or may not create a visible message to
		 *  the user.  However, it will also attempt a syslog.
		 *  The latter will either successfully log to the log file,
		 *  or it will appear on the console.
		 */
		syserr(MSGSTR(DL_EEXEC, "Cannot exec %s"), m->m_mailer); /*MSG*/

		/*
		 *  We may now queue or reject the mail.  The BSD software
		 *  would always queue it if it were for the local mailer, or
		 *  if certain temporary system conditions prevented exec'ing
		 *  the mailer.
		 *
		 *  The only way to reliably and promptly notify an originator 
		 *  that his mail has failed is to mail him a reply (error mode
		 *  EM_MAIL).  However, this is not always possible.  In the
		 *  most common case mail is entirely local, and to mail a
		 *  response requires the same mailer that has just failed
		 *  being exec'd!  Furthermore, if mail from any source is so 
		 *  close to being delivered that only a local mailer failure 
		 *  stands in the way, don't throw it away.  Let something be 
		 *  done on the local machine to allow delivery!  Therefore, it
		 *  is understandable that BSD software queued the mail 
		 *  unconditionally in the case that the local mailer failed.  
		 *  We will do the same.
		 *
		 *  In the case of other mailers, we will queue the mail only
		 *  if some temporary system problem has prevented exec'ing the
		 *  mailer.  However, our list will be tailored to AIX.
		 */
		if (m == LocalMailer || errno == EIO    || errno == EAGAIN ||
					errno == ENOMEM || errno == ETXTBSY)
			_exit(EX_TEMPFAIL);
		else
			_exit(EX_UNAVAILABLE);
	}

	/*
	**  Set up return value.
	*/

	(void) close(mpvect[0]);
	mfile = fdopen(mpvect[1], "w");
	if (clever)
	{
		(void) close(rpvect[1]);
		rfile = fdopen(rpvect[0], "r");
	}
	else
		rfile = NULL;

	*pmfile = mfile;
	*prfile = rfile;

# ifdef DEBUG
	if (tTd(11, 1))
	{
		(void) printf("openmailer: return pid %d\n", pid);
		(void) fflush(stdout);
	}
# endif DEBUG

	return (pid);
}
/*
**  GIVERESPONSE -- Interpret an error response from a mailer
**
**	Parameters:
**		stat -- the status code from the mailer (high byte
**			only; core dumps must have been taken care of
**			already).
**		m -- the mailer descriptor for this mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Errors may be incremented.
**		ExitStat may be set.
*/

giveresponse(stat, m, e)
	int stat;
	register MAILER *m;
	ENVELOPE *e;
{
	register char *statmsg;
	register int i;
	char buf[MAXLINE];

	/*
	**  Compute status message from code.
	*/

	i = stat - EX__BASE;
	if (stat == 0)
		statmsg = MSGSTR(DL_SENT, "250 Sent"); /*MSG*/
	else if (i < 0 || i > N_SysEx)
	{
		(void) sprintf(buf, MSGSTR(DL_EMAILER, "554 unknown mailer error %d"), stat); /*MSG*/
		stat = EX_UNAVAILABLE;
		statmsg = buf;
	}
	else if (stat == EX_TEMPFAIL)
	{
		(void) strcpy(buf, SysExMsg[i]);
		if (h_errno == TRY_AGAIN)
		{
			statmsg = errstring(h_errno+MAX_ERRNO);
		}
		else
		{
			if (errno != 0)
			{
				statmsg = errstring(errno);
			}
			else
			{
				statmsg = SmtpError;
			}
		}
		if (statmsg != NULL && statmsg[0] != '\0')
		{
			(void) strcat(buf, ": ");
			/* Call the special strcat() routine with Message
			 * Safety Check, that is strcat_msc().
			 */
			(void) strcat_msc(buf, statmsg);
		}
		statmsg = buf;
	}
	else
	{
		statmsg = SysExMsg[i];
	}

	/*
	**  Print the message as appropriate
	*/

	if (stat == EX_OK || stat == EX_TEMPFAIL)
		message(Arpa_Info, &statmsg[4]);
	else
	{
		Errors++;
		usrerr(statmsg);
	}

	/*
	**  Final cleanup.
	**	Log a record of the transaction.  Compute the new
	**	ExitStat -- if we already had an error, stick with
	**	that.
	*/

	if (LogLevel > ((stat == 0 || stat == EX_TEMPFAIL) ? 3 : 2))
		logdelivery(&statmsg[4]);

	if (stat != EX_TEMPFAIL)
		setstat(stat);
	if (stat != EX_OK)
	{
		if (e->e_message != NULL)
			free(e->e_message);
		e->e_message = newstr(&statmsg[4]);
	}
	errno = 0;
	h_errno = 0;
}

/* strcat_msc(): This is a special version of strcat() with Message
 *                Safety Check for the string "s2".
 * Right now, we check for "%" (percent sign). If "%" is presented
 * in string "s2", add an extra "%" immediately after the original
 * one, so sendmail won't core dump with segmentation violation
 * problem. Please see defect 173870 for details.
 * If we find more harmful sombol(s) in the receiving third party
 * system mailer's error message, this is a good place to check and
 * disarm it.
 */
static char * strcat_msc(char *s1, const char *s2)
{
	char *os1;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s2)
	{
		if( *s2 == PERCENT )
			*s1++ = PERCENT;
		*s1++ = *s2++;
	}
	return(os1);
}
/*
**  LOGDELIVERY -- log the delivery in the system log
**
**	Parameters:
**		stat -- the message to print for the status
**
**	Returns:
**		none
**
**	Side Effects:
**		none
*/

logdelivery(stat)
	char *stat;
{
# ifdef LOG
	char *hlds1, *hlds2;

	hlds1 = newstr(stat);
	hlds2 = newstr(MSGSTR(DL_STAT, "%s: to=%s, delay=%s, stat=%s"));/*MSG*/
	syslog(LOG_INFO, hlds2, CurEnv->e_id,
	       CurEnv->e_to, pintvl(cur_time - CurEnv->e_ctime, TRUE), hlds1);
	free(hlds1);
	free(hlds2);
# endif LOG
}
/*
**  PUTFROMLINE -- output a UNIX-style from line (or whatever)
**
**	This can be made an arbitrary message separator by changing $l
**
**	One of the ugliest hacks seen by human eyes is contained herein:
**	UUCP wants those stupid "remote from <host>" lines.  Why oh why
**	does a well-meaning programmer such as myself have to deal with
**	this kind of antique garbage????
**
**	Parameters:
**		fp -- the file to output to.
**		m -- the mailer describing this entry.
**
**	Returns:
**		none
**
**	Side Effects:
**		outputs some text to fp.
*/

putfromline(fp, m)
	register FILE *fp;
	register MAILER *m;
{
	char *template = "\001l\n";
	char buf[MAXLINE];

	if (bitnset(M_NHDR, m->m_flags))
		return;

	if (bitnset(M_UGLYUUCP, m->m_flags))
	{
		char *bang;
		char xbuf[MAXLINE];

		expand("\001g", buf, &buf[sizeof buf - 1], CurEnv);
		bang = strchr(buf, '!');
		if (bang == NULL)
			syserr(MSGSTR(DL_ENOBANG, "No ! in UUCP! (%s)"), buf); /*MSG*/
		else
		{
			*bang++ = '\0';
			(void) sprintf(xbuf, MSGSTR(DL_FROM, "From %s  \001d remote from %s\n"), bang, buf); /*MSG*/
			template = xbuf;
		}
	}
	expand(template, buf, &buf[sizeof buf - 1], CurEnv);

	putline(buf, fp, m);
}
/*
**  PUTBODY -- put the body of a message.
**
**	Parameters:
**		fp -- file to output onto.
**		m -- a mailer descriptor to control output format.
**		e -- the envelope to put out.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The message is written onto fp.
*/

putbody(fp, m, e, btype)
	FILE *fp;
	MAILER *m;
	register ENVELOPE *e;
	int btype;
{
	char buf[MAXLINE];
	char *tif;
	FILE *tifp;
	char *hlds;
	char *ic;

	/* do this in case error or NLS and KJI not defined */
	tif  = e->e_df;
	tifp = e->e_dfp;

	/*
	**  Determine the body type, create a new body type if necessary
	**  and set the input file pointer (tifp).
	*/
	switch(btype)  {
	case BT_UNK:	/* if type unknown, simply send out regular temp file */
	    break;
	case BT_NLS:	/* this is a local user */
	    if (e->e_dfn == NULL)
		make_newbody(btype, e);
	    tif  = e->e_dfn;
	    tifp = e->e_dfnp;
	    break;
	case BT_ESC: 	/* going out to another NLS system */
	    if (e->e_dfe == NULL)
		make_newbody(btype, e);
	    tif  = e->e_dfe;
	    tifp = e->e_dfep;
	    break;
	case BT_ISO:	/* going out with ISO 8859/1 characters */
	    if (e->e_dfi == NULL)
		make_newbody(btype, e);
	    tif  = e->e_dfi;
	    tifp = e->e_dfip;
	    break;
	case BT_FLAT:	/* going out with flat ASCII text */
	    if (e->e_dff == NULL)
		make_newbody(btype, e);
	    tif  = e->e_dff;
	    tifp = e->e_dffp;
	    break;
	case BT_MC:	/* this is a local user */
	    if (e->e_dfs == NULL)
		make_newbody(btype, e);
	    tif  = e->e_dfs;
	    tifp = e->e_dfsp;
	    break;
	case BT_NC: 	/* going out to another system */
	    if (e->e_dfj == NULL)
		make_newbody(btype, e);
	    tif  = e->e_dfj;
	    tifp = e->e_dfjp;
	    break;
	}

#ifdef DEBUG
	if (tTd(66, 10)) {
	    struct stat sbuf;
	    int i;
		i = stat(tif, &sbuf);
		(void) printf("putbody: bodytype=%d file=%s, size=%ld\n",
		    btype, tif, (i ? -1L : (long) sbuf.st_size));
	}
#endif DEBUG


	/*
	**  Output the body of the message
	*/

	if (tifp == NULL)
	{
		if (tif != NULL)
		{
			tifp = fopen(tif, "r");
			if (tifp == NULL)
				syserr(MSGSTR(DL_EOPEN, "putbody: Cannot open %s for %s from %s"), tif, e->e_to, e->e_from); /*MSG*/
		}
		else
		{
			hlds = newstr(MSGSTR(DL_NOMSG,
			    "<<< No Message Collected >>>"));
			if (putline(hlds, fp, m) == 0 && strcmp(SmtpPhase, "DATA wait") == 0 && TransmitTimeout) 
				return;
			free(hlds);
		}
	}
	if (tifp != NULL)
	{
		rewind(tifp);
		while ((ic=fgetc(tifp)) != EOF && ((char)ic != '\0'));
		rewind(tifp);
		if (ic == NULL) {
			while ((ic=fgetc(tifp)) != EOF)
				fputc(ic, fp);
		}
		else {
			while (!ferror(fp) && fgets(buf, sizeof buf, tifp) != NULL)
			{
#ifdef DEBUG
			if (tTd(66, 20))
				(void) printf("putbody: got string='%s'", buf);
#endif DEBUG

				if (buf[0] == 'F' && bitnset(M_ESCFROM, m->m_flags) &&
				    strncmp(buf, "From ", 5) == 0)
					(void) putc('>', fp);
				if (putline(buf, fp, m) == 0 && strcmp(SmtpPhase, "DATA wait") == 0 && TransmitTimeout)
					break;
			}

			if (ferror(tifp))
			{
				syserr(MSGSTR(DL_EPUTBODY, "putbody: read error")); /*MSG*/
				ExitStat = EX_IOERR;
			}
		}
	}

	(void) fflush(fp);
	if (ferror(fp) && errno != EPIPE)
	{
		syserr(MSGSTR(DL_EPUTBODY2, "putbody: write error")); /*MSG*/
		ExitStat = EX_IOERR;
	}
	errno = 0;
}
/*
**  MAILFILE -- Send a message to a file.
**
**	If the file has the setuid/setgid bits set, but NO execute
**	bits, sendmail will try to become the owner of that file
**	rather than the real user.  Obviously, this only works if
**	sendmail runs as root.
**
**	This could be done as a subordinate mailer, except that it
**	is used implicitly to save messages in ~/dead.letter.  We
**	view this as being sufficiently important as to include it
**	here.  For example, if the system is dying, we shouldn't have
**	to create another process plus some pipes to save the message.
**
**	Parameters:
**		filename -- the name of the file to send to.
**		ctladdr -- the controlling address header -- includes
**			the userid/groupid to be when sending.
**		btype -- the body type of the message
**
**	Returns:
**		The exit code associated with the operation.
**
**	Side Effects:
**		none.
*/

mailfile(filename, ctladdr, btype)
	char *filename;
	ADDRESS *ctladdr;
	int btype;
{
	register FILE *f;
	register int pid;
	ENVELOPE *e = CurEnv;

	/*
	**  Fork so we can change permissions here.
	**	Note that we MUST use fork, not vfork, because of
	**	the complications of calling subroutines, etc.
	*/

	pid = dfork(10);

	if (pid < 0)
		return (EX_OSERR);
	else if (pid == 0)
	{
		/* child -- actually write to file */
		struct stat stb;

		(void) signal(SIGINT, SIG_DFL);
		(void) signal(SIGHUP, SIG_DFL);
		(void) signal(SIGTERM, SIG_DFL);
		(void) umask(OldUmask);
		if (stat(filename, &stb) < 0)
		{
			errno = 0;
			stb.st_mode = 0666;
		}
		if (bitset(0111, stb.st_mode))
			exit(EX_CANTCREAT);
		if (ctladdr == NULL)
			ctladdr = &e->e_from;
		/* we have to open the dfile BEFORE setuid */
		if (e->e_dfp == NULL &&  e->e_df != NULL)
		{
			e->e_dfp = fopen(e->e_df, "r");
			if (e->e_dfp == NULL) {
				syserr("mailfile: Cannot open %s for %s from %s",
				e->e_df, e->e_to, e->e_from);
			}
		}

		if (!bitset(S_ISGID, stb.st_mode) || setgid(stb.st_gid) < 0)
		{
			if (ctladdr->q_uid == 0) {
				(void) setgid(DefGid);
				(void) initgroups(DefUser, DefGid);
			} else {
				(void) setgid(ctladdr->q_gid);
				(void) initgroups(ctladdr->q_ruser?
					ctladdr->q_ruser: ctladdr->q_user,
					ctladdr->q_gid);
			}
		}
		if (!bitset(S_ISUID, stb.st_mode) || setuid(stb.st_uid) < 0)
		{
			if (ctladdr->q_uid == 0)
				(void) setuid(DefUid);
			else
				(void) setuid(ctladdr->q_uid);
		}
		f = dfopen(filename, "a");
		if (f == NULL)
			exit(EX_CANTCREAT);

		putfromline(f, ProgMailer);
		(*CurEnv->e_puthdr)(f, ProgMailer, CurEnv, btype);
		putline("\n", f, ProgMailer);
		(*CurEnv->e_putbody)(f, ProgMailer, CurEnv, btype);
		putline("\n", f, ProgMailer);
		(void) fclose(f);
		(void) fflush(stdout);

		/* reset ISUID & ISGID bits for paranoid systems */
		(void) chmod(filename, (int) stb.st_mode);
		exit(EX_OK);
		/*NOTREACHED*/
	}
	else
	{
		/* parent -- wait for exit status */
		int st;

		st = waitfor(pid);
		if ((st & 0377) != 0)
			return (EX_UNAVAILABLE);
		else
			return ((st >> 8) & 0377);
		/*NOTREACHED*/
	}
}
/*
**  SENDALL -- actually send all the messages.
**
**	Parameters:
**		e -- the envelope to send.
**		mode -- the delivery mode to use.  If SM_DEFAULT, use
**			the current SendMode.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Scans the send lists and sends everything it finds.
**		Delivers any appropriate error messages.
**		If we are running in a non-interactive mode, takes the
**			appropriate action.
**		If delivery mode is background, this will attempt to fork.
*/

sendall(e, mode)
	ENVELOPE *e;
	char mode;
{
	register ADDRESS *q;
	int oldverbose;
	int pid;

	/* determine actual delivery mode */
	if (mode == SM_DEFAULT)
	{
		if (shouldqueue(e->e_msgpriority))
			mode = SM_QUEUE;
		else
			mode = SendMode;
	}

#ifdef DEBUG
	if (tTd(13, 1))
	{
		(void) printf("\nSENDALL: mode %c, sendqueue:\n", mode);
		printaddr(e->e_sendqueue, TRUE);
	}
#endif DEBUG

	/*
	**  Do any preprocessing necessary for the mode we are running.
	**	Check to make sure the hop count is reasonable.
	**	Delete sends to the sender in mailing lists.
	*/

	CurEnv = e;

	if (e->e_hopcount > MAXHOP)
	{
		errno = 0;
		syserr(MSGSTR(DL_EHOPS, "sendall: too many hops %d (%d max): from %s, to %s"), e->e_hopcount, MAXHOP, e->e_from, e->e_to); /*MSG*/
		return;
	}

	/*
	**  Adjust the file pointers in the env structure to indicate the
	**  body types that are available to send and to allow any conversions
	**  that have to be done.
	*/
	if (e->e_btype == BT_NLS)  {
	    e->e_dfn = e->e_df;
	    e->e_dfnp = e->e_dfp;
	} else if (e->e_btype == BT_ESC)  {
	    e->e_dfe = e->e_df;
	    e->e_dfep = e->e_dfp;
        } else  if (e->e_btype == BT_NC)  {
            e->e_dfj = e->e_df;
            e->e_dfjp = e->e_dfp;
        } else  if (e->e_btype == BT_MC)  {
            e->e_dfs = e->e_df;
            e->e_dfsp = e->e_dfp;
        }

	if (!MeToo)
	{
		e->e_from.q_flags |= QDONTSEND;
#ifdef DEBUG
		if (tTd(13, 1))
			(void) printf("\nSENDALL: mode %c, recip\n", mode);
#endif DEBUG
		(void) recipient(&e->e_from, &e->e_sendqueue);
	}

	if ((mode == SM_QUEUE || mode == SM_FORK ||
	     (mode != SM_VERIFY && SuperSafe)) &&
	    !bitset(EF_INQUEUE, e->e_flags))
		queueup(e, TRUE, mode == SM_QUEUE);

#ifdef DEBUG
	if (tTd(13, 1))
		(void) printf("\nSENDALL: mode %c, place 1, btype %d\n"
				, mode, e->e_btype);
#endif DEBUG

	oldverbose = Verbose;
	switch (mode)
	{
	  case SM_VERIFY:
		Verbose = TRUE;
		break;

	  case SM_QUEUE:
		e->e_flags |= EF_INQUEUE|EF_KEEPQUEUE;
		return;

	  case SM_FORK:
		if (e->e_xfp != NULL)
			(void) fflush(e->e_xfp);

		/*
		 *  We already have pulled a count from the clean semaphore.
		 *  Pull another count for our child.  Don't adjust our 
		 *  semadj factor for our process.  This means that when we 
		 *  die or exit, the semaphore count will be adjusted up
		 *  only by one, and our indication of queue usage won't
		 *  go away.  This prevents an inadvertent queue cleaning
		 *  operation taking place while we are forking.
		 *  The child must take responsibility for the semaphore
		 *  count due to us.  There are certain failures which
		 *  can cause the semaphore to be left permanently mis-
		 *  adjusted.  This is normally not fatal to sendmail
		 *  operations:  However, no more queue cleaning actions
		 *  will take place until after the next reboot or until
		 *  a new semaphore is allocated.
		 *
		 *  Conceivably, if that error happened often enough, the 
		 *  semaphore count would be exhausted and at some point 
		 *  sendmail would stall waiting on the semaphore.  There 
		 *  may need to be some way to prevent a complete stall, in 
		 *  this case.  If such code has been implemented, it will
		 *  probably be in main.c!
	 	 *
	 	 *  All exceptions cause a syserr, but don't otherwise interfere
	 	 *  with sendmail operation.  We assume that a failure here 
		 *  means that semaphore ops in orderq and elsewhere will also
		 *  fail, and no queue cleaning will be attempted.
		 */
		(void) semwait (Sid, 1, 0, 1);

#ifdef DEBUG
		if (tTd (13, 111))		/* fake kill for sema4 test */
		    exit (99);
#endif DEBUG

		pid = fork();
		if (pid < 0)			/* no resources, failed	 */
		{
			mode = SM_DELIVER;
			break;
		}
		else if (pid > 0)
		{
			/* be sure we leave the temp files to our child */
			e->e_id = e->e_df = NULL;
			return;
		}

		/*
		 *  CHILD
		 */

		/*
		 *  We are responsible for our queue usage semaphore.
		 *  This semaphore already has our main "wait" pulled from it.
		 *  However, our semadj value is zero (new process).
		 *  The way we take responsibility for it is to adjust
		 *  our semadj value so that when we exit or die, the
		 *  semaphore will be adjusted properly.  This is done
		 *  by performing a wait with semadj change, followed
		 *  by a signal without it.
	 	 *
	 	 *  All exceptions cause a syserr, but don't otherwise interfere
	 	 *  with sendmail operation.  We assume that any failure of a
		 *  semaphore operation means that all future operations will
		 *  also fail, in orderq and elsewhere.  No queue cleaning
		 *  will be attempted.
		 */
#ifdef DEBUG
		if (tTd (13, 110))		/* sema4 testing */
		    exit (99);
#endif DEBUG
		(void) semwait (Sid, 1, 0, 0);
		(void) semsig  (Sid, 1, 1);
#ifdef DEBUG
		if (tTd (13, 109))		/* sema4 testing */
		    exit (99);
#endif DEBUG
		/*
		 *  Disconnect our IO from the terminal.  Inhibit response
		 *  to SIGINT, SIGHUP, SIGQUIT.  But don't change process grp.
		 */
		disconnect (FALSE);

		break;
	}

#ifdef DEBUG
	if (tTd(13, 1))
		(void) printf("\nSENDALL: mode %c, place 2\n", mode);
#endif DEBUG

	/*
	**  Run through the list and send everything.
	*/

	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{

#ifdef DEBUG
		if (tTd(13, 1))
		    (void) printf("\nSENDALL: mode %c, q = 0x%x\n", mode, q);
#endif DEBUG

		if (mode == SM_VERIFY)
		{
			e->e_to = q->q_paddr;
			if (!bitset(QDONTSEND|QBADADDR, q->q_flags))
				message(Arpa_Info, MSGSTR(DL_DELIVERABLE, "deliverable")); /*MSG*/
		}
		else
		{
#ifdef DEBUG
			if (tTd(13, 1))
			   (void) printf("\nSENDALL: mode %c, deliver\n", mode);
#endif DEBUG
			(void) deliver(e, q);
		}
	}
	Verbose = oldverbose;

	/*
	**  Now run through and check for errors.
	*/

	if (mode == SM_VERIFY)
		return;

	for (q = e->e_sendqueue; q != NULL; q = q->q_next)
	{
		register ADDRESS *qq;

# ifdef DEBUG
		if (tTd(13, 3))
		{
			(void) printf("Checking ");
			printaddr(q, FALSE);
		}
# endif DEBUG

		/* only send errors if the message failed */
		if (!bitset(QBADADDR, q->q_flags))
			continue;
		/* if following is true, we have failed on
		 * returntosender. Queue message for delivery
		 * on next run. If this is not done message
		 * is LOST.
		*/ 
		if (bitset(EF_RESPONSE,e->e_flags))
		{
			q->q_flags |= QQUEUEUP;
			break;
		}
		/* we have an address that failed -- find the parent */
		for (qq = q; qq != NULL; qq = qq->q_alias)
		{
			char obuf[MAXNAME + 6];

			/* we can only have owners for local addresses */
			if (!bitnset(M_LOCAL, qq->q_mailer->m_flags))
				continue;

			/* see if the owner list exists */
			(void) strcpy(obuf, "owner-");
			if (strncmp(qq->q_user, "owner-", 6) == 0)
				(void) strcat(obuf, "owner");
			else
				(void) strcat(obuf, qq->q_user);
			if (!bitnset(M_USR_UPPER, qq->q_mailer->m_flags))
				makelower(obuf);
			if (aliaslookup(obuf) == NULL)
				continue;

# ifdef DEBUG
			if (tTd(13, 4))
				(void) printf("Errors to %s\n", obuf);
# endif DEBUG

			/* owner list exists -- add it to the error queue */
			sendtolist(obuf, (ADDRESS *) NULL, &e->e_errorqueue);
			ErrorMode = EM_MAIL;
			break;
		}

		/* if we did not find an owner, send to the sender */
		if (qq == NULL && bitset(QBADADDR, q->q_flags))
			sendtolist(e->e_from.q_paddr, qq, &e->e_errorqueue);
	}
	
	if (mode == SM_FORK)
		finis();
}
