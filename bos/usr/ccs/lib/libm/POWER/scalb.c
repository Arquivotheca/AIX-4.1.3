#if (!( _FRTINT ))
static char sccsid[] = "@(#)49	1.14  src/bos/usr/ccs/lib/libm/POWER/scalb.c, libm, bos411, 9428A410j 1/5/94 14:57:05";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: scalb
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 

#include <math.h>
#include <errno.h>
#include <float.h>
#include <values.h>

static int magic[] =  {0x43300800,0x00000000}; /* 2^52 + 2^44 */
#define TWO_TO_1023   8.9884656743115795386e307
#define TWO_TO_M1022  2.2250738585072015000e-308
#define MAGIC (*(double *) magic)

/*
 * NAME: scalb
 *                                                                    
 * SCALB (x,N)
 * FUNCTION: SCALES THE EXPONENT OF x BY N
 *                                                                    
 * NOTES:
 *       IEEE Recommended Scalb Function                                  
 *                                                                        
 *       y = scalb(x,N)                                                   
 *                                                                        
 *       This function scales the exponent of x by N. In theory it        
 *       performs a multiplication of 2^N without actually doing a        
 *       multiply.                                                        
 *                                                                        
 *       This function conforms to the IEEE P754 standard and is          
 *       a recommended function.                                          
 *                                                                        
 *       NOTE: The framers of the IEEE standard assumed that a multiply   
 *             by 2^N would take longer than scalb(x,N). This is not      
 *             true for RIOS where the multiplies are fast.               
 *             Therefore, this code actually does the multiply when       
 *             appropriate.                                               
 *                                                                        
 *                                                                        
 *                                                                        
 *       Performance estimate = 17 clocks for normalized numbers and      
 *                              a +N that doesn't cause over/underflow.   
 *                            = 17 clocks for normalized numbers and      
 *                              a -N that doesn't cause over/underflow    
 *                                                                        
 *      These performance estimates are DATED -- before all the switching to
 *      get errno set right for overflow/underflow case, as req'd by COSE.
 *                                                                        
 *      CAUTION:                                                          
 *                                                                        
 *      The code below was optimized for the common case where N is       
 *      in the range -1022 <= N <= +1023. Simplifing the nested if's      
 *      or reorganizing them might cause the common case to take          
 *      longer.                                                           
 *                                                                        
 *                                                                        
 *       IEEE Status Bits that might be set are:                          
 *                                                                        
 *          nvsnan       Signaling NaN. X was a NaN                       
 *              ox       Overflow                                         
 *              ux       Underflow                                        
 *              xx       May be set only if over/underflow occurred       
 *                                                                        
 * RETURNS: X * 2 to the N
 *
 */


double
scalb(double x, double N)
{

	/* Define a union to hold the floating point scale factor */

	union {
		double  dbl;
		struct {
			unsigned int hi;
			unsigned int low;
		} xx;
	} y, w;
	double fpscr;
	int in;
	double result;
	
	(N != N);		/* scheduling hint */
	(x != x);
	(__fabs(x) == HUGE_VAL);
	(x == 0.0);
	
	y.xx.low = 0;     /* init the low part of the scale factor */

	/* gotos are used below to make SURE that the fall thru case
	 * is the path where we want to otimize performance
	 */

	if (N != N)		/* take care of second argument NaN */
		goto NAN;
	
	if (x != x)		/* take care of first argument NaN */
		goto NAN;

	if (__fabs(x) == HUGE_VAL) /* take care of infinity */
		goto RETURNX;

	if (__fabs(x) == 0.0)
		goto RETURNX;

	if (N > 0.0) {

		if (N > 1023.0) {  /* The common case falls thru */

		    /* too big: return infinity and set IEEE flags */
		    if (N > 2100.0) {
			errno = ERANGE;
			return(((x *  DBL_MAX) * DBL_MAX) * DBL_MAX);
		    }
		    else {   /* 1023 < N < 2100 */

			while (N > 1023.0) {
			    x *= TWO_TO_1023; /* multiply x by 2^1023  and   */
			    N -= 1023.0;      /* reduce N until N <= 1023    */
			}
		    }
		}
	}
	else {              /* negative N */
		if (N < -1022.0) {  /* The common case falls thru */

		    if (N < -2100.0) {
			/* too small: return zero and set IEEE flags */
			errno = ERANGE;
			return((x * MINDOUBLE) * MINDOUBLE);
		    }
		    else {   /* -2100 < N < -1022 */
			while (N < -1022.0) {
			    x *= TWO_TO_M1022; /* multiply x by 2^-1022 and  */
			    N += 1022.0;       /* increase N until N >= -1022*/
			}
		    }
		}
	}

	/* In-line conversion to integer.  Since we know that 
	 * -2100 <= N <= 2100, we don't have to worry about special cases.
	 */

	w.dbl = N;
	if (N < 0.0)
	    {
	    fpscr = __setrnd(2); /* round to positive inf */
	    w.dbl += MAGIC;
	    in = w.xx.low;
	    }
	else /* positive case */
	    {
	    fpscr = __setrnd(3); /* round to negative inf */
	    w.dbl += MAGIC;
	    in = w.xx.low;
	    }
	(void) __setflm(fpscr);	/* restore user FPSCR */
	
	/*   Scale by 2^N   */
 	y.xx.hi = ((1023 + in) << 20) & 0x7ff00000; 

	result = x * y.dbl;
	/* x == INF  and x == 0 taken care of above; if result is infinity 
	 * or zero we've overflowed or underflowed.
	 */
	if ((__fabs(result) == HUGE_VAL) || (__fabs(result) == 0.0))
		errno = ERANGE;

	return result;

      NAN:
	return x + N;
	
      RETURNX:
	return x;
	
}
