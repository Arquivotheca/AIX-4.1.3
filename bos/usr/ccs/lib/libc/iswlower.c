static char sccsid[] = "@(#)15	1.2.1.2  src/bos/usr/ccs/lib/libc/iswlower.c, libcchr, bos411, 9428A410j 1/12/93 11:17:23";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: iswlower
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
 * FUNCTION: Determines if the process code, pc, is lowercase
 *	    
 *
 * PARAMETERS: pc  -- character to be classified
 *
 *
 * RETURN VALUES: 0 -- if pc is not lowercase
 *                >0 - If c is lowercase
 *
 *
 */
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <ctype.h>

#ifdef iswlower
#undef iswlower
#endif

int iswlower(wint_t pc)
{
	return( _CALLMETH(__lc_ctype, __is_wctype)(__lc_ctype, pc, _ISLOWER) );
}
