/* @(#)55	1.15  src/bos/usr/include/stdarg.h, incstd, bos411, 9428A410j 1/12/93 17:01:18 */

/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_STDARG
#define _H_STDARG

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *      The ANSI standard requires that certain values be in stdarg.h.
 *
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present. This header includes all the ANSI required entries. 
 *
 */


#ifdef _ANSI_C_SOURCE

/* ANSI requires that va_list is defined here */

#include <va_list.h>	/* va_list */

/* Argument list convention: Each argument is allocated a multiple of 4 bytes.
 * Each argument occupies the high-order bytes within its allocated multiple
 * of 4 bytes.  This implementation is restricted to one-quantity arguments.
 * The implementation does not support argument descriptors, which immediately
 * follow their respective argument value word(s) if they are used.
 */
#define va_start(__list,__parmN) __list = (char *)((unsigned int)&(__parmN) + (((sizeof(__parmN)+3)/4)*4))
#define va_end(__list) (__list)=(char *)0
#define va_arg(__list, __mode) ((__mode *)( (((__list)+=(((sizeof(__mode)+3)/4)*4))-sizeof(__mode)) ))[0]

#endif /* _ANSI_C_SOURCE */
#endif /* _H_STDARG */
