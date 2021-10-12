static char sccsid[] = "@(#)50	1.4  src/bos/usr/ccs/lib/libc/POWER/wcdsto2fp.c, libccnv, bos411, 9428A410j 9/7/93 09:08:25";
/*
 * COMPONENT_NAME: LIBCCNV wcdsto2fp
 *
 * FUNCTIONS: wcdsto2fp, wcchecknf, wcchecknan
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

#include <float.h>
#include <ctype.h>
#include <math.h>
#ifdef exp
#undef exp
#endif
#include <langinfo.h>		/* nl_langinfo, RADIXCHAR */
#include <wchar.h>
#include "atof.h"

/*
 *      Macro to multiply the current pair of fp numbers by 10.0 and
 *      add the new character. Only works on RIOS hardware or simulator.
 */
#define ACCUM d = ch[c & 0xf];\
  t = ten * h + d;\
  h = (ten * h - t) + d;\
  l = ten * l + h;\
  h = t

static double ch[10] = {
	0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

int wc_checknf(wchar_t *);
int wc_checknan(wchar_t *);

/*
 * NAME: dsto2fp
 *                                                                    
 * FUNCTION: convert decimal string to 2 fp numbers (hi part & low part)
 *                                                                    
 * NOTES:
 *
 * dsto2fp - convert decimal string to 2 fp numbers (hi part & low part)
 *
 *      Called with p -     pntr to ascii string
 *                  dbl -   pntr to array 2 doubles.
 *                  expon - pntr to the exponent returned.
 *                  class - pointer to result class
 *                          The class value is the same as returned by
 *                          the class() function except norms and denorms
 *                          both return FP_PLUS_NORM.
 *
 *      Returns pointer to character that terminates scan.
 *
 *      A valid decimal string has the following form:
 *
 *      ( {<digit>} [<radix>{<digit>}] [(e|E) [+|-|<sp>] {<digit>} ] |
 *      "NaNS" | "NaNQ" | "INF" | "NaN" | "INFINITY" )
 *
 *      {} = 0 or more times
 *      [] = 0 or 1 times
 *      radix = locale dependent radix character.
 *
 *
 * RETURNS: 
 *      Returns pointer to character that terminates scan.
 */
	#define RAD_SIZE 5

wchar_t *wcdsto2fp (wchar_t *p, double dbl[2], int *expon, int *class)
{
	wchar_t   *s = p;	/* current character pointer                  */
	wchar_t *orig = p;	/* save original pointer in case invalid string  */
	wchar_t   *Eptr;	/* start of exponent string                   */
	char   *radix;		/* radix character from locale info           */
	wchar_t   c;		/* current character holder                   */
	double d, h, l, t;	/* temps used to calculate the double results */
	double ten = 10.0;	/* constant 10.0                              */
	int    expsign = 1;	/* sign of exponent (+1 = '+'; -1 = '-')      */
	int    ct = 0;		/* number of significant digits collected     */
	unsigned long exp = 0;  /* holds the converted exponent               */
	int    nanret, ret = 0;
	int digits = 0;		/* number digits digested                     */
	int rad_size;
	wchar_t wcradix[RAD_SIZE + 1];

	/* start of "executable" code */
	Eptr = (void *) NULL;	/* start out with Eptr = NULL                 */
	*expon = 0;		/* start out with exponent = 0                */
	*class = FP_PLUS_NORM;	/* assume number will be normalized fp       */

	h = 0.0;		/* init the high & low part of result to +0.0 */
	l = 0.0;

	c = *s++;		/* get the first character */
	while (iswdigit(c))	/* while in front of the decimal point */
	{
         	digits++;
	        /* Multiply the previous value by 10 and add in the new digit */
		if (ct < 30) { 
			ACCUM; 
		} 
		else 
			(*expon)++;
		if ((c != '0') || ct ) 
			ct++;
		c = *s++;
	}
	/* Here on radix character, e, E, Q, S, I, or end of string */
	/* If radix character, get the stuff that's after it.       */
	/* If radix character is NULL, set to "." per X/Open.       */
	radix = nl_langinfo(RADIXCHAR);

	if( (radix == 0) || !(*radix) )
		radix = ".";
	rad_size = mbstowcs(wcradix, radix, RAD_SIZE + 1);
	s--;			/* back up for string compare */
	if ((rad_size <= RAD_SIZE) && !wcsncmp(wcradix, s, rad_size)) {
		s += rad_size;
		c = *s++;
		while ( iswdigit(c) ) {
		        digits++;
                	/* Multiply the previous value by 10 and add 
			 * in the new digit  */
			if (ct < 30) {
				ACCUM;
				(*expon)--;    /* decrement the exponent   */
			}
			if ((c != '0') || ct ) 
				ct++;
			c = *s++;
		}  /* end while */
	}
	else { 
	s++;  /* not a radix character; restore state of pointer */
	} /* end if */
	/* Get the exponent if there is one */
	if ( ((c == 'e') || (c == 'E')) && ( p != s-1)) {
		Eptr = s-1;      /* mark string start in case exp is invalid */
		switch (c = *s++) {
		case '-': 
			expsign = -1;
		case '+': 
			c = *s++;
		}

/*
 * accumulate exponent.  We can possible get a series of digits
 * which would cause a long to overflow, so we want to stop
 * accumulating when the value gets to be larger than anything
 * that won't cause an overflow in the floating point number
 */
		while ( iswdigit(c) ) {
			if (exp < 1000)
				exp = (exp*10) + (c & 0xf);
			c = *s++;
			Eptr = s-1;  /* mark string start when exponent is invalid */
		}
	}

	*expon += exp * expsign;

	/* If we are still at 1st char, look for the IEEE special strings. */
	if ( (s-1) == p) {
		switch (c) {
		case 'n': 
		case 'N': 
			nanret = wc_checknan(p);
			if (nanret == 1) {		/* NaNQ */
				h = DBL_QNAN;
				*class = FP_QNAN;
				digits++;
				s +=4; 
			}
			else if (nanret == 2) {		/* NaNS */
				h = DBL_SNAN;
				*class = FP_SNAN;
				digits++;
				s +=4; 
			}
			else if (nanret == 3) {		/* NaN */
				h = DBL_SNAN;
				*class = FP_SNAN;
				digits++;
				s +=3; 
			}
			break;
		case 'i': 
		case 'I': 
			if ((ret = wc_checknf(p)) != 0) {
				h = DBL_INFINITY;
				*class = FP_PLUS_INF;
				s += ret; 
				digits++;
		}
			break;
		}
	}

	if (h == 0.0) *class = FP_PLUS_ZERO;  /* if hi==0 then whole nbr == 0 */

	dbl[0] = h;           /* Result = h and l   */
	dbl[1] = l;           /* Return in memory */

	if (digits)
	    {
	    if (Eptr == (void *) NULL)
	      return (s-1);   /* Return ptr to char that ended scan */
	    return (Eptr);		/* Return ptr from bad convert of exponent */
	    }
	else
            {
            *class = FP_BAD_STRING;
	    return(orig);
            }
	
}

/*
 * NAME: wc_checknf
 *                                                                    
 * FUNCTION: check to see in string is infinity
 *                                                                    
 * NOTES:
 *	The check for infinity is case insensitive
 *
 * RETURNS: 3 for inf string;
 *          8 for infinity string;
 *          0 otherwise.
 *
 */

int wc_checknf(register wchar_t *bp)
{
	bp++;
	if ((*bp == 'n') || (*bp == 'N'))
	{   bp++;
	    if ((*bp == 'f') || (*bp == 'F'))
	    {   
	    /* if here, we have inf.  Scan some more to check for infinity. */
		if (
			((*++bp == 'i') || (*bp == 'I')) &&
			((*++bp == 'n') || (*bp == 'N')) &&
			((*++bp == 'i') || (*bp == 'I')) &&
			((*++bp == 't') || (*bp == 'T')) &&
			((*++bp == 'y') || (*bp == 'Y'))
	       	    )
			return (8);
		else 	return (3);
	    }
	}

	return (0);
}

/*
 * NAME: wc_checknan
 *                                                                    
 * FUNCTION: check to see in string is NaNQ or NaNS
 *                                                                    
 * NOTES:
 *	The check for NaN is case insensitive
 *
 * RETURNS: 1 for NaNQ string;
 *          2 for NaNS string;
 *          3 for NaN  string:
 *          0 otherwise
 *
 */
int wc_checknan(register wchar_t *bp)
{
	bp++;
	if ((*bp == 'a') || (*bp == 'A'))
	{	bp++;
		if ((*bp == 'n') || (*bp == 'N'))
		{	bp++;
		/* if here we have a NaN; now just classify it */
			if ((*bp == 'q') || (*bp == 'Q')) {
				return (1);		/* NaNQ */
			}
			else if ((*bp == 's') || (*bp == 'S')) {
				return (2);		/* explicit NaNS */
			}
			else return (3);		/* implicit NaNS */
		}
	}

	return(0);
}
