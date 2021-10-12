static char sccsid[] = "@(#)51	1.1  src/bos/usr/ccs/lib/libc/printf128.c, libcprnt, bos411, 9428A410j 10/3/93 13:15:13";
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: __printf128, __fprintf128, __sprintf128, __vfprintf128,
 *            __vprintf128, __vsprintf128, __vwsprintf128, __wsprintf128
 *
 * ORIGINS: 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define PRINTF     __printf128
#define FPRINTF    __fprintf128
#define SPRINTF    __sprintf128
#define VFPRINTF   __vfprintf128
#define VPRINTF    __vprintf128
#define VSPRINTF   __vsprintf128
#define VWSPRINTF  __vwsprintf128
#define WSPRINTF   __wsprintf128
#define DOPRNT     _doprnt128

#include "printf_macros.c"
