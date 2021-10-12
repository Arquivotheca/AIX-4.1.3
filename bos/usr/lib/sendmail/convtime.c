static char sccsid[] = "@(#)98	1.4  src/bos/usr/lib/sendmail/convtime.c, cmdsend, bos411, 9428A410j 4/21/91 17:04:15";
/* 
 * COMPONENT_NAME: CMDSEND convtime.c
 * 
 * FUNCTIONS: MSGSTR, PLURAL, convtime, pintvl 
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

# include <sys/types.h>
# include <ctype.h>
# include "useful.h"

#include <nl_types.h>
#include "sendmail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SENDMAIL,n,s) 

/*
**  CONVTIME -- convert time
**
**	Takes a time as an ascii string with a trailing character
**	giving units:
**	  s -- seconds
**	  m -- minutes
**	  h -- hours
**	  d -- days (default)
**	  w -- weeks
**	For example, "3d12h" is three and a half days.
**
**	Parameters:
**		p -- pointer to ascii time.
**
**	Returns:
**		time in seconds.
**
**	Side Effects:
**		none.
*/

long
convtime(p)
	char *p;
{
	register long t, r;
	register char c;

	r = 0;
	while (*p != '\0')
	{
		t = 0;
		while (isdigit(c = *p++))
			t = t * 10 + (c - '0');
		if (c == '\0')
			p--;
		switch (c)
		{
		  case 'w':		/* weeks */
			t *= 7;

		  case 'd':		/* days */
		  default:
			t *= 24;

		  case 'h':		/* hours */
			t *= 60;

		  case 'm':		/* minutes */
			t *= 60;

		  case 's':		/* seconds */
			break;
		}
		r += t;
	}

	return (r);
}
/*
**  PINTVL -- produce printable version of a time interval
**
**	Parameters:
**		intvl -- the interval to be converted
**		brief -- if TRUE, print this in an extremely compact form
**			(basically used for logging).
**
**	Returns:
**		A pointer to a string version of intvl suitable for
**			printing or framing.
**
**	Side Effects:
**		none.
**
**	Warning:
**		The string returned is in a static buffer.
*/

# define PLURAL(n)	((n) == 1 ? "" : "s")

char *
pintvl(intvl, brief)
	long intvl;
	int brief;
{
	static char buf[256];
	register char *p;
	int wk, dy, hr, mi, se;

	if (intvl == 0 && !brief)
		return (MSGSTR(CV_ZERO, "zero seconds")); /*MSG*/

	/* decode the interval into weeks, days, hours, minutes, seconds */
	se = intvl % 60;
	intvl /= 60;
	mi = intvl % 60;
	intvl /= 60;
	hr = intvl % 24;
	intvl /= 24;
	if (brief)
		dy = intvl;
	else
	{
		dy = intvl % 7;
		intvl /= 7;
		wk = intvl;
	}

	/* now turn it into a sexy form */
	p = buf;
	if (brief)
	{
		if (dy > 0)
		{
			(void) sprintf(p, "%d+", dy);
			p += strlen(p);
		}
		(void) sprintf(p, "%02d:%02d:%02d", hr, mi, se);
		return (buf);
	}

	/* use the verbose form */
	if (wk > 0)
	{
		if (wk == 1)
			(void) sprintf(p, MSGSTR(CV_WEEK, ", %d week"), wk); /*MSG*/
		else
			(void) sprintf(p, MSGSTR(CV_WEEKS, ", %d weeks"), wk); /*MSG*/
		p += strlen(p);
	}
	if (dy > 0)
	{
		if (dy == 1)
			(void) sprintf(p, MSGSTR(CV_DAY, ", %d day"), dy); /*MSG*/
		else
			(void) sprintf(p, MSGSTR(CV_DAYS, ", %d days"), dy); /*MSG*/
		p += strlen(p);
	}
	if (hr > 0)
	{
		if (hr == 1)
			(void) sprintf(p, MSGSTR(CV_HOUR, ", %d hour"), hr); /*MSG*/
		else
			(void) sprintf(p, MSGSTR(CV_HOURS, ", %d hours"), hr); /*MSG*/
		p += strlen(p);
	}
	if (mi > 0)
	{
		if (mi == 1)
			(void) sprintf(p, MSGSTR(CV_MIN, ", %d minute"), mi); /*MSG*/
		else
			(void) sprintf(p, MSGSTR(CV_MINS, ", %d minutes"), mi); /*MSG*/
		p += strlen(p);
	}
	if (se > 0)
	{
		if (se == 1)
			(void) sprintf(p, MSGSTR(CV_SEC, ", %d second"), se); /*MSG*/
		else
			(void) sprintf(p, MSGSTR(CV_SECS, ", %d seconds"), se); /*MSG*/
		p += strlen(p);
	}

	return (buf + 2);
}
