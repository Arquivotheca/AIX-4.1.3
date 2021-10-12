#if 0
static char sccsid[] = "@(#)76	1.1  src/bos/usr/ccs/lib/libc/strcoll_backwd_routines.c, libcstr, bos411, 9428A410j 5/4/94 16:47:19";
#endif
/*
 *   COMPONENT_NAME: LIBCSTR
 *
 *   FUNCTIONS: BACKWARD_COLLATE
 *		BACK_POS_COLLATE
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
 *  BACKWARD_COLLATE and BACK_POS_COLLATE are defined by the source files that
 *  #include it.  These are __strcoll_sb.c and __strcoll_std.c.  The _sb
 *  file will define __SINGLE_BYTE__ and the _std file will undefine it.
 *  Each of these files includes this file twice.  The first time,
 *  __FAST__ is turned off and the non-fast names are used (i.e.
 *  backward_collate_* & back_pos_collate_*.  The second time, it is turned on, 
 *  and the fast names are used (fast_backward_collate_* & fast_back_pos_collate_*).
 *  The _* at the end are for ths _sb and _std suffixes and are based on
 *  whether the _sb file included it or the _std file did.
 *
 *  So you get the following routines:
 *      -----------------------------------------------------------------
 *		  | __SINGLE_BYTE__ defined   | __SINGLE_BYTE__ undefined
 *	__FAST__  |     __strcoll_sb.c	      |	     __strcoll_std.c
 *      -----------------------------------------------------------------
 *	undefined | backward_collate_sb	      | backward_collate_std
 *	undefined | back_pos_collate_sb	      | back_pos_collate_std
 *	defined   | fast_backward_collate_sb  | fast_backward_collate_std
 *	defined   | fast_back_pos_collate_sb  | fast_back_pos_collate_std
 */

#ifndef __ALREADY_INCLUDED__
#define __ALREADY_INCLUDED__
#pragma alloca
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "patlocal.h"
#endif /* __ALREADY_INCLUDED */

int 
BACKWARD_COLLATE(_LC_collate_objhdl_t hdl, char *str1, 
			char *str2, int order)
{
    int *str1_colvals;  /* space for collation values for str1 */
    int *str2_colvals;  /* space for collation values for str1 */

    int rc;
    int len;
    int str1_colvals_len;
    int str2_colvals_len;
    wchar_t wc;
#ifndef __FAST__
    int done;
    char *wgt_str = NULL;
#endif /* __FAST__ */ 

    /**********
      get the space for the collation values.  currently there cannot
      be more collation values than bytes in the string
    *********/
    len = strlen(str1)*sizeof(int)+1;
    if ((str1_colvals=(int *)alloca(len)) == (int *)NULL) {
	perror("alloca");
	exit(-1);
    }
    len = strlen(str2)*sizeof(int)+1;
    if ((str2_colvals=(int *)alloca(len)) == (int *)NULL) {
	perror("alloca");
	exit(-1);
    }

    /**********
      put all of the colvals in str1_colvals and keep count
    **********/
    str1_colvals_len = 0;
    str2_colvals_len = 0;
    while (*str1 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
#ifdef __SINGLE_BYTE__
	wc = (wchar_t) *str1++ & 0xff;
#else
	if ((rc = mbtowc(&wc, str1, MB_CUR_MAX)) == -1) {
	    errno = EINVAL;
	    wc = (wchar_t) *str1++ & 0xff;
	}
	else
	    str1 += rc;
#endif /* __SINGLE_BYTE__ */

#ifdef __FAST__
	str1_colvals[str1_colvals_len] = __wccollwgt(wc)[order];
#else
	do {
	    str1 += _getcolval(hdl, &str1_colvals[str1_colvals_len], wc, str1, order, &wgt_str, &done);
#endif	/* __FAST__ */

	    if (str1_colvals[str1_colvals_len] != 0)
	        str1_colvals_len++;
#ifndef __FAST__
	    } while (!done);
#endif /* __FAST__ */
    }
    str1_colvals_len--;
    
    /**********
      do the same for str2
    **********/
    while (*str2 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
#ifdef __SINGLE_BYTE__
	wc = (wchar_t) *str2++ & 0xff;
#else
	if ((rc = mbtowc(&wc, str2, MB_CUR_MAX)) == -1) {
	    errno = EINVAL;
	    wc = (wchar_t) *str2++ & 0xff;
	}
	else
	    str2 += rc;
#endif /* __SINGLE_BYTE__ */

#ifdef __FAST__
	str2_colvals[str2_colvals_len] = __wccollwgt(wc)[order];
#else
	do {
	    str2 += _getcolval(hdl, &str2_colvals[str2_colvals_len], wc, str2, order, &wgt_str, &done);
#endif /* __FAST__ */

	    if (str2_colvals[str2_colvals_len] != 0)
	        str2_colvals_len++;
#ifndef __FAST__
	    } while (!done);
#endif /* __FAST__ */
    }
    str2_colvals_len--;

    /**********
      start at the end of both string and compare the values
    **********/
    while ((str1_colvals_len>=0) && (str2_colvals_len>=0)) {
	if (str1_colvals[str1_colvals_len] < str2_colvals[str2_colvals_len]) {
	    return(-1);
	}
	else if (str1_colvals[str1_colvals_len] > str2_colvals[str2_colvals_len]) {
	    return(1);
	}
	str1_colvals_len--;
	str2_colvals_len--;
    }
    /********
      if we are here, they are equal, if str1 is longer than str2, it is
      greater
    **********/
    return(str1_colvals_len - str2_colvals_len);
   
}


int 
BACK_POS_COLLATE(_LC_collate_objhdl_t hdl, char *str1, 
			char *str2, int order)
{
    int *str1_colvals;  /* space for collation values for str1 */
    int *str2_colvals;  /* space for collation values for str1 */

    int rc;
    int len;
    int str1_colvals_len;
    int str2_colvals_len;
    int str1_pos = 0;
    int str2_pos = 0;
    wchar_t wc;
#ifndef __FAST__
    int done;
    char *wgt_str=NULL;
#endif /* __FAST__ */

    /**********
      get the space for the collation values.  currently there cannot
      be more collation values than bytes in the string
    *********/
    len = strlen(str1)*2*sizeof(int)+1;
    if ((str1_colvals=(int *)alloca(len)) == (int *)NULL) {
	perror("alloca");
	exit(-1);
    }
    len = strlen(str2)*2*sizeof(int)+1;
    if ((str2_colvals=(int *)alloca(len)) == (int *)NULL) {
	perror("alloca");
	exit(-1);
    }

    /**********
      put all of the colvals in str1_colvals followed by
      their position and keep count
    **********/
    str1_colvals_len = 0;
    str2_colvals_len = 0;
    while (*str1 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
#ifdef __SINGLE_BYTE__
	wc = (wchar_t) *str1++ & 0xff;
#else
	if ((rc = mbtowc(&wc, str1, MB_CUR_MAX)) == -1) {
	    errno = EINVAL;
	    wc = (wchar_t) *str1++ & 0xff;
	}
	else
	    str1 += rc;
#endif /* __SINGLE_BYTE__ */

#ifdef __FAST__
	str1_colvals[str1_colvals_len] = __wccollwgt(wc)[order];
#else
	do {
	    str1 += _getcolval(hdl, &str1_colvals[str1_colvals_len], wc, str1, order, &wgt_str, &done);
#endif /* __FAST__ */
	    str1_pos++;
	    if (str1_colvals[str1_colvals_len] != 0) {
	        str1_colvals_len++;
	        str1_colvals[str1_colvals_len++] = str1_pos;
	        }
#ifndef __FAST__
	   } while (!done); 
#endif /* __FAST__ */
    }
    str1_colvals_len--;
    
    /**********
      do the same for str2
    **********/
    while (*str2 != '\0') {
	/**********
	  get the collating value for each character.  if it is an invalid
	  character, assume 1 byte and go on
	**********/
#ifdef __SINGLE_BYTE__
	wc = (wchar_t) *str2++ & 0xff;
#else
	if ((rc = mbtowc(&wc, str2, MB_CUR_MAX)) == -1) {
	    errno = EINVAL;
	    wc = (wchar_t) *str2++ & 0xff;
	}
	else
	    str2 += rc;
#endif /* __SINGLE_BYTE__ */

#ifdef __FAST__
	str2_colvals[str2_colvals_len] = __wccollwgt(wc)[order];
#else
	do {
	    str2 += _getcolval(hdl, &str2_colvals[str2_colvals_len], wc, str2, order, &wgt_str, &done);
#endif /* __FAST__ */
	    str2_pos++;
	    if (str2_colvals[str2_colvals_len] != 0) {
	        str2_colvals_len++;
	        str2_colvals[str2_colvals_len++] = str2_pos;
	    }
#ifndef __FAST__
	} while (!done);
#endif /* __FAST__ */
    }
    str2_colvals_len--;

    /**********
      start at the end of both string and compare the values
    **********/
    while ((str1_colvals_len>=0) && (str2_colvals_len>=0)) {
	if (str1_colvals[str1_colvals_len] < str2_colvals[str2_colvals_len]) {
	    return(-1);
	}
	else if (str1_colvals[str1_colvals_len] > str2_colvals[str2_colvals_len]) {
	    return(1);
	}
	str1_colvals_len--;
	str2_colvals_len--;
    }
    /********
      if we are here, they are equal, if str1 is longer than str2, it is
      greater
    **********/
    return(str1_colvals_len - str2_colvals_len);
   
}
