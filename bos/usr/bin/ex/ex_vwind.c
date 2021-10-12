#if !defined(lint)
static char sccsid [] = "@(#)06  1.13  src/bos/usr/bin/ex/ex_vwind.c, cmdedit, bos41J, 9512A_all 3/21/95 17:33:51";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_vwind.c
 *
 * FUNCTIONS: vback, vclean, vcontext, vcookit, vdepth, vdown, vfit, vjumpto,
 * vmoveto, vnline, vreset, vroll, vrollR, vshow, vup, vupdown
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

static void vreset(short);
static int vfit(line *, int);
static void vrollR(int);

/*
 * Routines to adjust the window, showing specified lines
 * in certain positions on the screen, and scrolling in both
 * directions.  Code here is very dependent on mode (open versus visual).
 */

/*
 * Move in a nonlocal way to line addr.
 * If it isn't on screen put it in specified context.
 * New position for cursor is curs.
 * Like most routines here, we vsave().
 */
void vmoveto(line *addr, wchar_t *curs, wchar_t context)
{

	markit(addr);
	vsave();
	vjumpto(addr, curs, context);
}

/*
 * Vjumpto is like vmoveto, but doesn't mark previous
 * context or save linebuf as current line.
 */
void vjumpto(line *addr, wchar_t *curs, wchar_t context)
{

	noteit((short)0);
	if (context != 0)
		vcontext(addr, context);
	else
		vshow(addr, NOLINE);
	noteit((short)1);
	vnline(curs);
	vlastshowp = NULL;	/* Force showmode at macro end */
}

/*
 * Go up or down cnt (negative is up) to new position curs.
 */
void vupdown(int cnt, wchar_t *curs)
{

	if (cnt > 0)
		vdown(cnt, 0, (short)0);
	else if (cnt < 0)
		vup(-cnt, 0, (short)0);
	if (vcnt == 0)
		vrepaint(curs);
	else
		vnline(curs);
}

/*
 * Go up cnt lines, afterwards preferring to be ind
 * logical lines from the top of the screen.
 * If scroll, then we MUST use a scroll.
 * Otherwise clear and redraw if motion is far.
 */
void vup(int cnt, int ind, short scroll)
{
	register int i, tot;

	if (dot == one) {
		ex_beep();
		return;
	}
	vsave();
	i = lineDOT() - 1;
	if (cnt > i) {
		ind -= cnt - i;
		if (ind < 0)
			ind = 0;
		cnt = i;
	}
	if (!scroll && cnt <= vcline) {
		vshow(dot - cnt, NOLINE);
		return;
	}
	vlastshowp=NULL;           /* Force showmode at macro end     */
	cnt -= vcline, dot -= vcline, vcline = 0;
	if (hold & HOLDWIG)
		goto contxt;
	if (state == VISUAL && !insert_line && !scroll_reverse &&
	    cnt <= WTOP - ZERO && vfit(dot - cnt, cnt) <= WTOP - ZERO)
		goto okr;
	tot = WECHO - ZERO;
	if (state != VISUAL || (!insert_line && !scroll_reverse) || (!scroll && (cnt > tot || vfit(dot - cnt, cnt) > tot / 3 + 1))) {
contxt:
		vclean();
		if (scroll) {
			/* to support ^Y on ibm3161 terminals */
			vshow(dot - cnt - vcline , dot - cnt - vcline);
			vcline += ind, dot += ind;
			if (vcline >= vcnt)
				dot -= vcline - vcnt + 1, vcline = vcnt - 1;
			getDOT();
		} else
			vshow(dot - cnt + ind, dot - cnt);
		return;
	}
okr:
	vrollR(cnt);
	if (scroll) {
		vcline += ind, dot += ind;
		if (vcline >= vcnt)
			dot -= vcline - vcnt + 1, vcline = vcnt - 1;
		getDOT();
	}
}

/*
 * Like vup, but scrolling down.
 */
void vdown(int cnt, int ind, short scroll)
{
	register int i, tot;

	if (dot == dol) {
		ex_beep();
		return;
	}
	if (scroll && (dot+cnt-ind > dol)) {
		ex_beep();
		return;
	}
	vsave();
	i = dol - dot;
	if (cnt > i) {
		ind -= cnt - i;
		if (ind < 0)
			ind = 0;
		cnt = i;
	}
	i = vcnt - vcline - 1;
	if (!scroll && cnt <= i) {
		vshow(dot + cnt, NOLINE);
		return;
	}
	vlastshowp=NULL;           /* Force showmode at macro end     */
	cnt -= i, dot += i, vcline += i;
	if (hold & HOLDWIG)
		goto dcontxt;
	if (!scroll) {
		tot = WECHO - WTOP;
		if (state != VISUAL || cnt - tot > 0 || vfit(dot, cnt) > tot / 3 + 1) {
dcontxt:
			vcontext(dot + cnt, (wchar_t)'.');
			return;
		}
	}
	if (cnt > 0)
		vroll(cnt);
	if (state == VISUAL && scroll) {
		vcline -= ind, dot -= ind;
		if (vcline < 0)
			dot -= vcline, vcline = 0;
		getDOT();
	}
}

/*
 * Show line addr in context where on the screen.
 * Work here is in determining new top line implied by
 * this placement of line addr, since we always draw from the top.
 */
void vcontext(line *addr, wchar_t where)
{
	register line *top;

	getline(*addr);
	if (state != VISUAL)
		top = addr;
	else switch (where) {

	case '^':
		addr = vback(addr, basWLINES - vdepth());
		getline(*addr);
		/* fall into ... */

	case '-':
		top = vback(addr, basWLINES - vdepth());
		getline(*addr);
		break;

	case '.':
		top = vback(addr, basWLINES / 2 - vdepth());
		getline(*addr);
		break;

	default:
		top = addr;
		break;
	}
	if (state == ONEOPEN && LINE(0) == WBOT)
		vup1();
	vcnt = vcline = 0;
	vclean();
	if (state == CRTOPEN)
		vup1();
	vshow(addr, top);
}

/*
 * Get a clean line.  If we are in a hard open
 * we may be able to reuse the line we are on
 * if it is blank.  This is a real win.
 */
void vclean(void)
{

	if (state != VISUAL && state != CRTOPEN) {
		destcol = 0;
		if (!ateopr())
			vup1();
		vcnt = 0;
	}
}

/*
 * Show line addr with the specified top line on the screen.
 * Top may be 0; in this case have vcontext compute the top
 * (and call us recursively).  Eventually, we clear the screen
 * (or its open mode equivalent) and redraw.
 */
void vshow(line *addr, line *top)
{
#ifndef CBREAK
	register short fried = 0;
#endif
	register int cnt = addr - dot;
	register int i = vcline + cnt;
	short oldhold = hold;

	if (state != HARDOPEN && state != ONEOPEN && i >= 0 && i < vcnt) {
		dot = addr;
		getDOT();
		vcline = i;
		return;
	}
	if (state != VISUAL) {
		dot = addr;
		vopen(dot, WBOT);
		return;
	}
	if (top == 0) {
		vcontext(addr, (wchar_t)'.');
		return;
	}
	dot = top;
#ifndef CBREAK
	if (vcookit(2))
		fried++, vcook();
#endif
	oldhold = hold;
	hold |= HOLDAT;
	vclear();
	vreset((short)0);
	vredraw(WTOP);
	/* error if vcline >= vcnt ! */
	vcline = addr - top;
	dot = addr;
	getDOT();
	hold = oldhold;
	vsync(LASTLINE);
#ifndef CBREAK
	if (fried)
		flusho(), vraw();
#endif
}

/*
 * reset the state.
 * If inecho then leave us at the beginning of the echo
 * area;  we are called this way in the middle of a :e escape
 * from visual, e.g.
 */
static
void vreset(short inecho)
{

	vcnt = vcline = 0;
	WTOP = basWTOP;
	WLINES = basWLINES;
	if (inecho)
		splitw = 1, vgoto(WECHO, 0);
}

/*
 * Starting from which line preceding tp uses almost (but not more
 * than) cnt physical lines?
 */
line *
vback(line *tp, int cnt)
{
	register int d;
	int ocnt = cnt;

	if (cnt > 0) {
		for (; tp > one; tp--) {
			getline(tp[-1]);
			d = vdepth();
			if (d > cnt) {
				/* defect 73931: if we weren't able to
				   move back enough to display some
				   new text on the screen, then we
				   will move back by one more line
				   just to make sure that the user
				   gets to see some more text */

				if (cnt + 1 == ocnt && tp > one)
					tp--;

				break;
			}
			cnt -= d;
		}
	}
	return (tp);
}

/*
 * How much scrolling will it take to roll cnt lines starting at tp?
 */
static
int vfit(line *tp, int cnt)
{
	register int j;

	j = 0;
	while (cnt > 0) {
		cnt--;
		getline(tp[cnt]);
		j += vdepth();
	}
	if (tp > dot)
		j -= WBOT - LASTLINE;
	return (j);
}

/*
 * Roll cnt lines onto the screen.
 */
void vroll(int cnt)
{
#ifndef CBREAK
	register short fried = 0;
#endif
	short oldhold = hold;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vroll(%d)\n", cnt);
#endif
	if (state != VISUAL)
		hold |= HOLDAT|HOLDROL;
	if (WBOT == WECHO) {
		vcnt = 0;
		if (state == ONEOPEN)
			vup1();
	}
#ifndef CBREAK
	if (vcookit(cnt))
		fried++, vcook();
#endif
	for (; cnt > 0 && Peekkey != ATTN; cnt--) {
		dot++, vcline++;
		vopen(dot, LASTLINE);
		vscrap();
	}
	hold = oldhold;
	if (state == HARDOPEN)
		sethard();
	vsyncCL();
#ifndef CBREAK
	if (fried)
		flusho(), vraw();
#endif
}

/*
 * Roll backwards (scroll up).
 */
static
void vrollR(int cnt)
{
#ifndef CBREAK
	register short fried = 0;
#endif
	short oldhold = hold;

#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vrollR(%d), dot=%d\n", cnt, lineDOT());
#endif
#ifndef CBREAK
	if (vcookit(cnt))
		fried++, vcook();
#endif
	if (WBOT == WECHO)
		vcnt = 0;
	heldech = 0;
	hold |= HOLDAT|HOLDECH;
	for (; cnt > 0 && Peekkey != ATTN; cnt--) {
		dot--;
		vopen(dot, WTOP);
		vscrap();
	}
	hold = oldhold;
	if (heldech)
		vclrech((short)0);
	vsync(LINE(vcnt-1));
#ifndef CBREAK
	if (fried)
		flusho(), vraw();
#endif
}

#ifndef CBREAK
/*
 * Go into cooked mode (allow interrupts) during
 * a scroll if we are at less than 1200 baud and not
 * a 'vi' command, or if we are in a 'vi' command and the
 * scroll is more than 2 full screens.
 *
 * BUG:		An interrupt during a scroll in this way
 *		dumps to command mode.
 */
static
int vcookit(int cnt)
{

	return (cnt > 1 && (ospeed < B1200 && !initev || cnt > lines * 2));
}
#endif

/*
 * Determine displayed depth of current line.
 */
int vdepth(void)
{
	register int d;

	d = (column(NOWCSTR) + WCOLS - 1 + (Putchar == (void(*)(int))listchar) + insert_null_glitch) / WCOLS;
#ifdef ADEBUG
	if (trace)
		tfixnl(), fprintf(trace, "vdepth returns %d\n", d == 0 ? 1 : d);
#endif
	return (d == 0 ? 1 : d);
}

/*
 * Move onto a new line, with cursor at position curs.
 */
void vnline(wchar_t *curs)
{

	if (curs)
		wcursor = curs;
	else if (vmoving)
		wcursor = vfindcol(vmovcol);
	else
		wcursor = vskipwh(linebuf);
	cursor = linebuf;
	vmove();
}
