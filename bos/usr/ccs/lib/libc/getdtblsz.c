static char sccsid[] = "@(#)68	1.4  src/bos/usr/ccs/lib/libc/getdtblsz.c, libcio, bos411, 9428A410j 3/4/94 10:28:07";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: getdtablesize 
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/limits.h>

/*
 * getdtablesize - returns size of file descriptor table, i.e.
 *	how many file descriptors you can open.
 *
 * 	getd(escriptor)t(a)bl(e)s(i)z(e) 
 */

getdtablesize(void)
{
	return(OPEN_MAX);
}

/* end of getdtblsz.c */
