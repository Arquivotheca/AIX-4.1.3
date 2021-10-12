static char sccsid[] = "@(#)66	1.8  src/bos/usr/ccs/lib/libc/regex.c, libcpat, bos411, 9428A410j 4/29/94 08:59:24";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: re_comp, re_exec
 *
 * ORIGINS: 26,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <nl_types.h>
#include "libc_msg.h"

#ifdef _THREAD_SAFE
#include <regex.h>
#endif	/* _THREAD_SAFE */


/*
 * FUNCTION: regular expression matching
 *
 */

/*
 * routines to do regular expression matching
 *
 * Entry points:
 *
 *	re_comp(s)
 *		char *s;
 *	 ... returns 0 if the string s was compiled successfully,
 *		     a pointer to an error message otherwise.
 *	     If passed 0 or a null string returns without changing
 *           the currently compiled re (see note 11 below).
 *
 *	re_exec(s)
 *		char *s;
 *	 ... returns 1 if the string s matches the last compiled regular
 *		       expression, 
 *		     0 if the string s failed to match the last compiled
 *		       regular expression, and
 *		    -1 if the compiled regular expression was invalid 
 *		       (indicating an internal error).
 *
 * The strings passed to both re_comp and re_exec may have trailing or
 * embedded newline characters; they are terminated by nulls.
 *
 * The identity of the author of these routines is lost in antiquity;
 * this is essentially the same as the re code in the original V6 ed.
 *
 * The regular expressions recognized are described below. This description
 * is essentially the same as that for ed.
 *
 *	A regular expression specifies a set of strings of characters.
 *	A member of this set of strings is said to be matched by
 *	the regular expression.  In the following specification for
 *	regular expressions the word `character' means any character but NUL.
 *
 *	1.  Any character except a special character matches itself.
 *	    Special characters are the regular expression delimiter plus
 *	    \ [ . and sometimes ^ * $.
 *	2.  A . matches any character.
 *	3.  A \ followed by any character except a digit or ( )
 *	    matches that character.
 *	4.  A nonempty string s bracketed [s] (or [^s]) matches any
 *	    character in (or not in) s. In s, \ has no special meaning,
 *	    and ] may only appear as the first letter. A substring 
 *	    a-b, with a and b in ascending ASCII order, stands for
 *	    the inclusive range of ASCII characters.
 *	5.  A regular expression of form 1-4 followed by * matches a
 *	    sequence of 0 or more matches of the regular expression.
 *	6.  A regular expression, x, of form 1-8, bracketed \(x\)
 *	    matches what x matches.
 *	7.  A \ followed by a digit n matches a copy of the string that the
 *	    bracketed regular expression beginning with the nth \( matched.
 *	8.  A regular expression of form 1-8, x, followed by a regular
 *	    expression of form 1-7, y matches a match for x followed by
 *	    a match for y, with the x match being as long as possible
 *	    while still permitting a y match.
 *	9.  A regular expression of form 1-8 preceded by ^ (or followed
 *	    by $), is constrained to matches that begin at the left
 *	    (or end at the right) end of a line.
 *	10. A regular expression of form 1-9 picks out the longest among
 *	    the leftmost matches in a line.
 *	11. An empty regular expression stands for a copy of the last
 *	    regular expression encountered.
 */


/*
 * constants for re's
 */
#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CBACK	18

#define	CSTAR	01

#ifdef _THREAD_SAFE

#define COMERR(a, b)	comerr(a, b, regex_data)
#define	BACKREF(a, b)	backref(a, b, regex_data)
#define	ADVANCE(a, b)	advance(a, b, regex_data)
#define	CIRCF		(regex_data->circf)

#else

#define COMERR		comerr
#define	BACKREF		backref
#define	ADVANCE		advance
#define	CIRCF		circf

#define	ESIZE	512
#define	NBRA	9

static char	expbuf[ESIZE], *braslist[NBRA], *braelist[NBRA];
static char	circf;
#endif	/* _THREAD_SAFE */

static char	*comerr();
static int	backref();
static int	advance();
static int	cclass();


/*
 * compile the regular expression argument into a dfa
 */
#ifdef _THREAD_SAFE
char *
re_comp_r(const char *sp, REGEXD *regex_data)
#else
char *
re_comp(const char *sp)
#endif	/* _THREAD_SAFE */
{
	register int	c;
#ifdef  _THREAD_SAFE
	register char   *expbuf = regex_data->expbuf;
#endif	/* _THREAD_SAFE */
	register char	*ep = expbuf;
	int	cclcnt, numbra = 0;
	char	*lastep = 0;
	char	bracket[NBRA];
	char	*bracketp = &bracket[0];

	if (sp == 0 || *sp == '\0') {
		if (*ep == 0)
			return(COMERR(M_NOPREV, "No previous regular expression"));
		return(0);
	}
	if (*sp == '^') {
		CIRCF = 1;
		sp++;
	}
	else
		CIRCF = 0;
	for (;;) {
		if (ep >= &expbuf[ESIZE])
			return(COMERR(M_RETOOLONG, "Regular expression too long"));
		if ((c = *sp++) == '\0') {
			if (bracketp != bracket)
				return(COMERR(M_UNMATCH, "unmatched \\("));
			*ep++ = CEOF;
			*ep++ = 0;
			return(0);
		}
		if (c != '*')
			lastep = ep;
		switch (c) {

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep == 0 || *lastep == CBRA || *lastep == CKET)
				goto defchar;
			*lastep |= CSTAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			*ep++ = CCL;
			*ep++ = 0;
			cclcnt = 1;
			if ((c = *sp++) == '^') {
				c = *sp++;
				ep[-2] = NCCL;
			}
			do {
				if (c == '\0')
					return(COMERR(M_MISSING, "missing ]"));
				if (c == '-' && ep [-1] != 0) {
					if ((c = *sp++) == ']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1] < c) {
						*ep = ep[-1] + 1;
						ep++;
						cclcnt++;
						if (ep >= &expbuf[ESIZE])
							return(COMERR(M_RETOOLONG, "Regular expression too long"));
					}
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[ESIZE])
					return(COMERR(M_RETOOLONG, "Regular expression too long"));
			} while ((c = *sp++) != ']');
			lastep[1] = cclcnt;
			continue;

		case '\\':
			if ((c = *sp++) == '(') {
				if (numbra >= NBRA)
					return(COMERR(M_TOOMANY, "too many \\(\\) pairs"));
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					return(COMERR(M_UNMATCHLEFT, "unmatched \\)"));
				*ep++ = CKET;
				*ep++ = *--bracketp;
				continue;
			}
			if (c >= '1' && c < ('1' + NBRA)) {
				*ep++ = CBACK;
				*ep++ = c - '1';
				continue;
			}
			*ep++ = CCHR;
			*ep++ = c;
			continue;

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
}

/*
 * get error text from message catalogue
 */
#ifdef _THREAD_SAFE
static char *
comerr(	register int	num,            /* msg number */
	register char	*str,           /* default error text */
	REGEXD		*regex_data)
#else
static char *
comerr(	register int	num,            /* msg number */
	register char	*str)           /* default error text */
#endif	/* _THREAD_SAFE */
{
	register char	*perr;		/* return error text */
	nl_catd	catd;			/* message catalogue descriptor */

#ifdef  _THREAD_SAFE
	register char   *expbuf = regex_data->expbuf;
#endif	/* _THREAD_SAFE */
	expbuf[0] = '\0';
	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
	perr = catgets(catd, MS_LIBC, num, str);
	catclose(catd);
	return(perr);
}

/* 
 * match the argument string against the compiled re
 */
#ifdef _THREAD_SAFE
int
re_exec_r(const char *p1, REGEXD *regex_data)
#else
int
re_exec(const char *p1)
#endif	/* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	register char   *p2 = regex_data->expbuf;
	register char   **braslist = regex_data->braslist;
	register char   **braelist = regex_data->braelist;
#else
	register char	*p2 = expbuf;
#endif	/* _THREAD_SAFE */
	register int	c;
	int	rv;

	for (c = 0; c < NBRA; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	if (CIRCF)
		return(ADVANCE(p1, p2));
	/*
	 * fast check for first character
	 */
	if (*p2 == CCHR) {
		c = p2[1];
		do {
			if (*p1 != c)
				continue;
			if (rv = ADVANCE(p1, p2))
				return(rv);
		} while (*p1++);
		return(0);
	}
	/*
	 * regular algorithm
	 */
	do
		if (rv = ADVANCE(p1, p2))
			return(rv);
	while (*p1++);
	return(0);
}

/* 
 * try to match the next thing in the dfa
 */
#ifdef _THREAD_SAFE
static int
advance(register char *lp, register char *ep, REGEXD *regex_data)
#else
static int
advance(register char *lp, register char *ep)
#endif	/* _THREAD_SAFE */
{
	register char	*curlp;
	int	ct, i;
	int	rv;
#ifdef _THREAD_SAFE
	register char	**braslist = regex_data->braslist;
	register char	**braelist = regex_data->braelist;
#endif	/* _THREAD_SAFE */

	for (;;)
		switch (*ep++) {

		case CCHR:
			if (*ep++ == *lp++)
				continue;
			return(0);

		case CDOT:
			if (*lp++)
				continue;
			return(0);

		case CDOL:
			if (*lp == '\0')
				continue;
			return(0);

		case CEOF:
			return(1);

		case CCL:
			if (cclass(ep, *lp++, 1)) {
				ep += *ep;
				continue;
			}
			return(0);

		case NCCL:
			if (cclass(ep, *lp++, 0)) {
				ep += *ep;
				continue;
			}
			return(0);

		case CBRA:
			braslist[*ep++] = lp;
			continue;

		case CKET:
			braelist[*ep++] = lp;
			continue;

		case CBACK:
			if (braelist[i = *ep++] == 0)
				return(-1);
			if (BACKREF(i, lp)) {
				lp += braelist[i] - braslist[i];
				continue;
			}
			return(0);

		case CBACK|CSTAR:
			if (braelist[i = *ep++] == 0)
				return(-1);
			curlp = lp;
			ct = braelist[i] - braslist[i];
			while (BACKREF(i, lp))
				lp += ct;
			while (lp >= curlp) {
				if (rv = ADVANCE(lp, ep))
					return(rv);
				lp -= ct;
			}
			continue;

		case CDOT|CSTAR:
			curlp = lp;
			while (*lp++)
				;
			goto star;

		case CCHR|CSTAR:
			curlp = lp;
			while (*lp++ == *ep)
				;
			ep++;
			goto star;

		case CCL|CSTAR:
		case NCCL|CSTAR:
			curlp = lp;
			while (cclass(ep, *lp++, ep[-1] == (CCL|CSTAR)))
				;
			ep += *ep;
			goto star;

		star:
			do {
				lp--;
				if (rv = ADVANCE(lp, ep))
					return(rv);
			} while (lp > curlp);
			return(0);

		default:
			return(-1);
		}
}

#ifdef _THREAD_SAFE
static int
backref(register int i, register char *lp, REGEXD *regex_data)
#else
static int
backref(register int i, register char *lp)
#endif	/* _THREAD_SAFE */
{
	register char	*bp, *be;

#ifdef _THREAD_SAFE
	bp = regex_data->braslist[i];
	be = regex_data->braelist[i];
#else
	bp = braslist[i];
	be = braelist[i];
#endif	/* _THREAD_SAFE */

	/* if bp == be  then the ith subexpression matched zero occurances. */
	if (bp == be)
		return(1);

	while (*bp++ == *lp++)
		if (bp >= be)
			return(1);
	return(0);
}

static int
cclass(set, c, af)
register char *set;
register char c;
int af;
{
	register int	n;

	if (c == 0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af);
}
