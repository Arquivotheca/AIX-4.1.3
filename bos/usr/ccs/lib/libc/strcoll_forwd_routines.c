#if 0
static char sccsid[] = "@(#)77	1.1  src/bos/usr/ccs/lib/libc/strcoll_forwd_routines.c, libcstr, bos411, 9428A410j 5/4/94 16:47:21";
#endif
/*
 *   COMPONENT_NAME: LIBCSTR
 *
 *   FUNCTIONS: FORWARD_COLLATE
 *		FAST_FORWARD_COLLATE
 *		FORW_POS_COLLATE
 *		FAST_FORW_POS_COLLATE
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
 *  FORWARD_COLLATE and FORW_POS_COLLATE are defined by the source files that
 *  #include it.  These are __strcoll_sb.c and __strcoll_std.c.  The _sb
 *  file will define __SINGLE_BYTE__ and the _std file will undefine it.
 *  Each of these files includes this file twice.  The first time,
 *  __FAST__ is turned off and the non-fast names are used (i.e.
 *  forward_collate_* & forw_pos_collate_*.  The second time, it is turned on, 
 *  and the fast names are used (fast_forward_collate_* & fast_forw_pos_collate_*).
 *  The _* at the end are for ths _sb and _std suffixes and are based on
 *  whether the _sb file included it or the _std file did.
 *
 *  So you get the following routines:
 *      -------------------------------------------------------
 *	 __SINGLE_BYTE__ defined  | __SINGLE_BYTE__ undefined
 *	    __strcoll_sb.c	  |    __strcoll_std.c
 *      -------------------------------------------------------
 *	 forward_collate_sb	  | forward_collate_std
 *	 forw_pos_collate_sb	  | forw_pos_collate_std
 *	 fast_forward_collate_sb  | fast_forward_collate_std
 *	 fast_forw_pos_collate_sb | fast_forw_pos_collate_std
 */

#ifndef __ALREADY_INCLUDED__
#define __ALREADY_INCLUDED__
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "patlocal.h"
#endif /* __ALREADY_INCLUDED__ */

/**********************************************************
  FUNCTION: forward collate

  PURPOSE: collates in string order in the forward direction
**********************************************************/
int 
FORWARD_COLLATE(_LC_collate_objhdl_t hdl, char *str1, char *str2, 
			int order)
{
    wchar_t wc;
    int     rc;
    int str1_colval;
    int str2_colval;
    int done1 = 1;
    int done2 = 1;
    char *wgt_str1 = NULL;
    char *wgt_str2 = NULL;

    /**********
      go thru all of the characters until a null is hit
    **********/
    while (!((*str1 == '\0') && done1) && !((*str2 == '\0') && done2)) {
	/**********
	  convert to a wc, if it is an invalid wc, assume a length of 1
	  and continue. If the collating value is 0 (non-collating) get the
	  collating value of the next character
	***********/
	do {
	    if (done1) { /* first time for this character */
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str1++ & 0x00ff;
#else
	        if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str1++ & 0x00ff;
	        }
	        else
		    str1 += rc;
#endif /* __SINGLE_BYTE__ */
	    }
	    str1 += _getcolval(hdl, &str1_colval, wc, str1, order, &wgt_str1, &done1);
	} while ((str1_colval == 0) && !((*str1 == '\0') && done1));
	
	do {
	    if (done2) { /* first time for this character */
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str2++ & 0x00ff;
#else
		if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str2++ & 0x00ff;
	        }
	        else
		    str2 += rc;
#endif /* __SINGLE_BYTE__ */
            }
	    str2 += _getcolval(hdl, &str2_colval, wc, str2, order, &wgt_str2, &done2);
	} while ((str2_colval == 0) && !((*str2 == '\0') && done2));
	
	/**********
	  if the collation values are not equal, then we have gone far
	  enough and may return
	**********/
	if (str1_colval < str2_colval)
	    return(-1);
	else if (str1_colval > str2_colval)
	    return(1);
    }
    
    /*********
      to get here, str1 and/or str2 are NULL and they have been equal so far.
      If one of them is non-null, check if the remaining characters are
      non-collating. If they are, then the strings are equal for this order.
    *********/
    /**********
      If str1 is non-null and there are collating characters left, then
      str1 is greater than str2
    **********/
    if ((*str1 != '\0') || !done1) {
	do {
	    if (done1) {
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str1++ & 0x00ff;
#else
                if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str1++ & 0x00ff;
	        }
	        else
		    str1 += rc;
#endif /* __SINGLE_BYTE__ */
            }
	    str1 += _getcolval(hdl, &str1_colval, wc, str1, order, &wgt_str1, &done1);
	} while ((str1_colval == 0) && !((*str1 == '\0') && done1));
	if (str1_colval != 0)
	    return(1);
    }
    
    
    /**********
      if str2 is non-null and there are collating characters left, then
      str1 is less than str2
    **********/
    else if ((*str2 != '\0') || !done2) {
	do {
	    if (done2) {
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str2++ & 0x00ff;
#else
                if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str2++ & 0x00ff;
	        }
	        else
		    str2 += rc;
#endif /* __SINGLE_BYTE__ */
            }
	    str2 += _getcolval(hdl, &str2_colval, wc, str2, order, &wgt_str2, &done2);
	} while ((str2_colval == 0) && !((*str2 == '\0') && done2));
	if (str2_colval != 0)
	    return(-1);
    }

    /**********
      if we get to here, they are equal
    **********/
    return(0);
}

int 
FAST_FORWARD_COLLATE(_LC_collate_objhdl_t hdl, char *str1, char *str2, 
			int order)
{
    wchar_t wc;
    int     rc;
    int str1_colval;
    int str2_colval;

    /**********
      go thru all of the characters until a null is hit
    **********/
    while ((*str1 != '\0') && (*str2 != '\0')) {
	/**********
	  convert to a wc, if it is an invalid wc, assume a length of 1
	  and continue. If the collating value is 0 (non-collating) get the
	  collating value of the next character
	***********/
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str1++ & 0x00ff;
#else
	    if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str1++ & 0x00ff;
	    }
	    else
		str1 += rc;
#endif /* __SINGLE_BYTE__ */
	    str1_colval = __wccollwgt(wc)[order];
	} while ((str1_colval == 0) && (*str1 != '\0'));
	
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str2++ & 0x00ff;
#else
	    if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str2++ & 0x00ff;
	    }
	    else
	        str2 += rc;
#endif /* __SINGLE_BYTE__ */

	    str2_colval = __wccollwgt(wc)[order];
	} while ((str2_colval == 0) && (*str2 != '\0'));
	
	/**********
	  if the collation values are not equal, then we have gone far
	  enough and may return
	**********/
	if (str1_colval < str2_colval)
	    return(-1);
	else if (str1_colval > str2_colval)
	    return(1);
    }
    
    /*********
      to get here, str1 and/or str2 are NULL and they have been equal so far.
      If one of them is non-null, check if the remaining characters are
      non-collating. If they are, then the strings are equal for this order.
    *********/
    /**********
      If str1 is non-null and there are collating characters left, then
      str1 is greater than str2
    **********/
    if (*str1 != '\0') {
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str1++ & 0x00ff;
#else
            if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str1++ & 0x00ff;
	    }
	    else
	        str1 += rc;
#endif /* __SINGLE_BYTE__ */
	    str1_colval = __wccollwgt(wc)[order];
	} while ((str1_colval == 0) && (*str1 != '\0'));
	if (str1_colval != 0)
	    return(1);
    }
    
    
    /**********
      if str2 is non-null and there are collating characters left, then
      str1 is less than str2
    **********/
    else if ((*str2 != '\0')) {
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str2++ & 0x00ff;
#else
            if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str2++ & 0x00ff;
	    }
	    else
		str2 += rc;
#endif /* __SINGLE_BYTE__ */
	    str2_colval = __wccollwgt(wc)[order];
	} while ((str2_colval == 0) && (*str2 != '\0'));
	if (str2_colval != 0)
	    return(-1);
    }

    /**********
      if we get to here, they are equal
    **********/
    return(0);
}

/**********************************************************
  FUNCTION: forw_pos collate

  PURPOSE: collates in string order in the forw_pos direction
**********************************************************/
int 
FORW_POS_COLLATE(_LC_collate_objhdl_t hdl, char *str1, char *str2, 
			 int order)
{
    wchar_t wc;
    int     rc;
    int str1_colval;
    int str2_colval;
    int str1_pos = 0;
    int str2_pos = 0;
    int done1 = 1;
    int done2 = 1;
    char *wgt_str1 = NULL;
    char *wgt_str2 = NULL;

    /**********
      go thru all of the characters until a null is hit
    **********/
    while (!((*str1 == '\0') && done1) && !((*str2 == '\0') && done2)) {
	/**********
	  convert to a wc, if it is an invalid wc, assume a length of 1
	  and continue. If the collating value is 0 (non-collating) get the
	  collating value of the next character
	***********/
	do {
	    if (done1) {  /* first time for this character */
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str1++ & 0x00ff;
#else
	        if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str1++ & 0x00ff;
	        }
	        else
		    str1 += rc;
#endif /* __SINGLE_BYTE */
	    }
	    str1_pos++;
	    str1 += _getcolval(hdl, &str1_colval, wc, str1, order, &wgt_str1, &done1);
	} while ((str1_colval == 0) && !((*str1 == '\0') && done1));
	
	do {
	    if (done2) {  /* first time for this character */
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str2++ & 0x00ff;
#else
	        if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str2++ & 0x00ff;
	        }
	        else
		    str2 += rc;
#endif /* __SINGLE_BYTE */
            }
	    str2_pos++;
	    str2 += _getcolval(hdl, &str2_colval, wc, str2, order, &wgt_str2, &done2);
	} while ((str2_colval == 0) && !((*str2 == '\0') && done2));
	
	/**********
	  If the collating values are not zero, check the position
	  in the string. 
	**********/
	if (str1_colval !=0 && str2_colval != 0) {
	    if (str1_pos != str2_pos)
		return (str1_pos - str2_pos);
	}

	/**********
	  if the collation values are not equal, then we have gone far
	  enough and may return
	**********/
	if (str1_colval < str2_colval)
	    return(-1);
	else if (str1_colval > str2_colval)
	    return(1);
    }
    
    /*********
      to get here, str1 and/or str2 are NULL and they have been equal so far.
      If one of them is non-null, check if the remaining characters are
      non-collating. If they are, then the strings are equal for this order.
    *********/
    /**********
      If str1 is non-null and there are collating characters left, then
      str1 is greater than str2
    **********/
    if ((*str1 != '\0') || !done1) {
	do {
            if (done1) { 
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str1++ & 0x00ff;
#else
	        if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str1++ & 0x00ff;
	        }
	        else
		    str1 += rc;
#endif /* __SINGLE_BYTE */
            }
	    str1 += _getcolval(hdl, &str1_colval, wc, str1, order, &wgt_str1, &done1);
	} while ((str1_colval == 0) && !((*str1 == '\0') && done1));
	if (str1_colval != 0)
	    return(1);
    }
    
    
    /**********
      if str2 is non-null and there are collating characters left, then
      str1 is less than str2
    **********/
    else if ((*str2 != '\0') || !done2) {
	do {
            if (done2) {
#ifdef __SINGLE_BYTE__
		wc = (wchar_t) *str2++ & 0x00ff;
#else
	        if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		    errno = EINVAL;
		    wc = (wchar_t) *str2++ & 0x00ff;
	        }
	        else
		    str2 += rc;
#endif /* __SINGLE_BYTE */
            }
	    str2 += _getcolval(hdl, &str2_colval, wc, str2, order, &wgt_str2, &done2);
	} while ((str2_colval == 0) && !((*str2 == '\0') && done2));
	if (str2_colval != 0)
	    return(-1);
    }

    /**********
      if we get to here, they are equal
    **********/
    return(0);
}

int 
FAST_FORW_POS_COLLATE(_LC_collate_objhdl_t hdl, char *str1, char *str2, 
			 int order)
{
    wchar_t wc;
    int     rc;
    int str1_colval;
    int str2_colval;
    int str1_pos = 0;
    int str2_pos = 0;

    /**********
      go thru all of the characters until a null is hit
    **********/
    while ((*str1 != '\0') && (*str2 != '\0')) {
	/**********
	  convert to a wc, if it is an invalid wc, assume a length of 1
	  and continue. If the collating value is 0 (non-collating) get the
	  collating value of the next character
	***********/
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str1++ & 0x00ff;
#else
	    if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str1++ & 0x00ff;
	    }
	    else
		str1 += rc;
#endif /* __SINGLE_BYTE */
	    str1_pos++;
	    str1_colval = __wccollwgt(wc)[order];
	} while ((str1_colval == 0) && (*str1 != '\0'));
	
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str2++ & 0x00ff;
#else
	    if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str2++ & 0x00ff;
	    }
	    else
		str2 += rc;
#endif /* __SINGLE_BYTE */
	    str2_pos++;
	    str2_colval = __wccollwgt(wc)[order];
	} while ((str2_colval == 0) && (*str2 != '\0'));
	
	/**********
	  If the collating values are not zero, check the position
	  in the string. 
	**********/
	if (str1_colval !=0 && str2_colval != 0) {
	    if (str1_pos != str2_pos)
		return (str1_pos - str2_pos);
	}

	/**********
	  if the collation values are not equal, then we have gone far
	  enough and may return
	**********/
	if (str1_colval < str2_colval)
	    return(-1);
	else if (str1_colval > str2_colval)
	    return(1);
    }
    
    /*********
      to get here, str1 and/or str2 are NULL and they have been equal so far.
      If one of them is non-null, check if the remaining characters are
      non-collating. If they are, then the strings are equal for this order.
    *********/
    /**********
      If str1 is non-null and there are collating characters left, then
      str1 is greater than str2
    **********/
    if (*str1 != '\0') {
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str1++ & 0x00ff;
#else
	    if ((rc = (mbtowc(&wc, str1, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str1++ & 0x00ff;
	    }
	    else
		str1 += rc;
#endif /* __SINGLE_BYTE */
	    str1_colval = __wccollwgt(wc)[order];
	} while ((str1_colval == 0) && (*str1 != '\0'));
	if (str1_colval != 0)
	    return(1);
    }
    
    
    /**********
      if str2 is non-null and there are collating characters left, then
      str1 is less than str2
    **********/
    else if (*str2 != '\0') {
	do {
#ifdef __SINGLE_BYTE__
	    wc = (wchar_t) *str2++ & 0x00ff;
#else
	    if ((rc = (mbtowc(&wc, str2, MB_CUR_MAX))) == -1) {
		errno = EINVAL;
		wc = (wchar_t) *str2++ & 0x00ff;
	    }
	    else
		str2 += rc;
#endif /* __SINGLE_BYTE */
	    str2_colval = __wccollwgt(wc)[order];
	} while ((str2_colval == 0) && (*str2 != '\0'));
	if (str2_colval != 0)
	    return(-1);
    }

    /**********
      if we get to here, they are equal
    **********/
    return(0);
}

