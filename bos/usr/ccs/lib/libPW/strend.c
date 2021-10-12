static char sccsid[] = "@(#)07	1.5  src/bos/usr/ccs/lib/libPW/strend.c, libPW, bos411, 9428A410j 6/16/90 00:57:14";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: strend
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
 
char *strend(p)
register char *p;
{
	while (*p++)
		;
	return(--p);
}
