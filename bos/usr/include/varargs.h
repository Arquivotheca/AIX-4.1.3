/* @(#)90	1.10  src/bos/usr/include/varargs.h, libcgen, bos411, 9428A410j 1/12/93 17:03:28 */

/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_VARARGS
#define _H_VARARGS

#include <va_list.h>	/* va_list */

#define va_dcl int va_alist;

#define va_start(__list) __list = (char *) &va_alist
#define va_end(__list)
#define va_arg(__list, __mode) ((__mode *)(__list += sizeof(__mode)))[-1]

#endif /* _H_VARARGS */
