static char sccsid[] = "@(#)92  1.26  src/bos/usr/ccs/lib/libc/POWER/ecvt.c, libccnv, bos41J, 9523A_all 6/2/95 11:49:56";
/*
 * COMPONENT_NAME: LIBCCNV ecvt
 *
 * FUNCTIONS: ecvt, fcvt, cvt, pwr10, ecvt_r, fcvt_r
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <math.h>	/* rint() */
#include <float.h>
#include <stdlib.h>	/* NULL, imul_dbl(), ecvt(), fcvt(), ecvt_r(), fcvt_r() */
#include <errno.h>	/* errno, EINVAL */

#define C_TRUE (int)1
#define C_FALSE (int)0
/* size of the working buffer for cvt-- see ANSI C Sec. 4.9.6.1,
 * "Environmental Limit" -- the mininum number of characters
 * produced is 509.
 *
 * Largest number (1.8e+308) to smallest denormal number (4.9e-324)
 * with 30 decimal digits of precision and trailing null char
 * comes up with 308+324+30+1 = 663 as a buffer to allow all numbers to be
 * printed in %.354f format.  354 = 324+30. BSIZ is max( 509, 663 ) rounded
 * up to next multiple of 10.
 *
 * cvtloop, since it is converting a quad, can do 30 digits of precision.
 */
#define BSIZ 670	/* can hold anything, DBL_MAX to DENORM_MIN, in F format */
#define NMAX 30		/* max digits convert by cvtloop */

#define MIN(A,B) 		((A)>(B) ? (B) : (A))

typedef struct fp { 
	long int i[2];	/* 2 x 32-bit longs == 1 x 64-bit double */
};

void   pwr10( int, double * );
void	mf2x1( double [2], double );
void	mf2x2( double [2], double [2] );

#ifdef _THREAD_SAFE
int __cvt_r( double, int, int *, int *, int, char *, int );
#define RETURN(a,b) return(b);
#else /* ! _THREAD_SAFE */
char   *cvt( double, int, int *, int *, int );
#define RETURN(a,b) return(a);
#endif

#if defined(_ANSI_C_SOURCE) && defined(_LINT)	/* compiler builtins or _ALL_SOURCE */
#ifdef _THREAD_SAFE
int	*cma_errno( void );	/* prototype in errno.h is missing void */
#endif
double	__setflm( double );
void	__setrnd( int );
void	imul_dbl( int, int, int [2] );
double	rint( double );
#endif

static struct fp pospow1[64][2];	/* forward decl */

static long twoto52[]={ 
	0x43300000, 0x0};             /* 2^52     */

#ifndef _THREAD_SAFE

static char __buf[BSIZ];               /* buffer to store results */

/*================================================================
 * NAME: ecvt
 *                                                                    
 * FUNCTION: convert double to character string
 *                                                                    
 * NOTES:
 * Calls cvt to perform the conversion
 *
 * RETURNS: a character string
 */

char *ecvt( double arg, int ndigits, int *decpt, int *sign )
{
	return( cvt(arg, ndigits, decpt, sign, C_TRUE));
}

/*================================================================
 * NAME: fcvt
 *                                                                    
 * FUNCTION: convert double to character string in FORTRAN f format
 *                                                                    
 * NOTES:
 * Calls cvt to perform the conversion
 *
 * RETURNS: a character string in FORTRAN f format
 */

char *fcvt( double arg, int ndigits, int *decpt, int *sign )
{
	return( cvt(arg, ndigits, decpt, sign, C_FALSE));
}

#else /* _THREAD_SAFE */

/*================================================================
 * NAME: ecvt_r
 *                                                                    
 * FUNCTION: reentrant version of ecvt
 *                                                                    
 * NOTES:
 *	Calls __cvt_r to perform the conversion
 *	output is put in char buf[len]
 *
 * RETURNS:
 *	 0 => all is OK.
 *	-1 => something bad.  Also sets errno to EINVAL
 */

int ecvt_r( double arg, int ndigits, int *decpt, int *sign, char *buf, int len )
{
	return( __cvt_r(arg, ndigits, decpt, sign, C_TRUE, buf, len) );
}

/*================================================================
 * NAME: fcvt_r
 *                                                                    
 * FUNCTION: reentrant version of fcvt
 *                                                                    
 * NOTES:
 *	Calls __cvt_r to perform the conversion
 *	output is put in char buf[len]
 *
 * RETURNS:
 *	 0 => all is OK.
 *	-1 => something bad.  Also sets errno to EINVAL
 */

int fcvt_r( double arg, int ndigits, int *decpt, int *sign, char *buf, int len )
{
	return( __cvt_r(arg, ndigits, decpt, sign, C_FALSE, buf, len) );
}

#endif /* _THREAD_SAFE */

/*================================================================
 * NAME: cvt and __cvt_r
 *                                                                    
 * FUNCTION: service routine to convert the double to char string
 *                                                                    
 * NOTES:
 *
 *      char *cvt  (double arg, int nd, int *decpt, int *sign, int eflag)
 *      int __cvt_r(double arg, int nd, int *decpt, int *sign, int eflag, char *buf, int len)
 *
 *          arg   = argument to convert to a string
 *          nd    = number of digits in the result string in E-format
 *		  = location of last digit, relative to radix, produce in F-format
 *          decpt = pointer to an int to store the position of decimal point
 *                  of the result relative to the start of the string.
 *          sign  = pointer to an int to store the an indication of the
 *                  sign of the result ( 0 = +, nonzero = - ).
 *          eflag = input param. TRUE if caller was ecvt. FALSE if the
 *                  caller was fcvt.
 *	    buf	  = where put string; char buf[len].
 *	    len	  = size of output buffer.
 *
 *      char *ecvt(double arg, int ndigits, int *decpt, int *sign)
 *      char *fcvt(double arg, int ndigits, int *decpt, int *sign)
 *
 *          arg     = argument to convert to a string
 *          ndigits = number of digits in E-format; scale factor in F-format
 *          decpt   = pointer to an int to store the position of decimal
 *                    point of the result relative to the start of the
 *                    string. ( A negative return value means the decimal
 *                    point is to the left of the returned digits.)
 *          sign  =   pointer to an int to store the an indication of the
 *                    sign of the result ( 0 = +, nonzero = - ).
 *
 *      ecvt always produces a string of the form: nnnnnn
 *      where the number of n's is determined by ndigits.
 *
 *      fcvt produces a string of the form: mmmmnnnnnn where the decimal
 *      point is between the m's and the n's. The number of n's is
 *      determined by ndigits, but the number of m digits varies with
 *      the magnitude of arg. The decimal point is between the m's and
 *      the n's.  If ndigits is 0, there are no n's, just m's.  If ndigits
 *	is negative, then there are no n's, and the rightmost abs(ndigits)
 *	m's are also suppressed.  In all cases, it is as if the argument
 *	is scaled (multiplied) by 10^digits, rounded to an integer, and
 *	that integer converted and output.  If the number is < 0.1, and
 *	ndigits is > 0, there are no m's, and there may be leading zeros
 *	in the n's.
 *
 *****************************************************************
 *
 *  This code was based on algorithms developed by Jerome Coonen and
 *  documented in a paper entitled "Accurate Yet Economical
 *  Binary - Decimal Conversions" published by the math dept. at
 *  UC Berkeley.
 *
 *************************
 *
 *  The Basic Algorithm (Greatly Simplified) Is:
 *
 *  loop = true
 *  *sign = sign of arg
 *  arg = abs(arg)
 *  handle the special numbers
 *  logx = log10(arg)
 *  while (loop) {
 *      scale = ndigits - logx -1  -- x*10^scale should yield nd digits
 *      y = 10^scale               -- Get from power of ten table.
 *      y = arg*y
 *      z = rint(y)
 *      if (abs(z) > 10^nd) logx++  else loop = false
 *      }
 *  -- We now have the fp integer to convert to a string --
 *  while ( more characters to convert ) {
 *      temp = z/10
 *      z = rint(z)      -- in round toward zero mode --
 *      temp = temp - z
 *      next_char = int(temp*10.0 + '0')
 *      store next_char in result buffer
 *      }
 *
 *  The actual algorithm below uses 2 doubles to represent most of the
 *  variables. This gives much greater accuracy. Also the actual algorithm
 *  has to worry about differences between ecvt and fcvt types of strings
 *
 * ******************
 *
 *      logx = log10(arg)
 *
 * Use Coonen's quicky algorithm to get an integer that is approximately
 * equal to the log10(arg). The algorithm always produces the correct
 * integer or the next smallest integer (never greater).
 *
 *   log10(arg) is approximately equal to: log10(2)*(exp_of_arg + .fffff)
 *   where .ffff.. are the fraction bits of arg (not including the
 *   hidden bit).
 *
 *
 *
 * RETURNS: a character string (cvt) or an int (__cvt_r)
 *
 */

union {
	double	dbl;
	unsigned int i[2];
} fpscr;

/*
 * If 'debug1' is defined debugging information will be
 * printed for use during unit testing.  Be careful -- don't
 * print floating point numbers using the %e %f or %g format --
 * printf calls ecvt and/or fcvt, and you can get infinite
 * recursion.  The following union is used to print the numbers
 * in internal format, which is the best option.
 */

#ifdef debug1
	union {
		double	dbl;
		unsigned int i[2];
	} prnt;
#endif

#ifdef _THREAD_SAFE
int __cvt_r( double arg, int nd, int *decpt, int *sign, int eflag, char *buf, int len)
#else
char *cvt( double arg, int nd, int *decpt, int *sign, int eflag )
#endif /* _THREAD_SAFE */
{

#ifndef _NO_PROTO
	void cvtloop(char *p, char *buf, double zhi, double zlo);
#else
	void cvtloop();
#endif

	static char INFSTR[]=  "INF\0";
	static char SNANSTR[]= "NaNS\0";
	static char QNANSTR[]= "NaNQ\0";
	char	*p;
	const char *pt;		/* points to chars in memory */
	int	i, j;
	int	logx[2];     /* logx[0] contains apx. integer = log10(arg*10^denorm) */
	int	scale;       /* power of 10 we need to mult. by             */
	double	saverm;      /* caller's rounding mode                      */
	int  denorm=0;       /* amount we scaled a denorm or tiny number    */
	int  ndigits = nd;   /* actual number of digits we need to convert  */
	int  loop = C_TRUE;    /* while loop boolean                          */
	double y, x[2], temp[2], z[2], a[2];  /* temps */
	double hihi, res, lohi;    /* temps used in double multiply */
	double *pt1, *p2to52;
	double q;                  /* compiler test, DEBUG */
	int flag;		/* pre-compute a branch */
	double t1, t2;		/* temp. registers for a quad precision add */
#ifndef _THREAD_SAFE
	char	*buf = __buf;	/* where put our output */
	int	len = BSIZ;	/* size of output buffer; char buf[len] */
#endif /* _THREAD_SAFE */


#ifdef debug1
    prnt.dbl = arg;
    printf("<> arg = %08x %08x nd = %d eflag = %d\n",
	   prnt.i[0], prnt.i[1], nd, eflag);
#endif

#ifdef _THREAD_SAFE
        if(NULL == buf) {    /* need a buffer to store digits */
            errno = EINVAL;     /* indicate invalid argument */
            RETURN( buf, -1 );
        }
#endif /* _THREAD_SAFE */

	buf[0] = '\0';			/* in case of null return.    */
	/* Make sure have a place to store output 
	 */
	if(   (NULL == decpt)	/* need a place to store dec. pt. location */
	   || (NULL == sign)	/* need a place to store sign */
#ifdef _THREAD_SAFE
	   || (len <= 1)	/* buffer must hold one digit and trailing null */
#endif /* _THREAD_SAFE */
	      ) { /* ugly but effective */
	    errno = EINVAL;	/* indicate invalid argument */
	    RETURN( buf, -1 );
	}

	/* What happens if we don't have enought space in the buffer to
	 * emit the number of requested digits?  ecvt is easy, since the
	 * number of digits requested is the number emitted, so we do
	 * it here.  fcvt will have to wait to see how many total digits
	 * are to be emitted.
	 */

#ifdef _THREAD_SAFE
	/* For the thread safe case, OSF is specific about what should
	 * happen.
	 */
	if (eflag && (nd >= len))
	    {
	    *decpt = 0;
	    *sign = 0;
	    errno = EINVAL;
	    RETURN (buf, -1);
	    }
#else
	/* for regular case we will emit something (for best compatibility
	 * with previous versions of this routine) but don't allow
	 * buffer to overflow.
	 */
	if (eflag && (nd >= len))
	  nd = len - 1;
#endif /* _THREAD_SAFE */

	/* for ecvt a negative number of digits is meaningless */
	if (eflag && (nd < 0))
	  nd = ndigits = 0;

	/*
	 * Do Initialization and Handle the Special Numbers
	 */

	*decpt = 0;	/* dec. pt. is just before digit string */

	/* save caller's rnd mode and switch to round nearest. */
	saverm = __setflm(0.0);

	*sign = *((unsigned int*)&arg) >>31;         /* return the sign  */
	*((unsigned int*)&arg) &= 0x7fffffff;        /* remove sign      */

	if( eflag ){			/* E-format */
	    if(ndigits > NMAX)		/* have already verified that ndigits > 0 */
		ndigits = NMAX;		/* cvtloop can do at most NMAX significant digits */
	}

#ifdef debug1
	printf ("ndigits = %d\n", ndigits);
#endif

        /* Check for zero */
        if( arg == 0.0 )                /* +/- 0.0 */
            goto zero;                  /* go format zero */

	/* Check for NaN's or Infinities */
	/* Pad inf, nan strings below.   */
	if((*((unsigned int*)&arg)&0x7ff00000)==0x7ff00000)
	{					/* ISSUE: should '*decpt' be set special for
						 * for any of these values? 
						 * ISSUE: does 'nd' matter to see if there
						 * is room for the funny string? */
		if( arg == arg ) {
			pt = INFSTR;
#ifdef _THREAD_SAFE
			if( len < 4 ){		/* make sure have room for string */
			    errno = EINVAL;	/* indicate buffer is too small */
			    __setflm(saverm);
			    RETURN( buf, -1 );	/* error return */
			}
#endif
		}
		else {
			/* Return nan string.    */
			if(*((unsigned int*)&arg) & 0x00080000)
				pt = QNANSTR;
			else
				pt = SNANSTR;
#ifdef _THREAD_SAFE
			if( len < 5 ){		/* make sure have room for string */
			    errno = EINVAL;	/* indicate buffer is too small */
			    __setflm(saverm);
			    RETURN( buf, -1 );	/* error return */
			}
#endif
		}
		p = buf;				/* where start storing */
		for ( ; *pt != '\0'; *p++ = *pt++);	/* copy string */
		*p = '\0';			/* NULL terminate string */
		__setflm(saverm);
		RETURN( buf, 0 );	/* all OK */
	}

	(arg < 1.0e-275);	/* scheduling help */

	a[0]= arg; 
	a[1]=0.0;               /* move arg to 2 doubles */

	/* Check for denormalized or small arg. If found scale 'a' by 10^45 and
	 * set the variable denorm to do the unscaling later.  If this is not done,
	 * then 'scale' can be as large as NMAX - log10(denorm) - 1 or 
	 * 30 - (-323) - 1 or 352, which then causes 10^scale to overflow to INF.
	 * scale must be <= 308 to avoid overflow; implies 30-(-279)-1 = 308 or
	 * numbers as small as 1.0e-279 will not cause problems.  Since the
	 * calculation of log10 is done in ints (hence has error), set the cutoff
	 * point at 1.0e-275.  Now, denorm (4.9e-324) must be scaled to be bigger
	 * than this cutoff point, so scale factor is -279-(-324) or 45.
	 */
	if (arg < 1.0e-275){      /* scale denorm or arg w/ exp < -275 */
		denorm = 46;	  /* table starts are 10^0 */

		mf2x2(a, (double*)&pospow1[denorm][0]);/* scale 'a' by 10^45 */

#ifdef debug1
  		printf("small or denorm -- scaled argument\n");
		prnt.dbl = a[0];
		printf ("a[0] = %08X %08X\n", prnt.i[0], prnt.i[1]);
		prnt.dbl = a[1];
		printf ("a[1] = %08X %08X\n", prnt.i[0], prnt.i[1]);
#endif

	}

	/* 
	 * Unbias the exp of arg and mult by 2 thus creating an integer 
	 * equal to:
	 *          ((exp_of_arg + .fffff..)*2^21)                  
	 */

	j = ( (*((unsigned *)a)) - 0x3ff00000) ;

	/* multiply 'j' by 32 bits of log10(2). Result is in high part of logx.
	 * if j is <0 ,add a little bit to log10(2) so result will be smaller.
	 */

	if (j < 0)
		imul_dbl(j,0x4d104d43,logx);    /* .4d104d42 = log10(2) */
	else
		imul_dbl(j,0x4d104d42,logx);
	logx[0] >>= 20;

	/*
 	 *  while (loop) {
 	 *      scale = ndigits - logx -1  -- x*10^scale should yield nd digits
 	 *      z = 10^scale               -- Get from power of ten table.
 	 *      z = arg*z
 	 *      z = rint(z)
 	 *      if (abs(z) > 10^nd) logx++  else loop = false
 	 *  }
 	 */

	while (loop) {  /* Will loop twice if logx was too small by 1 */

		/* calculate scale and ndigits for either ecvt or fcvt */
		if(eflag) { 
			scale = ndigits - logx[0] - 1 ;  
		}
		else { 
		/* If denorm == 0, ndigits is nd (number digits to left of
		 * radix) plus (logx[0] + 1) (the location of the first
		 * significat digit relative to radix).
		 * If denorm != 0, the location of the first significant
		 * digit is offset by (denorm).
		 */
			ndigits = nd + (logx[0] + 1)  - denorm;
			scale = ndigits - (logx[0] + 1);
			if (ndigits > NMAX)
			  ndigits = NMAX;
			scale = ndigits - (logx[0] + 1);
		}

#ifdef debug1
 		printf("LOOP: scale = %d,nd = %d,ndigits = %d,logx = %08X %08X logx = %d\n",
		      scale, nd, ndigits, logx[0],logx[1], logx[0]);     
#endif

		/* z = arg * 10^scale  where scale is 2 doubles */
		flag = (denorm == 0); /* scheduling help */
		
		pwr10(scale, z);

#ifdef debug1
  		printf("z is 10^scale\n");
		prnt.dbl = z[0];
		printf ("z[0] = %08X %08X\n", prnt.i[0], prnt.i[1]);
		prnt.dbl = z[1];
		printf ("z[1] = %08X %08X\n", prnt.i[0], prnt.i[1]);
#endif

		/* if not denorm, then a[0] contains the initial argument.
		 * If denorm, then we've multiplied the arg by 10^denorm, thus
		 * creating a quad value.  Don't want to pay for the mf2x2
		 * unless a is really a quad.
		 */

		if (denorm == 0)
		  mf2x1(z, a[0]);
		else
		  mf2x2(z, a);

#ifdef debug1
  		printf("z is  arg * 10^scale\n");
		prnt.dbl = z[0];
		printf ("z[0] = %08X %08X\n", prnt.i[0], prnt.i[1]);
		prnt.dbl = z[1];
		printf ("z[1] = %08X %08X\n", prnt.i[0], prnt.i[1]);
#endif
		
		/* get integer part of z, where z is 2 doubles */

		/* If the the user rounding mode was a directed rounding mode
		 * then xor the low bit with the sign which will change RP to
		 * RM and RM to RP if the sign was 1. Note: this code depends
		 * on the rounding mode encodings as defined in the ANSI C
		 * standard 
		 */

		fpscr.dbl = saverm;
                fpscr.i[1] &= 0x3;     /* get rounding mode only.  We don't 
                                          want to get flt. exception enabling bits */
		if (fpscr.i[1]) { 	/* if not round to the nearest */
			if (fpscr.i[1] > 1) 
                             fpscr.i[1] ^= *sign;
	 	        __setrnd(fpscr.i[1]);
		}

		p2to52 = (double *) twoto52;
		/* The following code is an integer part of two doubles   */
		x[0] = z[0] + z[1];            /* t = hi + lo */
		z[1] = (z[0] - x[0] ) + z[1];  /* round for user mode */
		z[0] = x[0] ;                  /* hi = t */
		if ( (*(long*)z &0x7ff00000) < 0x43300000)  /* if exp(z) < 52 */
		{ 
		/* must carefully add the two parts of z.  There are a few places
		 * where z[0] is xxxx.5 and z[1] is small and negative.  Without
		 * a careful addition, it gets rounded up instead of truncated.
		 */
			t1 = z[0] + *p2to52;
			t2 = ((*p2to52 - t1) + z[0]) + z[1];
			z[0] = ((t1 + t2) - *p2to52);
			z[1] = 0.0;
		}
		else 	/* hi part already int; round low part */
			z[1] = rint(z[1]);
		__setrnd(0);          /* go back to round nearest    */

#ifdef debug1
  		printf("IN LOOP: rint with modified user rnd mode\n");
		prnt.dbl = z[0];
		printf ("z[0] = %08X %08X\n", prnt.i[0], prnt.i[1]);
		prnt.dbl = z[1];
		printf ("z[1] = %08X %08X\n", prnt.i[0], prnt.i[1]);
#endif

		/* Check to see if result is < 10^ndigits. If not then our
		 * log10(x) was too small and we need to repeat the loop
		 * with a bigger log10(x)
		 */

		pwr10(ndigits, x);
		/*  Following is a compare of 2 doubles ( x-z ) to zero  */
		temp[0] = x[0] - z[0];
		temp[1] = x[1] - z[1];
		logx[0] ++;
		if ((temp[0] + temp[1]) > 0.0 )  
			loop = C_FALSE;

	}   /* end big while loop */

	*decpt = logx[0] - denorm;     /* return offset to decimal point  */
#ifdef debug1
	printf("at end of loop *decpt is %d\n", *decpt);
#endif

	if ((!eflag) && (z[0] == 0.0) )
	{ 
                goto zero;
	}


	/* Here we have sufficient information to test for overflow
	 * in the fcvt case.
	 */

#ifdef _THREAD_SAFE
	if ((!eflag) && ((nd + (*decpt > 0 ? *decpt : 0)) >= len))
	    {
	    errno = EINVAL;
	    *sign = 0;
	    *decpt = 0;
	    __setflm(saverm);
	    RETURN( buf, -1);
	    }
#endif /* _THREAD_SAFE */

	/*
	 *      Z now contains the integer we want to convert to a string.
	 *      The basic algorithm is:
	 *
	 *         while ( more characters to do ) {
	 *              temp = z/10
	 *              z = rint(temp)   -- in RZ round mode --
	 *              temp = temp - z
	 *              next char = int( temp*10 + '0')
	 *         }
	 */

        if(!eflag) {
           if(*decpt < 0) {
              p = buf;
              /* fill in leading zero's */
              for(j=*decpt; j < 0; j++) {
                 *p = '0';
                 p++;
              }
              p = p + ndigits;
              *p = '\0';
              p--;
              *decpt = 0;  /* leading zero's are properly filled, 
                              so *decpt is never negative   */
           }
           else {
              p = &buf[ndigits];
              *p = '\0';                  /* store null in end of buffer */
              p--;
           }
        }
        else {  /* ecvt */
	   p = &buf[ndigits];
	   *p = '\0';                  /* store null in end of buffer */
           p--;
        }
      

	/*
	 *      call "cvtloop" to generate the result digits from z
	 *
	 *      NOTE: cvtloop expects the rounding mode to be FP_RND_RN
	 */

	__setrnd(0);          /* round nearest */

#ifdef debug1
        printf("Value passed to cvtloop: p = %08x buf = %08x p - buf = %d\n",
	       p, buf, p - buf);
	prnt.dbl = z[0];
	printf ("z[0] = %08X %08X\n", prnt.i[0], prnt.i[1]);
	prnt.dbl = z[1];
	printf ("z[1] = %08X %08X\n", prnt.i[0], prnt.i[1]);
#endif

	cvtloop(p,buf,z[0],z[1]);

	/* If the number of digits requested exceeds the precision of the
	 * machine, pad the remaining digits with '0'
	 */

	if (eflag) {	/* %e format */
	    if (nd > NMAX) {
                j = strlen(buf);
                p = &buf[j];
                j = nd - j;  /* number of trailing zero's to be filled */
                if (j > 0) {
                    for (; j > 0; j--) {
                       *p = '0';
                       p++;
                    }
                    *p = '\0';
                    }
		}
	    }
	else  {			/* fcvt format */
	    /* we have already emitted some digits,
	     * if that's less than requested, pad the remainder
	     * with zero.  Take care not to overflow the
	     * buffer.
	     *
	     * nd is the number of digits requested to the right
	     * of the decimal point, *depct is the number of digits
	     * to the left.  The difference between the sum of these
	     * and ndigits, the actual digits requested, represents
	     * the remaining pad digits to be emitted.
	     */
            i = strlen(buf);
            p = &buf[i];
            j = nd + *decpt - i;  /* number of trailing zero's to be filled */
            if(j >= (len - i)) 
	      j = len - i - 1;
            if (j > 0) {
                for (; j > 0; j--) {
                   *p = '0';
                   p++;
                }
                *p = '\0';
                }
	    }		

	__setflm(saverm);
	RETURN (buf, 0);

        /*
         * The rounded value to print to nd digits is effectively zero.
         */
    zero:

#ifdef debug1
	prnt.dbl = arg;
	printf("AT `zero:' nd = %d ndigits = %d arg = %08x %08x\n",
	       nd, ndigits, prnt.i[0], prnt.i[1]);
#endif

        *decpt = 0;             /* ISSUE: is there a better value? */
        if( nd < 0 ) nd = 0;                                
        p = &buf[nd];           /* since already checked that nd < len,
                                 * no need for MIN(nd, (len - 1)) */

        *p = '\0';              /* set terminating char, dec buf ptr. */
        for( ; p >buf; )
            *--p = '0';         /* back-fill buf with ascii 0's. */

        __setflm(saverm);
        RETURN( buf, 0 );       /* all is OK */

}

static struct fp pospow1[64][2] = {
	0x3FF00000, 0x00000000,   0x00000000, 0x00000000,      /*  10^0    */
	0x40240000, 0x00000000,   0x00000000, 0x00000000,      /*  10^1    */
	0x40590000, 0x00000000,   0x00000000, 0x00000000,      /*  10^2    */
	0x408F4000, 0x00000000,   0x00000000, 0x00000000,      /*  10^3    */
	0x40C38800, 0x00000000,   0x00000000, 0x00000000,      /*  10^4    */
	0x40F86A00, 0x00000000,   0x00000000, 0x00000000,      /*  10^5    */
	0x412E8480, 0x00000000,   0x00000000, 0x00000000,      /*  10^6    */
	0x416312D0, 0x00000000,   0x00000000, 0x00000000,      /*  10^7    */
	0x4197D784, 0x00000000,   0x00000000, 0x00000000,      /*  10^8    */
	0x41CDCD65, 0x00000000,   0x00000000, 0x00000000,      /*  10^9    */
	0x4202A05F, 0x20000000,   0x00000000, 0x00000000,      /*  10^10   */
	0x42374876, 0xe8000000,   0x00000000, 0x00000000,      /*  10^11   */
	0x426D1A94, 0xa2000000,   0x00000000, 0x00000000,      /*  10^12   */
	0x42A2309C, 0xe5400000,   0x00000000, 0x00000000,      /*  10^13   */
	0x42D6BCC4, 0x1e900000,   0x00000000, 0x00000000,      /*  10^14   */
	0x430C6BF5, 0x26340000,   0x00000000, 0x00000000,      /*  10^15   */
	0x4341C379, 0x37e08000,   0x00000000, 0x00000000,      /*  10^16   */
	0x43763457, 0x85d8a000,   0x00000000, 0x00000000,      /*  10^17   */
	0x43ABC16D, 0x674ec800,   0x00000000, 0x00000000,      /*  10^18   */
	0x43E158E4, 0x60913d00,   0x00000000, 0x00000000,      /*  10^19   */
	0x4415AF1D, 0x78b58c40,   0x00000000, 0x00000000,      /*  10^20   */
	0x444B1AE4, 0xd6e2ef50,   0x00000000, 0x00000000,      /*  10^21   */
	0x4480F0CF, 0x064dd592,   0x00000000, 0x00000000,      /*  10^22   */
	0x44B52D02, 0xc7e14af6,   0x41600000, 0x00000000,      /*  10^23   */
	0x44EA7843, 0x79d99db4,   0x41700000, 0x00000000,      /*  10^24   */
	0x45208B2A, 0x2c280290,   0x41D28000, 0x00000000,      /*  10^25   */
	0x4554ADF4, 0xb7320334,   0x42072000, 0x00000000,      /*  10^26   */
	0x4589D971, 0xe4fe8401,   0x423CE800, 0x00000000,      /*  10^27   */
	0x45C027E7, 0x2f1f1281,   0x42584400, 0x00000000,      /*  10^28   */
	0x45F431E0, 0xfae6d721,   0x429F2A80, 0x00000000,      /*  10^29   */
	0x46293E59, 0x39a08ce9,   0x42DB7A90, 0x00000000,      /*  10^30   */
	0x465F8DEF, 0x8808b024,   0x42F4B268, 0x00000000,      /*  10^31   */
	0x4693B8B5, 0xb5056e16,   0x434677C0, 0x80000000,      /*  10^32   */
	0x46C8A6E3, 0x2246c99c,   0x43682B61, 0x40000000,      /*  10^33   */
	0x46FED09B, 0xead87c03,   0x439E3639, 0x90000000,      /*  10^34   */
	0x47334261, 0x72c74d82,   0x43C5C3C7, 0xf4000000,      /*  10^35   */
	0x476812F9, 0xcf7920e2,   0x4416CD2E, 0x7c400000,      /*  10^36   */
	0x479E17B8, 0x4357691b,   0x443900F4, 0x36a00000,      /*  10^37   */
	0x47D2CED3, 0x2a16a1b1,   0x445E8262, 0x88900000,      /*  10^38   */
	0x48078287, 0xf49c4a1d,   0x44A988BE, 0xcaad0000,      /*  10^39   */
	0x483D6329, 0xf1c35ca4,   0x44E7F577, 0x3eac2000,      /*  10^40   */
	0x48725DFA, 0x371a19e6,   0x452EF96A, 0x872b9400,      /*  10^41   */
	0x48A6F578, 0xc4e0a060,   0x4556B7C5, 0x28f67900,      /*  10^42   */
	0x48DCB2D6, 0xf618c878,   0x458C65B6, 0x73341740,      /*  10^43   */
	0x4911EFC6, 0x59cf7d4b,   0x45C1BF92, 0x08008e88,      /*  10^44   */
	0x49466BB7, 0xf0435c9e,   0x45EC5EED, 0x14016454,      /*  10^45   */
	0x497C06A5, 0xec5433c6,   0x45EBB542, 0xc80deb48,      /*  10^46   */
	0x49B18427, 0xb3b4a05b,   0x46691514, 0x9bd08b31,      /*  10^47   */
	0x49E5E531, 0xa0a1c872,   0x46975A59, 0xc2c4adfd,      /*  10^48   */
	0x4A1B5E7E, 0x08ca3a8f,   0x46BA61E0, 0x66ebb2f9,      /*  10^49   */
	0x4A511B0E, 0xc57e6499,   0x47043E96, 0x2029a7ee,      /*  10^50   */
	0x4A8561D2, 0x76ddfdc0,   0x46F4E3BA, 0x83411e91,      /*  10^51   */
	0x4ABABA47, 0x14957d30,   0x472A1CA9, 0x24116636,      /*  10^52   */
	0x4AF0B46C, 0x6cdd6e3e,   0x476051E9, 0xb68adfe2,      /*  10^53   */
	0x4B24E187, 0x8814c9cd,   0x47D14666, 0x4242d97e,      /*  10^54   */
	0x4B5A19E9, 0x6a19fc40,   0x480D97FF, 0xd2d38fdd,      /*  10^55   */
	0x4B905031, 0xe2503da8,   0x48427EFF, 0xe3c439ea,      /*  10^56   */
	0x4BC4643E, 0x5ae44d12,   0x48771EBF, 0xdcb54865,      /*  10^57   */
	0x4BF97D4D, 0xf19d6057,   0x4899CCDF, 0xa7c534fc,      /*  10^58   */
	0x4C2FDCA1, 0x6e04b86d,   0x48C04017, 0x91b6823b,      /*  10^59   */
	0x4C63E9E4, 0xe4c2f344,   0x4902280E, 0xbb121165,      /*  10^60   */
	0x4C98E45E, 0x1df3b015,   0x4936B212, 0x69d695be,      /*  10^61   */
	0x4CCF1D75, 0xa5709c1a,   0x49762F4B, 0x82261d97,      /*  10^62   */
	0x4D037269, 0x87666190,   0x49B5DD8F, 0x3157d27e       /*  10^63   */
};

static struct fp pospow2[7][2] = {
	0x3FF00000, 0x00000000,   0x00000000, 0x00000000,      /*  10^0    */
	0x4D384F03, 0xe93ff9f4,   0x49EB54F2, 0xfdadc71e,      /*  10^64   */
	0x5A827748, 0xf9301d31,   0x57337F19, 0xbccdb0db,      /*  10^128  */
	0x67CC0E1E, 0xf1a724ea,   0x6475AA16, 0xef894fd2,      /*  10^192  */
	0x75154FDD, 0x7f73bf3b,   0x71CA3776, 0xee406e64,      /*  10^256  */
	0x7ff00000, 0x00000000,   0x7ff00000, 0x00000000,      /*  10^320 == INF */
	0x7ff00000, 0x00000000,   0x7ff00000, 0x00000000,      /*  10^384 == INF */
};

static struct fp negpow1[64][2] = {
	0x3FF00000, 0x00000000,    0x00000000, 0x00000000,   /* 10^-0   */
	0x3FB99999, 0x99999999,    0x3C633333, 0x33333333,   /* 10^-1   */
	0x3F847AE1, 0x47ae147a,    0x3C3C28F5, 0xc28f5c29,   /* 10^-2   */
	0x3F50624D, 0xd2f1a9fb,    0x3C0CED91, 0x6872b021,   /* 10^-3   */
	0x3F1A36E2, 0xeb1c432c,    0x3BC4AF4F, 0x0d844d01,   /* 10^-4   */
	0x3EE4F8B5, 0x88e368f0,    0x3B908C3F, 0x3e0370ce,   /* 10^-5   */
	0x3EB0C6F7, 0xa0b5ed8d,    0x3B4B5A63, 0xf9a49c2c,   /* 10^-6   */
	0x3E7AD7F2, 0x9abcaf48,    0x3B15E1E9, 0x9483b023,   /* 10^-7   */
	0x3E45798E, 0xe2308c39,    0x3AFBF3F7, 0x0834acdb,   /* 10^-8   */
	0x3E112E0B, 0xe826d694,    0x3AC65CC5, 0xa02a23e2,   /* 10^-9   */
	0x3DDB7CDF, 0xd9d7bdba,    0x3A86FAD5, 0xcd10396a,   /* 10^-10  */
	0x3DA5FD7F, 0xe1796495,    0x3A47F7BC, 0x7b4d28aa,   /* 10^-11  */
	0x3D719799, 0x812dea11,    0x39F97F27, 0xf0f6e886,   /* 10^-12  */
	0x3D3C25C2, 0x68497681,    0x39E84CA1, 0x9697c81b,   /* 10^-13  */
	0x3D06849B, 0x86a12b9b,    0x394EA709, 0x09833de7,   /* 10^-14  */
	0x3CD203AF, 0x9ee75615,    0x3983643E, 0x74dc0530,   /* 10^-15  */
	0x3C9CD2B2, 0x97d889bc,    0x3925B4C2, 0xebe68799,   /* 10^-16  */
	0x3C670EF5, 0x4646d496,    0x39112426, 0xfbfae7eb,   /* 10^-17  */
	0x3C32725D, 0xd1d243ab,    0x38E41CEB, 0xfcc8b989,   /* 10^-18  */
	0x3BFD83C9, 0x4fb6d2ac,    0x388A52B3, 0x1e9e3d07,   /* 10^-19  */
	0x3BC79CA1, 0x0c924223,    0x38675447, 0xa5d8e536,   /* 10^-20  */
	0x3B92E3B4, 0x0a0e9b4f,    0x383F769F, 0xb7e0b75e,   /* 10^-21  */
	0x3B5E3920, 0x10175ee5,    0x3802C54C, 0x931a2c4b,   /* 10^-22  */
	0x3B282DB3, 0x4012b251,    0x37C13BAD, 0xb829e079,   /* 10^-23  */
	0x3AF357C2, 0x99a88ea7,    0x379A9624, 0x9354b394,   /* 10^-24  */
	0x3ABEF2D0, 0xf5da7dd8,    0x376544EA, 0x0f76f610,   /* 10^-25  */
	0x3A88C240, 0xc4aecb13,    0x37376A54, 0xd92bf80d,   /* 10^-26  */
	0x3A53CE9A, 0x36f23c0f,    0x370921DD, 0x7a89933d,   /* 10^-27  */
	0x3A1FB0F6, 0xbe506019,    0x36B06C5E, 0x54eb70c4,   /* 10^-28  */
	0x39E95A5E, 0xfea6b347,    0x3689F04B, 0x7722c09d,   /* 10^-29  */
	0x39B4484B, 0xfeebc29f,    0x3660C684, 0x960de6a5,   /* 10^-30  */
	0x398039D6, 0x6589687f,    0x3633D203, 0xab3e521e,   /* 10^-31  */
	0x3949F623, 0xd5a8a732,    0x35F2E99F, 0x7863b696,   /* 10^-32  */
	0x3914C4E9, 0x77ba1f5b,    0x35C587B2, 0xc6b62bab,   /* 10^-33  */
	0x38E09D87, 0x92fb4c49,    0x3585A5EA, 0xd789df78,   /* 10^-34  */
	0x38AA95A5, 0xb7f87a0e,    0x355E1E55, 0x793b192d,   /* 10^-35  */
	0x38754484, 0x932d2e72,    0x351696EF, 0x285e8eaf,   /* 10^-36  */
	0x3841039D, 0x428a8b8e,    0x34F5D5F9, 0x435905df,   /* 10^-37  */
	0x380B38FB, 0x9daa78e4,    0x34A2ACB7, 0x3de9ac65,   /* 10^-38  */
	0x37D5C72F, 0xb1552d83,    0x347BBD5F, 0x64baf050,   /* 10^-39  */
	0x37A16C26, 0x2777579c,    0x34463119, 0x1d6259da,   /* 10^-40  */
	0x376BE03D, 0x0bf225c6,    0x341E8DAD, 0xb11b7b15,   /* 10^-41  */
	0x37364CFD, 0xa3281e38,    0x33E87157, 0xc0e2c8dd,   /* 10^-42  */
	0x3701D731, 0x4f534b60,    0x33B38DDF, 0xcd823a4b,   /* 10^-43  */
	0x36CC8B82, 0x18854567,    0x33682C65, 0xc4d3edbc,   /* 10^-44  */
	0x3696D601, 0xad376ab9,    0x331A27AC, 0x0f72f8c0,   /* 10^-45  */
	0x366244CE, 0x242c5560,    0x331C372A, 0xce584c13,   /* 10^-46  */
	0x362D3AE3, 0x6d13bbce,    0x32BAFAAB, 0x8f01e6e1,   /* 10^-47  */
	0x35F7624F, 0x8a762fd8,    0x32859556, 0x0c018581,   /* 10^-48  */
	0x35C2B50C, 0x6ec4f313,    0x32656EEF, 0x38009bcd,   /* 10^-49  */
	0x358DEE7A, 0x4ad4b81e,    0x323DF258, 0xf99a163e,   /* 10^-50  */
	0x3557F1FB, 0x6f10934b,    0x320E5B7A, 0x614811cb,   /* 10^-51  */
	0x352327FC, 0x58da0f6f,    0x31DEAF95, 0x1aa00e3c,   /* 10^-52  */
	0x34EEA660, 0x8e29b24c,    0x31977F54, 0xf7667d2d,   /* 10^-53  */
	0x34B8851A, 0x0b548ea3,    0x316932AA, 0x5f8530f1,   /* 10^-54  */
	0x34839DAE, 0x6f76d883,    0x30EEAAA3, 0x26eb4b43,   /* 10^-55  */
	0x344F62B0, 0xb257c0d1,    0x30F4BBBB, 0x5b8bc3c3,   /* 10^-56  */
	0x34191BC0, 0x8eac9a41,    0x30B45F92, 0x2c12d2d2,   /* 10^-57  */
	0x33E41633, 0xa556e1cd,    0x309B596D, 0xab3ababa,   /* 10^-58  */
	0x33B011C2, 0xeaabe7d7,    0x306C478A, 0xef622efc,   /* 10^-59  */
	0x3379B604, 0xaaaca626,    0x300B6379, 0x2f412cb0,   /* 10^-60  */
	0x3344919D, 0x5556eb51,    0x2FF8AD7E, 0xa30d08f0,   /* 10^-61  */
	0x3310747D, 0xdddf22a7,    0x2FCA2465, 0x4f3da0c0,   /* 10^-62  */
	0x32DA53FC, 0x9631d10c,    0x2F803A3B, 0xb1fc3467    /* 10^-63  */
};

static struct fp negpow2[7][2] = {
	0x3FF00000, 0x00000000,    0x00000000, 0x00000000,   /* 10^-0    */
	0x32A50FFD, 0x44f4a73d,    0x2F3A53F2, 0x398d747b,   /* 10^-64   */
	0x255BBA08, 0xcf8c979c,    0x220282B1, 0xf2cfdb41,   /* 10^-128  */
	0x18123FF0, 0x6eea8479,    0x14C019ED, 0x8c1a8d19,   /* 10^-192  */
	0x0AC80628, 0x64ac6f43,    0x07539FA9, 0x11155ff0,   /* 10^-256  */
	0x00000000, 0x000007e8,    0x00000000, 0x00000000,   /* 10^-320  */
	0x00000000, 0x00000000,    0x00000000, 0x00000000,   /* 10^-384 == zero  */
};

/*================================================================
 * NAME: pwr10
 *                                                                    
 * FUNCTION: Calculate power of ten using 2 doubles to accumulate result
 *                                                                    
 * NOTES:
 *
 * Calculate power of ten using 2 doubles to accumulate result.
 *      k = power of ten desired
 *      zz = pointer to 2 doubles where result will be stored
 *
 * If k is larger than 308, returns +INF
 * If k is smaller than -324, returns +0.0
 *
 * RETURNS: 2 doubles at the location of zz
 *
 */

void pwr10( int k, double *zz )
{
	int i;
	double  *p1, *p2;
	if (k<0) { 
		k = -k;
		p1 = (double *)negpow1;  
		p2 = (double *)negpow2;
	}
	else { 
		p1 = (double *)pospow1;  
		p2 = (double *)pospow2;
	}
	i = (k & 0x3f) << 1;		/* Get 10^k (lo 6 bits)  */
	zz[0] = p1[i]; 
	zz[1] = p1[i+1];
	if (i = (k >> 6)) {		/* hi 3 bits */
	    if( i >= 6 ){		/* prevent out of array bounds */
		zz[0] = p2[12];		/* INF or 0.0 */
		zz[1] = p2[13];
	    }else{
		mf2x2(zz,&p2[i*2]);	/* zz *= 10^k (hi 3 bits) */
	    }
	}

	return ;
}

/*================================================================
 * NAME: __ld
 *                                                                    
 * FUNCTION: compute the position of the first significant decimal
 *           digit relative to the decimal radix.
 *           
 * INPUT:  double precision floating point number.
 *
 * OUTPUT: position of the first significant decimal digit relative
 *         to the radix.  This is expressed as an integer offset
 *         from the radix; positive is to the left, negative to
 *         the right.
 *
 *         For example:
 *
 *         NUMBER              OUTPUT
 *         10.0                  2
 *          1.0                  1
 *          0.1                  0
 *          0.01                 -1
 *
 *        For IEEE funny numbers (NaN, INF) 0 (zero) is returned.
 *
 * LIMITATIONS:
 *        Result is not correct for denormal number.  The result
 *        for denormal numbers is correct in sign and magnitude
 *        (i.e. about -300) but is not accurate.   Since for
 *        f-formatting we won't attempt to print more than about
 *        sixty digits after the decimal, is does not affect the
 *        use of this function, and save a compare operation.
 *
 * NOTES: The obvious (trivial) algorithm for this would is
 *
 *           x = fabs(x)
 *           dwork = log10(x)
 *           switch to round to minus infinity
 *           iwork = dtoi(dwork)
 *           restore old round mode
 *           result = 1 + iwork
 *         
 *        However, log10 and dtoi are not available.
 *
 * ALGORITHM:
 *
 * first, check for NaN, INF; if so ==> return zero
 *
 * second, if arg is negative take absolute value
 *
 * then compute answer as follows:
 *
 *      log base 10 (x) = log base 2 (x) * log base 2 (10)
 *
 *      but log base 2 (x) is the biased exponent of x
 *      and log base 2 (10) is a constant.
 *
 * So:  trial_result = (int) (exponent_of_x * constant)
 *
 *      After truncation to integer this will yield either
 *      the correct answer or the correct answer less 1.
 *
 *      To check, use power of ten tables to calculate
 *      10 ** (trial_result + 1)
 *
 *      If this is larger than original argument, then
 *      trial_result is correct, otherwise trial_result
 *      is low by 1.
 *
 *  This routine is for IEEE machines only.
 */

int __ld(double arg)
  {
 
  int	j;
  int	logx[2];	/* logx[0] contains apx. integer = log10(arg)  */
  double saverm;	/* caller's rounding mode */
  double trial[2];	/* power of ten */
  long i;		/* index into power of ten table */
  double  *p1, *p2;	/* pointer to powers of ten tables */
  int k;
  
  /*
   * Do Initialization and Handle the Special Numbers
   */
  
  /* Check for NaN's or Infinities */
  if((*((unsigned int*)&arg)&0x7ff00000)==0x7ff00000)
      {
      return 0;
      }
  
  *((unsigned int*)&arg) &= 0x7fffffff;	  /* absolute value      */

  /* make exact zero a special case */
  if (arg == 0.0)
    return 0;
  
  /* save caller's rnd mode and switch to round nearest. */
  saverm = __setflm(0.0);
  
  /*
   * Unbias the exp of arg and mult by 2 thus creating an integer 
   * equal to:
   *          ((exp_of_arg + .fffff..)*2^21)                  
   */
  
  j = ( (*((unsigned *)&arg)) - 0x3ff00000) ;

  /* multiply by 32 bits of log10(2). Result is in high part of logx.
   * if j is <0 ,add a little bit to log10(2) so result will be smaller.
   */
  
  if (j < 0)
    imul_dbl(j,0x4d104d43,logx);    /* .4d104d42 = log10(2) */
  else
    imul_dbl(j,0x4d104d42,logx);
  logx[0] >>= 20;
  
  /* at this point you've got the value correct about 95% of the
   * time.  However, this value will be less that the correct
   * value by 1 about 5% of the time.  The way to determine
   * if the value is correct is to perform the following
   * computations:
   *
   *   let k be current value.  if
   *          10**(k+1) <= arg then correct value is k+1
   *                           else correct value is k
   *
   * of course, we need to return k (once correct k is
   * determined), since number of digits to left of decimal
   * is equal to 1 + (int) log10(x).
   *
   */

  logx[0]++;			/* look at 10**(k+1) */

  k = logx[0];
  if (k < 0)
      {
      k = -k;
      p1 = (double *) negpow1;
      p2 = (double *) negpow2;
      }
  else
      {
      p1 = (double *)pospow1;
      p2 = (double *)pospow2;
      }
  
  i = (k & 0x3f) << 1;		/* index into table  */
  trial[0] = p1[i];		/* first f.p. number */
  trial[1] = p1[i+1];		/* second f.p. number */
  if ( i = (k >> 6)){		/* if abs(log10(num)) > 63 */
      if( i >= 6 ){		/* prevent out of array bounds */
	  trial[0] = p2[12];	/* INF or 0.0 */
	  trial[1] = p2[13];
      }else{
	  mf2x2(trial,&p2[i*2]);/* need multiply by 10**(64*n) */
      }
  } 	
  __setflm(saverm);		/* restore rounding mode */

  if (trial[0] <= arg)		/* is 10**(k+1) < arg? */
    return ++logx[0];		/* YES ==> result was 1 low */
  else
    return logx[0];		/* NO ==> result was correct */
  }

