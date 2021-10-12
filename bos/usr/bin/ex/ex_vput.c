#if !defined(lint)
static char sccsid [] = "@(#)05  1.18  src/bos/usr/bin/ex/ex_vput.c, cmdedit, bos411, 9439B411a 9/27/94 15:45:49";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_vput.c
 *
 * FUNCTIONS: enddm, endim, fixech, godm, goim, physdc, tfixnl, tracec,
 * tvierror, tvliny, vclear, vclrcp, vclrech, vclreol, vclrlin, vcsync, 
 * vcursaft, vcursat, vcursbef, vfixcurs, vgotab, vgoto, vgotoCL, vigoto,
 * vigotoCL, viin, vinschar, vishft, vmaktop, vneedpos, vnpins, vprepins,
 * vputch, vputchar, vrigid, vsetcurs, vshowmode
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
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

extern unsigned nerrors;

static void vmaktop(int, wchar_t *);
static void vneedpos(int);
static void vnpins(int);
static void vishft(void);
static void viin(int);
static void godm(void);
static void enddm(void);
static void vgotab(void);
static void vrigid(void);

/*
 * Deal with the screen, clearing, cursor positioning, putting characters
 * into the screen image, and deleting characters.
 * Really hard stuff here is utilizing insert character operations
 * on intelligent terminals which differs widely from terminal to terminal.
 */
void
vclear(void)
{

#ifdef TRACE
	if (trace)
		tfixnl(), fprintf(trace, "------\nvclear, clear_screen '%s'\n", clear_screen);
#endif
	tputs((char *)clear_screen, lines, (int(*)(int))putch);
	destcol = 0;
	outcol = 0;
	destline = 0;
	outline = 0;
	if (inopen)
		vclrcp(vtube0, WCOLS * (WECHO - ZERO + 1));
	nerrors = 0;
}

/*
 * Clear code points.
 */
void vclrcp(wchar_t *cp, int i)
{

	if (i > 0)
		do
			*cp++ = 0;
		while (--i != 0);
}

/*
 * Clear a physical display line, high level.
 */
void vclrlin(int l, line *tp)
{

	vigoto(l, 0);
	if ((hold & HOLDAT) == 0)
		ex_putchar(tp > dol ? ((UPPERCASE || tilde_glitch) ? '^' : '~') : '@');
	if (state == HARDOPEN)
		sethard();
	vclreol();
}

/*
 * Clear to the end of the current physical line
 */
void vclreol(void)
{
	register int i, j;
	register wchar_t *tp;

#ifdef ADEBUG
	if (trace)
		fprintf(trace, "vclreol(), destcol %d, ateopr() %d\n", destcol, ateopr());
#endif
	if (destcol == WCOLS)
		return;
	destline += destcol / WCOLS;
	destcol %= WCOLS;
	if (destline < 0 || destline > WECHO)
		error(MSGSTR(M_249, "Line too long for current screen size."), DUMMY_INT);
	i = WCOLS - destcol;
	tp = vtube[destline] + destcol;
	if (clr_eol) {
		if (insert_null_glitch && *tp || !ateopr()) {
			vcsync();
			vputp((char *)clr_eol, 1);
		}
		vclrcp(tp, i);
		return;
	}
	if (*tp == 0)
		return;
	while (i > 0 && (j = *tp)) {
		/* if (j != ' ' && (j & QUOTE) == 0) destcol... */
		if (j != ' ' && j != QUOTE_NUL && j != QUOTE_SP) {
			destcol = WCOLS - i;
			vputchar((wchar_t)' ');
		}
		--i, *tp++ = 0;
	}
}

/*
 * Clear the echo line.
 * If didphys then its been cleared physically (as
 * a side effect of a clear to end of display, e.g.)
 * so just do it logically.
 * If work here is being held off, just remember, in
 * heldech, if work needs to be done, don't do anything.
 */
void vclrech(short didphys)
{

#ifdef ADEBUG
	if (trace)
		fprintf(trace, "vclrech(%d), Peekkey %d, hold %o\n", didphys, Peekkey, hold);
#endif
	if (Peekkey == ATTN)
		return;
	if (hold & HOLDECH) {
		heldech = !didphys;
		return;
	}
	if (!didphys && (clr_eos || clr_eol)) {
		splitw++;
		/*
		 * If display is retained below, then MUST use clr_eos or
		 * clr_eol since we don't really know whats out there.
		 * Vigoto might decide (incorrectly) to do nothing.
		 */
		if (memory_below) {
			vgoto(WECHO, 0);
			/*
			 * This is tricky.  If clr_eos is as cheap we
			 * should use it, so we don't have extra junk
			 * floating around in memory below.  But if
			 * clr_eol costs less we should use it.  The real
			 * reason here is that clr_eos is incredibly
			 * expensive on the HP 2626 (1/2 second or more)
			 * which makes ^D scroll really slow.  But the
			 * 2621 has a bug that shows up if we use clr_eol
			 * instead of clr_eos, so we make sure the costs
			 * are equal so it will prefer clr_eol.
			 */
			if (costCE < costCD)
				vputp((char *)clr_eol, 1);
			else
				vputp((char *)clr_eos, 1);
		} else {
			if (teleray_glitch) {
				/* This code basically handles the t1061
				 * where positioning at (0, 0) won't work
				 * because the terminal won't let you put
				 * the cursor on it's magic cookie.
				 *
				 * Should probably be ceol_standout_glitch
				 * above, or even a
				 * new glitch, but right now t1061 is the
				 * only terminal with teleray_glitch.
				 */
				vgoto(WECHO, 0);
				vputp((char *)delete_line, 1);
			} else {
				vigoto(WECHO, 0);
				vclreol();
			}
		}
		splitw = 0;
		didphys = 1;
	}
	if (didphys)
		vclrcp(vtube[WECHO], WCOLS);
	heldech = 0;
	nerrors = 0;
}

/*
 * Fix the echo area for use, setting
 * the state variable splitw so we wont rollup
 * when we move the cursor there.
 */
void
fixech(void)
{

	splitw++;
	if (state != VISUAL && state != CRTOPEN) {
		vclean();
		vcnt = 0;
	}
	vgoto(WECHO, 0); flusho();
	nerrors = 0;
}

/*
 * Put the cursor ``before'' cp.
 */
void vcursbef(wchar_t *cp)
{

	if (cp <= linebuf)
		vgotoCL(value(NUMBER)?numb_offset():0);
	else
		vgotoCL(column(cp - 1) - 1);
}

/*
 * Put the cursor ``at'' cp.
 */
void vcursat(wchar_t *cp)
{

	if (cp <= linebuf && linebuf[0] == 0)
		vgotoCL(value(NUMBER)?numb_offset():0);
	else
		vgotoCL(column(cp - 1));
}

/*
 * Put the cursor ``after'' cp.
 */
void vcursaft(wchar_t *cp)
{

	vgotoCL(column(cp));
}

/*
 * Fix the cursor to be positioned in the correct place
 * to accept a command.
 */
void vfixcurs(void)
{

	vsetcurs(cursor);
}

/*
 * Compute the column position implied by the cursor at ``nc'',
 * and move the cursor there.
 */
void vsetcurs(wchar_t *nc)
{
	register int col;
	int	len;

	col = column(nc);
	if (linebuf[0]) {
		if (nc == NULL)
			nc = WCstrend(linebuf) - 1;
		col -= (((len = wcwidth(*nc)) == -1) ? 1 : len);
	}
	vgotoCL(col);
	cursor = nc;
}
/* * Move the cursor invisibly, i.e. only remember to do it.  */
void vigoto(int y, int x)
{

	destline = y;
	destcol = x;
}

/*
 * Move the cursor to the position implied by any previous
 * vigoto (or low level doing things with destcol/destline as in readecho).
 */
void vcsync(void)
{

	vgoto(destline, destcol);
}

/*
 * Goto column x of the current line.
 */
void vgotoCL(int x)
{

	if (splitw)
		vgoto(WECHO, x);
	else
		vgoto(LINE(vcline), x);
}

/*
 * Invisible goto column x of current line.
 */
static
vigotoCL(int x)
{

	if (splitw)
		vigoto(WECHO, x);
	else
		vigoto(LINE(vcline), x);
}

/*
 * Show the current mode in the right hand part of the echo line,
 * then return the cursor to where it is now.
 */
void vshowmode(char *txt)
{
	int savecol, saveline, savesplit;
	char *p;
	int	len;
	wchar_t	wch;

	if (!value(SHOWMODE))
		return;
	/* Don't flash it for ".", macros, etc. */
	if ( vmacp )
	{
		vflickp = txt;	/* -> New mode to show */
		return;
	}
	/* Don't flash it if vglobp is set */
	if (vglobp)
		if (*txt)
			return;
	savecol = outcol; saveline = outline; savesplit = splitw;
	splitw = 1;	/* To avoid scrolling */
	vigoto(WECHO, WCOLS-20);

	if (*txt) {
		vcsync();
		for (p=txt; *p;) {
			if ((len = mbtowc(&wch, p, MB_CUR_MAX)) < 1)
				break;
			p += len;
			vputchar(wch);
		}
	} else {
		/*
		 * Going back to command mode - clear the message.
		 */
		vclreol();
	}

	vlastshowp = txt; 	/* -> Currently showing mode */
	FLAGS(WECHO) |= VDIRT;
	vgoto(saveline, savecol);
	splitw = savesplit;
}

/*
 * Move cursor to line y, column x, handling wraparound and scrolling.
 */
void vgoto(int y, int x)
{
	register wchar_t *tp;
	register int c;
	int 	len;

	/*
	 * Fold the possibly too large value of x.
	 */
#ifdef FDEBUG
	if (trace) {
		fprintf(trace, "Before vgoto MSGSTR x %d, y %d, WCOLS %d\n, ",
			x, y, WCOLS);
	}
#endif
	if (x >= WCOLS) {
		y += x / WCOLS;
		x %= WCOLS;
	}
#ifdef FDEBUG
	if (trace) {
		fprintf(trace, "At vgoto MSGSTR x %d, y %d, WCOLS %d\n, ",
			x, y, WCOLS);
	}
#endif
	if (y < 0)
		error(MSGSTR(M_249, "Line too long for current screen size."), DUMMY_INT);
	if (outcol >= WCOLS) {
		if (auto_right_margin) {
			outline += outcol / WCOLS;
			outcol %= WCOLS;
		} else
			outcol = WCOLS - 1;
	}

	/*
	 * In a hardcopy or glass crt open, print the stuff
	 * implied by a motion, or backspace.
	 */
	if (state == HARDOPEN || state == ONEOPEN) {
		if (y != outline)
			error(MSGSTR(M_250, "Line too long for open"), DUMMY_INT);
		if (x + 1 < outcol - x || (outcol > x && !cursor_left))
			destcol = 0, fgoto();
		tp = vtube[WBOT] + outcol;
		while (outcol != x)
			if (outcol < x) {
				if (*tp == 0)
					*tp = ' ';
				c = *tp;
				if (c == QUOTE_NUL || c == QUOTE_SP)
					c = ' ';
				if (over_strike && !erase_overstrike)
					c = ' ';
				if (c == QUOTE_WCD) {
					vputp(cursor_left, 0);
					--outcol;
					--tp;
				} else {
					vputc(c);
					outcol += (((len = wcwidth(c)) == -1) ? 1 : len);
					tp += (((len = wcwidth(c)) == -1) ? 1 : len);
				}
			} else {
				vputp((char *)cursor_left, 0);
				outcol--;
			}
		destcol = outcol = x;
		destline = outline;
		return;
	}

	/*
	 * If the destination position implies a scroll, do it.
	 */
	destline = y;
	if (destline > WBOT && (!splitw || destline > WECHO)) {
		endim();
		vrollup(destline);
	}

	/*
	 * If there really is a motion involved, do it.
	 * The check here is an optimization based on profiling.
	 */
	destcol = x;
	if ((destline - outline) * WCOLS != destcol - outcol) {
		if (!move_insert_mode)
			endim();
		fgoto();
	}
}

/* AIX security enhancement */
#if defined(TVI)

/*
 * 	This code added to simplify posting of messages for TVI concerning
 *	the removal of function in visual mode.
 */
void	tvierror(char *str, char *cp)
{
	int savecol, saveline, savesplit;

	savecol = outcol;
	saveline = outline;
	savesplit = splitw;
	splitw = 1;
	vigoto(WECHO, 0);
	smerror(str, cp);
	vgoto(saveline, savecol);
	splitw = savesplit;
}
#endif
/* TCSEC Division C Class C2 */

/*
 * This is the hardest code in the editor, and deals with insert modes
 * on different kinds of intelligent terminals.  The complexity is due
 * to the cross product of three factors:
 *
 *	1. Lines may display as more than one segment on the screen.
 *	2. There are 2 kinds of intelligent terminal insert modes.
 *	3. Tabs squash when you insert characters in front of them,
 *	   in a way in which current intelligent terminals don't handle.
 *
 * The two kinds of terminals are typified by the DM2500 or HP2645 for
 * one and the CONCEPT-100 or the FOX for the other.
 *
 * The first (HP2645) kind has an insert mode where the characters
 * fall off the end of the line and the screen is shifted rigidly
 * no matter how the display came about.
 *
 * The second (CONCEPT-100) kind comes from terminals which are designed
 * for forms editing and which distinguish between blanks and ``spaces''
 * on the screen, spaces being like blank, but never having had
 * and data typed into that screen position (since, e.g. a clear operation
 * like clear screen).	On these terminals, when you insert a character,
 * the characters from where you are to the end of the screen shift
 * over till a ``space'' is found, and the null character there gets
 * eaten up.
 *
 *
 * The code here considers the line as consisting of several parts
 * the first part is the ``doomed'' part, i.e. a part of the line
 * which is being typed over.  Next comes some text up to the first
 * following tab.  The tab is the next segment of the line, and finally
 * text after the tab.
 *
 * We have to consider each of these segments and the effect of the
 * insertion of a character on them.  On terminals like HP2645's we
 * must simulate a multi-line insert mode using the primitive one
 * line insert mode.  If we are inserting in front of a tab, we have
 * to either delete characters from the tab or insert white space
 * (when the tab reaches a new spot where it gets larger) before we
 * insert the new character.
 *
 * On a terminal like a CONCEPT our strategy is to make all
 * blanks be displayed, while trying to keep the screen having ``spaces''
 * for portions of tabs.  In this way the terminal hardward does some
 * of the work for compression of tabs, although this tends to
 * disappear as you work on the line and spaces change into blanks.
 *
 * There are a number of boundary conditions (like typing just before
 * the first following tab) where we can avoid a lot of work.  Most
 * of them have to be dealt with explicitly because performance is
 * much, much worse if we don't.
 *
 * A final thing which is done here is two flavors of insert mode.
 * Datamedia's do this by an insert mode which you enter and leave
 * and by having normal motion character operate differently in this
 * mode, notably by having a newline insert a line on the screen in
 * this mode.  This generally means it is unsafe to move around
 * the screen ignoring the fact that we are in this mode.
 * This is possible on some terminals, and wins big (e.g. HP), so
 * we encode this as a ``can move in insert capability'' mi,
 * and terminals which have it can do insert mode with much less
 * work when tabs are present following the cursor on the current line.
 */

/*
 * Routine to expand a tab, calling the normal Outchar routine
 * to put out each implied character.  Note that we call outchar
 * with a QUOTE_NUL.  We use QUOTE_NUL internally to represent a position
 * which is part of the expansion of a tab.
 */
static
void vgotab(void)
{
	register int i = tabcol(destcol, value(TABSTOP)) - destcol;

	do {
		(*Outchar)(QUOTE_NUL);
	} while (--i);
}

/*
 * Variables for insert mode.
 */
static int	linend;		/* The column position of end of line */
static int	tabstart;	/* Column of start of first following tab */
static int	tabend;		/* Column of end of following tabs */
static int	tabsize;	/* Size of the following tabs */
static int	tabslack;	/* Number of ``spaces'' in following tabs */
static int	inssiz;		/* Number of characters to be inserted */
static int	inscol;		/* Column where insertion is taking place */
static int	shft;		/* Amount tab expansion shifted rest of line */
static int	slakused;	/* This much of tabslack will be used up */

/*
 * This routine MUST be called before insert mode is run,
 * and brings all segments of the current line to the top
 * of the screen image buffer so it is easier for us to
 * maniuplate them.
 */
void vprepins(void)
{
	register int i;
	register wchar_t *cp = vtube0;

	for (i = 0; i < DEPTH(vcline); i++) {
		vmaktop(LINE(vcline) + i, cp);
		cp += WCOLS;
	}
}

static
void vmaktop(int p, wchar_t *cp)
{
	register int i;
	wchar_t temp[TUBECOLS];

	if (p < 0 || vtube[p] == cp)
		return;
	for (i = ZERO; i <= WECHO; i++)
		if (vtube[i] == cp) {
			copy(temp, vtube[i], sizeof(wchar_t) * WCOLS);
			copy(vtube[i], vtube[p], sizeof(wchar_t) * WCOLS);
			copy(vtube[p], temp, sizeof(wchar_t) * WCOLS);
			vtube[i] = vtube[p];
			vtube[p] = cp;
			return;
		}
	error(MSGSTR(M_251, "Line too long"), DUMMY_INT);
}

/*
 * Insert character c at current cursor position.  Numerous cases to check.
 * Inserting a double-wide character in the right-most display column
 * is signified by displaying a partial character indicator and the character
 * at the start of the next display row.  When inserting a character causes
 * other parts of the line to shift over, partial character indicator(s) in
 * the right-most display column vanish.  Double-wide characters shifted to
 * the right-most display column are signified with the partial character
 * indicator.  Other multi-character inserts result as the expansion of tabs
 * (i.e. inssiz == wcwidth(c) except for tabs).
 * Code assumes this in several places to make life simpler.
 */
/*! In fact, it used to be that inssiz == 1 for non-tabs, which assumption may still be hidden here. !*/

void vinschar(int c)
{
	register int i;
	register wchar_t *tp;
	int nshifting;	/* columns shifting over */
	int ntabpos;	/* columns tab occupies on screen */
	int	len;
	wchar_t ttube[TUBESIZE + LBSIZE], *p, *tt = ttube;

/*
 * output a partial character indicator for double-wide character in the
 * last display column
 */
	if (is_dblwid(c) && (destcol % WCOLS == WCOLS - 1))
               if(( c != QUOTE_BULLET) && ( c != QUOTE_SP) &&
		  ( c != QUOTE_WCD) && ( c!= QUOTE_NUL))
			vinschar(QUOTE_BULLET);

	if ((!enter_insert_mode || !exit_insert_mode) && ((hold & HOLDQIK) || !value(REDRAW) || value(SLOWOPEN))) {
		/*
		 * Don't want to try to use terminal
		 * insert mode, or to try to fake it.
		 * Just put the character out; the screen
		 * will probably be wrong but we will fix it later.
		 */
		if (c == '\t') {
			vgotab();
			return;
		}
		vputchar((wchar_t)c);
		if (DEPTH(vcline) * WCOLS + !value(REDRAW) >
		    (destline - LINE(vcline)) * WCOLS + destcol)
			return;
		/*
		 * The next line is about to be clobbered
		 * make space for another segment of this line
		 * (on an intelligent terminal) or just remember
		 * that next line was clobbered (on a dumb one
		 * if we don't care to redraw the tail.
		 */
		if (insert_line) {
			vnpins(0);
		} else {
			c = LINE(vcline) + DEPTH(vcline);
			if (c < LINE(vcline + 1) || c > WBOT)
				return;
			i = destcol;
			vinslin(c, 1, vcline);
			DEPTH(vcline)++;
			vigoto(c, i);
			vprepins();
		}
		return;
	}
	/*
	 * Compute the number of positions in the line image of the
	 * current line.  This is done from the physical image
	 * since that is faster.  Note that we have no memory
	 * from insertion to insertion so that routines which use
	 * us don't have to worry about moving the cursor around.
	 */
	if (*vtube0 == 0)
		linend = 0;
	else {
		/*
		 * Search backwards for a non-null character
		 * from the end of the displayed line.
		 */
		i = WCOLS * DEPTH(vcline);
		if (i == 0)
			i = WCOLS;
		tp = vtube0 + i;
		while (*--tp == 0)
			if (--i == 0)
				break;
		linend = i;
	}

	/*
	 * We insert at a position based on the physical location
	 * of the output cursor.
	 */
	inscol = destcol + (destline - LINE(vcline)) * WCOLS;
	if (c == '\t') {
		/*
		 * Characters inserted from a tab must be
		 * remembered as being part of a tab, but we can't
		 * use QUOTE_NUL here since we really need to print blanks.
		 * QUOTE_SP is the representation of this.
		 */
		inssiz = tabcol(inscol, value(TABSTOP)) - inscol;
		c = QUOTE_SP;
	} else{
/*! We hope that c is never a quoted character here -- prove this? !*/
/* wrong !!! test for the quoted char QUOTE_BULLET */

		inssiz = (((len = wcwidth(c)) == -1) ? 1 : len);
		if( c == QUOTE_BULLET)
			inssiz = 1;
	}

	/*
	 * insert a single column character over the partial
	 * character indicator.
	 */
	 if (inssiz == 1 && inscol &&  vtube0[inscol-1] == QUOTE_BULLET) {
		destline--;
		destcol = WCOLS-1;
		/*! doom'ing accounts for partial character indicator? !*/
		if (doomed)
			doomed--;
		vputchar((wchar_t)c);
		return;
	}

	/*
	 * If the text to be inserted is less than the number
	 * of doomed positions, then we don't need insert mode,
	 * rather we can just typeover.
	 */
	if (inssiz <= doomed) {
		endim();
		if (inscol != linend)
			doomed -= inssiz;
		do {
			vputchar((wchar_t)c);
			inssiz -= (((len = wcwidth(c)) == -1) ? 1 : len);
		} while (inssiz);
		return;
	}

	/* insert causes characters to be shifted over */
	nshifting = inssiz - doomed;

	/*
	 * form desired screen image in ttube.
	 * When no more characters are shifting over,
	 * compare ttube to vtube and optimize updating of screen.
	 *
	 * Accounts for tabs being squished, expanded and shifted over.
	 *
	 * This code handles double-wide character replacing a single doomed
	 * position.
	 *
	 * This code adjusts for the partial character indicator representing
	 * a double-wide character that would have been in the last display
	 * column.  Also, the partial character indicator vanishes when a
	 * double-wide character is shifted over.
	 */
	/*
	 * Worst case is updating the line twice for a double-wide
	 * in the last column.  This case handled at start of vinschar()
	 * by passing QUOTE_BULLET to vinschar().
	 */
	/* c may be QUOTE_SP for inserting a tab */

	/* copy inserted character to ttube */
	for (i = inssiz; i > 0;) {
		*tt++ = c;
                i--;
                if (is_dblwid(c) && ( c != QUOTE_BULLET) && ( c != QUOTE_SP)
                   && ( c != QUOTE_WCD) && ( c != QUOTE_NUL)){
			*tt++ = QUOTE_WCD;
			i --;
		}
	}
	p = vtube0+inscol + doomed;
	doomed = 0;
	/* copy rest of line to ttube, properly shifting characters */
	/* Loop invariant: tt - ttube == p - (vtube0+inscol) + nshifting */
	while (nshifting) {
		if (p >= vtube0 + linend) {
			/* nshifting == (tt - ttube) - (linend - inscol) */
			vneedpos(nshifting);
			/* p += nshifting;  nshifting = 0; */
			break;
		}
		switch (*p) {
		case QUOTE_NUL:
		case QUOTE_SP:
			/* squish or expand tab */
			ntabpos = tabcol(p-vtube0,value(TABSTOP)) - (p-vtube0);
			p += ntabpos;
			if (ntabpos > nshifting) {
				/* squish tab */
				ntabpos -= nshifting;
				nshifting = 0;
			} else {
				/* recompute tab */
				i = (p-vtube0)-ntabpos+nshifting;
				i = tabcol(i, value(TABSTOP)) - i - ntabpos;
				nshifting += i;
				ntabpos += i;
			}
			for (; ntabpos; ntabpos--)
				*tt++ = QUOTE_SP;
			break;
		case QUOTE_BULLET:
			/* will be swallowed, since nshifting is positive */
			nshifting--;
			p++;
			break;
		default:
			if (is_dblwid(*p) &&
			    ((p - vtube0 + nshifting) % WCOLS == WCOLS - 1)) {
				/*
				 * QUOTE_BULLET indicates that double-wide
				 * character would be in right-most column
				 */

				*tt++ = QUOTE_BULLET;
				nshifting++;
			}
			*tt++ = *p++;
			break;
		}
	}

	vigotoCL(inscol);
	endim();
	/*
	 * vputchar() compares ttube value to character on screen,
	 * updating screen as needed.
	 */
	for (p = ttube; p < tt; p++)
		if (*p != QUOTE_WCD)          /* optimization */
			vputchar(*p);
		
	/*
	 * Now put the cursor in its final resting place.
	 */
	destline = LINE(vcline);
	destcol = inscol + inssiz;
	vcsync();
}

/*
 * Rigidify the rest of the line after the first
 * group of following tabs, typing blanks over ``spaces''.
 */
static
void vrigid(void)
{
	register int col;
	register wchar_t *tp = vtube0 + tabend;

	for (col = tabend; col < linend; col++, tp++)
		if (*tp == QUOTE_NUL || *tp == 0) {
			endim();
			vgotoCL(col);
			vputchar((wchar_t)QUOTE_SP);
		}
}

/*
 * We need cnt more positions on this line.
 * Open up new space on the screen (this may in fact be a
 * screen rollup).
 *
 * On a dumb terminal we may infact redisplay the rest of the
 * screen here brute force to keep it pretty.
 */
static
void vneedpos(int cnt)
{
	register int d = DEPTH(vcline);
	register int rmdr = d * WCOLS - linend;

	if (cnt <= rmdr - insert_null_glitch)
		return;
	endim();
	vnpins(1);
}

static
void vnpins(int dosync)
{
	register int d = DEPTH(vcline);
	register int e;

	e = LINE(vcline) + DEPTH(vcline);
	if (e < LINE(vcline + 1)) {
		vigoto(e, 0);
		vclreol();
		return;
	}
	DEPTH(vcline)++;
	if (e < WECHO) {
		e = vglitchup(vcline, d);
		vigoto(e, 0); vclreol();
		if (dosync) {
			void (*Ooutchar)(int) = Outchar;
			Outchar = (void (*)(int))vputchar;
			vsync(e + 1);
			Outchar = Ooutchar;
		}
	} else {
		vup1();
		vigoto(WBOT, 0);
		vclreol();
	}
	vprepins();
}

/*
 * Do the shift of the next tabstop implied by
 * insertion so it expands.
 */
static
void vishft(void)
{
	int tshft = 0;
	int j;
	register int i;
	register wchar_t *tp = vtube0;
	register wchar_t *up;
	short oldhold = hold;

	shft = value(TABSTOP);
	hold |= HOLDPUPD;
	if (!enter_insert_mode && !exit_insert_mode) {
		/*
		 * Dumb terminals are easy, we just have
		 * to retype the text.
		 */
		vigotoCL(tabend + shft);
		up = tp + tabend;
		for (i = tabend; i < linend; i++)
			vputchar(*up++);
	} else if (insert_null_glitch) {
		/*
		 * CONCEPT-like terminals do most of the work for us,
		 * we don't have to muck with simulation of multi-line
		 * insert mode.  Some of the shifting may come for free
		 * also if the tabs don't have enough slack to take up
		 * all the inserted characters.
		 */
		i = shft;
		slakused = inssiz - doomed;
		if (slakused > tabslack) {
			i -= slakused - tabslack;
			slakused -= tabslack;
		}
		if (i > 0 && tabend != linend) {
			tshft = i;
			vgotoCL(tabend);
			goim();
			do {
				vputchar((wchar_t)QUOTE_SP);
			} while (--i);
		}
	} else {
		/*
		 * HP and Datamedia type terminals have to have multi-line
		 * insert faked.  Do something with each segment after 
                 * where we are
		 * (going backwards to where we are.)  We then can
		 * do something with the segment where the end of the 
                 * first following
		 * tab group is.
		 */
		int jtop = tabstart / WCOLS + 1;
		int shfter = shft;
		for (j = (linend + shft -1) / WCOLS + 1; j > jtop; j--) {
			if (j - 1 == jtop && inscol >= WCOLS * (j - 1) - shft)
				if (inscol != tabstart)
					continue;
				else
					shfter = WCOLS * (j - 1) - tabstart;
			vgotoCL((j - 1) * WCOLS);
			goim();
			up = tp + (j - 1) * WCOLS - shfter;
			i = shfter;
			do {
				if (*up)
					vputchar(*up++);
				else
					break;
			} while (--i);
		}
		vigotoCL(tabstart);
		i = shft - (inssiz - doomed);
		if ((inscol > WCOLS - shft) && (tabstart < WCOLS))
			i = shft - (inssiz - doomed) + (WCOLS - tabstart);
		if (i > 0) {
			tabslack = inssiz - doomed;
			vcsync();
			goim();
			do
				vputchar((wchar_t)' ');
			while (--i);
		}
	}
	/*
	 * Now do the data moving in the internal screen
	 * image which is common to all three cases.
	 */
	tp += linend;
	up = tp + shft;
	i = linend - tabend;
	if (i > 0)
		do
			*--up = *--tp;
		while (--i);
	if (insert_null_glitch && tshft) {
		i = tshft;
		do
			*--up = QUOTE_SP;
		while (--i);
	}
	hold = oldhold;
}

/*
 * Now do the insert of the characters (finally).
 */
static
void viin(int c)
{
	register wchar_t *tp, *up;
	register int i, j;
	register short noim = 0;
	int remdoom;
	short oldhold = hold;
	int	len;

	hold |= HOLDPUPD;
	if (tabsize && (enter_insert_mode && exit_insert_mode) && inssiz - doomed > tabslack)
		/*
		 * There is a tab out there which will be affected
		 * by the insertion since there aren't enough doomed
		 * characters to take up all the insertion and we do
		 * have insert mode capability.
		 */
		if (inscol + doomed == tabstart) {
			/*
			 * The end of the doomed characters sits right at the
			 * start of the tabs, then we don't need to use insert
			 * mode; unless the tab has already been expanded
			 * in which case we MUST use insert mode.
			 */
			slakused = 0;
			noim = !shft;
		} else {
			/*
			 * The last really special case to handle is case
			 * where the tab is just sitting there and doesn't
			 * have enough slack to let the insertion take
			 * place without shifting the rest of the line
			 * over.  In this case we have to go out and
			 * delete some characters of the tab before we start
			 * or the answer will be wrong, as the rest of the
			 * line will have been shifted.  This code means
			 * that terminals with only insert chracter (no
			 * delete character) won't work correctly.
			 */
			i = inssiz - doomed - tabslack - slakused;
			i %= value(TABSTOP);
			if (i > 0) {
				vgotoCL(tabstart);
				godm();
				for (i = inssiz - doomed - tabslack; i > 0; i--)
					vputp((char *)delete_character, DEPTH(vcline));
				enddm();
			}
		}

	/* 
	 * Now put out the characters of the actual insertion.
	 */
	vigotoCL(inscol);
	remdoom = doomed;
	for (i = inssiz; i > 0;) {
		if (remdoom > 0) {
			remdoom--;
			endim();
		} else if (noim)
			endim();
		else if (enter_insert_mode && exit_insert_mode) {
			vcsync();
			goim();
		}
		vputchar((wchar_t)c);
		i -= (((len = wcwidth(c)) == -1) ? 1 : len);
	}

	if (!enter_insert_mode || !exit_insert_mode) {
		/*
		 * We are a dumb terminal; brute force update
		 * the rest of the line; this is very much an n^^2 process,
		 * and totally unreasonable at low speed.
		 *
		 * You asked for it, you get it.
		 */
		tp = vtube0 + inscol + doomed;
		for (i = inscol + doomed; i < tabstart; i++)
			vputchar(*tp++);
		hold = oldhold;
		vigotoCL(tabstart + inssiz - doomed);
		for (i = tabsize - (inssiz - doomed) + shft; i > 0; i--) {
			vputchar((wchar_t)QUOTE_SP);
		}
	} else {
		if (!insert_null_glitch) {
			/*
			 * On terminals without multi-line
			 * insert in the hardware, we must go fix the segments
			 * between the inserted text and the following
			 * tabs, if they are on different lines.
			 *
			 * Aaargh.
			 */
			tp = vtube0;
			for (j = (inscol + inssiz - 1) / WCOLS + 1;
			    j <= (tabstart + inssiz - doomed - 1) / WCOLS; j++) {
				vgotoCL(j * WCOLS);
				i = inssiz - doomed;
				up = tp + j * WCOLS - i;
				goim();
				do
					vputchar(*up++);
				while (--i && *up);
			}
		} else {
			/*
			 * On terminals with multi line inserts,
			 * life is simpler, just reflect eating of
			 * the slack.
			 */
			tp = vtube0 + tabend;
			for (i = tabsize - (inssiz - doomed); i >= 0; i--) {
				/* portable check */
				if (*--tp == QUOTE_NUL) {
					--tabslack;
					if (tabslack >= slakused)
						continue;
				}
				*tp = QUOTE_SP;
			}
		}
		/*
		 * Blank out the shifted positions to be tab positions.
		 */
		if (shft) {
			tp = vtube0 + tabend + shft;
			for (i = tabsize - (inssiz - doomed) + shft; i > 0; i--) {
				/* if ((*--tp & QUOTE) == 0) *tp = QUOTE_SP; */
				if (*--tp != QUOTE_NUL || *tp != QUOTE_SP)
					*tp = QUOTE_SP;
			}
		}
	}

	/*
	 * Finally, complete the screen image update
	 * to reflect the insertion.
	 */
	hold = oldhold;
	tp = vtube0 + tabstart; up = tp + inssiz - doomed;
	for (i = tabstart; i > inscol + doomed; i--)
		*--up = *--tp;
	if (inssiz == 2 && is_dblwid(c)) {
		*--up = QUOTE_WCD;
		*up = c;
	} else
		for (i = inssiz; i > 0; i--)
			*--up = c;
	doomed = 0;
}

/*
 * Go into ``delete mode''.  If the
 * sequence which goes into delete mode
 * is the same as that which goes into insert
 * mode, then we are in delete mode already.
 */
static
void godm(void)
{

	if (insmode) {
		if (eq(enter_delete_mode, enter_insert_mode))
			return;
		endim();
	}
	vputp((char *)enter_delete_mode, 0);
}

/*
 * If we are coming out of delete mode, but
 * delete and insert mode end with the same sequence,
 * it wins to pretend we are now in insert mode,
 * since we will likely want to be there again soon
 * if we just moved over to delete space from part of
 * a tab (above).
 */
static
void enddm(void)
{

	if (eq(enter_delete_mode, enter_insert_mode)) {
		insmode = 1;
		return;
	}
	vputp((char *)exit_delete_mode, 0);
}

/*
 * In and out of insert mode.
 * Note that the code here demands that there be
 * a string for insert mode (the null string) even
 * if the terminal does all insertions a single character
 * at a time, since it branches based on whether enter_insert_mode is null.
 */
void goim(void)
{

	if (!insmode)
		vputp((char *)enter_insert_mode, 0);
	insmode = 1;
}

void endim(void)
{

	if (insmode) {
		vputp((char *)exit_insert_mode, 0);
		insmode = 0;
	}
}

/*
 * Put the character c on the screen at the current cursor position.
 * This routine handles wraparound and scrolling and understands not
 * to roll when splitw is set, i.e. we are working in the echo area.
 * There is a bunch of code here dealing with the difference between
 * QUOTE_NUL, QUOTE_SP, and ' ' for CONCEPT-100 like terminals, and also
 * code to deal with terminals which overstrike, including CRT's where
 * you can erase overstrikes with some work.  CRT's which do underlining
 * implicitly which has to be erased (like CONCEPTS) are also handled.
 */
/*
 * Treat multiple byte characters somewhat like tabs.
 * Place QUOTE_WCD in copy of screen.
 */
void vputchar(wchar_t c)
{
	register wchar_t *tp;
	register int d;
	int	len;

#ifdef TRACE
	if (trace)
		tracec(c);
#endif
	/* Fix problem of >79 chars on echo line. */
	if ((len = wcwidth(c)) == -1)
		len = 1;
	if (destcol >= WCOLS-len && splitw && destline == WECHO)
		pofix();
	if (destcol >= WCOLS) {
		destline += destcol / WCOLS;
		destcol %= WCOLS;
	}
	if (destcol == WCOLS-1) {
                if (is_dblwid(c)){
                        if(( c != QUOTE_BULLET) && ( c != QUOTE_WCD) &&
                                (c != QUOTE_SP) && ( c != QUOTE_NUL)){
                                vputchar((wchar_t)QUOTE_BULLET);
                        }
                }
        } else if (c == QUOTE_BULLET)
		return ;

	/*  Check again vputchar() above might have put a character and so 
	    upset the column position  D58546 */
	if (destcol >= WCOLS) {
		destline += destcol / WCOLS;
		destcol %= WCOLS;
	}

	if (destline > WBOT && (!splitw || destline > WECHO))
		vrollup(destline);
	tp = vtube[destline] + destcol;
	switch (c) {

	/* previous character occupied two columns */
	case QUOTE_WCD:
		return;
	/* QUOTE_BULLET is converted to a character by vputc, really putch */
	case '\t':
		vgotab();
		return;

	case ' ':
		/*
		 * We can get away without printing a space in a number
		 * of cases, but not always.  We get away with doing nothing
		 * if we are not in insert mode, and not on a CONCEPT-100
		 * like terminal, and either not in hardcopy open or in hardcopy
		 * open on a terminal with no overstriking, provided,
		 * in all cases, that nothing has ever been displayed
		 * at this position.  Ugh.
		 */
		if (!insmode && !insert_null_glitch && (state != HARDOPEN || over_strike) && (*tp == QUOTE_NUL || *tp == 0)) {
			*tp = ' ';
			destcol++;
			return;
		}
		goto def;

	case QUOTE_NUL:
		if (insmode) {
			/*
			 * When in insert mode, tabs have to expand
			 * to real, printed blanks.
			 */
			c = QUOTE_SP;
			goto def;
		}
		if (*tp == 0) {
			/*
			 * A ``space''.
			 */
			if ((hold & HOLDPUPD) == 0)
				*tp = QUOTE_NUL;
			destcol++;
			return;
		}
		/*
		 * A ``space'' ontop of a part of a tab.
		 */
		if (*tp == QUOTE_NUL || *tp == QUOTE_SP) {
			destcol++;
			return;
		}
		c = QUOTE_SP;
		/* fall into ... */

def:
	default:
		d = (*tp == QUOTE_NUL ? '\0' : *tp == QUOTE_SP ? ' ' : *tp);
		/*
		 * Now get away with doing nothing if the characters
		 * are the same, provided we are not in insert mode
		 * and if we are in hardopen, that the terminal has overstrike.
		 */
		if ((d == c || *tp == c) && !insmode &&
		    (state != HARDOPEN || over_strike)) {
			if ((hold & HOLDPUPD) == 0) {
				*tp = c;
                                destcol++;
                                if (is_dblwid(c) && ( c != QUOTE_BULLET) &&
                                   ( c != QUOTE_WCD) && ( c != QUOTE_SP) &&
				   ( c != QUOTE_NUL)){
					*++tp = QUOTE_WCD;
					destcol++;
				}
			}
                        else {
                                destcol++;
                                if(is_dblwid(c) && ( c != QUOTE_BULLET) &&
                                  ( c != QUOTE_WCD) && ( c != QUOTE_SP) &&
				  ( c != QUOTE_NUL))
                                        destcol++;
                        }
			return;
		}
		/*
		 * Backwards looking optimization.
		 * The low level cursor motion routines will use
		 * a cursor motion right sequence to step 1 character
		 * right.  On, e.g., a DM3025A this is 2 characters
		 * and printing is noticeably slower at 300 baud.
		 * Since the low level routines are not allowed to use
		 * spaces for positioning, we discover the common
		 * case of a single space here and force a space
		 * to be printed.
		 */
		if (destcol == outcol + 1 && tp[-1] == ' ' && outline == destline) {
			vputc(' ');
			outcol++;
		}

		/*
		 * This is an inline expansion a call to vcsync() dictated
		 * by high frequency in a profile.
		 */
		if (outcol != destcol || outline != destline)
			vgoto(destline, destcol);

		/*
		 * Deal with terminals which have overstrike.
		 * We handle erasing general overstrikes, erasing
		 * underlines on terminals (such as CONCEPTS) which
		 * do underlining correctly automatically (e.g. on nroff
		 * output), and remembering, in hardcopy mode,
		 * that we have overstruct something.
		 */
		if (!insmode && d && d != ' ' && d != c) {
			if (erase_overstrike && (over_strike || transparent_underline && (c == '_' || d == '_'))) {
				vputc(' ');
				outcol++, destcol++;
				back1();
			} else
				rubble = 1;
		}

		/*
		 * Unless we are just bashing characters around for
		 * inner working of insert mode, update the display.
		 */
		if ((hold & HOLDPUPD) == 0) {
			*tp = c;
                        if (is_dblwid(c) && (c != QUOTE_BULLET) &&
			   ( c != QUOTE_WCD) && ( c != QUOTE_SP) &&
			   ( c != QUOTE_NUL))
				*++tp = QUOTE_WCD;
		}

		/*
		 * In insert mode, put out the insert_character sequence, padded
		 * based on the depth of the current line.
		 * A terminal which had no real insert mode, rather
		 * opening a character position at a time could do this.
		 * Actually should use depth to end of current line
		 * but this rarely matters.
		 */
		if (insmode) {
			register int i = (((len = wcwidth(c)) == -1) ? 1 : len);
                        if(( c == QUOTE_SP) || ( c == QUOTE_NUL) ||
			   ( c == QUOTE_WCD) || (c == QUOTE_BULLET)) i = 1;
			do vputp(insert_character, DEPTH(vcline)); while (--i);
		}

		vputc(c);

		/*
		 * In insert mode, insert_padding is a post insert pad.
		 */
		if (insmode)
			vputp((char *)insert_padding, DEPTH(vcline));
                if( ( c == QUOTE_SP) || ( c == QUOTE_NUL) ||
		    ( c == QUOTE_WCD) || ( c == QUOTE_BULLET)){
                        destcol++;
                        outcol++;
                }
                else {
                        destcol += (((len = wcwidth(c)) == -1) ? 1 : len);
                        outcol += (((len = wcwidth(c)) == -1) ? 1 : len);
                }

		/*
		 * CONCEPT braindamage in early models:  after a wraparound
		 * the next newline is eaten.  It's hungry so we just
		 * feed it now rather than worrying about it.
		 * Fixed to use	return linefeed to work right
		 * on vt100/tab132 as well as concept.
		 */
		if (eat_newline_glitch && outcol % WCOLS == 0) {
			vputc('\r');
			vputc('\n');
		}
	}
}

/*
 * Delete display positions stcol through endcol.
 * Amount of use of special terminal features here is limited.
 */
void physdc(int stcol, int endcol)
{
	register wchar_t *tp, *up;
	wchar_t *tpe;
	register int i;
	register int nc = endcol - stcol;

#ifdef IDEBUG
	if (trace)
		tfixnl(), fprintf(trace, "physdc(%d, %d)\n", stcol, endcol);
#endif
	if (!delete_character || nc <= 0)
		return;
	if (insert_null_glitch) {
		/*
		 * CONCEPT-100 like terminal.
		 * If there are any ``spaces'' in the material to be
		 * deleted, then this is too hard, just retype.
		 */
		vprepins();
		up = vtube0 + stcol;
		i = nc;
		do
			if (*up++ == QUOTE_NUL)
				return;
		while (--i);
		i = 2 * nc;
		do {
			/* if (*up == 0 || (*up++ & QUOTE)) return; */
			if (*up == '\0' || *up == QUOTE_NUL || *up++ == QUOTE_SP)
				return;
		} while (--i);
		vgotoCL(stcol);
	} else {
		/*
		 * HP like delete mode.
		 * Compute how much text we are moving over by deleting.
		 * If it appears to be faster to just retype
		 * the line, do nothing and that will be done later.
		 * We are assuming 2 output characters per deleted
		 * characters and that clear to end of line is available.
		 */
		i = stcol / WCOLS;
		if (i != endcol / WCOLS)
			return;
		i += LINE(vcline);
		stcol %= WCOLS;
		endcol %= WCOLS;
		up = vtube[i]; tp = up + endcol; tpe = up + WCOLS;
		while (tp < tpe && *tp)
			tp++;
		if (tp - (up + stcol) < 2 * nc)
			return;
		vgoto(i, stcol);
	}

	/*
	 * Go into delete mode and do the actual delete.
	 * Padding is on delete_character itself.
	 */
	godm();
	for (i = nc; i > 0; i--)
		vputp((char *)delete_character, DEPTH(vcline));
	vputp((char *)exit_delete_mode, 0);

	/*
	 * Straighten up.
	 * With CONCEPT like terminals, characters are pulled left
	 * from first following null.  HP like terminals shift rest of
	 * this (single physical) line rigidly.
	 */
	if (insert_null_glitch) {
		up = vtube0 + stcol;
		tp = vtube0 + endcol;
		while (i = *tp++) {
			if (i == QUOTE_NUL)
				break;
			*up++ = i;
		}
		do
			*up++ = i;
		while (--nc);
	} else {
		copy(up + stcol, up + endcol, sizeof(wchar_t) *(WCOLS - endcol));
		vclrcp(tpe - nc, nc);
	}
}

#ifdef TRACE
void tfixnl(void)
{

	if (trubble || techoin)
		fprintf(trace, "\n");
	trubble = 0, techoin = 0;
}

void tvliny(void)
{
	register int i;

	if (!trace)
		return;
	tfixnl();
	fprintf(trace, "vcnt = %d, vcline = %d, vliny = ", vcnt, vcline);
	for (i = 0; i <= vcnt; i++) {
		fprintf(trace, "%d", LINE(i));
		if (FLAGS(i) & VDIRT)
			fprintf(trace, "*");
		if (DEPTH(i) != 1)
			fprintf(trace, "<%d>", DEPTH(i));
		if (i < vcnt)
			fprintf(trace, " ");
	}
	fprintf(trace, "\n");
}

void tracec(wchar_t c)
{

	if (!techoin)
		trubble = 1;
	if (c == ESCAPE)
		fprintf(trace, "$");
	if (iswcntrl(c))
		fprintf(trace, "^%C", ctlof(c));
	else
		fprintf(trace, "%C", c);
}
#endif

/*
 * Put a character with possible tracing.
 */
void vputch(int c)
{

#ifdef TRACE
	if (trace)
		tracec(c);
#endif
	vputc(c);
}
