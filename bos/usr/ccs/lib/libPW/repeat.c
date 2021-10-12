static char sccsid[] = "@(#)52	1.5  src/bos/usr/ccs/lib/libPW/repeat.c, libPW, bos411, 9428A410j 6/16/90 00:56:53";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: repeat
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
 
/*
	Set `result' equal to `str' repeated `repfac' times.
	Return `result'.
*/

char *repeat(result,str,repfac)
char *result, *str;
register unsigned repfac;
{
	register char *r, *s;

	r = result;
	for (++repfac; --repfac > 0; --r)
		for (s=str; *r++ = *s++; );
	*r = '\0';
	return(result);
}
