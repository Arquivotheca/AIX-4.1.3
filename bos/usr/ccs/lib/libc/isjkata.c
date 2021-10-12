static char sccsid[] = "@(#)19	1.2  src/bos/usr/ccs/lib/libc/isjkata.c, libcnls, bos411, 9428A410j 6/8/91 16:02:53";
/*
 * COMPONENT NAME: (LIBCGEN/KJI) Standard C Library Classification Function
 *
 * FUNCTIONS: isjkata
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

#include <ctype.h>

#ifdef isjkata
#undef isjkata
#endif

int isjkata(const wchar_t c)
{
	return ( is_wctype(c, get_wctype("jkata")) );
}
