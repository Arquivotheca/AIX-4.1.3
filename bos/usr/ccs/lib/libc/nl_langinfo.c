static char sccsid[] = "@(#)70	1.2.1.2  src/bos/usr/ccs/lib/libc/nl_langinfo.c, libcloc, bos411, 9428A410j 1/12/93 11:18:11";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: nl_langinfo
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * FUNCTION: nl_langinfo
 *
 * DESCRIPTION: stub function which invokes locale specific method 
 * which implements the nl_langinfo() function.
 *
 * RETURNS:
 * char * ptr to locale string.
 */
#include <sys/lc_sys.h>
#include <langinfo.h>

char * nl_langinfo(nl_item item)
{
  return _CALLMETH(__lc_locale,__nl_langinfo)(__lc_locale, item);
}
