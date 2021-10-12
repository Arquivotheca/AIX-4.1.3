static char sccsid[] = "@(#)69	1.2.1.2  src/bos/usr/ccs/lib/libc/localeconv.c, libcloc, bos411, 9428A410j 1/12/93 11:17:53";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: localeconv
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
 * FUNCTION: localeconv
 *	    
 * DESCRIPTION: stub function which invokes the locale specific method
 * which implements the localeconv() function.
 *
 * RETURN VALUE: 
 * struct lconv * ptr to populated lconv structure
 */
#include <sys/lc_sys.h>
#include <sys/localedef.h>
#include <locale.h>

struct lconv *localeconv(void)
{
  return _CALLMETH(__lc_locale,__localeconv)(__lc_locale);
}
