static char sccsid[] = "@(#)51	1.2  src/bos/usr/ccs/lib/libc/POWER/wcstod.c, libccnv, bos411, 9428A410j 7/11/91 17:25:27";
/*
 * COMPONENT_NAME: (LIBCCNV) wcstod
 *
 * FUNCTIONS: wcstod
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

#include <float.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <fpxcp.h>
#include <stdlib.h>
#define TRUE  1
#define FALSE 0

/*
 * ***********************************************************************
 *
 *      Macro m2dbls
 *
 *      Macro to multiply x.rslt[0] and x.rslt[1] by a pair of doubles
 *      from the power of ten tables. This macro only works on RIOS!
 *      Each pair of fp doubles represents one real number. The high fp
 *      number of each pair contains the high order bits of the real number
 *      and the low fp number contains the low order bits. Thus each pair
 *      contains 106 bits of fraction. The algorithm to multiply these
 *      two numbers (e.g., x = x * y) is:
 *
 *      hi = yhi * xhi
 *      if (a != infinity)
 *         al = yhi * xhi - hi
 *         b  = yhi * xlo
 *         xlo = al + (b + ylo * xhi)
 *         xhi = hi + xlo
 *         xlo = (hi -xhi) + xlo
 *      else
 *         xhi = infinity
 *         xlo = 0.0
 *
 *    NOTE: for this macro to works hi, b, al, xhi, yhi, ylo, xlo must
 *          be doubles and py is a pointer to double.
 *
 *    Macro assumes y points to a double dimensioned array of 2 doubles
 *    where i is the second dimension. The "fixes" below for infinity
 *    have not been tested when both args being multiplied are near
 *    infinity since in atof at least one of the arguments is much
 *    smaller than infinity.
 *
 */

#define M2DBLS(y,i) py = ((double *) y+i); yhi = *py; xhi = *x.rslt; \
xlo = *(x.rslt+1); ylo = *(py+1); \
if ((hi = yhi * xhi) == DBL_INFINITY) hi = DBL_MAX; \
al  = yhi * xhi - hi; \
b      = yhi * xlo; \
xlo    = al+(b+(ylo * xhi)); \
if ((*x.rslt = hi + xlo) == DBL_INFINITY) *x.rslt = DBL_MAX; \
*(x.rslt+1) = (hi - *x.rslt) + xlo;  \


/*
 *
 *  Globals variables for all routines
 *
 */

#include "atof.h"

extern wchar_t *wcdsto2fp (const wchar_t *, double *, int *, int *);

/*
 * NAME: wcstod
 *                                                                    
 * FUNCTION: convert a wide character string to double
 *
 * double wcstod (const wchar_t * pwcs, whcar_t ** endptr)
 * 
 * The function wcstod() parses the wide character string pointed to by
 * pwcs to obtain a valid string for the purpose of conversion to a
 * double and returns the converted double value.  It point the endptr to
 * the position where an unrecognized character including the wide
 * character null terminator, is found.
 * 
 * Any character that is deemed as a space character by the function:
 * isspace() in the beginning of the wide character string pointed to by
 * pwcs will be skipped.  The first character that is not a space
 * character begins the search for the actual subject character.  the
 * subject character should be in the format of a floating point constant
 * (as per the ANSI C specification) without the floating suffix.  The
 * subject string is expected to be an optional wide character plus or
 * minus sign, then a nonempty sequence of wide character digits
 * optionally containing a decimal-point character then an optional
 * exponent part as defined in ANSI C, but no floating suffix.  The
 * subject sequence is defined as the longest initial substring of the
 * form required by a floating-point constant without the floating-point
 * suffix.  Any character that is not recognized as part of the floating
 * point constant as described above including the terminating null
 * character begins the endptr portion of the string.
 * 
 * If the subject sequence has the expected form, it is interpreted as a
 * floating point constant.  A pointer to the final string is stored in
 * endptr, if the endptr is not a null pointer.  If the subject sequence
 * is empty or does not have a valid form, no conversion is done, the
 * value of pwcs is stored in endptr, if endptr is not a null pointer.
 *                                                                    
 * RETURNS:
 *
 * Returns the converted value of double if a valid floating point
 * constant is found.
 * 
 * If no conversion could be performed, zero is returned.
 * 
 * If the converted value is outside the range (either too high or too
 * long), the variable errno is set to ERANGE.  In case of overflow, plus
 * or minus HUGE_VAL is returned.  In case of underflow, zero is returned. 
 */

double wcstod (const wchar_t *nptr, wchar_t ** endptr)
  {
  wchar_t ch;			/* Current character                    */
  wchar_t *orig;                /* save string pointer in case invalid  */
  double saverm;                /* Caller's round mode.                 */
  double temprm;                /* Saved mode for multiply */
  int exponent;                 /* Decimal exp. returned from dsto2fp   */
  int class;                    /* Result class type returned from " "  */
  unsigned int  i;              /* temp array index          */
  union { 
  double rslt[2];		/* result: hi and low parts  */
  long int i[4];
        } x;
  double sign = 1.0;		/* +1.0 = '+', -1.0 = '-' ; assume +   */
  double xhi,xlo,yhi,ylo;	/* needed by double mult macro  */
  double hi, al, b;		/* needed by double mult macro  */
  double *py;			/* needed by double mult macro  */
  wchar_t *valid_addr;		/* a valid address for endptr to point */
  double dblrslt;
  float  fltrslt;
  
  /* If endptr points to a null then point it to a valid address so */
  /* we don't have to worry about it later.                         */
  if ( endptr == (wchar_t **)0) endptr = &valid_addr;

  /* save the starting pointer in case we need it back */
  orig = (wchar_t *) nptr;
  
  saverm = __setflm(0.0);        /* set round mode=RN,save mode */
  
  while (iswspace(ch = *nptr) ) nptr++;  /* skip leading whitespace */
  
  switch (ch)
      {                     /* Look for + or - sign            */
    case '-': 
      sign = -sign;        /* set sign to -1.0, fall thru     */
    case '+': 
      nptr++;              /* bump nptr to skip + or - sign   */
      }

  /*
   * dsto2fp reads in the rest of the string and creates two RIOS double
   * fp values to represent the base string. It also returns an
   * exponent (base 10) to be used to scale the base string.
   * The return value is a pointer to the character that ended the
   * scan ( i.e., the character after the string if everything went
   * okay.) If the first character was not valid then it returns nptr.
   */

    *endptr = wcdsto2fp(nptr, x.rslt, &exponent, &class);
  
  /* Look for all the special numbers dsto2fp could have returned */

  switch (class) 
      {
    case FP_BAD_STRING:
      *endptr = orig;
      __setflm(saverm);
      return(0.0);
    case FP_PLUS_ZERO:
    case FP_PLUS_INF:
      __setflm(saverm);
      return(sign * x.rslt[0]);
    case FP_QNAN:
    case FP_SNAN:
      __setflm(saverm);
      return(copysign (x.rslt[0], sign));
      }

  /*
   *  Multiply the pair of fp numbers returned by dsto2fp by the power
   *  of 10 (returned in exponent). If  -63 < exponent < 63 then
   *  only one multiply is needed. If exponent > 63 or < -63 and
   *  > -320 then only 2 multiplies will be needed. Three multiplies
   *  are only needed when the exponent < -320.
   *
   *  We don't get here unless the pair of fp numbers is finite and
   *  non-zero.  If the result of the multiplies is infinity
   *  then an overflow occurred and we should set errno = ERANGE
   *  if strtod or strtof. If the result of the multiplies is a zero
   *  then underflow occurred and we should also set errno = ERANGE for
   *  strtod or strtof.
   *
   */

  temprm = __readflm();		/* save mode */
  if (exponent >= 0)		/* positive exponent */
      {  
      if (exponent > 319)
	exponent = 319;
      i = ((exponent&0x3f) << 1);
      M2DBLS(__pospow1,i);
      if (i = ((exponent&0x3c0) >> 5))   /* Don't mult. if zero */
	  { 
	  M2DBLS(__pospow2,i); 
	  }
      }
  else				/* negative exponent */
      {  
      if (exponent < -383) 
	exponent = 383; 
      else 
	exponent = -exponent;
      i = ((exponent&0x3f) << 1);
      M2DBLS(__negpow1,i);
      if (i = ((exponent&0x3c0) >> 5)) /* Don't mult if zero  */
	  { 
	  if ( i > 8 )		/* if exp < -256       */
	      {
	      M2DBLS(__negpow2,8);  /* mult by -256 first  */
	      i -= 8;             /* then by what's left */
	      }
	  M2DBLS(__negpow2,i);
	  }
      }
  __setflm(temprm);        /* restore mode */

  /*  Round to double.                                                 */
  /*  Add low part to high part (usually this only affects the low     */
  /*  order bits of the high part) using the caller's round mode.      */
  
  /* Restore caller's round mode for last operation  */
  __setflm(saverm); 	/* restore caller's round mode. */
  dblrslt = sign*x.rslt[0] + sign*x.rslt[1];
  if ((dblrslt == 0.0)          || 
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
  }
