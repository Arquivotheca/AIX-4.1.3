#if !defined(lint)
static char sccsid [] = "@(#)76  1.16  src/bos/usr/bin/ex/ex_v.c, cmdedit, bos41B, 9504A 12/19/94 11:46:54";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_v.c
 *
 * FUNCTIONS: fixzero, oop, ovbeg, ovend, savevis, setwind, undvis, vintr, vok,
 * vop, vsetsiz
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
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "ex.h"
#include "ex_re.h"
#include "ex_tty.h"
#include "ex_vis.h"
#include <limits.h>
#include <values.h>

void	ostop(ttymode);
ttymode	setty(ttymode);

static void ovbeg(void);
static void ovend(ttymode);
static void setwind(void);
static void vok(register wchar_t *);

/*
 * Entry points to open and visual from command mode processor.
 * The open/visual code breaks down roughly as follows:
 *
 * ex_v.c	entry points, checking of terminal characteristics
 *
 * ex_vadj.c	logical screen control, use of intelligent operations
 *		insert/delete line and coordination with screen image;
 *		updating of screen after changes.
 *
 * ex_vget.c	input of single keys and reading of input lines
 *		from the echo area, handling of \ escapes on input for
 *		uppercase only terminals, handling of memory for repeated
 *		commands and small saved texts from inserts and partline
 *		deletes, notification of multi line changes in the echo
 *		area.
 *
 * ex_vmain.c	main command decoding, some command processing.
 *
 * ex_voperate.c   decoding of operator/operand sequences and
 *		contextual scans, implementation of word motions.
 *
 * ex_vops.c	major operator interfaces, undos, motions, deletes,
 *		changes, opening new lines, shifts, replacements and yanks
 *		coordinating logical and physical changes.
 *
 * ex_vops2.c	subroutines for operator interfaces in ex_vops.c,
 *		insert mode, read input line processing at lowest level.
 *
 * ex_vops3.c	structured motion definitions of ( ) { } and [ ] operators,
 *		indent for lisp routines, () and {} balancing. 
 *
 * ex_vput.c	output routines, clearing, physical mapping of logical cursor
 *		positioning, cursor motions, handling of insert character
 *		and delete character functions of intelligent and unintelligent
 *		terminals, visual mode tracing routines (for debugging),
 *		control of screen image and its updating.
 *
 * ex_vwind.c	window level control of display, forward and backward rolls,
 *		absolute motions, contextual displays, line depth determination
 */

static jmp_buf venv;
/*
 * Enter open mode
 */
#ifdef u370
static	wchar_t	atube[TUBESIZE+LBSIZE];
#endif
void oop(void)
{
	register wchar_t *ic;
#ifndef u370
	wchar_t atube[TUBESIZE + LBSIZE];
#endif
	ttymode f;	/* mjm: was register */

	if (setjmp(venv)) {
		setsize();
		initev = (wchar_t *)0;
		inopen = 0;
		addr1 = addr2 = dot;
	}
	signal(SIGWINCH, winchk);
	ovbeg();
	if (peekchar() == '/') {
		ignore(compile(ex_getchar(), 1));
		savere(scanre);
		if (execute(0, dot) == 0)
			error(MSGSTR(M_220, "Fail|Pattern not found on addressed line"), DUMMY_INT);
		ic = loc1;
		if (ic > linebuf && *ic == 0)
			ic--;
	} else {
		getDOT();
		ic = vskipwh(linebuf);
	}
	donewline();

	/*
	 * If overstrike then have to HARDOPEN
	 * else if can move cursor up off current line can use CRTOPEN (~~vi1)
	 * otherwise (ugh) have to use ONEOPEN (like adm3)
	 */
	if (over_strike && !erase_overstrike)
		bastate = HARDOPEN;
	else if (cursor_address || cursor_up)
		bastate = CRTOPEN;
	else
		bastate = ONEOPEN;
	setwind();

	/*
	 * To avoid bombing on glass-crt's when the line is too long
	 * pretend that such terminals are 160 columns wide.
	 * If a line is too wide for display, we will dynamically
	 * switch to hardcopy open mode.
	 */
	if (state != CRTOPEN)
		WCOLS = TUBECOLS;
	if (!inglobal)
		savevis();
	vok(atube);
	if (state != CRTOPEN)
		columns = WCOLS;
	Outchar = (void(*)(int))vputchar;
	f = ostart();
	if (state == CRTOPEN) {
		if (outcol == UKCOL)
			outcol = 0;
                        vmoveitup(1, (short)1);
	} else
		outline = destline = WBOT;
	vshow(dot, NOLINE);
	vnline(ic);
	vmain();
	if (state != CRTOPEN)
		vclean();
	Command = "open";
	ovend(f);
	signal(SIGWINCH, SIG_DFL);
}

static void ovbeg(void)
{

	if (inopen)
		error(MSGSTR(M_221, "Recursive open/visual not allowed"), DUMMY_INT);
	if (value(NOVICE) && !initev)
		error(MSGSTR(M_222, "open/visual not allowed from novice mode"), DUMMY_INT);
	Vlines = lineDOL();
	fixzero();
	setdot();
	pastwh();
	dot = addr2;
}

static void ovend(ttymode f)
{

	splitw++;
	vgoto(WECHO, 0);
	vclreol();
	vgoto(WECHO, 0);
	holdcm = 0;
	splitw = 0;
	ostop(f);
	setoutt();
	undvis();
	columns = OCOLUMNS;
	inopen = 0;
	flusho();
	netchHAD(Vlines);
}

/*
 * Enter visual mode
 */
void vop(void)
{
	register int c;
#ifndef u370
	char atube[TUBESIZE + LBSIZE];
#endif
	ttymode f;	/* mjm: was register */
	extern char termtype[];
	sigset_t setsig;

	if (!cursor_address && !cursor_up) {
		if (initev) {
toopen:
			if (generic_type)
				smerror(MSGSTR(M_223, "I don't know what kind of terminal you are on - all I have is '%s'."), termtype);
			putNFL();
			merror(MSGSTR(M_224, "[Using open mode]"), DUMMY_INT);
			putNFL();
			oop();
			return;
		}
		error(MSGSTR(M_225, "Visual needs addressible cursor or upline capability"), DUMMY_INT);
	}
	if (over_strike && !erase_overstrike) {
		if (initev)
			goto toopen;
		error(MSGSTR(M_226, "Can't use visual on a terminal which overstrikes"), DUMMY_INT);
	}
	if (!clear_screen) {
		if (initev)
			goto toopen;
		error(MSGSTR(M_227, "Visual requires clear screen capability"), DUMMY_INT);
	}
	if (!scroll_forward) {
		if (initev)
			goto toopen;
		error(MSGSTR(M_228, "Visual requires scrolling"), DUMMY_INT);
	}

	if (setjmp(venv)) {
		setsize();
		initev = (wchar_t *)0;
		inopen = 0;
		addr1 = addr2 = dot;
	}

	ovbeg();
	bastate = VISUAL;
	c = 0;
	if (any(peekchar(), "+-^."))
		c = ex_getchar();
	pastwh();
	vsetsiz(iswdigit(peekchar()) ? getnum() : value(WINDOW));
	setwind();
	donewline();
	vok((wchar_t *)atube);

	if (vdepth() > basWLINES)
		error(MSGSTR(M_249, "Line too long for current screen size."), DUMMY_INT);

	if (!inglobal)
		savevis();
	Outchar = (void(*)(int))vputchar;
	vmoving = 0;
	f = ostart();
	if (initev == 0) {
		vcontext(((line *)dot), ((wchar_t)c));
		vnline(NOWCSTR);
	}

	sigemptyset(&setsig);
	sigaddset(&setsig, SIGWINCH);
	sigprocmask(SIG_BLOCK, &setsig, NULL);
	sigwinch_blocked++;		/* this will get unblocked after
					 * initialization in vmain. we want
					 * to at least get the file read in
					 * before we get sigwinch */
	signal(SIGWINCH, winchk);

	vmain();
	Command = "visual";
	ovend(f);
	signal(SIGWINCH, SIG_DFL);
}

/*
 * Modification to allow entry to visual with
 * empty buffer since routines internally
 * demand at least one line.
 */
void fixzero(void)
{

	if (dol == zero) {
		register short ochng = chng;

		vdoappend(WCemptystr);
		if (!ochng)
			ex_sync();
		addr1 = addr2 = one;
	} else if (addr2 == zero)
		addr2 = one;
}

/*
 * Save lines before visual between unddol and truedol.
 * Accomplish this by throwing away current [unddol,truedol]
 * and then saving all the lines in the buffer and moving
 * unddol back to dol.  Don't do this if in a global.
 *
 * If you do
 *	g/xxx/vi.
 * and then do a
 *	:e xxxx
 * at some point, and then quit from the visual and undo
 * you get the old file back.  Somewhat weird.
 */
void savevis(void)
{

	if (inglobal)
		return;
	truedol = unddol;
	saveall();
	unddol = dol;
	undkind = UNDNONE;
}

/*
 * Restore a sensible state after a visual/open, moving the saved
 * stuff back to [unddol,dol], and killing the partial line kill indicators.
 */
void undvis(void)
{

	if (ruptible)
		signal(SIGINT, onintr);
	squish();
	pkill[0] = pkill[1] = 0;
	unddol = truedol;
	unddel = zero;
	undap1 = one;
	undap2 = dol + 1;
	undkind = UNDALL;
	if (undadot <= zero || undadot > dol)
		undadot = zero+1;
}

/*
 * Set the window parameters based on the base state bastate
 * and the available buffer space.
 */
static void setwind(void)
{

	WCOLS = columns;
	switch (bastate) {

	case ONEOPEN:
		if (auto_right_margin)
			WCOLS--;
		/* fall into ... */

	case HARDOPEN:
		basWTOP = WTOP = WBOT = WECHO = 0;
		ZERO = 0;
		holdcm++;
		break;

	case CRTOPEN:
		basWTOP = lines - 2;
		/* fall into */

	case VISUAL:
		ZERO = lines - TUBESIZE / WCOLS;
		if (ZERO < 0)
			ZERO = 0;
		if (ZERO > basWTOP)
			error(MSGSTR(M_229, "Screen too large for internal buffer"), DUMMY_INT);
		WTOP = basWTOP; WBOT = lines - 2; WECHO = lines - 1;
		break;
	}
	state = bastate;
	basWLINES = WLINES = WBOT - WTOP + 1;
}

/*
 * Can we use an open/visual on this terminal?
 * If so, then divide the screen buffer up into lines,
 * and initialize a bunch of state variables before we start.
 */
static void vok(register wchar_t *atube)
{
	register int i;

	if (WCOLS == SHRT_MAX)
		serror(MSGSTR(M_230, "Don't know enough about your terminal to use %s"), Command);
	if (WCOLS > TUBECOLS)
		error(MSGSTR(M_231, "Terminal too wide"), DUMMY_INT);
	if (WLINES >= TUBELINES || WCOLS * (WECHO - ZERO + 1) > TUBESIZE)
		error(MSGSTR(M_232, "Screen too large"), DUMMY_INT);

	vtube0 = atube;
	vclrcp(atube, WCOLS * (WECHO - ZERO + 1));
	for (i = 0; i < ZERO; i++)
		vtube[i] = (wchar_t *) 0;
	for (; i <= WECHO; i++)
		vtube[i] = atube, atube += WCOLS;
	for (; i < TUBELINES; i++)
		vtube[i] = (wchar_t *) 0;
	vutmp = atube;
	vundkind = VNONE;
	vUNDdot = 0;
	OCOLUMNS = columns;
	inopen = 1;
#ifdef CBREAK
	signal(SIGINT, vintr);
#endif
	vmoving = 0;
	splitw = 0;
	doomed = 0;
	holdupd = 0;
	Peekkey = 0;
	vcnt = vcline = 0;
	if (vSCROLL == 0)
		vSCROLL = value(SCROLL);
}

#ifdef CBREAK
void
vintr(int sig)
{
	signal(SIGINT, vintr);
	if (vcatch)
		onintr();
	ungetkey(ATTN);
	draino();
}
#endif

/*
 * Set the size of the screen to size lines, to take effect the
 * next time the screen is redrawn.
 */
void vsetsiz(int size)
{
	register int b;

	if (bastate != VISUAL)
		return;
	b = lines - 1 - size;
	if (b >= lines - 1)
		b = lines - 2;
	if (b < 0)
		b = 0;
	basWTOP = b;
	basWLINES = WBOT - b + 1;
}

void
winchk(int signumber)
{
	signal(SIGWINCH, winchk);
	vsave();
	setty(normf);
	longjmp(venv, 1);
}
