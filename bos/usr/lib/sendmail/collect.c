static char sccsid[] = "@(#)95	1.14.1.1  src/bos/usr/lib/sendmail/collect.c, cmdsend, bos411, 9428A410j 4/21/92 13:08:31";
/* 
 * COMPONENT_NAME: CMDSEND collect.c
 * 
 * FUNCTIONS: MSGSTR, collect, eatfrom, tferror, flusheol 
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

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <sys/stat.h>
# include <netinet/in.h>

# include "sendmail.h"
# include <string.h>
# include <sys/access.h>   /*  security  */

extern long  InputCount;
extern int   local_nls;

char *hvalue();
char *arpadate();
FILE  *dfopen ();
char  *xalloc ();
char  *queuename ();
char  *sfgets ();
char *te_df;
int  ttfError = FALSE;

/*
**  COLLECT -- read & parse message header & make temp file.
**
**	Creates a temporary file name and copies the standard
**	input to that file.  Leading UNIX-style "From" lines are
**	stripped off (after important information is extracted).
**
**	Parameters:
**		sayok -- if set, give an ARPANET style message
**			to say we are ready to collect input.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Temp file is created and filled.
**		The from person may be set.
*/

collect(sayok)
	int sayok;
{
	register FILE *tf;
	register FILE *ttf;
	char buf[MAXFIELD], buf2[MAXFIELD];
	register char *workbuf, *freebuf;
	register char *p, *q, *tmp;
	register int workbuflen;
	register int ic;
	register int eic;
	int usrerr(), syserr();
	int Binary=FALSE;
	int SavedFile;

	/*
	**  Create the temp file name and create the file.
	*/

	CurEnv->e_df = (char *)newstr(queuename(CurEnv, 'd'));
	if ((tf = dfopen(CurEnv->e_df, "w")) == NULL)
	{
		syserr(MSGSTR(CO_ECREAT, "Cannot create %s"), CurEnv->e_df); /*MSG*/
		NoReturn = TRUE;
		finis();
	}
	(void) acl_set(CurEnv->e_df, UMODE(FileMode), GMODE(FileMode),
	    OMODE(FileMode));

	te_df = newstr(queuename(CurEnv, 'B'));
	if ((ttf = dfopen(te_df, "w+b")) == NULL)
	{
		syserr(MSGSTR(CO_ECREAT, "Cannot create %s"), te_df); /*MSG*/
		NoReturn = TRUE;
		finis();
	}
	(void) acl_set(te_df, UMODE(FileMode), GMODE(FileMode),
	    OMODE(FileMode));

	/*
	**  Tell ARPANET to go ahead.
	*/

	if (sayok)
	    message("354", MSGSTR(CO_ENTER, "Enter mail, end with \".\" on a line by itself")); /*MSG*/

	/* If exec'd locally with -i & -x flags:
	** Save message into a temporary file while checking for an
	** embedded null.  If an embedded null exists, set flag
	** 'Binary' to TRUE so we can process the body of the 
	** message as though it is binary data.
	*/
	/* check for -i flag and the -x flag */
	if (SavedFile = (IgnrDot && (local_nls == TRUE))) {
		while((ic=fgetc(InChannel)) != EOF) {
			putc(ic,ttf);
			if (ferror(ttf)) {	/* even senses media full */
				ttfError = TRUE;
				tferror(ttf);
			}
		}
		if (ttfError)
			freopen(te_df, "r", ttf);
		else
			rewind(ttf);
		while((ic=fgetc(ttf)) != EOF)
			if ((char)ic == '\0') {
				Binary = TRUE; /* maybe */
				break;
			}
		rewind(ttf);
	}

	/*
	**  Try to read a UNIX-style From line
	*/

	InputCount = 0;			/* begin counting */

	(void) memset(buf, 'X', MAXFIELD);
	if (sfgets(buf, MAXFIELD, (SavedFile) ? ttf : InChannel) == NULL)
		goto readerr;
#ifdef DEBUG
	if (tTd(30, 20))
		(void) printf("collect: got string from sfgets='%s'\n", buf);
#endif DEBUG

	/* Don't worry about fixing crlf's if mail originated locally */
	if(!SavedFile)
		fixcrlf(buf, FALSE);

	if (!SaveFrom && strncmp(buf, "From ", 5) == 0)
	{
		if (!flusheol(buf, (SavedFile) ? ttf : InChannel))
			goto readerr;
		eatfrom(buf);
		(void) memset(buf, 'X', MAXFIELD);
		if (sfgets(buf, MAXFIELD, (SavedFile) ? ttf : InChannel) == NULL)
			goto readerr;
		if(!SavedFile)
			fixcrlf(buf, FALSE);
	}

	workbuf = buf;		/* workbuf contains a header field */
	freebuf = buf2;		/* freebuf can be used for read-ahead */
	for (;;)
	{

#ifdef DEBUG
	if (tTd(30, 20))
		(void) printf("collect: header read loop: buf='%s'\n", workbuf);
#endif DEBUG

		/* first, see if the header is over */
		if (!isheader(workbuf))
		{
			if(!SavedFile) 
				fixcrlf(workbuf, TRUE);
			break;
		}

		/*  If the line is too long, throw the rest of it away. */ 
		if (!flusheol(workbuf, (SavedFile) ? ttf : InChannel))
			goto readerr;
		 
		/* it's ok to toss '\n' now (flusheol() needed it) */
		fixcrlf(workbuf, TRUE);

		workbuflen = strlen(workbuf);

		/* get the rest of this field */
		for (;;)
		{
			(void) memset(freebuf, 'X', MAXFIELD);
			if (sfgets(freebuf, MAXFIELD, (SavedFile) ? ttf : InChannel) == NULL)
				goto readerr;
			/* is this a continuation line? */
			if (*freebuf != ' ' && *freebuf != '\t')
				break;
			if (!flusheol(freebuf, (SavedFile) ? ttf : InChannel))
				goto readerr;
		
			/* yes; append line to 'workbuf' if there's room */
			if (workbuflen < MAXFIELD - 3)
			{
				p = workbuf + workbuflen;
				q = freebuf;

				/* we have room for more of this field */
				fixcrlf(freebuf, TRUE);
				*p++ = '\n';
				workbuflen++;
				while (*q != '\0' && workbuflen < MAXFIELD - 1)
				{
					*p++ = *q++;
					workbuflen++;
				}
				*p = '\0';
			}
		}
		/*
		**  The working buffer now becomes the free buffer, since
		**  the free buffer contains a new header field.
		**
		**  This is premature, since we still haven't called
		**  chompheader() to process the field we just created
		**  (so the call to chompheader() will use 'freebuf').
		**  This convolution is necessary so that if we break out
		**  of the loop due to H_EOH, 'workbuf' will always be
		**  the next unprocessed buffer.
		*/

		{
			tmp = workbuf;
			workbuf = freebuf;
			freebuf = tmp;
		}

		/*
		**  Snarf header away.
		*/

		if (bitset(H_EOH, chompheader(freebuf, FALSE)))
			break;
	} 

#ifdef DEBUG
	if (tTd(30, 1))
		(void) printf("collect: EOH\n");
#endif DEBUG

	if ((SavedFile && (*workbuf == '\n' || *workbuf == '\r')) ||
	    !SavedFile && *workbuf == '\0')
	{

		/* throw away a blank line */
		(void) memset(buf, 'X', MAXFIELD);
		if (sfgets(buf, MAXFIELD, (SavedFile) ? ttf : InChannel) == NULL)
			goto readerr;
	}
	else if (workbuf == buf2)	/* guarantee 'buf' contains data */
		(void) strcpy(buf, buf2);


	/*
	**  Collect the body of the message.
	*/

	if (!Binary)
	  do
	  {
		register char *bp = buf;

#ifdef DEBUG
	if (tTd(30, 20))
		(void) printf("collect: body read loop: buf='%s'\n", buf);
#endif DEBUG

		fixcrlf(buf, TRUE);

		/* check for end-of-message */
		if (!IgnrDot && buf[0] == '.' && (buf[1] == '\n' || buf[1] == '\0'))
			break;

		/* check for transparent dot */
		if (OpMode == MD_SMTP && !IgnrDot && bp[0] == '.' && bp[1] == '.')
			bp++;

		/*
		 *  Output the line to the temp file, and insert a newline.
		 */

		(void) fputs(bp, tf);
		(void) fputs("\n", tf);
		if (ferror(tf))		/* even senses media full */
			tferror(tf);
	  } while (sfgets(buf, MAXFIELD, (SavedFile) ? ttf : InChannel) != NULL);
	else {
		/* Process the remainder of the body as binary data */
		
		/* flush buffer -- uses previous memset() call to find ending */
		/* NULL char in buffer, then copy remainder of message.       */

		/* The first thing fgets might see is a NULL char */
		if (buf[0] == '\0') {
			fputc('\0',tf);
			InputCount++;
		}
		else {
			for(eic=MAXFIELD-1; eic >0; eic--)
				if (buf[eic] == '\0')
					break;
			if (eic > 0)
				for (ic=0; ic<eic; ic++) {
					fputc(buf[ic],tf);
					InputCount++;
				}
		}
		while ((ic=fgetc(ttf)) != EOF) {
			fputc(ic, tf);
			InputCount++;
		}
		(void) fclose(ttf);
	  }

readerr:
	if (fflush(tf) != 0)
		tferror(tf);
	(void) fclose(tf);
	if (remove(te_df) != NULL)
		syserr(MSGSTR(AL_EULINK, "%s cannot be removed."), te_df); /*MSG*/

	CurEnv->e_msgsize = InputCount;

	/* An EOF when running SMTP is an error */
	if ((feof(InChannel) || ferror(InChannel)) && OpMode == MD_SMTP)
	{
#ifdef LOG
		if (RealHostName != NULL && LogLevel > 0)
			syslog(LOG_NOTICE, MSGSTR(CO_ECLOSE2, "collect: unexpected close on connection from %s: %m\n"),
			    CurEnv->e_from.q_paddr, RealHostName);
#endif
		(feof(InChannel) ? usrerr: syserr)
			(MSGSTR(CO_ECLOSE, "collect: unexpected close, from=%s"), CurEnv->e_from.q_paddr); /*MSG*/

		/* don't return an error indication */
		CurEnv->e_to = NULL;
		CurEnv->e_flags &= ~EF_FATALERRS;

		/* and don't try to deliver the partial message either */
		finis();
	}

	/*
	**  Find out some information from the headers.
	**	Examples are who is the from person & the date.
	*/

	eatheader(CurEnv);

	/*
	**  Add an Apparently-To: line if we have no recipient lines.
	*/

	if (hvalue("to") == NULL && hvalue("cc") == NULL &&
	    hvalue("bcc") == NULL && hvalue("apparently-to") == NULL)
	{
		register ADDRESS *q;

		/* create an Apparently-To: field */
		/*    that or reject the message.... */
		for (q = CurEnv->e_sendqueue; q != NULL; q = q->q_next)
		{
			if (q->q_alias != NULL)
				continue;
#ifdef DEBUG
			if (tTd(30, 3))
			    (void) printf("Adding Apparently-To: %s\n", 
								q->q_paddr);
#endif DEBUG
			addheader("apparently-to", q->q_paddr, CurEnv);
		}
	}

	/*
	**  Check for the "X-NLSesc:" header on the recieved message and
	**  set the body type if found.  If the body type is NLS, add the
	**  same header since the outgoing mail may be coverted.  This header
	**  will be stripped by putbody() if necessary.
	*/
	if (hvalue("x-nlsesc") != NULL)
		CurEnv->e_btype = BT_ESC;
	else if (CurEnv->e_btype == BT_NLS)
		addheader("x-nlsesc", "NLS mail body created at \001j.", CurEnv);
#ifdef	DEBUG
	if (tTd(66, 1))
		(void) printf("collect: bodytype=%d\n", CurEnv->e_btype);
#endif


	if ((CurEnv->e_dfp = fopen(CurEnv->e_df, "r")) == NULL)
		syserr(MSGSTR(CO_EREOPEN, "Cannot reopen %s"), CurEnv->e_df); /*MSG*/
}
/*
**  FLUSHEOL -- if not at EOL, throw away rest of input line.
**
**	Parameters:
**		buf -- last line read in (checked for '\n').
**		fp -- file to be read from.
**
**	Returns:
**		FALSE on error from sfgets(), TRUE otherwise.
**
**	Side Effects:
**		none.
*/

int
flusheol(buf, fp)
	char *buf;
	FILE *fp;
{
	char junkbuf[MAXLINE];
	register char *p;

	p = buf;
	while (strchr(p, '\n') == NULL)
	{
		(void) memset(junkbuf, 'X', MAXLINE);
		if (sfgets(junkbuf, MAXLINE, fp) == NULL)
			return(FALSE);
		p = junkbuf;
	}
	return(TRUE);
}
/*
**  TFERROR -- signal error on writing the temporary file.
**
**	Parameters:
**		tf -- the file pointer for the temporary file.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Gives an error message.
**		Arranges for following output to go elsewhere.
*/

tferror(tf)
	FILE *tf;
{
	if (errno == ENOSPC)
	{
		if (ttfError)
			(void) freopen(te_df, "w", tf);
		else
			(void) freopen(CurEnv->e_df, "w", tf);
		(void) fputs(MSGSTR(CO_EDEL, "\nMAIL DELETED BECAUSE OF LACK OF DISK SPACE\n\n"), /*MSG*/
									    tf);
		usrerr(MSGSTR(CO_ESPACE, "452 Out of disk space for temp file")); /*MSG*/
	}
	else
		syserr(MSGSTR(CO_EWRITE, "collect: Cannot write %s"), ttfError ? te_df : CurEnv->e_df); /*MSG*/
	(void) freopen("/dev/null", "w", tf);
}
/*
**  EATFROM -- chew up a UNIX style from line and process
**
**	This does indeed make some assumptions about the format
**	of UNIX messages.
**
**	Parameters:
**		fm -- the from line.
**
**	Returns:
**		none.
**
**	Side Effects:
**		extracts what information it can from the header,
**		such as the date.
*/

char	*DowList[] =
{
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
};

char	*MonthList[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	NULL
};

eatfrom(fm)
	char *fm;
{
	register char *p;
	register char **dt;

#ifdef DEBUG
	if (tTd(30, 2))
		(void) printf("eatfrom(%s)\n", fm);
#endif DEBUG

	/* find the date part */
	p = fm;
	while (*p != '\0')
	{
		/* skip a word */
		while (*p != '\0' && *p != ' ')
			p++;
		while (*p == ' ')
			p++;
		if (!isupper(*p) || p[3] != ' ' || p[13] != ':' || p[16] != ':')
			continue;

		/* we have a possible date */
		for (dt = DowList; *dt != NULL; dt++)
			if (strncmp(*dt, p, 3) == 0)
				break;
		if (*dt == NULL)
			continue;

		for (dt = MonthList; *dt != NULL; dt++)
			if (strncmp(*dt, &p[4], 3) == 0)
				break;
		if (*dt != NULL)
			break;
	}

	if (*p != (char)NULL)
	{
		char *q;

		/* we have found a date */
		q = xalloc(25);
		(void) strncpy(q, p, 25);
		q[24] = '\0';
		define('d', q, CurEnv);
		define('a', arpadate(q), CurEnv);
	}
}
