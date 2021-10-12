static char sccsid[] = "@(#)33	1.1  src/bos/usr/lib/nls/loc/locale/ZH_CN/__mbtopc_utf8cn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:24:29";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __mbtopc_utf8cn
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

#include "mb-wc.h"

/* 
 * Convert a multi-byte string to process code for IBM_eucCN codeset
 *
 */

size_t  
__mbtopc_utf8cn(_LC_charmap_objhdl_t handle, wchar_t *p, const char *s, size_t n, int *err)
 {
	wchar_t dummy;
	long current_all;
	int firstbyte, c, nc, needbyte;
	Tab *t;

	/**********
	  If s is NULL return 0
	 **********/
	if (s == (char *)NULL) return(0);

	/*********
	  If p is NULL, set it to dummy
	*********/
        if (p == (wchar_t *)NULL) p = &dummy;

	/*********
	  Assume it is a bad character
	*********/
	*err = 0;

	firstbyte = *s & 0xff;
	current_all = firstbyte;
	needbyte = 0;

	for(t=tab; t->cmask; t++) {
	    if((firstbyte & t->cmask) == t->cval) {
		needbyte = t->nbytes;
		break;
	    }
	}

	/********
	 The fistbyte is invalid
	********/
	if(needbyte == 0) {
	    *err = -1;
	    return(0);
	}

	nc = 0;
	if(n <= nc) {
	  *err = needbyte;		/*  New changes ???? */
     	  return 0;
	}

	for(t=tab; t->cmask; t++) {
	    nc++;
	    if((firstbyte & t->cmask) == t->cval) {
		current_all &= t->lmask;
		
		/********
		 If multi-byte is out of range 
 		********/
		if(current_all < t->lval)  {
		    *err = -1;
		    return(0);
		}
		
		/*******
		 If multi-byte string is valid
		*******/
	  	*p = current_all;
		return(nc);
	    }

	    /*********
	     If a character connot be formed in n bytes or less 
	    *********/
	    if(n <= nc) {
		*err = needbyte;	/* ???? */
        	return(0);
	    }

	    /*********
	     Process next byte
	    *********/
	    s++;
	    c = (*s ^ 0x80) & 0xFF;

	    /*********
	     If this byte isnot with 80 head
	    *********/
	    if(c & 0xC0)  {
	      *err = -1;
	      return(0);
	    }
	    current_all = (current_all<<6) | c;
	 }
	 
	 /*********
	  The multi-byte string is invalid
	 *********/
	 *err = -1;
	 return(0);
}

