/* @(#)60       1.12  src/bos/usr/ccs/lib/libc/print.h, libcprnt, bos41J, 9509A_all 12/9/94 13:30:48 */
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Maximum number of digits in any integer representation */
/* With support for 64-bit long long integers, the largest */
/* unsigned number converted to octal is 22 characters long */
#define MAXDIGS 22

/* Maximum total number of digits in E format -- AIX documentation
 * limits the number of digits producted by ecvt to 17 for double
 * precision.  Although not documented, we similarly limit 128 bit
 * long double to 35 digits.  Although not specified in the documentation,
 * we make similar limitations for G format as well.
 */
#define MAXECVT 17
#ifdef LD_128
#define MAXQECVT 35
#endif

/* Max and Min macros */
#define max(a,b) ((a) > (b)? (a): (b))
#define min(a,b) ((a) < (b)? (a): (b))
