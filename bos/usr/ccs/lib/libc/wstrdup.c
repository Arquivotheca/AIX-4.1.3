static char sccsid[] = "@(#)47	1.3  src/bos/usr/ccs/lib/libc/wstrdup.c, libcnls, bos411, 9428A410j 6/9/91 17:14:42";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrdup
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 ,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <ctype.h>

/*
 *  returns a pointer to a wchar_t string which is a duplicate of the wchar 
 *  string pointed to by s1.  Space for the new string is allocated by using 
 *  MALLOC (BA_OS).  When a new string cannot be created a NULL pointer is 
 *  returned.
 */
wchar_t *wstrdup(wchar_t *s1)
{
	return ( (wchar_t *)NCstrdup (s1) );
}
