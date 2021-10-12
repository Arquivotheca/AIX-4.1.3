#ifndef STRTOLD
static char sccsid[] = "@(#)77	1.8  src/bos/usr/ccs/lib/libc/POWER/_qatof.c, libccnv, bos411, 9428A410j 2/26/94 11:46:28";
#endif
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: _isitinf
 *		_isitnan
 *		_qatof
 *		strtold
 *
 *   ORIGINS: 55,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) ISQUARE, Inc. 1990
 */

/*
 * NAME: _qatof
 *                                                                    
 * FUNCTION: convert ascii string to quad precision representation
 *
 * RETURNS: quad precision number
 *
 * Actually, if the preprocessor symbol STRTOLD is defined, this file
 * will build strtold().  Otherwise it builds _qatof().  The file
 * strtold.c defines STRTOLD and then includes this file.
 */

/****************************************************************************/
/*                                                                          */
/*           PROGRAM: Conversion routines: ascii to quad fl.Pt.             */
/*                    _qatof                                                */
/*           AUTHOR:  ISQUARE, Inc.                                         */
/*           DATE:    7/28/90                                               */
/*           MOD:     8/26/90 spec --> RS/6000                              */
/*           NOTICE:  Copyright (c) ISQUARE, Inc. 1990                      */
/*           NOTES:   _qatof is the quad precision equivalent of atof.      */
/*                    It converts the initial portion of the string pointed */
/*                    to by nptr to RS/6000 quad precision representation.  */
/*                    The input is assumed to consist of a certain amount   */
/*                    of white space, followed by a sequence of characters  */
/*                    representing a floating point number, and a final     */
/*                    character,  possibly the null character which         */
/*                    terminates the string.  The sequence of characters    */
/*                    that represent the floating point number is converted */
/*                    to quad precision, which is returned into an array.   */
/*                                                                          */
/*                    _qatof differs from atof in having to return a pair   */
/*                    of doubles.  A quad type is defined, which is a       */
/*                    structure consisting of two double precision words.   */
/*                    _qatof returns an object of type quad.  On the RS/6000*/
/*                    if this function is used from another language, the   */
/*                    first parameter should effectively be a pointer to a  */
/*                    double word aligned area where the two double words   */
/*                    of the result should be stored.                       */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*   Definition of the quad precision data type follows:                    */
/****************************************************************************/
#ifndef STRTOLD
   typedef struct {
        double          head, tail;
                  }     QUAD;
#endif /* !STRTOLD */

#include <float.h>
#include <limits.h>		/* INT_MAX */
#ifdef STRTOLD
#include <errno.h>		/* errno, ERANGE */
#include <langinfo.h>		/* nl_langinfo, RADIXCHAR */
#include <stdlib.h>		/* NULL */
#endif /* STRTOLD */

/* use DBL_INFINITY_D and LDBL_INFINITY_D
 * rather than obvious name to avoid a conflict
 * with DBL_INFINITY and LDBL_INFINITY in
 * float.h
 */

#ifdef STRTOLD
#define LDBL_QNAN_D (*((long double *)&qnan))
#define LDBL_SNAN_D (*((long double *)&snan))
#define LDBL_INFINITY_D (*((long double *)&infi))
#endif

#define DBL_QNAN_D (*((double *)&qnan))
#define DBL_SNAN_D (*((double *)&snan))
#define DBL_INFINITY_D (*((double *)&infi))

/****************************************************************************/
/*   The number of decimal digits collected in one double-word              */
/****************************************************************************/
#define ND 15
extern _q_mp10(int ndx, double a, double b, double c, double result[3]);
extern _q_mp10a(int ndx, double a, double result[3]);

#ifdef STRTOLD
long double 
strtold(const char *nptr, char **endptr)
#else /* must be _qatof, the default */
QUAD 
_qatof (const char *nptr)
#endif /* STRTOLD */
{
        static int two200[2]  = { 0x4c700000, 0x00000000 };
        static int twor200[2] = { 0x33700000, 0x00000000 };
        static int      infi[]={0x7ff00000, 0x0, 0x0, 0x0};
        static double   truncate=8192.0*8192.0*8192.0*8192.0+1.0;
        static double   epsilon=.5/(8192.0*8192.0*8192.0*8192.0);   /* .5 ulp*/
        char            ch;               /*current character                */
        char            *start;           /*ptr->start of numeric string     */
        int             extrapower=0;     /*digits left of dec. pt. ignored  */
        int             power=0;          /*exponent indicated in number     */
        int             powersign=1;      /*assume non-negative exponent     */
        int             i, j;             /*for temporary integer results    */
#ifdef STRTOLD /* strtold returns a long double */
	long double	result;
#else /* _qatof returns a "QUAD" */
        QUAD            result;
#endif /* STRTOLD */
        double          hi=0;             /*high order 15 digits, as read    */
        double          mid=0;            /*middle 15 digits, as read        */
        double          low=0;            /*low 15 digits, as read           */
        double          temp;             /*during multiplication use        */
        double          sign=1.0;         /*(+1) or (-1), depending on sign  */
        double          a,b,c, r[3];      /*temp results in macro multiply   */
        double          s[3], t[3];       /*temp results in macro multiply   */
        double          u,v,w,x,y,z;      /*temps for multiple precision add */
        double          oldfpscr;         /*save caller's fpscr              */
        double          *p10, *np10;
        int             count=0;          /*count of digits read             */
	int		digit_seen = 0;   /*flag; have we seen a valid digit?*/
#ifdef STRTOLD
	char		*radix;	          /*radix character for current locale*/
	char		*valid_address;	  /*in case nptr is null             */
	int             radix_size;
	char            *nptr_w;
	union  {
		long double l;
		struct { double hi, lo; } d;
		} rstruct;
#endif /* STRTOLD */

#ifdef STRTOLD
   /* make sure end pointer is valid */
   if (endptr == NULL)
     endptr = &valid_address;
	
   /* Update *endptr with nptr; after this we need only update
    * in case of good conversion.
    */
   *endptr = nptr;	

   /* if input string is NULL no need to do further work */
   if (nptr == NULL)
     return 0.0L;
    
#endif /* STRTOLD */
	
   oldfpscr=__setflm(1.0);                /*disable exceptions, set round-to-*/
                                          /*nearest.                         */
   while (isspace(ch=*nptr)) nptr++;      /*skip over white space            */

/******************************************************************************/
/*    At this point, start to look at the number.  This is the only           */
/*    point where there can be a sign for the number                          */
/******************************************************************************/
   switch (ch)
     {
       /* watch the fall-thru */
       case('-'): sign=-sign;               /*change assumed plus sign for    */
       case('+'): *nptr++;                  /*either sign,advance to next char*/
     }

/******************************************************************************/
/*  Now start to gobble up digits                                             */
/******************************************************************************/
   start=nptr;
   ch=*nptr++;
   while (isdigit(ch))
     {
       count < ND;                          /*prepare for all compares       */
       count < 2*ND;
       count < 3*ND;
       ch != '0';
       if (count < ND)
         {                                  /*update high order word         */
           hi=hi*10.0+(ch & 0x0f);
	   digit_seen++;
         }
       else if (count < 2*ND)
         {                                  /*or update middle word         */
           mid=mid*10.0+(ch & 0x0f);
         }
       else if (count < 3*ND)
         {                                  /*or update low order word      */
           low=low*10.0+(ch & 0x0f);
         }
       else
         {
           extrapower++;                    /*too many digits, increase exp.*/
           count--;                         /*& leave count unchaged in loop*/
         }
       if ((count > 0) || (ch !='0')) 
	 count++;                           /*count significant digits  */
       ch=*nptr++;                          /*point to next character       */
     }

/****************************************************************************/
/*  Should the non-numeric character be a decimal point, continue to scan   */
/*  the number for additional numeric digits.                               */
/****************************************************************************/
#ifdef STRTOLD
   radix = nl_langinfo(RADIXCHAR);
   if ( (radix == NULL) || !(*radix) )
     radix = ".";

   radix_size = strlen(radix);
   nptr_w = nptr;
   nptr_w--;
   if (!strncmp(radix, nptr_w, radix_size))
#else /* must be _qatof */
   if (ch == '.')
#endif /* ifdef STRTOLD */
     {                                     /*take care of the decimal point */
#ifdef STRTOLD
       if (radix_size > 1)
	 nptr+=(radix_size - 1);
#endif /* STRTOLD */

       ch=*nptr++;

       while (isdigit(ch))
         {
           count < ND;                     /*prepare for all compares       */
           count < 2*ND;
           count < 3*ND;

           ch !='0';
           if (count < ND)
             {                             /*update high order word         */
               hi=hi*10.0+(ch & 0x0f);
               extrapower--;               /*but decrease exponent          */
	       digit_seen++;
             }
           else if (count < 2*ND)
             {                             /*or update middle word          */
               mid=mid*10.0+(ch & 0x0f);
               extrapower--;
             }
           else if (count < 3*ND)
             {                             /*or update low order word       */
               low=low*10.0+(ch & 0x0f);
               extrapower--;
             }
           else                            /*insure counts remains unchanged*/
             count--;                      /*in loop                        */
           if ((count > 0) || (ch != '0')) 
	     count++;                      /*cnt significant digits */
           ch=*nptr++;                     /*point to next character        */
         }                                 /*finish reading numeric field   */
     }                                     /*finish exam fld after a dec pt */

/****************************************************************************/
/*  This point is reached on an I for infinity, N for NaNQ or NaNS,         */
/*  or an E, D, or Q for exponent starter.  The I or N must occur before    */
/*  any digits have been seen.                                              */
/****************************************************************************/
   if (nptr != start+1)
     {                                     /*a valid spot for an exponent   */
     /* if no digits seen yet string is invalid */
       if (digit_seen == 0)
	 {
#ifdef STRTOLD
	 result = 0.0L;
#else /* must be _qatof */
	 result.head = 0.0;
	 result.tail = 0.0;
#endif /* STRTOLD */
	 return result;
         }
 
       if (
#ifndef STRTOLD			/* Fortran respects 'Q','E' & 'D'; C only 'E' */
           (ch=='q') || (ch=='Q') ||
           (ch=='d') || (ch=='D') ||
#endif /* not STRTOLD */
	   (ch=='e') || (ch=='E')
	   )

         {

           switch (ch=*nptr++)
             {
               case('-'): powersign=-1;	/* fall-thru */
               case('+'): ch=*nptr++;
             }
           while (isdigit(ch))
             {
	     /* 0x20000000 is a semi-arbitrary constant designed to make
	      * any realistic string such as 0.00000000...000eNNNN work,
	      * while preventing overflow on the addition of power and
	      * extrapower below for strings such as 100000...000000.0e-NNNN
	      * The low-order 16 bits are zero so it can be generated easily.
	      */
	       if (power < 0x20000000)        /* avoid possible overflow */
                 power=power*10+(ch & 0x0f);  /*develop exponent magnitude   */
               ch=*nptr++;	              /* continue to digest all digits */
             }
         }
     }                                       /*finish reading exp, if legit */

   if (nptr == (start+1))                    /*if at the first character,   */
     {                                       /*Inf or Nan is still possible */
       __setflm(oldfpscr);                   /*restore user rounding mode,  */
       switch(ch)                            /*etc.                         */
         {
           case 'n':
           case 'N':
             {
               if (i = _isitnan(nptr, &result))  /* NaN ?                       */
                 {                             /*Yes                          */
                   if (sign < 0.0)             /*Honor the sign, if negative  */
                     {
#ifdef STRTOLD
		       result = -result;
#else /* must be _qatof */
                       result.head = -result.head;
                       result.tail = -result.tail;
#endif /* STRTOLD */
                     }
#ifdef STRTOLD
		   *endptr = (nptr - 1) + i;
#endif /* STRTOLD */
		   return result;              /*Yes-> ready to return        */
		 }
               else break;
             }
           case 'I':
           case 'i':
             {
               if (i = _isitinf(nptr, &result))  /* Infinity ?                  */
                 {
                   if (sign < 0.0)             /*Honor the sign, if negative  */
                     {
#ifdef STRTOLD
		       result = -result;
#else /* must be _qatof */
                       result.head = -result.head;
                       result.tail = -result.tail;
#endif /* STRTOLD */
                     }
#ifdef STRTOLD
		   *endptr = (nptr - 1) + i;
#endif /* STRTOLD */
                   return result;
                 }
               else break;
             }
         }                                   /*end of special cases         */
#ifdef STRTOLD
       result = 0.0L;
#else /* must be _qatof */
       result.head = 0.0;
       result.tail = 0.0;
#endif /* STRTOLD */
       return result;
     }                                       /*finish looking at 1st char   */

   power=extrapower+powersign*power;         /*effective power              */
   if (hi == 0.0)
     {                                       /*result is zero               */
       __setflm(oldfpscr);                   /*restore user rounding mode   */
#ifdef STRTOLD
       rstruct.d.hi = sign * hi;
       rstruct.d.lo = sign * hi;
       result = rstruct.l;
#else /* must be _qatof */
       result.head=sign * hi;
       result.tail=sign * hi;
#endif /* STRTOLD */
#ifdef STRTOLD
       if (digit_seen != 0)
	 *endptr = nptr - 1;	/* nptr has read 1 past the character
				 * which terminated the scan 
				 */
#endif /* STRTOLD */
       return result;
     }

   /* to avoid working in denorm region, scale the exponent up.
      It needs to scale the exponent down after computation is done.
   */
   if(power < -256) {  
      hi  *= *((double *)two200);
      mid *= *((double *)two200);
      low *= *((double *)two200);
   }

/****************************************************************************/
/*   The number must now be glued back.                                     */
/*       First, combine the three components.                               */
/****************************************************************************/
   if (count < (ND+1))                    /*simple case-multiply by exponent*/
     {
        _q_mp10a(power, hi, r);
        (r[0] > .179769e309);             /* compiler directive             */
        hi=r[0];
        mid=r[1];
        low=r[2];
     }
   else if (count < 2*ND+1)
     {                                    /*more complicated -- hi must be  */
                                          /*multiply by extra 10^(count-ND) */
       _q_mp10a(power+count-ND, hi, r);
       _q_mp10a(power, mid, s);           /*now add two vectors of length 3 */
       u=s[2]+r[2]+s[1];                  /*all the low order stuff added   */
       v=s[0];                            /*mid term -may be some overlap   */
       x=v+r[1];                          /*middle term                     */
       (__fabs(r[1]) > __fabs(v));        /*compiler directive              */
       if (r[0] > .179769e309)            /*near overflow force-->          */
         __setflm(truncate);              /*truncation instead of rounding  */
       hi=r[0]+x;                         /*high term                       */
       y=(__fabs(r[1]) > __fabs(v))? (r[1]-x)+v : (v-x)+r[1];
       mid=(v=((r[0]-hi)+x))+y;           /*middle term                     */
       low=u+((v-mid)+y);
     }
   else
     {                                    /*a fair amount of work for       */
                                          /* 31-45 digits                   */
       _q_mp10a(power+count-ND, hi, r);   /*mult hi by extra 10^(count-ND)  */
       _q_mp10a(power+count-2*ND, mid, s);/*mult mid by extra 10^(count-2ND)*/
       _q_mp10a(power, low, t);           /*multiply low just by power      */
                                          /*add 3 vectors of length 3       */
       u=t[2]+s[2]+t[1]+s[1]+r[2];        /*all the low order stuff added   */
       v=s[0]+t[0];                       /*mid term -may be some overlap   */
       w=(s[0]-v)+t[0];                   /*low order parts                 */
       x=v+r[1];                          /*middle term                     */
       (__fabs(r[1]) > __fabs(v));        /*compiler directive              */
       if (r[0] > .179769e309)            /*near overflow force -->         */
           __setflm(truncate);            /*truncate instead of rounding    */
       hi=r[0]+x;                         /*high term                       */
       y=(__fabs(r[1]) > __fabs(v))? (r[1]-x)+v : (v-x)+r[1];/*middle term  */
       mid=(v=((r[0]-hi)+x))+y;           /*middle term                     */
       low=u+w+((v-mid)+y);
     }

   /* scale the exponent back.  
      see if(power < -256) clause in the above.
   */
   if(power < -256) {
      hi  *= *((double *)twor200);
      mid *= *((double *)twor200);
      low *= *((double *)twor200);
   }

   if (!(r[0] > .179769e309))              /*not close to overflow -->      */
     {
        __setflm(oldfpscr);                /*restore user rounding mode     */
        temp=hi+mid;                       /*add terms, to get possible     */
        mid=(hi-temp)+mid+low;             /*carries between terms          */
     }
   else
/****************************************************************************/
/*   DANGER of overflow...stay in truncate mode and use different order to  */
/*   combine terms.  Rounding is abandonned to get extra bits in the low    */
/*   order word.                                                            */
/****************************************************************************/
     {
        mid=mid+low;                       /*get effects of low order word  */
        temp=hi+mid;                       /*get most significant term      */
        x=(hi-temp)+mid;                   /*and least significant term     */
        __setflm(oldfpscr);                /*restore users rounding mode    */
        if (x > epsilon*hi)                /*No choice!must accept overflow */
          {
             temp=hi-mid*(-1.0);           /*trick to avoid commoning!      */
             mid=(hi-temp)+mid;
          }
        else
             mid=x;
             if (temp==DBL_INFINITY_D)
               mid=0;
     }
   if  ((temp != temp) || (mid != mid))
     {
       temp=mid=DBL_INFINITY_D;
     }
#ifdef STRTOLD
   rstruct.d.hi = sign * temp;
   rstruct.d.lo = sign * mid;
   result = rstruct.l;
   *endptr = nptr - 1;	        /* nptr has read 1 past the character
				 * which terminated the scan 
				 */

   if (rstruct.d.hi == 0.0)
     errno = ERANGE;
	
   if (__fabs(rstruct.d.hi) == DBL_INFINITY_D)
     errno = ERANGE;

#else /* must be _qatof */
   result.head=sign*temp;
   result.tail=sign*mid;
#endif /* STRTOLD */
   return result;
  }

/*
 * NAME: _isitnan
 *                                                                    
 * FUNCTION: check string to see if it is a "NaN"
 *
 * RETURNS:  4 if string matches "NANQ"
 *           4 if string matches "NANS"
 *           3 if string matches "NAN"
 *           0 otherwise.
 *
 * (If non-zero, the return value is the length of the
 * matched string).
 */

#ifdef STRTOLD
static int
_isitnan(char *c, long double *q)
#else /* must be _qatof */
static int
_isitnan(char *c, QUAD *q)
#endif /* STRTOLD */
  {
  static int      qnan[]={0x7ff80000, 0x0, 0x0, 0x0};
  static int      snan[]={0x7ff55555, 0x55555555, 0x0, 0x0};
  char            ch;
  int             i;
  static const char s_AN[] = "AN";
  
  /* you already have identified a 'N' at this point;
   * if next two characters are "aN" it's a Nan
   */
  
  for (i=0; i < 2 ; i++)
      {
      /* Note that a char & 0xdf converts it to upper case */
      if (s_AN[i] != ((0xdf) & (*c++)))
          {
	  return 0;
          }
      }
  if ('Q'== ((0xdf) & (ch=*c)))
      {
                                           /*input was NaNQ                 */
#ifdef STRTOLD
      *q = LDBL_QNAN_D;
#else /* must be _qatof */
      (*q).head=DBL_QNAN_D;
      (*q).tail=0.0;
#endif /* STRTOLD */
      return 4;
      }
                                           /*not a NaN input                */
                                           /*input was NaNS                 */
#ifdef STRTOLD
      *q = LDBL_SNAN_D;
#else /* must be _qatof */
      (*q).head=DBL_SNAN_D;
      (*q).tail=0.0;
#endif /* STRTOLD */

  if ('S' == ((0xdf) & (ch = *c)))
    return 4;
  else
    return 3;
  }
     
/*
 * NAME: _isitinf
 *                                                                    
 * FUNCTION: check input string; return true if matches character
 *           string for infinity
 *
 * RETURNS: 3 if string matches "INF"
 *          8 if string matches "INFINITY"
 *          0 otherwise.
 *
 * (If non-zero, the return value is the length of the
 * matched string).
 */

#ifdef STRTOLD
static int
_isitinf(char *c, long double *q)
#else /* must be _qatof version */
static int
 _isitinf(char *c, QUAD *q)
#endif /* STRTOLD */
  {
  static int      infi[]={0x7ff00000, 0x0, 0x0, 0x0};
  int             i;
  static const char s_NF[] = "NF";
  static const char s_INITY[] = "INITY";
  
  /* just "inf" (case insensitive) is a match for infinity */
  
  for (i=0; i < 2 ; i++)
      {
      /* Note that a char & 0xdf converts it to upper case */
      if (s_NF[i] != ((0xdf) & (*c++)))
          {
	  return 0;
          }
      }
#ifdef STRTOLD
  *q = LDBL_INFINITY_D;
#else /* _qatof */
  (*q).head=DBL_INFINITY_D;
  (*q).tail=0.0;
#endif /* STRTOLD */

  for (i = 0; i < 5; i++)
      {
      /* Note that a char & 0xdf converts it to upper case */
      if (s_INITY[i] != ((0xdf) & (*c++)))
	return 3;
      }
  return 8;
  }
