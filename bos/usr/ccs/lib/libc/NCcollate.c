static char sccsid[] = "@(#)64	1.4.1.3  src/bos/usr/ccs/lib/libc/NCcollate.c, libcnls, bos411, 9428A410j 3/10/94 11:20:40";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCcollate
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

/* This function returns the primary collation value for the wchar_t
 * argument. The returned value can be either 
 *	positive -	the real primary (lc_collate) value 
 *	zero     -	the wchar_t is non-collating (ignore)
 *      negative -	the real collation value is to be found in
 *			lc_coldesc (see NLxcol)
 */

#include <sys/localedef.h>
#include <ctype.h>
#include <stdlib.h>

int NCcollate(wchar_t nlc)
{
    int colval;
    char *wgt_str=NULL;
    char *tmp_str=NULL;
    int done;


    /* get primary (first) weight */

    (void)_getcolval(__lc_collate,&colval,nlc,tmp_str,0,&wgt_str,&done);
    
    /**********
      If this is the first element of a collation element
      then return the negative of the wchar
    **********/
    if (__OBJ_DATA(__lc_collate)->co_coltbl[nlc].ct_collel != 
	(_LC_collel_t *)NULL)
	return (-nlc);

    /***************
      Since the these routines do not support 1-to-many mapping, simply return the
      first collation weight returned by _getcolval.
    if (!done) 
	return (-nlc);
    *************/
	     
    /**********
      if we are here, then there is nothing special about this
      character
    **********/
    return (colval);
}
