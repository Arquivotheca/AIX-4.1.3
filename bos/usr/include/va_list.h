/* @(#)26       1.1  src/bos/usr/include/va_list.h, incstd, bos411, 9428A410j 1/13/93 10:57:41 */

#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * Five possible situations:
 * 	- We are being included by {var,std}args.h (or anyone) before stdio.h.
 * 	  define real type.
 *
 * 	- We are being included by stdio.h before {var,std}args.h.
 * 	  define hidden type for prototypes in stdio, don't pollute namespace.
 *
 *      - We are being included by stdio.h the second time in _XOPEN_SOURCE
 *        before {var, std}args.h.
 * 	  define real type to match hidden type.  no longer use hidden type.
 * 
 * 	- We are being included by {var,std}args.h after stdio.h.
 * 	  do nothing.
 * 
 * 	- We are being included again after defining the real va_list.
 * 	  do nothing.
 * 
 */

#if	!defined(_HIDDEN_VA_LIST) && !defined(_VA_LIST)
#define _VA_LIST
typedef	char *va_list;

#elif	defined(_HIDDEN_VA_LIST) && !defined(_VA_LIST)
#define _VA_LIST
typedef char *__va_list;

#elif	defined(_HIDDEN_VA_LIST) && defined(_VA_LIST)
#undef _HIDDEN_VA_LIST
typedef __va_list va_list;

#endif
