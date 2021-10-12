/* @(#)44	1.7  src/bos/usr/include/standards.h, incstd, bos411, 9428A410j 6/16/90 00:14:02 */
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_STANDARDS
#define _H_STANDARDS

/* These directives must be processed in the current order when compiled with 
 * cc or they will not work correctly.
 */


#ifdef _XOPEN_SOURCE
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#ifndef _ANSI_C_SOURCE
#define _ANSI_C_SOURCE
#endif
#endif

#ifdef _POSIX_SOURCE
#ifndef _ANSI_C_SOURCE
#define _ANSI_C_SOURCE
#endif
#endif

#ifdef _ALL_SOURCE
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#ifndef _ANSI_C_SOURCE
#define _ANSI_C_SOURCE
#endif
#endif

#if (!defined (_XOPEN_SOURCE)) &&  (!defined (_POSIX_SOURCE)) && (!defined (_ANSI_C_SOURCE))
#define _XOPEN_SOURCE
#define _POSIX_SOURCE
#define _ANSI_C_SOURCE
#ifndef _ALL_SOURCE
#define _ALL_SOURCE
#endif
#endif

#endif /* _H_STANDARDS */
