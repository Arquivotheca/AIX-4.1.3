#if (!defined(ATOL)) && (!defined(LIBCSYS))
static char sccsid[] = "@(#)39	1.6  src/bos/usr/ccs/lib/libc/POWER/atoi.c, libccnv, bos411, 9428A410j 4/29/94 10:34:47";
#endif
/*
 * COMPONENT_NAME: LIBCCNV 
 *
 * FUNCTIONS: atoi, atol
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
 *
 */

#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include "atoi_local.h"

#ifdef ATOL
#define MAX  LONG_MAX
#define MIN  LONG_MIN
#else /* must be ATOI */
#define MAX  INT_MAX
#define MIN  INT_MIN
#endif

#define MAXDIV10 (MAX / 10)
#define MAXMOD10 (MAX % 10)
#define MINDIV10 (MIN / 10)
#define MINMOD10 (MIN % 10)

extern int __ctype[];

#ifdef ATOL
long int atol(const char *nptr)
#else /* must be ATOI */
int atoi(const char *nptr)
#endif

  {
#ifdef ATOL
  typedef long int int_type; 
#else /* must be ATOI */
  typedef int int_type; 
#endif
  int sign;			/* +1 = '+', -1 = '-'   */
  char   *s;			/* current character pointer  */
  char   *ptr1;			/* pointer to the begining of string after    */
				/* skipping spaces and sign */
  char   c,c1;			/* character holder                   */
  int_type rslt;		/* accumalated result */
  int_type cmp_rslt;		/* to compare with result when near overflow */
  int iter;			/* max number of characters less 3 in this base */
  int base = 10;
  int    ct;			/* number of significant digits collected     */
  int    itmp,itmp2;		/* temporary working integer */
  int maxbit,flag,flag2;
  int digits = 0;               /* non-zero if real digits have been digested */

   /* we will process 10 digits, however, two of them
    * and one of them are processed before and after
    * the for loop below 
    */
  iter = 7;  
  s = (char *) nptr;		/* initialize the current pointer */
  c1 = *s++;			/* 1st char in subject string */
  flag2 = (c1 == '0');		/* scheduling help */
  sign = 1;			/* +1 = '+', -1 = '-'   */

  while (c1 == '0')		/* skip leading zero(s) */
      { 
      c1 = *s++;
      digits++;
      }
  flag = __ctype[c1] - base;
  rslt = __ctype[c1];

  if (flag >= 0)                        /* all zero sequence  */
      {
      if (digits) return(0);            /* RETURN POINT */
      }

  c = *s++;
  itmp = __ctype[c];
  flag2 = (itmp >= base); 
  if (flag < 0)
      {
      if (itmp >= base) return(rslt); /* RETURN POINT */
      c = *s++;          
      goto start;
      }
        
  s = (char *) nptr;

  /* check for whitespace and sign character */
  for(;;s++)
      {
      switch(*s)
	  {
	case ' ':
	case '\t':
	case '\f':
	case '\n':
	case '\v':
	case '\r':
	  continue;
	  
	case '-':
	  sign = -1;
	case '+':
	  *s++;
	  }
      break;
      }
  
  c1 = *s++;			/* 1st char in subject string */
  c = c1;
  while (c == '0')		/* skip leading zero(s) */
      { 
      c = *s++;
      }
  rslt = __ctype[c];		/* initialize accumulation */
  flag = __ctype[c] - base;	/* compute for while in register */
  
  if (flag >= 0)			/* invalid character ?  */
      {
      /* at this point we've hit an invalid character, and the
       * only valid character we've possibly seen is 0.  If we've
       * seen a zero, return zero, else we did not have a valid
       * sequence in which case we need to set errno.
       */
      if (c1 == '0') return(0);	/* RETURN POINT */
      else goto invalid;	/* not a valid sequence */
      }
  
  c = *s++;			/* get next character */

  flag2 = __ctype[c] - base;	/* check 2nd digit if it is a digit  */
  itmp = __ctype[c];		/* keep it while its in a register */

  /* one digit string with optional sign returns here */
  if (flag2 >= 0) return(sign * rslt); /* RETURN POINT */

  c = *s++;			/* 3rd digit if it is a digit */

 start:
  /* Multiply the previous value by 10 and add in the new digit */
  /* using a for loop and returning out of it has a 
   * significant performance advantage, since it uses the
   * count register, and the branch on the count register is cheap
   */
  for(ct=0;ct<=iter;ct++) 
      {
      itmp2 = __ctype[c];
      flag2 = itmp2 - base;
      rslt = base * rslt + itmp;
      itmp = itmp2;
      if (flag2 >= 0) return(sign * rslt); /* RETURN POINT */
      c = *s++;
      }
  /* If we're here, the valid char string is at least 10 characters long
   * excluding possible leading zero's.  So now we have to start
   * looking for overflow.
   */

  /* First - if next character in string is a digit and overflow */
  if (__ctype[c] < base) 
    {
#ifndef LIBCSYS			/* functions in libcsys.a may not set errno */
    errno = ERANGE;
#endif /* LIBCSYS */
    if (sign == 1) return(MAX);	/* RETURN POINT */
    else return (MIN);		/* RETURN POINT */
    }

  /* check if number is larger than MAX divided by ten, or
   equal and digit greater than last digit of MAX */

  if (sign == 1) 
    {
    if (rslt > MAXDIV10 || (rslt == MAXDIV10 && itmp > MAXMOD10))
      {
#ifndef LIBCSYS			/* functions in libcsys.a may not set errno */
      errno = ERANGE;
#endif /* LIBCSYS */
      return(MAX);		/* RETURN POINT */
      }
    }
  else
    {
    rslt = -rslt;
    itmp = -itmp;
    if (rslt < MINDIV10 || (rslt == MINDIV10 && itmp < MINMOD10))
      {
#ifndef LIBCSYS			/* functions in libcsys.a may not set errno */
      errno = ERANGE;
#endif /* LIBCSYS */
      return(MIN);		/* RETURN POINT */
      }
    }

  /* here digit would not cause overflow, so put it in and return */

  return( rslt * base + itmp); 

  /* we reach this point in the case of any invalid sequence */
 invalid: 
  return(0);			/* RETURN POINT */
  }
