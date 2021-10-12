static char sccsid[] = "@(#)54  1.1  src/bos/usr/ccs/lib/libc/scanf64.c, libcio, bos411, 9428A410j 10/3/93 13:56:54";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: __scanf64, __fscanf64, __sscanf64, __wsscanf64
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

#define SCANF     __scanf64
#define FSCANF    __fscanf64
#define SSCANF    __sscanf64
#define WSSCANF   __wsscanf64

#define DOSCAN     _doscan

#include "scanf_macros.c"




