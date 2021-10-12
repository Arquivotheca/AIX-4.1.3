/* @(#)87	1.22  src/bos/usr/include/float.h, sysfp, bos411, 9437A411a 9/12/94 09:20:35 */

/*
 * COMPONENT_NAME: (SYSFP) floating point header file
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

#ifndef _H_FLOAT
#define _H_FLOAT

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *  Only a subset of the values in this header are required
 *  by the ANSI standard. The values not required will not be
 *  included if _ANSI_C_SOURCE is defined (for strict conformance).
 *
 *  ANSI required:
 *
 *      FLT_ROUNDS      Macro that returns current rounding mode value
 *      FLT_RADIX       Exponent radix
 *
 *               Values for "float" numbers
 *
 *      FLT_MANT_DIG    Number of bits in the significand
 *      FLT_EPSILON     1ulp when exponent = 0
 *      FLT_DIG         Number of decimal digits of precision
 *      FLT_MIN_EXP     Exponent of smallest NORMALIZED float number
 *      FLT_MIN         Smallest NORMALIZED float number
 *      FLT_MIN_10_EXP  Minimum base 10 exponent of NORMALIZED float
 *      FLT_MAX_EXP     Exponent of largest NORMALIZED float number
 *      FLT_MAX         Largest NORMALIZED float number
 *      FLT_MAX_10_EXP  Largest base 10 exponent of NORMALIZED float
 *
 *               Values for "double" numbers
 *
 *      DBL_MANT_DIG    Number of bits in the significand
 *      DBL_EPSILON     1ulp when exponent = 0
 *      DBL_DIG         Number of decimal digits of precision
 *      DBL_MIN_EXP     Exponent of smallest NORMALIZED double number
 *      DBL_MIN         Smallest NORMALIZED double number
 *      DBL_MIN_10_EXP  Minimum base 10 exponent of NORMALIZED double
 *      DBL_MAX_EXP     Exponent of largest NORMALIZED double number
 *      DBL_MAX         Largest NORMALIZED double number
 *      DBL_MAX_10_EXP  Largest base 10 exponent of NORMALIZED double
 *
 *                Values for "long double" numbers
 *
 *      LDBL_MANT_DIG   Number of bits in the significand
 *      LDBL_EPSILON    1ulp when unbiased exponent = 0
 *      LDBL_DIG        Number of decimal digits of precision
 *      LDBL_MIN_EXP    Exponent of smallest NORMALIZED long double number
 *      LDBL_MIN        Smallest NORMALIZED long double number
 *      LDBL_MIN_10_EX  Minimum base 10 exponent of NORMALIZED long double
 *      LDBL_MAX_EXP    Exponent of largest NORMALIZED long double number
 *      LDBL_MAX        Largest NORMALIZED long double number
 *      LDBL_MAX_10_EXP Largest base 10 exponent of NORMALIZED long double
 *
 *  Not required for ANSI compatibility:
 *
 *      FLT_INFINITY    Float Infinity
 *      DBL_INFINITY    Double Infinity
 *      LDBL_INFINITY   Long Double Infinity
 *      DBL_QNAN        Double QNaN
 *      FLT_QNAN        Float QNaN
 *      DBL_SNAN        Double SNaN
 *      FLT_SNAN        Float SNaN
 *      FP_RND_xx       Floating Point Rounding Mode Constants
 *      FP_xx           Floating Point Class Function Return Values
 *
 */

#ifdef _ANSI_C_SOURCE

#ifndef FLT_MAX
#define FLT_MAX	    3.4028234663852886e+38F   /* max decimal value of a float */
#endif

#ifndef FLT_MIN
#define FLT_MIN	    1.1754943508222875e-38F   /* min decimal value of a float */
#endif

#ifndef FLT_DIG
#define FLT_DIG		6
#endif

#ifndef DBL_MAX
#define DBL_MAX	    1.7976931348623158e+308  /* max decimal value of a double */
#endif

#ifndef DBL_MIN
#define DBL_MIN     2.2250738585072014e-308
#endif

#ifndef DBL_DIG
#define DBL_DIG		15
#endif


/*
 *      General definitions
 */

#define FLT_ROUNDS         ( fp_read_rnd() )
#define FLT_RADIX          2

/*
 *      Float definitions
 */

#define FLT_MANT_DIG       24
#define FLT_EPSILON	   1.1920928955078125e-7F
#define FLT_MIN_EXP        -125
#define FLT_MIN_10_EXP     -37
#define FLT_MAX_EXP        128
#define FLT_MAX_10_EXP     38

/*
 *      Double definitions
 */

#define DBL_MANT_DIG       53
#define DBL_EPSILON	   2.2204460492503131e-16
#define DBL_MIN_EXP        -1021
#define DBL_MIN_10_EXP     -307
#define DBL_MAX_EXP        1024
#define DBL_MAX_10_EXP     308

/*
 *      Long Double definitions
 *
 *      By default long double is the same as double (64 bits).
 *      An optional mode with 128-bit long double is available,
 *      and when this mode is in effect the compiler will define
 *      __LONGDOUBLE128.
 */

#ifdef __LONGDOUBLE128

#define LDBL_MANT_DIG      106
#define LDBL_EPSILON	   0.24651903288156618919116517665087070E-31L
#define LDBL_DIG           31
#define LDBL_MIN_EXP       ((long double) DBL_MIN_EXP)
#define LDBL_MIN           ((long double) DBL_MIN)
#define LDBL_MIN_10_EXP    ((long double) DBL_MIN_10_EXP)
#define LDBL_MAX_EXP       ((long double) DBL_MAX_EXP)
#define LDBL_MAX           0.1797693134862315807937289714053023E+309L
#define LDBL_MAX_10_EXP    ((long double) DBL_MAX_10_EXP)

#else /* #ifdef __LONGDOUBLE128 */

#define LDBL_MANT_DIG      DBL_MANT_DIG
#define LDBL_EPSILON       DBL_EPSILON
#define LDBL_DIG           DBL_DIG
#define LDBL_MIN_EXP       DBL_MIN_EXP
#define LDBL_MIN           DBL_MIN
#define LDBL_MIN_10_EXP    DBL_MIN_10_EXP
#define LDBL_MAX_EXP       DBL_MAX_EXP
#define LDBL_MAX           DBL_MAX
#define LDBL_MAX_10_EXP    DBL_MAX_10_EXP

#endif /* #ifdef __LONGDOUBLE128 */

#endif /* ANSI_C_SOURCE */

/* ******************************************************************
 *
 *      Non-ANSI definitions. The "old" definitions must be strict
 *      constants.
 */

#ifdef _ALL_SOURCE

#ifndef _H_LIMITS
#include <sys/limits.h>
#endif

#define DINFINITY _DBLINF

	extern  unsigned   int SINFINITY;
	extern  unsigned   int _DBLINF[2];
	extern  unsigned   int SQNAN;
	extern  unsigned   int DQNAN[2];
	extern  unsigned   int SSNAN;
	extern  unsigned   int DSNAN[2];

#define FLT_INFINITY	(*((float *) (&SINFINITY)))
#define DBL_INFINITY	(*((double *) (_DBLINF)))
#define LDBL_INFINITY	DBL_INFINITY
#define FLT_QNAN 	(*((float *) (&SQNAN)))
#define DBL_QNAN	(*((double *) (DQNAN)))
#define FLT_SNAN	(*((float *) (&SSNAN)))
#define DBL_SNAN	(*((double *) (DSNAN)))


/*
 *
 *      Values for the IEEE Rounding Modes (ANSI Encoding)
 *
 *      RZ = Round toward zero
 *      RN = Round toward nearest (default)
 *      RP = Round toward plus infinity
 *      RM = Round toward minus infinity
 *
 */
#define FP_RND_RZ       0
#define FP_RND_RN       1
#define FP_RND_RP       2
#define FP_RND_RM       3

typedef	unsigned short fprnd_t;

#ifdef _NO_PROTO

fprnd_t fp_read_rnd();
fprnd_t fp_swap_rnd();

#else

fprnd_t fp_read_rnd(void);
fprnd_t fp_swap_rnd(fprnd_t rnd);

#endif


/*
 *
 *      Floating Point Class Function Return Values
 *
 *      These are the values returned by the class function.
 *      The class function is one of the recommended functions in the
 *      IEEE standard.
 *
 */

#define FP_PLUS_NORM      0
#define FP_MINUS_NORM     1
#define FP_PLUS_ZERO      2
#define FP_MINUS_ZERO     3
#define FP_PLUS_INF       4
#define FP_MINUS_INF      5
#define FP_PLUS_DENORM    6
#define FP_MINUS_DENORM   7
#define FP_SNAN           8
#define FP_QNAN           9

#endif /* ALL_SOURCE */
#endif /* _H_FLOAT */

