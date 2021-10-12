static char sccsid[] = "@(#)49	1.2  src/bos/usr/lpp/Unicode/methods/__mbtopc_unistd.c, cfgnls, bos411, 9428A410j 2/8/94 09:04:16";
/*
 *   COMPONENT_NAME: CFGNLS
 *
 *   FUNCTIONS: __mbtopc_unistd
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
#include "uni.h"

/*
**	Convert a multibyte UTF-8 encoding to UNICODE.
**	It sets the err to specify how many bytes are needed
**	on error and returns zero..
**	On success, t returns the number of multibytes consumed.
*/


size_t __mbtopc_unistd (_LC_charmap_objhdl_t handle, wchar_t *p, 
	char *s, size_t n, int *err )
{

    long l;
    int c0, c, nc;
    Tab *t;

    /*
    **	Assume it is a valid character
    */
    *err = 0;	/* No errors */

    /*
    **	if s is NULL return 0
    */
    if (!s) {
	return 0;
    }

    /*
    **  FSS-UTF code (multibyte) to UNICODE (process code) conversions.
    */

    nc = 0;
    c0 = *s & 0xff;
    l = c0;
    for (t = tab; t->cmask; t++) {
	nc++;
	if (n < nc) {
	    *err = nc;
	    return 0;
	}
	if ((c0 & t->cmask) == t->cval) {
	    l &= t->lmask;
	    if (l < t->lval) {
		*err = -1;
		return 0;
	    }
	    if (p) {
		*p = l;
	    }
	    /* NOTE: Should this return 0 for a NULL byte? */
	    return nc;
	} 
	s++;
	c = (*s ^ 0x80) & 0xFF;
	if (c & 0xC0) {
	    *err = -1;
	    return 0;
	}
	l = (l<<6) | c;
    }
    /* 
    ** If it reaches here, there is an invlaid multibyte 
    ** character 
    */
    *err = -1;
    return 0;
}
    
		



