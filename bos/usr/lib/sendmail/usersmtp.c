static char sccsid[] = "@(#)39  1.12  src/bos/usr/lib/sendmail/usersmtp.c, cmdsend, bos411, 9428A410j 9/5/91 09:48:31";
/* 
 * COMPONENT_NAME: CMDSEND usersmtp.c
 * 
 * FUNCTIONS: MSGSTR, REPLYCLASS, REPLYTYPE, reply, smtpdata, 
 *            smtpinit, smtpmessage, smtpquit, smtprcpt 
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

# include <ctype.h>
# include "sysexits.h"
# include <errno.h>
# include <stdio.h>
# include <ctype.h>
# include <string.h>
# include <netdb.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>

# include "sendmail.h"

extern long  OutputCount;

long saveReadTimeout;
char *errstring();
char *statstring();
char *remotename();
char  *sfgets ();

extern char MsgBuf[];           /* err.c */
extern char Arpa_TSyserr[];     /* conf.c */

/*
**  USERSMTP -- run SMTP protocol from the user end.
**
**	This protocol is described in RFC821.
*/

#define REPLYTYPE(r)	((r) / 100)		/* first digit of reply code */
#define REPLYCLASS(r)	(((r) / 10) % 10)	/* second digit of reply code */
#define SMTPCLOSING	421			/* "Service Shutting Down" */

char	SmtpMsgBuffer[MAXLINE];		/* buffer for commands */
char	SmtpReplyBuffer[MAXLINE];	/* buffer for replies */
char	SmtpError[MAXLINE] = "";	/* save failure error messages */
FILE	*SmtpOut;			/* output file */
FILE	*SmtpIn;			/* input file */
int	SmtpPid;			/* pid of mailer */

/* following represents the state of the SMTP connection */
int	SmtpState;			/* connection state, see below */

#define SMTP_CLOSED	0		/* connection is closed */
#define SMTP_OPEN	1		/* connection is open for business */
#define SMTP_SSD	2		/* service shutting down */
/*
**  SMTPINIT -- initialize SMTP.
**
**	Opens the connection and sends the initial protocol.
**
**	Parameters:
**		m -- mailer to create connection to.
**		pvp -- pointer to parameter vector to pass to
**			the mailer.
**
**	Returns:
**		appropriate exit status -- EX_OK on success.
**		If not EX_OK, it should close the connection.
**
**	Side Effects:
**		creates connection and sends initial protocol.
*/

smtpinit(m, pvp)
	struct mailer *m;
	char **pvp;
{
	register int r;
	char buf[MAXNAME];

	/* SMTP code requires defined (valid) local host name */
	if (!*MyHostName || (strchr(MyHostName, '.') && 
		!gethostbyname(MyHostName))) {
#ifdef LOG
		syslog (LOG_ERR, MSGSTR(US_INVALID,  
			"usersmtp: %s is not a valid local hostname"), MyHostName);
#endif LOG
		return (EX_NOLHOST);
	}

	/*
	**  Open the connection to the mailer.
	*/

#ifdef DEBUG
	if (SmtpState == SMTP_OPEN)
		syserr("smtpinit: already open");
#endif DEBUG

	SmtpIn = SmtpOut = NULL;
	SmtpState = SMTP_CLOSED;
	SmtpError[0] = '\0';
	SmtpPhase = "user open";
	setproctitle("%s %s: %s", CurEnv->e_id, pvp[1], SmtpPhase);
	SmtpPid = openmailer(m, pvp, (ADDRESS *) NULL, TRUE, &SmtpOut, &SmtpIn);
	if (SmtpPid < 0)
	{
# ifdef DEBUG
		if (tTd(18, 1))
		    (void)printf("smtpinit: cannot open %s: stat %d errno %d\n",
			   pvp[0], ExitStat, errno);
# endif DEBUG
		if (CurEnv->e_xfp != NULL)
		{
			register char *p;

			if (errno == 0)
			{
				p = statstring(ExitStat);
/**/
/* do the fprintf's need error checking? */
				fprintf(CurEnv->e_xfp,
					"%.3s %s.%s... %s\n",
					p, pvp[1], m->m_name, p);
			}
			else
			{
				r = errno;
				fprintf(CurEnv->e_xfp,
					"421 %s.%s... Deferred: %s\n",
					pvp[1], m->m_name, errstring(errno));
				errno = r;
			}
		}
		return (ExitStat);
	}
	SmtpState = SMTP_OPEN;

	/*
	**  Get the greeting message.
	**	This should appear spontaneously.  Give it five minutes to
	**	happen.  "reply" calls "sfgets", which will time out if
	**	called for by the OG option in the config file.  However,
	**	that timeout, if set, is usually 2 hours.
	*/


	if (GreetTimeout)
	{
		saveReadTimeout = ReadTimeout;	/* temporarily chg timeout */
		ReadTimeout = GreetTimeout;
	}
	SmtpPhase = "greeting wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (GreetTimeout)
		ReadTimeout = saveReadTimeout;	/* restore previous timeout */
	if (r < 0 || REPLYTYPE(r) != 2)
		goto tempfail;

	/*
	**  Send the HELO command.
	**	My mother taught me to always introduce myself.
	*/

	smtpmessage("HELO %s", m, MyHostName);
	SmtpPhase = "HELO wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (r < 0)
		goto tempfail;
	else if (REPLYTYPE(r) == 5)
		goto unavailable;
	else if (REPLYTYPE(r) != 2)
		goto tempfail;

	/*
	**  If this is expected to be another sendmail, send some internal
	**  commands.
	*/

	if (bitnset(M_INTERNAL, m->m_flags))
	{
		/* tell it to be verbose */
		smtpmessage("VERB", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;

		/* tell it we will be sending one transaction only */
		smtpmessage("ONEX", m);
		r = reply(m);
		if (r < 0)
			goto tempfail;
	}

	/*
	**  Send the MAIL command.
	**	Designates the sender.
	*/

	expand("\001g", buf, &buf[sizeof buf - 1], CurEnv);
	if (CurEnv->e_from.q_mailer == LocalMailer ||
	    !bitnset(M_FROMPATH, m->m_flags))
	{
		smtpmessage("MAIL From:<%s>", m, buf);
	}
	else
	{
		smtpmessage("MAIL From:<@%s%c%s>", m, MyHostName,
			buf[0] == '@' ? ',' : ':', buf);
	}
	if (MailTimeout)
	{
		saveReadTimeout = ReadTimeout;
		ReadTimeout = MailTimeout;
	}
	SmtpPhase = "MAIL wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (MailTimeout)
		ReadTimeout = saveReadTimeout;
	if (r < 0 || REPLYTYPE(r) == 4)
		goto tempfail;
	else if (r == 250)
		return (EX_OK);
	else if (r == 552)
		goto unavailable;

	/* protocol error -- close up */
	smtpquit(m);
	return (EX_PROTOCOL);

	/* signal a temporary failure */
  tempfail:
	smtpquit(m);
	return (EX_TEMPFAIL);

	/* signal service unavailable */
  unavailable:
	smtpquit(m);
	return (EX_UNAVAILABLE);
}
/*
**  SMTPRCPT -- designate recipient.
**
**	Parameters:
**		to -- address of recipient.
**		m -- the mailer we are sending to.
**
**	Returns:
**		exit status corresponding to recipient status.
**
**	Side Effects:
**		Sends the mail via SMTP.
*/

smtprcpt(to, m)
	ADDRESS *to;
	register MAILER *m;
{
	register int r;

	smtpmessage("RCPT To:<%s>", m, remotename(to->q_user, m, FALSE, TRUE));

	if (RcptTimeout)
	{
		saveReadTimeout = ReadTimeout;	/* temporarily change timeout */
		ReadTimeout = RcptTimeout;
	}
	SmtpPhase = "RCPT wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (RcptTimeout)
		ReadTimeout = saveReadTimeout;	/* restore timeout */
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (REPLYTYPE(r) == 2)
		return (EX_OK);
	else if (r == 550 || r == 551 || r == 553)
		return (EX_NOUSER);
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
	return (EX_PROTOCOL);
}
/*
**  SMTPDATA -- send the data and clean up the transaction.
**
**	Parameters:
**		m -- mailer being sent to.
**		e -- the envelope for this message.
**
**	Returns:
**		exit status corresponding to DATA command.
**
**	Side Effects:
**		none.
*/

smtpdata(m, e, btype)
	struct mailer *m;
	register ENVELOPE *e;
	int btype;
{
	register int r;
	long  saveOutputCount;

	/*
	**  Send the data.
	**	First send the command and check that it is ok.
	**	Then send the data.
	**	Follow it up with a dot to terminate.
	**	Finally get the results of the transaction.
	*/

	/* send the command and check ok to proceed */
	smtpmessage("DATA", m);
	if (DataTimeout)
	{
		saveReadTimeout = ReadTimeout;	/* temporarily chg timeout */ 
		ReadTimeout = DataTimeout;
	}
	SmtpPhase = "DATA wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (DataTimeout)
		ReadTimeout = saveReadTimeout;	/* restore timeout */
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 554)
		return (EX_UNAVAILABLE);
	else if (r != 354)
		return (EX_PROTOCOL);

	/* now output the actual message */
	if (TransmitTimeout)
	{
		saveReadTimeout = ReadTimeout;
		ReadTimeout = TransmitTimeout;
	}
	OutputCount = 0;
	(*e->e_puthdr)(SmtpOut, m, CurEnv, btype);
	if (WriteErr)
		goto writefail;
	putline("\n", SmtpOut, m); 
	if (WriteErr)
		goto writefail;
	(*e->e_putbody)(SmtpOut, m, CurEnv, btype);
	if (WriteErr)
		goto writefail;
	saveOutputCount = OutputCount;
	if (TransmitTimeout)
		ReadTimeout = saveReadTimeout;

	/* terminate the message */
	fprintf(SmtpOut, ".%s", m->m_eol);
	saveOutputCount += 1 + strlen (m->m_eol);
	if (Verbose && !HoldErrs)
		nmessage(Arpa_Info, ">>> .");

	/* check for the results of the transaction */
	if (PeriodTimeout)
	{
		saveReadTimeout = ReadTimeout;
		ReadTimeout = PeriodTimeout;  /* temporarily change timeout */ 
	}
	SmtpPhase = "result wait";
	setproctitle("%s %s: %s", CurEnv->e_id, CurHostName, SmtpPhase);
	r = reply(m);
	if (PeriodTimeout)
		ReadTimeout = saveReadTimeout;	/* restore timeout */
	if (r < 0 || REPLYTYPE(r) == 4)
		return (EX_TEMPFAIL);
	else if (r == 250)
	{
		markstats (TRUE, m->m_mno, saveOutputCount);
		return (EX_OK);
	}
	else if (r == 552 || r == 554)
		return (EX_UNAVAILABLE);
	return (EX_PROTOCOL);
	
	/* signal a temporary failure */
  writefail:
	if (errno == 0)
		errno = ECONNRESET;
	message(Arpa_TSyserr, MSGSTR(US_DATA, "smtpdata: write error"));
	if (TransmitTimeout)
		ReadTimeout = saveReadTimeout;
	SmtpState = SMTP_CLOSED;
	smtpquit(m);
	return(EX_TEMPFAIL);
}
/*
**  SMTPQUIT -- close the SMTP connection.
**
**	Parameters:
**		m -- a pointer to the mailer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sends the final protocol and closes the connection.
*/

smtpquit(m)
	register MAILER *m;
{
	int i;

	/* if the connection is already closed, don't bother */
	if (SmtpIn == NULL)
		return;

	/* send the quit message if not a forced quit */
	if (SmtpState == SMTP_OPEN || SmtpState == SMTP_SSD)
	{
		smtpmessage("QUIT", m);
		(void) reply(m);
		if (SmtpState == SMTP_CLOSED)
			return;
	}

	/* now actually close the connection */
	(void) fclose(SmtpIn);
	(void) fclose(SmtpOut);
	SmtpIn = SmtpOut = NULL;
	SmtpState = SMTP_CLOSED;

	/* and pick up the zombie */
	i = endmailer(SmtpPid, m->m_argv[0]);
	if (i != EX_OK)
		syserr("smtpquit %s: stat %d", m->m_argv[0], i);
}
/*
**  REPLY -- read arpanet reply
**
**	Parameters:
**		m -- the mailer we are reading the reply from.
**
**	Returns:
**		reply code it reads.
**
**	Side Effects:
**		flushes the mail file.
*/

reply(m)
	MAILER *m;
{
	(void) fflush(SmtpOut);

#ifdef DEBUG
	if (tTd(18, 1))
		(void) printf("reply\n");
#endif DEBUG

	/*
	**  Read the input line, being careful not to hang.
	*/

	for (;;)
	{
		register int r;
		register char *p;

		/* actually do the read */
		if (CurEnv->e_xfp != NULL)
			(void) fflush(CurEnv->e_xfp);	/* for debugging */

		/* if we are in the process of closing just give the code */
		if (SmtpState == SMTP_CLOSED)
			return (SMTPCLOSING);

		/* get the line from the other side */
		p = sfgets(SmtpReplyBuffer, sizeof SmtpReplyBuffer, SmtpIn);
		if (p == NULL)
		{
			/* if the remote end closed early, fake an error */
			if (errno == 0)
				errno = ECONNRESET;

			message(Arpa_TSyserr, MSGSTR(US_REPL, "reply: read error")); /*MSG*/
# ifdef DEBUG
			/* if debugging, pause so we can see state */
			if (tTd(18, 100))
				(void) pause();
# endif DEBUG
# ifdef LOG
			syslog(LOG_INFO, "%s", &MsgBuf[4]);
# endif LOG
			SmtpState = SMTP_CLOSED;
			smtpquit(m);
			return (-1);
		}
		fixcrlf(SmtpReplyBuffer, TRUE);

		if (CurEnv->e_xfp != NULL && (int) strchr("45", SmtpReplyBuffer[0]) != (int)NULL)
		{
			/* serious error -- log the previous command */
			if (SmtpMsgBuffer[0] != '\0')
				fprintf(CurEnv->e_xfp, ">>> %s\n", SmtpMsgBuffer);
			SmtpMsgBuffer[0] = '\0';

			/* now log the message as from the other side */
			fprintf(CurEnv->e_xfp, "<<< %s\n", SmtpReplyBuffer);
		}

		/* display the input for verbose mode */
		if (Verbose && !HoldErrs)
			nmessage(Arpa_Info, "%s", SmtpReplyBuffer);

		/* if continuation is required, we can go on */
		if (SmtpReplyBuffer[3] == '-' || !isdigit(SmtpReplyBuffer[0]))
			continue;

		/* decode the reply code */
		r = atoi(SmtpReplyBuffer);

		/* extra semantics: 0xx codes are "informational" */
		if (r < 100)
			continue;

		/* reply code 421 is "Service Shutting Down" */
		if (r == SMTPCLOSING && SmtpState != SMTP_SSD)
		{
			/* send the quit protocol */
			SmtpState = SMTP_SSD;
			smtpquit(m);
		}

		/* save temporary failure messages for posterity */
		if (SmtpReplyBuffer[0] == '4' && SmtpError[0] == '\0')
			(void) strcpy(SmtpError, &SmtpReplyBuffer[4]);

		return (r);
	}
}
/*
**  SMTPMESSAGE -- send message to server
**
**	Parameters:
**		f -- format
**		m -- the mailer to control formatting.
**		a, b, c -- parameters
**
**	Returns:
**		none.
**
**	Side Effects:
**		writes message to SmtpOut.
*/

/*VARARGS1*/
smtpmessage(f, m, a, b, c)
	char *f;
	MAILER *m;
{
	(void) sprintf(SmtpMsgBuffer, f, a, b, c);
	if (
#ifdef DEBUG
	    tTd(18, 1) || 
#endif DEBUG
		          (Verbose && !HoldErrs))
		nmessage(Arpa_Info, ">>> %s", SmtpMsgBuffer);
	if (SmtpOut != NULL)
		fprintf(SmtpOut, "%s%s", SmtpMsgBuffer, 
			m == 0 ? "\r\n" : m->m_eol);
}
