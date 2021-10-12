#if !defined(lint)
static char sccsid [] = "@(#)29  1.20  src/bos/usr/bin/ex/ex_cmds2.c, cmdedit, bos41B, 9504A 12/19/94 11:46:27";
#endif
/*
 * COMPONENT_NAME: (CMDEDIT) ex_cmds2.c
 *
 * FUNCTIONS: cmdreg, donewline, endcmd, eol, erewind, error, error0, error1,
 * exclam, fixol, makargs, next, nomore, quickly, resetflav, serror, setflav,
 * skipend, skipeol, tail, tail2of, tailprim, tailspec, vcontin, vnfl
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

#include <limits.h>
#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include "ex_vis.h"

extern short	pflag, nflag;		/* mjm: extern; also in ex_cmds.c */
extern int	poffset;		/* mjm: extern; also in ex_cmds.c */

static void error0(void), error1(short);
void	ostop(ttymode);
void	glob(struct glob *);
void	fixterm(void);

/*
 * Subroutines for major command loop.
 */

/*
 * Is there a single letter indicating a named buffer next?
 */
int
cmdreg(void)
{
	register int c = 0;
	register int wh = skipwh();

	/* this tests for ascii alpha not iswalpha because the match has
	   to be only the ascii alpha as they are the only buffers     */

	if (wh && isascii(peekchar()) && isalpha(peekc))
		c = ex_getchar();
	return (c);
}

/*
 * Tell whether the character ends a command
 */
int
endcmd(int ch)
{
	switch (ch) {
	
	case '\n':
	case EOF:
		endline = 1;
		return (1);
	
	case '|':
	case '"':
		endline = 0;
		return (1);
	}
	return (0);
}

/*
 * Insist on the end of the command.
 */
void
eol(void)
{

	if (!skipend())
		error(MSGSTR(M_025, "Extra chars|Extra characters at end of command"), DUMMY_INT);
	ignnEOF();
}

/*
 * Print out the message in the error message file at str,
 * with i an integer argument to printf.
 */

void
error(register char *str, int i)
{

	if (!intty)
		exit_status = 1;
	flush();
	outfd = 2;
	message(str, i);
}

void
message(register char *str, int i)
{
	error0();
	merror(str, i);
	if (writing) {
		serror(MSGSTR(M_026, " [Warning - %s is incomplete]"), file);
		writing = 0;
	}
	error1((short)(str != NULL));
}

/*
 * Rewind the argument list.
 */
void
erewind(void)
{

	argc = argc0;
	argv = argv0;
	args = args0;
	if (argc > 1 && !hush && cur_term) {
		ex_printf(mesg(MSGSTR(M_027, "%d files@to edit")),argc);
		if (inopen)
			ex_putchar(' ');
		else
			putNFL();
	}
}

/*
 * Guts of the pre-printing error processing.
 * If in visual and catching errors, then we dont mung up the internals,
 * just fixing up the echo area for the print.
 * Otherwise we reset a number of externals, and discard unused input.
 */
static void error0(void)
{

	if (laste) {
		tlaste();
		laste = 0;
		ex_sync();
	}
	if (vcatch) {
		if (splitw == 0)
			fixech();
		if (!enter_standout_mode || !exit_standout_mode)
			dingdong();
		return;
	}
	if (input) {
		input += (wcslen(input) - 1);
		if (*input == '\n')
			setlastchar('\n');
		input = 0;
	}
	setoutt();
	flush();
	resetflav();
	if (!enter_standout_mode || !exit_standout_mode)
		dingdong();
	if (inopen) {
		/*
		 * We are coming out of open/visual ungracefully.
		 * Restore columns, undo, and fix tty mode.
		 */
		columns = OCOLUMNS;
		undvis();
		ostop(normf);
		/* ostop should be doing this
		putpad(cursor_normal);
		putpad(key_eol);
		*/
		putnl();
	}
	inopen = 0;
	holdcm = 0;
}

/*
 * Post error printing processing.
 * Close the i/o file if left open.
 * If catching in visual then throw to the visual catch,
 * else if a child after a fork, then exit.
 * Otherwise, in the normal command mode error case,
 * finish state reset, and throw to top.
 */
static void error1(short wasstr)
{
	short die;

	flush();
	outfd = 1;
	if (io > 0) {
		close(io);
		io = -1;
	}
	die = (getpid() != ppid);	/* Only children die */
	inappend = inglobal = 0;
	globp = vglobp = vmacp = 0;
	if (vcatch && !die) {
		inopen = 1;
		vcatch = 0;
		if (wasstr)
			noonl();
		fixol();
		longjmp(vreslab,1);
	}
	if (wasstr && !vcatch)
		putNFL();
	if (die) {
		catclose(catd);
		exit(1);
	}
	lseek(0, 0L, 2);
	if (inglobal)
		setlastchar('\n');
	while (lastchar() != '\n' && lastchar() != EOF)
		ignchar();
	ungetchar(0);
	endline = 1;
	reset();
}

void
fixol(void)
{
	if (Outchar != (void(*)(int))vputchar) {
		flush();
		if (state == ONEOPEN || state == HARDOPEN)
			outline = destline = 0;
		Outchar = (void(*)(int))vputchar;
		vcontin((short)1);
	} else {
		if (destcol)
			vclreol();
		vclean();
	}
}

/*
 * Does an ! character follow in the command stream?
 */
int
exclam(void)
{

	if (peekchar() == '!') {
		ignchar();
		return (1);
	}
	return (0);
}

/*
 * Make an argument list for e.g. next.
 */
void
makargs(void)
{

	glob(&frob);
	argc0 = frob.argc0;
	argv0 = frob.argv;
	args0 = argv0[0];
	erewind();
}

/*
 * Advance to next file in argument list.
 */
void
next(void)
{
	if (argc == 0)
		error(MSGSTR(M_028, "No more files@to edit"), DUMMY_INT);
	morargc = argc;
	if (savedfile[0])
		strcpy(altfile, savedfile);
	strcpy(savedfile, args);
	argc--;
	args = argv ? *++argv : strend(args) + 1;
}

/*
 * Eat trailing flags and offsets after a command,
 * saving for possible later post-command prints.
 */
void
donewline(void)
{
	register int c;

	resetflav();
	for (;;) {
		c = ex_getchar();
		switch (c) {

		case '^':
		case '-':
			poffset--;
			break;

		case '+':
			poffset++;
			break;

		case 'l':
			listf++;
			ignorf(setlist(1));
			break;

		case '#':
			nflag++;
			break;

		case 'p':
			listf = 0;
			break;

		case ' ':
		case '\t':
			continue;

		case '"':
			comment();
			setflav();
			return;

		default:
			if (!endcmd(c))
serror(MSGSTR(M_029, "Extra chars|Extra characters at end of \"%s\" command"), Command);
			if (c == EOF)
				ungetchar(c);
			setflav();
			return;
		}
		pflag++;
	}
}

/*
 * Before quit or respec of arg list, check that there are
 * no more files in the arg list.
 */
void
nomore(void)
{

	if (argc == 0 || morargc == argc)
		return;
	morargc = argc;
	if (argc == 1)
		error(MSGSTR(M_294, "%d more file to edit"), argc);
	else
		error(MSGSTR(M_291, "%d more files to edit"), argc);
}

/*
 * Before edit of new file check that either an ! follows
 * or the file has not been changed.
 */
int
quickly(void)
{

	if (exclam())
		return (1);
	if (chng && dol > zero) {
		xchng = 0;
		serror(MSGSTR(M_032, "No write@since last change (:%s! overrides)"), Command);
	}
	return (0);
}

/*
 * Reset the flavor of the output to print mode with no numbering.
 */
void
resetflav(void)
{

	if (inopen)
		return;
	listf = 0;
	nflag = 0;
	pflag = 0;
	poffset = 0;
	setflav();
}

/*
 * Print an error message with a %s type argument to printf.
 * Message text comes from error message file.
 */
void
serror(char *str, char *cp)
{

	if (!intty)
		exit_status = 1;
	flush();
	outfd = 2;
	error0();
	smerror(str, cp);
	error1((short)(str != NULL));
}

/*
 * Set the flavor of the output based on the flags given
 * and the number and list options to either number or not number lines
 * and either use normally decoded (ARPAnet standard) characters or list mode,
 * where end of lines are marked and tabs print as ^I.
 */
static void
setflav(void)
{

	if (inopen)
		return;
	setnumb(nflag || value(NUMBER));
	setlist(listf || value(LIST));
	setoutt();
}

/*
 * Skip white space and tell whether command ends then.
 */
int
skipend(void)
{

	pastwh();
	return (endcmd(peekchar()) && peekchar() != '"');
}

/*
 * Skip all characters up to and including the '\n' at the end of a line.
 */
int
skipeol(void)
{
	register int n = 0;

	while (peekchar() != '\n') {
		n++;
		ignchar();
	}
	return n;
}

/*
 * Set the command name for non-word commands.
 */
void
tailspec(wchar_t c)
{
	int retval;
	static char foocmd[MB_LEN_MAX];	/* static: 2 byte char is null terminated */

	if ((retval = wctomb(foocmd, c)) <= 0) {	
		foocmd[0] ='\0'; /* If conversion fails, ensure empty string */
	}
	else
		foocmd[retval] = '\0';
	Command = foocmd;
}

/*
 * Try to read off the rest of the command word.
 * If alphabetics follow, then this is not the command we seek.
 */
void
tail(wchar_t *comm)
{

	tailprim(comm, 1, (short)0);
}

void
tail2of(wchar_t *comm)
{

	tailprim(comm, 2, (short)0);
}

static wchar_t	tcommand[20];

void
tailprim(register wchar_t *comm, int i, short notinvis)
{
	register wchar_t *cp;
	register int c;
	static char Cmdbuf[30];

	Command = Cmdbuf;
	wcstombs(Command, comm, 30);
	for (cp = tcommand; i > 0; i--)
		*cp++ = *comm++;
	while (*comm && peekchar() == *comm)
		*cp++ = ex_getchar(), comm++;
	c = peekchar();
	if (notinvis || iswalpha(c)) {
		/*
		 * Of the trailing lp funny business, only dl and dp
		 * survive the move from ed to ex.
		 */
		if (tcommand[0] == 'd' && any(c, "lp"))
			goto ret;
		if (tcommand[0] == 's' && any(c, "gcr"))
			goto ret;
		while (cp < &tcommand[19] && iswalpha(peekchar()))
			*cp++ = ex_getchar();
		*cp = 0;
		if (notinvis)
			serror(MSGSTR(M_033, "What?|%S: No such command from open/visual"), (char *)tcommand);
		else
			serror(MSGSTR(M_034, "What?|%S: Not an editor command"), (char *)tcommand);
	}
ret:
	*cp = 0;
}

/*
 * Continue after a : command from open/visual.
 */
void
vcontin(short ask)
{

	if (vcnt > 0)
		vcnt = -vcnt;
	if (inopen) {
		if (state != VISUAL) {
			/*
			 * We don't know what a shell command may have left on
			 * the screen, so we move the cursor to the right place
			 * and then put out a newline.  But this makes an extra
			 * blank line most of the time so we only do it for :sh
			 * since the prompt gets left on the screen.
			 *
			 * BUG: :!echo longer than current line \\c
			 * will screw it up, but be reasonable!
			 */
			if (state == CRTOPEN) {
				termreset();
				vgoto(WECHO, 0);
			}
			if (!ask) {
				putch('\r');
				putch('\n');
			}
			return;
		}
		if (ask) {
			merror(MSGSTR(M_035, "[Hit return to continue] "), DUMMY_INT);
			flush();
		}
#ifndef CBREAK
		vraw();
#endif
		if (ask) {
			if(getkey() == ':') {
				/* Ugh. Extra newlines, but no other way */
				putch('\n');
				outline = WECHO;
				ungetkey(':');
			}
		}
		if (state == VISUAL)
			Outchar = (void (*)(int)) vputchar;
		vclrech((short)1);
		if (Peekkey != ':') {
			fixterm();
			putpad(enter_ca_mode);
			tostart();
		}
	}
}

/*
 * Put out a newline (before a shell escape)
 * if in open/visual.
 */
void
vnfl(void)
{

	if (inopen) {
		if (state != VISUAL && state != CRTOPEN && destline <= WECHO)
			vclean();
		else
			vmoveitup(1, (short)0);
		vgoto(WECHO, 0);
		vclrcp(vtube[WECHO], WCOLS);
		tostop();
	}
	flush();
}
