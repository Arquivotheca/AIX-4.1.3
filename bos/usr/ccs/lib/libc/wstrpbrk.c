static char sccsid[] = "@(#)51	1.2  src/bos/usr/ccs/lib/libc/wstrpbrk.c, libcnls, bos411, 9428A410j 6/8/91 17:18:04";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrpbrk
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

#include <string.h>
#include <ctype.h>

/*
 *  Return ptr to first occurrence of any character from `brkset'
 *  in the character string `string'; NULL if none exists.  
 */

wchar_t * wstrpbrk(wchar_t *string, wchar_t *brkset)
{
	return ( wcspbrk(string, brkset) );
}
