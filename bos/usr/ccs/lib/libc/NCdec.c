static char sccsid[] = "@(#)71	1.3  src/bos/usr/ccs/lib/libc/NCdec.c, libcnls, bos411, 9428A410j 2/26/91 12:43:16";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCdec
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

/*      This function converts one multi-byte character (1 or 2 bytes)
 *      to a wchar_t and returns the number of bytes converted.
 *      The ANSI equivalent is the mbtowc function.
 *	NOTE! This function is dependent on ctype.h definition of
 *	what constitutes a multi-byte character (NCisshift) which
 *      differs between NLS and JLS...
 */

#include <stdlib.h>
#include <ctype.h>

#ifdef NCdec
#undef NCdec
#endif

int NCdec(char *c, wchar_t *x)
{
	int rc;

	rc = mbtowc(x,c,MB_CUR_MAX);
	if (rc > 1)
		return(rc);
	else
		return(1);
}
