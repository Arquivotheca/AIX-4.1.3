static char sccsid[] = "@(#)40	1.5  src/bos/usr/ccs/lib/libPW/xpipe.c, libPW, bos411, 9428A410j 6/16/90 00:58:04";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS:  xpipe
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
	Interface to pipe(II) which handles all error conditions.
	Returns 0 on success,
	fatal() on failure.
*/

xpipe(t)
int *t;
{
	static char p[]="pipe";

	if (pipe(t) == 0)
		return(0);
	return(xmsg(p,p));
}
