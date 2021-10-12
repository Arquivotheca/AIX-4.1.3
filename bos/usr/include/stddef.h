/* @(#)66	1.12  src/bos/usr/include/stddef.h, incstd, bos411, 9428A410j 4/27/93 08:38:03 */

/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_STDDEF 
#define _H_STDDEF 

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *
 *      The ANSI standard requires that certain values be in stddef.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present. This header includes all the ANSI required entries.
 *
 */

#ifdef _ANSI_C_SOURCE

/*
 *	The following definitions are included in <sys/types.h>.  They
 *	are included in <stddef.h> to comply with ANSI.
 */

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef	int		ptrdiff_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef	unsigned long	size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef unsigned short	wchar_t;
#endif

#ifndef NULL
#define	NULL	0
#endif

#define offsetof(__s_name, __s_member) (size_t)&(((__s_name *)0)->__s_member)

#endif /* _ANSI_C_SOURCE */
#endif /* _H_STDDEF */
