static char sccsid[] = "@(#)18	1.2  src/bos/usr/ccs/lib/libc/isjkanji.c, libcnls, bos411, 9428A410j 6/8/91 15:59:18";
/*
 * COMPONENT NAME: (LIBCGEN/KJI) Standard C Library Classification Function
 *
 * FUNCTIONS: isjkanji
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

#include <ctype.h>

#ifdef isjkanji
#undef isjkanji
#endif

int isjkanji(const wchar_t c)
{
	return( is_wctype(c, get_wctype("jkanji")) );
}
