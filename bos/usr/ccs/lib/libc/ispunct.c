static char sccsid[] = "@(#)41	1.2.1.2  src/bos/usr/ccs/lib/libc/ispunct.c, libcchr, bos411, 9428A410j 1/12/93 11:16:42";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: ispunct
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
 * FUNCTION: Determines if a character is punctuation
 *	    
 *
 * PARAMETERS: c  -- character to be classified
 *
 *
 * RETURN VALUES: 0 -- if c is not punctuation
 *                >0 - If c is punctuation
 *
 *
 */
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <ctype.h>

#ifdef ispunct
#undef ispunct
#endif

int ispunct(int c)
{
	return( _CALLMETH(__lc_ctype, __is_wctype)(__lc_ctype, (wint_t)c, _ISPUNCT) );
}
