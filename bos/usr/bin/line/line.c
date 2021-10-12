static char sccsid[] = "@(#)77	1.4  src/bos/usr/bin/line/line.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:12:19";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

#include <locale.h>
#include <unistd.h>

#define	LSIZE	512		/* line buffer size		*/

static char nl = '\n';			/* newline character		*/
static int EOF;			/* End Of File flag		*/
char readc();			/* character read function	*/

/*
 * NAME:	line
 *
 * SYNTAX:	line
 *
 * FUNCTION:	line - read one line from standard input
 *
 * NOTES:	This program reads a single line from the standard input
 *		and writes it on the standard output.  It is probably most
 *		useful in conjunction with the Bourne shell.
 *
 * RETURN VALUE DESCRIPTION:	1 on end-of-file, else 0
 */

int
main()
{
	register char c;		/* last character read	*/
	char line[LSIZE];		/* line buffer		*/
	register char *linep, *linend;	/* pointers within line	*/

	(void) setlocale (LC_ALL, "");

	EOF = 0;			/* initialize eof flag	*/
	linep = line;			/* init current pointer	*/
	linend = line + LSIZE;		/* point to end of line	*/

	/*
	 * loop reading 'till we hit eof or eol ...
	 */
	while ((c = readc()) != nl)
	{
		/*
		 * filled line up?
		 */
		if (linep == linend)
		{
			/*
			 * yep, write it & start over...
			 */
			(void) write (1, line, (unsigned)LSIZE);
			linep = line;
		}
		/*
		 * save the character we read
		 */
		*linep++ = c;
	}

	/*
	 * write last buffer (if anything there)
	 */
	if (linep > line)
		write (1, line, (unsigned)(linep-line));

	/*
	 * write newline
	 */
	write(1, &nl, (unsigned)1);

	/*
	 * exit 1 on eof, else 0
	 */
	exit(EOF);

	/* NOTREACHED */
}


/*
 * NAME:	readc
 *
 * FUNCTION:	readc - read a character and return it
 *
 * NOTES:	Readc reads a character from standard input (fd = 0)
 *		and returns it.  On end-of-file, a newline is returned
 *		and the end-of-file flag is turned on.
 *
 * DATA STRUCTURES:	EOF is set on end of file
 *
 * RETURN VALUE DESCRIPTION:	the character read is returned.  on
 *		end of file 'nl' is returned (newline character)
 */

static char
readc()
{
	char c;

	if (read (0, &c, (unsigned)1) != 1) {
		EOF = 1;
		c = nl;
	}

	return (c);
}
