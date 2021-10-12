static char sccsid[] = "@(#)66	1.7  src/bos/usr/bin/echo/echo.c, cmdsh, bos411, 9435A411a 8/25/94 19:00:20";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS: echo
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
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

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

/*
 * NAME:	echo
 *
 * FUNCTION:	echo - write arguments to standard output
 *
 * SYNOPSIS:	echo [string ...]
 *
 * NOTES:	Echo writes its arguments to standard output.
 *		Escape characters are recognized and processed.
 *
 * RETURN VALUE DESCRIPTION:
 *		 0	successful completion
 *		>0	an error occurred
 *
 * Standards:	XPG4
 *
 */

int main(argc, argv)
int argc;
char *argv[];
{
	register char *cp;
	register int  i;
	int mbcnt, j;
	wchar_t wd;
	
	(void) setlocale (LC_ALL, "");
	
	/*
	 * any arguments at all?  if not, exit
	 */
	if(--argc == 0) {
		putchar('\n');
		return (0);
	}
	
	/*
	 * process arguments ...
	 */
	for(i = 1; i <= argc; i++) {
		for(cp = argv[i]; *cp; cp += (mbcnt > 0) ? mbcnt : 1) {
			mbcnt = mbtowc(&wd, cp, MB_CUR_MAX);
			if (mbcnt==1 && wd == '\\')
				/*
				 * process escape sequences
				 */
				switch(*++cp) {
				      case 'a':
					putchar('\a');
					continue;

				      case 'b':	/* backspace	*/
					putchar('\b');
					continue;
					
				      case 'c':	/* no newline	*/
					return (0);
					
				      case 'f':	/* formfeed	*/
					putchar('\f');
					continue;
					
				      case 'n':	/* newline	*/
					putchar('\n');
					continue;
					
				      case 'r':	/* carriage return */
					putchar('\r');
					continue;

				      case 'v':
					putchar('\v');
					continue;
					
				      case 't':	/* tab	*/
					putchar('\t');
					continue;
					
				      case '\\':	/* backslash	*/
					putchar('\\');
					continue;
					
				      case '0':	/* octal char	*/
					j = wd = 0;
					while ((*++cp >= '0' &&
						*cp <= '7') && j++ < 3) {
						wd <<= 3;
						wd |= (*cp - '0');
					}
					putchar(wd);
					--cp;
					continue;
					
				      default:
					cp--;
					
				}
			
			putwchar(wd);	
		}
		
		/*
		 * space between arguments...
		 */
		if (i < argc)
			putchar(' ');
	}
	
	/*
	 * ending newline
	 */
	putchar('\n');
	
	return (0);
}
