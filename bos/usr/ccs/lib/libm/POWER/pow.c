#if (!( _FRTINT ))
static char sccsid[] = "@(#)86  1.24  src/bos/usr/ccs/lib/libm/POWER/pow.c, libm, bos411, 9428A410j 1/24/94 13:39:15";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: pow, expinner2,
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <math.h>
#include <float.h>
#include <errno.h>
#include <values.h>
 
typedef long int32;
extern int32 __itab[];
extern int32 __tab[];

/* constants for expinner2 */
static double rbig2= 6.7553994410557440000e+15 +1023.0; /* 0x43380000, 0x000003ff */
static double rbig= 6.7553994410557440000e+15; /* 0x43380000, 0x00000000 */
static double rilog2=1.4426950408889633000e+00; /* 0x3ff71547,0x652b82fe */
static double ilog2h=6.9314718055994530000e-01; /* 0x3fe62e42,0xfefa39ef */
static double ilog2l=2.3190468138462996000e-17; /* 0x3c7abc9e,0x3b39803f */
static double cm1= 8.3333336309523691e-03; /* 0x3f811111,0x1b4af38e */
static double c0= 4.1666668402777808000e-02; /* 0x3fa55555,0x643f1505 */
static double c1= 1.6666666666666655000e-01; /* 0x3fc55555,0x55555551 */
static double c2= 4.9999999999999955000e-01; /* 0x3fdfffff,0xfffffff8 */

static double cc4 = 0.5000000000000000000;
static double cc3 = 0.16666666666663809522820;
static double cc2 = 0.04166666666666111110939;
static double cc1 = 0.008333338095239329810170;
static double cc0 = 0.001388889583333492938381;

static double xmaxexp=7.0978271289338397000e+02; /* 0x40862e42,0xfefa39ef */
static double xmin2exp= -7.4513321910194122000e+02; /*0xc0874910,0xd52d3052*/
static double denorm=2.9387358770557188000e-39; /* 0x37f00000,0x00000000 */
static double rdenorm=3.402823669209384635e+38; /* 0x47f00000,0x00000000 */
static double half = 0.5;

static struct A { 
	double t;
	double expt;
}*a; /*(struct A*)(__itab + 2832) init pntr to middle of array */

/* constants for loginner2 */
static int32 log2l[] = { 0x3c7abc9e, 0x3b39803f};
static int32 log2h[] = { 0x3fe62e42, 0xfefa39ef};
static double log_c6= -0.1666681511199968257150;
static double log_c5 = 0.2000014541571685319141;
static double log_c4 = -0.2499999999949632195759;
static double log_c3 = 0.3333333333296328456505;
static double log_c2 = -0.5000000000000000042725;

static double d8 =-0.1250122079043555957999; /* BFC0006668466517 */
static double d7 = 0.1428690115662276259345; /* 3FC249882224F72F */
static double d6 =-0.1666666662009609743117; /* BFC5555554554F15 */
static double d5 = 0.1999999996377879912068; /* 3FC9999998D278CB */
static double d4 =-0.2500000000000056849516; /* BFD0000000000066 */
static double d3 = 0.3333333333333360968212; /* 3FD5555555555587 */
static double d2 = -0.5000000000000000000000; /* bfe0000000000000 */
static int32 big[] ={ 0x47f00000, 0x00000000};
static int32 inf[] ={ 0x7ff00000, 0x00000000};
static double SMALL = 2.2250738585072015025e-308;
/* T: x(i), 1/x(i), ln(x(i))     */
/*      (0:6*(3*64)+5) int32     */
/*  T: x(i), 1/x(i), ln(x(i))    */


static struct tdef {
	double table;
	double recip;
	union { 
		double flt;
		int32 i;
	} fcn;
} *t;

/* constants for pow */
static double unnorm = 4503601774854144.000;   /* 2**52+2**31 */
 



extern int32 __tab[];

static double loginner2(double , double *);
static double expinner2 (double , double );
 
/*
 * NAME: pow
 *
 * POW (X, Y)
 * FUNCTION: COMPUTES X TO THE Y POWER
 *
 * NOTES:
 *
 * RETURNS: x to the y power
 *
 * If the correct value overflows, than HUGE_VAL is returned and errno
 * is set to ERANGE.
 * If x is negative and y is not an integer, than QNaN is returned
 * and errno is set to EDOM.
 * If x is 0 and y is less than 0, than HUGE_VAL is returned .
 *
 */
 
double
#ifdef _FRTINT
_pow(double aa, double bb)
#else
pow(double aa, double bb)
#endif
{
	union {
		double f;
		int i[2];        
	} a;  /* working copy of aa */
	union {
		double f;
		long i;     
	} b;  /* working copy of bb */
        /* below are some working variable for loginner2 and expinner2 */
	double x, tail, lgbase, logarg, arg, arglow, z;
        double result;   /* final result */
        double r;  /* flag on if input bb is even(0), odd(1), 
                      or something else */
        double signa, signb;  /* sign of input argument aa and bb */
	double p, q, fmode ;
	long flag ;
        double xflag;
	union {
		double flt;
		long i[2];   /* fnh, fnl */
	} fn;  /* exponent of a IEEE double of input argument aa */
        double ph,pl,xh[2],xl[2],zh,zl; /* used if int power is implemented */
        int i,iy,iyl,j,itmp,itmp2;
	union {
		double x;
		int i[2]; 
	} u,y;   /* some temperaty working variables */
        /* adj is a constant used to extract exponent from a IEEE double */
        double adj = 4.3934705024835902176e158; /* 0x60e0000000000000 */
        double zero = 0.0;
#ifdef _SVID
	struct exception exc;
#endif

        (bb == 2.0);
	
	/* the following compare will actually compute and have 
	 * ready all of the following cases:
	 *    aa > 0.0
	 *    aa == 0.0
	 *    aa < 0.0
	 *    aa <> aa (NaN ; doesn't compare)
	 */
        (aa > 0.0);
	a.f = aa;

        /* extracting n in aa=x*2^n, where 1.0 <= |x| < 2.0 
	 * logically this part should be in mainpath: below, 
	 * but it can be scheduled here to overlap the compares
	 * above and thus will be ready when we reach mainpath:.
	 */
        fn.i[0] = 0x00000000;
        fn.i[1] = a.i[0] & 0x7ff00000;
        fmode = __setflm(0.0);   /* set round-to-nearest */
                                 /* must use RIOS encoding - not ANSI */
        fn.flt = fn.flt * adj;
        fn.flt = fn.flt * adj - 1023.0;  /* number to multiply by log 2. */
        /* end of extracting n */ 

        /* is aa a denorm double? */
        if ((a.i[0] & 0x7ff00000) == 0) {
           if( aa != 0.0) {
                a.f = a.f * 4503599627370496.0;	/* 2^52 magic number */
                fn.i[0] = 0x00000000;
                fn.i[1] = a.i[0] & 0x7ff00000;
                fn.flt = fn.flt * adj;
                fn.flt = fn.flt * adj - 1075.0;  
           }
        }
        /* pow(x,2.0) is implemented for performance as a special case.
         * However, this code is not designed to get the fastest return 
         * of pow(x,2.0).  Instead, it is designed to have pow(x,2.0)
         * as fast as possible without hurting the performance for
         * the general cases.
	 */
        if(bb == 2.0){
               result = aa*aa;
               (result == HUGE);
               (result == zero);
               __setflm(fmode); /* restore rounding mode */

#ifndef _SVID
               if ((result == HUGE) && (fabs(aa) != HUGE))	
		   /* pow(+INF,2.0) == pow(-INF,2.0) == +INF with no errno */
		   errno = ERANGE; 
#else
               if (result == HUGE)
		   {
		   exc.name = "pow";
		   exc.arg1 = aa;
		   exc.arg2 = bb;
		   exc.type = OVERFLOW;
		   exc.retval = HUGE;
		   if (!matherr(&exc))
		       {
		       errno = ERANGE;
		       }
		   return (exc.retval);
		   }
#endif
	       
               else if((result == zero) && (aa != zero)) 
		   {
#ifndef _SVID
		   errno = ERANGE; 
#else
		   exc.name = "pow";
		   exc.arg1 = aa;
		   exc.arg2 = bb;
		   exc.type = OVERFLOW;
		   exc.retval = HUGE;
		   if (!matherr(&exc))
		       {
		       write(2,"pow: DOMAIN error\n",18);
		       errno = ERANGE;
		       }
		   return (exc.retval);
#endif
		   }
               return(result);
        }
	b.f = bb;

	/* if aa is greater than zero we go to mainpath no matter
	 * what the value of bb is.  (We want to get to mainpath
	 * as quickly as is possible).  We trust mainpath to 
	 * "do the right thing" if bb is an IEEE funny number.
	 * Also note we've already paid for the compare above.
	 */
        if (aa > 0.0) {
               signa = 1.0;
               goto mainpath;
        }

	/* if aa is less than zero you have to look at bb.  
	 * if it's not an integer it's a bad case; return NaN in
	 * libm or return 0 and muck with matherror if libmsaa.
	 * If it's an integer, figure out if it's odd or even
	 * to get the sign right.
	 */
        else if (aa < 0.0) {
        u.x = 0.5 * bb + unnorm;
        u.x = u.x - unnorm;
        r = bb - 2.0 * u.x; /* r = 0 if bb is even,
                             * |r| = 1 if bb is odd,
                             * 0 < |r| < 1 if bb is not an integer,
                             * r = NaN if |bb| is NaN or INF 
			     */
               flag = ( ((r*r-1.0)*r) == 0.0);
               flag = ( r == 0.0);
               flag = ( (r*r-1.0) == 0.0);

               a.f = fabs(a.f);
               if(r == 0.0) {
                       signa = 1.0;
                       goto mainpath;
               }
               else if( (r*r -1.0) == 0.0) {
                       signa = -1.0;
                       goto mainpath;
               }
               else if(bb != bb) {
                       return(bb+bb);	/* quiet the NaN */
               }
               else if(r != r) {  /* |bb| must be INF */
                   __setflm(fmode); /* restore rounding mode */
		   if (aa == -1.0) {
                        return(zero/zero); /* (-1) ** +-INF = NaN */
                   }
		   else if (aa < -1.0) {
			if (bb > 0) return(bb); /* (|x| > 1) ** +INF = +INF */
			else return(0.0);       /* (|x| > 1) ** -INF = +0 */
                   }
		   else { /* must be 0 > aa > -1.0 */
			if (bb > 0) return(0.0); /* (|x| < 1) ** +INF = +0 */
			else return (-bb);       /* (|x| < 1) ** -INF = +INF */
		   }
                }
               else if(r > 1.0) { /* r=0x4340000000000000 
                                   * when bb=0x46a0000000000000 
				   */
                       signa = 1.0;
                       goto mainpath;
               }
               else {  /* neg ** non_integer = NaN */
                        __setflm(fmode); /* restore rounding mode */
#ifndef _SVID
                        errno = EDOM;
                        return(zero/zero);    /* Generate NaN */
#else
			exc.name = "pow";
			exc.arg1 = aa;
			exc.arg2 = bb;
                        exc.type = DOMAIN;
                        exc.retval = 0.0;
                        if (!matherr(&exc))
                        {
                                write(2,"pow: DOMAIN error\n",18);
                                errno = EDOM;
                        }
                        return (exc.retval);
#endif
               }

        }

	/* when aa is zero there are lots of special cases depending
	 * on the value of bb, as well as the sign of aa.  See Kahan's 
	 * list, in the NCEG draft, for the list.
	 */

        else if(aa == 0.0) {
              u.x = 0.5 * bb + unnorm;
              u.x = u.x - unnorm;
              r = bb - 2.0 * u.x; /* r = 0 if bb is even,
                                  |r| = 1 if bb is odd,
                                   0 < |r| < 1 if bb is not an integer,
                                   r = NaN if |bb| is NaN or INF  */
               __setflm(fmode); /* restore rounding mode */
               if(bb == 0.0){
#ifdef _SVID
			exc.name = "pow";
			exc.arg1 = aa;
			exc.arg2 = bb;
                        exc.type = DOMAIN;
                        exc.retval = 0.0;
                        if (!matherr(&exc))
                        {
                                write(2,"pow: DOMAIN error\n",18);
                                errno = EDOM;
                        }
                        return (exc.retval);   /* 0**0 */
#else
                        return(1.0);   /* 0**0 */
#endif
               }
               else if(bb > 0.0) {
                        if(r == 0.0) return(zero); /* +-0 ** (pos even) = 0  */
                        else if((r*r - 1.0) == 0.0) 
                                 return(aa); /* +-0 ** (pos odd) = +-0  */
                        else if(r > 1.0) { /* r=0x4340000000000000
                                              when bb=0x46a0000000000000 */
                                 return(zero); /* 0**0x46a0000000000000 */
                        }
                        else { /* bb is a pos noninteger or +INF */
                                 u.x = aa;
                                 if(u.i[0] == 0)  /* +0**(pos noninteger or INF) */
                                       return (0.0);
                                 else  { /* (-0)**(pos noninteger) or 
                                            (-0)**(+INF)             */
#ifndef _SVID 
				       return(zero);
#else
				       exc.name = "pow";
				       exc.arg1 = aa;
				       exc.arg2 = bb;
                                       exc.type = DOMAIN;
                                       exc.retval = 0.0;
                                       if (!matherr(&exc))
                                       {
                                             write(2,"pow: DOMAIN error\n",18);
                                             errno = EDOM;
                                       }
                                       return (exc.retval);
#endif
                                 }
                         }
               }
               else if(bb < 0.0) {
                        if(r == 0.0){
                             return(1.0 / zero);
                        }
                        else if((r*r - 1.0) == 0.0) {
#ifndef _SVID
			     return(1.0/aa);    /*  +-0 ** (neg odd) = +-0  */
#else
			     exc.name = "pow";
			     exc.arg1 = aa;
			     exc.arg2 = bb;
			     exc.type = DOMAIN;
			     exc.retval = 0.0;
			     if (!matherr(&exc))
				 {
				 write(2,"pow: DOMAIN error\n",18);
				 errno = EDOM;
				 }
			     return (exc.retval);
#endif
                        }
                        else if(r > 1.0) { /* r=0x4340000000000000
                                      when bb=0x46a0000000000000
                                      Large integer is considered as even */
			     return (1.0 / zero);
                        }
                        else { /* bb is a neg noninteger or -INF */
                                 u.x = aa;
                                 if(u.i[0] == 0) { /* +0**(neg noninteger or -INF) */
				    return (1.0 / zero);
                                 }
                                 else  { /* (-0)**(neg noninteger or -INF) */
#ifndef _SVID
				       return(1.0/zero);
#else
				       exc.name = "pow";
				       exc.arg1 = aa;
				       exc.arg2 = bb;
                                       exc.type = DOMAIN;
                                       exc.retval = 0.0;
                                       if (!matherr(&exc))
                                       {
                                             write(2,"pow: DOMAIN error\n",18);
                                             errno = EDOM;
                                       }
                                       return (exc.retval);
#endif
                                 }
                        }
               }
               else return(bb + bb);  /* 0.0**NaN = NaN */
        }

	/* last big case:  aa is NaN 
	 */

        else {  
               __setflm(fmode); /* restore rounding mode */
               if(bb == 0.0) {
                        return(1.0); /* per Kahan ANYTHING to 0.0 is 1.0 */
               }
               else return(aa + aa ); /* quiet the NaN if signalling */
        }
mainpath:
        /*
         * for 1. (aa>0) or 
         *     2. (aa < 0 and integer bb) 
         */

	/* answer = a ^ b
	 * answer = exp(b * ln(a))
	 * answer = exp(b * ln(2^n * z))
	 *        note:  1.0 <= z < 2.0
	 * answer = exp(b * ((n * ln(2)) + ln(z)))
	 *
	 * b == second input arg
	 * n == computed above, in fn.flt
	 * ln(2) == constant, in quad precision log2h, log2l
	 * ln(z) == obtained from loginner2, as a quad,
	 *          high part to lgbase, low part to tail
	 *
	 * and then put back together, in quad precision
	 */
        (a.f == HUGE);		/* check for special cases */
	(fabs(bb) == HUGE);
        y.i[1] = a.i[1];
        y.i[0] = (a.i[0] & 0x000fffff) | 0x3ff00000;
	x = y.x;              /* force 1.0 <= x < 2.0 */
 
	p = fn.flt * *(double*)log2h;		/* high order product */
        if(a.f == HUGE) goto a_infinity;	/* go do special case */
	if(fabs(bb) == HUGE) goto bb_infinity;	/* and other special case */
	lgbase = loginner2(x, &tail);

	q = fn.flt * *(double*)log2h - p;	/* residual of product */
	logarg = p + lgbase;
	/* careful!  p, logarg and lgbase are all possibly large, but
	 * the expression p - logarg + lgbase is of similar magnitude
	 * as (fn.flt * *(double)log2l + tail +q
	 */
	arglow = p - logarg + lgbase + (fn.flt * *(double*)log2l + tail + q);
	z = b.f * arglow;
	arg = logarg * b.f + z;
	arglow = logarg * b.f - arg + z;
        result = expinner2(arg, arglow);
	__setflm(fmode);               /* restore original rounding mode */

#ifdef _SVID
	if (result == HUGE)
	    {
	    exc.name = "pow";
	    exc.arg1 = aa;
	    exc.arg2 = bb;
	    exc.type = OVERFLOW;
	    exc.retval = signa * result;
	    if (!matherr(&exc))
		{
		errno = ERANGE;
		}
	    return (exc.retval);
	    }

	if ((result == 0.0) && (aa != 0.0))
	    {
	    exc.name = "pow";
	    exc.arg1 = aa;
	    exc.arg2 = bb;
	    exc.type = UNDERFLOW;
	    exc.retval = signa * result;
	    if (!matherr(&exc))
		{
		errno = ERANGE;
		}
	    return (exc.retval);
	    }
#endif

        return(signa*result);          /* normal return */

bb_infinity:	/* for |bb| = INF */
        {
	    __setflm(fmode);               /* restore original rounding mode */
#ifndef _SVID
	    if( bb > 0.0 ){	/* pow( aa, +INF ) */
		if(fabs(aa) > 1.0)
		    return(HUGE);
		else if(fabs(aa) < 1.0)
		    return(zero);
		else
		    return(0.0*HUGE);	/* pow( +/- 1.0, +INF) is NaN */
	    }else{		/* pow( aa, -INF ) */
		if(fabs(aa) > 1.0)
		    return(zero);
		else if(fabs(aa) < 1.0)
		    return(HUGE);
		else
		    return(0.0*HUGE);	/* pow( +/- 1.0, -INF) is NaN */
	    }
#else	/* SVID */
	    if( ((bb>0.0)&&(fabs(aa)>1.0)) || ((bb<0.0)&&(fabs(aa)<1.0)) ){
	      exc.name = "pow";
	      exc.arg1 = aa;
	      exc.arg2 = bb;
              exc.type = OVERFLOW;
              exc.retval = HUGE;
              if (!matherr(&exc))
              {
                   errno = ERANGE;
              }
              return (exc.retval);
	    }else if( ((bb>0.0)&&(fabs(aa)<1.0)) || ((bb<0.0)&&(fabs(aa)>1.0)) ){
	      exc.name = "pow";
	      exc.arg1 = aa;
	      exc.arg2 = bb;
              exc.type = UNDERFLOW;
              exc.retval = zero;
              if (!matherr(&exc))
              {
                   errno = ERANGE;
              }
              return (exc.retval);
	    }else{ 		/* pow( +/- 1.0, +/- INF ) is NaN */
	      exc.name = "pow";
	      exc.arg1 = aa;
	      exc.arg2 = bb;
              exc.type = DOMAIN;
              exc.retval = 0.0 * HUGE;	/* NaN */
              if (!matherr(&exc))
              {
                   errno = EDOM;
              }
              return (exc.retval);
	    }
#endif
	}

a_infinity:
        /* for |aa| = INF */
	__setflm(fmode);               /* restore original rounding mode */
        if ( bb == 0.0 ) return ( 1.0 );
        else if (bb < 0.0){
	
#ifndef _SVID
              return(signa * zero);
#else
	      exc.name = "pow";
	      exc.arg1 = aa;
	      exc.arg2 = bb;
              exc.type = UNDERFLOW;
              exc.retval = zero;
              if (!matherr(&exc))
              {
                   errno = ERANGE;
              }
              return (exc.retval);
#endif
        }
	
        else if (bb > 0.0){
#ifndef _SVID
              return(signa * HUGE);
#else
	      exc.name = "pow";
	      exc.arg1 = aa;
	      exc.arg2 = bb;
              exc.type = OVERFLOW;
              exc.retval = HUGE;
              if (!matherr(&exc))
              {
                   errno = ERANGE;
              }
              return (exc.retval);
#endif
        }
        else {
             return(bb + bb); /* bb = NaN */
        }
}

/*
 * NAME: expinner2
 *
 * FUNCTION:  Compute a double precision exponent from a 
 *            quad precision argument.
 * 
 * NOTES:  local to pow()
 *
 * RETURNS: exp of the QUAD precision arguement (xx + eps)
 */


static double expinner2 (double xx, double eps)
{
	union { 
		int32 i[2];
		double flt;
	} x;
	union  { 
		int32 i[2];
		double  flt;
	} scale;

	union  { 
		int32 i[2];
		double flt;
	} uexp ;
	union  { 
		int32 i[2];
		double flt;
	} z;
	register long in;
#ifdef _SVID
	struct exception exc;
#endif

	double y, yl, diff, arg, res, p, result, power, fake;
	double sx,uexp2,argsq, addend, fmode; /* , rbig, rilog2; */
	int flag;

	x.flt = xx;
	flag = fabs(x.flt) < 512.0;
	fake = rbig + rilog2;
	scale.flt = 0.0; 
	fake = ilog2h + ilog2l + c0;  
	a = (struct A *) __itab + 177;
        (x.flt * rilog2 + rbig2);
	if ( fabs(x.flt) < 512.0)
	{
		uexp.flt = x.flt * rilog2 + rbig2;
		uexp2 = uexp.flt - rbig2;
		/* uexp.flt = uexp.flt + 1023.0;  */
		y = x.flt - ilog2h * uexp2;
		z.flt = 512.0 * y +  rbig;
		in = (long)z.i[1] ;
		diff = y - a[in].t;		/* reduced arg */
		arg = diff - (ilog2l * uexp2);	        /* fully reduced arg */
		argsq = arg * arg;
		res = diff - arg -  (ilog2l * uexp2) + eps;	/* residual of fully 
						 * reduced arg */
		p = ((((cc0*arg+cc1)*arg+cc2)*arg+cc3)*arg+cc4)*arg*arg + arg;
/*
		p = ( ( cm1*argsq+c1)*arg+(c0*argsq+c2) ) * argsq+arg; 
*/
		/* p = (((cm1*arg+c0)*arg+c1)*arg+c2)*arg+arg; */
		scale.i[0] = uexp.i[1] << 20;
	        result = res + res * arg;		
		sx = scale.flt * a[in].expt;
		addend = sx*p;
		result = addend + sx * result;
		result = result + sx;
exit:
		return (result);
	}
	flag = (x.flt==DBL_INFINITY);
	flag = (x.flt == -DBL_INFINITY);
	flag = (x.flt > xmaxexp);


	if (x.flt == DBL_INFINITY)
	{ 
		errno = ERANGE;           /* Is exp(inf) an overflow? */
		return (DBL_INFINITY);    /* Set errno for overflow.  */
	}
	flag =  (x.flt < xmin2exp);
	if (x.flt == -DBL_INFINITY) {
	        errno = ERANGE;           /* Is exp(-inf) an underflow? */
		return (0.0);
	}
	flag = (x.flt >= 0.0);
        if (x.flt > xmaxexp)		/* Overflow to inf, set 
						 * errno=ERANGE */
	{ 
		errno = ERANGE;
		return (DBL_INFINITY);
	}
	else if (x.flt >= 0)
	{   
		uexp.flt = x.flt * rilog2 + rbig;
		scale.i[0] = (uexp.i[1] + 1023-1) << 20;
		power = 2.0;
ex_range:
		uexp.flt -=  rbig;
		y = x.flt -  ilog2h * uexp.flt;
		yl =  ilog2l * uexp.flt;
		z.flt = 512.0 * y + rbig;
		in = z.i[1] ;
		diff = y - a[in].t;		/* reduced arg */
		arg = diff - yl;		/* fully reduced arg */
		argsq = arg * arg;
		res = diff - arg - yl + eps;		/* residual of fully 
						 * reduced arg */
		p = ((cm1*argsq+ c1) * arg +(c0*argsq+c2))*argsq+arg;
		addend = p + (res + res * p);
		result =  ((a[in].expt+a[in].expt * addend)* scale.flt) 
			* power;
		return (result);
	}
	else if ( x.flt > xmin2exp)
	{  
		uexp.flt = x.flt * rilog2 + rbig;
		scale.i[0] = (uexp.i[1] + 1023+128) << 20;
		power = denorm;
		goto ex_range;
	}
	else if ( x.flt <= xmin2exp)
	{
                errno = ERANGE;
		return(0.0);
	}
        else if(x.flt != x.flt) return(x.flt + 1.0);
}




/*
 * NAME: loginner2
 *                                                                    
 * FUNCTION: COMPUTEST NATURAL LOG OF X
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This routine does not handle exceptional cases.
 * It assumes that the machine is in round-to-nearest mode.
 *                                                                   
 * NOTES:
 *
 * Computes ln(x) and returns the high order bits of the
 * result.  The low order bits of the result are returned in the
 * parameter "tail". 
 *
 * RETURNS: ln(x)
 *
 */

static double loginner2(double tmpx, double *tail)
{
	double unnorm = 4503601774854144.000;   /* 2**53+2**31 */
	int32 adjust = 0x7ffffc01;

	union { 
		double flt;
		unsigned long fx[2];
	} x;
	union { 
		double flt;
		unsigned long i[2];   /* fy, fyl */
	} y;
	double n;
	union { 
		double flt;
		unsigned long i[2];   /* fnh, fnl */
	} fn;
	union { 
		double flt;
		unsigned long h;      /* fcrh */
	} fcr;
	double arg, dely, high, low, z, t1, t2, t0, t3, result;
	long i, flag;

	t = (struct tdef *)__tab;      /* init table pointer */

	y.flt = tmpx;             /*  1.0 <= y < 2.0 */

	if (y.flt >= 1.5)
	{ 
		n =  1.0;             /* number to multiply by log 2. */
		i = (y.i[0] >>13) & 0x3f ;   /* table lookup index  */
		arg = 0.5 * y.flt;
	}
	else
	{ 
		n = 0.0;
		i = ((y.i[0] >> 12) & 0x07f) + 64;  /* table lookup index  */
		arg = y.flt;
	}

	y.flt = (arg - t[i].table) * t[i].recip;
	flag = (t[i].fcn.i != 0);
	dely = ((arg-t[i].table) - y.flt * t[i].table) * t[i].recip;
	z = y.flt * y.flt;                /* polynomial evaluation  */
	if (n == 0.0)
	{
		t0 = (((d8*z + d6)*z + d4)*y.flt+((d7*z+ d5)*z + d3))*y.flt+ d2;
		t1 = y.flt + t[i].fcn.flt;
		low = t[i].fcn.flt - t1 + y.flt + (dely-y.flt*dely);
		result = (t0*z + low) + t1;
		*tail = t1 - result + (t0*z + low);
	}

	else if (t[i].fcn.i != 0)         /* small argument?        */
	{
		low = n * *(double*) log2l + dely;
		high = y.flt + t[i].fcn.flt;
		dely = t[i].fcn.flt - high +y.flt;

		/* high order part of n*log2     */
		t1 = n* *(double*)log2h + low;

		/* low order part of n*log2 */
		low = (n* *(double*)log2h - t1) + low;

		t0 = ((log_c5 * z + log_c3) * y.flt + 
		      (log_c6 * z + log_c4) * z) + log_c2;

		/* No. use 6th degr. approx           */
		t2 = high + t1;  /* high order part of fcn(i) + log2*n */
		t3 = (t1 - t2) + high; /* low order part of fcn(i)+log2*n  */

		/* final result  */
		result = ((((t0*z + low) + t3) + dely) + t2);

		*tail = t2 - result + dely + t3+ (t0 * z + low);  /* residual */
	}
	else /* Yes, arg. close to 1, use 8th deg. poly.   */
	{
		low=n* *(double*)log2l+dely; /* n times the low bits of log2 */
		t1 = n* *(double*)log2h + low;  /* high order part of n*log2 */
		low = (n * *(double*)log2h - t1) + low;
		t0 = (((d8*z + d6)*z + d4)*y.flt +  
			((d7*z+ d5)*z + d3))*y.flt + d2;
		result = ((t0*z + low) + y.flt) + t1 ;  /* final result  */
		*tail = t1 - result + y.flt +(t0*z + low);  /* residual */
	}
	return(result);
} /* end loginner */
