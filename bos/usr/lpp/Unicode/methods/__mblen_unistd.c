static char sccsid[] = "@(#)46	1.2  src/bos/usr/lpp/Unicode/methods/__mblen_unistd.c, cfgnls, bos411, 9428A410j 2/8/94 09:04:38";
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: __mblen_unistd
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


size_t __mblen_unistd(_LC_charmap_objhdl_t handle, char *s, size_t n)
{
    long l;	/* variable for accumulating UCS value */
    int c0, 	/* value of first */
    c, 		/* value of 2nd or greater byte */
    nc;		/* nubmer of bytes */
    Tab *t;	/* UTF-8 definition table entry */

    /*
    **	if s is NULL or points to a NULL, return 0
    */
    if (!s || !(*s)) {
	return 0;
    }

    nc = 0;
    c0 = *s & 0xFF;
    l = c0;
    for (t = tab; t->cmask; t++) {
	nc++;
	if (n < nc) {
	    errno = EILSEQ;
	    return -1;
	}
	if ((c0 & t->cmask) == t->cval) {
	    /* Check that accumulated UCS value is in correct range. */
	    l &= t->lmask;
	    if (l < t->lval) {
		errno = EILSEQ;
		return -1;
	    }
	    return nc;
	}
	s++;
	c = (*s ^ 0x80) & 0xFF;
	if (c & 0xC0) {
	    errno = EILSEQ;
	    return -1;
	}
	l = (l<<6) | c;
    }
    /* If it reaches here, there is an invalid multibyte 
     * character. */
    errno = EILSEQ;
    return -1;
}
