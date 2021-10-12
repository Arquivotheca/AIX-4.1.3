static char sccsid[] = "@(#)11	1.2  src/bos/usr/ccs/lib/libc/isj1kana.c, libcnls, bos411, 9428A410j 6/8/91 16:02:40";
/*
 * COMPONENT_NAME: (LIBCGEN/KJI) Standard C Library National Language Support
 *
 * FUNCTIONS: isj1bytekana
 *
 * ORIGINS: 10
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

#ifdef isj1bytekana
#undef isj1bytekana
#endif

int
isj1bytekana(int nlc)
{
	return( is_wctype(nlc, get_wctype("j1kana")) );
}
