static char sccsid[] = "@(#)65	1.6.1.5  src/bos/usr/ccs/lib/libc/__fnmatch_C.c, libcpat, bos411, 9428A410j 3/30/94 14:54:39";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: __fnmatch_C
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/localedef.h>
#include <fnmatch.h>
#include "patlocal.h"


static int bracket(		/* compare [] expression to character	*/
		_LC_collate_objhdl_t,
		const char *,
		char **,
		uchar_t,
		int);

/************************************************************************/
/* __fnmatch_C()   - Single byte method for Filename Matching		*/
/*               - works for C locale					*/
/*									*/
/*	hdl     - ptr to __lc_collate structure			*/
/*	ppat	 - ptr to filename pattern				*/
/*	string	 - ptr to beginning of filename				*/
/*	pstr	 - ptr to current position within filename		*/
/*	flags	 - fnmatch() flags, see <fnmatch.h>			*/
/************************************************************************/

int
__fnmatch_C(_LC_collate_objhdl_t hdl, const char *ppat, const char *string, 
	    const char *pstr, int flags)
{
	int	stat;		/* recursive __fnmatch_C() return status	*/

/*
 * Loop through pattern, matching string characters with pattern
 * Return success when end-of-pattern and end-of-string reached simultaneously
 * Return no match if pattern/string mismatch
 */
	while (*ppat != '\0')
		{
		switch (*ppat)
			{
/*
 * <backslash> does not quote the next character if FNM_NOESCAPE flag is set
 * Return no match if pattern ends with quoting <backslash>
 */
		case '\\':
			if ((flags & FNM_NOESCAPE) == 0)
				if (*++ppat == '\0')
					return (FNM_NOMATCH);
/*
 * Ordinary character in pattern matches itself in string
 * Continue if pattern character matches string character
 * Return no match if pattern character does not match string character
 */
		default:
		ordinary:
			if (*ppat++ == *pstr++)
				break;
			else
				return (FNM_NOMATCH);
/*
 * <asterisk> matches zero or more string characters
 * Cannot match <slash> if FNM_PATHNAME is set
 * Cannot match leading <period> if FNM_PERIOD is set
 * Consecutive <asterisk> are redundant
*
 * Return success if remaining pattern matches remaining string
 * Otherwise advance to the next string character and try again
 * Return no match if string exhausted and more pattern remains
 */
		case '*':
			while (*++ppat == '*')
				;
			if (*ppat == '\0')
				{
				if ((flags & FNM_PATHNAME) != 0 && strchr(pstr, '/') != NULL)
					return (FNM_NOMATCH);
				if (*pstr == '.' && (flags & FNM_PERIOD) != 0)
					if (pstr == string || (pstr[-1] == '/' && (flags & FNM_PATHNAME) != 0))
						return (FNM_NOMATCH);
				return (0);
				}
			while (*pstr != '\0')
				{
				stat = __fnmatch_C(hdl, ppat, string, pstr, 
						   flags);
				if (stat != FNM_NOMATCH)
					return (stat);
				if (*pstr == '/')
					{
					if ((flags & FNM_PATHNAME) != 0)
						return (FNM_NOMATCH);
					}
				else if (*pstr == '.' && (flags & FNM_PERIOD) != 0)
					if (pstr == string || (pstr[-1] =='/' && (flags & FNM_PATHNAME) != 0))
						return (FNM_NOMATCH);
				pstr++;
				}
			return (FNM_NOMATCH);
/*
 * <question-mark> matches any single character
 * Cannot match <slash> if FNM_PATHNAME is set
 * Cannot match leading <period> if FNM_PERIOD is set
 *
 * Return no match if string is exhausted
 * Otherwise continue with next pattern and string character
 */
		case '?':
			if (*pstr == '/')
				{
				if ((flags & FNM_PATHNAME) != 0)
					return (FNM_NOMATCH);
				}
			else if (*pstr == '.' && (flags & FNM_PERIOD) != 0)
				if (pstr == string || (pstr[-1] == '/' && (flags & FNM_PATHNAME) != 0))
					return (FNM_NOMATCH);
			if (*pstr != '\0')
			{
				pstr++;
				ppat++;
				break;
			}
			else
				return(FNM_NOMATCH);
/*
 * <left-bracket> begins a [bracket expression] which matches single collating element
 * [bracket expression] cannot match <slash> if FNM_PATHNAME is set
 * [bracket expression] cannot match leading <period> if FNM_PERIOD is set
 */
		case '[':
			if (*pstr == '/')
				{
				if ((flags & FNM_PATHNAME) != 0)
					return (FNM_NOMATCH);
				}
			else if (*pstr == '.' && (flags & FNM_PERIOD) != 0)
				if (pstr == string || (pstr[-1] == '/' && (flags & FNM_PATHNAME) != 0))
					return (FNM_NOMATCH);
/*
 * Compare next filename character to [bracket expression]
 *   > 0  no match
 *   = 0  match found
 *   < 0  error, treat [] as individual characters
 */
			stat = bracket(hdl, ppat+1, &ppat, *pstr, flags);
			if (stat == 0)
				pstr++;
			else if (stat > 0)
				return (FNM_NOMATCH);
			else
				goto ordinary;
			break;
			}
		}
/*
 * <NUL> following end-of-pattern
 * Return success if string is also at <NUL>
 * Return no match if string not at <NUL>
 */
		if (*pstr == '\0')
			return (0);
		else
			return (FNM_NOMATCH);
	}


/************************************************************************/
/* bracket()    - Determine if [bracket] matches filename character	*/
/*									*/
/*	pdhl	 - ptr to __lc_collate structure			*/
/*	ppat	 - ptr to position of '[' in pattern			*/
/*	pend     - ptr to character after trailing ']' in pattern	*/
/*	fc	 - process code of next filename character		*/
/*	flags	 - fnmatch() flags, see <fnmatch.h>			*/
/************************************************************************/

static int
bracket(_LC_collate_objhdl_t hdl, const char *ppat, char **pend, uchar_t fc, 
	int flags)
{
	int	dash;		/* <hyphen> found for a-z range expr	*/
	int	found;		/* match found flag			*/
	uchar_t	min_fc;		/* minimum range value			*/
	uchar_t	max_fc;		/* maximum range value			*/
	int	neg;		/* nonmatching [] expression	*/
	char	*pb;		/* ptr to [bracket] pattern		*/
	char	*pclass;	/* class[] ptr				*/
	char	*pi;		/* ptr to international [] expression	*/
	uchar_t	prev_min_fc;	/* low end of range expr		*/
	char	type;		/* international [] type =:.		*/
	char	class[CLASS_SIZE+1]; /* character class with <NUL>	*/
/*
 * Leading <exclamation-mark> designates nonmatching [bracket expression]
 */
	pb = ppat;
	neg = 0;
	if (*pb == '!')
		{
		pb++;
		neg++;
		}
/*
 * Loop through each [] character, comparing to file code of string character
 */
	dash = 0;
	found = 0;
	prev_min_fc = 0;
	min_fc = 0;
	max_fc = 0;
	while (*pb != '\0')
		{
/*
 * Preserve minimum file code of previous character for range expression
 * Decode next [] element
 */
		if (dash == 0)
			prev_min_fc = min_fc;
		switch (*pb)
			{
/*
 * Ordinary character
 */
		default:
		ordinary:
			if (*pb == fc)
				found = 1;
			min_fc = *pb++;
			max_fc = min_fc;
			break;
/*
 * <hyphen> deliniates a range expression unless it is first character of []
 * or it immediately follows another <hyphen> and is therefore an end point
 */
		case '-':
			if (dash == 0 && !((neg == 0 && pb == ppat) || (neg != 0 && pb == ppat + 1)))
				{
				dash = '-';
				pb++;
				continue;
				}
			goto ordinary;
/*
 * Final <right-bracket> so return status based upon whether match was found
 * Return character after final ] if match is found
 * Ordinary character if first character of [barcket expression]
 */
		case ']':
			if ((neg == 0 && pb > ppat) || (neg != 0 && pb > ppat + 1))
				{
				*pend = ++pb;
				if (dash && dash == fc)
					found = 1;
				if ((found ^ neg) != 0)
					return (0);
				else
					return (FNM_NOMATCH);
				}
			goto ordinary;
/*
 * Embedded <slash> not allowed if FNM_PATHNAME is set
 */
		case '/':
			if ((flags & FNM_PATHNAME) != 0)
				return (-1);
			goto ordinary;
/*
 * <left-bracket> initiates one of the following internationalization
 *   character expressions
 *   [: :] character class
 *   [= =] equivalence character class
 *   [. .] collation symbol
 *
 * it is treated as itself if not followed by appropriate special character
 * it is treated as itself if any error is encountered
 */
		case '[':
			pi = pb + 2;
			if ((type = pb[1]) == ':')
				{
				pclass = class;
				while (1)
					{
					if (*pi == '\0')
						return (-1);
					if (*pi == ':' && pi[1] == ']')
						break;
					if (pclass >= &class[CLASS_SIZE])
						return (-1);
					*pclass++ = *pi++;
					}
				if (pclass == class)
					return (-1);
				*pclass = '\0';
				if (is_wctype((wchar_t)fc, get_wctype(class)) != 0)
					found = 1;
				min_fc = 0;
				pb = pi + 2;
				break;
				}
/*
 * equivalence character class or collation symbol
 *   treat as ordinary if not [= =]
 */
			else if (type == '=' || type == '.')
				{
				if (*pi == '\0' || pi[1] != type || pi[2] != ']')
					return (-1);
				if ((uchar_t)*pi == fc)
					found = 1;
				min_fc = (uchar_t)*pi;
				max_fc = min_fc;
				pb = pi + 3;
				break;
				}
			else
				goto ordinary;
			} /* end of switch */
/*
 * Return if range expression is illegal
 * Check for the completion of a range expression and determine
 * whether filename character falls between end points
 */
		if (dash != 0)
			{
			dash = 0;
				if (prev_min_fc == 0 || min_fc == 0 || prev_min_fc > max_fc)
					return (-1);
				if (fc >= prev_min_fc && fc <= max_fc)
					found = 1;
				min_fc = 0;
			}
		} /* end of while */
/*
 * Return < 0 since <NUL> was found
 */
	return (-1);
}
