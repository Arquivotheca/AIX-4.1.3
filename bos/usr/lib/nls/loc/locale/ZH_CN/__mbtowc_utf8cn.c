static char sccsid[] = "@(#)34	1.1  src/bos/usr/lib/nls/loc/locale/ZH_CN/__mbtowc_utf8cn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:24:31";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __mbtowc_utf8cn
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
 * Convert a UTF multi-byte string to UNICODE process code for UTF2 codeset
 *
 */

int
__mbtowc_utf8cn(_LC_charmap_objhdl_t handle, wchar_t *p, char *s, size_t n)
{
        wchar_t dummy;
	long current_all;
	int firstbyte, c, nc;
	Tab *t;

	/*********
	 If length is equal to zero returns -1
        *********/
	nc = 0;
	if(n <= nc) {
	   return(-1);
	}

	/*******
	 If s is NULL returns zero
	*******/
	if(s == (char *)NULL ) return 0;

	/*******
	 If p is NULL, set it to dummy
	*******/
	if(p == (wchar_t *)NULL) p = &dummy;

	/*******
	  If s is null string, return zero
	*******/
	if(*s == '\0')  {
	     *p = (wchar_t)s[0];
	     return 0;   		 /* ???? */
	}

	firstbyte = *s & 0xff;
	current_all = firstbyte;

	for(t=tab; t->cmask; t++) {
	    nc++;
	    if((firstbyte & t->cmask) == t->cval) {
		current_all &= t->lmask;
		if(current_all < t->lval)  {
		   errno = EILSEQ;
		   return(-1);
		}
	  	*p = current_all;
		return(nc);
	    }
	    if(n <= nc) {
		errno = EILSEQ;
		return(-1);
	    }

	    /********
	    Process next byte
	    ********/
	    s++;
	    c = (*s ^ 0x80) & 0xFF;
	
	    /********
	    If the latter byte without 80 head
	    ********/
	    if(c & 0xC0)  {
		errno = EILSEQ;
		return(-1);
	    } 

	    current_all = ( current_all<<6 ) | c;
	 }
	 /*********
	  If the bytes are invalid
	 *********/
	 errno = EILSEQ;
	 return(-1);
}


