static char sccsid[] = "@(#)12	1.2.1.2  src/bos/usr/ccs/lib/libc/iswcntrl.c, libcchr, bos411, 9428A410j 1/12/93 11:17:08";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: iswcntrl
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
 * FUNCTION: Determines if the process code, pc, is a control character
 *	    
 *
 * PARAMETERS: pc  -- character to be classified
 *
 *
 * RETURN VALUES: 0 -- if pc is not a control character
 *                >0 - If c is a control character
 *
 *
 */
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <ctype.h>

#ifdef iswcntrl
#undef iswcntrl
#endif

int iswcntrl(wint_t pc)
{
	return( _CALLMETH(__lc_ctype, __is_wctype)(__lc_ctype, pc, _ISCNTRL) );
}
