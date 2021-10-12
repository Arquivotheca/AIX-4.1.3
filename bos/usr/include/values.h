/* @(#)88	1.12.1.2  src/bos/usr/include/values.h, libm, bos411, 9428A410j 1/12/94 14:00:24 */
/*
 * COMPONENT_NAME: (LIBM) Header file of common values
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

#ifndef _H_VALUES
#define	_H_VALUES

#include <limits.h>

#define BITSPERBYTE	CHAR_BIT
#define BITS(type)	(BITSPERBYTE * sizeof(type))

/* short, regular and long ints with only the high-order bit turned on */
#define HIBITS	((short)-32768)	       /* ((short)(1 << BITS(short) - 1)) */
#define HIBITI	((int)-2147483648) 	       /* (1 << BITS(int) - 1)    */
#define HIBITL	((long)-2147483648)	       /* (1L << BITS(long) - 1)  */

/* largest short, regular and long int */
#define MAXSHORT ((short)~HIBITS)
#define MAXINT   ((int)~HIBITI)
#define MAXLONG  ((long)~HIBITL)

/* various values that describe the binary floating-point representation
 * DMAXEXP 	- the maximum exponent of a double (as returned by frexp())
 * FMAXEXP 	- the maximum exponent of a float  (as returned by frexp())
 * DMINEXP 	- the minimum exponent of a double (as returned by frexp())
 * FMINEXP 	- the minimum exponent of a float  (as returned by frexp())
 * MAXDOUBLE	- the largest double
			((_EXPBASE ** DMAXEXP) * (1 - (_EXPBASE ** -DSIGNIF)))
 * MAXFLOAT	- the largest float
			((_EXPBASE ** FMAXEXP) * (1 - (_EXPBASE ** -FSIGNIF)))
 * MINDOUBLE	- the smallest double (_EXPBASE ** (DMINEXP - 1))
 * MINFLOAT	- the smallest float (_EXPBASE ** (FMINEXP - 1))
 * DSIGNIF	- the number of significant bits in a double
 * FSIGNIF	- the number of significant bits in a float
 * DMAXPOWTWO	- the largest power of two exactly representable as a double
 * FMAXPOWTWO	- the largest power of two exactly representable as a float
 * LN_MAXDOUBLE	- the natural log of the largest double  -- log(MAXDOUBLE)
 * LN_MINDOUBLE	- the natural log of the smallest double -- log(MINDOUBLE)
 * _DEXPLEN	- the number of bits for the exponent of a double (11)
 * _FEXPLEN	- the number of bits for the exponent of a float (8)
 *
 *  These values are no longer defined, however, they are reference in other
 *  defines to show how they were calculated.
 *
 * _EXPBASE	- the exponent base (2)
 * _IEEE	- 1 if IEEE standard representation is used (1)
 * _LENBASE     - the number of bits in the exponent base (1 for binary)
 * _HIDDENBIT	- 1 if high-significance bit of mantissa is implicit
 */
/* these are for the IEEE format machines */
#define MAXDOUBLE     1.7976931348623157e+308

/* MAXFLOAT is also defined in math.h */
#ifndef MAXFLOAT
extern unsigned int _SFPMAX;
#define MAXFLOAT            (*((float *) (&_SFPMAX)))
#endif

#define MINDOUBLE     4.94065645841246544e-324
#define MINFLOAT      ((float)1.40129846432481707e-45)
#define DMINEXP       -1073     /* (-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))    */
#define FMINEXP       -148 	/* (-(FMAXEXP + FSIGNIF - _HIDDENBIT - 3))    */
#define DSIGNIF       53      /* (BITS(double) - _DEXPLEN + _HIDDENBIT - 1)  */
#define FSIGNIF       24      /* (BITS(float)  - _FEXPLEN + _HIDDENBIT - 1)  */
#define DMAXPOWTWO    4.503599627370496E15 /* ((double)(1L << BITS(long) - 2)*/ 
		       			   /* * (1L<<DSIGNIF-BITS(long)+1))  */ 
#define FMAXPOWTWO    8388608	       /* ((float)(1L << FSIGNIF - 1))       */
#define DMAXEXP       1024     	       /* ((1 << _DEXPLEN - 1) - 1 + _IEEE)  */
#define FMAXEXP       128              /* ((1 << _FEXPLEN - 1) - 1 + _IEEE)  */
#define LN_MAXDOUBLE  7.0978271289338397310E2   /*Hex 2^9 * 1.62E42FEFA39EF  */
#define LN_MINDOUBLE  -7.444400719213812100E2   /* (M_LN2 * (DMINEXP - 1))   */

#define _DEXPLEN      11
#define _FEXPLEN      8

#define H_PREC        (6.7108864E7 * M_SQRT2)   /* (DSIGNIF % 2 ? 	     */
						/* (1L << DSIGNIF/2)*M_SQRT2 */
						/* : 1L << DSIGNIF/2)        */

#define X_PLOSS       ((double)(long)(M_PI * H_PREC))
#define X_TLOSS       (M_PI * DMAXPOWTWO)

/* The next values are duplicated in math.h. They have to be    */
/* here too for to keep from having to include math.h.		*/
/* The number of useful digits depends upon the size of the 	*/
/* hardware floating point word.  Several word sizes 		*/
/* and the corresponding digits are:				*/
/*     Word size   Significand bits   Decimal digits            */
/*        32            24                 9			*/
/*        64            53                17			*/
/*        80            64                21			*/
/*       128           106                33			*/
/*       128           113                36			*/

#define M_PI       3.14159265358979323846264338327950288
#define M_SQRT2    1.41421356237309504880168872420969808

#endif	/* _H_VALUES */
