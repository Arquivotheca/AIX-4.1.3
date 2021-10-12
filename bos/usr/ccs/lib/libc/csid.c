static char sccsid[] = "@(#)62	1.2.1.2  src/bos/usr/ccs/lib/libc/csid.c, libcchr, bos411, 9428A410j 1/12/93 11:13:08";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: csid
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
*  FUNCTION: csid
*
*  DESCRIPTION: 
*  This is the function stub which calls the method which implements csid().
*/
int csid(char *mbs)
{
    extern _LC_charmap_objhdl_t __lc_charmap;

    return _CALLMETH(__lc_charmap,__csid)(__lc_charmap, mbs);
}

