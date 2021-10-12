/* @(#)99	1.4  src/bos/usr/ccs/lib/libc/POWER/strto.c, libccnv, bos411, 9428A410j 6/30/94 15:23:26 */
/*
 * COMPONENT_NAME: LIBCCNV strto
 *
 * FUNCTIONS: strtol, strtoul, strtoll, strtoull,
 *            wcstol, wcstoul, wcstoll, wcstoull
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Note that this source file has a "header" style SCCS
 * line because it is never compiled directly; it is always
 * #include'd by another .c file.
 */

#include <limits.h>		/* LONG_MAX, LONG_MIN, ULONG_MAX,
				 * LONGLONG_MAX, LONGLONG_MIN,
				 * ULONGLONG_MAX */
#include <stddef.h>		/* NULL */
#include <errno.h>		/* errno */
#include <ctype.h>		/* isalnum, isspace, isdigit, tolower */
#include <wchar.h>		/* iswalnum, iswspace, iswdigit,
				 * towlower */


#if WIDECHAR == 0		/* normal narrow char */

#include "atoi_local.h"		/* SP */
extern const int __ctype[];
#define DIGIT(x) (__ctype[x])

#else				/* wide chars */

 /*
  * For any base > 10, the digits incrementally following 9 are assumed
  * to be "abc...z" or "ABC...Z". 
  */
/* This macro assumes ASCII (a-z are contiguous). Fails for EBCDIC */
#define DIGIT(x) \
(IS_DIGIT(x) ? ((int)((x)&0xf)) : (10+(int)(TO_LOWER(x)-CHAR('a'))))

#endif

#define MBASE 36		/* Max base allowed */

/*
 * NAME:
 *      strto
 *
 * FUNCTION:
 *      This code is included by:
 *            strtol, strtoul, strtoll, strtoull,
 *            wcstol, wcstoul, wcstoll, wcstoull
 *      to build those eight functions.  This way there is common code
 *      that just differs by the #defines.
 *
 *      Character type can be normal or wide.
 *      Sign type can be signed or unsigned.
 *      Int type can be long or long long.
 *      POS_OVERFLOW can be Utype_MAX or type_MAX.
 *      NEG_OVERFLOW can be Utype_MAX or type_MIN.
 *
 * NOTES:
 *
 * The function  strto() converts the initial  portion of the  (wide or
 * narrow) character string pointed to by  nptr to a (64-bit or 32-bit)
 * (signed or unsigned) integer representation.   To do this, it parses
 * the (wide or narrow) character string pointed to by nptr to obtain a
 * valid string (i.e., subject string) for the purpose of conversion to
 * a  (signed or  unsigned)  integer.   It  points  the  endptr to  the
 * position where  an unrecognized character including  the terminating
 * null is found.
 *
 * The base can take the following values:  0, 2 thru 9, a (or A) through
 * z (or Z).  This means that there can be potentially 36 values for
 * the base.  If the base is zero, the expected form of the subject
 * string is that of an signed integer constant as per the ANSI C
 * specifications with the option of a plus or minus sign, but not
 * including the integer suffix.  If the base value is between 2 and
 * 36, the expected form of the subject sequence is a sequence of
 * letters and digits representing an integer with the radix specified
 * by the base, optionally preceeded by a plus or minus sign, but not
 * including an integer suffix.  The letters a (or A) throught z (or Z)
 * are ascribed the values 10 to 35; only letters whose values are less
 * than that of the base are permitted.  If the value of base is 16,
 * the characters 0x or 0X may optionally precede the sequence of
 * letters or digits, following the sign, if present.
 *
 * If the subject sequence has the expected form, it is interpreted as
 * an integer constant in the appropriate base.  A pointer to the final
 * string is stored in endptr, if the endptr is not a null character.
 * If the subject sequence is empty or does not have a valid form, no
 * conversion is done, the value of nptr is stored in endptr, if endptr
 * is not a null pointer.
 *
 * RETURN VALUES:
 *
 * Returns the converted value if the expected form is found.
 *
 * If no conversion could be performed, zero is returned.
 *
 * If the converted value is outside the range of representable values,
 * POS_OVERFLOW or NEG_OVERFLOW is returned (according to the sign of
 * the value), and the value of the variable errno is set to ERANGE.
 */

/*
 * The maximum number of significant base-nn "digits" before overflow.
 * Each entry is ceil(MAX_BITS*ln(2)/ln(base)); MAX_BITS = 32 or 64.
 * Each entry is smallest n such that base**n >= 2**MAX_BITS.
 * The actual table is in atoi_table.c
 */
extern const int MAX_DIGITS[];

RET_SIGN RET_TYPE
FUNC(const CHAR_TYPE *nptr,
CHAR_TYPE ** endptr,
     int base)
{
    CHAR_TYPE *valid_endptr;	/* If endptr is NULL, then have endptr
				 * update this pointer.  This
				 * eliminates many tests for endptr
				 * being NULL. */
    unsigned RET_TYPE val;	/* The converted value, no sign. */
    unsigned RET_TYPE lastval;	/* 'val' before this digit. */
    long int num_digits;	/* Number of significant digits.  Does
				 * not count leading zeros.  Needed to
				 * see if number overflowed. */
    long int digits = 0;	/* Number of valid digits read. Leading
				 * zeros count as one valid digit, so
				 * that the case of LONG_MAX leading
				 * zeros does not overflow this var. */
    int xx;			/* digit current converting */
    int sign = 1;		/* -1 == '-', +1 == '+' sign on string */
    int overflag = 0;		/* 1 if ERANGE error (overflow). */
    int hex_prefix = 0;		/* 1 if a "0x" or "0X" prefix */
    unsigned RET_TYPE maxdiv;   /* greatest number such that base * maxdiv
				 * won't overflow */

    val = 0;			/* Default return value if any errors. */
    if (NULL == endptr) {	/* Make sure *endptr can be assigned */
	endptr = &valid_endptr;
    };
    /*
     * As a default, *endptr is updated to point to the start of the
     * input string. *endptr may be updated later to point to the first
     * invalid character. 
     */
    *endptr = (CHAR_TYPE *) nptr;
    if ((base < 0) || (base > MBASE) || (base == 1)) {	/* Is base valid ? */
	errno = EINVAL;		/* Bad base => Error INVALid base */
	return (val);
    };
    if (NULL == nptr) {		/* Have no string to process. */
	/*
	 * X/Open allows errno to be set to EINVAL, but previous AIX
	 * has not done this, so to maintain binary compatibility,
	 * errno is left alone. 
	 */
	return (val);
    };
    while (IS_SPACE(*nptr)) {	/* skip leading "spaces" */
	++nptr;
    };
    if (*nptr == CHAR('-')) {	/* Did we find a '-' sign? */
	++nptr;			/* Advance past sign. */
	sign = -1;		/* Indicate we found a '-' sign. */
    } else if (*nptr == CHAR('+')) {	/* Did we find a '+' sign? */
	++nptr;			/* Advance past sign. */
    };
    if (base == 0) {		/* Base 0 is a special case. */
	if (*nptr == CHAR('0')) {	/* 0nnnn => octal. 0x, 0X =>
					 * hex. */
	    ++nptr;		/* Advance past leading zero. */
	    if ((*nptr == CHAR('x')) || (*nptr == CHAR('X'))) {
		++nptr;		/* Advance past x char. */
		hex_prefix = 1;	/* Indicate 0x prefix. */
		base = 16;
	    } else {		/* No x => must be octal. */
		++digits;	/* Bump count of valid digits. */
		base = 8;
	    };
	} else {		/* Non-zero => decimal. */
	    base = 10;
	};
    } else if (base == 16) {	/* Base 16 allows leading prefix. */
	if ((nptr[0] == CHAR('0'))
	    && ((nptr[1] == CHAR('x')) || (nptr[1] == CHAR('X')))) {
	    hex_prefix = 1;	/* Indicate got hex prefix. */
	    nptr += 2;		/* Advance past prefix. */
	};
    };

    /*
     * Check for leading zeros.  This allows for an easier overflow
     * check later.  This allows for the case of LONG_MAX leading zeros
     * that would overflow 'digits'. 
     */
    if (*nptr == CHAR('0')) {	/* Is there a leading zero? */
	++digits;		/* Bump count of valid digits. */
	do {
	    ++nptr;		/* Advance past zero. */
	} while (*nptr == CHAR('0'));
    };

    /*
     * maxdiv is that largest value that won't overflow when
     * multiplied by the base.  It is used when checking to
     * see if an overflow may happen.  It would be faster to 
     * get this from a table at some future time.  In the
     * conversion loop you can't simply check for (val < lastval)
     * to catch overflow; there are some cases that this
     * misses.  However, if you prescreen by checking for
     * val > maxdiv, then the only overflow case that will go
     * throught the loop is the case where (val * base) == maxval,
     * so (val * base + xx) will be at most 32 after the overflow.
     */

#if SIGNED==1
    if (sign == 1) 
	{
	maxdiv = POS_OVERFLOW / base;
	}
    else
	{
	maxdiv = NEG_OVERFLOW / base;
	}
#else				/* unsigned */
	maxdiv = POS_OVERFLOW / base;
#endif /* SIGNED==1 */

    /*
     * Convert, digit by digit, left to right.  Check for overflow.
     * Since leading zeros have been taken care of, we know that
     * successive values of 'val' must be greater than all previous
     * values of 'val' (using unsigned arithmetic). 
     */
    num_digits = 0;		/* Have no significant digits yet. */
/* Special case bases 8, 10, 16 since compiler can do shifts and adds instead of multiply */
    switch (base) {
      case 8:

	while (IS_ALNUM(*nptr) && ((xx = DIGIT(*nptr)) < base)) {
      	    if (val > maxdiv) { /* if so, another digit would overflow */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    lastval = val;	/* Need old value to check for overflow */
	    val = 8 * val + xx;	/* Accumulate base-8 number */
	    if (val <= lastval) {	/* New value should be larger
					 * than last value. */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    ++nptr;		/* Advance past digit. */
	    ++num_digits;	/* Indicate got another signif. digit. */
	} /* End loop to convert digit by digit. */ ;
	break;
      case 10:

	while (IS_ALNUM(*nptr) && ((xx = DIGIT(*nptr)) < base)) {
      	    if (val > maxdiv) { /* if so, another digit would overflow */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    lastval = val;	/* Need old value to check for overflow */
	    val = 10 * val + xx;/* Accumulate base-10 number */
	    if (val <= lastval) {	/* New value should be larger
					 * than last value. */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    ++nptr;		/* Advance past digit. */
	    ++num_digits;	/* Indicate got another signif. digit. */
	} /* End loop to convert digit by digit. */ ;
	break;

      case 16:

	while (IS_ALNUM(*nptr) && ((xx = DIGIT(*nptr)) < base)) {
      	    if (val > maxdiv) { /* if so, another digit would overflow */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    lastval = val;	/* Need old value to check for overflow */
	    val = 16 * val + xx;/* Accumulate base-16 number */
	    if (val <= lastval) {	/* New value should be larger
					 * than last value. */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    ++nptr;		/* Advance past digit. */
	    ++num_digits;	/* Indicate got another signif. digit. */
	} /* End loop to convert digit by digit. */ ;
	break;

      default:

	while (IS_ALNUM(*nptr) && ((xx = DIGIT(*nptr)) < base)) {
      	    if (val > maxdiv) { /* if so, another digit would overflow */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    lastval = val;	/* Need old value to check for overflow */
	    val = base * val + xx;	/* Accumulate base-nn number */
	    if (val <= lastval) {	/* New value should be larger
					 * than last value. */
		overflag = 1;	/* Indicate number is too large. */
		break;		/* Leave the while loop. */
	    };
	    ++nptr;		/* Advance past digit. */
	    ++num_digits;	/* Indicate got another signif. digit. */
	} /* End loop to convert digit by digit. */ ;
	break;

    } /* End switch (base) */ ;

    if (num_digits > MAX_DIGITS[base])
	overflag = 1;		/* Number has too many digits, so it
				 * overflowed.  Catches 0x123456789 */
    digits += num_digits;	/* Add in number of significant digits */
#if SIGNED == 1
    /*
     * If we are working with signed numbers, and the sign bit is set,
     * then overflow occured, except for the one case of -type_MIN, eg,
     * -LONG_MIN or -LONGLONG_MIN.  But remember, this is unsigned
     * math. 
     */
    if ((sign == +1) && (val >= (POS_OVERFLOW + 1ul)))
	overflag = 1;
    if ((sign == -1) && (val > (POS_OVERFLOW + 1ul)))
	overflag = 1;
#endif
    /*
     * If overflow occurred, keep scanning characters because the
     * endptr is the first character that is not consistant with the
     * base. 
     */
    if (overflag) {
	/* 'digits' already is greater than zero, so no need to inc it. */
	while (IS_ALNUM(*nptr) && ((xx = DIGIT(*nptr)) < base)) {
	    ++nptr;		/* Scan past valid digits */
	};
	if (sign == 1) {	/* Sign determines which end. */
	    val = POS_OVERFLOW;	/* + overflow */
	} else {
	    val = NEG_OVERFLOW;	/* - overflow */
	};
	errno = ERANGE;		/* Overflow error code */
	sign = 1;		/* Make sure that val is left alone for
				 * return. */
    };				/* End overflow */

    /*
     * Update *endptr to point to the first invalid digit. *endptr has
     * already been done updated to point to the first char of input
     * string in case there were errors. 
     */
    if (0 != digits) {		/* Found some valid digits */
	*endptr = (CHAR_TYPE *) nptr;	/* Normal case. */
    } else {			/* No valid digits. */
	if (hex_prefix) {
	    /*
	     * String is just "0x" means that the '0' is a valid digit
	     * and the 'x' is the first invalid digit. There may be
	     * leading spaces. 
	     */
	    *endptr = (CHAR_TYPE *) (--nptr);
	} else {
	    /*
	     * String is not of the expected form.  *endptr = original
	     * nptr. X/Open allows errno to be set to EINVAL, but
	     * previous AIX has not done this, so to maintain binary
	     * compatibility, errno is left alone. 
	     */
	    sign = 1;		/* Positive zero must be returned, even
				 * if string is just "-" */
	};
    };

    if (sign == -1) {		/* Was there a minus sign? */
	return (-val);		/* Negate value.  This is true even for
				 * unsigned numbers.  See ANSI C page
				 * 35, section 3.2.1.2 */
    } else {
	return (val);
    };
}
