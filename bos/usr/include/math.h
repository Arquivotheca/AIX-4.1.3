/* @(#)30	1.28.1.13  src/bos/usr/include/math.h, libm, bos411, 9428A410j 5/20/94 11:33:42 */

/*
 * COMPONENT_NAME: (LIBM) math header file
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

#ifndef _H_MATH
#define _H_MATH

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *      The ANSI standard requires that certain values be in math.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present.
 *
 *      This header includes all the ANSI required entries. In addition
 *      other entries for the AIX system are included.
 *
 */

/*
 *      ANSI required entries in math.h
 *
 */
#ifdef _ANSI_C_SOURCE

extern  unsigned _DBLINF[2];

#define HUGE_VAL (*((double *)(_DBLINF)))


#ifdef _NO_PROTO

extern  double acos();
extern  double asin();
extern  double atan();
extern  double atan2();
extern  double ceil();
extern  double cos();
extern  double cosh();
extern  double exp();
extern  double fabs();
extern  double floor();
extern  double fmod();
extern  double frexp();
extern  double ldexp();
extern  double log();
extern  double log10();
extern  double modf();
extern  double pow();
extern  double sin();
extern  double sinh();
extern  double sqrt();
extern  double tan();
extern  double tanh();

#ifdef __LONGDOUBLE128
long double acosl();
long double asinl();
long double atanl();
long double atan2l();
long double cosl();
long double coshl();
long double sinl();
long double sinhl();
long double tanl();
long double tanhl();
long double expl();
long double frexpl();
long double ldexpl();
long double logl();
long double log10l();
long double modfl();
long double powl();
long double sqrtl();
long double ceill();
long double fabsl();
long double floorl();
long double fmodl();
#endif /* #ifdef __LONGDOUBLE128 */

#else  /*_NO_PROTO */			/* Use ANSI C required prototyping */

extern  double acos(double);
extern  double asin(double);
extern  double atan(double);
extern  double atan2(double,double);
extern  double ceil(double);
extern  double cos(double);
extern  double cosh(double);
extern  double exp(double);
extern  double fabs(double);
extern  double floor(double);
extern  double fmod(double, double);
extern  double frexp(double, int *);
extern  double ldexp(double, int);
extern  double log(double);
extern  double log10(double);
extern  double modf(double, double *);
extern  double pow(double, double);
extern  double sin(double);
extern  double sinh(double);
extern  double sqrt(double);
extern  double tan(double);
extern  double tanh(double);

#ifdef __LONGDOUBLE128
/*
 * The following interfaces are available only for the
 * non-default 128-bit long double mode.  By default long
 * double is the same as double (64-bit) and should use the
 * the double precision versions of these routines.
 */
long double acosl(long double);
long double asinl(long double);
long double atanl(long double);
long double atan2l(long double , long double);
long double cosl(long double);
long double coshl(long double);
long double sinl(long double);
long double sinhl(long double);
long double tanl(long double);
long double tanhl(long double);
long double expl(long double);
long double frexpl(long double ,   /* value */
		   int *);         /* exponent */
long double ldexpl(long double ,   /* value */
		   int);           /* exponent */
long double logl(long double);
long double log10l(long double);
long double modfl(long double ,    /* value */
		  long double *);  /* integer part */
long double powl(long double , long double);
long double sqrtl(long double);
long double ceill(long double);
long double fabsl(long double);
long double floorl(long double);
long double fmodl(long double , long double);
#endif /* #ifdef __LONGDOUBLE128 */

#endif /*_NO_PROTO */

/*
 *   The following macro definitions cause the XLC compiler to inline
 *   these functions whenever possible.  __MATH__ is defined by the compiler.
 */
 
#ifdef __MATH__
#define acos(__x)         __acos(__x)
#define asin(__x)         __asin(__x)
#define atan(__x)         __atan(__x)
#define atan2(__x,__y)      __atan2(__x,__y)
#define cos(__x)          __cos(__x)
#define exp(__x)          __exp(__x)
#define fabs(__x)         __fabs(__x)
#define log(__x)          __log(__x)
#define log10(__x)        __log10(__x)
#define sin(__x)          __sin(__x)
#define sqrt(__x)         __sqrt(__x)
#define tan(__x)          __tan(__x)
#endif

#endif /*_ANSI_C_SOURCE */

/*
 *
 * The following function prototypes define functions available in the
 * AIX system but not required by the ANSI standard. They will not be
 * included in _ANSI_C_SOURCE is defined (strict ANSI conformance).
 *
 */

#ifdef _XOPEN_SOURCE

extern int signgam;

/*
 *      Useful mathmatical constants:
 *
 * M_E          -- e
 * M_LOG2E      -- log2(e)
 * M_LOG10E     -- log10(e)
 * M_LN2        -- ln(2)
 * M_PI         -- pi
 * M_PI_2       -- pi/2
 * M_PI_4       -- pi/4
 * M_1_PI       -- 1/pi
 * M_2_PI       -- 2/pi
 * M_2_SQRTPI   -- 2/(sqrt(pi))
 * M_SQRT2      -- sqrt(2)
 * M_SQRT1_2    -- 1/sqrt(2)
 *
 * These constants are provided to more significant digits
 * than is necessary for a 64-bit double precision number; they 
 * may be used for other purposes where the extra precision
 * is necessary or useful.
 */

#define M_E         2.71828182845904523536028747135266250
#define M_LOG2E     1.44269504088896340735992468100189214
#define M_LOG10E    0.434294481903251827651128918916605082
#define M_LN2       0.693147180559945309417232121458176568
#define M_LN10      2.30258509299404568401799145468436421
#define M_PI        3.14159265358979323846264338327950288
#define M_PI_2      1.57079632679489661923132169163975144
#define M_PI_4      0.785398163397448309615660845819875721
#define M_1_PI      0.318309886183790671537767526745028724
#define M_2_PI      0.636619772367581343075535053490057448
#define M_2_SQRTPI  1.12837916709551257389615890312154517
#define M_SQRT2     1.41421356237309504880168872420969808
#define M_SQRT1_2   0.707106781186547524400844362104849039

/* MAXFLOAT is also defined in values.h */
#ifndef MAXFLOAT
extern unsigned int _SFPMAX;
#define MAXFLOAT            (*((float *) (&_SFPMAX)))
#endif

#ifdef _NO_PROTO
extern     double   erf();
extern     double   erfc();
extern     int      isnan();
extern     double   hypot();
extern     double   j0();
extern     double   j1();
extern     double   jn();
extern     double   gamma();
extern     double   lgamma();
extern     double   y0();
extern     double   y1();
extern     double   yn();

#ifdef __LONGDOUBLE128
long double erfl();
long double erfcl();
long double lgammal();
#endif /* #ifdef __LONGDOUBLE128 */

#else
extern     double   erf(double);
extern     double   erfc(double);
extern     int      isnan(double);
extern     double   hypot(double,double);
extern     double   j0(double);
extern     double   j1(double);
extern     double   jn(int, double);
extern     double   gamma(double);
extern     double   lgamma(double);
extern     double   y0(double);
extern     double   y1(double);
extern     double   yn(int, double);

#ifdef __LONGDOUBLE128
long double erfl(long double);
long double erfcl(long double);
long double lgammal(long double);
#endif /* #ifdef __LONGDOUBLE128 */

#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#define DINFINITY _DBLINF

#ifndef	_BSD
#ifndef _H_STDLIB
#include <stdlib.h>
#endif
#else	/* _BSD */
/*
 *	Any changes to the below declaration must be verified with
 *	changes to the same function declaration in <stdlib.h>
 */
#ifndef	_NO_PROTO
extern double   atof(const char *);
#else	/* _NO_PROTO */
extern double   atof();
#endif	/* _NO_PROTO */
#endif	/* _BSD */

#ifdef _NO_PROTO

extern     double   acosh();
extern     double   asinh();
extern     double   atanh();
extern     double   cabs();
extern     double   cbrt();
extern     double   copysign ();
extern     double   drem();
extern     double   exp__E();
extern     double   expm1();
extern     double   log1p();
extern     double   log__L();
extern     double   logb();
extern     double   nearest();
extern     double   nextafter();
extern     double   remainder();
extern     double   rint();
extern     double   rsqrt();
extern     double   scalb();
extern     double   trunc();
extern     int      class();
extern     int      finite();
extern     int      ilogb();
extern     int      itrunc();
extern     int      unordered();
extern     unsigned  uitrunc();
#ifdef _POWER
extern     int      dtoi();
#endif

#else /* _NO_PROTO */			/* Use ANSI C required prototyping */

     struct dbl_hypot {
         double x, y;
     };

extern     double   acosh(double);
extern     double   asinh(double);
extern     double   atanh(double);
extern     double   cabs(struct dbl_hypot);
extern     double   cbrt(double);
extern     double   copysign (double, double);
extern     double   drem(double, double);
extern     double   exp__E(double, double);
extern     double   expm1(double);
extern     double   log1p(double);
extern     double   log__L(double);
extern     double   nearest(double);
extern     double   remainder(double, double);
extern     double   rint(double);
extern     double   rsqrt(double);
extern     double   scalb(double, double);
extern     double   trunc(double);
extern     int      itrunc(double);
extern     int      unordered(double, double);
extern     unsigned  uitrunc(double);
#ifdef _POWER
extern     int      dtoi(double);
     /* The POWER wants arguments in both GPR's and FPR's
      * By not specifying a prototype of double, the compiler
      * will put the argument in both types of registers.
      */
extern     double   logb();
extern     int      ilogb();
extern     double   nextafter();
extern     int      class();
extern     int      finite();
#else
extern     double   logb(double);
extern     int      ilogb(double);
extern     double   nextafter(double, double);
extern     int      class(double);
extern     int      finite(double);
#endif


#endif /* _NO_PROTO */

struct exception {
	int type;
	char *name;
	double arg1;
	double arg2;
	double retval;
};

#define 	DOMAIN		01
#define		SING		02
#define		OVERFLOW	03
#define		UNDERFLOW	04
#define		TLOSS		05
#define		PLOSS		06


/*
 *      Useful mathmatical constants:
 *
 * HUGE         - +infinity
 * M_2PI        - 2*pi
 *
 */
#define HUGE       HUGE_VAL
#define M_2PI      6.2831853071795862320E0  /*Hex  2^ 2 * 1.921FB54442D18 */

/* This is the nearest number to the cube root of MAXDOUBLE that   */
/*      doesn't cause the cube of it to overflow.                  */
/* In double precision hex this constant is: 554428a2 f98d728a     */
#define CUBRTHUGE      5.6438030941223618e102
#define INV_CUBRTHUGE  1.7718548704178434e-103

#endif /* ALL_SOURCE */

/*
 * 64-bit integer support, known as long long int and unsigned long long int
 */
#if (defined(_LONG_LONG) && defined(_ALL_SOURCE))
#ifdef _NO_PROTO

extern long long int __multi64( );
extern long long int __divi64( );
extern unsigned long long int __divu64( );
extern long long int __maxi64( );
extern long long int __mini64( );
extern long long int __f64toi64rz( );
extern unsigned long long int __f64tou64rz( );

#ifdef __LONGDOUBLE128
extern long long int __f128toi64rz( );
extern unsigned long long int __f128tou64rz( );
#endif /* __LONGDOUBLE128 */

#else /* ifdef _NO_PROTO */

extern long long int __multi64( long long int, long long int );
extern long long int __divi64( long long int, long long int );
extern unsigned long long int __divu64( unsigned long long int, 
					unsigned long long int );
extern long long int __maxi64( long long int, long long int );
extern long long int __mini64( long long int, long long int );
extern long long int __f64toi64rz( double );
extern unsigned long long int __f64tou64rz( double );

#ifdef __LONGDOUBLE128
extern long long int __f128toi64rz( long double );
extern unsigned long long int __f128tou64rz( long double );
#endif /* __LONGDOUBLE128 */

#endif /* _NO_PROTO */
#endif /* if defined(_LONG_LONG) && defined(_ALL_SOURCE) */

/*
 *   __XLC121__ is automatically defined by the XLC 1.2.1 compiler so that
 *   the compiler can inline the following function when possible.
 */

 
#if (defined (__MATH__) &&  defined (__XLC121__) && defined (_ALL_SOURCE) )
#define copysign(x,y)     __copysign(x,y)
#endif

#endif /*_H_MATH */
