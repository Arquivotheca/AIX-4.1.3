static char sccsid[] = "@(#)73	1.2.1.2  src/bos/usr/ccs/lib/libc/__localeconv_std.c, libcloc, bos411, 9428A410j 1/12/93 11:09:44";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __localeconv_std
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
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <locale.h>

#include <locale.h>

struct lconv *__localeconv_std(_LC_locale_objhdl_t hdl)
{
  return __OBJ_DATA(hdl)->nl_lconv;
}
