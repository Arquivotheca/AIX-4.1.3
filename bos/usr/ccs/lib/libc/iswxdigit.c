static char sccsid[] = "@(#)20	1.2.1.2  src/bos/usr/ccs/lib/libc/iswxdigit.c, libcchr, bos411, 9428A410j 1/12/93 11:17:42";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: iswxdigit
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *
 * FUNCTION: Determines if the process code, pc, is a hex digit
 *	    
 *
 * PARAMETERS: pc  -- character to be classified
 *
 *
 * RETURN VALUES: 0 -- if pc is not a hex digit
 *                >0 - If c is a hex digit
 *
 *
 */
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <ctype.h>

#ifdef iswxdigit
#undef iswxdigit
#endif

int iswxdigit(wint_t pc)
{
	return( _CALLMETH(__lc_ctype, __is_wctype)(__lc_ctype, pc, _ISXDIGIT) );
}
