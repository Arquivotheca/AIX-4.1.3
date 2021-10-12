#if !defined(lint)
static char sccsid[] = "@(#)99	1.33  src/bos/usr/bin/ex/ex_vmain.c, cmdedit, bos41J, 9513A_all 3/28/95 16:26:46";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_vmain.c
 *
 * FUNCTION: grabtag, prepapp, vmain, vremote, vsave, vzop
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
#include "ex_argv.h"
#include "ex_tty.h"
#include "ex_vis.h"

/*
 * This is the main routine for visual.
 * We here decode the count and possible named buffer specification
 * preceding a command and interpret a few of the commands.
 * Commands which involve a target (i.e. an operator) are decoded
 * in the routine operate in ex_voperate.c.
 */

#define forbid(a)	{ if (a) goto fonfon; }
#define VS2WCS(s)	(mbstowcs(vbufc,s,WCSIZE(vbufc)),vbufc)

/* Macro & buffer for '~' case switching */
#define TOGGLE_CASE(c)	if (iswalpha(c)) \
				c = iswupper(c) ? towlower(c) : towupper(c)
static wchar_t tbuf[LINE_MAX*3+3];

/* AIX security enhancement */
#if !defined(TVI)
static void grabtag(void);
#endif
/* TCSEC Division C Class C2 */
static void vzop(short, int, wchar_t);
void common_insert(int c,int cnt);
int map(wchar_t, struct maps *, int *);
void ostop(ttymode);

static wchar_t vbufc[10];

void vmain(void)
{
	register wchar_t c;
	register int cnt, i;
	wchar_t esave[TUBECOLS];
	wchar_t *oglobp;
	short d;
	line *addr;
	int ind, nlput;
	int shouldpo = 0;
	int onumber, olist;
	void (*OPline)(int), (*OPutchar)(int);
	sigset_t setsig;
/* D99366 */
	int PressedCtrlZ = 0;
	extern int SuspendCharRemapped;

	vch_mac = VC_NOTINMAC;
	vflickp = NULL;         /* No mode change by macro yet */

	/*
	 * If we started as a vi command (on the command line)
	 * then go process initial commands (recover, next or tag).
	 */
	if (initev) {
		oglobp = globp;
		globp = initev;
		hadcnt = cnt = 0;
		i = tchng;
		addr = dot;
		goto doinit;
	}
	else if (sigwinch_blocked--) {
		/*
		 * We block sigwinch at startup so we can at least get the
		 * file read in before we get any sigwinch'es.  We will
		 * either unblock here or down inside the above goto
		 * (doinit).
		 */
		sigemptyset(&setsig);
		sigaddset(&setsig, SIGWINCH);
		sigprocmask(SIG_UNBLOCK, &setsig, NULL);
	}

#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in vmain() at vshowmode()\n");
#endif
	vshowmode("");		/* As a precaution */
	/*
	 * NB:
	 *
	 * The current line is always in the line buffer linebuf,
	 * and the cursor at the position cursor.  You should do
	 * a vsave() before moving off the line to make sure the disk
	 * copy is updated if it has changed, and a getDOT() to get
	 * the line back if you mung linebuf.  The motion
	 * routines in ex_vwind.c handle most of this.
	 */
	for (;;) {
		/*
		 * Decode a visual command.
		 * First sync the temp file if there has been a reasonable
		 * amount of change.  Clear state for decoding of next
		 * command.
		 */
		TSYNC();
		vglobp = 0;
		vreg = 0;
		hold = 0;
		seenprompt = 1;
		wcursor = 0;
		Xhadcnt = hadcnt = 0;
		Xcnt = cnt = 1;
		splitw = 0;
		if (i = holdupd) {
			if (state == VISUAL)
				ignore(peekkey());
			holdupd = 0;
			if (state != VISUAL) {
				vcnt = 0;
				vsave();
				vrepaint(cursor);
			} else if (i == 3)
				vredraw(WTOP);
			else
				vsync(WTOP);
			vfixcurs();
		}

		/*
		 * Gobble up counts and named buffer specifications.
		 */
		for (;;) {
looptop:
#ifdef MDEBUG
			if (trace)
				fprintf(trace,"trace in vmain at for (::), pc=%c\n",peekkey());
#endif
			if (iswdigit(peekkey()) && peekkey() != '0') {
				hadcnt = 1;
				cnt = vgetcnt();
				forbid (cnt <= 0);
			}
			if (peekkey() != '"')
				break;
			ignore(getkey()), c = getkey();
			/*
			 * Buffer names be letters or digits.
			 * But not '0' as that is the source of
			 * an 'empty' named buffer spec in the routine
			 * kshift (see ex_temp.c).
			 */
			/* use the ascii functions not the isw() functions
			   as we don't want to match non-ascii alpha chars  */

			forbid (c == '0' || !isalpha(c) && !isdigit(c));
			vreg = c;
		}
reread:
		/*
		 * Come to reread from below after some macro expansions.
		 * The call to map allows use of function key pads
		 * by performing a terminal dependent mapping of inputs.
		 */
#ifdef MDEBUG
		if (trace)
			fprintf(trace,"trace in vmain before getkey(), pcb=%C\n",peekkey());
#endif
		op = getkey();
/* D99366 */
		if (op == Ctrl('Z'))
			PressedCtrlZ = 1;

		maphopcnt = 0;
		do {
			/*
			 * Keep mapping the wchar_t as long as it changes.
			 * This allows for double mappings, e.g., q to #,
			 * #1 to something else.
			 */
			c = op;
			op = map(c,arrows, NULL);
#ifdef MDEBUG
			if (trace)
				fprintf(trace, "trace in vmain after map(), pca=%c\n",c);
#endif
			/*
			 * Maybe the mapped to wchar_t is a count. If so, we have
			 * to go back to the "for" to interpret it. Likewise
			 * for a buffer name.
			 */
			if ((iswdigit(c) && c!='0') || c == '"') {
				ungetkey(c);
				goto looptop;
			}
			if (!value(REMAP)) {
				c = op;
				break;
			}
			if (++maphopcnt > 256)
				error(MSGSTR(M_239, "Infinite macro loop"), DUMMY_INT);
		} while (c != op);

		/*
		 * Begin to build an image of this command for possible
		 * later repeat in the buffer workcmd.	It will be copied
		 * to lastcmd by the routine setLAST
		 * if/when completely specified.
		 */
		lastcp = workcmd;
		if (!vglobp)
			*lastcp++ = c;

#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in vmain() at switch (c)\n");
#endif
		/*
		 * First level command decode.
		 */
		switch (c) {

		/*
		 * ^L		Clear screen e.g. after transmission error.
		 */

		/*
		 * ^R		Retype screen, getting rid of @ lines.
		 *		If in open, equivalent to ^L.
		 *		On terminals where the right arrow key sends
		 *		^L we make ^R act like ^L, since there is no
		 *		way to get ^L.	These terminals (adm31, tvi)
		 *		are intelligent so ^R is useless.  Soroc
		 *		will probably foul this up, but nobody has
		 *		one of them.
		 */
		case Ctrl('L'):
		case Ctrl('R'):
			if (c == Ctrl('L') || (key_right && *key_right==Ctrl('L'))) {
				vclear();
				vdirty(0, vcnt);
			}
			if (state != VISUAL) {
				/*
				 * Get a clean line, throw away the
				 * memory of what is displayed now,
				 * and move back onto the current line.
				 */
				vclean();
				vcnt = 0;
				vmoveto(dot, cursor, (wchar_t)0);
				continue;
			}
			else if (Outchar != (void (*)(int)) vputchar)
				Outchar = (void (*)(int)) vputchar;
			vredraw(WTOP);
			/*
			 * Weird glitch -- when we enter visual
			 * in a very small window we may end up with
			 * no lines on the screen because the line
			 * at the top is too long.  This forces the screen
			 * to be expanded to make room for it (after
			 * we have printed @'s ick showing we goofed).
			 */
			if (vcnt == 0)
				vrepaint(cursor);
			vfixcurs();
			continue;

		/*
		 * $		Escape just cancels the current command
		 *		with a little feedback.
		 */
		case ESCAPE:
			ex_beep();
			continue;

		/*
		 * @		Macros. Bring in the macro and put it
		 *		in vmacbuf, point vglobp there and punt.
		 */
		 case '@':
			c = getesc();
			if (c == 0)
				continue;
			if (c == '@')
				c = lastmac;
			if (iswupper(c))
				c = towlower(c);
			forbid(!iswlower(c));
			lastmac = c;
			vsave();
			CATCH
				wchar_t tmpbuf[LBSIZE];

				regbuf(c,tmpbuf,WCSIZE(vmacbuf));
				macpush(tmpbuf, 1);
			ONERR
				lastmac = 0;
				splitw = 0;
				getDOT();
				vrepaint(cursor);
				continue;
			ENDCATCH
			vmacp = vmacbuf;
			goto reread;

		/*
		 * .		Repeat the last (modifying) open/visual command.
		 */
		case '.':
			/*
			 * Check that there was a last command, and
			 * take its count and named buffer unless they
			 * were given anew.  Special case if last command
			 * referenced a numeric named buffer -- increment
			 * the number and go to a named buffer again.
			 * This allows a sequence like "1pu.u.u...
			 * to successively look for stuff in the kill chain
			 * much as one does in EMACS with C-Y and M-Y.
			 */
			forbid (lastcmd[0] == 0);
			if (hadcnt)
				lastcnt = cnt;
			if (vreg)
				lastreg = vreg;
			else if (iswdigit(lastreg) && lastreg < '9')
				lastreg++;
			vreg = lastreg;
			cnt = lastcnt;
			hadcnt = lasthad;
			vglobp = lastcmd;
			goto reread;

		/*
		 * ^U		Scroll up.  A count sticks around for
		 *		future scrolls as the scroll amount.
		 *		Attempt to hold the indentation from the
		 *		top of the screen (in logical lines).
		 *
		 * BUG: 	A ^U near the bottom of the screen
		 *		on a dumb terminal (which can't roll back)
		 *		causes the screen to be cleared and then
		 *		redrawn almost as it was.  In this case
		 *		one should simply move the cursor.
		 */
		case Ctrl('U'):
			if (hadcnt)
				vSCROLL = cnt;
			cnt = vSCROLL;
			if (state == VISUAL)
				ind = vcline, cnt += ind;
			else
				ind = 0;
			vmoving = 0;
			vup(cnt, ind, (short)1);
			vnline(NOWCSTR);
			continue;

		/*
		 * ^D		Scroll down.  Like scroll up.
		 */
		case Ctrl('D'):
#ifdef TRACE
		if (trace)
			fprintf(trace, "vmain: before vdown in ^D, dot=%d, wdot=%d, dol=%d\n", lineno(dot), lineno(wdot), lineno(dol));
#endif
			if (hadcnt)
				vSCROLL = cnt;
			cnt = vSCROLL;
			if (state == VISUAL)
				ind = vcnt - vcline - 1, cnt += ind;
			else
				ind = 0;
			vmoving = 0;
			vdown(cnt, ind, (short)1);
#ifdef TRACE
		if (trace)
			fprintf(trace, "vmain: before vnline in ^D, dot=%d, wdot=%d, dol=%d\n", lineno(dot), lineno(wdot), lineno(dol));
#endif
			vnline(NOWCSTR);
#ifdef TRACE
		if (trace)
			fprintf(trace, "after vnline in ^D, dot=%d, wdot=%d, dol=%d\n", lineno(dot), lineno(wdot), lineno(dol));
#endif
			continue;

		/*
		 * ^E		Glitch the screen down (one) line.
		 *		Cursor left on same line in file.
		 */
		case Ctrl('E'):
			if (state != VISUAL)
				continue;
			if (!hadcnt)
				cnt = 1;
			/* Bottom line of file already on screen */
			forbid(lineDOL()-lineDOT() <= vcnt-1-vcline);
			ind = vcnt - vcline - 1 + cnt;
			vdown(ind, ind, (short)1);
			vnline(cursor);
			continue;

		/*
		 * ^Y		Like ^E but up
		 */
		case Ctrl('Y'):
			if (state != VISUAL)
				continue;
			if (!hadcnt)
				cnt = 1;
			forbid(lineDOT()-1<=vcline); /* line 1 already there */
			ind = vcline + cnt;
			vup(ind, ind, (short)1);
			vnline(cursor);
			continue;


		/*
		 * m		Mark position in mark register given
		 *		by following letter.  Return is
		 *		accomplished via ' or `; former
		 *		to beginning of line where mark
		 *		was set, latter to column where marked.
		 */
		case 'm':
			/*
			 * Getesc is generally used when a character
			 * is read as a latter part of a command
			 * to allow one to hit rubout/escape to cancel
			 * what you have typed so far.	These characters
			 * are mapped to 0 by the subroutine.
			 */
			c = getesc();
			if (c == 0)
				continue;

			/*
			 * Markreg checks that argument is a letter
			 * and also maps ' and ` to the end of the range
			 * to allow '' or `` to reference the previous
			 * context mark.
			 */
			c = markreg(c);
			forbid (c == 0);
			vsave();
			names[c - 'a'] = (*dot &~ 01);
			ncols[c - 'a'] = cursor;
			anymarks = 1;
			continue;

		/*
		 * ^F		Window forwards, with 2 lines of continuity.
		 *		Count repeats.
		 */
		case Ctrl('F'):
			vsave();
			if (vcnt > 2) {
				addr = dot + (vcnt - vcline) - 2 + (cnt-1)*(basWLINES-2);
				forbid(addr >= dol);
				dot = addr;
				vcnt = vcline = 0;
			} else {
				/* if there are only one or two lines on the
				 * screen, then move forward by one line to
				 * make sure that the user gets to see some
				 * new text. */
				forbid(dot+1 >= dol);
				dot++;
				vcnt = vcline = 0;
			}
			vzop((short)0, 0, (wchar_t)'+');
			continue;

		/*
		 * ^B		Window backwards, with 2 lines of continuity.
		 *		Inverse of ^F.
		 */
		case Ctrl('B'):
			vsave();
			if (one + vcline != dot && vcnt > 2) {
				line *top;

				addr = dot - vcline + 1 - (cnt-1)*(basWLINES-2);
				forbid (addr < one);
				dot = addr;
				vmoving = 0;

				getline(*addr);
				top = vback(addr, basWLINES - vdepth());
				vcnt = vcline = 0;
				vclean();
				if ((top+1 == dot) && (top > one))
					top--;
				vshow(addr, top);
				vnline(0);
			} else
				vzop((short)0, 0, (wchar_t)'^');
			continue;

		/*
		 * z		Screen adjustment, taking a following character:
		 *			zcarriage_return		current line to top
		 *			z<NL>		like zcarriage_return
		 *			z-		current line to bottom
		 *		also z+, z^ like ^F and ^B.
		 *		A preceding count is line to use rather
		 *		than current line.  A count between z and
		 *		specifier character changes the screen size
		 *		for the redraw.
		 *
		 */
		case 'z':
			if (state == VISUAL) {
				i = vgetcnt();
				if (i > 0)
					vsetsiz(i);
				c = getesc();
				if (c == 0)
					continue;
			}
			vsave();
			vzop(hadcnt, cnt, c);
			continue;

		/*
		 * Y		Yank lines, abbreviation for y_ or yy.
		 *		Yanked lines can be put later if no
		 *		changes intervene, or can be put in named
		 *		buffers and put anytime in this session.
		 */
		case 'Y':
			ungetkey('_');
			c = 'y';
			break;

		/*
		 * J		Join lines, 2 by default.  Count is number
		 *		of lines to join (no join operator sorry.)
		 */
		case 'J':
			forbid (dot == dol);
			if (cnt == 1)
				cnt = 2;
			if (cnt > (i = dol - dot + 1))
				cnt = i;
			vsave();
			vmacchng((short)1);
			setLAST();
			cursor = WCstrend(linebuf);
			vremote(cnt, join, 0);
			notenam = "join";
			vmoving = 0;
			killU();
			vreplace(vcline, cnt, 1);
			if (!*cursor && cursor > linebuf)
				cursor--;
			if (notecnt == 2)
				notecnt = 0;
			vrepaint(cursor);
			continue;

		/*
		 * S		Substitute text for whole lines, abbrev for c_.
		 *		Count is number of lines to change.
		 */
		case 'S':
			ungetkey('_');
			c = 'c';
			break;

		/*
		 * O		Create a new line above current and accept new
		 *		input text, to an escape, there.
		 *		A count specifies, for dumb terminals when
		 *		slowopen is not set, the number of physical
		 *		line space to open on the screen.
		 *
		 * o		Like O, but opens lines below.
		 */
		case 'O':
		case 'o':
			vmacchng((short)1);
			voOpen(c, cnt);
			continue;

		/*
		 * C		Change text to end of line, short for c$.
		 */
		case 'C':
			if (*cursor) {
				ungetkey('$'), c = 'c';
				break;
			}
			goto appnd;

		/*
		 * ~	Switch case of letter under cursor
		 */
		case '~':
			{
				int i, j;

				setLAST();
				j=0;
				for(i=0; i<cnt && cursor[i]; i++) {
					tbuf[j++] = 'r';
					tbuf[j] = cursor[i];
					TOGGLE_CASE(tbuf[j]);
					j++;
					tbuf[j++] = cursor[i+1]==0 ? 0 : ' ';
				}
				tbuf[j++] = 0;
				macpush(tbuf, 1);
			}
			continue;


		/*
		 * A		Append at end of line, short for $a.
		 */
		case 'A':
			operate((wchar_t)'$', 1);
appnd:
			c = 'a';
			/* fall into ... */

		/*
		 * a		Appends text after cursor.  Text can continue
		 *		through arbitrary number of lines.
		 */
		case 'a':
			if (*cursor) {
				if (state == HARDOPEN)
					ex_putchar(*cursor);
				cursor++;
			}
			goto insrt;

		/*
		 * I		Insert at beginning of whitespace of line,
		 *		short for ^i.
		 */
		case 'I':
			operate((wchar_t)'^', 1);
			c = 'i';
			/* fall into ... */

		/*
		 * R		Replace characters, one for one, by input
		 *		(logically), like repeated r commands.
		 *
		 * BUG: 	This is like the typeover mode of many other
		 *		editors, and is only rarely useful.  Its
		 *		implementation is a modification in a low level
		 *		routine and it doesn't work very well, e.g.
		 *		you can't move around within a R, etc.
		 *//*unsure*/
		case 'R':
			/* fall into... */

		/*
		 * i		Insert text to an escape in the buffer.
		 *		Text is arbitrary.  This command reminds of
		 *		the i command in bare teco.
		 */
		case 'i':
insrt:
			/*
			 * Common code for all the insertion commands.
			 * Save for redo, position cursor, prepare for append
			 * at command and in visual undo.  Note that nothing
			 * is doomed, unless R when all is, and save the
			 * current line in a the undo temporary buffer.
			 */
                        common_insert(c,cnt);
                        continue;

		/*
		 * ^?		An attention, normally a ^?, just beeps.
		 *		If you are a vi command within ex, then
		 *		two ATTN's will drop you back to command mode.
		 */
		case ATTN:
			ex_beep();
			if (initev || peekkey() != ATTN)
				continue;
			/* fall into... */

		/*
		 * ^\		A quit always gets command mode.
		 */
		case QUIT:
			/*
			 * Have to be careful if we were called
			 *	g/xxx/vi
			 * since a return will just start up again.
			 * So we simulate an interrupt.
			 */
			if (inglobal)
				onintr();
			/* fall into... */

		/*
		 * q		Quit back to command mode, unless called as
		 *		vi on command line in which case dont do it
		 */
		case 'q':	/* quit */
			if (initev) {
				vsave();
				CATCH
					error(MSGSTR(M_240, "Q gets ex command mode, :q leaves vi"), DUMMY_INT);
				ENDCATCH
				splitw = 0;
				getDOT();
				vrepaint(cursor);
				continue;
			}
			/* fall into... */

		/*
		 * Q		Is like q, but always gets to command mode
		 *		even if command line invocation was as vi.
		 */
		case 'Q':
/* AIX security enhancement */
#if defined(TVI)
			tvierror(MSGSTR(M_502, "Q command not allowed"), DUMMY_CHARP);
			break;
#else
			vsave();
			/*
			 * If we are in the middle of a macro, throw away
			 * the rest and fix up undo.
			 * This code copied from getbr().
			 */
			if (vmacp) {
				vmacp = 0;
				if (inopen == -1)	/* don't screw up undo for esc esc */
					vundkind = VMANY;
				inopen = 1;	/* restore old setting now that macro done */
			}
			return;
#endif
/* TCSEC Division C Class C2 */

		/*
		 * ZZ		Like :x
		 */
		 case 'Z':
			forbid(getkey() != 'Z');
			oglobp = globp;
			globp = VS2WCS("x");
			vclrech((short)0);
			goto gogo;

		/*
		 * P		Put back text before cursor or before current
		 *		line.  If text was whole lines goes back
		 *		as whole lines.  If part of a single line
		 *		or parts of whole lines splits up current
		 *		line to form many new lines.
		 *		May specify a named buffer, or the delete
		 *		saving buffers 1-9.
		 *
		 * p		Like P but after rather than before.
		 */
		case 'P':
		case 'p':
			vmoving = 0;
			/*
			 * If previous delete was partial line, use an
			 * append or insert to put it back so as to
			 * use insert mode on intelligent terminals.
			 */
			if (!vreg && DEL[0]) {
				forbid (DEL_OVFLOW);
				setLAST();
				vglobp = DEL;
                                pp_insert(c,1,i);  /** special insert **/
				continue;
			}

			/*
			 * If a register wasn't specified, then make
			 * sure there is something to put back.
			 */
			forbid (!vreg && unddol == dol);
			/*
			 * If we just did a macro the whole buffer is in
			 * the undo save area.	We don't want to put THAT.
			 */
			forbid (vundkind == VMANY && undkind==UNDALL);
			vsave();
			vmacchng((short)1);
			setLAST();
			i = 0;
			if (vreg && partreg(vreg) || !vreg && pkill[0]) {
				/*
				 * Restoring multiple lines which were partial
				 * lines; will leave cursor in middle
				 * of line after shoving restored text in to
				 * split the current line.
				 */
				i++;
				if (c == 'p' && *cursor)
					cursor++;
			} else {
				/*
				 * In whole line case, have to back up dot
				 * for P; also want to clear cursor so
				 * cursor will eventually be positioned
				 * at the beginning of the first put line.
				 */
				cursor = 0;
				if (c == 'P') {
					dot--, vcline--;
					c = 'p';
				}
			}
			killU();

			/*
			 * The call to putreg can potentially
			 * bomb since there may be nothing in a named buffer.
			 * We thus put a catch in here.  If we didn't and
			 * there was an error we would end up in command mode.
			 */
			addr = dol;	/* old dol */
			CATCH
				vremote(1, vreg ? (void(*)(int))putreg : 
                                                  (void(*)(int))put, vreg);
			ONERR
				if (vreg == -1) {
					splitw = 0;
					if (op == 'P')
						dot++, vcline++;
					goto pfixup;
				}
			ENDCATCH
			splitw = 0;
			nlput = dol - addr + 1;
			if (!i) {
				/*
				 * Increment undap1, undap2 to make up
				 * for their incorrect initialization in the
				 * routine vremote before calling put/putreg.
				 */
				if (FIXUNDO)
					undap1++, undap2++;
				vcline++;
				nlput--;

				/*
				 * After a put want current line first line,
				 * and dot was made the last line put in code
				 * run so far.	This is why we increment vcline
				 * above and decrease dot here.
				 */
				dot -= nlput - 1;
			}
#ifdef TRACE
			if (trace)
				fprintf(trace, "vmain: vreplace(%d, %d, %d), undap1=%d, undap2=%d, dot=%d\n", vcline, i, nlput, lineno(undap1), lineno(undap2), lineno(dot));
#endif
			vreplace(vcline, i, nlput);
			if (state != VISUAL) {
				/*
				 * Special case in open mode.
				 * Force action on the screen when a single
				 * line is put even if it is identical to
				 * the current line, e.g. on YP; otherwise
				 * you can't tell anything happened.
				 */
				vjumpto(dot, cursor, (wchar_t)'.');
				continue;
			}
pfixup:
			vrepaint(cursor);
			vfixcurs();
			continue;

		/*
		 * ^^		Return to previous file.
		 *		Like a :e #, and thus can be used after a
		 *		"No Write" diagnostic.
		 */
		/*
		 * ^A		Return to previous file.
		 *		Like a :e #, and thus can be used after a
		 *		"No Write" diagnostic.
		 */
		case Ctrl('^'):
	 	case Ctrl('A'):
			forbid (hadcnt);
			vsave();
			ckaw();
			oglobp = globp;
			if (value(AUTOWRITE) && !value(READONLY)) {
				globp = VS2WCS("e! #");
			} else {
				globp = VS2WCS("e #");
			}
			goto gogo;

		/*
		 * ^]		Takes word after cursor as tag, and then does
		 *		tag command.  Read ``go right to''.
		 */
		case Ctrl(']'):
		/*
		 * ^T		Takes word after cursor as tag, and then does
		 *		tag command.
		 */
		case Ctrl('T'):
/* AIX security enhancement */
#if !defined(TVI)
			grabtag();
			oglobp = globp;
			globp = VS2WCS("tag");
			goto gogo;
#else
			tvierror(MSGSTR(M_500, "Tags not allowed"), DUMMY_CHARP);
			break;
#endif
/* TCSEC Division C Class C2 */
		/*
		 * &		Like :&
		 */
		 case '&':
			setLAST();
			oglobp = globp;
			globp = VS2WCS("&");
			goto gogo;

		/*
		 * ^G		Bring up a status line at the bottom of
		 *		the screen, like a :file command.
		 *
		 * BUG: 	Was ^S but doesn't work in cbreak mode
		 */
		case Ctrl('G'):
			oglobp = globp;
			globp = VS2WCS("file");
gogo:
			addr = dot;
			vsave();
			goto doinit;

#ifdef SIGTSTP
		/*
		 * ^Z:	suspend editor session and temporarily return
		 *	to shell.  Only works with Berkeley/IIASA process
		 *	control in kernel.
		 */
		case Ctrl('Z'):
/* D99366 */
        /* 
		 * Checking for the following:
		 * 1. If Ctrl Z pressed and Suspend character is not remapped
		 *    if condition is valid
		 * 2. If Crtl Z pressed and suspend character is remapped then
		 *    if condition is not valid
		 * 3. If Suspend character is remapped then condition is valid
		 */
			if ( SuspendCharRemapped != PressedCtrlZ )
			{
				forbid(dosusp == 0);
# ifdef NTTYDISC
				forbid(ldisc != NTTYDISC);
# endif
				vsave();
				oglobp = globp;
				globp = VS2WCS("stop");
				goto gogo;
# endif
			}
			else
			{
				PressedCtrlZ = 0;
				break;
			}

		/*
		 * :		Read a command from the echo area and
		 *		execute it in command mode.
		 */
		case ':':
			forbid (hadcnt);
			vsave();
			i = tchng;
			addr = dot;
			if (readecho(c)) {
				esave[0] = 0;
				goto fixup;
			}
			getDOT();
			/*
			 * Use the visual undo buffer to store the global
			 * string for command mode, since it is idle right now.
			 */
			oglobp = globp; CP(vutmp, genbuf+1); globp = vutmp;
doinit:
			esave[0] = 0;
			fixech();

			/*
			 * Have to finagle around not to lose last
			 * character after this command (when run from ex
			 * command mode).  This is clumsy.
			 */
			d = peekc; ungetchar(0);
			if (shouldpo) {
				/*
				 * So after a "Hit return..." ":", we do
				 * another "Hit return..." the next time
				 */
				pofix();
				shouldpo = 0;
			}
			/*
			 * Save old values of options so we can
			 * notice when they change; switch into
			 * cooked mode so we are interruptible.
			 */
			onumber = value(NUMBER);
			olist = value(LIST);
			OPline = Pline;
			OPutchar = Putchar;

			CATCH
#ifndef CBREAK
				vcook();
#endif
				commands((short)1, (short)1);
				if (dot == zero && dol > zero)
					dot = one;
				if (initev && firstpat) {
					globp = firstpat;
					commands((short)1, (short)1);
					free(firstpat);
					firstpat = globp = NULL;
				}
#ifndef CBREAK
				vraw();
#endif
			ONERR
#ifndef CBREAK
				vraw();
#endif
				copy(esave, vtube[WECHO],
				     sizeof(wchar_t) * TUBECOLS);
			ENDCATCH
			fixol();
			Pline = OPline;
			Putchar = OPutchar;
			ungetchar(d);
			globp = oglobp;

			/*
			 * If we ended up with no lines in the buffer, make one.
			 */
			if (dot == zero) {
				fixzero();
			}
			splitw = 0;

			/*
			 * Special case: did list/number options change?
			 */
			if (onumber != value(NUMBER))
				setnumb(value(NUMBER));
			if (olist != value(LIST))
				setlist(value(LIST));

fixup:
			/*
			 * If a change occurred, other than
			 * a write which clears changes, then
			 * we should allow an undo even if .
			 * didn't move.
			 *
			 * BUG: You can make this wrong by
			 * tricking around with multiple commands
			 * on one line of : escape, and including
			 * a write command there, but its not
			 * worth worrying about.
			 */
			if (FIXUNDO && tchng && tchng != i)
				vundkind = VMANY, cursor = 0;

			/*
			 * If we are about to do another :, hold off
			 * updating of screen.
			 */
			if (vcnt < 0 && Peekkey == ':') {
				getDOT();
				shouldpo = 1;
				if (sigwinch_blocked--) {
					sigemptyset(&setsig);
					sigaddset(&setsig, SIGWINCH);
					sigprocmask(SIG_UNBLOCK, &setsig, NULL);
				}
				continue;
			}
			shouldpo = 0;

			/*
			 * In the case where the file being edited is
			 * new; e.g. if the initial state hasn't been
			 * saved yet, then do so now.
			 */
			if (unddol == truedol) {
				vundkind = VNONE;
				Vlines = lineDOL();
				if (!inglobal)
					savevis();
				addr = zero;
				vcnt = 0;
				if (esave[0] == 0)
					copy(esave, vtube[WECHO],
					     sizeof(wchar_t) * TUBECOLS);
			}

			/*
			 * If the current line moved reset the cursor position.
			 */
			if (dot != addr) {
				vmoving = 0;
				cursor = 0;
			}

			/*
			 * If current line is not on screen or if we are
			 * in open mode and . moved, then redraw.
			 */
			i = vcline + (dot - addr);
			if (i < 0 || i >= vcnt && i >= -vcnt || state != VISUAL && dot != addr) {
				if (state == CRTOPEN)
					vup1();
				if (vcnt > 0)
					vcnt = 0;
				vjumpto(dot, (wchar_t *) 0, (wchar_t)'.');
			} else {
				/*
				 * Current line IS on screen.
				 * If we did a [Hit return...] then
				 * restore vcnt and clear screen if in visual
				 */
				vcline = i;
				if (vcnt < 0) {
					vcnt = -vcnt;
					if (state == VISUAL)
						vclear();
					else if (state == CRTOPEN) {
						vcnt = 0;
					}
				}

				/*
				 * Limit max value of vcnt based on $
				 */
				i = vcline + lineDOL() - lineDOT() + 1;
				if (i < vcnt)
					vcnt = i;

				/*
				 * Dirty and repaint.
				 */
				vdirty(0, lines);
				vrepaint(cursor);
			}

			/*
			 * If in visual, put back the echo area
			 * if it was clobberred.
			 */
			if (state == VISUAL) {
				int sdc = destcol, sdl = destline;

				splitw++;
				vigoto(WECHO, 0);
				for (i = 0; i < TUBECOLS - 1; i++) {
					if (esave[i] == 0)
						break;
					vputchar(esave[i]);
				}
				splitw = 0;
				vgoto(sdl, sdc);
			}
			if (sigwinch_blocked) {
				sigemptyset(&setsig);
				sigaddset(&setsig, SIGWINCH);
				sigprocmask(SIG_UNBLOCK, &setsig, NULL);
				sigwinch_blocked = 0;
			}
			continue;

		/*
		 * u		undo the last changing command.
		 */
		case 'u':
			setLAST();
			vundo((short)1);
			continue;

		/*
		 * U		restore current line to initial state.
		 */
		case 'U':
			vUndo();
			continue;

fonfon:
			ex_beep();
			vmacp = 0;
			inopen = 1;	/* might have been -1 */
			continue;
		}
/* AIX security enhancement */
#if defined(TVI)
		if (c == '!')
 			tvierror(MSGSTR(M_501, "Shell escape not allowed"), DUMMY_CHARP);
#endif
/* TCSEC Division C Class C2 */

		/*
		 * Rest of commands are decoded by the operate
		 * routine.
		 */
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in vmain() at operate(c, cnt)\n");
#endif
		operate(c, cnt);
	}
}
/* AIX security enhancement */
#if !defined(TVI)
/*
 * Grab the word after the cursor so we can look for it as a tag.
 */
static void grabtag(void)
{
	register wchar_t *cp, *dp;

	cp = vpastwh(cursor);
	if (*cp) {
		dp = lasttag;
		do {
			if (dp < &lasttag[sizeof lasttag - 2])
				*dp++ = *cp;
			cp++;
		} while (iswalnum(*cp) || *cp == '_' || *cp == '$');
		*dp++ = 0;
	}
}
#endif
/* TCSEC Division C Class C2 */

/*
 * Before appending lines, set up addr1 and
 * the command mode undo information.
 */
void prepapp(void)
{

	addr1 = dot;
	deletenone();
	addr1++;
	appendnone();
}

/*
 * Execute function f with the address bounds addr1
 * and addr2 surrounding cnt lines starting at dot.
 */
void vremote(int cnt, void (*f)(int), int arg)
{
	register int oing = inglobal;

	addr1 = dot;
	addr2 = dot + cnt - 1;
	inglobal = 0;
	if (FIXUNDO)
		undap1 = undap2 = dot;
	(*f)(arg);
	inglobal = oing;
	if (FIXUNDO)
		vundkind = VMANY;
	vmcurs = 0;
}

/*
 * Save the current contents of linebuf, if it has changed.
 */
void vsave(void)
{
	wchar_t temp[LBSIZE];

	CP(temp, linebuf);
	if (FIXUNDO && vundkind == VCHNG || vundkind == VCAPU) {
		/*
		 * If the undo state is saved in the temporary buffer
		 * vutmp, then we sync this into the temp file so that
		 * we will be able to undo even after we have moved off
		 * the line.  It would be possible to associate a line
		 * with vutmp but we assume that vutmp is only associated
		 * with line dot (e.g. in case ':') above, so beware.
		 */
		prepapp();
		strcLIN(vutmp);
		putmark(dot);
		vremote(1, (void(*)(int))yank, 0);
		vundkind = VMCHNG;
		notecnt = 0;
		undkind = UNDCHANGE;
	}
	/*
	 * Get the line out of the temp file and do nothing if it hasn't
	 * changed.  This may seem like a loss, but the line will
	 * almost always be in a read buffer so this may well avoid disk i/o.
	 */
	getDOT();
	if (wcscmp( linebuf, temp) == 0)
		return;
	strcLIN(temp);
	putmark(dot);
}

#undef	forbid
#define forbid(a)	if (a) { ex_beep(); return; }

/*
 * Do a z operation.
 */
static void vzop(short hadcnt_vzop, int cnt, wchar_t c)
{
	register line *addr;

	if (state != VISUAL) {
		/*
		 * Z from open; always like a z=.
		 * This code is a mess and should be cleaned up.
		 */
		vmoveitup(1, (short)1);
		vgoto(outline, 0);
		ostop(normf);
		setoutt();
		addr2 = dot;
		vclear();
		destline = WECHO;
		zop2(Xhadcnt ? Xcnt : value(WINDOW) - 1, '=');
		if (state == CRTOPEN)
			putnl();
		putNFL();
		termreset();
		Outchar = (void(*)(int))vputchar;
		ostart();	
		vcnt = 0;
		outline = destline = 0;
		vjumpto(dot, cursor, (wchar_t)0);
		return;
	}
	if (hadcnt_vzop) {
		addr = zero + cnt;
		if (addr < one)
			addr = one;
		if (addr > dol)
			addr = dol;
		markit(addr);
	} else
		switch (c) {

		case '+':
			addr = dot + vcnt - vcline;
			break;

		case '^':
			addr = dot - vcline - 1;
			forbid (addr < one);
			c = '-';
			break;

		default:
			addr = dot;
			break;
		}
	switch (c) {

	case '.':
	case '-':
		break;

	case '^':
		forbid (addr <= one);
		break;

	case '+':
		forbid (addr >= dol);
		/* fall into ... */

	case '\r':
	case '\n':
		c = '\r';
		break;

	default:
		ex_beep();
		return;
	}
	vmoving = 0;
	vjumpto(addr, NOWCSTR, c);
}
static void pp_insert(int c,int cnt, int i)      /** P p insert - allows macros such as "dwPP" **/
{
wchar_t *save_vmacp;
        save_vmacp=vmacp;
        vmacp=0;        /** make sure vmacp is not pointing at macro code **/
        if ( c == 'p' && *cursor )
           cursor++;
        common_insert(c,cnt);
        vmacp=save_vmacp;       /** point to rest of macro command **/
}
/*
 * common_insert - Common code for all the insertion commands.
 * Save for redo, position cursor, prepare for append
 * at command and in visual undo.  Note that nothing
 * is doomed, unless R when all is, and save the
 * current line in a the undo temporary buffer.
 */
static void common_insert(int c,int cnt)
{
                        vmacchng((short)1);
                        setLAST();
                        vcursat(cursor);
                        prepapp();
                        vnoapp();
                        doomed = c == 'R' ? 10000 : 0;
                        if(FIXUNDO)
                                vundkind = VCHNG;
                        vmoving = 0;
                        CP(vutmp, linebuf);

                        /*
                         * If this is a repeated command, then suppress
                         * fake insert mode on dumb terminals which looks
                         * ridiculous and wastes lots of time even at 9600B.
                         */
                        if (vglobp)
                                hold = HOLDQIK;
                        vappend(c, cnt, 0);
}

