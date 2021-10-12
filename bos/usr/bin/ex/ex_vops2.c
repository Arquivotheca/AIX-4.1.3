#if !defined(lint)
static char sccsid [] = "@(#)02  1.22  src/bos/usr/bin/ex/ex_vops2.c, cmdedit, bos41J, 9508A 2/14/95 16:31:24";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_vops2.c
 *
 * FUNCTIONS: ateopr, back1, bleep, takeout, vappend, vdcMID, vdoappend,
 * vgetline, vgetsplit, vmaxrep
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
#include "ex_tty.h"
#include "ex_vis.h"

int map(wchar_t, struct maps *, int *);

static int vgetsplit(void);
static int vmaxrep(wchar_t, int);

int sigsetmask(int);
int sigblock(int);

/*
 * Low level routines for operations sequences,
 * and mostly, insert mode (and a subroutine
 * to read an input line, including in the echo area.)
 */
extern wchar_t	*vUA1, *vUA2;		/* mjm: extern; also in ex_vops.c */
extern wchar_t	*vUD1, *vUD2;		/* mjm: extern; also in ex_vops.c */
static wchar_t nl[2] = { '\n', 0 };	/* "\n"  as an wchar_t string */

static wchar_t sp[2] = { ' ', 0 };		/* " "  as an wchar_t string */
/*
 * Obleeperate characters in hardcopy
 * open with \'s.
 */
void bleep(int i, wchar_t *cp)
{

	i -= column(cp);
	do {
		ex_putchar('\\');
	} while (--i >= 0);
	rubble = 1;
}

/*
 * Common code for middle part of delete
 * and change operating on parts of lines.
 */
int vdcMID(void)
{
	register wchar_t *cp;

	squish();
	setLAST();
	if (FIXUNDO)
		vundkind = VCHNG, CP(vutmp, linebuf);
	if (wcursor < cursor)
		cp = wcursor, wcursor = cursor, cursor = cp;
	vUD1 = vUA1 = vUA2 = cursor; vUD2 = wcursor;
	return (column(wcursor - 1));
}

/*
 * Take text from linebuf and stick it
 * in the VBSIZE buffer BUF.  Used to save
 * deleted text of part of line.
 */
void takeout(wchar_t *BUF, short *overbuf)
{
	register wchar_t *cp;

	if (wcursor < linebuf)
		wcursor = linebuf;
	if (cursor == wcursor) {
		ex_beep();
		return;
	}
	if (wcursor < cursor) {
		cp = wcursor;
		wcursor = cursor;
		cursor = cp;
	}
	setBUF(BUF,overbuf);
	if (*overbuf == TRUE)
		ex_beep();
}

/*
 * Are we at the end of the printed representation of the
 * line?  Used internally in hardcopy open.
 */
int ateopr(void)
{
	register int i;
	register wchar_t c, *cp = vtube[destline] + destcol;

	for (i = WCOLS - destcol; i > 0; i--) {
		c = *cp++;
		if (c == 0) {
			/*
			 * Optimization to consider returning early, saving
			 * CPU time.  We have to make a special check that
			 * we aren't missing a mode indicator.
			 */
			if (destline == WECHO && destcol < WCOLS-11 && vtube[WECHO][WCOLS-20])
				return 0;
			return (1);
		}
		/* if (c != ' ' && (c & QUOTE) == 0) return (0); */
		if (c != ' ' && c != QUOTE_NUL && c != QUOTE_SP)
			return (0);
	}
	return (1);
}

/*
 * Append.
 *
 * This routine handles the top level append, doing work
 * as each new line comes in, and arranging repeatability.
 * It also handles append with repeat counts, and calculation
 * of autoindents for new lines.
 */
static short	vaifirst;
static short	gobbled;
static wchar_t	*ogcursor;

void vappend(int ch, int cnt, int indent)
{
	register int i;
	register wchar_t *gcursor;
	short escape;
	int repcnt, savedoomed;
	short oldhold = hold;
#if defined(MYSIG)
	int oldmask;
#endif

	/*
	 * Before a move in hardopen when the line is dirty
	 * or we are in the middle of the printed representation,
	 * we retype the line to the left of the cursor so the
	 * insert looks clean.
	 */
	if (ch != 'o' && state == HARDOPEN && (rubble || !ateopr())) {
		rubble = 1;
		gcursor = cursor;
		i = *gcursor;
		*gcursor = ' ';
		wcursor = gcursor;
		vmove();
		*gcursor = i;
	}
	vaifirst = indent == 0;

	/*
	 * Handle replace character by (eventually)
	 * limiting the number of input characters allowed
	 * in the vgetline routine.
	 */
	if (ch == 'r')
		repcnt = 2;
	else
		repcnt = 0;

	/*
	 * If an autoindent is specified, then
	 * generate a mixture of blanks to tabs to implement
	 * it and place the cursor after the indent.
	 * Text read by the vgetline routine will be placed in genbuf,
	 * so the indent is generated there.
	 */
	if (value(AUTOINDENT) && indent != 0) {
		gcursor = genindent(indent);
		*gcursor = 0;
		vgotoCL(qcolumn(cursor - 1, genbuf));
	} else {
		gcursor = genbuf;
		*gcursor = 0;
		if (ch == 'o')
			vfixcurs();
	}

	/*
	 * Prepare for undo.  Pointers delimit inserted portion of line.
	 */
	vUA1 = vUA2 = cursor;

	/*
	 * If we are not in a repeated command and a ^@ comes in
	 * then this means the previous inserted text.
	 * If there is none or it was too long to be saved,
	 * then beep() and also arrange to undo any damage done
	 * so far (e.g. if we are a change.)
	 */
	switch (ch) {
	case 'r':
		break;
	case 'R':
		vshowmode(value(TERSE) ? MSGSTR(M_280, "R") : MSGSTR(M_242, "REPLACE MODE"));
		break;
	default:
		vshowmode(value(TERSE) ? MSGSTR(M_281, "I") : MSGSTR(M_243, "INPUT MODE"));
	}
	if ((vglobp && *vglobp == 0) || peekbr()) {
		if (INS_OVFLOW == TRUE) {
			ex_beep();
			if (!splitw)
				ungetkey('u');
			doomed = 0;
			hold = oldhold;
			return;
		}
		/*
		 * Unread input from INS.
		 * An escape will be generated at end of string.
		 * Hold off n^^2 type update on dumb terminals.
		 */
		vglobp = INS;
		hold |= HOLDQIK;
	} else if (vglobp == 0) {
		/*
		 * Not a repeated command, get
		 * a new inserted text for repeat.
		 */
		INS[0] = 0;
		INS_OVFLOW = FALSE;
	}

	/*
	 * For wrapmargin to remove second space after a '.'
	 * when the first space caused a line break we keep
	 * track that this happened in gobblebl, which says
	 * to gobble up a blank silently.
	 */
	gobblebl = 0;

	/*
	 * Text gathering loop.
	 * New text goes into genbuf starting at gcursor.
	 * cursor preserves place in linebuf where text will eventually go.
	 */
	if (*cursor == 0 || state == CRTOPEN)
		hold |= HOLDROL;
#if defined(MYSIG)
	oldmask = sigblock(sigmask(SIGWINCH));
#endif
	for (;;) {
		if (ch == 'r' && repcnt == 0)
			escape = TRUE;
		else {
			static wchar_t nl_arrow[] = { '^', 0 };
			static wchar_t nl_zero[] = { '0', 0 };
			static wchar_t nl_ctld[] = { QUOTE_CTRL_D, 0 };
			static wchar_t nl_blank[] = { ' ', 0 };

			gcursor = vgetline(repcnt, gcursor, &escape, 
                                           (wchar_t)ch);

			/*
			 * After an append, stick information
			 * about the ^D's and ^^D's and 0^D's in
			 * the repeated text buffer so repeated
			 * inserts of stuff indented with ^D as backtab's
			 * can work.
			 */
			if (HADUP)
				addtext(nl_arrow);
			else if (HADZERO)
				addtext(nl_zero);
			for (; CDCNT > 0; CDCNT--)
				addtext(nl_ctld);
			if (gobbled)
				addtext(nl_blank);
			addtext(ogcursor);
		}
		repcnt = 0;

		/*
		 * Smash the generated and preexisting indents together
		 * and generate one cleanly made out of tabs and spaces
		 * if we are using autoindent.
		 */
		if (!vaifirst && value(AUTOINDENT)) {
			i = fixindent(indent);
			if (!HADUP)
				indent = i;
			gcursor = WCstrend(genbuf);
		}

		/*
		 * Limit the repetition count based on maximum
		 * possible line length; do output implied
		 * by further count (> 1) and cons up the new line
		 * in linebuf.
		 */
		cnt = vmaxrep((wchar_t)ch, cnt);
		CP(gcursor + 1, cursor);
		do {
			CP(cursor, genbuf);
			if (cnt > 1) {
				int oldhld = hold;

				Outchar = vinschar;
				hold |= HOLDQIK;
				ex_printf("%S", genbuf);
				hold = oldhld;
				Outchar = (void (*)(int))vputchar;
			}
			cursor += gcursor - genbuf;
		} while (--cnt > 0);
		endim();
		vUA2 = cursor;
		if (escape)
			CP(cursor, gcursor + 1);

		/*
		 * If doomed characters remain, clobber them,
		 * and reopen the line to get the display exact.
		 */
		if (state != HARDOPEN) {
			DEPTH(vcline) = 0;
			savedoomed = doomed;
			if (doomed > 0) {
				register int cind = cindent();

				physdc(cind, cind + doomed);
				doomed = 0;
			}
			i = vreopen(LINE(vcline), lineDOT(), vcline);
#ifdef TRACE
			if (trace)
				fprintf(trace, "restoring doomed from %d to %d\n", doomed, savedoomed);
#endif
			if (ch == 'R')
				doomed = savedoomed;
		}

		/*
		 * All done unless we are continuing on to another line.
		 */
		if (escape) {
			vshowmode("");
			break;
		}

		/*
		 * Set up for the new line.
		 * First save the current line, then construct a new
		 * first image for the continuation line consisting
		 * of any new autoindent plus the pushed ahead text.
		 */
		killU();
		{
			static wchar_t nl_blank[] = { ' ', 0 };
			static wchar_t nl_newline[] = { '\n', 0 };
			addtext(gobblebl ? nl_blank : nl_newline);
		}
		vsave();
		cnt = 1;
		if (value(AUTOINDENT)) {
			if (value(LISP))
				indent = lindent(dot + 1);
			else
			     if (!HADUP && vaifirst)
				indent = whitecnt(linebuf);
			vaifirst = 0;
			strcLIN(vpastwh(gcursor + 1));
			gcursor = genindent(indent);
			*gcursor = 0;
			if (gcursor + wcslen(linebuf) > &genbuf[LBSIZE - 2])
				gcursor = genbuf;
			CP(gcursor, linebuf);
		} else {
			CP(genbuf, gcursor + 1);
			gcursor = genbuf;
		}

		/*
		 * If we started out as a single line operation and are now
		 * turning into a multi-line change, then we had better yank
		 * out dot before it changes so that undo will work
		 * correctly later.
		 */
		if (FIXUNDO && vundkind == VCHNG) {
			vremote(1, (void(*)(int))yank, 0);
			undap1--;
		}

		/*
		 * Now do the append of the new line in the buffer,
		 * and update the display.  If slowopen
		 * we don't do very much.
		 */
		vdoappend(genbuf);
		vundkind = VMANYINS;
		vcline++;
		if (state != VISUAL)
			vshow(dot, NOLINE);
		else {
			i += LINE(vcline - 1);
			vopen(dot, i);
			if (value(SLOWOPEN))
				vscrap();
			else
				vsync1(LINE(vcline));
		}
		vlastshowp=NULL;
		vshowmode(value(TERSE) ? MSGSTR(M_281, "I") : MSGSTR(M_243, "INPUT MODE"));
		strcLIN(gcursor);
		*gcursor = 0;
		cursor = linebuf;
		vgotoCL(qcolumn(cursor - 1, genbuf));
	}
#if defined(MYSIG)
	(void) sigsetmask(oldmask);
#endif

	/*
	 * All done with insertion, position the cursor
	 * and sync the screen.
	 */
	hold = oldhold;
	if (cursor > linebuf)
		cursor--;
	if (state != HARDOPEN)
		vsyncCL();
	else if (cursor > linebuf)
		back1();
	doomed = 0;
	wcursor = cursor;
	vmove();
}

/*
 * Subroutine for vgetline to back up a single character position,
 * backwards around end of lines (vgoto can't deal with columns which are
 * less than 0 in general).
 */
int back1(void)
{

	vgoto(destline - 1, WCOLS + destcol - 1);
}

/*
 * Get a line into genbuf after gcursor.
 * Cnt limits the number of input characters
 * accepted and is used for handling the replace
 * single character command.  Aescaped is the location
 * where we stick a termination indicator (whether we
 * ended with an ESCAPE or a newline/return.
 *
 * We do erase-kill type processing here and also
 * are careful about the way we do this so that it is
 * repeatable.  (I.e. so that your kill doesn't happen,
 * when you repeat an insert if it was escaped with \ the
 * first time you did it.  commch is the command character
 * involved, including the prompt for readline.
 */
wchar_t *
vgetline(int cnt, wchar_t *gcursor, short *aescaped, wchar_t commch)
{
	register int c, ch;
	register wchar_t *cp;
	int x, y, iwhite, backsl=0, len;
	wchar_t *iglobp;
	wchar_t cstr[2];
	wchar_t *savecp;	/* for wrapping */
	/* length of closing punctuation, an ellipsis (...) is 3 */
 	int cplen, dispcol;
	wchar_t tmpc;
	char in_abv = 0;	/* inside the abbreviation? */
	void (*OO)(int) = Outchar;
	int	fmap=0;

	/*
	 * Clear the output state and counters
	 * for autoindent backwards motion (counts of ^D, etc.)
	 * Remember how much white space at beginning of line so
	 * as not to allow backspace over autoindent.
	 */
	*aescaped = TRUE;
	ogcursor = gcursor;
	flusho();
	CDCNT = 0;
	HADUP = 0;
	HADZERO = 0;
	gobbled = 0;
	iwhite = whitecnt(genbuf);
	iglobp = vglobp;

	/*
	 * Carefully avoid using vinschar in the echo area.
	 */
	if (splitw)
		Outchar = (void (*)(int))vputchar;
	else {
		Outchar = vinschar;
		vprepins();
	}
	for (;;) {
		struct maps *mapstr;
		backsl = 0;
		if (gobblebl)
			gobblebl--;
		if (cnt != 0) {
			cnt--;
			if (cnt == 0)
				goto vadone;
		}
		c = getkey();
		ch = c;
		maphopcnt = 0;
		if (c != ATTN && vglobp == 0 && Peekkey == 0 && commch != 'r') {
			if (commch == 'R')
				if (cursor == linebuf || cursor[1] == 0)
					mapstr = rmmacs1;
				else
					mapstr = rmmacs;
			else
				mapstr = immacs;

			while (((ch = map((wchar_t)c, mapstr, &fmap)) != c)||(fmap)) {
				fmap = 0;
				c = ch;
				if (!value(REMAP))
					break;
				if (++maphopcnt > 256)
					error(MSGSTR(M_245, "Infinite macro loop"), DUMMY_INT);
			}
		}
		if (!iglobp) {

			/*
			 * Erase-kill type processing.
			 * Only happens if we were not reading
			 * from untyped input when we started.
			 * Map users erase to ^H, kill to -1 for switch.
			 */
#ifndef USG
			if (c == tty.sg_erase)
				c = Ctrl('H');
			else if (c == tty.sg_kill)
				c = -1;
#else
			if (c == tty.c_cc[VERASE])
				c = Ctrl('H');
			else if (c == tty.c_cc[VKILL])
				c = -1;
#endif
			switch (c) {

			/*
			 * ^?		Interrupt drops you back to visual
			 *		command mode with an unread interrupt
			 *		still in the input buffer.
			 *
			 * ^\		Quit does the same as interrupt.
			 *		If you are a ex command rather than
			 *		a vi command this will drop you
			 *		back to command mode for sure.
			 */
			case ATTN:
			case QUIT:
		        case Ctrl('Z'):
				ungetkey(c);
				goto vadone;

			/*
			 * ^H		Backs up a character in the input.
			 *
			 * BUG:		Can't back around line boundaries.
			 *		This is hard because stuff has
			 *		already been saved for repeat.
			 */
			case Ctrl('H'):
bakchar:
				cp = gcursor - 1;
				if (cp < ogcursor) {
					if (splitw) {
						/*
						 * Backspacing over readecho
						 * prompt. Pretend delete but
						 * don't beep.
						 */
						ungetkey(c);
						goto vadone;
					}
					ex_beep();
					continue;
				}
				goto vbackup;

			/*
			 * ^W		Back up a white/non-white word.
			 */
			case Ctrl('W'):
				wdkind = 1;
				for (cp = gcursor; cp > ogcursor && iswspace(cp[-1]); cp--)
					continue;
				for (c = wordch(cp - 1);
				    cp > ogcursor && wordof((wchar_t)c, cp - 1); cp--)
					continue;
				goto vbackup;

			/*
			 * users kill	Kill input on this line, back to
			 *		the autoindent.
			 */
			case -1:
				cp = ogcursor;
vbackup:
				if (cp == gcursor) {
					ex_beep();
					continue;
				}
				endim();
				*cp = 0;
				c = cindent();
				vgotoCL(qcolumn(cursor - 1, genbuf));
				if (doomed >= 0)
					doomed += c - cindent();
				gcursor = cp;
				continue;

			/*
			 * \		Followed by erase or kill
			 *		maps to just the erase or kill.
			 */
			case '\\':
				x = destcol, y = destline;
				ex_putchar('\\');
				vcsync();
				c = getkey();
#ifndef USG
				if (c == tty.sg_erase
				    || c == tty.sg_kill)
#else
				if (c == tty.c_cc[VERASE]
				    || c == tty.c_cc[VKILL])
#endif
				{
					vgoto(y, x);
					if (doomed >= 0)
						doomed++;
					goto def;
				}
				ungetkey(c), c = '\\';
				backsl = 1;
				break;

			/*
			 * ^Q		Super quote following character
			 *		Only ^@ is verboten (trapped at
			 *		a lower level) and \n forces a line
			 *		split so doesn't really go in.
			 *
			 * ^V		Synonym for ^Q
			 */
			case Ctrl('Q'):
			case Ctrl('V'):
				x = destcol, y = destline;
				ex_putchar('^');
				vgoto(y, x);
				c = getkey();
				if (c != '\n') {
#ifdef USG
				if (c == ATTN)
					c = tty.c_cc[VINTR];
#else
#ifdef TIOCSETC
				if (c == ATTN)
					c = nttyc.t_intrc;
#endif
#endif
					if (doomed >= 0)
						doomed++;
					goto def;
				}
				break;
			}
		}

/*
 * When vi splits a line for wrapping, any white space directly preceding 
 * the wrapmargin is deleted.
 *
 * wraptype determines what vi does at the wrapmargin.
 * By default, wraptype=g(eneral) and wrapmargin=0 (no wrapping).
 * Note that closepunct affects wrapping when wt is general, flexible or rigid.
 *
 *	general:
 *		A mixture of "word" and "flexible" wrap, general purpose wrap
 *		attempts to satisfy Japanese and English rules for wrapping.
 *		A line is wrapped on a "word break."  A word break is either
 *		1) white space, or 2) between two Japanese characters.  See
 *		"word" and "flexible" descriptions.  Here, Japanese characters
 *		are defined as any nonASCII character.
 *		( characters are wide and hence isascii is not a valid test 
 *		  iswgraph is used to test for ascii chars -- depends on the 
 *		  definition in the other locales. This may require some changes)
 *
 *	word:
 *		Same as original vi.  A newline is inserted to split the line.
 *		A word crossing the wrapmargin is moved to the next line, and
 *		white space preceding the wrapmargin is deleted.
 *
 *
 *	rigid:
 *		The character crossing the wrapmargin is wrapped to the next line.
 *		Closing punctuation is treated as one character when wrapping.
 *		Both closing punctuation and the preceding character are wrapped,
 *		unless the preceding character forms closing punctuation.
 *
 *	flexible:
 *		flexible wrap is similar to rigid column wrap but allows 1 character of
 *	  closing punctuation to extend past the wrapmargin.  With flexible wrap,
 *		1 character or 1 closing punctuation is wrapped.
 */
		/*
		 * If we get a blank not in the echo area
		 * consider splitting the window in the wrapmargin.
		 */
		if (c != '\n' && !splitw) {
			if (c == ' ' && gobblebl) {
				gobbled = 1;
				continue;
			}
			if (value(WRAPMARGIN) &&
				(outcol + printwid((wchar_t)c,outcol) > OCOLUMNS - value(WRAPMARGIN) ||
				 backsl && outcol==0) &&
				commch != 'r') {
				/*
				 * At end of word and hit wrapmargin.
				 * Move the word to next line and keep going.
				 */

				/* check for ESCAPE */
				if (c == ESCAPE)			
					goto dontbreak;

				 /*
				  * general and flexible column wrap permits 1 closing
				  * punctuation past the wrap margin
				 */
				if (svalue(WRAPTYPE)[0] == 'g' || svalue(WRAPTYPE)[0] == 'f') { 
					static npastwm = 0;	/* characters past wrapmargin */

					tmpc = *gcursor;				/*! needed? !*/
					*gcursor = c;
					if (npastwm == 0 && anycp(ogcursor,gcursor)) {
						npastwm++;
						*gcursor = tmpc;
						ex_beep();
						goto dontbreak;
					}
					npastwm = 0;	
				}
				wdkind = 1;
				*gcursor++ = c;
				if (backsl)
					*gcursor++ = getkey();
				*gcursor = 0;
  				if ((len = wcwidth(*(gcursor -1))) < 0)
					len = 1;
  				dispcol = outcol + len;

				/*
				 * Find end of previous word if we are past it.
				 */

				/* if only white space before wrap margin, remove it */
  				for (cp=gcursor; cp>ogcursor && iswspace(cp[-1]); cp--){
					if ((len = wcwidth(cp[-1])) < 0)
						len = 1;
  					dispcol -= len;
  				}

				/*
				 * Find beginning of previous word.
				 */

  				if (dispcol+(backsl?OCOLUMNS:0) > (OCOLUMNS-value(WRAPMARGIN))){
					switch (svalue(WRAPTYPE)[0]) {
					default:  /* should never happen */
					case 'w': /* word wrap - find beginning of previous word */
							for (; cp>ogcursor && !iswspace(cp[-1]); cp--)
									;
							break;
							
					case 'g': /* general purpose wrap */
						/*
						 * general purpose wrap:
						 *	split line after white space (word-like)
						 *	or move 1 or 2 characters to the next line (flexible-like)
						 */
						 	for (savecp = cp; cp > ogcursor ; cp--)
								if (!iswgraph(*cp)) {
									cp = savecp;
									goto flexcase ;
								} else if (iswspace(cp[-1]))
									break;
						 	break;

					case 'f': /* flexible column wrap */
					case 'r': /* rigid column wrap */
					/*
					 * wrapping rules:
					 *	wrap 1 character that is not closing punctuation
					 *	wrap 1 closing punctuation and preceding regular character
					 *	wrap 1 closing punctuation if preceded by closing punctuation
					 */
flexcase:
							cp--;	/* wrap a character */
							if ((cplen = anycp(ogcursor,cp)) && cp - cplen > ogcursor) {
								if (anycp(ogcursor,(cp - cplen)) == 0)
									/* wrap cpunct and previuos character */
									cp -= cplen;
								else if (cplen > 1)
									/* wrap cpunct without previous characeter */
									cp = cp - cplen - 1;
							}
							break;
					}
					/* don't wrap whole lines, nor from beginning of insert */

					if (cp <= ogcursor) {
						/*
						 * There is a single word that
						 * is too long to fit.  Just
						 * let it pass, but beep for
						 * each new letter to warn
						 * the luser.
						 */
						c = *--gcursor;
						*gcursor = 0;
						ex_beep();
						goto dontbreak;
					}
					/*
					 * Save it for next line.
					 */
					macpush(cp, 0);
					/*
					 * move to space before the word.
					 * After "goto vbackup," a '\0' replaces the space
					 */
					if (svalue(WRAPTYPE)[0] == 'w' || iswspace(cp[-1]))
						cp--;
					/*
					 * a leading space gets gobbled so,
					 * put an extra leading space
					 * before closed punctuation when
					 * wrapping a line.
					 */
					/*! misses multi-char punctuation !*/
					else if (iswspace(*cp) && anycp(ogcursor,cp+1))
						macpush(sp, 0);	/* preserve space */
				}
				macpush(nl, 0);
				/*
				 * Erase white space before the word.
				 */
				while (cp > ogcursor && iswspace(cp[-1]))
					cp--;	/* skip blank */
				gobblebl = 3;
				goto vbackup;
			}
		dontbreak:;
		}

		/*
		 * Word abbreviation mode.
		 */
		cstr[0] = c;
		if (in_abv && !(vmacp))	/* Was in abbreviation and now    */
			in_abv = 0;	/* consumed everything from macro */
					/* buffer, so not in abv anymore  */

		if ( !(in_abv) && anyabbrs && gcursor > ogcursor && !wordch(cstr) && wordch(gcursor-1)) {
				int wdtype, abno;

				in_abv = 1;
				cstr[1] = 0;
				wdkind = 1;
				cp = gcursor - 1;
				for (wdtype = wordch(cp - 1);
				    cp > ogcursor && wordof((wchar_t)wdtype, cp - 1); cp--)
					;
				*gcursor = 0;
				for (abno=0; abbrevs[abno].mapto; abno++) {
					if (WCeq(cp, abbrevs[abno].cap)) {
						macpush(cstr, 0);
						macpush(abbrevs[abno].mapto, 0);
						goto vbackup;
					}
				}
		}

		switch (c) {

		/*
		 * ^M		Except in repeat maps to \n.
		 */
		case '\r':
			if (vglobp)
				goto def;
			c = '\n';
			/* presto chango ... */

		/*
		 * \n		Start new line.
		 */
		case '\n':
			*aescaped = FALSE;
			goto vadone;

		/*
		 * escape	End insert unless repeat and more to repeat.
		 */
		case ESCAPE:
			if (lastvgk)
				goto def;
			goto vadone;

		/*
		 * ^D		Backtab.
		 * ^T		Software forward tab.
		 *
		 *		Unless in repeat where this means these
		 *		were superquoted in.
		 */
		case Ctrl('D'):
		case Ctrl('T'):
			if (vglobp)
				goto def;
			/* fall into ... */

		/*
		 * QUOTE_CTRL_D	Is a backtab (in a repeated command).
		 */
		case QUOTE_CTRL_D:
			*gcursor = 0;
			cp = vpastwh(genbuf);
			c = whitecnt(genbuf);
			if (ch == Ctrl('T')) {
				/*
				 * ^t just generates new indent replacing
				 * current white space rounded up to soft
				 * tab stop increment.
				 */
				if (cp != gcursor)
					/*
					 * BUG:		Don't use ^T except
					 *		right after initial
					 *		white space.
					 */
					continue;
				cp = genindent(iwhite = backtab(c + value(SHIFTWIDTH) + 1));
				ogcursor = cp;
				goto vbackup;
			}
			/*
			 * ^D works only if we are at the (end of) the
			 * generated autoindent.  We count the ^D for repeat
			 * purposes.
			 */
			if (c == iwhite && c != 0)
				if (cp == gcursor) {
					iwhite = backtab(c);
					CDCNT++;
					ogcursor = cp = genindent(iwhite);
					goto vbackup;
				} else if (&cp[1] == gcursor &&
				    (*cp == '^' || *cp == '0')) {
					/*
					 * ^^D moves to margin, then back
					 * to current indent on next line.
					 *
					 * 0^D moves to margin and then
					 * stays there.
					 */
					HADZERO = *cp == '0';
					ogcursor = cp = genbuf;
					HADUP = 1 - HADZERO;
					CDCNT = 1;
					endim();
					back1();
					vputchar((wchar_t)' ');
					goto vbackup;
				}
			if (vglobp && vglobp - iglobp >= 2 &&
			    (vglobp[-2] == '^' || vglobp[-2] == '0')
			    && gcursor == ogcursor + 1)
				goto bakchar;
			continue;

		default:
			/*
			 * Possibly discard control inputs.
			 */
			if (!vglobp && junk(c)) {
				ex_beep();
				continue;
			}
def:
			if (!backsl) {
				ex_putchar(c);
				flush();
			}
			if (gcursor > &genbuf[LBSIZE])
				error(MSGSTR(M_246, "Line too long"), DUMMY_INT);
			/* *gcursor++ = c & TRIM; */
			*gcursor++ = (c == QUOTE_NUL ? '\0' : c == QUOTE_SP ? ' ' : c);
			vcsync();
			if (value(SHOWMATCH) && !iglobp)
				if (c == ')' || c == '}')
					lsmatch(gcursor);
			continue;
		}
	}
vadone:
	*gcursor = 0;
	if (Outchar != termchar)
		Outchar = OO;
	endim();
	return (gcursor);
}

static wchar_t	*vsplitpt;

/*
 * Append the line in buffer at lp
 * to the buffer after dot.
 */
void vdoappend(wchar_t *lp)
{
	register int oing = inglobal;

	vsplitpt = lp;
	inglobal = 1;
	ignore(append(vgetsplit, dot));
	inglobal = oing;
}

/*
 * Subroutine for vdoappend to pass to append.
 */
static int vgetsplit(void)
{

	if (vsplitpt == 0)
		return (EOF);
	strcLIN(vsplitpt);
	vsplitpt = 0;
	return (0);
}

/*
 * Vmaxrep determines the maximum repetitition factor
 * allowed that will yield total line length less than
 * LBSIZE characters and also does things for the R command.
 */
static int vmaxrep(wchar_t ch, int cnt)
{
	register int len, replen;

	if (cnt > LBSIZE - 2)
		cnt = LBSIZE - 2;
	replen = wcslen(genbuf);
	if (ch == 'R') {
		len = wcslen(cursor);
		if (replen < len)
			len = replen;
		CP(cursor, cursor + len);
		vUD2 += len;
	}
	len = wcslen(linebuf);
	if (len + cnt * replen <= LBSIZE - 2)
		return (cnt);
	cnt = (LBSIZE - 1 - len) / replen;
	if (cnt == 0) {
		vsave();
		error(MSGSTR(M_246, "Line too long"), DUMMY_INT);
	}
	return (cnt);
}
