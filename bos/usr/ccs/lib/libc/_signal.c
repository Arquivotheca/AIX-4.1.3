static char sccsid[] = "@(#)90	1.1  src/bos/usr/ccs/lib/libc/_signal.c, libcproc, bos411, 9428A410j 2/26/91 17:47:16";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: _signal
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * NOTES: This provides Fortran with the _signal alias entry point
 *	for the signal function.  The normal signal code is included
 *	with __SIGNAL defined which causes it to compile as _signal.
 *	This approach was taken rather than calling signal from here
 *	to make sure Fortran links with this routine.  Otherwise, a
 *	relinked program could end up calling the wrong signal!
 */

#define __SIGNAL
#include "signal.c"
