static char sccsid [] = "@(#)88  1.18  src/bos/usr/bin/ex/ex_put.c, cmdedit, bos41B, 9504A 12/19/94 11:46:38";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_put.c
 *
 * FUNCTION: fgoto, draino, flush, flush1, flush2, flusho, gTTY, gotab,
 * listchar, lprintf, noonl, normal, normchar, normline, noteinp, numbline,
 * ostart, ostop, plod, plodput, pstart, pstop, putNFL, putch, ex_putchar,
 * putnl, putpad, sTTY, setlist, setnumb, setoutt, setty, slobber, termchar,
 * termreset, tostart, tostop, ttcharoff, vcook, vraw
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

/*
 * Terminal driving and line formatting routines.
 * Basic motion optimizations are done here as well
 * as formatting of lines (printing of control characters,
 * line numbering and the like).
 */

/*
 * The routines outchar, putchar and pline are actually
 * variables, and these variables point at the current definitions
 * of the routines.  See the routine setflav.
 * We sometimes make outchar be routines which catch the characters
 * to be printed, e.g. if we want to see how long a line is.
 * During open/visual, outchar and putchar will be set to
 * routines in the file ex_vput.c (vputchar, vinschar, etc.).
 */
static	void normchar(int);
void	(*Outchar)(int) = termchar;
void	(*Putchar)(int) = normchar;
void	(*Pline)(int) = (void(*)(int))normline;
# ifdef 	TIOCSETC	/* V7 */
struct tchars nttyc, ottyc;
# endif
# ifdef		TIOCLGET	/* Berkeley 4BSD */
struct ltchars nlttyc, olttyc;
# endif
static struct termios posnttyc, posottyc;

static void slobber(int);
static void flush2(void);
static int plod(int);
static void ttcharoff(void);
static void sTTY(int);

void (*
setlist(int t))(int)
{
	register void (*P)(int);

	listf = t;
	P = Putchar;
	Putchar = t ? (void(*)(int))listchar : normchar;
	return (P);
}

void (*
setnumb(int t))(int)
{
	register void (*P)(int);

	P = Pline;
	Pline = t ? numbline : (void(*)(int))normline;
	return (P);
}

/*
 * Format c for list mode; leave things in common
 * with normal print mode to be done by normchar.
 */
void listchar(register wchar_t c)
{
	if (c == QUOTE_NL) {
		outchar('$');
	} else if (c != '\n' && iswcntrl(c)) {
		outchar('^');
		c = ctlof(c);
	}
	normchar(c);
}

/*
 * Format c for printing.  Handle funnies of upper case terminals
 * and crocky hazeltines which don't have ~.
 */
static void normchar(register int c)
{
	register  char *colp;

	if (c == '~' && tilde_glitch) {
		normchar('\\');
		c = '^';
	}
	if (iswcntrl(c) && (c != '\b' || !over_strike) && c != '\n' && c != '\t') {
		ex_putchar('^');
		c = ctlof(c);
	} else {
		switch (c) {
		case QUOTE_NUL:
		case QUOTE_WCD:
			return;
		case QUOTE_SP:
		case QUOTE_BSP:
			break;
		case QUOTE_NL:
			c = '\n';
			break;
		case QUOTE_CR:
			c = '\r';
			break;
		default:
			if (UPPERCASE) {
				if (iswupper(c)) {
					outchar('\\');
					c = towlower(c);
				} else {
					colp = "({)}!|^~'`";
					while (*colp++) {
						if (c == *colp++) {
							outchar('\\');
							c = colp[-2];
							break;
						}
					}
				}
			}
		}
	}
	outchar(c);
}

/*
 * Print a line with a number.
 */
void numbline(int i)
{

	if (shudclob)
		slobber(' ');
	ex_printf("%+6d\t", i);
	normline();
}

/*
 * Normal line output, no numbering.
 */
void normline(void)
{
	register wchar_t *cp;

	if (shudclob)
		slobber(linebuf[0]);
	/* pdp-11 doprnt is not reentrant so can't use "printf" here
	   in case we are tracing */
	for (cp = linebuf; *cp;)
		ex_putchar(*cp++);
	if (!inopen)
		ex_putchar(QUOTE_NL);
}

/*
 * Given c at the beginning of a line, determine whether
 * the printing of the line will erase or otherwise obliterate
 * the prompt which was printed before.  If it won't, do it now.
 */
static void slobber(int c)
{

	shudclob = 0;
	switch (c) {

	case '\t':
		if (Putchar == (void(*)(int))listchar)
			return;
		break;

	default:
		return;

	case ' ':
	case 0:
		break;
	}
	if (over_strike)
		return;
	flush();
	putch(' ');
	tputs(cursor_left, 0, (int (*)(int))putch);
}

static	wchar_t	linb[66];
static	wchar_t	*linp = linb;

/*
 * Phadnl records when we have already had a complete line ending with \n.
 * If another line starts without a flush, and the terminal suggests it,
 * we switch into -nl mode so that we can send lineffeeds to avoid
 * a lot of spacing.
 */
static	short phadnl;

/*
 * Indirect to current definition of putchar.
 */
void ex_putchar(int c)
{

	(*Putchar)(c);
}

/*
 * Termchar routine for command mode.
 * Watch for possible switching to -nl mode.
 * Otherwise flush into next level of buffering when
 * small buffer fills or at a newline.
 */
void termchar(int c)
{

	if (pfast == 0 && phadnl)
		pstart();
	if (c == '\n')
		phadnl = 1;
	else if (linp >= &linb[63])
		flush1();
	*linp++ = c;
	if (linp >= &linb[62]) {
		fgoto();
		flush1();
	}
}

void flush(void)
{

	flush1();
	flush2();
}

/*
 * Flush from small line buffer into output buffer.
 * Work here is destroying motion into positions, and then
 * letting fgoto do the optimized motion.
 */
void flush1(void)
{
	register wchar_t *lp;
	register wchar_t c;
	register int len;

	*linp = 0;
	lp = linb;
	while (*lp) {
		switch (c = *lp++) {

		case '\r':
			destline += destcol / columns;
			destcol = 0;
			continue;

		case '\b':
/*! ignore jxterm that moves across an entire double-wide character !*/
			if (destcol)
				destcol--;
			continue;

		case ' ':
			destcol++;
			continue;

		case '\t':
			destcol += numb_tab_offset(destcol, value(TABSTOP));
			continue;

		case '\n':
			destline += destcol / columns + 1;
			if (destcol != 0 && destcol % columns == 0)
				destline--;
			destcol = 0;
			continue;

		default:
			fgoto();
			for (;;) {
				if (auto_right_margin == 0 && outcol == columns)
					fgoto();
				/* c &= TRIM; */
				switch (c) {
				case QUOTE_NUL:
					c = '\0';
					break;
				case QUOTE_BSP:
					c = '\b';
					break;
				case QUOTE_SP:
					c = ' ';
					break;
				}
				putch(c);
				if (c == '\b') {
/*
 * Note: This is only used in one place -- to erase the '^' or '0' before a ^D
 * -- so we don't need to worry about double-wide characters here.
 */
					putch(' ');
					putch('\b');
					outcol--;
					destcol--;
				} else if (iswprint(c)) {
					if ((len = wcwidth(c)) < 0)
						len = 1;
					outcol += len;
					destcol += len;
					if (eat_newline_glitch && outcol % columns == 0) {
						putch('\r');
						putch('\n');
					}
				}
				c = *lp++;
				if (c <= ' ')
					break;
			}
			--lp;
			continue;
		}
	}
	linp = linb;
}

static void flush2(void)
{

	fgoto();
	flusho();
	pstop();
}

/*
 * Sync the position of the output cursor.
 * Most work here is rounding for terminal boundaries getting the
 * column position implied by wraparound or the lack thereof and
 * rolling up the screen to get destline on the screen.
 */
void fgoto(void)
{
	register int l, c;

	if (destcol > columns - 1) {
		destline += destcol / columns;
		destcol %= columns;
	}
	if (outcol > columns - 1) {
		l = (outcol + 1) / columns;
		outline += l;
		outcol %= columns;
		if (auto_right_margin == 0) {
			while (l > 0) {
				if (pfast)
					tputs(carriage_return, 0, (int (*)(int))putch);
				tputs(cursor_down, 0, (int (*)(int))putch);
				l--;
			}
			outcol = 0;
		}
		if (outline > lines - 1) {
			destline -= outline - (lines - 1);
			outline = lines - 1;
		}
	}
	if (destline > lines - 1) {
		l = destline;
		destline = lines - 1;
		if (outline < lines - 1) {
			c = destcol;
			if (pfast == 0 && (!cursor_address || holdcm))
				destcol = 0;
			fgoto();
			destcol = c;
		}
		while (l > lines - 1) {
			/*
			 * The following linefeed (or simulation thereof)
			 * is supposed to scroll up the screen, since we
			 * are on the bottom line.
			 *
			 * Superbee glitch:  in the middle of the screen we
			 * have to use esc B (down) because linefeed screws up
			 * in "Efficient Paging" (what a joke) mode (which is
			 * essential in some SB's because CRLF mode puts garbage
			 * in at end of memory), but you must use linefeed to
			 * scroll since down arrow won't go past memory end.
			 * I turned this off after recieving a
			 * Superbee description which wins better.
			 */
			if (scroll_forward /* && !beehive_glitch */ && pfast)
				tputs(scroll_forward, 0, (int (*)(int))putch);
			else
				putch('\n');
			l--;
			if (pfast == 0)
				outcol = 0;
		}
	}
	if (destline < outline && !(cursor_address && !holdcm || cursor_up || cursor_home))
		destline = outline;
	if (cursor_address && !holdcm)
		if (plod(costCM) > 0)
			plod(0);
		else
			tputs(tparm(cursor_address, destline, destcol), 0, (int (*)(int))putch);
	else
		plod(0);
	outline = destline;
	outcol = destcol;
}

/*
 * Tab to column col by flushing and then setting destcol.
 * Used by "set all".
 */
void gotab(int col)
{

	flush1();
	destcol = col;
}

/*
 * Move (slowly) to destination.
 * Hard thing here is using home cursor on really deficient terminals.
 * Otherwise just use cursor motions, modifying use of tabs and overtabbing
 * and backspace.
 */

static int plodcnt, plodflg;

static void plodput(int c)
{

	if (plodflg)
		plodcnt--;
	else
		putch(c);
}

static int plod(int cnt)
{
	register int i, j, k;
	register int soutcol, soutline;
	int	len;

	plodcnt = plodflg = cnt;
	soutcol = outcol;
	soutline = outline;
	/*
	 * Consider homing and moving down/right from there, vs moving
	 * directly with local motions to the right spot.
	 */
	if (cursor_home) {
		/*
		 * i is the cost to home and tab/space to the right to
		 * get to the proper column.  This assumes cursor_right costs
		 * 1 char.  So i+destcol is cost of motion with home.
		 */
		if (tab)
			i = (destcol / value(HARDTABS)) + (destcol % value(HARDTABS));
		else
			i = destcol;
		/*
		 * j is cost to move locally without homing
		 */
		if (destcol >= outcol) {	/* if motion is to the right */
			j = destcol / value(HARDTABS) - outcol / value(HARDTABS);
			if (tab && j)
				j += destcol % value(HARDTABS);
			else
				j = destcol - outcol;
		} else
			/* leftward motion only works if we can backspace. */
			if (outcol - destcol <= i && (cursor_left))
				i = j = outcol - destcol; /* cheaper to backspace */
			else
				j = i + 1; /* impossibly expensive */

		/* k is the absolute value of vertical distance */
		k = outline - destline;
		if (k < 0)
			k = -k;
		j += k;

		/*
		 * Decision.  We may not have a choice if no cursor_up.
		 */
		if (i + destline < j || (!cursor_up && destline < outline)) {
			/*
			 * Cheaper to home.  Do it now and pretend it's a
			 * regular local motion.
			 */
			tputs(cursor_home, 0, (int (*)(int))plodput);
			outcol = outline = 0;
		} else if (cursor_to_ll) {
			/*
			 * Quickly consider homing down and moving from there.
			 * Assume cost of cursor_to_ll is 2.
			 */
			k = (lines - 1) - destline;
			if (i + k + 2 < j && (k<=0 || cursor_up)) {
				tputs(cursor_to_ll, 0, (int (*)(int))plodput);
				outcol = 0;
				outline = lines - 1;
			}
		}
	} else
		/*
		 * No home and no up means it's impossible, so we return an
		 * incredibly big number to make cursor motion win out.
		 */
		if (!cursor_up && destline < outline)
			return (500);
	if (tab)
		i = destcol % value(HARDTABS)
		    + destcol / value(HARDTABS);
	else
		i = destcol;
		j = outcol - destcol;
	/*
	 * If we will later need a \n which will turn into a \r\n by
	 * the system or the terminal, then don't bother to try to \r.
	 */
	if ((NONL || !pfast) && outline < destline)
		goto dontcr;
	/*
	 * If the terminal will do a \r\n and there isn't room for it,
	 * then we can't afford a \r.
	 */
	if (!carriage_return && outline >= destline)
		goto dontcr;
	/*
	 * If we've scrolled past the echo area, then we don't want to
	 * go too far left then recover, as this may overwrite significant
	 * characters with junk.  E.g., type a ':' in vi mode, and type more
	 * than a line's worth, and then a ^X.  We don't want to trounce
	 * the ':'.
	 */
	if (splitw && (Outchar == termchar) && (destcol != 0))
		goto dontcr;
	/*
	 * If it will be cheaper, or if we can't back up, then send
	 * a return preliminarily.
	 */
	if (j > i + 1 || outcol > destcol && !cursor_left) {
		/*
		 * BUG: this doesn't take the (possibly long) length
		 * of carriage_return into account.
		 */
		if (carriage_return) {
			tputs(carriage_return, 0, (int (*)(int))plodput);
			outcol = 0;
		} else if (newline) {
			tputs(newline, 0, (int (*)(int))plodput);
			outline++;
			outcol = 0;
		}
	}
dontcr:
	/* Move down, if necessary, until we are at the desired line */
	while (outline < destline) {
		j = destline - outline;
		if (j > costDP && parm_down_cursor) {
			/* Win big on Tek 4025 */
			tputs(tparm(parm_down_cursor, j), j, (int (*)(int))plodput);
			outline += j;
		}
		else {
			outline++;
			if (cursor_down && pfast)
				tputs(cursor_down, 0, (int (*)(int))plodput);
			else
				plodput('\n');
		}
		if (plodcnt < 0)
			goto out;
		if (NONL || pfast == 0)
			outcol = 0;
	}
	if (back_tab){
		k = strlen(back_tab);	/* should probably be cost(back_tab) and moved out */
		/*
		 * Make sure we don't go too far left if we've scrolled
		 * out of the echo area.
		 */
		if (splitw && (Outchar == termchar) && (value(HARDTABS) > 4))
			k += value(HARDTABS) - 4;
	}
	/* Move left, if necessary, to desired column */
	while (outcol > destcol) {
		if (plodcnt < 0)
			goto out;
		if (back_tab && !insmode && outcol - destcol > 4+k) {
			tputs(back_tab, 0, (int (*)(int))plodput);
			outcol--;
			outcol -= outcol % value(HARDTABS); /* outcol &= ~7; */
			continue;
		}
		j = outcol - destcol;
		if (j > costLP && parm_left_cursor) {
			tputs(tparm(parm_left_cursor, j), j, (int (*)(int))plodput);
			outcol -= j;
		}
		else {
			outcol--;
			tputs(cursor_left, 0, (int (*)(int))plodput);
		}
	}
	/* Move up, if necessary, to desired row */
	while (outline > destline) {
		j = outline - destline;
		if (parm_up_cursor && j > 1) {
			/* Win big on Tek 4025 */
			tputs(tparm(parm_up_cursor, j), j, (int (*)(int))plodput);
			outline -= j;
		}
		else {
			outline--;
			tputs(cursor_up, 0, (int (*)(int))plodput);
		}
		if (plodcnt < 0)
			goto out;
	}
	/*
	 * Now move to the right, if necessary.  We first tab to
	 * as close as we can get.
	 */
	if (tab && !insmode && destcol - outcol > 1) {
		/* tab to right as far as possible without passing col */
		for (;;) {
			i = tabcol(outcol, value(HARDTABS));
			if (i > destcol)
				break;
			if (tab)
				tputs(tab, 0, (int (*)(int))plodput);
			else
				plodput('\t');
			outcol = i;
		}
		/* consider another tab and then some backspaces */
		if (destcol - outcol > 4 && i < columns && cursor_left) {
			tputs(tab, 0, (int (*)(int))plodput);
			outcol = i;
			/*
			 * Back up.  Don't worry about parm_left_cursor because
			 * it's never more than 4 spaces anyway.
			 */
			while (outcol > destcol) {
				outcol--;
				tputs(cursor_left, 0, (int (*)(int))plodput);
			}
		}
	}
	/*
	 * We've tabbed as much as possible.  If we still need to go
	 * further (not exact or can't tab) space over.  This is a
	 * very common case when moving to the right with space.
	 */
	while (outcol < destcol) {
		j = destcol - outcol;
		if (j > costRP && parm_right_cursor) {
			/*
			 * This probably happens rarely, if at all.
			 * It is mainly useful for ANSI terminals
			 * with no hardware tabs, and I don't know
			 * of any such terminal at the moment.
			 */
			tputs(tparm(parm_right_cursor, j), j, (int (*)(int))plodput);
			outcol += j;
		}
		else {
			/*
			 * move one column to the right.  We don't use right
			 * because it's better to just print the character we
			 * are moving over.  There are various exceptions,
			 * however.
			 * If !inopen, vtube contains garbage.  If the character
			 * is a null or a tab we want to print a space.  Other
			 * random chars we use space for instead, too.
			 */
			/*
			 * In addition, if we've scrolled past the echo area,
			 * we have avoided backing up too far, so a forward
			 * space means we need a blank.
			 */
			if (!inopen || (vtube[outline]==NULL) ||
			    (splitw && (Outchar == termchar)) ||
				(i=vtube[outline][outcol]) < ' ')
				i = ' ';
			if (i == QUOTE_NUL || i == QUOTE_SP)
				i = ' ';
			if ((len = wcwidth(i)) < 0)
				len = 1;
			if (cursor_right && (insmode || i == QUOTE_WCD || j < len)) {
				tputs(cursor_right, 0, (int (*)(int))plodput);
				outcol++;
			} else {
				plodput(i);
				outcol += len;
			}
		}
		if (plodcnt < 0)
			goto out;
	}
out:
	if (plodflg) {
		outcol = soutcol;
		outline = soutline;
	}
	return(plodcnt);
}

/*
 * An input line arrived.
 * Calculate new (approximate) screen line position.
 * Approximate because kill character echoes newline with
 * no feedback and also because of long input lines.
 */
void noteinp(void)
{

	outline++;
	if (outline > lines - 1)
		outline = lines - 1;
	destline = outline;
	destcol = outcol = 0;
}

/*
 * Something weird just happened and we
 * lost track of whats happening out there.
 * Since we cant, in general, read where we are
 * we just reset to some known state.
 * On cursor addressible terminals setting to unknown
 * will force a cursor address soon.
 */
void termreset(void)
{

	endim();
	if (enter_ca_mode)
		putpad(enter_ca_mode);	
	destcol = 0;
	destline = lines - 1;
	if (cursor_address) {
		outcol = UKCOL;
		outline = UKCOL;
	} else {
		outcol = destcol;
		outline = destline;
	}
}

/*
 * Low level buffering, with the ability to drain
 * buffered output without printing it.
 */
static char	*obp = obuf;

void draino(void)
{

	obp = obuf;
}

void flusho(void)
{
	register int togo, wrote;
	register char *p = obuf;
	static int eflag;

	while (togo = obp - p) {
		if ((wrote = write(outfd, p, togo)) < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			if (eflag)
				break;
			eflag = 1;
			onhup();
		}
		p += wrote;
	}
	obp = obuf;
}

void putnl(void)
{

	ex_putchar('\n');
}

void putch(int c)
{

#ifdef OLD3BTTY		/* mjm */
	if(c == '\n')	/* mjm: Fake "\n\r" for '\n' til fix in 3B firmware */
		putch('\r');	/* mjm: vi does "stty -icanon" => -onlcr !! */
#endif
	if (c == QUOTE_WCD) return;	/*! This probably isn't possible !*/
	if (c == QUOTE_BULLET) c = svalue(PARTIALCHARACTER)[0];
	if (c == QUOTE_NUL) c = '\0';	/*! This probably isn't possible !*/
	if (c == QUOTE_SP) c = ' ';
	/* this is the conversion from wide character to multibyte  */
	obp += wctomb(obp, c);
	if (obp >= &obuf[sizeof obuf]-1)
		flusho();
}

/*
 * Miscellaneous routines related to output.
 */

/*
 * Put with padding
 */
void putpad(char *cp)
{

	flush();
	tputs(cp, 0, (int (*)(int))putch);
}

/*
 * Set output through normal command mode routine.
 */
void setoutt(void)
{

	Outchar = termchar;
}

/*
 * Printf (temporarily) in list mode.
 */
void lprintf(char *cp, char *dp)
{
	register void (*P)(int);

	P = setlist(1);
	ex_printf(cp, dp);
	Putchar = P;
}

/*
 * Newline + flush.
 */
void putNFL(void)
{

	putnl();
	flush();
}

/*
 * Try to start -nl mode.
 */
void pstart(void)
{

	if (NONL)
		return;
 	if (!value(OPTIMIZE))
		return;
	if (ruptible == 0 || pfast)
		return;
	fgoto();
	flusho();
	pfast = 1;
	normtty++;
#if !defined(USG) && !defined(_POSIX_SOURCE)
	tty.sg_flags = normf & ~(ECHO|XTABS|CRMOD);
#else
	tty = normf;
	tty.c_oflag &= ~(ONLCR|TAB3);
	tty.c_lflag &= ~ECHO;
#endif
	saveterm();
	sTTY(2);
}

/*
 * Stop -nl mode.
 */
void pstop(void)
{

	if (inopen)
		return;
	phadnl = 0;
	linp = linb;
	draino();
	normal(normf);
	pfast &= ~1;
}

/*
 * Prep tty for open mode.
 */
ttymode
ostart(void)
{
	ttymode f;

	/*
	if (!intty)
		error("Open and visual must be used interactively");
	*/
	gTTY(2);
	normtty++;
#if !defined(USG) && !defined(_POSIX_SOURCE)
	f = tty.sg_flags;
	tty.sg_flags = (normf &~ (ECHO|XTABS|CRMOD)) |
# ifdef CBREAK
							CBREAK;
# else
							RAW;
# endif
# ifdef TIOCGETC
	ttcharoff();
# endif
#else
	f = tty;
	tty = normf;
	tty.c_iflag &= ~ICRNL;
	tty.c_lflag &= ~(ECHO|ICANON);
	tty.c_oflag &= ~(TAB3|ONLCR);
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;
	ttcharoff();
#endif
	sTTY(2);
	tostart();
	pfast |= 2;
	saveterm();
	return (f);
}

/* actions associated with putting the terminal in open mode */
void tostart(void)
{
	putpad(cursor_visible);
	putpad(keypad_xmit);
	if (!value(MESG)) {
		if (ttynbuf[0] == 0) {
			register char *tn;
			if ((tn=ttyname(2)) == NULL &&
			    (tn=ttyname(1)) == NULL &&
			    (tn=ttyname(0)) == NULL)
				ttynbuf[0] = 1;
			else
				strcpy(ttynbuf, tn);
		}
		if (ttynbuf[0] != 1) {
#if defined(_SECURITY)
			ttyaclp = acl_get(ttynbuf);
			acl_set(ttynbuf,W_ACC|R_ACC,NO_ACC,NO_ACC);	
#else
			struct stat sbuf;
			stat(ttynbuf, &sbuf);
			ttymesg = sbuf.st_mode & 0777;
			chmod(ttynbuf,
#ifdef UCBV7
	/*
	 * This applies to the UCB V7 Pdp-11 system with the
	 * -u write option only.
	 */
					0611	/* 11 = urgent only allowed */
#else
					0600
						);
#endif
#endif /* _SECURITY */
		}
	}
}

/*
 * Turn off start/stop chars if they aren't the default ^S/^Q.
 * This is so idiots who make esc their start/stop don't lose.
 * We always turn off quit since datamedias send ^\ for their
 * right arrow key.
 * For code page useability, the '\377' entries in the code
 * below were changed to '\000'then back when codepage 850 came in.
 * 
 */
static

#if defined(_POSIX_SOURCE) 

void ttcharoff(void)
{
	tty.c_cc[VQUIT] = _POSIX_VDISABLE;
	if (tty.c_cc[VSTART] != Ctrl('Q'))
		tty.c_cc[VSTART] = _POSIX_VDISABLE;
	if (tty.c_cc[VSTOP] != Ctrl('S'))
		tty.c_cc[VSTOP] = _POSIX_VDISABLE;
#ifdef VDISCRD
	tty.c_cc[VDISCRD] = _POSIX_VDISABLE;	/* ^O */
#endif
#ifdef VLNEXT
	tty.c_cc[VLNEXT] = _POSIX_VDISABLE;	/* ^V */
#endif
#ifdef _POSIX_JOB_CONTROL
	tty.c_cc[VSUSP] = _POSIX_VDISABLE;	/* ^Z */
#ifdef VDSUSP
	tty.c_cc[VDSUSP] = _POSIX_VDISABLE;	/* ^Y */
#endif
#endif	
}

#elif defined(USG)

void ttcharoff(void)
{
	tty.c_cc[VQUIT] = '\377';
#ifdef VSTART
	/*
	 * The following code is necessary because we support changing all the 
	 * extra characters.
	 */
	if (tty.c_cc[VSTART] != Ctrl('Q'))
		tty.c_cc[VSTART] = '\377';
	if (tty.c_cc[VSTOP] != Ctrl('S'))
		tty.c_cc[VSTOP] = '\377';
	posnttyc.c_cc[VSUSP] = '\377';	/* ^Z */
/* While bringing the OSF code into the AIX environment, the following
   two lines were commented out, because VSWTCH and VFLUSH are not
   defined in termios.h.
   */
/*	tty.c_cc[VSWTCH] = '\377';	*//* ^Z */
/*	posnttyc.c_cc[VFLUSH] = '\377';	*//* ^O */
	posnttyc.c_cc[VLNEXT] = '\377';	/* ^V */
# endif
}

#else /* USG */

void ttcharoff(void)
{
	nttyc.t_quitc = '\377';
	if (nttyc.t_startc != Ctrl('Q'))
		nttyc.t_startc = '\377';
	if (nttyc.t_stopc != Ctrl('S'))
		nttyc.t_stopc = '\377';
#ifdef TIOCLGET
	nlttyc.t_suspc = '\377';	/* ^Z */
	nlttyc.t_dsuspc = '\377';	/* ^Y */
	nlttyc.t_flushc = '\377';	/* ^O */
	nlttyc.t_lnextc = '\377';	/* ^V */
#endif
}

#endif /* USG */

/*
 * Stop open, restoring tty modes.
 */
void ostop(ttymode f)
{

#ifndef USG
	pfast = (f & CRMOD) == 0;
#else
	pfast = (f.c_oflag & ONLCR) == 0;
#endif
	termreset(), fgoto(), flusho();
	normal(f);
	tostop();
}

/* Actions associated with putting the terminal in the right mode. */
void tostop(void)
{
	putpad(clr_eos);
	putpad(cursor_normal);
	putpad(keypad_local);
	if (!value(MESG) && ttynbuf[0]>1)
#if defined(_SECURITY)
		acl_put(ttynbuf,ttyaclp,1);
#else
		chmod(ttynbuf, ttymesg);
#endif
}

#ifndef CBREAK
/*
 * Into cooked mode for interruptibility.
 */
void vcook(void)
{

	tty.sg_flags &= ~RAW;
	sTTY(2);
}

/*
 * Back into raw mode.
 */
void vraw(void)
{

	tty.sg_flags |= RAW;
	sTTY(2);
}
#endif

/*
 * Restore flags to normal state f.
 */
void normal(ttymode f)
{

	if (normtty > 0) {
		setty(f);
		normtty--;
	}
}

/*
 * Straight set of flags to state f.
 */
ttymode
setty(ttymode f)
{
	int isnorm = 0;
#ifndef USG
	register int ot = tty.sg_flags;
#else
	ttymode ot;
	ot = tty;
#endif

#if !defined(USG) && !defined(_POSIX_SOURCE)
	if (f == normf) {
		nttyc = ottyc;
		isnorm = 1;
# ifdef		TIOCLGET	/* Berkeley 4BSD */
		nlttyc = olttyc;
# endif
	} else
		ttcharoff();
	tty.sg_flags = f;
#else				/* USG or POSIX */
	if (tty.c_lflag & ICANON)
		ttcharoff();
	else
	{
		posnttyc = posottyc; 	/* Restore the posix modes too. */
		isnorm = 1;
	}
	
	tty = f;
#endif				/* USG or POSIX */
	sTTY(2);
	if (!isnorm)
		saveterm();
	return (ot);
}

void gTTY(int i)
{
#ifdef _POSIX_SOURCE
	tcgetattr(i, &tty);
#else					/* _POSIX_SOURCE */
#ifndef USG
	ignore(gtty(i, &tty));
# ifdef TIOCGETC
	ioctl(i, TIOCGETC, &ottyc);
	nttyc = ottyc;
# endif				/* TIOCGETC */
#else				/* USG */
	ioctl(i, TCGETA, (char *)&tty);
#endif				/* USG */
# ifdef		TIOCLGET	/* Berkeley 4BSD */
/* Moved out of USG/non-USG since Locus has both */
/* Turned back off with ifndef when tcgetattr() added.*/

	ioctl(i, TIOCGLTC, (char *)&olttyc);
	nlttyc = olttyc;
# endif				/* TIOCGETC */
#endif				/* !_POSIX_SOURCE */
}

/*
 * sTTY: set the tty modes on file descriptor i to be what's
 * currently in global "tty".  (Also use nttyc if needed.)
 */
static void sTTY(int i)
{
/*
 * Moved this out of ifndef USG since we have a system with both the
 * USG ioctl's and Berkeley ^Z, etc. (I do this before the TCSETAW so
 * the non-Berkeley chars will be overwritten). Turned back off with
 * ifndef when tcsetattr() added.
 */
# ifdef _POSIX_SOURCE
	tcsetattr(i, TCSADRAIN, &tty);
# else					/* _POSIX_SOURCE */
# ifndef TIOCSLTC
	ioctl(i, TIOCSLTC, &nlttyc);
# endif

#ifndef USG

# ifdef TIOCSETN
	/* Don't flush typeahead if we don't have to */
	ioctl(i, TIOCSETN, &tty);
# else
	/* We have to.  Too bad. */
	stty(i, &tty);
# endif

# ifdef TIOCGETC
	/* Update the other random chars while we're at it. */
	ioctl(i, TIOCSETC, &nttyc);
# endif
#else
	tcsetattr(i,TCSANOW, &posnttyc);
	/* USG 3 very simple: just set everything */
	ioctl(i, TCSETAW, (char *)&tty);
#endif
#endif					/* _POSIX_SOURCE */
}

/*
 * Print newline, or blank if in open/visual
 */
void noonl(void)
{

	ex_putchar(Outchar != termchar ? ' ' : '\n');
}

