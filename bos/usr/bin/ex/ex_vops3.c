static char sccsid[] = "@(#)03	1.14  src/bos/usr/bin/ex/ex_vops3.c, cmdedit, bos41J, 9523B_all 6/7/95 15:32:40";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_vops3.c
 *
 * FUNCTIONS: endPS, endsent, isa, lbrack, lfind, lindent, lmatchp, lnext,
 * lskipsent, lskipatom, lskipbal, lsmatch, ltosolid
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1981 Regents of the University of California
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

static int endsent(void);
static int endPS(void);
static int ltosolid(void);
static int lskipbal(char *);
static int lskipatom(void);
static int lskipsent();
static int isa(char *);

/*
 * Routines to handle structure.
 * Operations supported are:
 *	( ) { } [ ]
 *
 * These cover:		LISP		TEXT
 *	( )		s-exprs		sentences
 *	{ }		list at same	paragraphs
 *	[ ]		defuns		sections
 *
 * { and } for C used to attempt to do something with matching {}'s, but
 * I couldn't find definitions which worked intuitively very well, so I
 * scrapped this.
 *
 * The code here is very hard to understand.
 */
static line	*llimit;
static int	(*lf)(void);


static short	wasend;

/*
 * Find over structure, repeated count times.
 * Don't go past line limit.  F is the operation to
 * be performed eventually.  If pastatom then the user said {}
 * rather than (), implying past atoms in a list (or a paragraph
 * rather than a sentence.
 */
int lfind(short pastatom, int cnt, void (*f)(void), line *limit)
{
	register int c;
	register int rc = 0;
	wchar_t savec[LBSIZE];

	/*
	 * Initialize, saving the current line buffer state
	 * and computing the limit; a 0 argument means
	 * directional end of file.
	 */
	wasend = 0;
	lf = (int (*)(void))f;
	wcscpy(savec, linebuf);
	if (limit == 0)
		limit = dir < 0 ? one : dol;
	llimit = limit;
	wdot = dot;
	wcursor = cursor;

	if (pastatom >= 2) {
		if (pastatom == 3) {
			while (cnt-- > 0)
				eend(f);
		} else {
			while (cnt > 0 && word(f, cnt))
				cnt--;
		}
		if (dot == wdot) {
			wdot = 0;
			if (cursor == wcursor)
				rc = -1;
		}
	}
	else if (!value(LISP)) {
		wchar_t *icurs;
		line *idot;

		if (linebuf[0] == 0) {
			do
				if (!lnext())
					goto ret;
			while (linebuf[0] == 0);
			if (dir > 0) {
				wdot--;
				linebuf[0] = 0;
				wcursor = linebuf;
				/*
				 * If looking for sentence, next line
				 * starts one.
				 */
				if (!pastatom) {
					icurs = wcursor;
					idot = wdot;
					goto begin;
				}
			}
		}
		icurs = wcursor;
		idot = wdot;

		/*
		 * Advance so as to not find same thing again.
		 */
		if (dir > 0) {
			if (!lnext()) {
				rc = -1;
				goto ret;
			}
		} else
			ignore(lskipsent());

		/*
		 * Count times find end of sentence/paragraph.
		 */
begin:
		for (;;) {
			while (!endsent())
				if (!lnext())
					goto ret;
			if (!pastatom || wcursor == linebuf && endPS())
				if (--cnt <= 0)
					break;
			if (linebuf[0] == 0) {
				do
					if (!lnext())
						goto ret;
				while (linebuf[0] == 0);
			} else
				if (!lnext())
					goto ret;
		}

		/*
		 * If going backwards, and didn't hit the end of the buffer,
		 * then reverse direction.
		 */
		if (dir < 0 && (wdot != llimit || wcursor != linebuf)) {
			dir = 1;
			llimit = dot;
			/*
			 * Empty line needs special treatement.
			 * If moved to it from other than begining of next line,
			 * then a sentence starts on next line.
			 */
			if (linebuf[0] == 0 && !pastatom && 
			   (wdot != dot - 1 || cursor != linebuf)) {
				lnext();
				goto ret;
			}
		}

		/*
		 * If we are not at a section/paragraph division,
		 * advance to next.
		 */
		if (wcursor == icurs && wdot == idot || wcursor != linebuf || !endPS())
			ignore(lskipsent());
	}
	else {
		c = *wcursor;
		/*
		 * Startup by skipping if at a ( going left or a ) going
		 * right to keep from getting stuck immediately.
		 */
		if (dir < 0 && c == '(' || dir > 0 && c == ')') {
			if (!lnext()) {
				rc = -1;
				goto ret;
			}
		}
		/*
		 * Now chew up repitition count.  Each time around
		 * if at the beginning of an s-exp (going forwards)
		 * or the end of an s-exp (going backwards)
		 * skip the s-exp.  If not at beg/end resp, then stop
		 * if we hit a higher level paren, else skip an atom,
		 * counting it unless pastatom.
		 */
		while (cnt > 0) {
			c = *wcursor;
			if (dir < 0 && c == ')' || dir > 0 && c == '(') {
				if (!lskipbal("()"))
					goto ret;
				if (!lnext() || !ltosolid())
					goto ret;
				--cnt;
			} else if (dir < 0 && c == '(' || dir > 0 && c == ')') {
				/* Found a higher level paren */
				if (!lnext() || !ltosolid())
					goto ret;
				--cnt;
			} else {
				if (!lskipatom())
					goto ret;
				if (!pastatom)
					--cnt;
			}
		}
	}
ret:
	strcLIN(savec);
	return (rc);
}

/*
 * Is this the end of a sentence?
 */
static int endsent(void)
{
	register wchar_t *cp = wcursor;
	register int d;

	/*
	 * If this is the beginning of a line, then
	 * check for the end of a paragraph or section.
	 */
	if (cp == linebuf)
		return (endPS());

	/*
	 * Sentences end with . ! ? not at the beginning
	 * of the line, and must be either at the end of the line,
	 * or followed by 2 spaces.  Any number of intervening ) ] ' "
	 * characters are allowed.
	 */
	if (!any(*cp, ".!?"))
		goto tryps;
	do
		if ((d = *++cp) == 0)
			return (1);
	while (any(d, ")]'"));
	if (*cp == 0 || *cp++ == ' ' && *cp == ' ')
		return (1);
tryps:
	if (cp[1] == 0)
		return (endPS());
	return (0);
}

/*
 * End of paragraphs/sections are respective
 * macros as well as blank lines and form feeds.
 */
static int endPS(void)
{

	return (linebuf[0] == 0 ||
		linebuf[0] == '{' ||
		linebuf[0] == '' ||
		isa(svalue(PARAGRAPHS)) || isa(svalue(SECTIONS)));
	    
}

int lindent(line *addr)
{
	register int i;
	wchar_t *swcurs = wcursor;
	line *swdot = wdot;

again:
	if (addr > one) {
		register wchar_t *cp;
		register int cnt = 0;

		addr--;
		getline(*addr);
		for (cp = linebuf; *cp; cp++)
			if (*cp == '(')
				cnt++;
			else if (*cp == ')')
				cnt--;
		cp = vpastwh(linebuf);
		if (*cp == 0)
			goto again;
		if (cnt == 0)
			return (whitecnt(linebuf));
		addr++;
	}
	wcursor = linebuf;
	linebuf[0] = 0;
	wdot = addr;
	dir = -1;
	llimit = one;
	lf = (int(*)(void))lindent;
	if (!lskipbal("()"))
		i = 0;
	else if (wcursor == linebuf)
		i = 2;
	else {
		register wchar_t *wp = wcursor;

		dir = 1;
		llimit = wdot;
		if (!lnext() || !ltosolid() || !lskipatom()) {
			wcursor = wp;
			i = 1;
		} else
			i = 0;
		i += column(wcursor) - 1;
		if (!inopen)
			i--;
	}
	wdot = swdot;
	wcursor = swcurs;
	return (i);
}

int lmatchp(line *addr)
{
	register int i;
	register wchar_t *cp;
	register char *parens;

	for (cp = cursor; !any(*cp, "({[)}]");)
		if (*cp++ == 0)
			return (0);
	lf = 0;
	parens = any(*cp, "()") ? "()" : any(*cp, "[]") ? "[]" : "{}";
	if (*cp == parens[1]) {
		dir = -1;
		llimit = one;
	} else {
		dir = 1;
		llimit = dol;
	}
	if (addr)
		llimit = addr;
	if (splitw)
		llimit = dot;
	wcursor = cp;
	wdot = dot;
	i = lskipbal(parens);
	return (i);
}

void lsmatch(wchar_t *cp)
{
	wchar_t savec[LBSIZE];
	register wchar_t *sp = savec;
	register wchar_t *scurs = cursor;

	wcursor = cp;
	wcscpy(sp, linebuf);
	*wcursor = 0;
	wcscpy(cursor, genbuf);
	cursor = WCstrend(linebuf) - 1;
	if (lmatchp(dot - vcline)) {
		register int i = insmode;
		register int c = outcol;
		register int l = outline;

		if (!move_insert_mode)
			endim();
		vgoto(splitw ? WECHO : LINE(wdot - llimit), column(wcursor) - 1);
		flush();
		sleep(1);
		vgoto(l, c);
		if (i)
			goim();
	}
	else {
		strcLIN(sp);
		wcscpy(scurs, genbuf);
		if (!lmatchp((line *) 0))
			ex_beep();
	}
	strcLIN(sp);
	wdot = 0;
	wcursor = 0;
	cursor = scurs;
}

static int ltosolid(void)
{
	if (!*wcursor && !lnext())
		return (0);
	while (iswspace(*wcursor) || (*wcursor == 0))
		if (!lnext())
			return (0);
	return (1);
}

static lskipbal(char *parens)
{
	register int level = dir;
	register int c;

	do {
		if (!lnext()) {
			wdot = NOLINE;
			return (0);
		}
		c = *wcursor;
		if (c == parens[1])
			level--;
		else if (c == parens[0])
			level++;
	} while (level);
	return (1);
}

static int lskipatom(void)
{
	register int c;
	register wchar_t *cp;

	for (;;) {
		if (dir < 0 && wcursor == linebuf) {
			if (!lnext())
				return (0);
			break;
		}
		c = *wcursor;
		if (c && (iswspace(c) || any(c, "()")))
			break;
		if (!lnext())
			return (0);
		if (dir > 0 && wcursor == linebuf)
			break;
	}

	if (!*wcursor && !lnext())
		return (0);
	while (iswspace(*wcursor) || (*wcursor == 0))
		if (!lnext())
			return (0);
	return (1);
}

static int lskipsent()
{
	register int c;
	register wchar_t *cp;

	for (;;) {
		if (dir < 0 && wcursor == linebuf) {
			if (!lnext())
				return (0);
			break;
		}
		c = *wcursor;
		if (c && iswspace(c))
			break;
		if (!lnext())
			return (0);
		if (dir > 0 && wcursor == linebuf)
			break;
	}

	while (iswspace(*wcursor))
		if (!lnext())
			return (0);
	if (dir > 0)
		return (1);
	for (cp = wcursor; cp > linebuf; cp--)
		if (any(cp[-1], "!?."))
			break;
	wcursor = cp;
	return (1);
}

int lnext(void)
{

	if (dir > 0) {
		if (*wcursor)
			wcursor++;
		if (*wcursor)
			return (1);
		if (wdot >= llimit) {
			if (lf == (int (*)(void))vmove && wcursor > linebuf)
				wcursor--;
			return (0);
		}
		wdot++;
		getline(*wdot);
		wcursor = linebuf;
		return (1);
	} else {
		--wcursor;
		if (wcursor >= linebuf)
			return (1);
		if (lf == (int(*)(void))lindent && linebuf[0] == '(')
			llimit = wdot;
		if (wdot <= llimit) {
			wcursor = linebuf;
			return (0);
		}
		wdot--;
		getline(*wdot);
		wcursor = linebuf[0] == 0 ? linebuf : WCstrend(linebuf) - 1;
		return (1);
	}
}

int lbrack(int c, void (*f)(void))
{
	register line *addr;

	addr = dot;
	for (;;) {
		addr += dir;
		if (addr < one || addr > dol) {
			addr -= dir;
			break;
		}
		getline(*addr);
		if (linebuf[0] == '{' ||
		    value(LISP) && linebuf[0] == '(' ||
		    isa(svalue(SECTIONS))) {
			if (c == ']' && f != vmove) {
				addr--;
				getline(*addr);
			}
			break;
		}
		if (c == ']' && f != vmove && linebuf[0] == '}')
			break;
	}
	if (addr == dot)
		return (0);
	if (f != vmove)
		wcursor = c == ']' ? WCstrend(linebuf) : linebuf;
	else
		wcursor = 0;
	wdot = addr;
	vmoving = 0;
	return (1);
}

static int isa(char *cp)
{
	wchar_t ch0, ch1;

	if (linebuf[0] != '.')
		return (0);
	for(;;){
		cp += mbtowc(&ch0, cp, MB_CUR_MAX);
		cp += mbtowc(&ch1, cp, MB_CUR_MAX);
		if((ch0 == '\0') || (ch1 == '\0'))
			break;
		if (linebuf[1] == ch0) {
			if (linebuf[2] == ch1)
				return (1);
			if (linebuf[2] == 0 && ch1 == ' ')
				return (1);
		}
	}
	return (0);
}
