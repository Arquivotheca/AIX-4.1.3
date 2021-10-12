#if !defined(lint)
static char sccsid [] = "@(#)19  1.28  src/bos/usr/bin/ex/ex_subr.c, cmdedit, bos41J, 9523B_all 6/6/95 07:39:40";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_subr.c
 *
 * FUNCTIONS: tabcol, Ignore, Ignorf, WCstrend, any, anycp,
 * backtab, change, column, comment, copyw, copywR, dingdong, error, filioerr,
 * fixindent, genindent, getDOT, getmark, if, ignnEOF, iswhite, junk, killcnt,
 * killed, lineDOL, lineDOT, lineno, markDOT, markit, markpr, markreg, merror,
 * mwarn, mesg, morelines, netchHAD, netchange, nonzero, notable, notempty,
 * oncore, onhup, onintr, onsusp, preserve, printwid, putmark, putmk1, qcolumn,
 * qcount, qdistance, reverse, save, save12, saveall, setrupt, skipwh, smerror,
 * span, strcLIN, strend, ex_sync, syserror, vfindcol, vpastwh, vskipwh,
 * whitecnt
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
#include "ex_re.h"
#include "ex_tty.h"
#include "ex_vis.h"

#include <string.h>

/* The variable nerrors records the number of error messages
   that are currently displayed on the screen. */
unsigned nerrors = 0;

ttymode	setty(ttymode);
void	normal(ttymode);
static void qcount(int);

/*
 * Random routines, in alphabetical order.
 */

int any(int c, register char *s)
{
	register int x;
	int char_len;
	wchar_t pwcptr;
	
	while ((char_len = mbtowc(&pwcptr, s, MB_CUR_MAX)) > 0)
	{
		s += char_len;
		x = pwcptr;
		if (x == c)
			return (1);
	}

	return (0);
}

/*
 * anycp - return the length of the longest closing punctuation match (0 to 9)
 *	   Example: an ellipsis (...) is 3 characters.
 *	   Checking is right to left.
 */
int anycp(wchar_t *sb, wchar_t *se)
{
	register char *cpunct;
	register int i, retval;
	register wchar_t *mp, *s;
	int char_len;
	wchar_t pwcptr;
	/* multi-char punctuation up to 9 characters */
	wchar_t multipunct[10];

	++se;
	retval = 0;		/* retval holds longest match */
	cpunct = svalue(CLOSEPUNCT);
	while (*cpunct)  {
		if (*cpunct >= '1' && *cpunct <= '9')
			i = *cpunct++ - '0';
		else
			i = 1;
		if (i <= retval) {
			while (i--) {
			if ((char_len = mbtowc(&pwcptr, cpunct, MB_CUR_MAX)) > 0)
				cpunct += char_len;
			else
				error(MSGSTR(M_652, "Incomplete or invalid multibyte character, conversion failed."), DUMMY_INT);

			}
		} else {
			mp = multipunct;
			while (i--) {
				int char_len;
				wchar_t pwcptr;

				if ((char_len = mbtowc(&pwcptr, cpunct, MB_CUR_MAX)) > 0)
				{
					*mp++ = pwcptr;
					cpunct += char_len;
				}
				else
					error(MSGSTR(M_652, "Incomplete or invalid multibyte character, conversion failed."), DUMMY_INT);

			}

			s = se;
			/* right to left matching */
			do {
				if (mp == multipunct) { retval = se-s; break; }
			} while (s > sb && *--mp == *--s);
		}
	}
	return (retval);
}


int backtab(register int i)
{
	register int j;

	j = i % value(SHIFTWIDTH);
	if (j == 0)
		j = value(SHIFTWIDTH);
	i -= j;
	if (i < 0)
		i = 0;
	return (i);
}

void change(void)
{

	tchng++;
	chng = tchng;
}

/*
 * Column returns the number of
 * columns occupied by printing the
 * characters through position cp of the
 * current line.
 */
int column(register wchar_t *cp)
{

	if (cp == 0)
		cp = &linebuf[LBSIZE - 2];
	return (qcolumn(cp, (wchar_t *) 0));
}

/*
 * Ignore a comment to the end of the line.
 * This routine eats the trailing newline so don't call donewline().
 */
void comment(void)
{
	register int c;

	do {
		c = ex_getchar();
	} while (c != '\n' && c != EOF);
	if (c == EOF)
		ungetchar(c);
}

void copyw(register line *to, register line *from, register int size)
{

	if (size > 0)
		do
			*to++ = *from++;
		while (--size > 0);
}

void copywR(register line *to, register line *from, register int size)
{

	while (--size >= 0)
		to[size] = from[size];
}

void dingdong(void)
{

	if (flash_screen && value(FLASH))
		putpad(flash_screen);
	else if (value(ERRORBELLS))
		putpad(bell);
}

int fixindent(int indent)
{
	register int i;
	register wchar_t *cp;

	i = whitecnt(genbuf);
	cp = vpastwh(genbuf);
	if (*cp == 0 && i == indent && linebuf[0] == 0) {
		genbuf[0] = 0;
		return (i);
	}
	CP(genindent(i), cp);
	return (i);
}

void filioerr(char *cp)
{
	register int oerrno = errno;

	lprintf("\"%s\"", cp);
	errno = oerrno;
	syserror(1);
}

wchar_t * genindent(register int indent)
{
	register wchar_t *cp;

	for (cp = genbuf; indent >= value(TABSTOP); indent -= value(TABSTOP))
		*cp++ = '\t';
	for (; indent > 0; indent--)
		*cp++ = ' ';
	return (cp);
}

void getDOT(void)
{

	getline(*dot);
}

line *
getmark(register int c)
{
	register line *addr;
	
	for (addr = one; addr <= dol; addr++)
		if (names[c - 'a'] == (*addr &~ 01)) {
			return (addr);
		}
	return (0);
}

void ignnEOF(void)
{
	register int c = ex_getchar();

	if (c == EOF)
		ungetchar(c);
	else if (c=='"')
		comment();
}

/* returns one if blank or tab found */
int iswhite(int c)
{

	return (c == ' ' || c == '\t');
}

int junk(register int c)
{

	if (c && !value(BEAUTIFY))
		return (0);
	if (iswprint(c))
		return (0);
	switch (c) {

	case '\t':
	case '\n':
	case '\f':
		return (0);

	default:
		return (1);
	}
}

void killed(void)
{

	killcnt(addr2 - addr1 + 1);
}

void killcnt(register int cnt)
{

	if (inopen) {
		notecnt = cnt;
		notenam = "";
		notesgn = 0;
		return;
	}
	if (!notable(cnt))
		return;
	ex_printf(MSGSTR(M_148, "%d lines"), cnt);
	if (value(TERSE) == 0) {
		ex_printf(" %c%s", Command[0] | ' ', Command + 1);
		if (Command[strlen(Command) - 1] != 'e')
			ex_putchar('e');
		ex_putchar('d');
	}
	putNFL();
}

int lineno(line *a)
{

	return (a - zero);
}

int lineDOL(void)
{

	return (lineno(dol));
}

int lineDOT(void)
{

	return (lineno(dot));
}

void markDOT(void)
{

	markpr(dot);
}

void markpr(line *which)
{

/*
 * This used to say "which <= endcore", but endcore is now virtually
 * meaningless.  Anyway, as near as I can tell the test is always true.
 */
	if ((inglobal == 0 || inopen) && which <= dol) {
		names['z'-'a'+1] = *which & ~01;
		if (inopen)
			ncols['z'-'a'+1] = cursor;
	}
}

int markreg(register int c)
{

	if (c == '\'' || c == '`')
		return ('z' + 1);
	if (c >= 'a' && c <= 'z')
		return (c);
	return (0);
}

/*
 * Mesg decodes the terse/verbose strings. Thus
 *	'xxx@yyy' -> 'xxx' if terse, else 'xxx yyy'
 *	'xxx|yyy' -> 'xxx' if terse, else 'yyy'
 * All others map to themselves.
 */
char *
mesg(register char *str)
{
	static char errbuf[256];	/* return w/ error message processing */
	register char *cp;
	int	n;
	
	str = strcpy(errbuf, str);
	for (cp = str; *cp; cp++)
		switch (*cp) {

		case '@':
			if (value(TERSE))
				*cp = 0;
			else
				*cp = ' ';
			break;

		case '|':
			if (value(TERSE) == 0)
				return (cp + 1);
			*cp = 0;
			break;

		default :
			if ((n = mblen(cp, MB_CUR_MAX)) > 1)
				cp += n - 1;

		}
	return (str);
}

/*   VARARGS2   */
void
merror(char *str, int i)
{
	register char *cp = str;

	if (str == 0)
		return;
	if (nerrors++)		/* separate error messages with a space */
		ex_putchar(' ');
	if (*cp == '\n')
		putnl(), cp++;
	if (inopen > 0 && clr_eol)
		vclreol();
	if (enter_standout_mode && exit_standout_mode)
		putpad(enter_standout_mode);
	ex_printf(mesg(cp), i);
	if (enter_standout_mode && exit_standout_mode)
		putpad(exit_standout_mode);
}

/*   VARARGS2   */
void
mwarn(char *str, int i)
{
/*
 *  The "No write since last change" message is simply a warning;
 *  it should not cause a non-zero exit at any time.
 *
	if (!intty)
		exit_status = 1;
*/
	flush();
	outfd = 2;
	merror(str, i);
	flush();
	outfd = 1;
}

int morelines(void)
{
#ifdef UNIX_SBRK
	char *sbrk();

	if ((int) sbrk(1024 * sizeof (line)) == -1)
		return (-1);
	endcore += 1024;
	return (0);
#else
	/*
	 * We can never be guaranteed that we can get more memory
	 * beyond "endcore".  So we just punt every time.
	 */
	return -1;
#endif
}

void nonzero(void)
{

	if (addr1 == zero) {
		notempty();
		error(MSGSTR(M_149, "Nonzero address required@on this command"), DUMMY_INT);
	}
}

int notable(int i)
{

	return (hush == 0 && !inglobal && i > value(REPORT));
}


void notempty(void)
{

	if (dol == zero)
		error(MSGSTR(M_150, "No lines@in the buffer"), DUMMY_INT);
}


void netchHAD(int cnt)
{

	netchange(lineDOL() - cnt);
}

void netchange(register int i)
{
	if (i > 0) {
		notesgn = 1;
	} else {
		notesgn = -1;
		i = -i;
	}
	if (inopen) {
		notecnt = i;
		notenam = "";
		return;
	}
	if (!notable(i))
		return;
	if (notesgn > 0) {
		ex_printf(mesg(MSGSTR(M_282, "%d more lines@in file after %s")), i, Command);
	} else {
		ex_printf(mesg(MSGSTR(M_283, "%d fewer lines@in file after %s")), i, Command);
	}
	putNFL();
}

int numb_offset()
{
	int offset;

	offset = 6 + value(TABSTOP);
	offset -= offset % value(TABSTOP);
	return (offset);
}

int numb_tab_offset(int col, int ts)
{
	int offset;
	if (value(NUMBER))
		offset = (col - numb_offset() + ts) % ts;
	else
		offset = col % ts;
	return (ts - offset);
}

void putmark(line *addr)
{

	putmk1(addr, putline());
}


int printwid(wchar_t c,int fromcol)
{
	register int len;

	if (c == '\t') {
		return (numb_tab_offset(fromcol, value(TABSTOP)));
	} else if (iswcntrl(c)) {
		return (2);
	} else {
		if ((len = wcwidth(c)) < 0)
			len = 1;
		return (len);
	}
}

void putmk1(register line *addr, int n)
{
	register line *markp;
	register oldglobmk;

	oldglobmk = *addr & 1;
	*addr &= ~1;
	for (markp = (anymarks ? names : &names['z'-'a'+1]);
	  markp <= &names['z'-'a'+1]; markp++)
		if (*markp == *addr)
			*markp = n;
	*addr = n | oldglobmk;
}

static short	vcntcol;

int qcolumn(register wchar_t *lim, register wchar_t *gp)
{
	register int x;
	void (*OO)(int);

	OO = Outchar;
	Outchar = qcount;
	vcntcol = 0;
	if (lim != NULL)
		x = lim[1], lim[1] = 0;
	pline(0);
	if (lim != NULL)
		lim[1] = x;
	if (gp)
		while (*gp)
			ex_putchar(*gp++);
	Outchar = OO;
	return (vcntcol);
}

static void qcount(int c)
{
	register int len;

	if (c == '\t') {
		vcntcol += numb_tab_offset(vcntcol, value(TABSTOP));
		return;
	}

	/*
	 * count the PARTIALCHAR indicator that is put in the last display
	 * column when bumping over a dblwid character 
	 */
	if (is_dblwid(c) && vcntcol % WCOLS == (WCOLS - 1))
		vcntcol++;
	if ((len = wcwidth(c)) < 0)
		len = 1;
	vcntcol += len;
}

/* qdistance returns display distance between two points */
static int
qdistance(wchar_t *begin,wchar_t *bend)
{
	register wchar_t *cp;
	void (*OO)(int);

	OO = Outchar;
	Outchar = qcount;
	vcntcol = (Pline == numbline ? numb_offset() : 0);
	for (cp = begin; cp < bend; ++cp)
		ex_putchar(*cp);
	Outchar = OO;
	return (vcntcol);
}

void reverse(register line *a1, register line *a2)
{
	register line t;

	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

static void save(line *a1, register line *a2)
{
	register int more;

	if (!FIXUNDO)
		return;
#ifdef UNDOTRACE
	if (trace)
		vudump("before save\n");
#endif
	undkind = UNDNONE;
	undadot = dot;
	more = (a2 - a1 + 1) - (unddol - dol);
	while (more >= (endcore - truedol))
		if (morelines() < 0)
			error(MSGSTR(M_153, "Out of memory@saving lines for undo - try using ed"), DUMMY_INT);
	if (more)
		(*(more > 0 ? copywR : copyw))(unddol + more + 1, unddol + 1,
		    (truedol - unddol));
	unddol += more;
	truedol += more;
	copyw(dol + 1, a1, a2 - a1 + 1);
	undkind = UNDALL;
	unddel = a1 - 1;
	undap1 = a1;
	undap2 = a2 + 1;
#ifdef UNDOTRACE
	if (trace)
		vudump("after save\n");
#endif
}

void save12(void)
{

	save(addr1, addr2);
}

void saveall(void)
{

	save(one, dol);
}

int span(void)
{

	return (addr2 - addr1 + 1);
}

void ex_sync(void)
{

	chng = 0;
	tchng = 0;
	xchng = 0;
}


int skipwh(void)
{
	register int wh;

	wh = 0;
	while (iswhite(peekchar())) {
		wh++;
		ignchar();
	}
	return (wh);
}

/*   VARARGS2   */
void smerror(char *str, char *cp)
{

	if (str == 0)
		return;
	if (inopen && clr_eol)
		vclreol();
	if (nerrors++)		/* separate error messages with a space */
		ex_putchar(' ');
	if (enter_standout_mode && exit_standout_mode)
		putpad(enter_standout_mode);
	lprintf(mesg(str), cp);
	if (enter_standout_mode && exit_standout_mode)
		putpad(exit_standout_mode);
}

wchar_t * WCstrend(register wchar_t *cp)
{
	while (*cp)
		cp++;
	return (cp);
}

char * strend(register char *cp)
{
	while (*cp)
		cp++;
	return (cp);
}

void strcLIN(wchar_t *dp)
{

	CP(linebuf, dp);
}

/*
 * A system error has occurred that we need to perror.
 * danger is true if we are unsure of the contents of
 * the file or our buffer, e.g. a write error in the
 * middle of a write operation, or a temp file error.
 */
void syserror(int danger)
{
	register int e = errno;
	extern int sys_nerr;

	dirtcnt = 0;
	ex_putchar(' ');
	if (danger)
		edited = 0;	/* for temp file errors, for example */
	exit_status = 1;
	if (e >= 0 && e < sys_nerr)
		error(strerror(e), DUMMY_INT);
	else
		error(MSGSTR(M_190, "System error %d"), e);
}

/*
 * Return the column number that results from being in column col and
 * hitting a tab, where tabs are set every ts columns.	Work right for
 * the case where col > columns, even if ts does not divide columns.
 */
int tabcol(int col, int ts)
{
	int offset, result;

	offset = value(NUMBER) ? numb_offset() : 0;
	if (col+offset >= columns) {
		offset += columns * (col/columns);
		col -= offset;
		result = col + ts - (col % ts) + offset;
	} else {
		result = col + numb_tab_offset(col, ts);
	}
	return (result);
}

wchar_t * vfindcol(int i)
{
	register wchar_t *cp;
	register void (*OO)(int) = Outchar;

	Outchar = qcount;
	ignore(qcolumn(linebuf - 1, NOWCSTR));
	for (cp = linebuf; *cp && vcntcol < i; cp++)
		ex_putchar(*cp);
	if (cp != linebuf)
		cp--;
	Outchar = OO;
	return (cp);
}

wchar_t * vskipwh(register wchar_t *cp)
{

	while (iswhite(*cp) && cp[1])
		cp++;
	return (cp);
}

wchar_t * vpastwh(register wchar_t *cp)
{

	while (iswhite(*cp))
		cp++;
	return (cp);
}

int whitecnt(register wchar_t *cp)
{
	register int i;

	i = 0;
	for (;;)
		switch (*cp++) {

		case '\t':
			i
 += value(TABSTOP) - i % value(TABSTOP);
			break;

		case ' ':
			i++;
			break;

		default:
			return (i);
		}
}

#ifdef lint
void Ignore(char *a)
{

	a = a;
}

void Ignorf(int (*a)())
{

	a = a;
}
#endif

void markit(line *addr)
{

	if (addr != dot && addr >= one && addr <= dol)
		markDOT();
}

/*
 * When a hangup occurs our actions are similar to a preserve
 * command.  If the buffer has not been [Modified], then we do
 * nothing but remove the temporary files and exit.
 * Otherwise, we sync the temp file and then attempt a preserve.
 * If the preserve succeeds, we unlink our temp files.
 * If the preserve fails, we leave the temp files as they are
 * as they are a backup even without preservation if they
 * are not removed.
 */
void
onhup(void)
{

	/*
	 * USG tty driver can send multiple HUP's!!
	 */
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	if (chng == 0) {
		cleanup((short)1);
		catclose(catd);
		exit(exit_status);
	}
	if (setexit() == 0) {
/* AIX security enhancement */
#if !defined(TVI)
		if (preserve()) {
			cleanup((short)1);
			catclose(catd);
			exit(exit_status);
		}
#endif
/* TCSEC Division C Class C2 */
	}
	catclose(catd);
	exit(1);
}

/*
 * Similar to onhup.  This happens when any random core dump occurs,
 * e.g. a bug in vi.  We preserve the file and then generate a core.
 */

void
oncore(int sig)
{
	static int timescalled = 0;
	sigset_t setsig;
	
	/*
	 * USG tty driver can send multiple HUP's!!
	 */
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(sig, SIG_DFL);	/* Insure that we don't catch it again */
	sigemptyset(&setsig);
	sigaddset(&setsig, sig);
	sigprocmask(SIG_UNBLOCK, &setsig, NULL);
	
	if (timescalled++ == 0 && chng && setexit() == 0) {
		if (inopen)
			vsave();
/* AIX security enhancement */
#if !defined(TVI)
		preserve();
		{
			char *txt = MSGSTR(M_191, "\r\nYour file has been preserved\r\n");
			write(1, txt, strlen(txt));
		}
#endif
/* TCSEC Division C Class C2 */
	}
	if (timescalled < 2) {
		normal(normf);
		cleanup((short)2);
		kill(getpid(), sig);	/* Resend ourselves the same signal */
		/* We won't get past here */
	}
	catclose(catd);
	exit(1);
}

/*
 * An interrupt occurred.  Drain any output which
 * is still in the output buffering pipeline.
 * Catch interrupts again.  Unless we are in visual
 * reset the output state (out of -nl mode, e.g).
 * Then like a normal error (with the \n before Interrupt
 * suppressed in visual mode).
 */
void
onintr(void)
{
#ifndef CBREAK
	signal(SIGINT, onintr);
#else
	signal(SIGINT, inopen ? vintr : onintr);
#endif
	cancelalarm();
	draino();
	if (!inopen) {
		pstop();
		setlastchar('\n');
#ifdef CBREAK
	}
#else
	} else
		vraw();
#endif
	if (inopen != 0)
		error(MSGSTR(M_192, "Interrup"), DUMMY_INT);
	else
		error(MSGSTR(M_193, "\nInterrupt"), DUMMY_INT);
}

/*
 * If we are interruptible, enable interrupts again.
 * In some critical sections we turn interrupts off,
 * but not very often.
 */
void setrupt(void)
{

	if (ruptible) {
#ifndef CBREAK
		signal(SIGINT, onintr);
#else
		signal(SIGINT, inopen ? vintr : onintr);
#endif
#ifdef SIGTSTP
		if (dosusp)
			signal(SIGTSTP, onsusp);
#endif
	}
}

/* AIX security enhancement */
#if !defined(TVI)
int preserve(void)
{

	tflush();
	synctmp();
	pid = fork();
	if (pid < 0)
		return (0);
	if (pid == 0) {
		close(0);
		dup(tfile);
		execl(EXPRESERVE, "expreserve", (char *) 0);
		catclose(catd);
		exit(1);
	}
	waitfor();
	if (rpid == pid && status == 0)
		return (1);
	return (0);
}
#endif
/* TCSEC Division C Class C2 */

#ifdef SIGTSTP
/*
 * We have just gotten a susp.	Suspend and prepare to resume.
 */
void
onsusp(void)
{
	ttymode f;
	int savenormtty;
	int killreturn;
	struct winsize win;
	sigset_t setsig;
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"before onsusp\n");
#endif

        if ( state == VISUAL )
           vsave();

	f = setty(normf);
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after setty()\n");
#endif
	vnfl();
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after vnfl()\n");
#endif
	putpad(exit_ca_mode);
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after putpad()\n");
#endif
	flush();
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after flush()\n");
#endif
	resetterm();
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after resetterm()\n");
#endif
	savenormtty = normtty;
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after savenormtty()\n");
#endif
	normtty = 0;
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after normtty()\n");
#endif

	signal(SIGTSTP, SIG_DFL);
	sigemptyset(&setsig);
	sigaddset(&setsig, SIGTSTP);
	sigprocmask(SIG_UNBLOCK, &setsig, NULL);
	
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after signal()\n");
#endif
	killreturn=kill(0, SIGTSTP);

#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after kill--killreturn=%d\n",killreturn);
#endif
	/* the pc stops here */

	signal(SIGTSTP, onsusp);
#ifdef SUSPTRACE
	if (trace)
           fprintf(trace,"in onsusp after signal()\n");
#endif
	normtty = savenormtty;
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after normtty\n");
#endif
	vcontin((short)0);
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after vcontin()\n");
#endif


	setty(f);
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after setty() vcnt = %d, cursor = %d, vmcurs = %d, vcline = %d\n",vcnt, cursor, vmcurs, vcline);
#endif
	if (!inopen)
		error((char *)0, DUMMY_INT);
	else {
		if (ioctl(0, TIOCGWINSZ, ((char *)&win)) >= 0)
			if (win.ws_row != winsz.ws_row ||
			    win.ws_col != winsz.ws_col)
				winchk(DUMMY_INT);
		if (vcnt < 0) {
			vcnt = -vcnt;
			if (state == VISUAL)
				vclear();
			else if (state == CRTOPEN)
				vcnt = 0;
		}
		vdirty(0, lines);
		vcnt = 0;
		splitw = 0;
		vrepaint(cursor);
	}
#ifdef SUSPTRACE
	if (trace)
		fprintf(trace,"in onsusp after if-- vcnt = %d, cursor = %d, vmcurs = %d, vcline = %d\n",vcnt, cursor, vmcurs, vcline);
#endif
}
#endif
