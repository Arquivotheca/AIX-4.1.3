static char sccsid[] = "@(#)15	1.14  src/bos/usr/ccs/lib/libc/NLtmtime.c, libcnls, bos411, 9428A410j 1/9/93 12:48:15";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support 
 *
 * FUNCTIONS: NLtmtime, doscan, number, string, getfield, setparam,
 *	      setgen, setampm, setzone, detype, l_getshowa
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */
#define NULL 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NLctype.h>
#include <values.h>
#include <time.h>

#define GETC(c)		(c = *wc_strp++)
#define UNGETC(c)	(c = *--wc_strp)
#define IGETC(c)	(c = *(*wc_strp)++)
#define UNIGETC(c)	(c = *--*wc_strp)

#define ISSTRING	0
#define ISNUM		1
#define ISERROR 	2

#define NOSECONDS	1

static int Noyear = 0, Noseconds = 0, finish = 0;
static int Yearfnd = 0;
static wchar_t **savestrp;

/* Enumerated type used for field descriptor values;  easier to code
   and debug than preprocessor definitions
*/

static enum desc { m, d, y, H, M, S, j, w, D, T, a, h, b, r, Y, la, lh, B,
		   lD, sH, sD, sT, p, z, Js, Jy
} desc;

static void setgen(), setampm(), setzone();
static int string(), number(), doscan();
char  *NLgetenv();
static int setparam();

/*
 * NAME: NLtmtime
 *
 * FUNCTION: Sets a time structure from string data.
 *
 * NOTE: doscan() processes data and return value.
 * 
 * RETURN VALUE DESCRIPTION: The number of matches found between format
 * 	     specifications and data.
 */
int
NLtmtime(str, fmt, tm)
unsigned char *str, *fmt;
struct tm *tm;
{
	int num, len;
	wchar_t *wc_str, *wc_fmt;

	len = strlen(str);
	wc_str = (wchar_t *)malloc((len+1) * (sizeof(wchar_t)));
	mbstowcs(wc_str, str, len+1);
	len = strlen(fmt);
	wc_fmt = (wchar_t *)malloc((len+1) * (sizeof(wchar_t)));
	mbstowcs(wc_fmt, fmt, len+1);
	num = doscan(wc_str, wc_fmt, tm);
	finish = 0;
	free(wc_str);
	free(wc_fmt);
	return (num);
}

/*
 * NAME: doscan
 *
 * FUNCTION: Scans data and converts them according to fmt.
 *
 * NOTE: detype() determines format type; number() processes number type;
 *       string() processes char type.
 *
 * RETURN VALUE DESCRIPTION: The number of matches found between data and
 *	 format specifications.
 */
static int
doscan(wc_str, wc_fmt, tm)
wchar_t *wc_str;	/* a wide char string to be converted */
wchar_t *wc_fmt;	/* wide char format specifications */
struct tm *tm;	/* tm struct */
{
	wchar_t *wc_strp;	/* the next character position */
	wchar_t wch, inchar;
	int nmatch = 0, len, stow, size;

	wc_strp = wc_str;
	savestrp = &wc_strp;
	for( ; ; ) {
		if((wch = *wc_fmt++) == '\0' || finish)
			return(nmatch); /* end of format */
		if(iswspace(wch)) {
			GETC(inchar);
			while(iswspace(inchar))
				GETC(inchar);
			if (UNGETC(inchar) != NULL)
				continue;
			break;
		}
		if(wch != '%' || (wch = *wc_fmt++) == '%') {
			if (GETC(inchar) == wch)
				continue;
			if (UNGETC(inchar) != NULL)
				return(nmatch); /* failed to match input */
			break;
		}

		/* If we reach here, we must have encountered a '%' */
		if(wch == '*') {
			stow = 0;
			wch = *wc_fmt++;
		} else
			stow = 1;

		for(len = 0; isdigit(wch); wch = *wc_fmt++)
			len = len * 10 + wch - '0';
		if(len == 0)
			len = MAXINT;

		if(wch == '\0' )
			return(NULL); /* unexpected end of format */

		while(iswspace(GETC(inchar)))
			;
		if(UNGETC(inchar) == NULL)
			break;

		switch (detype(wch, wc_fmt) ) {
		case ISSTRING: size = string(stow, len, tm, &wc_strp, wc_fmt); break;
		case ISNUM:
			size = number(stow, len, tm, &wc_strp);
			break;
		case ISERROR:  size = 0; 
		}
		nmatch += size;
		if(size == 0)
			return(nmatch); /* failed to match input */
		switch (desc) {
			case la: case lh: case lD:
			case sH: case sD: case sT:
			case Js: case Jy:
				wc_fmt++ ;
		}
	}
	return(nmatch); /* end of input */
}

/*
 * NAME: number
 *
 * FUNCTION: Processes digit.
 *
 * RETURN VALUE DESCRIPTION: The number of digit processed; 0 for
 *           the unrecognized conversion char.
 */
static int
number(stow, len, tm, wc_strp)
int stow, len;	/*----  set if format is '*'   ----*/
               	/*----  the field length  ----*/
struct tm *tm;
wchar_t **wc_strp;	/*----  the input data string ----*/
{
	wchar_t numbuf[64];	/*----  input data buffer  ----*/
	wchar_t *np = numbuf; /*---- next char position   ----*/
	wint_t c;
	int digitseen = 0;	/*----  the number of digit  ----*/
	long lcval = 0;	/*----  digit  ----*/
	unsigned char *s1;

	IGETC(c);
	for( ; --len >= 0 && iswdigit(c); *np++ = c, IGETC(c)) {
		int digit = c - '0';
		if(stow)
			lcval = 10 * lcval + digit;
		digitseen=1;
	}
	if(stow && digitseen)
		switch(desc) {
		case m:
			tm->tm_mon  = lcval - 1; break;
		case d:
			tm->tm_mday = lcval; break;
		case y:
			if (!Noyear)
				tm->tm_year = lcval;
			else 
				Noyear = 0;
			break;
		case H:
			tm->tm_hour = lcval; break;
		case M:
			tm->tm_min  = lcval; break;
		case S:
			if (!Noseconds)
				tm->tm_sec  = lcval;
			else
				Noseconds = 0;
			break;
		case j:
			tm->tm_yday = lcval - 1; break;
		case w:
			tm->tm_wday = lcval - 1; break;
		case Y:
			if (!Noyear)
				tm->tm_year = lcval - 1900;
			else
				Noyear = 0;
			break;

		case sH:
			tm->tm_hour = lcval;  break;
		case Jy:
			if (Yearfnd) {			/* was Js found? */
				tm->tm_year += lcval;
				tm->tm_year -= 1901;
				Yearfnd = 0;
				break;
			} else {	/* if not, and only one NLYEAR entry */
				if ((s1 = NLgetenv("NLYEAR")) == NULL)
					return (0);
				if (strchr(s1, ':'))
					return (0);
				else if (l_getshowa("NLYEAR","",&tm->tm_year)) {
					tm->tm_year += lcval;
					tm->tm_year -= 1901;
					break;
				}
				return (0);	/* do not know which era */
			}
		default:
			return(0); /* unrecognized conversion character */
		}
	if(UNIGETC(c) == NULL)
		wc_strp = NULL; /* end of input */
	return(digitseen);
}

/*
 * NAME: string
 *
 * FUNCTION: Processes a string according to the format.
 *
 * RETURN VALUE DESCRIPTION: The number of matches found between the format
 * 	     and the input; 0 for the unrecognized conversion character.	
 */

static int
string(stow, len, tm, wc_strp, wc_fmt)
register int stow, len;
struct tm *tm;
wchar_t **wc_strp;	/*----  the input string  ----*/
wchar_t *wc_fmt;     	/*----  the time format  ----*/
{
	wint_t wch;
	wchar_t *wc_ptr;	/*----  the next char position  ----*/
	wchar_t *wc_start;
	int ret = 1;	/*----  return value  ----*/

	wc_start = wc_ptr = stow ? *wc_strp : NULL;

	/* While we haven't reached end of string 
	   and the characters we are reading are not spaces */

	while(IGETC(wch) != NULL && !iswspace(wch)) {

		if(stow)
			*wc_ptr = wch;
		wc_ptr++;
		if(--len <= 0)
			break;
	}
	if (wch == NULL)
		UNIGETC(wch);

	switch (desc) {
	case D:
		ret = setparam(NLgetenv("NLDATE"), wc_start, tm, wc_fmt + 1); break;
	case T:
	case r:
		ret = setparam(NLgetenv("NLTIME"), wc_start, tm, wc_fmt + 1); break;
	case a:
		setgen("NLSDAY", wc_start, &tm->tm_wday); break;
	/* added the format %b to support posix */
	case h: case b:
		setgen("NLSMONTH", wc_start, &tm->tm_mon); break;
	case la:	
		setgen("NLLDAY", wc_start, &tm->tm_wday); break;
	/* added the format %B to support posix */
	case lh: case B:
		setgen("NLLMONTH", wc_start, &tm->tm_mon); break;
	case lD:
		ret = setparam(NLgetenv("NLLDATE"), wc_start, tm, wc_fmt + 2); break;
	case sD:
	 	Noyear = 1; setparam(NLgetenv("NLLDATE"), wc_start, tm, wc_fmt + 2); break;
	case sT:
		Noseconds = 1; setparam(NLgetenv("NLTIME"), wc_start, tm, wc_fmt + 2); break;
	case p:
		setampm(wc_start, tm); break;
	case z:
		setzone(wc_start, tm); break;
	case Js:
		ret = Yearfnd = l_getshowa("NLYEAR", wc_start, &tm->tm_year); break;
	default:
		return (0);  /* could not recognize descriptor */
	}
	if (wch == NULL || len > 0 && UNIGETC(wch) == NULL)
		*wc_strp = NULL; /* end of input */
	if (wc_ptr == wc_start)
		return(0); /* no match */
	return(ret); /* successful match */
}

/*
 * NAME: detype
 *
 * FUNCTION: Determines the type of format specifiers.
 *
 * RETURN VALUE DESCRIPTION: 0 for string; 1 for number; 2 for error.
 */
/*
   return type of format specifier;  assumes ch is first char in format
   specifier and that next char is = to *rest.  set global variable desc
   (enumerated type) to proper specifier.
*/

int
detype(wch, wc_rest)
wchar_t wch;	/*----  the first character in format specifier  ----*/
wchar_t *wc_rest;	/*----  the remaining characters  ----*/
{
	int typestat;	/*----  number, or string or error  ----*/

	/* initialize default to ISNUM to avoid duplicating code */

	typestat = ISNUM;

	switch (wch) {

	case 'm': desc = m; break;
	case 'd': desc = d; break;
	case 'y': desc = y; break;
	case 'H': desc = H; break;
	case 'M': desc = M; break;
	case 'S': desc = S; break;
	case 'j': desc = j; break;
	case 'w': desc = w; break;
	case 'Y': desc = Y; break;
	case 's':
		switch (*wc_rest) {
		case 'H': desc = sH; break;
		case 'D': desc = sD; typestat = ISSTRING; break;
		case 'T': desc = sT; typestat = ISSTRING; break;
		default:
			typestat = ISERROR;
		}

		break;

	case 'l':

		typestat = ISSTRING;

		switch (*wc_rest) {
		case 'a': desc = la; break;
		case 'h': desc = lh; break;
		case 'D': desc = lD; break;
		default:
			typestat = ISERROR;
		}

		break;

	case 'D': desc = D; typestat = ISSTRING; break;
	case 'T': desc = T; typestat = ISSTRING; break;
	case 'a': desc = a; typestat = ISSTRING; break;
	case 'h': desc = h; typestat = ISSTRING; break;
	/* added the format %b and %B to support posix */
	case 'b': desc = b; typestat = ISSTRING; break;
	case 'B': desc = B; typestat = ISSTRING; break;
	case 'r': desc = r; typestat = ISSTRING; break;
	case 'p': desc = p; typestat = ISSTRING; break;
	case 'z': desc = z; typestat = ISSTRING; break;
	case 'J':
		switch (*wc_rest) {
		case 'y': desc = Jy; typestat = ISNUM; break;
		case 's': desc = Js; typestat = ISSTRING; break;
		default:
			typestat = ISERROR;
		}
		break;
	default:
		typestat = ISERROR;
	}

	return (typestat);
}

/*
 * NAME: getfield
 *
 * FUNCTION: Counts the delimiter ':' and get the field.
 *
 * RETURN VALUE DESCRIPTION: the number of occurrence of ':'.
 */
static int
getfield(source, wc_pat)
unsigned char *source;
wchar_t *wc_pat;	/*----  an environment variable  ----*/ 
                       	/*----  a string to be scanned  ----*/
{

	wchar_t *ws1, *wc_newsource;	/*----  the next field position  ----*/
                           	/*----  a temporary storage for source  ----*/
	register int count = 0;	/*----  a counter for ':' ----*/

	wc_newsource = (wchar_t *)malloc ((strlen(source) + 1) * (sizeof(wchar_t)));
	if (wc_newsource == NULL)
		return 0;
	mbstowcs(wc_newsource, source, strlen(source) +1);
	ws1 = wcstok(wc_newsource, L":");

	while (wcsncmp(ws1, wc_pat, wcslen(ws1))) {
		ws1 = wcstok(NULL, L":");
		count++;
	}

	free (wc_newsource);

	if (!ws1)
		return 0;
	else
		return count;
}


/*
 * NAME: setparam
 *
 * FUNCTION: Sets parameters recursively.
 *
 * NOTE:     doscan() does scan and set parameters.
 * 
 * RETURN VALUE DESCRIPTION: the number of matches found between 
 *           the format and the input string. 
 */
static int
setparam(env, wc_start, tm, wc_rest)
unsigned char *env;
wchar_t *wc_start, *wc_rest;	/*----  the environment variable  ----*/
               			/*----  the input string  ----*/
				/*----  the format specification  ----*/
struct tm *tm;
{
	extern wchar_t **savestrp;
	int num = 1, len;
	wchar_t *wc_env;

	len = strlen(env);
	wc_env = (wchar_t *)malloc((len+1)*(sizeof(wchar_t)));
	mbstowcs(wc_env, env, len+1);
	if (env[0] != '*')
		(void) doscan(wc_start, wc_env, tm);
	else
		num = doscan(wc_start, wc_env + 1, tm);

	num += doscan (*savestrp, wc_rest, tm);
	if (wc_rest)
		++finish;

	free(wc_env);
	return(num);
}


/*
 * NAME: setgen
 *
 * FUNCTION: Set environment and get fields.
 *
 * RETURN VALUE DESCRIPTION: None
 */
static void
setgen(ep, wc_start, field)
unsigned char *ep;	/*----  the environment string  ----*/
int *field;	/*----  the field  ----*/
wchar_t *wc_start;	/*----  the input string  ----*/
{
	unsigned char *env;

	env = (unsigned char *)NLgetenv(ep);
	*field = getfield(env, wc_start);
}

/*
 * NAME: setampm
 *
 * FUNCTION: Sets am or pm.
 *
 * RETURN VALUE DESCRIPTION: None
 */
static void
setampm(wc_start, tm)
struct tm *tm;
wchar_t *wc_start;
{
	unsigned char *miscenv, *amex, *pmex, *s1;
	wchar_t *wc_miscenv, *wc_amex, *wc_pmex, *ws1;
	register int i;

	miscenv = (unsigned char *)NLgetenv("NLTMISC");
	s1 = (unsigned char *)malloc(strlen(miscenv) + 1);
	if (s1 == (unsigned char *)0)
		return;
	strcpy(s1, miscenv);
	miscenv = s1;
	wc_miscenv = (wchar_t *)malloc((strlen(miscenv) +1)*(sizeof(wchar_t)));
        mbstowcs(wc_miscenv, s1, strlen(s1)+1);
	wcstok(wc_miscenv, L":");
	for (i=1; i<=4; i++) 
		 wcstok(NULL, L":");
	wc_amex = wcstok(NULL, L":");
	wc_pmex = wcstok(NULL, L":");

	if (!wcsncmp(wc_amex, wc_start, (size_t)4)) 
		tm->tm_hour %= 12;
	else if (!wcsncmp(wc_pmex, wc_start, (size_t)4))
		if (tm->tm_hour != 12) 
			tm->tm_hour += 12;
	free(s1);
	free(wc_miscenv);
}

/*
 * NAME: setzone
 *
 * FUNCTION: Sets zone.
 *
 * RETURN VALUE DESCRIPTION: None
 */
static void
setzone(wc_start, tm)
wchar_t *wc_start;	/*----  the input string  ----*/
struct tm *tm;
{
	unsigned char *env;
	int envlen;
	wchar_t *wc_env;

	env = (unsigned char *)getenv("TZ");
	envlen = strlen(env);
	
	wc_env = (wchar_t *)malloc((envlen +1) * (sizeof(wchar_t)));

	mbstowcs(wc_env,env,envlen+1);
	if (!wcsncmp(wc_start, wc_env, (size_t)3))
		tm->tm_isdst = 0;
	else if (envlen > 3 && !wcsncmp(wc_start, wc_env + envlen - 3, (size_t)3))
		tm->tm_isdst = 1;
}

l_getshowa(ep, wc_start, field)
char *ep;
char *wc_start;
int *field;

{
	/* This routine searches the NLYEAR string for an entry with
	   matching era name (give era's names unless there is only
	   one entry...); when found, set switch and move the starting
	   year of the era into tm_year, for later use by %Jy.
	 */
	register int n, n1, n2;
	char *s1, *s2, *s3, *s4;
	wchar_t *ws1, *ws2, *ws3, *ws4;

	s1 = NLgetenv(ep);
	if (s1 == 0)
		return (0);
	s3 = malloc(strlen(s1) + 1);
	(void) strcpy(s3, s1);
	ws3 = (wchar_t *) malloc ((strlen(s1)+1) * (sizeof(wchar_t)));
	mbstowcs(ws3,s3,strlen(s3)+1);
	ws1 = wcstok(ws3, L":");
	if (ws1 == 0)
		ws1 = ws3;
	do {
	    ws2 = ws1;
	    ws4 = wcstok(ws2, L',');
	    if (!ws4) {
		free (s3);
		free (ws3);
		return (0);
	    }
            ++ws4;
	    if (wcsncmp(ws4, wc_start,wcslen(ws4)) == 0) {
	    	if (wcsncmp(ws2, "-", 1) == 0) {
			++ws2;
			n2 = 1;
		} else  n2 = 0;
	    	for (n = 0, n1 = 0; n1 < 4; n1++)
			n = n*10 + *ws2++ - '0';
	    	if (n2)	n = -n;	
		*field = n;
		free (s3);
		free (ws3);
		return (1);
	    }
	}
	while (ws1 = wcstok(NULL, L":"));
	free (s3);
	free (ws3);
	return(0);
}

/*
static
wchar_t *
wcstok(s1, s2)
wchar_t *s1;
wchar_t *s2;
{
	wchar_t	*p, *q, *r;
	static wchar_t	*savept;
*/
	/*first or subsequent call*/
/*
	p = (s1 == NULL)? savept: s1;

	if(p == 0)		/* return if no tokens remaining */
/*
		return(NULL);

	q = p + wcsspn(p, s2);	/* skip leading separators */
/*

	if(*q == '\0')		/* return if no tokens remaining */
/*
		return(NULL);

	if((r = wcspbrk(q, s2)) == NULL)	/* move past token */
/*
		savept = 0;	/* indicate this is last token */
/*
	else {
		/* skip over escaped separators */
/*
		while (*(r-1) == '\\') {
			wcscpy(r-1, r);
			if((r = wcspbrk(r, s2)) == NULL) {
				savept = 0;
				return (q);
			}
		}
		*r = '\0';
		savept = ++r;
	}
	return(q);
}
*/
