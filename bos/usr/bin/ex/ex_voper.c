#if !defined(lint)
static char sccsid [] = "@(#)00  1.15  src/bos/usr/bin/ex/ex_voper.c, cmdedit, bos41J, 9514A_all 3/29/95 13:26:37";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_voper.c
 *
 * FUNCTIONS: edge, eend, find, margin, operate, word, wordch, wordof
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
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

#define blank() 	iswspace(wcursor[0])
#define forbid(a)	if (a) goto errlab;

static wchar_t	vscandir[2] =	{ '/', 0 };

int map(wchar_t, struct maps *, int *);

static int find(wchar_t);
static int edge(void);
static int margin(void);

/*
 * Decode an operator/operand type command.
 * Eventually we switch to an operator subroutine in ex_vops.c.
 * The work here is setting up a function variable to point
 * to the routine we want, and manipulation of the variables
 * wcursor and wdot, which mark the other end of the affected
 * area.  If wdot is zero, then the current line is the other end,
 * and if wcursor is zero, then the first non-blank location of the
 * other line is implied.
 */
void operate(wchar_t c, int cnt)
{
	register int i;
	void (*moveop)(void), (*deleteop)(void);
	void (*opf)(int);
	short subop = 0;
	wchar_t *oglobp, *ocurs;
	register line *addr;
	line *odot;
	static wchar_t lastFKND, lastFCHR;
	short d;

#ifdef TRACE
		if (trace)
			fprintf(trace, "entering operate\n");
#endif
	moveop = vmove, deleteop = (void(*)(void))vdelete;
	wcursor = cursor;
	wdot = NOLINE;
	notecnt = 0;
	dir = 1;

	switch (c) {

	/*
	 * d		delete operator.
	 */
	case 'd':
		moveop = (void (*)(void))vdelete;
		deleteop = ex_beep;
		break;

	/*
	 * s		substitute characters, like c\040, i.e. change space.
	 */
	case 's':
		ungetkey(' ');
		subop++;
		/* fall into ... */

	/*
	 * c		Change operator.
	 */
	case 'c':
		if (c == 'c' && workcmd[0] == 'C' || workcmd[0] == 'S')
			subop++;
		moveop = (void (*)(void))vchange;
		deleteop = ex_beep;
		break;

/* AIX security enhancement */
#if !defined(TVI)
	/*
	 * !		Filter through a UNIX command.
	 */
	case '!':
		moveop = vfilter;
		deleteop = ex_beep;
		break;
#endif
/* TCSEC Division C Class C2 */

	/*
	 * y		Yank operator.	Place specified text so that it
	 *		can be put back with p/P.  Also yanks to named buffers.
	 */
	case 'y':
		moveop = vyankit;
		deleteop = ex_beep;
		break;

	/*
	 * =		Reformat operator (for LISP).
	 */
	case '=':
		forbid(!value(LISP));
		/* fall into ... */

	/*
	 * >		Right shift operator.
	 * <		Left shift operator.
	 */
	case '<':
	case '>':
		moveop = vshftop;
		deleteop = ex_beep;
		break;

	/*
	 * r		Replace character under cursor with single following
	 *		character.
	 */
	case 'r':
		vmacchng((short)1);
		vrep(cnt);
		return;

	default:
		goto nocount;
	}
	vmacchng((short)1);
	/*
	 * Had an operator, so accept another count.
	 * Multiply counts together.
	 */
	if (iswdigit(peekkey()) && peekkey() != '0') {
		cnt *= vgetcnt();
		Xcnt = cnt;
		forbid (cnt <= 0);
	}

	/*
	 * Get next character, mapping it and saving as
	 * part of command for repeat.
	 */
	c = map((wchar_t)getesc(),arrows, NULL);
	if (c == 0)
		return;
	if (!subop)
		*lastcp++ = c;
nocount:
	opf = (void (*)(int))moveop;
	switch (c) {

	/*
	 * b		Back up a word.
	 * B		Back up a word, liberal definition.
	 */
	case 'b':
	case 'B':
		dir = -1;
		/* fall into ... */

	/*
	 * w		Forward a word.
	 * W		Forward a word, liberal definition.
	 */
	case 'W':
	case 'w':
		wdkind = c & ' ';
		forbid(lfind((short)2, cnt, (void (*)(void))opf, 0) < 0);
		vmoving = 0;
		break;

	/*
	 * E		to end of following blank/nonblank word
	 */
	case 'E':
		wdkind = 0;
		goto ein;

	/*
	 * e		To end of following word.
	 */
	case 'e':
		wdkind = 1;
ein:
		forbid(lfind((short)3, cnt, (void (*)(void))opf, 0) < 0);
		vmoving = 0;
		break;

	/*
	 * (		Back an s-expression.
	 */
	case '(':
		dir = -1;
		/* fall into... */

	/*
	 * )		Forward an s-expression.
	 */
	case ')':
		forbid(lfind((short)0, cnt, (void (*)(void))opf, (line *) 0) < 0);
		markDOT();
		break;

	/*
	 * {		Back an s-expression, but don't stop on atoms.
	 *		In text mode, a paragraph.  For C, a balanced set
	 *		of {}'s.
	 */
	case '{':
		dir = -1;
		/* fall into... */

	/*
	 * }		Forward an s-expression, but don't stop on atoms.
	 *		In text mode, back paragraph.  For C, back a balanced
	 *		set of {}'s.
	 */
	case '}':
		forbid(lfind((short)1, cnt, (void (*)(void))opf, (line *) 0) < 0);
		markDOT();
		break;

	/*
	 * %		To matching () or {}.  If not at ( or { scan for
	 *		first such after cursor on this line.
	 */
	case '%':
		vsave();
		i = lmatchp((line *) 0);
#ifdef TRACE
		if (trace)
			fprintf(trace, "after lmatchp in %, dot=%d, wdot=%d, dol=%d\n", lineno(dot), lineno(wdot), lineno(dol));
#endif
		getDOT();
		forbid(!i);
		if (opf != (void (*)(int))vmove)
			if (dir > 0)
				wcursor++;
			else
				cursor++;
		else
			markDOT();
		vmoving = 0;
		break;

	/*
	 * [		Back to beginning of defun, i.e. an ( in column 1.
	 *		For text, back to a section macro.
	 *		For C, back to a { in column 1 (~~ beg of function.)
	 */
	case '[':
		dir = -1;
		/* fall into ... */

	/*
	 * ]		Forward to next defun, i.e. a ( in column 1.
	 *		For text, forward section.
	 *		For C, forward to a } in column 1 (if delete or such)
	 *		or if a move to a { in column 1.
	 */
	case ']':
		if (!vglobp)
			forbid(getkey() != c);
		forbid (Xhadcnt);
		vsave();
		i = lbrack(c, (void (*)(void))opf);
		getDOT();
		forbid(!i);
		markDOT();
		if (ospeed > B300)
			hold |= HOLDWIG;
		break;

	/*
	 * ,		Invert last find with f F t or T, like inverse
	 *		of ;.
	 */
	case ',':
		forbid (lastFKND == 0);
		c = iswupper(lastFKND) ? towlower(lastFKND)
		    : towupper(lastFKND);
		i = lastFCHR;
		if (vglobp == 0)
			vglobp = WCemptystr;
		subop++;
		goto nocount;

	/*
	 * 0		To beginning of real line.
	 */
	case '0':
		wcursor = linebuf;
		vmoving = 0;
		break;

	/*
	 * ;		Repeat last find with f F t or T.
	 */
	case ';':
		forbid (lastFKND == 0);
		c = lastFKND;
		i = lastFCHR;
		subop++;
		goto nocount;

	/*
	 * F		Find single character before cursor in current line.
	 * T		Like F, but stops before character.
	 */
	case 'F':	/* inverted find */
	case 'T':
		dir = -1;
		/* fall into ... */

	/*
	 * f		Find single character following cursor in current line.
	 * t		Like f, but stope before character.
	 */
	case 'f':	/* find */
	case 't':
		if (!subop) {
			i = getesc();
			if (i == 0)
				return;
			*lastcp++ = i;
		}
		if (vglobp == 0)
			lastFKND = c, lastFCHR = i;
		for (; cnt > 0; cnt--)
			forbid (find((wchar_t)i) == 0);
		vmoving = 0;
		switch (c) {

		case 'T':
			wcursor++;
			break;

		case 't':
			wcursor--;
		case 'f':
fixup:
			if (moveop != vmove)
				wcursor++;
			break;
		}
		break;

	/*
	 * |		Find specified print column in current line.
	 */
	case '|':
		if (Pline == numbline)
			cnt += numb_offset();
		vmovcol = cnt;
		vmoving = 1;
		wcursor = vfindcol(cnt);
		break;

	/*
	 * ^		To beginning of non-white space on line.
	 */
	case '^':
		wcursor = vskipwh(linebuf);
		vmoving = 0;
		break;

	/*
	 * $		To end of line.
	 */
	case '$':
		if (opf == (void (*)(int))vmove) {
			vmoving = 1;
			vmovcol = 20000;
		} else
			vmoving = 0;
		if (cnt > 1) {
			if (opf == (void (*)(int))vmove) {
				wcursor = 0;
				cnt--;
			} else
				wcursor = linebuf;
			/* This is wrong at EOF */
			wdot = dot + cnt;
			break;
		}
		if (linebuf[0]) {
			wcursor = WCstrend(linebuf) - 1;
			goto fixup;
		}
		wcursor = linebuf;
		break;

	/*
	 * h		Back a character.
	 * ^H		Back a character.
	 */
	case 'h':
	case Ctrl('H'):
		dir = -1;
		/* fall into ... */

	/*
	 * space	Forward a character.
	 */
	case 'l':
	case ' ':
		forbid (margin() || opf == (void (*)(int))vmove && edge());
		while (cnt > 0 && !margin())
			wcursor += dir, cnt--;
		if (margin() && opf == (void (*)(int))vmove || 
                   wcursor < linebuf)
			wcursor -= dir;
		vmoving = 0;
		break;

	/*
	 * D		Delete to end of line, short for d$.
	 */
	case 'D':
		cnt = INF;
		goto deleteit;

	/*
	 * X		Delete character before cursor.
	 */
	case 'X':
		dir = -1;
		/* fall into ... */
deleteit:
	/*
	 * x		Delete character at cursor, leaving cursor where it is.
	 */
	case 'x':
		if (margin())
			goto errlab;
		vmacchng((short)1);
		while (cnt > 0 && !margin())
			wcursor += dir, cnt--;
		opf = (void(*)(int))deleteop;
		vmoving = 0;
		break;

	default:
		/*
		 * Stuttered operators are equivalent to the operator on
		 * a line, thus turn dd into d_.
		 */
		if (opf == (void(*)(int))vmove || c != workcmd[0]) {
errlab:
			ex_beep();
			vmacp = 0;

			if ( vflickp != NULL ) /* Mode change occurred */
			{
				if ( vflickp != vlastshowp ) /* Mode not same now */
					vshowmode( vflickp );
				vflickp = NULL;
			}
			return;
		}
		/* fall into ... */

	/*
	 * _		Target for a line or group of lines.
	 *		Stuttering is more convenient; this is mostly
	 *		for aesthetics.
	 */
	case '_':
		wdot = dot + cnt - 1;
		vmoving = 0;
		wcursor = 0;
		break;

	/*
	 * H		To first, home line on screen.
	 *		Count is for count'th line rather than first.
	 */
	case 'H':
		wdot = (dot - vcline) + cnt - 1;
		if (opf == (void (*)(int))vmove)
			markit(wdot);
		vmoving = 0;
		wcursor = 0;
		break;

	/*
	 * -		Backwards lines, to first non-white character.
	 */
	case '-':
		wdot = dot - cnt;
		vmoving = 0;
		wcursor = 0;
		break;

	/*
	 * ^P		To previous line same column.  Ridiculous on the
	 *		console of the VAX since it puts console in LSI mode.
	 */
	case 'k':
	case Ctrl('P'):
		wdot = dot - cnt;

		if (vmoving == 0) {
			vmoving = 1, vmovcol = column(cursor);
			/* On a double-wide character, use left most column */
			if (cursor && is_dblwid(*cursor))
				vmovcol -= wcwidth(*cursor) - 1;
		}
		wcursor = 0;
		break;

	/*
	 * L		To last line on screen, or count'th line from the
	 *		bottom.
	 */
	case 'L':
		wdot = dot + vcnt - vcline - cnt;
		if (opf == (void(*)(int))vmove)
			markit(wdot);
		vmoving = 0;
		wcursor = 0;
		break;

	/*
	 * M		To the middle of the screen.
	 */
	case 'M':
		wdot = dot + ((vcnt + 1) / 2) - vcline - 1;
		if (opf == (void(*)(int))vmove)
			markit(wdot);
		vmoving = 0;
		wcursor = 0;
		break;

	/*
	 * +		Forward line, to first non-white.
 	 *
	 * CR		Convenient synonym for +.
	 */
	case '+':
	case '\r':
		wdot = dot + cnt;
		vmoving = 0;
		wcursor = 0;
		break;

	/*
	 * ^N		To next line, same column if possible.
	 *
	 * LF		Linefeed is a convenient synonym for ^N.
	 */
	case Ctrl('N'):
	case 'j':
	case '\n':
		wdot = dot + cnt;

		if (vmoving == 0) {
			vmoving = 1, vmovcol = column(cursor);
			/* On a double-wide character, use left most column */
			if (cursor && is_dblwid(*cursor))
				vmovcol -= wcwidth(*cursor) - 1;
		}

		wcursor = 0;
		break;

	/*
	 * n		Search to next match of current pattern.
	 */
	case 'n':
		vglobp = vscandir;
		c = *vglobp++;
		goto nocount;

	/*
	 * N		Like n but in reverse direction.
	 */
	case 'N':
		{
			static wchar_t str_query[2] = { '?', '\0' };
			static wchar_t str_slash[2] = { '/', '\0' };

			vglobp = vscandir[0] == '/' ? str_query : str_slash;
			c = *vglobp++;
			goto nocount;
		}

	/*
	 * '		Return to line specified by following mark,
	 *		first white position on line.
	 *
	 * `		Return to marked line at remembered column.
	 */
	case '\'':
	case '`':
		d = c;
		c = getesc();
		if (c == 0)
			return;
		c = markreg(c);
		forbid (c == 0);
		wdot = getmark(c);
		forbid (wdot == NOLINE);
		forbid (Xhadcnt);
		vmoving = 0;
		wcursor = d == '`' ? ncols[c - 'a'] : 0;
		if (opf == (void(*)(int))vmove && 
                    (wdot != dot || (d == '`' && wcursor != cursor)))
			markDOT();
		if (wcursor) {
			vsave();
			getline(*wdot);
			if (wcursor > WCstrend(linebuf))
				wcursor = 0;
			getDOT();
		}
		if (ospeed > B300)
			hold |= HOLDWIG;
		break;

	/*
	 * G		Goto count'th line, or last line if no count
	 *		given.
	 */
	case 'G':
		if (!Xhadcnt)
			cnt = lineDOL();
		wdot = zero + cnt;
		forbid (wdot < one || wdot > dol);
		if (opf == (void(*)(int))vmove)
			markit(wdot);
		vmoving = 0;
		wcursor = 0;
		break;

	/*
	 * /		Scan forward for following re.
	 * ?		Scan backward for following re.
	 */
	case '/':
	case '?':
		forbid (Xhadcnt);
		vsave();
		ocurs = cursor;
		odot = dot;
		wcursor = 0;
		if (state == VISUAL && Outchar != (void (*)(int)) vputchar)
			Outchar = (void (*)(int)) vputchar;
		if (readecho(c))
			return;
		if (!vglobp)
			vscandir[0] = genbuf[0];
		oglobp = globp; CP(vutmp, genbuf); globp = vutmp;
		d = peekc;
fromsemi:
		ungetchar(0);
		fixech();
		CATCH
#ifndef CBREAK
			/*
			 * Lose typeahead (ick).
			 */
			vcook();
#endif
			addr = address(cursor);
#ifndef CBREAK
			vraw();
#endif
		ONERR
#ifndef CBREAK
			vraw();
#endif
slerr:
			globp = oglobp;
			dot = odot;
			cursor = ocurs;
			ungetchar(d);
			splitw = 0;
			vclean();
			vjumpto(dot, ocurs, (wchar_t)0);
			return;
		ENDCATCH
		if (globp == 0)
			globp = WCemptystr;
		else if (peekc)
			--globp;
		if (*globp == ';') {
			/* /foo/;/bar/ */
			globp++;
			dot = addr;
			cursor = loc1;
			goto fromsemi;
		}
		dot = odot;
		ungetchar(d);
		c = 0;
		if (*globp == 'z')
			globp++, c = '\n';
		if (any(*globp, "^+-."))
			c = *globp++;
		i = 0;
		while (iswdigit(*globp))
			i = i * 10 + *globp++ - '0';
		if (any(*globp, "^+-."))
			c = *globp++;
		if (*globp) {
			/* random junk after the pattern */
			ex_beep();
			goto slerr;
		}
		globp = oglobp;
		splitw = 0;
		vmoving = 0;
		wcursor = loc1;
		if (i != 0)
			vsetsiz(i);
		if (opf == (void(*)(int))vmove) {
			int need_full_refresh = 0;

			if (state == VISUAL && Outchar != (void (*)(int)) vputchar) {
				Outchar = (void (*)(int)) vputchar;
				need_full_refresh++;
			}
			if (state == ONEOPEN || state == HARDOPEN)
				outline = destline = WBOT;
			if (addr != dot || loc1 != cursor)
				markDOT();
			if (loc1 > linebuf && *loc1 == 0)
				loc1--;
			if (c)
				vjumpto(addr, loc1, c);
			else {
				vmoving = 0;
				if (loc1) {
					vmoving++;
					vmovcol = column(loc1);
				}
				getDOT();
				if (state == CRTOPEN && addr != dot)
					vup1();
				vupdown(addr - dot, NOWCSTR);
			}
			if (need_full_refresh) {
				/* We imitate a ^L refresh here so that the
				   screen is redrawn correctly after cases
				   where the echo line is forced to scroll,
				   like when we've searched for a really
				   long pattern. */

				vclear();
				vdirty(0, vcnt);
				vredraw(WTOP);
				if (vcnt == 0)
					vrepaint(cursor);
				vfixcurs();
			}
			return;
		}
		lastcp[-1] = 'n';
		getDOT();
		wdot = addr;
		break;
	}
	/*
	 * Apply.
	 */
	if (vreg && wdot == 0)
		wdot = dot;
#ifdef TRACE
		if (trace)
			fprintf(trace, "in operator, force wdot\n");
#endif
	(*opf)(c);
	wdot = NOLINE;
}

/*
 * Find single character c, in direction dir from cursor.
 */
static find(wchar_t c)
{

#ifdef TRACE
		if (trace)
			fprintf(trace, "entering find\n");
#endif
	for(;;) {
		if (edge())
			return (0);
		wcursor += dir;
		if (*wcursor == c)
			return (1);
	}
}

/*
 * Do a word motion with operator oprtr, and cnt more words
 * to go after this.
 */
int word(void (*oprtr)(void), int cnt)
{
	register int which;
	register wchar_t *iwc;
	register line *iwdot = wdot;

#ifdef TRACE
		if (trace)
			fprintf(trace, "entering word\n");
#endif
	if (dir == 1) {
		iwc = wcursor;
		which = wordch(wcursor);
		while (wordof((wchar_t)which, wcursor)) {
			if (cnt == 1 && 
                            oprtr != vmove && wcursor[1] == 0) {
				wcursor++;
				break;
			}
			if (!lnext())
				return (0);
			if (wcursor == linebuf)
				break;
		}
		/* Unless last segment of a change skip blanks */
		if (oprtr != (void (*)(void))vchange || cnt > 1) {
			while (!margin() && blank()) {
				if (!lnext())
					return (0);
			}
		} else {
			if (wcursor == iwc && iwdot == wdot && *iwc)
				wcursor++;
		}
	} else {
		if (!lnext())
			return (0);
		while (blank())
			if (!lnext())
				return (0);
		if (!margin()) {
			which = wordch(wcursor);
			while (!margin() && wordof((wchar_t)which, wcursor))
				wcursor--;
			if (wcursor < linebuf || !wordof((wchar_t)which, wcursor))
				wcursor++;
		}
	}
	return (1);
}

/*
 * To end of word, with operator oprtr and cnt more motions
 * remaining after this.
 */
void eend(void (*oprtr)(void))
{
	register int which;

#ifdef TRACE
		if (trace)
			fprintf(trace, "entering eend\n");
#endif
	if (!lnext())
		return;
	while (blank())
		if (!lnext())
			return;
	if (!margin()) {
		which = wordch(wcursor);
		while (!margin() && wordof((wchar_t)which, wcursor)) {
			if (wcursor[1] == 0) {
				wcursor++;
				break;
			}
			if (!lnext())
				return;
		}
	}
	if (oprtr != (void(*)(void))vchange && 
            oprtr != (void(*)(void))vdelete && wcursor > linebuf)
		wcursor--;
}

/*
 * Wordof tells whether the character at *wc is in a word of
 * kind which (blank/nonblank words are 0, conservative words 1).
 */
int wordof(wchar_t which, wchar_t *wc)
{
#ifdef TRACE
		if (trace)
			fprintf(trace, "entering wordof\n");
#endif

	if (iswspace(*wc))
		return (0);
	return (!wdkind || wordch(wc) == which);
}

/*
 * Wordch tells whether character at *wc is a word character
 * i.e. any graphical printing characters other than punct characters,
 * plus underscore.
 */
int wordch(wchar_t *wc)
{
	register wchar_t c;

#ifdef TRACE
		if (trace)
			fprintf(trace, "entering wordch\n");
#endif
	c = wc[0];
/* N.B. For English locales, iswgraph && !iswpunct is
	equivalent to iswalnum */
	return ( c == '_' || (iswgraph(c) && !iswpunct(c)));
}

/*
 * Edge tells when we hit the last character in the current line.
 */
static edge(void)
{

#ifdef TRACE
		if (trace)
			fprintf(trace, "entering edge\n");
#endif
	if (linebuf[0] == 0)
		return (1);
	if (dir == 1)
		return (wcursor[1] == 0);
	else
		return (wcursor == linebuf);
}

/*
 * Margin tells us when we have fallen off the end of the line.
 */
static int margin(void)
{
#ifdef TRACE
		if (trace)
			fprintf(trace, "entering margin\n");
#endif

	return (wcursor < linebuf || wcursor[0] == 0);
}
