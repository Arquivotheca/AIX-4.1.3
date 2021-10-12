#if (!( _FRTINT ))
static char sccsid[] = "@(#)35	1.10  src/bos/usr/ccs/lib/libm/POWER/atan.c, libm, bos411, 9428A410j 6/15/90 17:56:56";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: atan
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <float.h>
#include <math.h>

#ifdef atan
#undef atan
#endif

/*
 * NAME: atan
 *                                                                    
 * FUNCTION: RETURN THE PRINCIPAL VALUE OF THE ARC TANGENT OF X
 *                                                                    
 * NOTES:
 *
 * Input argument needs to be in fixed point and floating point registers
 *
 * RETURNS: arc tangent of x in the range [-PI/2,PI/2]
 *
 */

typedef long int32;

#ifdef _FRTINT
double _atan (double x)
#else
double atan (double x)
#endif
{
	union { 
		int32 i[2];
		double flt;
	} xx;
	union { 
		int32 i[2];
		double flt;
	} a;
	double  d, t1, t2, y, z, x2, x3, x4;
	double  hih, hil, cor, yy1, fmode, altfmode;
	int32   i, j;
	int	flag;
	extern long tantab[];

#include "atan.h"

	xx.flt = x;                             /* move arg to union */
	t =  (struct tdef *) (tantab - (16 *14));
	fmode = __setflm(0.0);	/* Must use RIOS encoding - not ANSI */
	j = xx.i[0] & 0x7fffffff;
	flag = (j < bound);
	flag = (j <= 0x3fb00000);
	flag = (j > 0x3ff00000);

	y = fabs(x);
	x2 = CCLVI + big2;       		/* cause load constants early */
	if (j < bound)    			/* bound = 434d0297 */
	{
		a.flt = CCLVI * y + big2; 	/* Isolated special cases */
		if (j <= 0x3fb00000)
		{                       	/* fast case-simple polynomial*/
			x2 = x * x;
			x4 = x2 * x2;
			t1 = c9 + x4 * c13;
			t2 = c7 + x4 * c11;
			t1 = c5 + x4 * t1;
			t2 = c3 + x4 * t2;
			x3 = x * x2;
			t1 = t2 + x2 * t1;
			__setflm(fmode);
			return(x + x3 * t1);
		}
		else if (j > 0x3ff00000)
		/* computation for large arguments */
		{
			hih= pihh; 		/* for args >= 1.0, 
						 * compute atan(1/x) */
			hil= pihl; /* and then (pi/2) - atan(1/x). */
			y = 1.0 / fabs(x);
			a.flt = CCLVI * y + big2;
			if (j >= 0x40300000)
			{
				yy1 = y * (1.0 - fabs(x) * y);
				x2 = y * y;	/* Special case: */
				x4 = x2 * x2;	/* Argument >= 16.0 in 
						 * magnitude  */
				x3 = x2 * y;
				/*  (New argument <= 0.625 in magnitude */
				t1 = c9 + x4 * c13;
				t2 = c7 + x4 * c11;
				t1 = c5 + x4 * t1;
				t2 = c3 + x4 * t2;
				d =  hih - y;
				t1 = t2 + x2 * t1; 	/* all terms added, 
							 * except linear one. */
				cor = d - hih + y;
				t1 = hil - x3 * t1;
				altfmode = fmode;
				if ((xx.i[0] & 0x80000000) == 0 )
				{
					/* positive */
					__setflm(altfmode);
					return (((t1 - yy1) - cor ) + d);
				}
				else
				{
					/* negative */
					__setflm(altfmode);
					return ((( yy1 - t1) + cor) - d);
				}
			}
			else /*     1 <= abs(x) < 16  */
			{
				yy1 = y * (1.0 - fabs(x) * y);
				i = a.i[1];
				z = (y - t[i].p) + yy1;
				flag = (xx.i[0] >= 0);
				d = hih - t[i].f0;
				t1 = ((((t[i].f5*z+t[i].f4)*z+t[i].f3)*z +
					t[i].f2)*z+t[i].f1);
				__setflm(fmode);
				if (xx.i[0] >= 0 )
				{
					/* positive */
					return (hil - z*t1 + d);
				}
				else 
				{
					/* negative */
					return (z*t1 - hil - d);
				}
			}
		}
		else
		{
			i = a.i[1];
			z = y - t[i].p;
			t1 = (((t[i].f5*z+t[i].f4)*z+t[i].f3)*z+t[i].f2)*z +
				t[i].f1;
			if ((xx.i[0] & 0x80000000) == 0 )
			{ 
				/* positive */
				__setflm(fmode);
				return (t[i].f0 + z*t1);
			}
			else
			{ 
				/* negative */
				__setflm(fmode);
				return( -(t[i].f0 + z*t1));
			}
		}
	}
	__setflm(fmode);
	if ((j > 0x7ff00000) ||
	    ((j == 0x7ff00000) && (xx.i[1]!=0)))	/* atan(NaN) = NaN */
		return (x);

	if (xx.i[0]>0)  	/* large arg: return pi/2 */
		return (pihh);
	else 
		return (- pihh);
}
