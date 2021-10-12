static char sccsid[] = "@(#)09  1.1  src/bos/usr/ccs/lib/libc/POWER/wcstoll.c, libccnv, bos411, 9428A410j 9/10/93 13:42:29";
/*
 * COMPONENT_NAME: LIBCCNV 
 *
 * FUNCTIONS: wcstoll
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:
 *      wcstoll
 *
 * FUNCTION:
 *      Converts a wide character string (in process code format) to
 *      a 64-bit signed long long integer.
 *
 * NOTES:
 *
 * The function wcstoll() converts the initial portion of the wide
 * character string pointed to by nptr to a 64-bit signed long long
 * integer representation.  To do this, it parses the wide character
 * string pointed to by nptr to obtain a valid string (i.e., subject
 * string) for the purpose of conversion to an signed long long
 * integer.  It points the endptr to the position where an unrecognized
 * character including the terminating null is found.
 *
 * The base can take the following values:  0 thru 9, a (or A) through
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
 * Returns the converted value of long long integer if the expected
 * form is found.
 *
 * If no conversion could be performed, zero is returned.
 *
 * If the converted value is outside the range of representable values,
 * LONGLONG_MAX or LONGLONG_MIN is returned (according to the sign of
 * the value), and the value of the variable errno is set to ERANGE.
 */

#define FUNC wcstoll		/* strtol, strtoul, strtoll, strtoull,
				 * wcstol, wcstoul, wcstoll, wcstoull */
#define CHAR_TYPE wchar_t	/* char or wchar_t */
#define RET_TYPE long long int	/* without 'signed' or unsigned */
#define RET_SIGN		/* 'signed' or unsigned */
#define MAX_BITS 64		/* 32 or 64 */
#define MAX_DIGITS __64		/* __32 or __64 */
#define SIGNED 1		/* 1 for signed, 0 for unsigned */
#define WIDECHAR 1		/* 1 for wide chars, 0 for reg char */
#define CHAR(c) L##c		/* L => wide char, blank => reg char */
#define IS_SPACE iswspace	/* isspace or iswspace */
#define IS_ALNUM iswalnum	/* isalnum or iswalnum */
#define IS_DIGIT iswdigit	/* isdigit or iswdigit */
#define TO_LOWER towlower	/* tolower or towlower */
#define POS_OVERFLOW LONGLONG_MAX	/* U...MAX or ...MAX */
#define NEG_OVERFLOW LONGLONG_MIN	/* U...MAX or ...MIN */

#include "strto.c"		/* The real code */
