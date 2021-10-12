/* @(#)48	1.2  src/bos/usr/ccs/lib/libc/POWER/atof.h, libccnv, bos411, 9428A410j 7/11/91 17:25:10 */
/*
 * COMPONENT_NAME: (LIBCCNV) 
 *
 * FUNCTIONS: none
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
 * typedef for the power of two tables used
 * by the string to double conversion functions
 */

extern struct fp __pospow1[][2];
extern struct fp __pospow2[][2];
extern struct fp __negpow1[][2];
extern struct fp __negpow2[][2];

#define FP_BAD_STRING (0x8000)
