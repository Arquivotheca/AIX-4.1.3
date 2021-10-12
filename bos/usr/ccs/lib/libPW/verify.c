static char sccsid[] = "@(#)74	1.5  src/bos/usr/ccs/lib/libPW/verify.c, libPW, bos411, 9428A410j 6/16/90 00:57:38";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: verify
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
	If `astr1' contains any characters not in `str2' return the
	offset of the first such character found in `astr1',
	else return -1.
*/

verify(astr1,str2)
char *astr1;
register char *str2;
{
	register char *str1;

	for (str1=astr1; *str1; str1++)
		if (!any(*str1,str2))
			return(str1 - astr1);
	return(-1);
}
