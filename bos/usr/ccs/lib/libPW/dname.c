static char sccsid[] = "@(#)09	1.6  src/bos/usr/ccs/lib/libPW/dname.c, libPW, bos411, 9428A410j 6/2/91 22:12:49";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: dname
 *
 * ORIGINS: 3
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 * 
 */

# include	"macros.h"

/*
	Returns directory name containing a file
	(by modifying its argument).
	Returns "." if current
	directory; handles root correctly.
	Returns its argument.
	Bugs: doesn't handle null strings correctly.
*/

char *dname(p)
char *p;
{
	register char *c;
	register int s;

	s = size(p);
	for(c = p+s-2; c > p; c--)
		if(*c == '/') {
			*c = '\0';
			return(p);
		}
	if (p[0] != '/')
		p[0] = '.';
	p[1] = 0;
	return(p);
}
