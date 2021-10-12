static char sccsid[] = "@(#)10        1.12.2.2  src/bos/usr/lib/sendmail/err.c, cmdsend, bos411, 9431A411a 8/2/94 17:58:47";
/* 
 * COMPONENT_NAME: CMDSEND err.c
 * 
 * FUNCTIONS: MSGSTR, errstring, fmtmsg, message, nmessage, puterrmsg, 
 *            putmsg, syserr, usrerr 
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
# include <iconv.h>
# include <langinfo.h>
# include <stdlib.h>
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

# include <stdio.h>
# include <ctype.h>
# include <sys/types.h>
# include <sys/stat.h>

# include "conf.h"
# include "useful.h"

# include <sys/uio.h>
# include <sys/socket.h>
# include <netinet/in.h>

# include "sysexits.h"

# include "sendmail.h"
# include <errno.h>
# include <netdb.h>
# include <string.h>

char *errstring();

extern char Arpa_PSyserr[];
extern char Arpa_TSyserr[];
extern char Arpa_Usrerr[];
extern int sys_nerr;
char *SmtpPhase;
int QuickAbort = FALSE;
int QuickMsgs = 0;

/*
**  SYSERR -- Print error message.
**
**	Prints an error message via printf to the diagnostic
**	output.  If LOG is defined, it logs it also.
**
**	Parameters:
**		f -- the format string
**		a, b, c, d, e -- parameters
**
**	Returns:
**		none
**
**	Side Effects:
**		increments Errors.
**		sets ExitStat.
*/

char	MsgBuf[BUFSIZ*4];	/* text of most recent message */
int     SyserrLog = -1;

/*VARARGS1*/
syserr(fmt, a, b, c, d, e)
	char *fmt;
{
	register char *p;
	int olderrno = errno;
	int syserrlog;

	/* format and output the error message */
	if (olderrno == 0)
		p = Arpa_PSyserr;
	else
		p = Arpa_TSyserr;
	fmtmsg(MsgBuf, (char *) NULL, p, olderrno, fmt, a, b, c, d, e);
	puterrmsg(MsgBuf);

	/* determine exit status if not already set */
	if (ExitStat == EX_OK)
	{
		if (olderrno == 0)
			ExitStat = EX_SOFTWARE;
		else
			ExitStat = EX_OSERR;
	}

# ifdef LOG
	if (SyserrLog >= 0)
	{
		syserrlog = SyserrLog;
		SyserrLog = -1;
	}
	else
		syserrlog = LOG_ERR;

	syslog(syserrlog, "%s: SYSERR: %s",
			CurEnv->e_id == NULL ? "NOQUEUE" : CurEnv->e_id,
			&MsgBuf[4]);
# endif LOG
	errno = 0;
}
/*
**  USRERR -- Signal user error.
**
**	This is much like syserr except it is for user errors.
**
**	Parameters:
**		fmt, a, b, c, d -- printf strings
**
**	Returns:
**		none
**
**	Side Effects:
**		increments Errors.
*/

/*VARARGS1*/
usrerr(fmt, a, b, c, d, e)
	char *fmt;
{
	if (SuprErrs)                   /* controlled by envelope */
		return;

	fmtmsg(MsgBuf, CurEnv->e_to, Arpa_Usrerr, errno, fmt, a, b, c, d, e);
	puterrmsg(MsgBuf);
}
/*
**  MESSAGE -- print message (not necessarily an error)
**
**	Parameters:
**		num -- the default ARPANET error number (in ascii)
**		msg -- the message (printf fmt) -- if it begins
**			with a digit, this number overrides num.
**		a, b, c, d, e -- printf arguments
**
**	Returns:
**		none
**
**	Side Effects:
**		none.
*/

/*VARARGS2*/
message(num, msg, a, b, c, d, e)
	register char *num;
	register char *msg;
{
	errno = 0;
	fmtmsg(MsgBuf, CurEnv->e_to, num, 0, msg, a, b, c, d, e);
	putmsg(MsgBuf, FALSE);
}
/*
**  NMESSAGE -- print message (not necessarily an error)
**
**	Just like "message" except it never puts the to... tag on.
**
**	Parameters:
**		num -- the default ARPANET error number (in ascii)
**		msg -- the message (printf fmt) -- if it begins
**			with three digits, this number overrides num.
**		a, b, c, d, e -- printf arguments
**
**	Returns:
**		none
**
**	Side Effects:
**		none.
*/

/*VARARGS2*/
nmessage(num, msg, a, b, c, d, e)
	register char *num;
	register char *msg;
{
	errno = 0;
	fmtmsg(MsgBuf, (char *) NULL, num, 0, msg, a, b, c, d, e);
	putmsg(MsgBuf, FALSE);
}
/*
**  PUTMSG -- output error message to transcript and channel
**
**	Parameters:
**		msg -- message to output (in SMTP format).
**		holdmsg -- if TRUE, don't output a copy of the message to
**			our output channel.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Outputs msg to the transcript.
**		If appropriate, outputs it to the channel.
**		Deletes SMTP reply code number as appropriate.
*/

putmsg(msg, holdmsg)
	char *msg;
	int holdmsg;
{
	iconv_t	convd;
	char	outbuf[BUFSIZ];

#ifdef DEBUG
	if (tTd (73, 20))
	    (void) printf("putmsg: holdmsg=%d, msg='%s'\n", holdmsg, msg);
#endif DEBUG

	/* output to transcript if serious */
	if (CurEnv->e_xfp != NULL && (msg[0] == '4' || msg[0] == '5'))
		(void) fprintf(CurEnv->e_xfp, "%s\n", msg);

	/* output to channel if appropriate */
	if (!holdmsg && (Verbose || msg[0] != '0'))
	{
		(void) fflush(stdout);
		if (OpMode == MD_SMTP || OpMode == MD_ARPAFTP) {
		    if (!*NetCode && !*MailCode) {
			if (OutChannel != stdout) {
				/* We will flatten the text since it is 
				 * going out to another (possibly non-AIX)
				 * system.
				 */
				convd = iconv_open("fold7", nl_langinfo(CODESET));
				if (convd != (-1)) {
					char  *outmsg = outbuf;
					size_t inbytes, outbytes;
					int ret;
					
					inbytes = strlen(msg) + 1;
					outbytes = sizeof(outbuf);
					ret = iconv(convd, &msg, &inbytes, &outmsg, &outbytes);
   /* I think we should set msg = outbuf regardless of the return code, and
      if ret < 0 we should syslog an error message and make sure there is a null at
      the end of outbuf.  iconv seems to translate as much as possible and then
      modify msg to point at the place where it ran out of space.  If we print at least
     the first part of the message, the other side will see the 3 digit number and the
     protocol will probably still work. */
					msg = outbuf;
					if ( ret < 0 ){
						outbuf[sizeof(outbuf)-1] = (char)NULL;
						syslog(LOG_ERR, "sendmail: error number %d from iconv",
							errno);
					}
				}
			}
		     }
#ifdef DEBUG
			if (tTd (73, 20))
			    (void) printf("putmsg: after flatstr: msg='%s'\n",
				msg);
#endif DEBUG

			(void) fprintf(OutChannel, "%s\r\n", msg);
		} else
			(void) fprintf(OutChannel, "%s\n", &msg[4]);
		(void) fflush(OutChannel);
	}
} 
/*
**  PUTERRMSG -- like putmsg, but does special processing for error messages
**
**	Parameters:
**		msg -- the message to output.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Sets the fatal error bit in the envelope as appropriate.
*/

puterrmsg(msg)
	char *msg;
{
	FILE  *fp;
	struct stat  buf;
	static int  first = 1;
	static int  null_st_dev, null_st_ino;

	/*
	 *  The following code only allows one message at most to be generated
	 *  in response to an SMTP command.
	 */
	if (QuickAbort)         /* controlled by srvrsmtp */
	{
	    QuickMsgs++;
	    if (QuickMsgs > 1)
		return;
	}

	/* output the message as usual */
	putmsg(msg, HoldErrs);

	/* signal the error */
	Errors++;
	if (msg[0] == '5')
		CurEnv->e_flags |= EF_FATALERRS;

	/*
	 *  Output to console if disconnected from any TTY (this means daemon),
	 *  provided there is either no transcript, or the transcript is set
	 *  to /dev/null.
	 *
	 *  A real transcript probably means that this error is going to be 
	 *  mailed back to the sender.
	 *
	 *  This is a heuristic kloodge to get errors from the daemon that
	 *  we need to see and might otherwise miss, especially configuration
	 *  and initialization errors.  Perhaps someone who understands the
	 *  reporting system can make this possible in a more natural way.
	 */

	/*
	 *  First, check to see if we are daemon, in any form.  If not, exit.
	 */
	if (getpgrp () != MotherPid)
	    return;

	/*
	 *  Get device/inode parameters to uniquely identy /dev/null.
	 *  If this doesn't work, print on /dev/console anyway.
	 */
	if (first)			/* need parms for /dev/null?	*/
	{
	    if (stat ("/dev/null", &buf) == 0)
	    {
		null_st_dev = buf.st_dev;	/* save the parms	*/
		null_st_ino = buf.st_ino;

		first = 0;		/* prevent recurrence if it worked */
	    }
	}

	/*
	 *  If we recovered the /dev/null parms, check them against the current
	 *  transcript file for a match.  A match, or no transcript means use 
	 *  /dev/console.  Else, return.
	 */
	if (!first && (fp = CurEnv->e_xfp) != NULL)
	{
	    if (fstat (fileno (fp), &buf) == 0)
	    {
		if (buf.st_dev != null_st_dev || buf.st_ino != null_st_ino)
		    return;
	    }
	}

	/*
	 *  Write msg to console, if possible.
	 */
	fp = fopen ("/dev/console", "a");
	if (fp != NULL)
	{
	    (void) fprintf (fp, "sendmail daemon: %s\r\n", &msg[4]);
	    (void) fclose (fp);
	}
}
/*
**  FMTMSG -- format a message into buffer.
**
**	Parameters:
**		eb -- error buffer to get result.
**		to -- the recipient tag for this message.
**		num -- arpanet error number.
**		en -- the error number to display.
**		fmt -- format of string.
**		a, b, c, d, e -- arguments.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

/*VARARGS5*/
fmtmsg(eb, to, num, eno, fmt, a, b, c, d, e)
	register char *eb;
	char *to;
	char *num;
	int eno;
	char *fmt;
{
	char del;
	char hlds[NL_TEXTMAX], *hldp;

	/* preserve the string since we make more calls to MSGSTR */
	strcpy(hlds, fmt);
	hldp = hlds;

	/* output the reply code */
	if (isdigit(hlds[0]) && isdigit(hlds[1])
            && isdigit(hlds[2]))               
	{
		num = hldp;
		hldp += 4;
	}
	if (num[3] == '-')
		del = '-';
	else
		del = ' ';
	(void) sprintf(eb, "%3.3s%c", num, del);
	eb += 4;

	/* output the file name and line number */
	if (FileName != NULL)
	{
		(void) sprintf(eb, MSGSTR(DL_LINE, "%s: line %d: "), FileName, LineNumber); /*MSG*/
		eb += strlen(eb);
	}

	/* output the "to" person */
	if (to != NULL && to[0] != '\0')
	{
		(void) sprintf(eb, "%s... ", to);
		eb += strlen (eb);
	}

	/* output the message */
	(void) sprintf(eb, hldp, a, b, c, d, e);
	eb += strlen (eb);

	/* output the error code, if any */
	if (eno != 0)
	{
		(void) sprintf(eb, ": %s", errstring(eno));
		eb += strlen(eb);
	}
}
/*
**  ERRSTRING -- return string description of error code
**
**	Parameters:
**		err -- the error number to translate
**
**	Returns:
**		A string description of err.
**
**	Side Effects:
**		none.
*/

char *
errstring (err)
	int err;
{
	static char buf[MAXNAME];

	/*
	**  Handle special network error codes.
	**
	**	These are 4.2/4.3bsd specific; they should be in daemon.c.
	*/

	switch (err)
	{
	  case ETIMEDOUT:
	  case ECONNRESET:
		(void) strcpy(buf, strerror(err));
		if (SmtpPhase != NULL)
		{
			(void) strcat(buf, MSGSTR(ER_DUR, " during ")); /*MSG*/
			(void) strcat(buf, SmtpPhase);
		}
		if (CurHostName != NULL)
		{
			(void) strcat(buf, MSGSTR(ER_WITH, " with ")); /*MSG*/
			(void) strcat(buf, CurHostName);
		}
		return (buf);

	  case EHOSTDOWN:
		if (CurHostName == NULL)
			break;
		(void) sprintf(buf, MSGSTR(ER_EHOST, "Host %s is down"), CurHostName); /*MSG*/
		return (buf);

	  case ECONNREFUSED:
		if (CurHostName == NULL)
			break;
		(void) sprintf(buf, MSGSTR(EN_ECONN, "Connection refused by %s"), CurHostName); /*MSG*/
		return (buf);

	  case (TRY_AGAIN+MAX_ERRNO):
		(void) sprintf(buf, MSGSTR(EN_EHOST, "Host Name Lookup Failure")); /*MSG*/
		return (buf);
	}

	if (err > 0 && err < sys_nerr)
		return (strerror(err));

	(void) sprintf(buf, MSGSTR(EN_ERROR, "Error %d"), err); /*MSG*/
	return (buf);
}
