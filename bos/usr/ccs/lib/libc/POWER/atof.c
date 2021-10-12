#if (!( ATOFF | STRTOD | STRTOF))
static char sccsid[] = "@(#)20	1.21  src/bos/usr/ccs/lib/libc/POWER/atof.c, libccnv, bos411, 9428A410j 3/3/94 09:21:22";
#endif

/*
 * COMPONENT_NAME: LIBCCNV atof
 *
 * FUNCTIONS: atof, atoff, strtod, strtof
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <float.h>
#include <errno.h>
#include <math.h>
#include <fpxcp.h>
#include <nl_types.h>
#include <langinfo.h>
#include <ctype.h>	/* isspace() */
#define TRUE  1
#define FALSE 0

typedef struct fp { 
	long int i[2]; 
};
#ifdef _ATOF_LINT
  /* compiler built-in */
  double __setflm(double); /* get old round mode; set new round mode */
  int __abs( int );
#endif
extern struct fp atof_pow1[128][2];
extern struct fp atof_pospow2[5][2];
extern struct fp atof_negpow2[6][2];
extern int atof_digit[];
extern int checknf(char *);
extern int checknan(char *);

static double negpow[18] = {
 1.0e-0, 1.0e-1, 1.0e-2, 1.0e-3, 1.0e-4, 1.0e-5, 1.0e-6, 1.0e-7, 1.0e-8,
 1.0e-9, 1.0e-10, 1.0e-11, 1.0e-12, 1.e-13, 1.e-14, 1.e-15, 1.e-15, 1.e-15};
static double ch[10] = {
	0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

/*
 * NAME: atof
 *                                                                    
 * FUNCTION: convert an ascii string to a float/double
 *                                                                    
 * NOTES:
 *
 *  RIOS Specific Versions of:
 *
 *  atof --   Ascii string to double conversion
 *  strtod -- Ascii string to double conversion
 *  atoff --  Ascii string to float conversion
 *  strtof -- Ascii string to float conversion
 *
 *
 *   The following descriptions are extracted from the proposed ANSI C
 *   standard.
 *
 * ***********************************************************************
 *
 *   double strtod(const char *nptr, char ** endptr)
 *
 *   "The strtod function converts the initial portion of the
 *   string pointed to by nptr to double representation. First it
 *   decomposes the input string into three parts: an initial,
 *   possibly empty, sequence of white-space characters (as specified
 *   by the isspace function), a subject sequence resembling a floating
 *   point constant; and a final string of one unrecognized character,
 *   including the terminating null character of the input string. Then
 *   it attempts to convert the subject sequence to a floating-point
 *   number, and returns the result.
 *
 *   The expected form of the subject sequence is an optional plus or
 *   minus sign, then a sequence of digits optionally containing a decimal
 *   point character, then an optional exponent part ...
 *
 *   ... if neither an exponent part nor a decimal point character
 *   appears, a decimal point is assumed to follow the last digit in the
 *   string. ... A pointer to the final string is stored in the object
 *   pointed to by endptr, provided that endptr is not a null pointer.
 *   ....
 *
 *      A valid decimal string has the following form:
 *
 *      ( [+|-] ((<d> {d} [<radix>{d}]) | (<radix> <d> {d})) [(e|E) [+|-] <d> {d} ] |
 *      "NaNS" | "NaNQ" | "INF" | "NaN" | "INFINITY" )
 *
 *      {} = 0 or more times
 *      [] = 0 or 1 times
 *	<> = one time  
 *	d  = one of the digits
 *      radix = locale dependent radix character.
 *
 *   (Note: this code can only take care of single byte radix character.)
 *
 *   If the subject sequence is empty or does not have the expected form,
 *   no conversion is performed; the value of nptr is stored in the object
 *   pointed to by endptr, provided that endptr is not a null pointer."
 *
 *   "The strtod function returns the converted value, if any. If no
 *   conversion could be be performed, zero is returned. If the correct
 *   value would cause overflow, plus or minus HUGE_VAL is returned.
 *   (according to the sign of the value), and the value of the macro
 *   ERANGE is stored in errno. If the correct value would cause underflow,
 *   zero is returned and the value of the macro ERANGE is stored in errno."
 *
 *   On RIOS HUGE_VAL is infinity; +0.0 is returned if no conversion
 *   is possible; and a properly signed 0.0 is returned on underflow.
 *
 *   The RIOS version will also recognize the strings: "NaNQ", "NaNS", and
 *   "infinity" and convert them to the proper IEEE value. If NaNQ or NaNS is
 *   preceeded by a sign, it is honored. If infinity is preceeded by a sign
 *   then a properly signed infinity is generated.
 *
 * ***********************************************************************
 *
 *   double atof(const char *nptr)
 *
 *   "The atof function converts the initial portion of the string pointed
 *   to by nptr to double representation. Except for the behavior on error
 *   it is equivalent to:
 *
 *              strtod(nptr, (char **)NULL) "
 *
 * ***********************************************************************
 *
 *   strtof and atoff are the same as strtod and atof except that
 *   they return a float value rather than a double. These routines
 *   avoid the double round that occurs when a string is first converted
 *   to a double (causing one round) and then converted to a float
 *   (causing another round).
 *
 *   strtof and atoff are not required by the proposed ANSI C standard.
 *
 *
 *   This source is written to generate all 4 functions above depending
 *   on the setting of the pre-processor values STRTOD, ATOFF, and STRTOF.
 *   (If none of these are set then ATOF is assumed.
 *
 *
 * RETURNS: a float/double value
 *
 * SIDE EFFECTS: errno may be altered
 */

#if STRTOD

double strtod(const char *nptr, char **endptr)

#else
#if ATOFF

float atoff (nptr)
char *nptr;

#else
#if STRTOF

float strtof (nptr,endptr)
char *nptr;
char **endptr;

#define STRTOD 1
#else

/* Must be ATOF */
double atof (const char *nptr)

#endif
#endif
#endif

  {
  double saverm;                /* Caller's round mode.                 */
  unsigned int  i,j;            /* temp array index          */
  union { 
  	double rslt[2];		/* result: hi and low parts  */
	long int i[4];
	} x;
  double sign = 1.0;		/* sign of mantissa, +1 = '+', -1 = '-'*/
  double expsign;		/* sign of exponent (+1 = '+'; -1 = '-')      */
  double xhi,xlo;		/* computed hi and lo result    */
  double yhi,ylo;		/* hi and lo parts of exponent multiplier */ 
  double *py,*py2;        
  char *valid_addr;		/* a valid address for endptr to point */
  double dblrslt;		/* result to be returned for atof and strtod */
  float  fltrslt;		/* result to be returned for atoff and strtof */
  int    expon;			/* index to atof_pow1 array */
  double dexpon;		/* work double of expon for performance */
  char   *s = (char *) nptr;	/* current character pointer                  */
  char   *ptr1;			/* pointer to the begining of string after    */
				/* skipping spaces and sign */
  char   *Eptr, *Eptr1;		/* start of exponent string                   */
  char   *radix;		/* radix character from locale info           */
  char   c;			/* current character holder                   */
  double h, l;			/* the 1st and 2nd 15 significant digit result */
  double ten;			/* constant 10.0                              */
  int    ct;			/* number of significant digits collected     */
  int    itmp,itmp2,flag;	/* temporary working integer */
  unsigned int uitmp;		/* temporary working integer */
  double xtmp,xflag;		/* temporary working double */
  double expwork;		/* holds the converted exponent            */
  int    nanret, ret;		/* nanret is flag returned from checknan; ret is */
				/* flag returned from checknf */
  union {
  	double x;
	int i[2];
	} ux;  /* work union */

  (NULL == s);
#if STRTOD
  if ( endptr == (char **)NULL)
    endptr = &valid_addr;
#endif
  saverm = __setflm(0.0);	/* set round mode=RN,save mode */
  ten = 10.0;
  expsign = 1.0;
  ct = 0;
  expwork = 0.0;
  ret = 0;
  Eptr = (void *) NULL;		/* start out with Eptr = NULL                 */
  dexpon = 0.0;			/* start out with exponent = 0                */
  if( NULL == s ) goto invalid; /* prevent dereferencing a NULL pointer       */ 
  
  /* check first to see if string is already pointing to a valid digit (probably
   * the most common case).  By having this check first we avoid the further
   * overhead of skipping leading space, etc.
   */

  if (atof_digit[*s])
      {
      /* s is already pointing to a valid digit */
      h = 0.0;			/* init the high & low part of result to +0.0 */
      l = 0.0;
      ptr1 = s;			/* pointer to the beginning of string after   */
				/* skipping spaces and sign */
      c = *s++;
      if (c == '0') 
	goto skipzero;
      else 
	goto start;
      }

  /* if here, first character in string was NOT a valid digit.  So we 
   * first look for white space.
   */

  while (isspace(c = *s) ) s++;	/* skip leading whitespace */

  /* once again look for most likely case, a valid digit */

  if (atof_digit[c]) 
      {
      h = 0.0;			/* init the high & low part of result to +0.0 */
      l = 0.0;
      ptr1 = s;			/* pointer to the beginning of string after   */
      /* skipping spaces and sign */
      s++;
      if (c == '0') 
	goto skipzero;
      else
	goto start;
      }

  /* if here we are looking at a non-white space character which is
   * not a valid digit.  Look at next possibility, a sign character 
   */
  
  switch (c)                    /* Look for + or - sign            */
      { 
    case '-': 
      sign = -sign;		/* set sign to -1.0 */
    case '+':			/* FALL THRU */
      s++;              
      }
  
  ptr1 = s;			/* pointer to the beginning of string after   */
				/* skipping spaces and sign */
  
  c = *s++;			/* get the first nonspace and nonsign charater */
  (c == '0');			/* scheduling directive */
  h = 0.0;			/* init the high & low part of result to +0.0 */
  l = 0.0;
  
  if (!(atof_digit[c]))		/* first char is not a digit */
      goto radix;
 skipzero:
  (ct = 1);			/* scheduling directive */
  /*  */
  while (c == '0')		/* skip leading zeros */
      { 
      c = *s++;
      if (!(atof_digit[c]))	/* first sig char is not a digit */
	goto radix;
      }
  
  /* while in front of the decimal point */
  
 start:
  /* Multiply the previous value by 10 and add in the new digit */
  
  /* accumulate first 15 significant characters into variable h.
   * We know it can hold 15 decimal digits without losing a bit.
   * The following 15 are accumulated into variable l.
   * Any following digits must be skipped to get the string
   * pointer right, but don't influence the final value.
   *
   * we use for loops for the accumulation because the count
   * variable ct is held in the counter register, and it can
   * be updated for "free".
   */

  for (ct=1;ct<=15;ct++)
      {
      itmp = c & 0xf;
      c = *s++;
      h = ten * h + ch[itmp];
      if (!(atof_digit[c])) 
	  goto radix;
      }
  for (ct=16;ct<=30;ct++)
      {
      itmp = c & 0xf;
      c = *s++;
      dexpon = dexpon + 1.0;
      l = ten * l + ch[itmp];
      if (!(atof_digit[c])) goto radix;
      }

  /* NOTE:  Should this be re-coded skip ALL following
   *        characters? YES! (defect 102679).
   */

  for (       ;         ;      )    /* too many digits */
      {   
      c = *s++;
      dexpon = dexpon + 1.0;
      if (!(atof_digit[c])) 
	goto radix;
      }

 radix:
  /* so far, dexpon = 0.0 if ct < 16; otherwise dexpon = ct -15  */
  
  /* Here on radix character, e, E, n, N, i, I, or end of string 
   * If radix character, get the stuff that's after it.       
   * If radix character is NULL, set to "." per X/Open.       
   * The radix character is a one byte character.             
   */
  radix = nl_langinfo(RADIXCHAR);
  if ( (radix == NULL) || !(*radix) )
    radix = ".";
  (h == 0.0);			/* scheduling directive */
  ((ct + 1) <= 15);		/* scheduling directive */
  if ( c == *radix )
      {
      c = *s++;
      (c == '0');		/* scheduling directive */
      /* to the right of decimal point */
      if (h == 0.0)		/* when no sig digit to the left of '.' */
	  {      
	  while (c == '0')		/* skip leading zeros and */
	      {    
	      dexpon = dexpon - 1.0;   /* decrement the exponent */
	      c = *s++; 
	      }
	  }
      ( ptr1 == (s-2) );	/* scheduling directive */
      if (!(atof_digit[c])) 
	  {
	  /* no digits before and after radix char is found */
	  if (ptr1 == (s-2)) goto invalid; 
	  else goto end_mantissa;
	  }

      /* start of sig digits after radix char */
      /* Depends on ct variable still being valid! */
      for (ct=ct+1;ct<=15;ct++) 
	  {
	  itmp = c & 0xf;
	  c = *s++;
	  h = ten * h + ch[itmp];
	  dexpon = dexpon - 1.0;   
	  if (!(atof_digit[c]))
	      goto end_mantissa;
	  }
      for (ct;ct<=30;ct++)
	  {
	  itmp = c & 0xf;
	  c = *s++;
	  l = ten * l + ch[itmp];
	  if (!(atof_digit[c])) 
	    goto end_mantissa;
	  }

  /* NOTE:  Should this be re-coded skip ALL following
   *        characters? YES! (defect 102679).
   */
      for (       ;        ;      )
	  {   /* too many digits */
	  c = *s++;
	  if (!(atof_digit[c])) 
	    goto end_mantissa;
	  }
      }  /* end if */
 end_mantissa:
#ifdef _ATOF_DEBUG
  printf("h=%.17g=%08x_%08x, l=%.17g=%08x_%08x, ct=%i\n", h,h, l,l, ct);
#endif
  ((s-1) == ptr1);		/* scheduling directive */
  flag = ((c == 'e') || (c == 'E')) ; /* scheduling directive */
  
  /* If we are still at 1st char, look for the IEEE special strings. */
  if ( (s-1) == ptr1 )
    goto special;
  
  /* Get the exponent if there is one 
   * flag is true if we found 'e' or 'E' in the right
   * place to begin an exponent
   */
  if (flag)			/* c == 'e' or 'E' */
      { 
	c = *s++;		/* advance past 'e' or 'E' */
	(c == '-');		/* scheduling directive */
	(c == '+');		/* scheduling directive */
  
      Eptr = s-2;  /* mark exp string start in case exp is invalid */

      switch (c) 
	  {
	case '-': 
	  expsign = -expsign;
	case '+': 
	  c = *s++;  /* 1st digit in exp string */
	  }
#if STRTOD
      Eptr1 = s; /* mark the position next to c in (e|E)[+|-]c...  */
#endif
      /* obtain exponent */
      while (atof_digit[c])
	  {
	  expwork = expwork * ten +  ch[c & 0xf];
	  c = *s++;
	  }
      dexpon = dexpon + expwork * expsign;
#if STRTOD
      /* If s is not changed since we recorded Eptr1
       * then there is no digit found next to (e|E)[+|-]
       * This means we don't have valid exponent and
       * endptr should point to e or E 
       */ 
      if (Eptr1 == s) *endptr = Eptr; /* for invalid exponent */ 
      else *endptr = s - 1; 
#endif
	}
  else
      {
#if STRTOD
      *endptr = s - 1;
#endif
      }
  /*  no more character will be read beyond this point */
  
  (h == 0.0);			/* scheduling directive */
  (dexpon == 0.0);		/* scheduling directive */
  (dexpon > 350.0);
  (dexpon < -350.0);

  /* since our power table goes up to 63, we will have to
   * handle our number differently depending on if the magnitude
   * of the exponent is greater than 63.  We compute the flag
   * here so it'll be ready when we need it.
   */
  xflag = dexpon*dexpon - 3969.0; /* xflag < 0 if |dexpon| < 63 */

  (double *)atof_pow1;		/* scheduling directive */
  (xflag > 0.0);		/* scheduling directive */
  ux.x = dexpon;
  uitmp = (ux.i[0] & 0x80000000); /* will be zero for positive exponent,
				   * otherwise 0x80000000
				   */

  /* if the value accumulated was zero return now */
  if ( h == 0.0)
      {
      __setflm(saverm);
      return(sign * 0.0);
      }
#ifdef _ATOF_DEBUG
  printf("dexpon=%g, h=%.17g=%08x_%08x, l=%.17g=%08x_%08x, ct=%i\n", dexpon, h,h, l,l, ct);
#endif
  
  /* if exponent is zero, then our value is obtained by
   * converting the quad precision accumulation to a double
   * or single, depending on what we're supposed to return.
   */
  
   if ( dexpon == 0.0) 
      { 
      
	/* if we've accumulated into the lower double precision value
	 * we need to normalized that value to the high value.  We
	 * use a power of ten table for this, and we generate an index
	 * into this table based on how many digits we accumulated into
	 * the lower value.  Note that if we didn't accumulate anything
	 * into the lower value the power of ten will be bogus, but it
	 * doesn't matter because we multiply it by l, which is zero.
	 */

      itmp = __abs(ct - 15);
      xtmp = negpow[itmp];

#if (ATOFF | STRTOF)
      xhi = h;
      xlo = l * xtmp;
      goto finish;
#else
      /* careful!  you have to multiply sign in BEFORE doing last operation
       * to get rounding right in round to minus infinity or positive infinity!
       */
      h *= sign;			/* update sign for each side of addition */
      l *= sign;			/* update sign for each side of addition */
      __setflm(saverm); 	/* restore caller's round mode. */
      return(h + l*xtmp);  /* fast return for regular argument */
#endif
      }

/* if here, a non-zero value and a non-zero exponent 
 * The algorithm below for converting dexpon to int
 * only works for |dexpon| < 1000.  The following
 * test will branch to infinity handling if exponent will
 * cause overflow 
 *
 * The low part has NOT been normalized w.r.t. the hi part
 * (so we still have exact values).  The return value is:
 * for 15 or less digits:
 *   sign*( h*10**dexpon )
 * for 16 to 30 digits:
 *   sign*( h*10**dexpon + l*10**(dexpon-(ct-15)) )
 */ 
 
  if (dexpon > 350.0)		/* overflow? */
      {
      xhi = (*((double *) (_DBLINF)));
      xlo = 0.0;
#if ( ATOFF | STRTOF)
      goto finish;
#else
      goto overflow;
#endif
      }

  if (dexpon < -350.0)		/* underflow? */
      {
      xhi = 0.0;
      xlo = 0.0;
#if ( ATOFF | STRTOF)
      goto finish;
#else
      goto overflow;
#endif
      }

  /* fast algorithm to convert double |dexpon|  to int expon 
   * this algorithm is valid for IEEE machine only
   * NB result always positive
   */
  itmp = (ux.i[0] & 0x7fffffff) >> 20;
  itmp2 = ((ux.i[0] & 0x000fffff) | 0x00100000);
  expon = itmp2 >> (1043 - itmp);
  /* end of converting double dexpon to int *expon  */

  i = ( (expon & 0x3f) << 1);	/* table index -- positive  */
  i |= uitmp >> 24;		/* move index down to negative part of table
				 * if necessary.
				 */
  
  py = (double *)atof_pow1+i;	/* construct pointer to correct table entry */
  
  yhi = *py;			/* power of ten -- high part of a quad */
  ylo = *(py+1);		/* power of ten -- low  part of a quad */
  
  if( ct > 15 ){		/* l has a value */
      if( (uitmp==0) && (expon == (ct-15)) ){
	  /* 
	   * string we are converting is exactly an integer
	   */
	  xlo = l;
      }else{
	  itmp = (ct - 15);	/* figure out how much to normalize */
	  xtmp = negpow[itmp];
	  l *= xtmp;		/* normalize 'l' w.r.t. 'h' */
	  xlo = l * yhi + h * ylo;
      }
  }else{			/* ct <= 15; means l is zero  */
      xlo = h * ylo;
  }

  /* see comments about xflag above. */
  
  if (xflag > 0.0)		/* |expon| > 63 */
      {
      /* calculate the number of times 10^64 must be 
       * multiplied into the result.  This could be done
       * either with a table or a loop
       */
      i = (expon & 0xffffffc0);
      i = i >> 6;

      /* below we will multiply the exponent in increments of
       * 10^64.  Here the leftover part is done.
       */

      xhi = h * yhi + xlo;
      xlo = xlo - (xhi - h * yhi);

      if ( dexpon >= 0.0) 
	py2 = (double *)atof_pospow2+2;
      else 
	py2 = (double *)atof_negpow2+2;

      /* construct the power of ten needed in increments
       * of 10^64.  This avoids use of table but costs
       * a few more cycles.
       */


      for (j=1; j<=i; j++)
	  {
	  yhi = *py2;
	  ylo = *(py2+1);
	  h = xhi;
	  l = xlo;

	  /* check if overflow */
	  if ( (h * yhi) == (*((double *) (_DBLINF))) )
	      {
	      xhi = (*((double *) (_DBLINF)));
	      xlo = 0.0;
#if (ATOFF | STRTOF)
	      goto finish;
#else
	      goto overflow;
#endif
	      }
	  else
	      {
	      xlo = l * yhi + h * ylo;
	      xhi = h * yhi + xlo;
	      xlo = (h * yhi - xhi) + xlo;

	      /* did we underflow? */
	      if (((xhi) == 0.0) && (h!=0.0))
		errno = ERANGE;
	      }
	  }
#if (ATOFF | STRTOF)
      goto finish;
#else
      (xhi != DBL_INFINITY );	/* scheduling directive */
      __setflm(saverm); 	/* restore caller's round mode. */
      if ( xhi != DBL_INFINITY )
	  {
	  xhi = xhi + xlo;
	  dblrslt = sign*xhi;
	  return (dblrslt);  /* return for expon > 63 but not overflow */
	  }
      else 
	  {
	  xhi = DBL_INFINITY;
	  xlo = 0.0;
	  goto overflow;
	  }
#endif
      }
#if (ATOFF | STRTOF)
  xhi = h * yhi + xlo;
  xlo = (h * yhi - xhi) + xlo;
  goto finish;
#else
/* careful!  you have to multiply sign in BEFORE doing last operation
 * to get rounding right in round to minus infinity or positive infinity!
 */
  h *= sign;			/* update sign for each side of addition */
  xlo *= sign;			/* update sign for each side of addition */
#ifdef _ATOF_DEBUG
  printf("h=%.17g=%08x_%08x, xlo=%.17g=%08x_%08x, yhi=%.17g=%08x_%08x\n", h,h, xlo,xlo, yhi,yhi);
#endif

  __setflm(saverm);		/* return to user's rounding mode */
  dblrslt = h * yhi + xlo;	/* final operation in user's rounding mode. */
				/* careful -- depends on only one round off error */
  return (dblrslt);		/* return for regular arguments */

#endif
  
 special:
  /* handling special char strings */
  switch (c) 
      {
    case 'n': 
    case 'N': 
      nanret = checknan(ptr1);
      if (nanret == 1) 
	  {		/* NaNQ */
#if STRTOD
	  *endptr = ptr1 + 4;
#endif
	  __setflm(saverm);
#if (STRTOF | ATOFF)
	  /* Should use floats in this expression.
	   * (float)(double)flt may cause a different
	   * value to be returned, instead of just flt.
	   * This also avoids a call to copysign.
	   */
	  if( sign >= 0.0 )
	      fltrslt =  FLT_QNAN;
	  else
	      fltrslt = -FLT_QNAN;
	  return( fltrslt );
#else
	  return(copysign (DBL_QNAN, sign));
#endif
	  }
      else if (nanret == 2) 
	  {		/* NaNS */
#if STRTOD
	  *endptr = ptr1 + 4;
#endif
	  __setflm(saverm);
#if (STRTOF | ATOFF)
	  /* Must use floats in this expression.
	   * (float)dbl  causes a frsp instruction
	   * to be generated, which causes a double
	   * signaling NaN to be quieted as it is
	   * converted to single.
	   */
	  if( sign >= 0.0 )
	      fltrslt =  FLT_SNAN;
	  else
	      fltrslt = -FLT_SNAN;
	  return( fltrslt );
#else
	  return(copysign (DBL_SNAN, sign));
#endif
	  }
      else if (nanret == 3)
	  {		/* NaN */
#if STRTOD
	  *endptr = ptr1 + 3;
#endif
	  __setflm(saverm);
#if (STRTOF | ATOFF)
	  /* Must use floats in this expression.
	   * (float)dbl  causes a frsp instruction
	   * to be generated, which causes a double
	   * signaling NaN to be quieted as it is
	   * converted to single.
	   */
	  if( sign >= 0.0 )
	      fltrslt =  FLT_SNAN;
	  else
	      fltrslt = -FLT_SNAN;
	  return( fltrslt );
#else
	  return(copysign (DBL_SNAN, sign));
#endif
	  }
      break;
    case 'i': 
    case 'I': 
      if ((ret = checknf(ptr1)) != 0)
	  {
#if STRTOD
	  *endptr = ptr1 + ret;
#endif
	  __setflm(saverm);
#if (STRTOF | ATOFF)
	  return((float)(sign * DBL_INFINITY));
#else
	  return(sign * DBL_INFINITY);
#endif
	  }
      break;
      }
invalid:
  /* invalid string returns here  */
#if STRTOD
  *endptr = (char *) nptr;
#endif
  __setflm(saverm); 	/* restore caller's round mode. */
  errno = EINVAL;	/* as per X/Open XPG4 */
  return(0.0);
  
#if (ATOFF | STRTOF)
  /*
   * all single precision result returns here except 
   * unrecognized string, special strings (NaN, Inf etc).
   * This part is not optimized for performance.
   * We only get to finish in the case where we need to
   * return a single precision number.
   */

 finish:

  x.rslt[0] = xhi;
  x.rslt[1] = xlo;
  
  /* Round to single for atoff or strtof -- with only 1 rounding err. */
  /* If the low fp pair is zero then the high is exact and can be     */
  /* rounded to float as is. If the low fp pair is positive then it   */
  /* is just sticky bits for the high. If low is negative then the    */
  /* high has been rounded up and we need to "unround" it.            */


  /* we will ON PURPOSE restore the caller' fpscr here and again
   * below.  Here we want to get the correct rounding mode for this
   * operation to work; be we will restore it again below so to
   * get sticky bits right
   */

  __setflm(saverm); 	/* restore caller's round mode. */
  if (x.rslt[1] != 0.0) 
    if (x.rslt[0] != (x.rslt[0] + x.rslt[1] ))
      {   /* if some sticky bits */
      if (x.rslt[1] < 0.0) 
	  {
	  __setrnd(1);
	  x.rslt[0] += x.rslt[1]; /* unrounds the upper double */
	  }
      x.i[1] |= 0x01;       /* set sticky bit in hi part */
      }

  /* Restore caller's round mode for last operation  */
  __setflm(saverm); 	/* restore caller's round mode. */
  fltrslt = (float) (sign*x.rslt[0]);        /* round to float result */
  if ((fltrslt == 0.0) || 
      (fltrslt == FLT_INFINITY) ||
      (fltrslt == -FLT_INFINITY) ) 
      {
      if (fltrslt == 0.0)
	fp_set_flag (FP_INEXACT | FP_UNDERFLOW);
      else
	fp_set_flag (FP_INEXACT | FP_OVERFLOW);
      }
  if ((fltrslt == FLT_INFINITY) || 
      (fltrslt == -FLT_INFINITY))
    errno = ERANGE;
  else if ((fltrslt == 0.0) && (xhi != 0.0) ) 
    errno = ERANGE;
  else if (fltrslt != fltrslt) 
      {
      if ( sign == 1.0) fltrslt = FLT_INFINITY;
      else fltrslt = -FLT_INFINITY;
      errno = ERANGE;
      }
  
  return (fltrslt);
  
#else
 overflow:
  /* handle the overflow case */
  
  x.rslt[0] = xhi;
  x.rslt[1] = xlo;
  /*  Round to double.                                                 */
  /*  Add low part to high part (usually this only affects the low     */
  /*  order bits of the high part) using the caller's round mode.      */
  
  /* Restore caller's round mode for last operation  */
  __setflm(saverm); 	/* restore caller's round mode. */
  dblrslt = sign*x.rslt[0] + sign*x.rslt[1];
  if ((dblrslt == 0.0) || 
      (dblrslt == DBL_INFINITY) ||
      (dblrslt == -DBL_INFINITY) ) 
      {
      if (dblrslt == 0.0)
	fp_set_flag (FP_INEXACT | FP_UNDERFLOW);
      else
	fp_set_flag (FP_INEXACT | FP_OVERFLOW);
      errno = ERANGE;
      }
  if (dblrslt != dblrslt)
      {
      errno = ERANGE;
      dblrslt = copysign (HUGE_VAL, dblrslt);
      }
  return (dblrslt);
  
#endif
  }

