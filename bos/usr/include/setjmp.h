/* @(#)01	1.13  src/bos/usr/include/setjmp.h, incstd, bos411, 9428A410j 6/24/94 16:45:08 */

/*
 * COMPONENT_NAME: (INCSTD)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_SETJMP
#define _H_SETJMP

#ifndef _H_STANDARDS
#include <standards.h>
#endif /* _H_STANDARDS */

/*
 *
 *      The ANSI and POSIX standards require that certain values be in setjmp.h.
 *      They also require that if _ANSI_C_SOURCE or _POSIX_SOURCE is defined 
 *      then ONLY those standard specific values are present. This header 
 *      includes all the ANSI and POSIX required entries.
 *
 */

#ifdef _ANSI_C_SOURCE
#define _JBLEN	64	/* regs, fp regs, cr, sigmask, context, etc. */
typedef int jmp_buf[_JBLEN];

#ifdef   _NO_PROTO
extern void longjmp();
extern int setjmp(); 

#else  /*_NO_PROTO */

extern void longjmp(jmp_buf, int);
extern int setjmp(jmp_buf); 

#endif /*_NO_PROTO */

#if __STDC__ == 1
#define	setjmp(__X)	setjmp(__X)
#endif	/* __STDC__ */

#endif /* _ANSI_C_SOURCE */

#ifdef _POSIX_SOURCE

typedef int sigjmp_buf[_JBLEN];
 
#ifdef _NO_PROTO
extern int sigsetjmp();
extern void siglongjmp();
#else /* _NO_PROTO */
extern int sigsetjmp(sigjmp_buf, int);
extern void siglongjmp(sigjmp_buf, int);
#endif /* _NO_PROTO */

#if __STDC__ == 1
#define	sigsetjmp(__X, __Y)	sigsetjmp(__X, __Y)
#endif	/* __STDC__ */

#endif /* _POSIX_SOURCE */

#ifdef _ALL_SOURCE
#ifdef _NO_PROTO
extern int _setjmp();
extern void _longjmp();
#else /* _NO_PROTO */
extern int _setjmp(jmp_buf);
extern void _longjmp(jmp_buf, int);
#endif /* _NO_PROTO */

#pragma reachable (setjmp, _setjmp, sigsetjmp)
#pragma leaves (longjmp, _longjmp, siglongjmp)

#endif /* _ALL_SOURCE */

#endif /* _H_SETJMP */
