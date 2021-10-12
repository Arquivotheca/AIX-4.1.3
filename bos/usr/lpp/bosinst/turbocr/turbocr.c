static char sccsid[] = "@(#)56 1.1 src/bos/usr/lpp/bosinst/turbocr/turbocr.c, bosinst, bos41J, 9514A_all 95/04/05 09:13:16";
/*
 * COMPONENT_NAME: BOSINST
 *
 *	turbocr - print copyrights for Turbo Install
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>

void fatal_error(const char *msg)
{
	perror(msg);
	exit(1);
}

/*
 *  read a line from standard input
 *
 *	returns: -1 if EOF encountered
 *		line length, otherwise (including the '\n')
 */
int readline(char *buf, int buflen)
{
	int	ch;			/* next input character */
	int	linelen = 0;

	while ((ch = getchar()) != EOF)  {
		*buf++ = ch;
		linelen++;
		if (ch == '\n')		/* end of line? */
			return linelen;
	}

	/* EOF encountered -- if there is a partial line, return it */
	if (linelen > 0)
		return linelen;
	else
		return -1;
}

/*
 *  check if this is a Package Name Line
 *
 *	returns: 1 if line starts with <<<
 *		 0, otherwise
 */
int packageline(char *buf, int linelen)
{
	if (strncmp(buf,"<<<",3) == 0)
		return 1;
	else
		return 0;
}

/*
 *  check if this is a number line
 *
 *	returns: 1 if line starts with space and then a number
 *		 0, otherwise
 */
int numberline(char *buf, int linelen)
{
	return (linelen >= 2) &&
		(buf[0] == ' ') && isdigit(buf[1]);
}

/*
 *  main program
 *
 *	Reads lines from stdin.  If the line begins with a "<<<" string
 *	sleep 3 seconds before continuing  and do not sleep on next 
 *      number line. If this is a number line, that follows a number line with
 *	no intervening package name line, the sleep.
 *	Print all lines to standard out.
 *	
 *
 */
int main()
{
	int	rc;			/* scratch return code */
	int	first_number = 1;	/* first number line? */
	char	linebuf[LINE_MAX];	/* current input line */
	int	linelen;		/* length of current line */

	while ((linelen = readline(linebuf, LINE_MAX)) >= 0)  {

		if (numberline(linebuf, linelen))  {
			if (first_number)
				first_number = 0;
			else
				(void) sleep(3); /* ignore signals */
		}
		else 
			if (packageline(linebuf, linelen)) {
				(void) sleep(3); /* ignore signals */
				first_number = 1;
			}
		/* print the line just read */
		rc = fwrite(linebuf, sizeof(char),
				linelen, stdout);
		if (rc != linelen)
			fatal_error("turbocr: write error on stdout");
	}
	return 0;
}
