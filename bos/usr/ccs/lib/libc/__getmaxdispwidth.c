static char sccsid[] = "@(#)05	1.1  src/bos/usr/ccs/lib/libc/__getmaxdispwidth.c, libccppc, bos411, 9428A410j 1/12/93 12:41:13";
/* 
 * COMPONENT_NAME: LIBCCPPC
 * 
 * FUNCTIONS: __getmaxdispwidth
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Routine which return the maximum number of display width
 * in a multibyte character according to the current
 * locale. __max_disp_width is defined as a call to this 
 * in stdlib.h.
 * 
 * Written to avoid referencing __lc_charmap in stdlib.h
 */

#include <sys/localedef.h>


int __getmaxdispwidth(void)
{
	return (__OBJ_DATA(__lc_charmap)->cm_max_disp_width);
}
