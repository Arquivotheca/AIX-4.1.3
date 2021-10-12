static char sccsid[] = "@(#)91	1.8  src/bos/usr/ccs/lib/libc/drand48.c, libcgen, bos411, 9428A410j 10/20/93 14:27:59";
#ifdef _POWER_PROLOG
/*
 *   COMPONENT_NAME: LIBCGEN
 *
 *   FUNCTIONS: drand48, drand48_r, irand48, krand48, lcong48, lcong48_r,
 *		lrand48, lrand48_r, mrand48, mrand48_r, next, seed48, seed48_r,
 *		srand48, srand48_r
 *
 *   ORIGINS: 3, 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

/*
 * drand48.c,v $ $Revision: 1.4.2.2 $ (OSF) $Date: 91/12/27 20:40:49 $";
 */
/*
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 *
 * drand48.c	1.7  com/lib/c/gen,3.1,8943 10/11/89 13:47:39
 */

#include <stdio.h>
#ifdef _THREAD_SAFE
#include <errno.h>
#include <stdlib.h>
#endif	/* _THREAD_SAFE */

/*
 *	drand48, etc. pseudo-random number generator
 *	This implementation assumes unsigned short integers of at least
 *	16 bits, long integers of at least 32 bits, and ignores
 *	overflows on adding or multiplying two unsigned integers.
 *	Two's-complement representation is assumed in a few places.
 *	Some extra masking is done if unsigneds are exactly 16 bits
 *	or longs are exactly 32 bits, but so what?
 *	An assembly-language implementation would run significantly faster.
 */
#ifndef HAVEFP
#define HAVEFP 1
#endif
#define N	16
#define MASK	((unsigned)(1 << (N - 1)) + (1 << (N - 1)) - 1)
#define LOW(x)	((unsigned)(x) & MASK)
#define HIGH(x)	LOW((x) >> N)
#define MUL(x, y, z)	{ long l = (long)(x) * (long)(y); \
		(z)[0] = LOW(l); (z)[1] = HIGH(l); }
#define CARRY(x, y)	((long)(x) + (long)(y) > MASK)
#define ADDEQU(x, y, z)	(z = CARRY(x, (y)), x = LOW(x + (y)))
#define X0	0x330E
#define X1	0xABCD
#define X2	0x1234
#define A0	0xE66D
#define A1	0xDEEC
#define A2	0x5
#define C	0xB
#define SET3(x, x0, x1, x2)	((x)[0] = (x0), (x)[1] = (x1), (x)[2] = (x2))
#define SETLOW(x, y, n) SET3(x, LOW((y)[n]), LOW((y)[(n)+1]), LOW((y)[(n)+2]))

#define HI_BIT	(1L << (2 * N - 1))


#ifdef _THREAD_SAFE

/*
 * The thread-safe versions of the drand48 routines.
 * The interfaces are similar, but completely re-entrant.
 */
#define SEED(x0, x1, x2) (SET3(dp->x, x0, x1, x2), \
			    SET3(dp->a, A0, A1, A2), dp->c = C)

#define NEST(TYPE, f, F) \
	int f(xsubi, dp, randval) \
	unsigned short xsubi[3]; \
	struct drand48_data *dp; \
	TYPE *randval; \
	{ \
		int i; \
		unsigned temp[3]; \
		if ((xsubi == NULL) || (dp == NULL) || (randval == NULL)) { \
			errno = EFAULT; return (-1); } \
		/*********\
		  init structure if not done\
		**********/\
		if (!dp->init) {\
			SET3(dp->a, A0, A1, A2);\
			dp->c = C;\
			dp->init = TRUE;\
		}\
		for (i = 0; i < 3; i++) { \
			temp[i] = dp->x[i]; dp->x[i] = LOW(xsubi[i]); \
		}  \
		if (F(dp, randval) != 0) \
			return (-1); \
		for (i = 0; i < 3; i++) { \
			xsubi[i] = dp->x[i]; dp->x[i] = temp[i]; \
		} \
		return (0); \
	}


static void
next(struct drand48_data *dp)
{
	unsigned p[2], q[2], r[2], carry0, carry1;

	MUL(dp->a[0], dp->x[0], p);
	ADDEQU(p[0], dp->c, carry0);
	ADDEQU(p[1], carry0, carry1);
	MUL(dp->a[0], dp->x[1], q);
	ADDEQU(p[1], q[0], carry0);
	MUL(dp->a[1], dp->x[0], r);
	dp->x[2] = LOW(carry0 + carry1 + CARRY(p[1], r[0]) + q[1] + r[1] +
		dp->a[0] * dp->x[2] + dp->a[1] * dp->x[1] + dp->a[2] *
		dp->x[0]);
	dp->x[1] = LOW(p[1] + r[0]);
	dp->x[0] = LOW(p[0]);
}


/*
 * NAME:       drand48_r
 */
int
drand48_r(struct drand48_data *dp, double *randval)
{
	static double two16m = 1.0 / (1L << N);

 	if ((dp == NULL) || (randval == NULL)) {
                errno = EFAULT;
                return (-1);            /* illegal value for drand48_r() */
        }

	/*
 	 * If the entries in the drand48_data structure haven't been initialized
 	 * by srand48(), seed48(), or lcong48(), they have to be initialized
	 * with default values.
	 */
	if (!dp->init) {
		SET3(dp->x, X0, X1, X2);
		SET3(dp->a, A0, A1, A2);
		dp->c = C;
		dp->init = TRUE;
	}
	next(dp);
	*randval = two16m *
		   (two16m * (two16m * dp->x[0] + dp->x[1]) + dp->x[2]);
	return (0);
}


/*
 * NAME:	erand48_r
 */
NEST(double, erand48_r, drand48_r)


/*
 * NAME:	lrand48_r
 */
int
lrand48_r(struct drand48_data *dp, long *randval)
{

 	if ((dp == NULL) || (randval == NULL)) {
                errno = EFAULT;
                return (-1);            /* illegal value for lrand48_r() */
        }
	/*
 	 * If the entries in the drand48_data structure haven't been initialized
 	 * by srand48(), seed48(), or lcong48(), they have to be initialized
	 * with default values.
	 */
	if (!dp->init) {
		SET3(dp->x, X0, X1, X2);
		SET3(dp->a, A0, A1, A2);
		dp->c = C;
		dp->init = TRUE;
	}
	next(dp);
	*randval = ((long)dp->x[2] << (N - 1)) + (dp->x[1] >> 1);
	return (0);
}


/*
 * NAME:	mrand48_r
 */
int
mrand48_r(struct drand48_data *dp, long *randval)
{
	long l;

 	if ((dp == NULL) || (randval == NULL)) {
                errno = EFAULT;
                return (-1);            /* illegal value for mrand48_r() */
        }
	/*
 	 * If the entries in the drand48_data structure haven't been initialized
 	 * by srand48(), seed48(), or lcong48(), they have to be initialized
	 * with default values.
	 */
	if (!dp->init) {
		SET3(dp->x, X0, X1, X2);
		SET3(dp->a, A0, A1, A2);
		dp->c = C;
		dp->init = TRUE;
	}
	next(dp);
	*randval = (l = ((long)dp->x[2] << N) + dp->x[1])
		   & HI_BIT ? l | HI_BIT : l;

	return (0);
}


/*
 * NAME:	srand48_r
 */
int
srand48_r(long seedval, struct drand48_data *dp)
{
 	if (dp == NULL) {
                errno = EFAULT;
                return (-1);            /* illegal value for srand48_r() */
        }
	SEED(X0, LOW(seedval), HIGH(seedval));

	/*
	 * Mark the drand48_data structure as initialized.
	 */
	dp->init = TRUE;
	return (0);
}


/*
 * NAME:	seed48_r
 */
int
seed48_r(unsigned short seed16v[], struct drand48_data *dp)
{
 	if ((seed16v == NULL) || (dp == NULL)) {
                errno = EFAULT;
                return (-1);            /* illegal value for seed48_r() */
        }
	SETLOW(dp->lastx, dp->x, 0);
	SEED(LOW(seed16v[0]), LOW(seed16v[1]), LOW(seed16v[2]));

	/*
	 * Mark the drand48_data structure as initialized.
	 */
	dp->init = TRUE;
	return (0);
}


/*
 * NAME:	lcong48_r
 */
int
lcong48_r(unsigned short param[], struct drand48_data *dp)
{
 	if ((param == NULL) || (dp == NULL)) {
                errno = EFAULT;
                return (-1);            /* illegal value for lcong48_r() */
        }
	SETLOW(dp->x, param, 0);
	SETLOW(dp->a, param, 3);
	dp->c = LOW(param[6]);

	/*
	 * Mark the drand48_data structure as initialized.
	 */
	dp->init = TRUE;
	return (0);
}

NEST(long, nrand48_r, lrand48_r)

NEST(long, jrand48_r, mrand48_r)


#else	/* _THREAD_SAFE */


#define REST(v)	for (i = 0; i < 3; i++) { xsubi[i] = x[i]; x[i] = temp[i]; } \
		return (v);

#define SEED(x0, x1, x2) (SET3(x, x0, x1, x2), SET3(a, A0, A1, A2), c = C)

#define NEST(TYPE, f, F) \
	TYPE f(xsubi) \
	unsigned short *xsubi; \
	{ \
		int i; \
		TYPE v; \
		unsigned temp[3]; \
		for (i = 0; i < 3; i++) { \
			temp[i] = x[i]; x[i] = LOW(xsubi[i]); \
		}  \
		v = F(); REST(v); \
	}

static unsigned		x[3] = { X0, X1, X2 }, a[3] = { A0, A1, A2 }, c = C;
static unsigned short	lastx[3];


static void
next(void)
{
	unsigned p[2], q[2], r[2], carry0, carry1;

	MUL(a[0], x[0], p);
	ADDEQU(p[0], c, carry0);
	ADDEQU(p[1], carry0, carry1);
	MUL(a[0], x[1], q);
	ADDEQU(p[1], q[0], carry0);
	MUL(a[1], x[0], r);
	x[2] = LOW(carry0 + carry1 + CARRY(p[1], r[0]) + q[1] + r[1] +
		a[0] * x[2] + a[1] * x[1] + a[2] * x[0]);
	x[1] = LOW(p[1] + r[0]);
	x[0] = LOW(p[0]);
}


/*
 * NAME:	drand48
 *
 * FUNCTION:	compute and return a double precision floating point
 *		random number
 *
 * RETURN VALUE DESCRIPTION:	the random number
 */
#if HAVEFP
double
drand48(void)
{
	static double two16m = 1.0 / (1L << N);

	next();

	return (two16m * (two16m * (two16m * x[0] + x[1]) + x[2]));
}


/*
 * NAME:	erand48
 *
 * FUNCTION:
 *
 * RETURN VALUE DESCRIPTION:
 */
NEST(double, erand48, drand48)


#else

/*
 * Since the following two functions are never compiled they are not made
 * thread safe.
 */

long
irand48(unsigned short m)
/* Treat x[i] as a 48-bit fraction, and multiply it by the 16-bit
 * multiplier m.  Return integer part as result.
 */
{
	unsigned r[4], p[2], carry0 = 0;

	next();
	MUL(m, x[0], &r[0]);
	MUL(m, x[2], &r[2]);
	MUL(m, x[1], p);
	if (CARRY(r[1], p[0]))
		ADDEQU(r[2], 1, carry0);

	return (r[3] + carry0 + CARRY(r[2], p[1]));
}

long
krand48(unsigned short *xsubi, unsigned short m)
/* same as irand48, except user provides storage in xsubi[] */
{
	int i;
	long iv;
	unsigned temp[3];

	for (i = 0; i < 3; i++) {
		temp[i] = x[i];
		x[i] = xsubi[i];
	}
	iv = irand48(m);
	REST(iv);
}
#endif	/* HAVEFP */


/*
 * NAME:	lrand48
 *
 * FUNCTION:	compute and return a random non-negative long integer
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:	the random long integer
 */
long
lrand48(void)
{
	next();
	return (((long)x[2] << (N - 1)) + (x[1] >> 1));
}


/*
 * NAME:	mrand48
 *
 * FUNCTION:	compute and return a random signed long integer
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:	the random signed long integer
 */
long
mrand48(void)
{
	long l;

	next();

	/* sign-extend in case length of a long > 32 bits */
	return ((l = ((long)x[2] << N) + x[1]) & HI_BIT ? l | HI_BIT : l);
}


/*
 * NAME:	srand48
 *
 * FUNCTION:	initialize the random number generator
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:	none
 */
void
srand48(long seedval)
{
	SEED(X0, LOW(seedval), HIGH(seedval));
}


/*
 * NAME:	seed48
 *
 * FUNCTION:	another initialization routine
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 */
unsigned short *
seed48(unsigned short seed16v[])
{
	SETLOW(lastx, x, 0);
	SEED(LOW(seed16v[0]), LOW(seed16v[1]), LOW(seed16v[2]));
	return (lastx);
}


/*
 * NAME:	lcong48
 *
 * FUNCTION:	another initialization routine
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 */
void
lcong48(unsigned short param[])
{
	SETLOW(x, param, 0);
	SETLOW(a, param, 3);
	c = LOW(param[6]);
}


NEST(long, nrand48, lrand48)


NEST(long, jrand48, mrand48)


#ifdef DRIVER
/*
	This should print the sequences of integers in Tables 2
		and 1 of the TM:
	1623, 3442, 1447, 1829, 1305, ...
	657EB7255101, D72A0C966378, 5A743C062A23, ...
 */

main()
{
	int i;

	for (i = 0; i < 80; i++) {
		printf("%4d ", (int)(4096 * drand48()));
		printf("%.4X%.4X%.4X\n", x[2], x[1], x[0]);
	}
}
#endif	/* DRIVER */

#endif /* _THREAD_SAFE */
