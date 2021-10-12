static char sccsid[] = "@(#)07	1.8  src/bos/usr/ccs/lib/libc/POWER/fp_const.c, libccnv, bos411, 9428A410j 9/29/93 16:27:25";
/*
 * COMPONENT_NAME: LIBCCNV floating point constants
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fp.h>

/*
 * NAME: fp constants
 *                                                                    
 * FUNCTION: define several floating point constants
 *                                                                    
 * NOTES:
 *
 *      This file contains the external definitions for various
 *      values declared in float.h and math.h. This sort of definition
 *      is allowed in ANSI C.
 *
 *      For FORTRAN programs these external variables can be
 *      referenced directly as external variables. Thus the names
 *      below must be FORTRAN acceptable labels.
 *
 *
 */

/*
 *      Doubles
 */


	unsigned int DEPSILON[2]    = { INTS2DBL(0x3cb00000, 0x00000000) };
	unsigned int DFPMIN[2]      = { INTS2DBL(0x00100000, 0x00000000) };
	unsigned int DFPMAX[2]      = { INTS2DBL(0x7fefffff, 0xffffffff) };
	unsigned int _DBLINF[2]     = { INTS2DBL(0x7ff00000, 0x00000000) };
	unsigned int DQNAN[2]       = { INTS2DBL(0x7ff80000, 0x00000000) };
	unsigned int DSNAN[2]       = { INTS2DBL(0x7ff55555, 0x55555555) };

/*
 *      Floats (SINGLES)
 */

	unsigned int SEPSILON  = 0x34000000;
	unsigned int SFPMIN    = 0x00800000;
	unsigned int SFPMAX    = 0x7f7fffff;
	unsigned int _SFPMAX    = 0x7f7fffff;
	unsigned int SINFINITY = 0x7f800000;
	unsigned int SQNAN     = 0x7fc00000;
	unsigned int SSNAN     = 0x7f855555;

	/*
	    The following definition is for binary compatibility .
	   _DBLINF should now be used
         */
	unsigned int DINFINITY[2]   = { INTS2DBL(0x7ff00000, 0x00000000) };
