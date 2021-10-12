static char sccsid[] = "@(#)55	1.2  src/bos/usr/lpp/Unicode/methods/__wctomb_unistd.c, cfgnls, bos411, 9428A410j 2/8/94 09:03:51";
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: __wctomb_unistd
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *
 * FUNCTION: 
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>
#include "uni.h"
/*
**	Convert Unicode to UTF-8 (Multibyte form).
*/

int __wctomb_unistd(_LC_charmap_objhdl_t handle, char *s, wchar_t wc)
{
    long l;
    int c, nc;
    Tab *t;

    if (!s) {
	return 0;
    }

    l = wc;
    nc = 0;
    for (t = tab; t->cmask; t++) {
	nc++;
	if (l <= t->lmask) {
	    c = t->shift;
	    *s = t->cval | (l>>c);
	    while (c > 0) {
		c -= 6;
		s++;
		*s = 0x80 | ((l>>c) & 0x3F);
	    }
	    return nc;
	}
    }
    return -1;
}
