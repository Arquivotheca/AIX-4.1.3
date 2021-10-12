static char sccsid[] = "@(#)43	1.3  src/bos/usr/ccs/lib/libc/wstrchr.c, libcnls, bos411, 9428A410j 6/9/91 17:14:41";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrchr
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 ,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <string.h>
#include <ctype.h>

/*
 * NAME: wstrchr()
 *
 * FUNCTION: look for the occurrence of an wchar_t character within a string of
 * wchar_t characters.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first occurrence
 * of the wchar_t or a NULL on failure.
 *
 */
wchar_t * wstrchr(wchar_t *sp, int c)
{
	return ( wcschr(sp, (wchar_t) c) );
}
