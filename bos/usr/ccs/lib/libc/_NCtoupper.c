static char sccsid[] = "@(#)85	1.4  src/bos/usr/ccs/lib/libc/_NCtoupper.c, libcnls, bos411, 9428A410j 2/26/91 12:44:23";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: _NCtoupper
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

#ifdef _NCtoupper 
#undef _NCtoupper
#endif 

int _NCtoupper(int c) 
{
	return ( towupper(c) );
}