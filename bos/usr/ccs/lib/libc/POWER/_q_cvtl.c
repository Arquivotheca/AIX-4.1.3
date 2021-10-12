static char sccsid[] = "@(#)74	1.1  src/bos/usr/ccs/lib/libc/POWER/_q_cvtl.c, libccnv, bos411, 9428A410j 12/13/90 19:58:44";
/*
 * COMPONENT_NAME: LIBCCNV
 *
 * FUNCTIONS: _q_cvt, _qsuperdigit
 *
 * ORIGINS: 55
 *
 *                  SOURCE MATERIAL
 *
 * Copyright (c) ISQUARE, Inc. 1990
 */

/*
 * NAME: _q_cvt
 *                                                                    
 * FUNCTION: helper routine for _qcvt
 *
 * RETURNS: void
 */

/****************************************************************************/
/*                                                                          */
/*           PROGRAM: Conversion support routine for _qcvt.c                */
/*                    _q_cvtl                                               */
/*           AUTHOR:  ISQUARE, Inc., (V. Markstein)                         */
/*           DATE:    7/28/90                                               */
/*           MOD:     8/26/90 spec --> RS/6000                              */
/*           NOTICE:  Copyright (c) ISQUARE, Inc. 1990                      */
/*           NOTES:   Routine is called in round-to-zero mode.              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*   SUPPORT ROUTINES:                                                      */
/*      _q_mp10   multiplies a triple precision number by power of 10.      */
/*      _q_mp10a  multiplies a double precision number by power of 10.      */
/*                                                                          */
/****************************************************************************/
/*   CALLING SEQUENCE DESCRIPTIONS:                                         */
/*                                                                          */
/*   (void)_q_cvtl (char *p, char *area, double a, double b, double c)      */
/*                                                                          */
/*     area  -->character strin for converted digits                        */
/*     *p    -->where low order digit of character string is placed         */
/*     (a,b,c)->number to be converted in triple working precision          */
/*                                                                          */
/****************************************************************************/

#define RS6000
/****************************************************************************/
/*   Note: to compile for a machine other than the RS/6000, undefine        */
/*         the above RS6000 variable.  You must then provide routines       */
/*         to replace the nmsg subroutines to perform the same functions    */
/*         as the corresponding RS6000 macros which follow below.           */
/****************************************************************************/

/****************************************************************************/
/* macro to compute (x*y)-z with only one rounding error. x,y,z are numbers */
/****************************************************************************/

#ifdef RS6000
#define nmsf(i,j,k) i - j*k

#else
 double nmsf(double i, double j, double k);
#endif

extern double _qsuperdigit();

void   _q_cvtl (char *p, char *area, double a, double b, double c)

/****************************************************************************/
/*  Technique:  superdigits are extracted from (a,b,c) by effectively       */
/*              computing mod((a,b,c), pow10).  Each superdigit holds       */
/*              15 output digits.  High precision is only needed when       */
/*              extracting superdigits.  The remaining computations         */
/*              are all carried out in working precision!                   */
/****************************************************************************/

{
        double          q[3], s, t;
        static double   tenth=.1;
        static double   ten  =10.0;
        static double   big=8192.0*8192.0*8192.0*8192.0;
        union {double x;
               char ch[8];
               }        d;
        double          savefpscr;
        int             n=0;

  ten + tenth + big;                  /*compiler directive-load outside loop */
  do {
    if (n == 0) {
      s=_qsuperdigit(a, b, c, q);     /*15 digits compacted in s             */
      a=q[0];                         /*unpack quotient q into (a,b,c)       */
      b=q[1];
      c=q[2];
      n=15;
      }
    t=(s*tenth+big) - big;            /*like divide by 10                    */
    d.x=(s-ten*t) + big;              /*d is remainder, to be output         */
    n--;
    s=t;

    *p=d.ch[7]+'0';                   /*create character, put in output      */

    } while (area != p--  );          /* enddo                               */

    return;
  }

/*
 * NAME: _qsuperdigit
 *                                                                    
 * FUNCTION: extract superdigits from three doubles
 *
 * RETURNS: double precision number with 15 superdigits
 */

 double  _qsuperdigit(double a, double b, double c, double t[3])
{

        static double   pow10=1.0e15;
        static double   rpow10=1.0e-15;
        static double   epsilon=1.0e-30;
        static double   big=8192.0*8192.0*8192.0*8192.0;
        static double   zero=0.0;
        static double   one=1.0;
        double          d, lost ;
        double          q,r,s,u,v,w,x,y,z;
        int             j;
/*****************************************************************************/
/*    _qsuperdigit accepts the triple (a,b,c) and extracts a superdigit      */
/*    by computing (a,b,c)/10^15, and truncating the result.  The quotient,  */
/*    multiplied by 10^15 is subtracted from (a,b,c) to compute superdigit.  */
/*    The quotient is returned in array t.                                   */
/*****************************************************************************/
  if (a < pow10)
    {                                 /*piece of cake when 15 or less digits */

      d=(a+big) - big;
      t[0]=t[1]=t[2]=zero;
      j=0;
      return d;
    }
  else
    {                                 /*less easy ->  isolate the low order  */
                                      /* 15 superdigits                      */
      q=a*rpow10;                     /*approximate quotient                 */
      if (q < big) q=(q+big) - big;
      r=nmsf(a, q, pow10);            /*exact remainder                      */
      s=r+b;                          /*total remainder                      */
      if (s < zero)                   /*negative remainders not wanted       */
        {
          q=nmsf(q, q, epsilon);      /*this knocks down q by one ulp        */
          if (q < big) q=(q + big) - big;
          r=nmsf(a, q, pow10);        /*in round-to-nearest mode.            */
          s=r+b;                      /*recompute remainders.                */
        }
      lost=(r > b)? (r-s) + b : (b-s)+r;
      if (q < big)                    /*quotient just one superdigit         */
        {
          s=s+c;
          while (s >= pow10)
            {
              s=s-pow10;              /*if remainder is too large,           */
              q=q+one;                /*adjust in the other direction        */
            }
          t[0]=q;
          t[1]=t[2]=zero;
          return s;
        }
      t[0]=q;
      q=s*rpow10;                    /*next word of quotient                 */
      if (q < big) q=(q+big) - big;
      u=nmsf(s, q, pow10);           /*next remainder                        */
      v=u+c+lost;
      if (v < zero)
        {
          q=nmsf(q, q, epsilon);      /*this knocks down q by one ulp in     */
          if (q < big) q=(q + big) - big; /*round to nearest mode.           */
          u=nmsf(s, q, pow10);
          v=u+c+lost;                 /*remainder                            */
        }
      if (q < big)                    /* Good remainder?                     */
        {
          while (v >= pow10)
            {
              v=v-pow10;              /*for too a large remainder,           */
              q=q+one;                /*adjust in the other direction        */
            }
          t[1]=q;
          t[2]=zero;
          return v;
        }
      t[1]=q;                         /*second superdigit of quotient        */
      q=v*rpow10;
      if (q < big) q=(q+big) - big;
      w=nmsf(v, q, pow10);            /*final remainder                      */
      if (w < zero)
        {
          q=nmsf(q, q, epsilon);      /*this knocks down q by one ulp in     */
          if (q < big) q=(q+big) - big; /*round-to-nearest mode.             */
          w=nmsf(v, q, pow10);         /*remainder                           */
        }
      t[2]=q;
      return w;
    }
  }
