static char sccsid[] = "@(#)53	1.2.1.2  src/bos/usr/ccs/lib/libc/__get_wctype_std.c, libcchr, bos411, 9428A410j 1/12/93 11:09:11";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __get_wctype_std
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *
 * FUNCTION: 
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */
#include <ctype.h>
#include <string.h>

wctype_t __get_wctype_std(_LC_ctype_objhdl_t hdl, char *name)
{
    int i;
    int rc = -1;

    /**********
      search thru the list of names and try to
      find a match
    **********/
    for (i=0; i<__OBJ_DATA(hdl)->nclasses &&
	      (rc=strcmp(name, __OBJ_DATA(hdl)->classnms[i].name)) > 0; i++);

    /**********
      if a match was found, return the mask
    **********/
    if (rc==0)
	return(__OBJ_DATA(hdl)->classnms[i].mask);
    
    else
	return(-1);
}
