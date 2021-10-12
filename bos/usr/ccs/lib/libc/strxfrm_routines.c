#if 0
static char sccsid[] = "@(#)78	1.1  src/bos/usr/ccs/lib/libc/strxfrm_routines.c, libcstr, bos411, 9428A410j 5/4/94 16:47:23";
#endif
/*
 *   COMPONENT_NAME: LIBCSTR
 *
 *   FUNCTIONS: FORWARD_XFRM
 *		FORW_POS_XFRM
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
 *  FORWARD_XFRM and FORW_POS_XFRM are defined by the source files that
 *  #include it.  These are __strxfrm_sb.c and __strxfrm_std.c.  The _sb
 *  file will define __SINGLE_BYTE__ and the _std file will undefine it.
 *  Each of these files includes this file twice.  The first time,
 *  __FAST__ is turned off and the non-fast names are used (i.e.
 *  forward_xfrm_* & forw_pos_xfrm_*.  The second time, it is turned on, 
 *  and the fast names are used (fast_forward_xfrm_* & fast_forw_pos_xfrm_*).
 *  The _* at the end are for ths _sb and _std suffixes and are based on
 *  whether the _sb file included it or the _std file did.
 *
 *  So you get the following routines:
 *      ----------------------------------------------------------------
 *		  | __SINGLE_BYTE__ defined  | __SINGLE_BYTE__ undefined
 *	__FAST__  |     __strxfrm_sb.c	     |	     __strxfrm_std.c
 *      ----------------------------------------------------------------
 *	undefined | forward_xfrm_sb	     | forward_xfrm_std
 *	undefined | forw_pos_xfrm_sb	     | forw_pos_xfrm_std
 *	defined   | fast_forward_xfrm_sb     | fast_forward_xfrm_std
 *	defined   | fast_forw_pos_xfrm_sb    | fast_forw_pos_xfrm_std
 */

int 
FORWARD_XFRM(_LC_collate_objhdl_t hdl, const char *str_in,
			 char *str_out, int count, int n, int order)
{
    char *str_in_ptr;
    char *str_out_ptr;
    wchar_t wc;
    int rc;
    int fill_flag;
    int nn = n-1;
    int str_in_colval;
#ifndef __FAST__
    int done;
    char *wgt_str = NULL;
#endif /* __FAST__ */

    str_in_ptr = str_in;
    str_out_ptr = ((str_out && n) ? &str_out[count] : (char *)NULL);

    if (count < nn)
	fill_flag = 1;
    else
	fill_flag = 0;

    if (!str_out_ptr)
	fill_flag = 0;
    
    /**********
      go thru all of the characters until a null is hit
    **********/
    while (*str_in_ptr != '\0') {
	/**********
	  get the collating value for each character
	**********/
#ifdef __SINGLE_BYTE__
	wc = (wchar_t) (unsigned char)*str_in_ptr++;
#else
	/**********
	  if mbtowc fails, set errno and return.
	**********/
	if ((rc = mbtowc(&wc, str_in_ptr, MB_CUR_MAX)) == -1) {
	    errno = EINVAL;
	    return((size_t)-1);
	}
	str_in_ptr += rc;
#endif /* __SINGLE_BYTE__ */

#ifdef __FAST__
	str_in_colval = __wccollwgt(wc)[order];
#else
	do {
	    str_in_ptr += _getcolval(hdl, &str_in_colval, wc, str_in_ptr, order, &wgt_str, &done);
	
#endif /* __FAST__ */
	
	    /**********
	      if this character has collation, put it in the output string
	    **********/
	    if (str_in_colval != 0) {
	        if (fill_flag) {
		    if ( count < nn )
		        *str_out_ptr++ = (char) (str_in_colval >> 8) & 0x00ff;
		    count++;
		
		    if ( count < nn )
		        *str_out_ptr++ = (char) str_in_colval & 0x00ff;
		    else {
		        *str_out_ptr = '\0';
		        fill_flag = 0;
		    }
		    count++;
	        }
	        else
		    count += 2;
	    }
#ifndef __FAST__
	} while (!done);
#endif /* __FAST__ */
    }

    /**********
	add the low weight
    **********/
    if (fill_flag) {
	if ( count < nn)
	    *str_out_ptr++ = (char) (__OBJ_DATA(hdl)->co_col_min >> 8) & 0x00ff;
	count++;
	
	if (count < nn)
	    *str_out_ptr++ = (char) __OBJ_DATA(hdl)->co_col_min & 0x00ff;
	count++;

	/*********
	  always add a null to the end.  If this was not the last order, it will
	  be overwritten on the next pass
	**********/
	*str_out_ptr = '\0';
    }
    else
	count += 2;
    
    return(count);
}

int 
FORW_POS_XFRM(_LC_collate_objhdl_t hdl, const char *str_in, 
			char *str_out, int count, int n, int order)
{
    char *str_in_ptr;
    char *str_out_ptr;
    wchar_t wc;
    int rc;
    int fill_flag;
    int nn = n-1;
    int str_pos;
    int str_in_colval;
#ifndef __FAST__
    int done;
    char *wgt_str = NULL;
#endif /* __FAST__ */

    str_in_ptr = str_in;
    str_out_ptr = ((str_out && n) ? &str_out[count] : (char *)NULL);

    if (count < nn)
	fill_flag = 1;
    else
	fill_flag = 0;

    if (!str_out_ptr)
	fill_flag = 0;
    
    str_pos = __OBJ_DATA(hdl)->co_col_min;
    /**********
      go thru all of the characters until a null is hit
    **********/
    while (*str_in_ptr != '\0') {
	/**********
	  get the collating value for each character
	**********/
#ifdef __SINGLE_BYTE__
	wc = (wchar_t) (unsigned char)*str_in_ptr++;
#else
	/**********
	  if mbtowc fails, set errno and return.
	**********/
	if ((rc = mbtowc(&wc, str_in_ptr, MB_CUR_MAX)) == -1) {
	    errno = EINVAL;
	    return((size_t)-1);
	}
	str_in_ptr += rc;
#endif /* __SINGLE_BYTE__ */


#ifdef __FAST__
	str_in_colval = __wccollwgt(wc)[order];
#else
	do {
	    str_in_ptr += _getcolval(hdl, &str_in_colval, wc, str_in_ptr, order, &wgt_str, &done);
#endif /* __FAST__ */

	    /**********
	     if this is a multiple of 256, add 1 more (this will make sure that
	     none of the bytes are 0x00
	    **********/
	    str_pos++;
	    if (!(str_pos % 256))
	        str_pos++;
	
	    /**********
	     if this character has collation, put its position and its weight
	     in the output string
	    **********/
	    if (str_in_colval != 0) {
	        if (fill_flag) {
		    if ( count < nn )
		        *str_out_ptr++ = (char) (str_pos >> 8) & 0x00ff;
		    count++;

		    if ( count < nn )
		        *str_out_ptr++ = (char) str_pos & 0x00ff;
		    count++;

		    if ( count < nn )
		        *str_out_ptr++ = (char) (str_in_colval >> 8) & 0x00ff;
		    count++;
		
		    if ( count < nn )
		        *str_out_ptr++ = (char) str_in_colval & 0x00ff;
		    else {
		        *str_out_ptr = '\0';
		        fill_flag = 0;
		    }
		    count++;
	        }
	        else
		    count += 4;
	    }
#ifndef __FAST__
	}  while (!done);
#endif /* __FAST__ */
    }

    /**********
	add the low weight
    **********/
    if (fill_flag) {
	if ( count < nn)
	    *str_out_ptr++ = (char) (__OBJ_DATA(hdl)->co_col_min >> 8) & 0x00ff;
	count++;
	
	if (count < nn)
	    *str_out_ptr++ = (char) __OBJ_DATA(hdl)->co_col_min & 0x00ff;
	count++;

	if ( count < nn)
	    *str_out_ptr++ = (char) (__OBJ_DATA(hdl)->co_col_min >> 8) & 0x00ff;
	count++;
	
	if (count < nn)
	    *str_out_ptr++ = (char) __OBJ_DATA(hdl)->co_col_min & 0x00ff;
	count++;

	/*********
	  always add a null to the end.  If this was not the last order, it will
	  be overwritten on the next pass
	**********/
	*str_out_ptr = '\0';
    }
    else
	count += 4;
    
    return(count);
}
