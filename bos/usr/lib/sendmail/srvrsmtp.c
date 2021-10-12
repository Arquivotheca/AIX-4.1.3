static char sccsid[] = "@(#)28	1.10.1.1  src/bos/usr/lib/sendmail/srvrsmtp.c, cmdsend, bos41J, 9510A_all 2/15/95 19:49:18";
/* 
 * COMPONENT_NAME: CMDSEND srvrsmtp.c
 * 
 * FUNCTIONS: MSGSTR, help, runinchild, skipword, smtp 
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
# include <sys/types.h>

# include "conf.h"
# include "useful.h"

# include <sys/uio.h>
# include <sys/socket.h>
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"

char *helpmsg[] = {
"cpyr	",
"cpyr	Sendmail",
"cpyr	Copyright (c) 1983  Eric P. Allman",
"cpyr	Berkeley, California",
"cpyr	",
"cpyr	Copyright (c) 1983 Regents of the University of California.",
"cpyr	All rights reserved.  The Berkeley software License Agreement",
"cpyr	specifies the terms and conditions for redistribution.",
"cpyr	",
"cpyr	@(#)sendmail.hf	4.2 (Berkeley) 6/7/85",
"cpyr	",
"cpyr	Copyright (c) 1987 IBM Corporation.  All rights reserved.",
"cpyr",
"smtp	Commands:",
"smtp		HELO	MAIL	RCPT	DATA	RSET",
"smtp		NOOP	QUIT	HELP	VRFY	EXPN",
"smtp	For more info use \"HELP <topic>\".",
"smtp	To report bugs in the implementation, contact your service representative.",
"smtp	For local information contact postmaster at this site.",
"help	HELP [ <topic> ]",
"help		The HELP command gives help info.",
"helo	HELO <hostname>",
"helo		Identify yourself.",
"mail	MAIL FROM: <sender>",
"mail		Specify the sender.",
"rcpt	RCPT TO: <recipient>",
"rcpt		Specify the recipient.  Can be used any number of times.",
"data	DATA",
"data		Following text is collected as the message.",
"data		End with a single dot.",
"rset	RSET",
"rset		Resets the protocol state.",
"quit	QUIT",
"quit		Exit sendmail (SMTP).",
"vrfy	VRFY <recipient>",
"vrfy		Allows verification of recipient.  Not part of protocol.",
"expn	EXPN <recipient>",
"expn		Same as VRFY in this implementation.",
"noop	NOOP",
"noop		Do nothing.",
"send	SEND FROM: <sender>",
"send		replaces the MAIL command, and can be used to send",
"send		directly to a users terminal.  Not supported in this",
"send		implementation.",
"soml	SOML FROM: <sender>",
"soml		Send or mail.  If the user is logged in, send directly,",
"soml		otherwise mail.  Not supported in this implementation.",
"saml	SAML FROM: <sender>",
"saml		Send and mail.  Send directly to the user's terminal,",
"saml		and also mail a letter.  Not supported in this",
"saml		implementation.",
"turn	TURN",
"turn		Reverses the direction of the connection.  Not currently",
"turn		implemented.",
NULL
};

void  exit ();
int sameword();
char *macvalue();
ADDRESS *recipient();
ENVELOPE *newenvelope();
char  *sfgets ();
ADDRESS  *parseaddr ();
char  *queuename ();
char  *xalloc ();

ENVELOPE BlankEnvelope;
extern int  QuickAbort;
extern int  QuickMsgs;

extern int  Sid;			/* queue clean sema4 id */

static char *skipword(register char *, char *);
static void logsmtp(char *, char *);

/*
**  SMTP -- run the SMTP protocol.  This function never returns.  It can
**		be called directly from main.c, or from the daemon code.
**		This code will finis () when through.  Each communication
**		request that the daemon receives will come to smtp as a 
**		separate child of the daemon.
**
**	Parameters:
**		none.
**
**	Returns:
**		never.
**
**	Side Effects:
**		Reads commands from the input channel and processes
**			them.
*/

struct cmd
{
	char	*cmdname;	/* command name */
	int	cmdcode;	/* internal code, see below */
};

/* values for cmdcode */
# define CMDERROR	0	/* bad command */
# define CMDMAIL	1	/* mail -- designate sender */
# define CMDRCPT	2	/* rcpt -- designate recipient */
# define CMDDATA	3	/* data -- send message text */
# define CMDRSET	4	/* rset -- reset state */
# define CMDVRFY	5	/* vrfy -- verify address */
# define CMDHELP	6	/* help -- give usage info */
# define CMDNOOP	7	/* noop -- do nothing */
# define CMDQUIT	8	/* quit -- close connection and die */
# define CMDHELO	9	/* helo -- be polite */
# define CMDVERB	10	/* verb -- go into verbose mode */
# define CMDONEX	11	/* onex -- sending one transaction only */

static struct cmd	CmdTab[] =
{
	"mail",		CMDMAIL,
	"rcpt",		CMDRCPT,
	"data",		CMDDATA,
	"rset",		CMDRSET,
	"vrfy",		CMDVRFY,
	"expn",		CMDVRFY,
	"help",		CMDHELP,
	"noop",		CMDNOOP,
	"quit",		CMDQUIT,
	"helo",		CMDHELO,
	"verb",		CMDVERB,
	"onex",		CMDONEX,
	NULL,		CMDERROR,
};

int	InChild = FALSE;		/* true if running in a subprocess */
int	OneXact = FALSE;		/* one xaction only this run */

#define EX_QUIT		22		/* special code for QUIT command */

smtp()
{
	register char *p;
	register struct cmd *c;
	char *cmd;
	int hasmail;			/* mail command received */
	ADDRESS *vrfyqueue;
	ADDRESS *a;
	char *sendinghost;
	char inp[MAXLINE];
	char cmdbuf[100];

	hasmail = FALSE;
	if (OutChannel != stdout)
	{
		/* arrange for debugging output to go to remote host */
		(void) close(1);
		(void) dup(fileno(OutChannel));
	}
	settime();
	if (RealHostName != NULL)
	{
		CurHostName = RealHostName;
		setproctitle("srvrsmtp %s", CurHostName);
	}
	else
	{
		/* this must be us!! */
		CurHostName = MyHostName;
	}
	expand("\001e", inp, &inp[sizeof inp], CurEnv);
	message("220", inp);
	SmtpPhase = "startup";
	sendinghost = NULL;
	while (1)
	{
		QuickAbort = FALSE;	/* normal syserr/usrerr processing */
		HoldErrs = FALSE;

		/* setup for the read */
		CurEnv->e_to = NULL;
		Errors = 0;
		(void) fflush(stdout);

		/* read the input line */
		p = sfgets(inp, sizeof inp, InChannel);
#ifdef DEBUG
		if (tTd (18, 20))
		    printf("smtp: got string='%s'\n", inp);
#endif DEBUG

		/* handle errors */
		if (p == NULL)
		{
			/* end of file, just die */
			message("421", MSGSTR(SP_LOST, "%s Lost input channel from %s"), MyHostName, CurHostName); /*MSG*/
			finis();
		}

		/* clean up end of line */
		fixcrlf(inp, TRUE);

/**/
/* do fprintf's below need error checking? */
		/* echo command to transcript */
		if (CurEnv->e_xfp != NULL)
			fprintf(CurEnv->e_xfp, "<<< %s\n", inp);

		/* break off command */
		for (p = inp; isspace(*p); p++)
			continue;
		cmd = p;
		for (cmd = cmdbuf; *p != '\0' && !isspace(*p); )
			*cmd++ = *p++;
		*cmd = '\0';

		/* throw away leading whitespace */
		while (isspace(*p))
			p++;

		/* decode command */
		for (c = CmdTab; c->cmdname != NULL; c++)
		{
			if (sameword(c->cmdname, cmdbuf))
				break;
		}

		/* process command */
		switch (c->cmdcode)
		{
		  case CMDHELO:		/* hello -- introduce yourself */
			SmtpPhase = "HELO";
			setproctitle("%s: %s", CurHostName, inp);
			if (sameword(p, MyHostName))
			{
				/*
				 * didn't know about alias,
				 * or connected to an echo server
				 */
				message("553", MSGSTR(SP_ESELF, "Local configuration error, hostname not recognized as local"));
				break;
			}
			if (RealHostName != NULL && !sameword(p, RealHostName))
			{
				char hostbuf[MAXNAME];

				(void) sprintf(hostbuf, "%s (%s)", p, RealHostName);
				sendinghost = newstr(hostbuf);
			}
			else
				sendinghost = newstr(p);
			message("250", MSGSTR(SP_PLEASED, "%s Hello %s, pleased to meet you"), MyHostName, sendinghost); /*MSG*/
			break;

		  case CMDMAIL:		/* mail -- designate sender */
			SmtpPhase = "MAIL";

			/* force a sending host even if no HELO given */
			if (RealHostName != NULL && 
					     macvalue('s', CurEnv) == NULL)
				sendinghost = RealHostName;

			/* check for validity of this command */
			if (hasmail)
			{
				message("503", MSGSTR(SP_ESENDER, "Sender already specified")); /*MSG*/
				break;
			}
			if (InChild)
			{
				errno = 0;
				syserr(MSGSTR(SP_ENEST, "Nested MAIL command")); /*MSG*/
				exit(0);
			}

			/*
			 *  A subprocess is created to execute the MAIL
			 *  command.  Why?
			 */
			if (runinchild("SMTP-MAIL") > 0)
				break;
			define('s', sendinghost, CurEnv);
			define('r', "SMTP", CurEnv);
			initsys();
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);

			/* child -- go do the processing */
			p = skipword(p, "from");
			if (p == NULL)
				break;
			setsender(p);
			if (Errors == 0)
			{
				message("250", MSGSTR(SV_OK, "Sender ok")); /*MSG*/
				hasmail = TRUE;
			}
			else if (InChild)
				finis();
			break;

		  case CMDRCPT:		/* rcpt -- designate recipient */
			SmtpPhase = "RCPT";
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);

			QuickAbort = TRUE;	/* sys/usrerr emits 1st msg */
			QuickMsgs  = 0;		/* init the count */

			p = skipword(p, "to");
			if (p == NULL)
				goto fixerr;
			a = parseaddr(p, (ADDRESS *) NULL, 1, '\0');
			if (a == NULL)
				goto fixerr;
			a->q_flags |= QPRIMARY;
			a = recipient(a, &CurEnv->e_sendqueue);
			if (Errors != 0)
			{
			    fixerr:
				if (LogSMTP)
				    logsmtp("RCPT", p);
				CurEnv->e_flags &= ~EF_FATALERRS;
				break;
			}

			/* no errors during parsing, but might be a duplicate */
			CurEnv->e_to = p;
			if (!bitset(QBADADDR, a->q_flags))
				message("250", MSGSTR(SP_RECIP, "Recipient ok")); /*MSG*/
			else
			{
				if (LogSMTP)
				    logsmtp("RCPT", p);
				/* punt -- should keep message in ADDRESS.... */
				message("550", MSGSTR(SP_EADDR, "Addressee unknown")); /*MSG*/
			}
			CurEnv->e_to = NULL;
			break;

		  case CMDDATA:		/* data -- text of mail */
			SmtpPhase = "DATA";
			if (!hasmail)
			{
				message("503", MSGSTR(SP_MAIL, "Need MAIL command")); /*MSG*/
				break;
			}
			else if (CurEnv->e_nrcpts <= 0)
			{
				message("503", MSGSTR(SP_RCPT, "Need RCPT (recipient)")); /*MSG*/
				break;
			}

			/* collect the text of the message */
			SmtpPhase = "collect";
			setproctitle("%s %s: %s", CurEnv->e_id,
				CurHostName, inp);
			collect(TRUE);
			if (Errors != 0)
				break;

			/*
			 *  Save statistics for the "from" mailer.
			 */
			markstats (FALSE, CurEnv->e_from.q_mailer->m_mno, 
							CurEnv->e_msgsize);

			/*
			**  Arrange to send to everyone.
			**	If sending to multiple people, mail back
			**		errors rather than reporting directly.
			**	In any case, don't mail back errors for
			**		anything that has happened up to
			**		now (the other end will do this).
			**	Truncate our transcript -- the mail has gotten
			**		to us successfully, and if we have
			**		to mail this back, it will be easier
			**		on the reader.
			**	Then send to everyone.
			**	Finally give a reply code.  If an error has
			**		already been given, don't mail a
			**		message back.
			**	We goose error returns by clearing error bit.
			*/

			SmtpPhase = "delivery";
			if (CurEnv->e_nrcpts != 1)
			{
				HoldErrs = TRUE;
				ErrorMode = EM_MAIL;
			}
			CurEnv->e_flags &= ~EF_FATALERRS;
			CurEnv->e_xfp = freopen(queuename(CurEnv, 'x'), "w", CurEnv->e_xfp);

			/* send to all recipients */
			sendall(CurEnv, SM_DEFAULT);
			CurEnv->e_to = NULL;

			/* issue success if appropriate and reset */
			if (Errors == 0 || HoldErrs)
				message("250", MSGSTR(SP_OK, "Ok")); /*MSG*/
			else
				CurEnv->e_flags &= ~EF_FATALERRS;

			/* if in a child, pop back to our parent */
			if (InChild)
				finis();

			/* clean up a bit */
			hasmail = 0;
			dropenvelope(CurEnv);
			CurEnv = newenvelope(CurEnv);
			CurEnv->e_flags = BlankEnvelope.e_flags;
			break;

		  case CMDRSET:		/* rset -- reset state */
			message("250", MSGSTR(SP_RESET, "Reset state")); /*MSG*/
			if (InChild)
				finis();
			break;

		  case CMDVRFY:		/* vrfy -- verify address */
			if (LogSMTP)
			    logsmtp("VRFY", p);
			if (SecureSMTP)
			{
			    message("250", "<%s>", p);
			    break;
			}
			if (runinchild("SMTP-VRFY") > 0)
				break;
			setproctitle("%s: %s", CurHostName, inp);
			vrfyqueue = NULL;
			QuickAbort = TRUE;	/* sys/usrerr emits 1st msg */
			QuickMsgs  = 0;		/* init the count */
			sendtolist(p, (ADDRESS *) NULL, &vrfyqueue);
			if (Errors != 0)
			{
				if (InChild)
					finis();
				break;
			}
			while (vrfyqueue != NULL)
			{
			    register ADDRESS *aa = vrfyqueue->q_next;
			    char *code;

			    while (aa != NULL &&
                                   bitset(QDONTSEND|QBADADDR, aa->q_flags))
			    	aa = aa->q_next;

			    if (!bitset(QDONTSEND|QBADADDR, vrfyqueue->q_flags))
			    {
			    	if (aa != NULL)
			    		code = "250-";
			    	else
			    		code = "250";
			    	if (vrfyqueue->q_fullname == NULL)
			    	    message(code, "<%s>", vrfyqueue->q_paddr);
			    	else
			    	    message(code, "%s <%s>",
			    		    vrfyqueue->q_fullname,
                                            vrfyqueue->q_paddr);
			    }
			    else if (aa == NULL)
			    	message("554", MSGSTR(SP_DESTR, "Self destructive alias loop")); /*MSG*/
			    vrfyqueue = aa;
			}
			if (InChild)
				finis();
			break;

		  case CMDHELP:		/* help -- give user info */
			if (*p == '\0')
				p = "SMTP";
			help(p);
			break;

		  case CMDNOOP:		/* noop -- do nothing */
/* was code 200 */	message("250", MSGSTR(SP_OK2, "OK")); /*MSG*/
			break;

		  case CMDQUIT:		/* quit -- leave mail */
			message("221", MSGSTR(SP_CLOSE, "%s closing connection"), MyHostName); /*MSG*/
			if (InChild)
				ExitStat = EX_QUIT;
			finis();

		  case CMDVERB:		/* set verbose mode */
			Verbose = TRUE;
			SendMode = SM_DELIVER;
			message("200", MSGSTR(SP_VERB, "Verbose mode")); /*MSG*/
			break;

		  case CMDONEX:		/* doing one transaction only */
			OneXact = TRUE;
			message("200", MSGSTR(SP_EONE, "Only one transaction")); /*MSG*/
			break;

		  case CMDERROR:	/* unknown command */
			message("500", MSGSTR(SP_ECMD, "Command unrecognized")); /*MSG*/
			break;

		  default:
			syserr(MSGSTR(SP_CODE, "smtp: unknown code %d"), c->cmdcode); /*MSG*/
			break;
		}
	}
}
/*
**  SKIPWORD -- skip a fixed word.
**
**	Parameters:
**		p -- place to start looking.
**		w -- word to skip.
**
**	Returns:
**		p following w.
**		NULL on error.
**
**	Side Effects:
**		clobbers the p data area.
*/

static char *
skipword(register char *p, char *w)
{
	register char *q;

	/* find beginning of word */
	while (isspace(*p))
		p++;
	q = p;

	/* find end of word */
	while (*p != '\0' && *p != ':' && !isspace(*p))
		p++;
	while (isspace(*p))
		*p++ = '\0';
	if (*p != ':')
	{
	  syntax:
		message("501", MSGSTR(SP_SYNTAX, "Syntax error")); /*MSG*/
		Errors++;
		return (NULL);
	}
	*p++ = '\0';
	while (isspace(*p))
		p++;

	/* see if the input word matches desired word */
	if (!sameword(q, w))
		goto syntax;

	return (p);
}
/*
**  LOGSMTP -- log SMTP action
**
**	Parameters:
**		c -- command executed
**		p -- parameter given to command
**
**	Returns:
**
**	Side Effects:
**		adds a syslog entry
*/

static void
logsmtp(char *c, char *p)
{
    syslog(LOG_WARNING, "SMTP (%s) %s %s", CurHostName, c, p);
}
/*
**  HELP -- implement the HELP command.
**
**	Parameters:
**		topic -- the topic we want help for.
**
**	Returns:
**		none.
**
**	Side Effects:
**		outputs the help file to message output.
*/

help(topic)
	char *topic;
{
	int len;
	int i;
	int noinfo;
	char *buf;

	len = strlen(topic);
	makelower(topic);
	noinfo = TRUE;

	for (i=0; helpmsg[i]; i++) {
		buf = MSGSTR(HELPMSG+i, helpmsg[i]);
		if (strncmp(buf, topic, len) == 0) {
			register char *p;

			p = strchr(buf, '\t');
			if (p == NULL)
				p = buf;
			else
				p++;
			message("214-", p);
			noinfo = FALSE;
		}
	}

	if (noinfo)
		message("504", MSGSTR(SP_EHELP, "HELP topic unknown")); /*MSG*/
	else
		message("214", MSGSTR(SP_END, "End of HELP info")); /*MSG*/
}
/*
**  RUNINCHILD -- return twice -- once in the child, then in the parent again
**
**	Parameters:
**		label -- a string used in error messages
**
**	Returns:
**		zero in the child
**		one in the parent
**
**	Side Effects:
**		none.
*/

runinchild(label)
	char *label;
{
	int  childpid;
	int  i;

	/*
	 *  If OneXact is set, then parent returns as child without forking or 
	 *  meddling with InChild, QuickAbort, or dropenvelope ().
	 */
	if (!OneXact)
	{
		/*
		 *  Change our process semadj value so that inadvertently
		 *  exiting our process won't replace our semaphore count
		 *  and possibly allow a queue clean while we are in the
		 *  queue.  An exception here prevents semsig from being
		 *  executed below.  In that case, the child was unprotected
		 *  against a queue clean if the parent died while the child
		 *  was active.  There will be a syserr in that case.
	 	 *
		 *  All semop exceptions cause syserr.  It is assumed that once
		 *  a semop fails, all subsequent semops will fail.  This
		 *  means that queue cleaning can't occur.
		 */
		(void) semwait (Sid, 1, 0, 1);		/* 2 waits, 1 semadj */

#ifdef DEBUG
		if (tTd (18, 111))
		    exit (99);
#endif DEBUG

		/*
		 *  In BSD code this is a fork with retry.  I don't know
		 *  if there is a better way to handle this on system
		 *  overload.  We go ahead and retry, also.
		 */
		childpid = dfork(10);
		if (childpid < 0)
		{
			syserr(MSGSTR(SP_EFORK, "%s: cannot fork"), label); /*MSG*/

			/*
			 *  An exception in semsig just permanently
			 *  disables queue clean.
	 		 *
			 *  All semop failures are logged with syserr.
			 */
			(void) semsig (Sid, 1, 1);	/* 1 wait, 1 semadj */

			return (1);
		}
		if (childpid > 0)
		{
			int st;
#ifdef DEBUG
			if (tTd (18, 110))
		    	    exit (99);
#endif DEBUG
			/* parent -- wait for child to complete */
			st = waitfor(childpid);
			if (st == -1)
				syserr(MSGSTR(SP_ECHILD, "%s: lost child"), label); /*MSG*/
#ifdef DEBUG
			if (tTd (18, 109))
		    	    exit (99);
#endif DEBUG
			/*
			 *  All semop failures are logged with syserr.
			 */
			(void) semsig (Sid, 1, 1);	/* 1 wait, 1 semadj */
#ifdef DEBUG
			if (tTd (18, 108))
		    	    exit (99);
#endif DEBUG
			/* if we exited on a QUIT command,complete the process*/
			if (st == (EX_QUIT << 8))
				finis();

			return (1);
		}
		else
		{
			/* child */
			InChild = TRUE;
			QuickAbort = FALSE;	/* normal sys/usrerr */
			clearenvelope(CurEnv, FALSE);
		}
	}

	/* open alias database */
	if (i = openaliases(AliasFile))
	{
#ifdef LOG
	    syslog(LOG_INFO, MSGSTR(QU_EALIAS, "alias file \"%s\" data base open failure"), AliasFile); /*MSG*/
#endif LOG
	    ExitStat = i;		/* inaccessible	data base	*/
	    finis ();
	}

	return (0);
}
