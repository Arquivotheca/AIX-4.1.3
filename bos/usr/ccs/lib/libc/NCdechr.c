static char sccsid[] = "@(#)66	1.1.1.2  src/bos/usr/ccs/lib/libc/NCdechr.c, libcnls, bos411, 9428A410j 2/26/91 12:43:19";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCdechr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This function converts 1 or 2 chars to a "wchar_t" and returns it. */


/*	NOTE! This function is dependent on ctype.h definition of
 *	what constitutes a multi-byte character (NCisshift) which
 *      differs between NLS and JLS...
 */
#include <stdlib.h>
#include <ctype.h>

#ifdef NCdechr
#undef NCdechr
#endif

int NCdechr(char *c)
{
	wchar_t	pc;

	(void) mbtowc(&pc, c, MB_CUR_MAX);

	return((int)pc);
}
