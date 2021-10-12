static char sccsid[] = "@(#)67	1.5.2.5  src/bos/usr/ccs/lib/libc/__strcoll_std.c, libcstr, bos411, 9428A410j 5/4/94 16:04:39";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __strcoll_std
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#pragma alloca

#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include "patlocal.h"


int backward_collate_std(_LC_collate_objhdl_t hdl, char *str1,
			    char *str2, int order);

int fast_backward_collate_std(_LC_collate_objhdl_t hdl, char *str1,
			    char *str2, int order);

int char_collate_std(_LC_collate_objhdl_t hdl, char *str1, char *str2);

int forward_collate_std(_LC_collate_objhdl_t hdl, char *str1,
			   char *str2, int order);

int fast_forward_collate_std(_LC_collate_objhdl_t hdl, char *str1,
			   char *str2, int order);

/*
 * FUNCTION: Compares the strings pointed to by str1 and str2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If str1 is less than str2
 *		Equal to 0	If str1 is equal to str2
 *		Greater than 0	If str1 is greater than str2.
 *
 *	     The comparison is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *str1 - first string
 *	     char *str2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */

int __strcoll_std(_LC_collate_objhdl_t hdl, const char *str1, const char *str2)
{
    int	cur_order;  /* current order being collated */
    short sort_mod; /* the current order's modification params */
    int fast_locale;/* if no 1-to-many mapping or multi-character */
		    /* collating elements, then we can use fast routines */
    
    int rc;         /* generic return code */

    /**********
      see if str1 and str2 are the same string
    **********/
    if (str1 == str2)
	return(0);

    fast_locale = FAST_LOCALE;
    
    /**********
      if str1 and str2 are null, they are equal
    **********/
    if (*str1 == '\0' && *str2 == '\0')
	return(0);
    
    for (cur_order=0; cur_order<= __OBJ_DATA(hdl)->co_nord; cur_order++) {
	/**********
	  get the sort modifier for this order
	**********/
	sort_mod = __OBJ_DATA(hdl)->co_sort.n[cur_order];
	
#if 0
        /**********
          if this order uses replacement strings, set them up
        **********/
        if (__OBJ_DATA(hdl)->co_nsubs && !(sort_mod & _COLL_NOSUBS_MASK)) {

            if (! str1_rep) {
                str1_rep = alloca(strlen(str1) * 2 + 20);
            }
            str1_ptr = do_replacement(hdl, str1, cur_order, str1_rep);

            if (! str2_rep) {
                str2_rep = alloca(strlen(str2) * 2 + 20);
            }
            str2_ptr = do_replacement(hdl, str2, cur_order, str2_rep);
        }
        /**********
          otherwise use the strings as they came in
        **********/
        else {
            str1_ptr = str1;
            str2_ptr = str2;
        }
#endif

	/**********
	  check for direction of collation for this order.
	  If neither forward nor backward are specified, then
	  this is to be done by character.
	**********/
	/**********
	  CHARACTER: if it is character collation, return the
	  value from char_collate.  It does all of the orders
	**********/
	if (sort_mod == 0) {
	    rc = char_collate_std(hdl, str1, str2);
	    return(rc);
	}

	/**********
	  backwards
	**********/
	else if (sort_mod & _COLL_BACKWARD_MASK) {
	    if (sort_mod & _COLL_POSITION_MASK) {
		if (fast_locale)
	    	    rc = fast_back_pos_collate_std(hdl, str1, str2, cur_order);
		else
	    	    rc = back_pos_collate_std(hdl, str1, str2, cur_order);
		}
	    else {
		if (fast_locale)
	    	    rc = fast_backward_collate_std(hdl, str1, str2, cur_order);
		else
	    	    rc = backward_collate_std(hdl, str1, str2, cur_order);
		}
	}

	/**********
	  or forwards (the default if sort_mod is non-zero)
	**********/
	else {
	    if (sort_mod & _COLL_POSITION_MASK) {
		if (fast_locale)
	    	    rc = fast_forw_pos_collate_std(hdl, str1, str2, cur_order);
		else
	    	    rc = forw_pos_collate_std(hdl, str1, str2, cur_order);
		}
	    else {
		if (fast_locale)
	    	    rc = fast_forward_collate_std(hdl, str1, str2, cur_order);
		else
	    	    rc = forward_collate_std(hdl, str1, str2, cur_order);
		}
	}

	/**********
	  if the strings are not equal, we can leave
	  otherwise continue on to next order
	**********/
	if (rc != 0) {
	    return(rc);
	}
    }
    /**********
      must be equal, return 0
    **********/
    return(0);
}

/* The backward_collate_std and fast_backward_collate_std are identical
 * routines except that fast_backward_collate_std only looks at one
 * weight, whereas backward_collate_std has to look through all weights
 * and must use getcolval to do it.  This happens inside of a tight
 * loop, so rather than have a single routine which would be slower
 * in the first case, it was split into these two routines.  Since
 * maintaining this code becomes more difficult if you have to make
 * the same fix in two places, the code has been merged with the
 * differences between these two paths set in #ifdef __FAST__ and
 * stored in the strcoll_backwd_routines.c file.  This file is then 
 * included once with __FAST__ turned off and once with it turned
 * on.  The exact same is true of back_pos_collate_std and 
 * fast_back_pos_collate_std.
 * Actually, these routines are further shared with __strcoll_sb.c
 * in which the _std suffixes are replaced with _sb.  The macro
 * __SINGLE_BYTE__, is then used to distingush the _sb path from
 * the _std path where needed. 
 */

#undef __SINGLE_BYTE__

#undef __FAST__
#define BACKWARD_COLLATE	backward_collate_std
#define BACK_POS_COLLATE	back_pos_collate_std
#include "strcoll_backwd_routines.c"

#define __FAST__
#undef BACKWARD_COLLATE
#undef BACK_POS_COLLATE
#define BACKWARD_COLLATE	fast_backward_collate_std
#define BACK_POS_COLLATE	fast_back_pos_collate_std
#include "strcoll_backwd_routines.c"

/*
 * Once again, source is being shared with the _sb path.
 * Note that __SINGLE_BYTE__ is again used to traverse
 * the _sb path while undefining it traverses the _std
 * path, but that __FAST__ is not used for these routines
 * because the fast and non-fast paths were not easily
 * intertwined (actually VERY difficult to read if done).
 * Instead, there are four routines defined in
 * strcoll_forwd_routines.c as opposed to two in
 * strcoll_backwd_routines.c.
 */

#define FORWARD_COLLATE		forward_collate_std
#define FAST_FORWARD_COLLATE	fast_forward_collate_std
#define FORW_POS_COLLATE   	forw_pos_collate_std
#define FAST_FORW_POS_COLLATE	fast_forw_pos_collate_std
#include "strcoll_forwd_routines.c"
