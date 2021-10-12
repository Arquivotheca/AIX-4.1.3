static char sccsid[] = "@(#)34  1.6  src/bos/usr/lib/sendmail/sysexits.c, cmdsend, bos411, 9428A410j 4/21/91 17:11:32";
/* 
 * COMPONENT_NAME: CMDSEND sysexits.c
 * 
 * FUNCTIONS: MSGSTR, statstring 
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

# include "sysexits.h"

/*
**  SYSEXITS.C -- error messages corresponding to sysexits.h
*/

char	*SysExMsg[] =
{
	/* 64 USAGE */		"500 Bad usage",
	/* 65 DATAERR */	"501 Data format error",
	/* 66 NOINPUT */	"550 Cannot open input",
	/* 67 NOUSER */		"550 User unknown",
	/* 68 NOHOST */		"550 Host unknown",
	/* 69 UNAVAILABLE */	"554 Service unavailable",
	/* 70 SOFTWARE */	"554 Internal error",
	/* 71 OSERR */		"451 Operating system error",
	/* 72 OSFILE */		"554 System file missing",
	/* 73 CANTCREAT */	"550 Can't create output",
	/* 74 IOERR */		"451 I/O error",
	/* 75 TEMPFAIL */	"250 Deferred",
	/* 76 PROTOCOL */	"554 Remote protocol error",
	/* 77 NOPERM */		"550 Insufficient permission",
	/* 78 CONFIG */		"554 Local configuration error",
	/* 79 DB */		"554 Local database file error",
	/* 80 NOLHOST */	"554 Local host not defined"
};

int	N_SysEx = sizeof SysExMsg / sizeof SysExMsg[0];
/*
**  STATSTRING -- return string corresponding to an error status
**
**	Parameters:
**		stat -- the status to decode.
**
**	Returns:
**		The string corresponding to that status
**
**	Side Effects:
**		none.
*/

char *
statstring(stat)
	int stat;
{
	static char ebuf[50];

	stat -= EX__BASE;
	if (stat < 0 || stat >= N_SysEx)
	{
		(void) sprintf(ebuf, MSGSTR(SY_USTAT, "554 Unknown status %d"), stat + EX__BASE); /*MSG*/
		return (ebuf);
	}

	return (SysExMsg[stat]);
}
