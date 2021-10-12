static char sccsid[] = "@(#)80	1.2  src/bos/usr/ccs/lib/libc/POWER/cvtloop.c, libccnv, bos411, 9428A410j 6/15/90 17:53:15";
/*
 * COMPONENT_NAME: (LIBCCNV) math library
 *
 * FUNCTIONS: cvtloop
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
                                                                                
#include <math.h>
#include <float.h>
                                                                                
static long twoto52[]  = { 0x43300000, 0x00000000 }; 	/* 2^52 */
                                                                                
/*
 * NAME: cvtloop
 *
 * FUNCTION: Convert two doubles to ASCII string
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */
                                                                                
void
cvtloop(char *p, char *buf, double zhi, double zlo)
{
	double temphi, templo, xhi;
	double hihi, lohi, res, eps, zhiold, zloold;
	double *p2to52;
                                                                                
	union {
		unsigned long i[2];
		double		x ;
	} pt1hi;
	union {
		unsigned long i[2];
		double		x ;
	} pt1lo;
        union {
               unsigned long i[2];
               double          x ;
        } quickconvert;
                                                                                
	pt1hi.i[0] = 0x3fb99999;
	pt1hi.i[1] = 0x99999999;
	pt1lo.i[0] = 0x3c633333;
	pt1lo.i[1] = 0x33333333;
	p2to52 = (double *) twoto52;

	for ( ; p >= buf; p--)
	{
		/* temp = z / 10;  with round nearest */
                                                                                
                temphi = zhi * pt1hi.x;
                res = (zhi - temphi * 10.0) + zlo;
                templo = res * pt1hi.x + (res * pt1lo.x);
		
		/*
		 * The following code works, albeit slowly -- the above is
         	 * supposed to be a faster realization
		 *

                   temphi = zhi / 10.0;
                   res = (zhi - temphi * 10.0) + zlo;
                   templo = res /10.0;
 		 */

 		/*  z = rint(temp);   with round zero */
                __setrnd(1);                                         

		/* save original number */
		zhiold = zhi;
		zloold = zlo;

 		xhi = temphi + templo;		   /* make sure high and  */
 		templo = (temphi - xhi) + templo;  /* low order quotients */
						   /* have the same sign  */
 		temphi = xhi;
                templo < *p2to52;
 		if ( xhi < *p2to52 )	/* truncate to integer */
 		{
 			zhi = temphi + *p2to52;	/* if the high order part is */
 			zhi -=  *p2to52;	/* small enough,  the entire */
 			zlo =  0.0;		/* integer resides in h.o    */
 		}
 		else {	/* for large numbers, the h.o. part is all integer */
 			zhi = temphi;
 			zlo = templo;
 			if ( templo < *p2to52 )	/* only if the l.o. part is */
 			{ 			/* small enough do we have  */
						/* to throw away fraction   */
						/* bits                     */
 				zlo = templo + *p2to52;
 				zlo -= *p2to52 ; /* we know zlo is positive */
 			}
 		}
                                                                                
 		/* temp = temp - z; */
		/* now just compute zold - 10 * (zold/10) to get the next
		 * digit from the right
		 */

		res = (zhiold - 10.0 * zhi) - 10.0 * zlo + zloold;
                quickconvert.x = res + *p2to52;

                __setrnd(0);
                *p = quickconvert.i[1] + '0';
	}
}
