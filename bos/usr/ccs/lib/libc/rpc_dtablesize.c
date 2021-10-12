static char sccsid[] = "@(#)19  1.1  src/bos/usr/ccs/lib/libc/rpc_dtablesize.c, libcrpc, bos411, 9428A410j 10/25/93 20:54:02";
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: _rpc_dtablesize
 *
 *   ORIGINS: 24,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)rpcdtablesize.c	1.3 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.3 88/02/08 
 * Used to be in file: rpc_dtablesize.c
 */


/*
 * Cache the result of getdtablesize(), so we don't have to do an
 * expensive system call every time.
 */
_rpc_dtablesize()
{
	static int size;
	
	if (size == 0) {
		size = getdtablesize();
	}
	return (size);
}
