static char sccsid[] = "@(#)73	1.6.2.5  src/bos/usr/ccs/lib/libc/__strxfrm_sb.c, libcstr, bos411, 9428A410j 5/4/94 16:04:41";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __strxfrm_sb
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

int forward_xfrm_sb(_LC_collate_objhdl_t hdl, const char *str_in, 
			char *str_out, int count, int n, int order);

int fast_forward_xfrm_sb(_LC_collate_objhdl_t hdl, const char *str_in, 
			char *str_out, int count, int n, int order);

int forw_pos_xfrm_sb(_LC_collate_objhdl_t hdl, const char *str_in, 
			char *str_out, int count, int n, int order);

int fast_forw_pos_xfrm_sb(_LC_collate_objhdl_t hdl, const char *str_in, 
			char *str_out, int count, int n, int order);
/*
 * FUNCTION: __strxfrm_sb
 *
 * PARAMETERS:
 *           _LC_collate_objhdl_t hdl - unused
 *
 *	     char *str_out - output string of collation weights.
 *	     char *str_int - input string of characters.
 *           size_t n     - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to str_out.
 */

size_t __strxfrm_sb(_LC_collate_objhdl_t hdl, char *str_out, const char *str_in, size_t n)
{
    
    int	cur_order;
    char *str_in_rep=(char *)NULL;
    int rev_start;
    int i;
    char sv1;
    char sv2;
    char save[5];
    int rc;
    int count = 0;
    int sort_mod;
    int  copy_start;
    int  xfrm_byte_count;
    int  limit;
    int  fast_locale;

    fast_locale = FAST_LOCALE;
    /**********
      loop thru the orders
    **********/
    for (cur_order=0; (cur_order<= __OBJ_DATA(hdl)->co_nord); cur_order++) {
	
	/**********
	  get the sort modifier for this order
	**********/
	sort_mod = __OBJ_DATA(hdl)->co_sort.n[cur_order];

#if 0
        /**********
          if this order uses replacement strings, set them up
        **********/
        if (__OBJ_DATA(hdl)->co_nsubs && !(sort_mod & _COLL_NOSUBS_MASK)) {
            if (! str_in_rep) {
                str_in_rep = alloca(strlen(str_in) * 2 + 20);
            }

            str_in_ptr = do_replacement(hdl, str_in, cur_order, str_in_rep);
        }
        else
            str_in_ptr = str_in;
#endif
	/**********
	  check for direction of the collation for this order
	**********/
	/**********
	  CHARACTER
	**********/
	if (sort_mod == 0) {
	    /**********
	      until char collation is defined
	    ***********/
	    if (fast_locale)
	        count = fast_forward_xfrm_sb(hdl, str_in, str_out, count,
				     n, cur_order);
	    else
	        count = forward_xfrm_sb(hdl, str_in, str_out, count,
				     n, cur_order);
	}

	/****************
	    backwards COLLATION
	****************/
	else if (sort_mod & _COLL_BACKWARD_MASK) {
	    rev_start = count;
	    if (sort_mod & _COLL_POSITION_MASK) {
		if (fast_locale)
	    	    count = fast_forw_pos_xfrm_sb(hdl, str_in, str_out, count,
				     n, cur_order);
		else
	    	    count = forw_pos_xfrm_sb(hdl, str_in, str_out, count,
				     n, cur_order);
		xfrm_byte_count = 4;
	    }
	    else {
		if (fast_locale)
	    	    count = fast_forward_xfrm_sb(hdl, str_in, str_out, count,
				     n, cur_order);
                else
	    	    count = forward_xfrm_sb(hdl, str_in, str_out, count,
				     n, cur_order);
		xfrm_byte_count = 2;
	    }
	    /**********
	      reverse the collation orders
	      Change:
		  +-------+-------+-------+-------+-------+-------+-------+-------+
		  | a1|a2 | b1|b2 | c1|c2 | d1|d2 | e1|e2 | f1|f2 | g1|g2 |low|low|
		  +-------+-------+-------+-------+-------+-------+-------+-------+
		To:   
		  +-------+-------+-------+-------+-------+-------+-------+-------+
		  | g1|g2 | f1|f2 | e1|e2 | d1|d2 | c1|c2 | b1|b2 | a1|a2 |low|low|
		  +-------+-------+-------+-------+-------+-------+-------+-------+
	    **********/
	    limit = (count - rev_start - xfrm_byte_count)/(2*xfrm_byte_count);
	    copy_start = count-xfrm_byte_count;
	    for (i=0; i<limit && str_out; i++) {
		copy_start -=  xfrm_byte_count;
		strncpy(save, &str_out[rev_start], xfrm_byte_count);
		strncpy(&str_out[rev_start], &str_out[copy_start], xfrm_byte_count);
		strncpy(&str_out[copy_start], save, xfrm_byte_count);
		rev_start += xfrm_byte_count;
	    }
	    
	    
	}

	/**********
	  forward is the default
	**********/
	else {
	    if (sort_mod & _COLL_POSITION_MASK)  {
		if (fast_locale)
		    count = fast_forw_pos_xfrm_sb(hdl, str_in, str_out, count,
				    n, cur_order);
		else
		    count = forw_pos_xfrm_sb(hdl, str_in, str_out, count,
				    n, cur_order);
		}
	    else {
                if (fast_locale)
		    count = fast_forward_xfrm_sb(hdl, str_in, str_out, count,
				    n, cur_order);
                else
		    count = forward_xfrm_sb(hdl, str_in, str_out, count,
				    n, cur_order);
		 }
	}
    }

    /**********
      special case, forward_xfrm_sb does not handle n == 1.
    **********/
    if (n == 1 && str_out)
	*str_out = '\0';
    
    return(count);
}

/* The forward_xfrm_sb and fast_forward_xfrm_sb are identical
 * routines except that fast_forward_xfrm_sb only looks at one
 * weight, whereas forward_xfrm_sb has to look through all weights
 * and must use getcolval to do it.  This happens inside of a tight
 * loop, so rather than have a single routine which would be slower
 * in the first case, it was split into these two routines.  Since
 * maintaining this code becomes more difficult if you have to make
 * the same fix in two places, the code has been merged with the
 * differences between these two paths set in #ifdef __FAST__ and
 * stored in the strxfrm_routines.c file.  This file is then 
 * included once with __FAST__ turned off and once with it turned
 * on.  The exact same is true of forw_pos_xfrm_sb and 
 * fast_forw_pos_xfrm_sb.
 * Actually, these routines are further shared with __strxfrm_std.c
 * in which the _sb suffixes are replaced with _std.  The macro
 * __SINGLE_BYTE__, is then used to distingush the _sb path from
 * the _std path where needed. 
 */

#define __SINGLE_BYTE__

#undef __FAST__
#define FORWARD_XFRM            forward_xfrm_sb
#define FORW_POS_XFRM           forw_pos_xfrm_sb
#include "strxfrm_routines.c"

#define __FAST__
#undef FORWARD_XFRM
#undef FORW_POS_XFRM
#define FORWARD_XFRM            fast_forward_xfrm_sb
#define FORW_POS_XFRM           fast_forw_pos_xfrm_sb
#include "strxfrm_routines.c"
