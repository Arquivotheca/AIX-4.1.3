static char sccsid[] = "@(#)13	1.4.1.1  src/bos/usr/ccs/lib/libc/__strxfrm_C.c, libcstr, bos411, 9428A410j 5/25/92 14:07:24";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __strxfrm_C
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
#include <string.h>

/*
 * FUNCTION: __strxfrm_C - Method implementing strxfrm() for C locale.
 *
 * PARAMETERS:
 *           _LC_collate_objhdl_t hdl - unused.
 *
 *	     char *s1 - output string of collation weights.
 *	     char *s2 - input string of characters.
 *           size_t n - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to s1.
 */

int __strxfrm_C(_LC_collate_objhdl_t hdl, char *s1, const char *s2, size_t n)
{
    size_t i;
    int len;

    len = strlen(s2);

    /**********
      if s1 is null or n is 0, just return the length
    **********/
    if (s1==NULL || n==0)
	return len;

    /**********
      copy n-1 bytes from s2 to s1 and null terminate s1
    **********/
    n--;
    strncpy(s1, s2, n);
    s1[n] = '\0';

    return len;
}
