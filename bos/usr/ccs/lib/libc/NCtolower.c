static char sccsid[] = "@(#)56	1.1  src/bos/usr/ccs/lib/libc/NCtolower.c, libcnls, bos411, 9428A410j 2/26/91 17:44:32";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: NCtolower
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <ctype.h>

/*
 *
 *  If arg is upper-case, return the lower-case, else return the arg.
 *  This version uses _NLctab to classify c according to the current
 *  NLS environment.
 */

int
NCtolower(pwc)
register wchar_t pwc;
{
	return ( towlower(pwc) );
}
