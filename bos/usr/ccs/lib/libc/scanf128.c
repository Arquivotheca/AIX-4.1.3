static char sccsid[] = "@(#)55  1.1  src/bos/usr/ccs/lib/libc/scanf128.c, libcio, bos411, 9428A410j 10/3/93 13:56:56";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: __scanf128, __fscanf128, __sscanf128, __wsscanf128
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

#define SCANF     __scanf128
#define FSCANF    __fscanf128
#define SSCANF    __sscanf128
#define WSSCANF   __wsscanf128

#define DOSCAN     _doscan128

#include "scanf_macros.c"




