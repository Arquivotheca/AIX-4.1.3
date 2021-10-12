static char sccsid[] = "@(#)50	1.1  src/bos/usr/lpp/Unicode/methods/__mbtowc_unistd.c, cfgnls, bos411, 9428A410j 1/21/94 10:14:57";
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: __mbtowc_unistd
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
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>
#include "uni.h"

/*
	Convert the multibyte character value to its widechar (UNICODE) code.
*/

int __mbtowc_unistd(_LC_charmap_objhdl_t handle, wchar_t *p, 
	const char *s, size_t n)
{
    long l;
    int c0, c, nc;
    Tab *t;

    if (!s) {
	return 0;
    }

    nc = 0;
    c0 = *s & 0xff;
    l = c0;
    for (t = tab; t->cmask; t++) {
	nc++;
	if (n < nc) {
	    errno = EILSEQ;
	    return -1;
	}
	if ((c0 & t->cmask) == t->cval) {
	    l &= t->lmask;
	    if (l < t->lval) {
		errno = EILSEQ;
		return -1;
	    }
	    if (p) {
		*p = l;
	    }
	    if (l) {
		return nc;
	    }
	    else {
		return 0;
	    }
	}
	s++;
	c = (*s ^ 0x80) & 0xFF;
	if (c & 0xC0) {
	    errno = EILSEQ;
	    return -1;
	}
	l = (l<<6) | c;
    }
    /* 
    ** If it reaches here, there is an invlaid multibyte 
    ** character 
    */
    errno = EILSEQ;
    return -1;
}
 
