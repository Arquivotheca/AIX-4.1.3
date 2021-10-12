static char sccsid[] = "@(#)63	1.5  src/bos/usr/ccs/lib/libPW/xwrite.c, libPW, bos411, 9428A410j 6/16/90 00:58:12";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xwrite
 *
 * ORIGINS: 3 27
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */
 
# include "errno.h"

/*
	Interface to write which handles
	all error conditions.
	Returns number of bytes written on success,
	returns fatal(<mesg>) on failure.
*/

xwrite(fildes,buffer,nbytes)
char *buffer;
{
	register int n;

	if ((n = nbytes) > 0 && write(fildes,buffer,n) != n)
		return(xmsg("","xwrite"));
	return(n);
}
