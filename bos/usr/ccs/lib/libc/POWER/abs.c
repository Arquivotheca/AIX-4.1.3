static char sccsid[] = "@(#)57	1.2  src/bos/usr/ccs/lib/libc/POWER/abs.c, libccnv, bos411, 9428A410j 5/2/91 15:03:18";
/*
 * COMPONENT_NAME: LIBCCNV abs
 *
 * FUNCTIONS: abs
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
 * NAME: abs
 *                                                                    
 * FUNCTION: Return absolute value of an integer
 *                                                                    
 *                                                                   
 * NOTES:
 *
 * This R2 version uses the compiler built-in __abs()
 * to get directly to the abs machine instruction.
 *
 * RETURNS: integer absolute value of j
 *
 */

/* make sure, just in case,
 * that the define in stdlib.h is
 * not on.
 */

#ifdef abs
#undef abs
#endif

int abs(int j)
  {
  return (__abs(j));
  }
