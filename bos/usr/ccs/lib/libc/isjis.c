static char sccsid[] = "@(#)17	1.2  src/bos/usr/ccs/lib/libc/isjis.c, libcnls, bos411, 9428A410j 6/8/91 15:59:16";
/*
 * COMPONENT NAME: (LIBCNLS) Standard C Library Classification Function
 *
 * FUNCTIONS: isjis
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

#include <NLchar.h>

#ifdef isjis
#undef isjis
#endif

int isjis(const wchar_t c)
{
	return( (c>0x7f) );
}
