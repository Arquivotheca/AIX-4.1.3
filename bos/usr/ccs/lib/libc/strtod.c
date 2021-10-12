static char sccsid[] = "@(#)40	1.4  src/bos/usr/ccs/lib/libc/strtod.c, libccnv, bos411, 9428A410j 6/16/90 01:05:47";
/*
 * COMPONENT_NAME: LIBCCNV strtod
 *
 * FUNCTIONS: strtod
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: strtod
 *                                                                    
 * FUNCTION: convert character string to double
 *                                                                    
 * NOTES:
 *
 *      C library - string to double (strtod)
 *
 *      This version compiles strtod because STRTOD is set for atof.c.
 *      The only difference between atof and strtod is the storage
 *      of a pointer to the character which terminated the conversion.
 *
 * RETURNS: a double value
 */

#ifndef STRTOD
#define STRTOD	1
#endif
#include "atof.c"
