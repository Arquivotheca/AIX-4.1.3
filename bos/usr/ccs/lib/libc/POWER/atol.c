static char sccsid[] = "@(#)40	1.1  src/bos/usr/ccs/lib/libc/POWER/atol.c, libccnv, bos411, 9428A410j 5/1/91 10:28:23";
/*
 * COMPONENT_NAME: LIBCCNV atol
 *
 * FUNCTIONS: atol
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * NAME: atol
 *                                                                    
 * FUNCTION: Converts a string to an long
 *                                                                    
 * NOTES:
 *
 * RETURNS: returns a long formed from *nptr
 *	    INT_MIN or INT_MAX of overflow occurs on conversion
 *
 */

#define ATOL
#include "atoi.c"
