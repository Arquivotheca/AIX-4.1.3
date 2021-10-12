static char sccsid[] = "@(#)21	1.2  src/bos/usr/ccs/lib/libc/isjparen.c, libcnls, bos411, 9428A410j 6/8/91 15:59:21";
/*
 * COMPONENT NAME: (LIBCGEN/KJI) Standard C Library Classification Function
 *
 * FUNCTIONS: isjparen, isjparent
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

#ifdef isjparen
#undef isjparen
#endif

int isjparen(const unsigned c)
{
	return( is_wctype(c, get_wctype("jparen")) );
}

#ifdef isjparent
#undef isjparent
#endif

int isjparent(const unsigned c)
{
	return( is_wctype(c, get_wctype("jparen")) );
}
