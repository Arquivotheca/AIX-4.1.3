static char sccsid[] = "@(#)29	1.6  src/bos/usr/ccs/lib/libtermcap/tgoto.c, libtermcap, bos411, 9428A410j 6/16/90 02:41:24";
/*
 * COMPONENT_NAME: (LIBTERMC) Termcap Library 
 *
 * FUNCTIONS: tgoto
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#define	CTRL(c)	('c' & 037)

char	*UP;
char	*BC;

/*
 * Routine to perform cursor addressing.
 * CM is a string containing printf type escapes to allow
 * cursor addressing.  We start out ready to print the destination
 * line, and switch each time we print row or column.
 * The following escapes are defined for substituting row/column:
 *
 *	%d	as in printf
 *	%2	like %2d
 *	%3	like %3d
 *	%.	gives %c hacking special case characters
 *	%+x	like %c but adding x first
 *
 *	The codes below affect the state but don't use up a value.
 *
 *	%>xy	if value > x add y
 *	%r	reverses row/column
 *	%i	increments row/column (for one origin indexing)
 *	%%	gives %
 *	%B	BCD (2 decimal digits encoded in one byte)
 *	%D	Delta Data (backwards bcd)
 *
 * all other characters are ``self-inserting''.
 */
char *
tgoto(CM, destcol, destline)
char *CM;                                    /* string returned from tgetstr */ 
int destcol, destline;             /* destination column and destination line */
{
	static char result[16];
	static char added[10];
	char *cp = CM;
	char *dp = result;
	int c;
	int oncol = 0;
	int which = destline;

	if (cp == 0) {       /* if string empty return */
		return ("\0");
	}
	added[0] = 0;
	while (c = *cp++) {
		if (c != '%') {
			*dp++ = c;
			continue;
		}
		switch (c = *cp++) {
		case 'd':
			if (which < 10)
			{
				*dp++ = which % 10 | '0';
				oncol = 1 - oncol;
				which = oncol ? destcol : destline;
				continue;
			}
			if (which < 100)
			{
				*dp++ = which / 10 | '0';
				*dp++ = which % 10 | '0';
				oncol = 1 - oncol;
				which = oncol ? destcol : destline;
				continue;
			}
			/* fall into... */
		case '3':
			*dp++ = (which / 100) | '0';
			which %= 100;
			/* fall into... */

		case '2':
			*dp++ = which / 10 | '0';
			*dp++ = which % 10 | '0';
			oncol = 1 - oncol;
			which = oncol ? destcol : destline;
			continue;
		case '+':
			which += *cp++;
			/* fall into... */

		case '.':
		/*
		 * This code is worth scratching your head at for a
		 * while.  The idea is that various weird things can
		 * happen to nulls, EOT's, tabs, and newlines by the
		 * tty driver, arpanet, and so on, so we don't send
		 * them if we can help it.
		 *
		 * Tab is taken out to get certain terminals to work, otherwise
		 * when they go to column 9 we increment which is wrong
		 * because bcd isn't continuous.  We should take out
		 * the rest too, or run the thing through more than
		 * once until it doesn't make any of these, but that
		 * would make termlib bigger, and also somewhat slower.  
                 * This requires all programs which use termlib to stty 
                 * tabs so they don't get expanded.  They should do 
                 * this anyway because some terminals use ^I for other 
                 * things, like nondestructive space.
		 */
			if (which == 0 || which == CTRL(d) || which == '\t' 
							   || which == '\n') {
				if(oncol || UP)/* Assumption: backspace works */
				/*
				 * Loop needed because newline happens
				 * to be the successor of tab.
				 */
				   do {
					strcat(added,oncol ? (BC ? BC:"\b"):UP);
					which++;
			           } while (which == '\n');
			}
			*dp++ = which;
			oncol = 1 - oncol;
			which = oncol ? destcol : destline;
			continue;

		case 'r':
			oncol = 1;
			which = oncol ? destcol : destline;
			continue;

		case 'i':
			destcol++;
			destline++;
			which++;
			continue;

		case '%':
			*dp++ = c;
			continue;
		default:
			return ("\0");     /* too hard */
		}
	}
	strcpy(dp, added);
	return (result);
}
