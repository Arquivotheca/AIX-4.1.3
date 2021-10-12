#if !defined(lint)
static char sccsid[] = "@(#)97	1.21  src/bos/usr/bin/ex/ex_vget.c, cmdedit, bos41J, 9508A 2/21/95 16:49:05";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_vget.c
 *
 * FUNCTION: addtext, addto, cancelalarm, ex_beep, fastpeekkey, getbr, getesc,
 * getkey, macpush, map, noteit, peekbr, peekkey, readecho, setBUF, setDEL,
 * setLAST, setalarm, trapalarm, ungetkey, vgetcnt, visdump, vudump
 *
 * ORIGINS: 3, 10, 13, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ex_vget.c  1.7  com/cmd/edit/vi,3.1,9013 3/2/90 12:46:30
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */
/* ex_vget.c Revision: 2.6.1.2  (OSF) Date: 90/10/08 23:33:53  */

#include <sys/time.h>
#include "ex.h"
#include "ex_tty.h"
#include "ex_vis.h"

int map(wchar_t, struct maps *, int *);

static int fastpeekkey(void);

static int getbr(void);
static void addto(wchar_t *, short *, wchar_t *);
static void setalarm(void);
static void trapalarm(void);

/*
 * Input routines for open/visual.
 * We handle upper case only terminals in visual and reading from the
 * echo area here as well as notification on large changes
 * which appears in the echo area.
 */

/*
 * Return the key.
 */
void ungetkey(int c)
{

	if (Peekkey != ATTN)
		Peekkey = c;
}

/*
 * Return a keystroke, but never a ^@.
 */
int getkey(void)
{
	register int c;

	do {
		c = getbr();
		if (c==0)
			ex_beep();
	} while (c == 0);
	return (c);
}

/*
 * Tell whether next keystroke would be a ^@.
 */
int peekbr(void)
{

	Peekkey = getbr();
	return (Peekkey == 0);
}

static short	precbksl;
static jmp_buf readbuf;
static int 	doingread = 0;

/*
 * Get a keystroke, including a ^@.
 * If an key was returned with ungetkey, that
 * comes back first.  Next comes unread input (e.g.
 * from repeating commands with .), and finally new
 * keystrokes.
 *
 * The hard work here is in mapping of \ escaped
 * characters on upper case only terminals.
 */
static int getbr(void)
{
	unsigned char ch;
	register int c, d;
	static char Peek2key;
	char inbuf[MB_LEN_MAX + 1];
	wchar_t widc;
	int count, char_ok;
	extern short slevel, ttyindes;

getATTN:
	if (Peekkey) {
		c = Peekkey;
		Peekkey = 0;
		return (c);
	}
	if (Peek2key) {
		c = Peek2key;
		Peek2key = 0;
		return (c);
	}
	if (vmacp) {
		if (*vmacp)
			return(*vmacp++);
		/* End of a macro or set of nested macros */
		vmacp = 0;

		if ( vflickp != NULL )	/* Mode change occurred */
		{
			if ( vflickp != vlastshowp ) /* Mode not same now */
				vshowmode(vflickp);  /* Update the screen */
			vflickp = NULL;
		}

		if (inopen == -1)	/* don't screw up undo for esc esc */
			vundkind = VMANY;
		inopen = 1;	/* restore old setting now that macro done */
		vch_mac = VC_NOTINMAC;
	}
	if (vglobp) {
		if (*vglobp)
			return (lastvgk = *vglobp++);
		lastvgk = 0;
		return (ESCAPE);
	}
	flusho();
again:
	if (setjmp(readbuf))
		goto getATTN;
	doingread = 1;
	for (count = 0, char_ok = 0; count < MB_CUR_MAX; count++)
	{
	
		int ret ;
		if((ret = read(slevel == 0 ? 0 : ttyindes, &inbuf[count], 1)) != 1)
		{
			if (ret == -1 && errno == EINTR)
				goto getATTN;
			error(MSGSTR(M_236, "ex: An error occurred while reading input."), DUMMY_INT);
			break;
		}
		if((mbtowc(&widc, inbuf, count + 1)) == count + 1)
		{
			c = widc;
			char_ok = 1;	/* prevent error on exit of loop */
			break;
		}
 		if(( count == 0) && (inbuf[count] == 0)){
 			c = 0;
 			char_ok = 1;	/* pass the null on it is handled later */
 			break;
 		}
	}

 		if(char_ok != 1){
 			ex_beep();
			error(MSGSTR(M_652, "ex: Incomplete or invalid multibyte character encountered, conversion failed."), DUMMY_INT);
		}

	if (beehive_glitch && slevel==0 && c == ESCAPE) {
		if (read(0, &Peek2key, 1) != 1)
			goto getATTN;
		switch (Peek2key) {
		case 'C':	/* SPOW mode sometimes sends \EC for space */
			c = ' ';
			Peek2key = 0;
			break;
		case 'q':	/* f2 -> ^C */
			c = Ctrl('C');
			Peek2key = 0;
			break;
		case 'p':	/* f1 -> esc */
			Peek2key = 0;
			break;
		}
	}

	/*
	 * The algorithm here is that of the UNIX kernel.
	 * See the description in the programmers manual.
	 */
	if (UPPERCASE) {
		if (iswupper(c))
			c = towlower(c);
		if (c == '\\') {
			if (precbksl < 2)
				precbksl++;
			if (precbksl == 1)
				goto again;
		} else if (precbksl) {
			d = 0;
			if (iswlower(c))
				d = towupper(c);
			else {
				register char *colp;
				colp = "({)}!|^~'~";
				while (d = *colp++)
					if (d == c) {
						d = *colp++;
						break;
					} else
						colp++;
			}
			if (precbksl == 2) {
				if (!d) {
					Peekkey = c;
					precbksl = 0;
					c = '\\';
				}
			} else if (d)
				c = d;
			else {
				Peekkey = c;
				precbksl = 0;
				c = '\\';
			}
		}
		if (c != '\\')
			precbksl = 0;
	}
#ifdef TRACE
	if (trace) {
		if (!techoin) {
			tfixnl();
			techoin = 1;
			fprintf(trace, "*** Input: ");
		}
		tracec(c);
	}
#endif
	lastvgk = 0;
	return (c);
}

/*
 * Get a key, but if a delete, quit or attention
 * is typed return 0 so we will abort a partial command.
 */
int getesc(void)
{
	register int c;			/* code point or EOF */

	c = getkey();
	switch (c) {

	case Ctrl('V'):
	case Ctrl('Q'):
		c = getkey();
		return (c);

	case ATTN:
	case QUIT:
		ungetkey(c);
		return (0);

	case ESCAPE:
		return (0);
	}
	return (c);
}

/*
 * Peek at the next keystroke.
 */
int peekkey(void)
{

	Peekkey = getkey();
	return (Peekkey);
}

/*
 * Read a line from the echo area, with single character prompt c.
 * A return value of 1 means the user blewit or blewit away.
 */
int readecho(wchar_t c)
{
	register wchar_t *sc = cursor;
	register void (*OP)(int);
	short waste;
	register int OPeek;

	if (WBOT == WECHO)
		vclean();
	else
		vclrech((short)0);
	splitw++;
	vgoto(WECHO, 0);
	ex_putchar(c);
	vclreol();
	vgoto(WECHO, 1);
	cursor = linebuf; linebuf[0] = 0; genbuf[0] = c;
	if (peekbr()) {
		if (!INS[0] || INS_OVFLOW)
			goto blewit;
		vglobp = INS;
	}
	OP = Pline; Pline = (void (*)(int))normline;
	ignore(vgetline(0, genbuf + 1, &waste, c));
	if (Outchar == termchar && state != VISUAL)
		ex_putchar('\n');
	vscrap();
	Pline = OP;
	if (Peekkey != ATTN && Peekkey != Ctrl('H')) {
		cursor = sc;
		vclreol();
		return (0);
	}
blewit:
	OPeek = Peekkey==Ctrl('H') ? 0 : Peekkey; Peekkey = 0;
	splitw = 0;
	vclean();
	vshow(dot, NOLINE);
	vnline(sc);
	Peekkey = OPeek;
	return (1);
}

/*
 * A complete command has been defined for
 * the purposes of repeat, so copy it from
 * the working to the previous command buffer.
 */
void setLAST(void)
{

	if (vglobp || vmacp)
		return;
	lastreg = vreg;
	lasthad = Xhadcnt;
	lastcnt = Xcnt;
	*lastcp = 0;
	CP(lastcmd, workcmd);
}

/*
 * Gather up some more text from an insert.
 * If the insertion buffer oveflows, then destroy
 * the repeatability of the insert.
 */
void addtext(wchar_t *cp)
{

	if (vglobp)
		return;
	addto(INS, &INS_OVFLOW, cp);
	if (INS_OVFLOW)
		lastcmd[0] = 0;
}

void setDEL(void)
{

	setBUF(DEL,&DEL_OVFLOW);
}

/*
 * Put text from cursor upto wcursor in BUF.
 */
void setBUF(wchar_t *BUF, short *overflow)
/* overflow:		 buffer overflow */
{
	register wchar_t c;
	register wchar_t *wp = wcursor;

	c = *wp;
	*wp = 0;
	BUF[0] = 0;
	*overflow = FALSE;
	addto(BUF, overflow, cursor);
	*wp = c;
}

static void addto(wchar_t *buf, short *overflow, wchar_t *str)
{

	if (*overflow)
		return;
	if (wcslen(buf) + wcslen(str) + 1 >= VBSIZE) {
		*overflow = TRUE;
		return;
	}
	ignore(wcscat(buf, str));
}

/*
 * Note a change affecting a lot of lines, or non-visible
 * lines.  If the parameter must is set, then we only want
 * to do this for open modes now; return and save for later
 * notification in visual.
 */
int noteit(short must)
{
	register int sdl = destline, sdc = destcol;

	if (notecnt < 2 || !must && state == VISUAL)
		return (0);
	splitw++;
	if (WBOT == WECHO)
		vmoveitup(1, (short)1);
	vigoto(WECHO, 0);
/*! Not sure whether notesgn==0 is in fact possible -- check this !*/
	if (notesgn > 0) {
		ex_printf(MSGSTR(M_284, "%d more lines"), notecnt);
	} else if (notesgn < 0) {
		ex_printf(MSGSTR(M_285, "%d fewer lines"), notecnt);
	} else {
		ex_printf(MSGSTR(M_286, "%d lines"), notecnt);
	}
	if (*notenam) {
		ex_printf(" %s'd", notenam);
	}
	vclreol();
	notecnt = 0;
	if (state != VISUAL)
		vcnt = vcline = 0;
	splitw = 0;
	if (state == ONEOPEN || state == CRTOPEN)
		vup1();
	destline = sdl; destcol = sdc;
	return (1);
}

/*
 * Rrrrringgggggg.
 * If possible, flash screen.
 */
void ex_beep(void)
{
	if (flash_screen && value(FLASH))
		vputp((char *)flash_screen, 0);
	else if (bell)
		vputp((char *)bell, 0);
}

/*
 * Map the command input character c,
 * for keypads and labelled keys which do cursor
 * motions.  I.e. on an adm3a we might map ^K to ^P.
 * DM1520 for example has a lot of mappable characters.
 */

int map(wchar_t c, struct maps *maps, int *fmap)
{
	register int d;
	register wchar_t *p;
	register wchar_t *q;
	wchar_t b[10];	/* Assumption: no keypad sends string longer than 10 */

	/*
	 * Mapping for special keys on the terminal only.
	 * BUG: if there's a long sequence and it matches
	 * some chars and then misses, we lose some chars.
	 *
	 * For this to work, some conditions must be met.
	 * 1) Keypad sends SHORT (2 or 3 char) strings
	 * 2) All strings sent are same length & similar
	 * 3) The user is unlikely to type the first few chars of
	 *    one of these strings very fast.
	 * Note: some code has been fixed up since the above was laid out,
	 * so conditions 1 & 2 are probably not required anymore.
	 * However, this hasn't been tested with any first char
	 * that means anything else except escape.
	 */
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"map(%C): ",c);
#endif
	/*
	 * If c==0, the wchar_t came from getesc typing escape.  Pass it through
	 * unchanged.  0 messes up the following code anyway.
	 */
	if (c==0)
		return(0);

	b[0] = c;
	b[1] = 0;
	for (d=0; maps[d].mapto; d++) {
#ifdef MDEBUG
		if (trace)
			fprintf(trace,"\ntry '%S', ",maps[d].cap);
#endif
		if (p = maps[d].cap) {
			for (q=b; *p; p++, q++) {
#ifdef MDEBUG
				if (trace)
					fprintf(trace,"q->b[%d], ",q-b);
#endif
				if (*q==0) {
					/*
					 * Is there another wchar_t waiting?
					 *
					 * This test is oversimplified, but
					 * should work mostly. It handles the
					 * case where we get an ESCAPE that
					 * wasn't part of a keypad string.
					 */
					if ((c=='#' ? peekkey() : fastpeekkey()) == 0) {
#ifdef MDEBUG
						if (trace)
							fprintf(trace,"fpk=0: will return '%C'",c);
#endif
						/*
						 * Nothing waiting.  Push back
						 * what we peeked at & return
						 * failure (c).
						 *
						 * We want to be able to undo
						 * commands, but it's nonsense
						 * to undo part of an insertion
						 * so if in input mode don't.
						 */
#ifdef MDEBUG
						if (trace)
							fprintf(trace, "Call macpush, b %d %d %d\n", b[0], b[1], b[2]);
#endif
						macpush(&b[1],maps == arrows);
#ifdef MDEBUG
						if (trace)
							fprintf(trace, "return %C\n", c);	
#endif
						return(c);
					}
					*q = getkey();
					q[1] = 0;
				}
				if (*p != *q)
					goto contin;
			}
			macpush(maps[d].mapto,maps == arrows);
			if (fmap)
				*fmap = 1;
			c = getkey();
#ifdef MDEBUG
			if (trace)
				fprintf(trace,"Success: push(%S), return %C",maps[d].mapto, c);
#endif
			return(c);	/* first wchar_t of map string */
			contin:;
		}
	}
#ifdef MDEBUG
	if (trace)
		fprintf(trace,"Fail: push(%S), return %C", &b[1], c);
#endif
	macpush(&b[1],0);
	return(c);
}

/*
 * Push st onto the front of vmacp. This is tricky because we have to
 * worry about where vmacp was previously pointing. We also have to
 * check for overflow (which is typically from a recursive macro)
 * Finally we have to set a flag so the whole thing can be undone.
 * canundo is 1 iff we want to be able to undo the macro.  This
 * is false for, for example, pushing back lookahead from fastpeekkey(),
 * since otherwise two fast escapes can clobber our undo.
 */
void macpush(wchar_t *st, int canundo)
{
	wchar_t tmpbuf[LBSIZE];

	if (st==0 || *st==0)
		return;
#ifdef MDEBUG
	if (trace){
		fprintf(trace, "*** From canundo(): ");
		fprintf(trace, "macpush(%S), canundo=%d\n",st,canundo);
	}
#endif
	if ((vmacp ? wcslen(vmacp) : 0) + wcslen(st) > LBSIZE)
		error(MSGSTR(M_238, "The macro is too long.@It may be recursive."), DUMMY_INT);
	if (vmacp) {
		wcscpy(tmpbuf, vmacp);
		if (!FIXUNDO)
			canundo = 0;	/* can't undo inside a macro anyway */
	}
	wcscpy(vmacbuf, st);
	if (vmacp)
		wcscat(vmacbuf, tmpbuf);
	vmacp = vmacbuf;
	/* arrange to be able to undo the whole macro */
	if (canundo) {
		vch_mac = VC_NOCHANGE;
	}
}

#ifdef UNDOTRACE
void visdump(char *s)
{
	register int i;

	if (!trace) return;

	fprintf(trace, "\n%s: basWTOP=%d, basWLINES=%d, WTOP=%d, WBOT=%d, WLINES=%d, WCOLS=%d, WECHO=%d\n",
		s, basWTOP, basWLINES, WTOP, WBOT, WLINES, WCOLS, WECHO);
	fprintf(trace, "   vcnt=%d, vcline=%d, cursor=%d, wcursor=%d, wdot=%d\n",
		vcnt, vcline, cursor-linebuf, wcursor-linebuf, wdot-zero);
	fprintf(trace, "*** From visdump(): ");
	for (i=0; i<TUBELINES; i++)
		if (vtube[i] && *vtube[i])
			fprintf(trace, "%d: '%S'\n", i, vtube[i]);
	tvliny();
}

void vudump(char *s)
{
	register line *p;
	wchar_t savelb[1024];

	if (!trace) return;

	fprintf(trace, "\n%s: undkind=%d, vundkind=%d, unddel=%d, undap1=%d, undap2=%d,\n",
		s, undkind, vundkind, lineno(unddel), lineno(undap1), lineno(undap2));
	fprintf(trace, "  undadot=%d, dot=%d, dol=%d, unddol=%d, truedol=%d\n",
		lineno(undadot), lineno(dot), lineno(dol), lineno(unddol), lineno(truedol));
	fprintf(trace, "  [\n");
	CP(savelb, linebuf);
	fprintf(trace, "linebuf = '%S'\n", linebuf);
	for (p=zero+1; p<=truedol; p++) {
		fprintf(trace, "%o ", *p);
		getline(*p);
		fprintf(trace, "'%S'\n", linebuf);
	}
	fprintf(trace, "]\n");
	CP(linebuf, savelb);
}
#endif

/*
 * Get a count from the keyed input stream.
 * A zero count is indistinguishable from no count.
 */
int vgetcnt(void)
{
	register int c;
	register int cnt;

	cnt = 0;
	for (;;) {
		c = getkey();
		if (!iswdigit(c))
			break;
		cnt *= 10, cnt += c - '0';
	}
	ungetkey(c);
	Xhadcnt = 1;
	Xcnt = cnt;
	return(cnt);
}

/*
 * fastpeekkey is just like peekkey but insists the character come in
 * fast (within 1 second). This will succeed if it is the 2nd wchar_t of
 * a machine generated sequence (such as a function pad from an escape
 * flavor terminal) but fail for a human hitting escape then waiting.
 */
static
int fastpeekkey(void)
{
	register int c;

	/*
	 * If the user has set notimeout, we wait forever for a key.
	 * If we are in a macro we do too, but since it's already
	 * buffered internally it will return immediately.
	 * In other cases we force this to die in 1 second.
	 * This is pretty reliable (VMUNIX rounds it to .5 - 1.5 secs,
	 * but UNIX truncates it to 0 - 1 secs) but due to system delays
	 * there are times when arrow keys or very fast typing get counted
	 * as separate.  notimeout is provided for people who dislike such
	 * nondeterminism.
	 */
	CATCH
		if (value(TIMEOUT) && inopen >= 0) {
			(void)signal(SIGALRM, trapalarm);
			setalarm();
		}
		c = peekkey();
		cancelalarm();
	ONERR
		c = 0;
	ENDCATCH
	/* Should have an alternative method based on select for 4.2BSD */
	return(c);
}

#ifdef FTIOCSET
static int ftfd;
struct requestbuf {
	short time;
	short signo;
};
#endif

/*
 * Arrange for SIGALRM to come in shortly, so we don't
 * hang very long if the user didn't type anything.  There are
 * various ways to do this on different systems.
 */
static void setalarm(void)
{
	static useconds, once;

	/*
	 * Sigh.  The brain dead architecture forced on the networking folks
	 * comes around to bite me again.
	 * When using VI over a rlogin session, 1/2 second ESCDELAY is required
	 * for function key sequences to be interpreted reliably.  Since this
	 * has been out in the field for some time, I am forced to keep the
	 * 1/2 second default value.
	 * (When using VI over a rlogin session to a SUN, there is no
	 *  problem.  When using VI over a telnet session to a rs6k, there
	 *  is also no problem. This is not an rlogin problem, though.)
	 *
	 * The scaling of ESCDELAY is the same that is used in curses.
	 */
	if (!once) {
		char *delay;

		delay = getenv("ESCDELAY");
		if (delay)
			useconds = atoi(delay) * 200;
		else
			useconds = 500000;	/* .5 seconds	*/

		++once;
	}

	ualarm(useconds, 0);
}

/*
 * Get rid of any impending incoming SIGALRM.
 */
void cancelalarm(void)
{
	alarm(0);
}

static void trapalarm(void) {
	alarm(0);
	longjmp(vreslab,1);
}
