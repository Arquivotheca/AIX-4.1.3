static char sccsid[] = "@(#)78	1.5  src/bos/usr/ccs/lib/libc/POWER/_qcvt.c, libccnv, bos411, 9428A410j 10/3/93 12:56:44";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: _qecvt, _qfcvt, _qcvt
 *
 *   ORIGINS: 55,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) ISQUARE, Inc. 1990
 */

/****************************************************************************/
/*                                                                          */
/*           PROGRAM: Conversion routines: Quad Fl.Pt. to ASCII             */
/*                    *_qecvt and *_qfcvt                                   */
/*           AUTHOR:  ISQUARE, Inc.                                         */
/*           DATE:    7/28/90                                               */
/*           MOD:     8/26/90 spec --> RS/6000                              */
/*                    4/2/92  Correction of rounding problem                */
/*           NOTICE:  Copyright (c) ISQUARE, Inc. 1990                      */
/*           NOTES:   The output buffer size is limited to 512 characters   */
/*                    which is unreasonably large.  Internal calculations   */
/*                    are performed in triple working percision, that is,   */
/*                    roughly 48 digits.  Digits to the right of about the  */
/*                    48th are all zeros.                                   */
/*                                                                          */
/*          RS/6000 dependency:  use of __setflm to control rounding modes  */
/*          General dependency:  most computations in this module depend on */
/*                               round-to-nearest.  Even one-ulp accuracy   */
/*                               may not be acheived by this code without   */
/*                               round-to-nearest where specified.          */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*   SUPPORT ROUTINES:                                                      */
/*      _q_mp10   multiplies a triple precision number by power of 10.      */
/*      _q_mp10a  multiplies a double precision number by power of 10.      */
/*      _q_pow10  returns 10^i                                              */
/*                                                                          */
/****************************************************************************/
/*   CALLING SEQUENCE DESCRIPTIONS:                                         */
/*                                                                          */
/*   char *_qecvt((quad) arg, int nr, int *ptr, int *sign)                  */
/*   char *_qfcvt((quad) arg, int nr, int *ptr, int *sign)                  */
/*                                                                          */
/*     arg  -->structure containing the quad number to convert              */
/*     nr   -->number of digits to the right of the decimal point           */
/*     *ptr -->position of decimal point relative to the start of the       */
/*            result string.                                                */
/*     *sign-->where to return the sign (0->plus, otherwise->negative)      */
/*                                                                          */
/*                                                                          */
/*   _qecvt produces a string of the form nnnnn, where the number of        */
/*          digits are given by nr.                                         */
/*                                                                          */
/*   _qfcvt produces a string of the form mmmnnnnn, where the decimal       */
/*          point lies between the string of m's and n's.                   */
/*          The number of n's is given by nr, and the number of m's         */
/*          depends on the magnitude of the argument                        */
/*                                                                          */
/****************************************************************************/
typedef struct {
        double  head, tail;
  }     quad;
extern _q_mp10(int ndx, double a, double b, double c, double result[3]);
extern _q_mp10a(int ndx, double a, double result[3]);
extern _q_pow10(int ndx, double result[3]);
char    *_qcvt();                          /*internal routine to carry       */
                                           /*out the details of conversion   */
#define shr_itrunc(X) ((int) (X))
/*
 * NAME: _qecvt
 *                                                                    
 * FUNCTION: convert quad precision number to ascii string
 *
 * RETURNS: pointer to converted string
 */

char *_qecvt(arg, nr, ptr, sign)
        quad    arg;
        int     nr, *ptr, *sign;
  {
    return _qcvt(arg, nr, ptr, sign, 1);
  }

/*
 * NAME: _qfcvt
 *                                                                    
 * FUNCTION: convert quad precision number to ascii string
 *
 * RETURNS: pointer to converted string
 */

char *_qfcvt(arg, nr, ptr, sign)
        quad    arg;
        int     nr, *ptr, *sign;
  {
    return _qcvt(arg, nr, ptr, sign, 0);
  }

/****************************************************************************/
/*                                                                          */
/*   char *_qcvt((quad) arg, int nr, int *ptr, int *sign, ecvtform)         */
/*                                                                          */
/*     See above for the definitions of arg, nr, ptr, and sign              */
/*     ecvtform --> TRUE  for eqcvt type of conversion                      */
/*              --> FALSE for fqcvt type of conversion.                     */
/*                                                                          */
/****************************************************************************/

/* size of the working buffer for cvt-- see ANSI C Sec. 4.9.6.1,
 * "Environmental Limit" -- the mininum number of characters
 * produced is 509.
 */
#define BUFSZ 512                         /*maximum string length returned  */
#define DBL_INFINITY (*((double *)&infi))

/*
 * NAME: _qcvt
 *                                                                    
 * FUNCTION: workhorse function for _qecvt, _qfcvt
 *
 * RETURNS: pointer to converted string
 */

char *_qcvt(arg, nr, nptr, sign, ecvtform)
        quad    arg;
        int     nr, *nptr, *sign, ecvtform;
{
        union   fpscr   {double reg; char nibble[8];}
                         oldfpscr, copyscr;
        static char      nanqstr[] = "NaNQ\0";
        static char      nansstr[] = "NaNS\0";
        static char      infstr[]  = "INF\0";
        static char      area[BUFSZ];              /*work area for results    */
        static double    small = 1.0e-265;
        static double    lg210 = 0.30102997;       /*log 2 (base 10)          */
        static double    two52 = 8192.0*8192.0*8192.0*8192.0;      /*2**52    */
        static double    twom20 = 1.0/1048576.0;
        double           higher[3], factor[3];
        static int       infi[2]={0x7ff00000,0};
        char             *rp, *ptr, *p;
        int              i, j;
        int              ndigits, scale;
        int              trop;
        int              scalefactor = 0;
        int              scaleadj = 0;     /*adjust scaling if too many digits*/
        double           a, b, c;          /*working copy number left to cvt  */
        double           aa, bb, cc;
        double           r[3], s[3], t[3]; /*working temporaries              */
        double           u, v, w, x, y, z; /*other temporaries-used for times */
        double           *ptbl, *p10, *np10;
        quad             work;             /*copy of the argument             */


    ndigits = nr;                          /*guess digits to rt of dec. pt.   */
    work = arg;                            /*make working copy of argument    */
    oldfpscr.reg = __setflm(two52);        /*save original rounding mode,     */
                                           /*force round-to-nearest           */
    copyscr.reg = 0.0;                     /*manufactured fpscr               */
    *nptr = 0;                             /*Initialize position of fraction  */
    *sign = *((unsigned int*)&arg) >> 31;  /*calculate the sign               */
    (__fabs(work.tail) > __fabs(work.head));
    (work.tail != work.tail);
    if (work.head < 0.0)
      {
        work.head = -work.head;            /*change sign to force positive    */
        work.tail = -work.tail;            /*head/tail may have opposite signs*/
      }
    ndigits = (ndigits < 0)?0:ndigits;     /*negative number of digits? NO    */
    ndigits = (ndigits < BUFSZ-1)?ndigits:BUFSZ-2;         /*Limit size of    */
                                           /* result to BUFSZ characters      */
    p = &area[ndigits];                    /* point to last char of result    */
    area[0] = '\0';                        /* put null, should str. be empty. */

    /**********************/
    /*Special cases follow*/
    /**********************/

    if (__fabs(work.tail) > __fabs(work.head))  /* tail is larger than head???*/
      {
        u = work.head + work.tail;              /* put into canonical form!   */
        if (__fabs(work.tail) != DBL_INFINITY)  /* avoid creation of NaN      */
          work.tail = work.tail - u + work.head;
        work.head = u;
        *sign = *((unsigned int*)&u) >> 31;     /* recalculate the sign       */
        if (work.head < 0)
          {
            work.head = -work.head;
            work.tail = -work.tail;
          }
      }

    if (work.tail != work.tail)            /*low order part of input is NaN   */
      if (work.head == work.head)          /*but high order part is real      */
         work.head = work.tail;            /*make h.o. part a NaN, too        */

    if (work.head == 0.0)                  /* Output a zero                   */
      {
        *p = '\0';                         /* Null,  to end output string     */
        while ( p != area )
          {
            *--p = '0';                    /* pad with zeros                  */
          }
        __setflm(oldfpscr.reg);
        return area;                       /* and return                      */
      }
    if ((*((int *)&work.head) & 0x7ff00000) == 0x7ff00000) /*infinity ?       */
      {                                                   /* or NaN ?         */
        *p = '\0';                         /* Null,  to end output string     */
        while ( p != area )
          {
            *--p = '\0';                   /* pad with nulls                  */
          }
                                           /* p -> start of output field      */
        rp = infstr;                       /* Prepare to output an infinity   */
        if (work.head != work.head)        /* CAUTION ! -- it might be a NaN  */
          {
                                           /* What kind of NaN is here?       */
            rp = ( *((int *)&work.head) & 0x00080000) ? nanqstr : nansstr;

          }
        while ( *rp != '\0') *p++ = *rp++; /*Insert NaN or INF into output    */
        *p  = *rp;
        __setflm(oldfpscr.reg);
        return area;
      }
    a = work.head;                         /*prepare a working copy of arg,   */
    b = work.tail;                         /*pad with a word of zeros.        */
    c = 0.0;

    if (work.head < small)                 /*for very small numbers rescale   */
      {                                    /*by 10^48                         */
        _q_mp10(64,a,b,0.0,r);
        scalefactor = 64;
        a = r[0];
        b = r[1];
        c = r[2];
      }

    /*************************************************************************/
    /*Approximate floor(log10(a)) to get number of digits to the left of the */
    /*binary point.  Coonen's algorithm always arrives at this number or one */
    /*too small.                                                             */
    /*************************************************************************/
    __setflm(two52+1.0);                    /*truncate mode will take care of*/
    u = a + b;                              /*small differences from powers  */
    __setflm(two52);                        /*of ten.                        */
    i = *((int *)&u) - (1023<<20);
    x = i * (twom20 * lg210);
    j = shr_itrunc((i >= 0) ? x : x-1);                 /*j is guess to floor(log10(a))  */

    /*************************************************************************/
    /*Now compute the scale and number of digits, for either ecvt or fcvt.   */
    /*May be repeated once if j too great an underestimate of log10(a)       */
    /*************************************************************************/

    trop = 1;

    aa = a;                                 /*remember original triple(a,b,c)*/
    bb = b;                                 /*in holding area, aa, bb, cc.   */
    cc = c;
    while(trop)
      {
        if (ecvtform)                       /* ecvt case ... all digits to   */
          {                                 /* right of dec. pt.             */
            scale = nr - j - 1;             /* multiplicative power of 10    */
            ndigits = nr;
          }
        else
          {                                 /*fcvt case ...                  */
            scale = nr;
            ndigits = j + 1 + nr;           /*when too many digits requested */
            if (ndigits > BUFSZ-2)
              {
                ndigits = BUFSZ-2;
                scale = BUFSZ - j - 1;
              }
          }
        if (ndigits < 0)
          {
            scale = -(j+1);
            ndigits = 0;
          }
        scaleadj = (ndigits -48 > 0)?ndigits-48 : 0;
        _q_mp10(scale-scaleadj, a,b,c, s);  /*prod of (a,b,c) by 10^scale    */
        a = s[0];
        b = s[1];
        c = s[2];
    /**************************************************************************/
    /* Take care of rounding here....>                                        */
    /*   put the machine into the user's rounding mode with all exception     */
    /*   enabling trap bits turned OFF.                                       */
    /*   For directed rounding mode and negative numbers, reverse the         */
    /*   sense of the rounding direction because the absolute value of the    */
    /*   argument is being processed.                                         */
    /**************************************************************************/
        i = 0x3 & oldfpscr.nibble[7];       /*get user's rounding mode bits   */
        copyscr.nibble[7] = (*sign)? i ^ (i >> 1) : i;
        __setflm(copyscr.reg);
        u = a + b;                          /*get integer part of product     */
        if (u < two52)
            { x = (u + two52) - two52;     /*force an integer*/
              y = u - x + b + c;           /*sum the remainder of quantity to */
              u = ((u + two52) + y) - two52;  /*be converted, and add to known*/
              v = w = 0; }                 /*large integer to produce an int*/
        else
          {
            x = (a - u) + b;
            y = ((a - u) - x) + b;          /*May not be in round to nearest! */
            w = y + c;                      /*No shortcuts here               */
            v = x + w;
            w = (x - v) + w;
            if (__fabs(v) < two52)
              {x = (v > 0 )? (v + two52) - two52: (v - two52) + two52;
               y = v - x + w;
               v = (v > 0)? ((v + two52) + y) - two52:
                            ((v - two52) + y) + two52;
               w = 0;}
            else
              {
                w = (w > 0)? (w + two52) - two52: (w - two52) + two52;
              }
          }
        __setflm(two52);                    /*return to round-to-nearest      */
        _q_pow10(ndigits-scaleadj,higher);  /*generate 10^digits >= (a,b,c)   */
        z = (higher[0]-u) + (higher[1]-v) + (higher[2]-w);
        j++;
        if (z > 0)
          {
            trop = 0;                       /*exponent guess was good         */
    /**************************************************************************/
    /*Round-to-nearest is now used to force the same sign on all components.  */
    /**************************************************************************/

            __setflm(two52+1.0);           /*stay truncate mode until done    */
            a = u + v;
            z = (u - a) + v;               /*very tricky w/o rd-to-nearest    */
            y = ((u-a) - z) + v;           /*y not necessarily 0 !            */
            c = y + w;
            b = z + c;
            c = (z - b) + c;
          }
        else                               /*exponent guess off by one        */
          {
            a = aa;                        /*one more time --note: power      */
            b = bb;                        /*of 10 has already been ++        */
            c = cc;
          }
      }                                    /*end do while                     */

    *nptr =  j - scalefactor;
    if ((!ecvtform) && (a == 0.0))         /*on zero, f format, return 0      */
      {
        __setflm(oldfpscr.reg);            /*restore fpscr                    */
        return area;
      }
    p = &area[ndigits];
    *p-- = '\0' ;                          /*NULL -> output end               */
    for (i=0; i<scaleadj; i++)*p-- = '0';  /*pad low order positions w zeroes */
    if (ndigits) _q_cvtl(p,area,a,b,c);
    __setflm(oldfpscr.reg);
    return area;
  }
