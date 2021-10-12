/* @(#)94	1.9  src/bos/usr/include/fp.h, sysfp, bos411, 9428A410j 6/16/90 00:10:11 */
/*
 * COMPONENT_NAME: (fp.h) floating point header file
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FP
#define _H_FP

/*
 *	Common header file allowing for floating point exception handling
 *	and containing machine independent macros.
 */

#include <fpxcp.h>
#include <sys/FP.h>

/*
 *      DBL(val,hi,lo)
 *
 *      The hi word of the double variable val is set to hi and
 *      the lo word of the double variable val is set to lo.
 *
 *      val must be a simple variable.
 */

#define DBL(val,hi,lo) \
{ VALH(val) = hi;\
VALL(val) = lo; \
}

/*
 *      FINITE(x)
 *
 *      Is true if double x is finite (Not NaN or INF).
 *
 *      This macro is similar to the finite(x) function in the IEEE
 *      standard except it is not a function. 
 *	x must be a simple variable and not an expresion.
 */

#define FINITE(x) \
	( ( VALH(x) & 0x7ff00000 ) != 0x7ff00000 )

/*
 *      IS_INF(x)
 *
 *      Is true if double x is +INF or -INF.
 *      x must be a simple variable and not an expression.
 */

#define IS_INF(x) \
	( ( ( VALH(x) & 0x7fffffff ) == 0x7ff00000 ) && \
		( VALL(x) == 0 ) )

/*
 *      IS_QNAN(x)
 *
 *      Is true if double x is a quiet NaN.
 *      x must be a simple variable and not an expression.
 */

#define IS_QNAN(x) \
	( ( VALH(x) & 0x7ff80000 ) == 0x7ff80000 )

/*
 *      IS_NAN(x)
 *
 *      Is true if double x is any NaN.
 *      x must be a simple variable and not an expression.
 */

#define IS_NAN(x) \
	( ( ( VALH(x) & 0x7ff00000 ) == 0x7ff00000 ) && \
		( ( VALH(x) & 0x000fffff) | VALL(x) ) )

/*
 *      IS_ZERO(x)
 *
 *      Is true if double x is +0 or -0.
 *      x must be a simple variable and not an expression.
 */

#define IS_ZERO(x) \
	( !( ( VALH(x) & 0x7fffffff ) | VALL(x) ) )

/*
 *	IS_DENORM(x)
 *
 *	Is true if double x is denormal.  I.e. A non-zero IEEE double 
 *	value with a zero exponent.
 *	x must be a simple variable and not an expression.
 */

#define IS_DENORM(x) \
	(  !( VALH(x) & 0xfff00000 ) && \
		( ( VALH(x) & 0x000fffff ) | VALL(x) ) )

#endif	/* _H_FP */
