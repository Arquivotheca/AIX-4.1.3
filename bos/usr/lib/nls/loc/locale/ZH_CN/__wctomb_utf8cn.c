static char sccsid[] = "@(#)39	1.1  src/bos/usr/lib/nls/loc/locale/ZH_CN/__wctomb_utf8cn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:24:39";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __wctomb_utf8cn
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

#include "mb-wc.h"

/*
  Converts a UNICODE process code to a string of characters for the UTF2 codeset
*/

int
__wctomb_utf8cn(_LC_charmap_objhdl_t handle, char *s, wchar_t wc)
{
	long l;
	int c, nc;
	Tab *t;

	if(s == (char *)NULL) return 0;

	l = wc;
	nc = 0;
	for(t=tab; t->cmask; t++)  {
	    nc++;
	    if(l <= t->lmask) {
		c = t->shift;
		*s = t->cval | (l>>c);
		while(c > 0)  {
		     c -= 6;
		     s++;
		     *s = 0x80 | ((l>>c) & 0x3F);
		}
		return nc;
	     }
	}
	errno = EILSEQ;
	return -1;
}

