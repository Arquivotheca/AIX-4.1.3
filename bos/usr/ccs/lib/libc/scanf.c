static char sccsid[] = "@(#)56  1.17  src/bos/usr/ccs/lib/libc/scanf.c, libcio, bos411, 9428A410j 10/3/93 13:54:46";
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

#define DOSCAN     _doscan

#include "scanf_macros.c"




