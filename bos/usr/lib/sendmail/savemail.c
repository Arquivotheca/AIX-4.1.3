static char sccsid[] = "@(#)22	1.9.2.2  src/bos/usr/lib/sendmail/savemail.c, cmdsend, bos41J, 9508A 2/20/95 11:09:56";
/* 
 * COMPONENT_NAME: CMDSEND savemail.c
 * 
 * FUNCTIONS: MSGSTR, errbody, returntosender, savemail 
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

# include <pwd.h>
# include <stdio.h>
# include <ctype.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/stat.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"

extern long  OutputCount;

void  exit ();
char *ttypath();
int putheader(), errbody();
ENVELOPE *newenvelope();
FILE  *dfopen ();
char  *queuename ();
ADDRESS  *parseaddr ();
char *strcpy();

/*
**  SAVEMAIL -- Save mail on error
**
**	If mailing back errors, mail it back to the originator
**	together with an error message; otherwise, just put it in
**	dead.letter in the user's home directory (if he exists on
**	this machine).
**
**	Parameters:
**		e -- the envelope containing the message in error.
**
**	Returns:
**		none
**
**	Side Effects:
**		Saves the letter, by writing or mailing it back to the
**		sender, or by putting it in dead.letter in her home
**		directory.
*/

/* defines for state machine */
# define ESM_REPORT	0	/* report to sender's terminal */
# define ESM_MAIL	1	/* mail back to sender */
# define ESM_QUIET	2	/* messages have already been returned */
# define ESM_DEADLETTER	3	/* save in ~/dead.letter */
# define ESM_POSTMASTER	4	/* return to postmaster */
# define ESM_USRTMP	5	/* save in /usr/tmp/dead.letter */
# define ESM_PANIC	6	/* leave the locked queue/transcript files */
# define ESM_DONE	7	/* the message is successfully delivered */


savemail(e)
	register ENVELOPE *e;
{
	register struct passwd *pw;
	register FILE *fp;
	int state;
	ADDRESS *q;
	char buf[MAXLINE+1];
	register char *p;
	typedef int (*fnptr)();
	long  saveOutputCount;

# ifdef DEBUG
	if (tTd(6, 1))
		(void) printf("\nsavemail, ErrorMode = %c\n", ErrorMode);
# endif DEBUG

	if (bitset(EF_RESPONSE, e->e_flags))
		return;
	if (e->e_class < 0)
	{
		message(Arpa_Info, MSGSTR(SV_JUNK, "Dumping junk mail")); /*MSG*/
		return;
	}
	e->e_flags &= ~EF_FATALERRS;

	/*
	**  In the unhappy event we don't know who to return the mail
	**  to, make someone up.
	*/

	if (e->e_from.q_paddr == NULL)
	{
		if (parseaddr("root", &e->e_from, 0, '\0') == NULL)
		{
			syserr(MSGSTR(SV_ROOT, "Cannot parse root!")); /*MSG*/
			ExitStat = EX_SOFTWARE;
			finis();
		}
	}
	e->e_to = NULL;

	/*
	**  Basic state machine.
	**
	**	This machine runs through the following states:
	**
	**	ESM_QUIET	Errors have already been printed iff the
	**			sender is local.
	**	ESM_REPORT	Report directly to the sender's terminal.
	**	ESM_MAIL	Mail response to the sender.
	**	ESM_DEADLETTER	Save response in ~/dead.letter.
	**	ESM_POSTMASTER	Mail response to the postmaster.
	**	ESM_PANIC	Save response anywhere possible.
	*/

	/* determine starting state */
	switch (ErrorMode)
	{
	  case EM_WRITE:
		state = ESM_REPORT;
		break;

	  case EM_BERKNET:
		/* mail back, but return o.k. exit status */
		ExitStat = EX_OK;

		/* fall through.... */

	  case EM_MAIL:
		state = ESM_MAIL;
		break;

	  case EM_PRINT:
	  case '\0':
		state = ESM_QUIET;
		break;

	  case EM_QUIET:
		/* no need to return anything at all */
		return;

	  default:
		syserr(MSGSTR(SV_ERRORM, "savemail: ErrorMode x%x\n")); /*MSG*/
		state = ESM_MAIL;
		break;
	}

	while (state != ESM_DONE)
	{
# ifdef DEBUG
		if (tTd(6, 5))
			(void) printf("  state %d\n", state);
# endif DEBUG

		switch (state)
		{
		  case ESM_QUIET:
			if (e->e_from.q_mailer == LocalMailer)
				state = ESM_DEADLETTER;
			else
				state = ESM_MAIL;
			break;

		  case ESM_REPORT:

			/*
			**  If the user is still logged in on the same terminal,
			**  then write the error messages back to him.
			*/

			p = ttypath();
			if (p == NULL || freopen(p, "w", stdout) == NULL)
			{
				state = ESM_MAIL;
				break;
			}

			expand("\001n", buf, &buf[sizeof buf - 1], e);
			(void) printf(MSGSTR(SV_FROM, "\r\nMessage from %s...\r\n"), buf); /*MSG*/
			(void)printf(MSGSTR(SV_ERRORS, "Errors occurred while sending mail.\r\n")); /*MSG*/
			if (e->e_xfp != NULL)
			{
				(void) fflush(e->e_xfp);
				fp = fopen(queuename(e, 'x'), "r");
			}
			else
				fp = NULL;
			if (fp == NULL)
			{
				syserr(MSGSTR(DL_EOPEN, "Cannot open %s for %s from %s"), queuename(e, 'x'), e->e_to, e->e_from.q_user); /*MSG*/
				(void) printf(MSGSTR(SV_TRANS, "Transcript of session is unavailable.\r\n")); /*MSG*/
			}
			else
			{
				(void) printf(MSGSTR(SV_TRANS2, "Transcript follows:\r\n")); /*MSG*/
				while (fgets(buf, sizeof buf, fp) != NULL &&
				       !ferror(stdout))
					(void) fputs(buf, stdout);
				(void) fclose(fp);
			}
			(void) printf(MSGSTR(SV_DEAD, "Original message will be saved in dead.letter.\r\n")); /*MSG*/
			state = ESM_DEADLETTER;
			break;

		  case ESM_MAIL:
		  case ESM_POSTMASTER:
			/*
			**  If mailing back, do it.
			**	Throw away all further output.  Don't alias,
			**	since this could cause loops, e.g., if joe
			**	mails to joe@x, and for some reason the network
			**	for @x is down, then the response gets sent to
			**	joe@x, which gives a response, etc.  Also force
			**	the mail to be delivered even if a version of
			**	it has already been sent to the sender.
			*/

			if (state == ESM_MAIL)
			{
				if (e->e_errorqueue == NULL)
					sendtolist(e->e_from.q_paddr,
						(ADDRESS *) NULL,
						&e->e_errorqueue);

				/* deliver a cc: to the postmaster if desired */
				if (PostMasterCopy != NULL)
					sendtolist(PostMasterCopy,
						(ADDRESS *) NULL,
						&e->e_errorqueue);
				q = e->e_errorqueue;
			}
			else
			{
				if (parseaddr("postmaster", q, 0, '\0') == NULL)
				{
					syserr(MSGSTR(SV_POWTM, "cannot parse postmaster!")); /*MSG*/
					ExitStat = EX_SOFTWARE;
					state = ESM_USRTMP;
					break;
				}
			}
			if (returntosender(e->e_message != NULL ? e->e_message :
					   MSGSTR(SV_UDEL, "Unable to deliver mail"), q, TRUE) == 0) /*MSG*/
			{
				state = ESM_DONE;
				break;
			}

			state = state == ESM_MAIL ? ESM_POSTMASTER : ESM_USRTMP;
			break;

		  case ESM_DEADLETTER:
			/*
			**  Save the message in dead.letter.
			**	If we weren't mailing back, and the user is
			**	local, we should save the message in
			**	~/dead.letter so that the poor person doesn't
			**	have to type it over again -- and we all know
			**	what poor typists UNIX users are.
			*/

			p = NULL;
			if (e->e_from.q_mailer == LocalMailer)
			{
			    if (e->e_from.q_home != NULL)
			    {
				p = e->e_from.q_home;
			    }
			    else
			    {
				char  ubuf[MAXNAME];

				(void) unquotestr (ubuf, e->e_from.q_user, 
								    MAXNAME);
				if ((pw = _getpwnam_shadow(ubuf,0)) != NULL)
				    p = pw->pw_dir;
			    }
			}
			if (p == NULL)
			{
				syserr(MSGSTR(SV_ERETURN, "Can't return mail to %s"), e->e_from.q_paddr); /*MSG*/
				state = ESM_MAIL;
				break;
			}
			if (e->e_dfp != NULL)
			{
			    ADDRESS *qq;
			    int oldverb = Verbose;
			    int  fd;
			    struct stat  sbuf;

			    /* we have a home directory; open dead.letter */
			    define('z', p, e);
			    expand("\001z/dead.letter", buf, &buf[sizeof buf - 1], e);
			    /*
			     *  Take care of dead.letter file existence.
			     *
			     *  The normal case is to use the dead.letter
			     *  as-is, regardless of mode and ownership.
			     *  This is the case if dead.letter can be
			     *  successfully stat'd.
			     *
			     *  If the dead.letter file can't be properly
			     *  stat'd (except for nonexistence), OR if we
			     *  have to uid/gid to assign the file to, OR
			     *  if the file creation fails, OR if the uid/gid
			     *  can't be set, then give up and mail it back.
			     */
			    errno = 0; /*in case stat works, but not reg file */
			    if (stat (buf, &sbuf) < 0    || 
				!(sbuf.st_mode & S_IFREG)  )
			    {
			      if (errno != ENOENT                             ||
	    		          !bitset (e->e_from.q_flags, QGOODUID)       ||
	        	 	  (fd = open (buf, O_RDWR | O_CREAT, 
				  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0 ||
				  close (fd) < 0  /* clean up */              ||
				  chown (buf, e->e_from.q_uid, 
					      e->e_from.q_gid) < 0             )
	    		      {
				state = ESM_MAIL;	/* give up */
				break;
	    		      }
			    }
			    Verbose = TRUE;
			    message(Arpa_Info, MSGSTR(SV_SV, "Saving message in %s"), buf); /*MSG*/
			    Verbose = oldverb;
			    e->e_to = buf;
			    qq = NULL;
			    ForceMail = TRUE;
			    sendtolist(buf, (ADDRESS *) NULL, &qq);
			    ForceMail = FALSE;
			    if (deliver(e, qq) == 0)
				state = ESM_DONE;
			    else
				state = ESM_MAIL;
			}
			else
			{
				/* no data file -- try mailing back */
				state = ESM_MAIL;
			}
			break;

		  case ESM_USRTMP:
			/*
			**  Log the mail in /usr/tmp/dead.letter.
			*/

			fp = dfopen("/usr/tmp/dead.letter", "a");
			if (fp == NULL)
			{
				state = ESM_PANIC;
				break;
			}

			OutputCount = 0;
			putfromline(fp, ProgMailer);
			(*e->e_puthdr)(fp, ProgMailer, e, e->e_btype);
			putline("\n", fp, ProgMailer);
			(*e->e_putbody)(fp, ProgMailer, e, e->e_btype);
			putline("\n", fp, ProgMailer);
			saveOutputCount = OutputCount;
			(void) fflush(fp);
			state = ferror(fp) ? ESM_PANIC : ESM_DONE;
			(void) fclose(fp);
			if (state == ESM_DONE)
			    markstats (TRUE, ProgMailer->m_mno,saveOutputCount);

			break;

		  default:
			syserr(MSGSTR(SV_ESTATE, "savemail: unknown state %d"), state); /*MSG*/

			/* fall through ... */

		  case ESM_PANIC:
			syserr(MSGSTR(SV_HELP, "savemail: HELP!!!!")); /*MSG*/
# ifdef LOG
			syslog(LOG_ALERT, MSGSTR(SV_HELP, "savemail: HELP!!!!")); /*MSG*/
# endif LOG
			/* leave the locked queue & transcript files around */
			exit(EX_SOFTWARE);
		}
	}
}
/*
**  RETURNTOSENDER -- return a message to the sender with an error.
**
**	Parameters:
**		msg -- the explanatory message.
**		returnq -- the queue of people to send the message to.
**		sendbody -- if TRUE, also send back the body of the
**			message; otherwise just send the header.
**
**	Returns:
**		zero -- if everything went ok.
**		else -- some error.
**
**	Side Effects:
**		Returns the current message to the sender via
**		mail.
*/

static int	SendBody;

#define MAXRETURNS	6	/* max depth of returning messages */

returntosender(msg, returnq, sendbody)
	char *msg;
	ADDRESS *returnq;
	int sendbody;
{
	char buf[MAXNAME];
	register ENVELOPE *ee;
	static ENVELOPE errenvelope;
	static int returndepth;
	register ADDRESS *q;

# ifdef DEBUG
	if (tTd(6, 1))
	{
	    (void)printf("Return To Sender: msg=\"%s\", depth=%d, CurEnv=%x,\n",
		       			             msg, returndepth, CurEnv);
	    (void) printf("\treturnq=");
		printaddr(returnq, TRUE);
	}
# endif DEBUG

	if (++returndepth >= MAXRETURNS)
	{
		if (returndepth != MAXRETURNS)
			syserr(MSGSTR(SV_RECUR, "returntosender: infinite recursion on %s"), returnq->q_paddr); /*MSG*/
		/* don't "unrecurse" and fake a clean exit */
		/* returndepth--; */
		return (0);
	}

	SendBody = sendbody;
	define('g', "\001f", CurEnv);
	ee = newenvelope(&errenvelope);
	define('a', arpadate(NULL), ee);  /* use current time */
	ee->e_puthdr = putheader;
	ee->e_putbody = errbody;
	ee->e_flags |= EF_RESPONSE;
	ee->e_sendqueue = returnq;
	openxscript(ee);
	for (q = returnq; q != NULL; q = q->q_next)
	{
		if (q->q_alias == NULL)
			addheader("to", q->q_paddr, ee);
	}

	(void) sprintf(buf, "Returned mail: %s", msg);
	addheader("subject", buf, ee);

	/* fake up an address header for the from person */
	expand("\001n", buf, &buf[sizeof buf - 1], CurEnv);
	if (parseaddr(buf, &ee->e_from, -1, '\0') == NULL)
	{
		syserr(MSGSTR(SV_EPARSE, "Can't parse myself!")); /*MSG*/
		ExitStat = EX_SOFTWARE;
		returndepth--;
		return (-1);
	}
	loweraddr(&ee->e_from);

	/* push state into submessage */
	CurEnv = ee;
	define('f', "\001n", ee);
	define('x', "Mail Delivery Subsystem", ee);
	eatheader(ee);

	/* actually deliver the error message */
	sendall(ee, SM_DEFAULT);

	/* restore state */
	dropenvelope(ee);
	CurEnv = CurEnv->e_parent;
	returndepth--;

	/* should check for delivery errors here */
	return (0);
}
/*
**  ERRBODY -- output the body of an error message.
**
**	Typically this is a copy of the transcript plus a copy of the
**	original offending message.
**
**	Parameters:
**		fp -- the output file.
**		m -- the mailer to output to.
**		e -- the envelope we are working in.
**
**	Returns:
**		none
**
**	Side Effects:
**		Outputs the body of an error message.
*/

errbody(fp, m, e)
	register FILE *fp;
	register struct mailer *m;
	register ENVELOPE *e;
{
	register FILE *xfile;
	char buf[MAXLINE];
	char *p;
	char *hlds;

	/*
	**  Output transcript of errors
	*/

	(void) fflush(stdout);
	p = queuename(e->e_parent, 'x');
/**/
/* do fprintf's on fp need error checking? */
	if ((xfile = fopen(p, "r")) == NULL)
	{
		syserr(MSGSTR(DL_EOPEN, "Cannot open %s for %s from %s"), p, e->e_to, e->e_from.q_user); /*MSG*/
		hlds = newstr(MSGSTR(SV_TRANSU,
		    "  ----- Transcript of session is unavailable -----\n"));
		fprintf(fp, hlds);
	}
	else
	{
		hlds = newstr(MSGSTR(SV_TRANSHDR,
		    "   ----- Transcript of session follows -----\n"));
		fprintf(fp, hlds);
		if (e->e_xfp != NULL)
			(void) fflush(e->e_xfp);
		while (!ferror (fp) && fgets(buf, sizeof buf, xfile) != NULL)
			putline(buf, fp, m);
		(void) fclose(xfile);
	}
	free(hlds);
	errno = 0;

	/*
	**  Output text of original message
	*/

	if (NoReturn) {
		hlds = newstr(MSGSTR(SV_SUPR,
		    "\n   ----- Return message suppressed -----\n\n"));
		fprintf(fp, hlds);
	} else if (e->e_parent->e_dfp != NULL)
	{
		if (SendBody)
		{
			putline("\n", fp, m);
			hlds = newstr(MSGSTR(SV_UNSENT,
			    "   ----- Unsent message follows -----\n"));
			putline(hlds, fp, m);
			(void) fflush(fp);
			putheader(fp, m, e->e_parent, e->e_btype);
			putline("\n", fp, m);
			putbody(fp, m, e->e_parent, e->e_btype);
		}
		else
		{
			putline("\n", fp, m);
			hlds = newstr(MSGSTR(SV_MSG,
			    "  ----- Message header follows -----\n"));
			putline(hlds, fp, m);
			(void) fflush(fp);
			putheader(fp, m, e->e_parent, e->e_btype);
		}
	}
	else
	{
		putline("\n", fp, m);
		hlds = newstr(MSGSTR(SV_NOMSG,
		    "  ----- No message was collected -----\n"));
		putline(hlds, fp, m);
		putline("\n", fp, m);
	}
	free(hlds);

	/*
	**  Cleanup and exit
	*/

	if (errno != 0)
		syserr(MSGSTR(SV_EBODY, "errbody: I/O error")); /*MSG*/
}
