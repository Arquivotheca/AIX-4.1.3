static char sccsid[] = "@(#)52	1.1  src/bos/usr/ccs/lib/libc/printf64.c, libcprnt, bos411, 9428A410j 10/3/93 13:15:15";
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: __printf64, __fprintf64, __sprintf64, __vfprintf64,
 *            __vprintf64, __vsprintf64, __vwsprintf64, __wsprintf64
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

#define PRINTF     __printf64
#define FPRINTF    __fprintf64
#define SPRINTF    __sprintf64
#define VFPRINTF   __vfprintf64
#define VPRINTF    __vprintf64
#define VSPRINTF   __vsprintf64
#define VWSPRINTF  __vwsprintf64
#define WSPRINTF   __wsprintf64
#define DOPRNT     _doprnt

#include "printf_macros.c"
