static char sccsid[] = "@(#)86	1.2  src/bos/usr/ccs/lib/libc128/scanf.c, libc128, bos411, 9428A410j 10/3/93 14:00:43";
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: scanf, fscanf, sscanf, wsscanf
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

#define SCANF     scanf
#define FSCANF    fscanf
#define SSCANF    sscanf
#define WSSCANF   wsscanf

#define DOSCAN     _doscan128

#include "scanf_macros.c"	/* found in usr/ccs/lib/libc directory */




