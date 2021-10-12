static char sccsid[] = "@(#)63	1.4.1.2  src/bos/usr/ccs/lib/libc/__csid_std.c, libcchr, bos411, 9428A410j 1/12/93 11:08:53";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __csid_std (method)
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

#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <stdlib.h>

/*
*  FUNCTION: __csid_std
*
*  DESCRIPTION: 
*  This method returns the character set of which the specified character 
*  is a member.
*/
int __csid_std(_LC_charmap_objhdl_t hdl, char *mbs)
{
    wchar_t pwc;
    int     rc;

    rc = _CALLMETH(hdl,__mbtowc)(hdl,&pwc, mbs, __OBJ_DATA(hdl)->cm_mb_cur_max);

    if (rc < 0)
	return 0;
    else
	return __OBJ_DATA(hdl)->cm_cstab[pwc];
}
