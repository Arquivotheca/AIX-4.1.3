static char sccsid[] = "@(#)64	1.2.1.2  src/bos/usr/ccs/lib/libc/__wcsid_std.c, libcchr, bos411, 9428A410j 1/12/93 11:11:31";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __wcsid_std (method)
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
*  FUNCTION: __wcsid_std
*
*  DESCRIPTION: 
*  This method returns the character set of which the specified wide 
*  character is a member.
*/
int __wcsid_std(_LC_charmap_objhdl_t hdl, wchar_t wc)
{
    return __OBJ_DATA(hdl)->cm_cstab[wc];
}
