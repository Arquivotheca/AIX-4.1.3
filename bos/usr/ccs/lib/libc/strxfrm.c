static char sccsid[] = "@(#)82	1.18.1.2  src/bos/usr/ccs/lib/libc/strxfrm.c, libcstr, bos411, 9428A410j 1/12/93 11:19:37";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strxfrm
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <string.h>

/*
 * FUNCTION: strxfrm - converter character string to string
 *                     of collation weights.
 *
 * PARAMETERS:
 *	     char *s1 - output string of collation weights.
 *	     char *s2 - input string of characters.
 *           size_t n - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to s1.
 */

size_t strxfrm(char *s1, const char *s2, size_t n)
{
	return _CALLMETH(__lc_collate,__strxfrm)(__lc_collate,s1, s2, n);
}
