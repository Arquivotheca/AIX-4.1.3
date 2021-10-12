static char sccsid[] = "@(#)99	1.2  src/bos/usr/ccs/lib/libsvid/matherr.c, libsvid, bos411, 9428A410j 6/16/90 02:38:32";
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: matherr
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <math.h>

int
matherr(x)
struct exception *x;
{
	return (0);
}
