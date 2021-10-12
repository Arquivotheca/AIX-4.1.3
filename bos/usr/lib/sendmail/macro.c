static char sccsid[] = "@(#)14	1.3.1.1  src/bos/usr/lib/sendmail/macro.c, cmdsend, bos411, 9428A410j 10/12/91 14:38:27";
/* 
 * COMPONENT_NAME: CMDSEND macro.c
 * 
 * FUNCTIONS: freemac, define, expand, macvalue 
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

# include <stdio.h>
# include <ctype.h>
# include <memory.h>

# include "conf.h"
# include "useful.h"
# include <sys/types.h>
# include <netinet/in.h>

# include "sendmail.h"

char *macvalue();

/*
**  EXPAND -- macro expand a string using $x escapes.
**
**	Parameters:
**		s -- the string to expand.
**		buf -- the place to put the expansion.
**		buflim -- the buffer limit, i.e., the address
**			of the last usable position in buf.
**		e -- envelope in which to work.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

expand(s, buf, buflim, e)
	register char *s;
	register char *buf;
	char *buflim;
	register ENVELOPE *e;
{
	register char *xp;
	register char *q;
	int skipping;		/* set if conditionally skipping output */
	int recurse = FALSE;	/* set if recursion required */
	int i;
	char xbuf[BUFSIZ];

# ifdef DEBUG
	if (tTd(35, 24))
	{
		(void) printf("expand(");
		xputs(s);
		(void) printf(")\n");
	}
# endif DEBUG

	skipping = FALSE;
	if (s == NULL)
		s = "";
	for (xp = xbuf; *s != '\0'; s++)
	{
		char c;

		/*
		**  Check for non-ordinary (special?) character.
		**	'q' will be the interpolated quantity.
		*/

		q = NULL;
		c = *s;
		switch (c)
		{
		  case CONDIF:		/* see if var set */
			c = *++s;
			skipping = macvalue(c, e) == NULL;
			continue;

		  case CONDELSE:	/* change state of skipping */
			skipping = !skipping;
			continue;

		  case CONDFI:		/* stop skipping */
			skipping = FALSE;
			continue;

		  case '\001':		/* macro interpolation */
			c = *++s;
			q = macvalue(c & 0177, e);
			if (q == NULL)
				continue;
			break;
		}

		/*
		**  Interpolate q or output one character
		*/

		if (skipping || xp >= &xbuf[sizeof xbuf])
			continue;
		if (q == NULL)
			*xp++ = c;
		else
		{
			/* copy to end of q or max space remaining in buf */
			while ((c = *q++) != '\0' && xp < &xbuf[sizeof xbuf - 1])
			{
				if (iscntrl(c) && !isspace(c))
					recurse = TRUE;
				*xp++ = c;
			}
		}
	}
	*xp = '\0';

# ifdef DEBUG
	if (tTd(35, 24))
	{
		(void) printf("expand ==> ");
		xputs(xbuf);
		(void) printf("\n");
	}
# endif DEBUG

	/* recurse as appropriate */
	if (recurse)
	{
		expand(xbuf, buf, buflim, e);
		return;
	}

	/* copy results out */
	i = buflim - buf - 1;
	if (i > xp - xbuf)
		i = xp - xbuf;
	(void) memcpy(buf, xbuf, i);
	buf[i] = '\0';
}
/*
**  FREEMAC -- free and clear the definition of a macro.
**
**	Parameters:
**		n -- the macro name.
**		e -- the envelope that the macro is defined in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		the space allocated for the macro definition is freed and
**		the macro definition is cleared.
**
**	Notes:
**		NOTE that this assumes that a defined macro has been
**		previously malloc()ed; so BE CAREFUL when and where this
**		is called!!!
**
*/

void freemac(n, e)
char n;
register ENVELOPE *e;
{

register char *cp;

    if (cp = e->e_macro[n & 0177]) {  /* defined: assume it's been malloc()ed */

# ifdef DEBUG
	if (tTd(35, 9)) {
		(void) printf("freemac: freeing macro %c (%#x=", n, cp);
		xputs(cp);
		(void) printf(")\n");
	}
# endif DEBUG

	free(cp);
	e->e_macro[n & 0177] = NULL;
    }
}
/*
**  DEFINE -- define a macro.
**
**	this would be better done using a #define macro.
**
**	Parameters:
**		n -- the macro name.
**		v -- the macro value.
**		e -- the envelope to store the definition in.
**
**	Returns:
**		none.
**
**	Side Effects:
**		e->e_macro[n] is defined.
**
**	Notes:
**		There is one macro for each ASCII character,
**		although they are not all used.  The currently
**		defined macros are:
**
**		$a   date in ARPANET format (preferring the Date: line
**		     of the message)
**		$b   the current date (as opposed to the date as found
**		     the message) in ARPANET format
**		$c   hop count
**		$d   (current) date in UNIX (ctime) format
**		$e   the SMTP entry message+
**		$f   raw from address
**		$g   translated from address
**		$h   to host
**		$i   queue id
**		$j   official SMTP hostname, used in messages+
**		$l   UNIX-style from line+
**		$n   name of sendmail ("MAILER-DAEMON" on local
**		     net typically)+
**		$o   delimiters ("operators") for address tokens+
**		$p   my process id in decimal
**		$q   the string that becomes an address -- this is
**		     normally used to combine $g & $x.
**		$r   protocol used to talk to sender
**		$s   sender's host name
**		$t   the current time in seconds since 1/1/1970
**		$u   to user
**		$v   version number of sendmail
**		$w   our host name (if it can be determined)
**		$x   signature (full name) of from person
**		$y   the tty id of our terminal
**		$z   home directory of to person
**
**		Macros marked with + must be defined in the
**		configuration file and are used internally, but
**		are not set.
**
**		There are also some macros that can be used
**		arbitrarily to make the configuration file
**		cleaner.  In general all upper-case letters
**		are available.
*/

define(n, v, e)
	char n;
	char *v;
	register ENVELOPE *e;
{
# ifdef DEBUG
	if (tTd(35, 9))
	{
		(void) printf("define(%c as %#x=", n, v);
		xputs(v);
		(void) printf(")\n");
	}
# endif DEBUG
	e->e_macro[n & 0177] = v;
}
/*
**  MACVALUE -- return uninterpreted value of a macro.
**
**	Parameters:
**		n -- the name of the macro.
**
**	Returns:
**		The value of n.
**
**	Side Effects:
**		none.
*/

char *
macvalue(n, e)
	char n;
	register ENVELOPE *e;
{
	n &= 0177;
	while (e != NULL)
	{
		register char *p = e->e_macro[n];

		if (p != NULL)
			return (p);
		e = e->e_parent;
	}
	return (NULL);
}
