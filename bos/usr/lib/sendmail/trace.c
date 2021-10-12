static char sccsid[] = "@(#)37	1.6  src/bos/usr/lib/sendmail/trace.c, cmdsend, bos411, 9428A410j 6/15/90 23:26:48";
/* 
 * COMPONENT_NAME: CMDSEND trace.c
 * 
 * FUNCTIONS: tTflag, tTsetup 
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

# include "conf.h"

# ifdef DEBUG

# include <ctype.h>
# include <stdio.h>
# include <ctype.h>

# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>

# include "sendmail.h"

/*
**  TtSETUP -- set up for trace package.
**
**	Parameters:
**		vect -- pointer to trace vector.
**		size -- number of flags in trace vector.
**		defflags -- flags to set if no value given.
**
**	Returns:
**		none
**
**	Side Effects:
**		environment is set up.
*/

static unsigned char  *tTvect;
static           int   tTsize;
static          char  *DefFlags;

tTsetup(vect, size, defflags)
	unsigned char *vect;
	int size;
	char *defflags;
{
	tTvect = vect;
	tTsize = size;
	DefFlags = defflags;
}
/*
**  TtFLAG -- process an external trace flag description.
**
**	Parameters:
**		s -- the trace flag.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sets/clears trace flags.
*/

tTflag(s)
	register char *s;
{
	int first = 0, last = 0;
	register int i;

	if (*s == '\0')
		s = DefFlags;

	for (;;)
	{
		/* find first flag to set */
		i = 0;
		while (isdigit(*s))
			i = i * 10 + (*s++ - '0');
		first = i;

		/* find last flag to set: if not specified, set it to
		   first one */
		i = 0;
		if (*s == '-')
			while (isdigit(*++s))
				i = i * 10 + (*s - '0');
		last = (i ? i : first);

		/* find the level to set it to */
		i = 1;
		if (*s == '.')
		{
			i = 0;
			while (isdigit(*++s))
				i = i * 10 + (*s - '0');
		}

		/* clean up args */
		if (first >= tTsize)
			first = tTsize - 1;
		if (last >= tTsize)
			last = tTsize - 1;

		/* set the flags */
		while (first <= last) {
#ifndef LOCAL_DEBUG  /* only set the allowed flag */
			if (first == ALLOWED)
#endif
				tTvect[first] = i;
			++first;
		}

		/* more arguments? */
		if (*s++ == '\0')
			return;
	}
}

#endif DEBUG
