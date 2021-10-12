static char sccsid[] = "@(#)85	1.24  src/bos/usr/bin/more/command.c, cmdscan, bos41J, 9507C 2/8/95 17:37:33";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *       start_mca, prompt, commands, editfile, showlist, expand,
 *	 cmd_erase, cmd_clear, getcc, exec_mca, mca_char, taileq.
 *
 * ORIGINS: 85, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 * command.c,v $ $Revision: 1.7.3.2 $ (OSF) $Date: 1992/08/24 18:16:01 $
 *
 *
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define _ILS_MACROS

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <paths.h>
#include "less.h"

#define	NO_MCA		0
#define	MCA_DONE	1
#define	MCA_MORE	2

static char cmdbuf[120];	/* Buffer for holding a multi-char command */
static char *shellcmd = NULL;	/* For holding last shell command for "!!" */
static char *cp;		/* Pointer into cmdbuf */
static int cmd_col;		/* Current column of the multi-char command */
static int longprompt;		/* if stat command instead of prompt */
static int mca;			/* The multicharacter command (action) */
static int number;		/* The number typed by the user */
static int wsearch;		/* Search for matches (1) or non-matches (0) */

static int	cmd_erase(void);
static int	cmd_char(int);
static int	getcc(void);
static void	exec_mca(void);
static int	mca_char(int c);
static int	taileq(char *, char *);
static void	expand(char **, char *);
static void editfile(void);
static void showlist(void);

#define	CMD_RESET	cp = cmdbuf	/* reset command buffer to empty */
#define	CMD_EXEC	lower_left(); flush()

/* backspace in command buffer. */
static int
cmd_erase(void)
{
	/*
	 * backspace past beginning of the string: this usually means
	 * abort the command.
	 */
	if (cp == cmdbuf)
		return(1);

	/* erase an extra character, for the carat. */
	--cp;
	if (CONTROL_CHAR(*cp)) {
		backspace();
		--cmd_col;
		--cp;
	}

	backspace();
	--cmd_col;
	return(0);
}

/* set up the display to start a new multi-character command. */
static void
start_mca(int action, char *prompt)
{
	lower_left();
	clear_eol();
	putstr(prompt);
	cmd_col = strlen(prompt);
	mca = action;
}

/*
 * process a single character of a multi-character command, such as
 * a number, or the pattern of a search command.
 */
static int
cmd_char(int c)
{
	extern int sc_width;
	extern int erase_char, kill_char, werase_char;

	if (c == erase_char)
		return(cmd_erase());
	/* in this order, in case werase == erase_char */
	if (c == werase_char) {
		if (cp > cmdbuf) {
			while (isspace(cp[-1]) && !cmd_erase());
			while (!isspace(cp[-1]) && !cmd_erase());
			while (isspace(cp[-1]) && !cmd_erase());
		}
		return(cp == cmdbuf);
	}
	if (c == kill_char) {
		while (!cmd_erase());
		return(1);
	}
	/*
	 * No room in the command buffer, or no room on the screen;
	 * {{ Could get fancy here; maybe shift the displayed line
	 * and make room for more chars, like ksh. }}
	 */
	if (cp >= &cmdbuf[sizeof(cmdbuf)-1] || cmd_col >= sc_width-3)
		bell();
	else {
		*cp++ = c;
		if (CONTROL_CHAR(c)) {
			putchr('^');
			cmd_col++;
			c = CARAT_CHAR(c);
		}
		putchr(c);
		cmd_col++;
	}
	return(0);
}

static int
prompt(void)
{
	extern int linenums, verbose;
	extern int curr_ac;
	extern char *current_name, *firstsearch, *next_name;
	extern int quit_at_eof;
	extern int hit_eof;
	extern int ispipe;
	extern int ac;
	extern int screen_trashed;	/* The screen has been overwritten */
	extern int curr_pos;
	extern char *first_cmd;
	off_t len, pos;
	char pbuf[40];

	if (first_cmd != NULL && *first_cmd != '\0')
	{
		/*
		 * No prompt necessary if commands are from first_cmd
		 * rather than from the user.
		 */
		return(1);
	}

	/*
	 * if nothing is displayed yet, display starting from line 1;
	 */
	if (position(TOP) == NULL_POSITION) {
		if (forw_line((off_t)0) == NULL_POSITION)
			return(0);
		jump_back(1);
	}
	else if (screen_trashed)
		repaint();

	/* if -e flag and we've hit EOF on the last file, quit. */
	if (quit_at_eof && hit_eof && curr_ac + 1 >= ac)
		quit();

	/* select the proper prompt and display it. */
	lower_left();
	clear_eol();
	if (longprompt) {
		so_enter();
		putstr(current_name);
		putstr(":");
		if (!ispipe) {
			(void)sprintf(pbuf, MSGSTR(FILEMSG, " file %d/%d"), curr_ac + 1, ac);
			putstr(pbuf);
		}
		if (linenums) {
			(void)sprintf(pbuf, MSGSTR(LINEMSG, " line %d"), currline(curr_pos));
			putstr(pbuf);
		}
		if ((pos = position(BOTTOM)) != NULL_POSITION) {
			(void)sprintf(pbuf, MSGSTR(BYTEMSG, " byte %d"), pos);
			putstr(pbuf);
			if (!ispipe && (len = ch_length())) {
				(void)sprintf(pbuf, MSGSTR(PERCENT, "/%d pct %d%%"),
				    len, ((100 * pos) / len));
				putstr(pbuf);
			}
		}
		so_exit();
		longprompt = 0;
	}
	else {
		so_enter();
		putstr(current_name);
		if (hit_eof)
			if (next_name) {
				putstr(MSGSTR(EOFNEXT, ": END (next file: "));
				putstr(next_name);
				putstr(")");
			}
			else
				putstr(MSGSTR(END, ": END"));
		else if (!ispipe &&
		    (pos = position(BOTTOM)) != NULL_POSITION &&
		    (len = ch_length())) {
			(void)sprintf(pbuf, " (%d%%)", ((100 * pos) / len));
			putstr(pbuf);
		}
		if (verbose)
			putstr(MSGSTR(PHELP, "[Press space to continue, q to quit, h for help]"));
		so_exit();
	}
	return(1);
}
/*
 * Get command character.
 * The character normally comes from the keyboard,
 *  but may come from the "first_cmd" string.
 */
static int
getcc(void)
{
	extern int cmdstack, tagoption;
	extern char *first_cmd;
	int ch;

	/* left over from error() routine. */
	if (cmdstack) {
		ch = cmdstack;
		cmdstack = NULL;
		return(ch);
	}
	if (first_cmd == NULL)
		return (getchr());

	if (*first_cmd == '\0')
	{
		/*
		 * Reached end of first_cmd input.
		 */
		first_cmd = NULL;
		if (cp > cmdbuf &&(tagoption || position(TOP) == NULL_POSITION))
		{
			/*
			 * Command is incomplete, so try to complete it.
			 * There are only two cases:
			 * 1. We have "/string" but no newline.  Add the \n.
			 * 2. We have a number but no command.  Treat as #g.
			 * (This is all pretty hokey.)
			 */
			if (mca != A_DIGIT)
				/* Not a number; must be search string */
				return ('\n'); 
			else
				/* A number; append a 'g' */
				return ('g');
		}
		return (getchr());
	}
	return (*first_cmd++);
}

/* execute a multicharacter command. */
static void
exec_mca(void)
{
	extern char *tagfile;
	register char *p;
	register int n;

	*cp = '\0';
	CMD_EXEC;
	switch (mca) {
	case A_F_SEARCH:
		(void)search(1, cmdbuf, number, wsearch);
		break;
	case A_B_SEARCH:
		(void)search(0, cmdbuf, number, wsearch);
		break;
	case A_EXAMINE:
		{
			extern int exit_status;
			int old_exit_status = exit_status;

			for (p = cmdbuf; isspace(*p); ++p);
			if (edit(mglob(p)) == 0) {
				/**
				 * Defect #162244:
				 * Bad file should not cause a bad
				 * return code when examining a file.
				**/
				if (old_exit_status == 0) {
					exit_status = 0;
				}
			}
		}
		break;
	case A_TAGFILE:
		for (p = cmdbuf; isspace(*p); ++p);
		findtag(p);
		if (tagfile == NULL)
			break;
		if (edit(tagfile))
			(void)tagsearch();
		break;
	case A_SHELL:
		expand(&shellcmd, cmdbuf);

		if (shellcmd == NULL)
			lsystem("");
		else
			lsystem(shellcmd);
		error(MSGSTR(SHDONE, "!done"));
		break;
	}
}

/* add a character to a multi-character command. */
static int
mca_char(int c)
{
	extern int erase_char, kill_char, werase_char;

	switch (mca) {
	case 0:			/* not in a multicharacter command. */
	case A_PREFIX:		/* in the prefix of a command. */
		return(NO_MCA);
	case A_DIGIT:
		/*
		 * Entering digits of a number.
		 * Terminated by a non-digit.
		 */
		if (!isascii(c) || !isdigit(c) &&
		    c != erase_char && c != werase_char) {
			/*
			 * Not part of the number.
			 * Treat as a normal command character.
			 */
			*cp = '\0';
			number = atoi(cmdbuf);
			CMD_RESET;
			mca = 0;
			return(NO_MCA);
		}
		break;
	}

	/*
	 * Any other multicharacter command
	 * is terminated by a newline.
	 */
	if (c == '\n' || c == '\r') {
		exec_mca();
		return(MCA_DONE);
	}

	/* append the char to the command buffer. */
	if (cmd_char(c))
		return(MCA_DONE);

	return(MCA_MORE);
}

/*
 * Main command processor.
 * Accept and execute commands until a quit command, then return.
 */
void
commands(void)
{
	register int c;
	register int action;
	int skip;
	int last_mca;		/* The previous mca */
	extern int scroll;
	extern int sigs;
	extern int sc_height;
	extern int sc_window;
	extern int ispipe;
	extern int quitting;
	extern int verbose;
	extern int erase_char, kill_char;
	extern char *first_cmd;

	last_mca = 0; 
        scroll = (sc_window?sc_window:sc_height) / 2;/* adjust to window size */

	for (;;) {
		mca = 0;
		number = 0;

		/*
		 * See if any signals need processing.
		 */
		if (sigs) {
			psignals();
			if (quitting)
				quit();
		}
		/*
		 * Display prompt and accept a character.
		 */
		CMD_RESET;
		if (!prompt()) {
			next_file(1);
			continue;
		}
		noprefix();
		c = getcc();

again:		if (sigs)
			continue;

		/*
		 * If we are in a multicharacter command, call mca_char.
		 * Otherwise we call cmd_decode to determine the
		 * action to be performed.
		 */
		if (mca)
			switch (mca_char(c)) {
			case MCA_MORE:
				/*
				 * Need another character.
				 */
				c = getcc();
				goto again;
			case MCA_DONE:
				/*
				 * Command has been handled by mca_char.
				 * Start clean with a prompt.
				 */
				continue;
			case NO_MCA:
				/*
				 * Not a multi-char command
				 * (at least, not anymore).
				 */
				break;
			}

		/* decode the command character and decide what to do. */
		switch (action = cmd_decode(c)) {
		case A_DIGIT:		/* first digit of a number */
			start_mca(A_DIGIT, ":");
			goto again;
		case A_F_SCREEN:	/* forward one screen */
			CMD_EXEC;
			if (number <= 0 && (number = sc_window) <= 0)
				number = sc_height - 1;
			if (first_cmd)
				jump_back(1);
			forward(number, 1);
			break;
		case A_B_SCREEN:	/* backward one screen */
			CMD_EXEC;
			if (number <= 0 && (number = sc_window) <= 0)
				number = sc_height - 1;
			backward(number, 1);
			break;
		case A_F_LINE:		/* forward N (default 1) line */
			CMD_EXEC;
			number = number <= 0 ? 1 : number;
			if (first_cmd) /* same action as G command */
				jump_back(number);
			else 
				forward(number, 0);
			break;
		case A_B_LINE:		/* backward N (default 1) line */
			CMD_EXEC;
			backward(number <= 0 ? 1 : number, 0);
			break;
		case A_F_SCROLL:	/* forward N lines */
			CMD_EXEC;
			if (number > 0)
				scroll = number;
			if (first_cmd)
				jump_back(1);
			forward(scroll, 0);
			break;
		case A_SF_SCROLL:	/* forward N lines - set screen size */
			CMD_EXEC;
			if (number > 0)
				sc_window = number;
			if (number <= 0 && (number = sc_window) <= 0)
				number = sc_height - 1;
			/* Display 1st screen before forwarding */
			if (first_cmd) 
				jump_back(1);
			forward(number, 0);
			break;
		case A_SKIP:		/* skip N lines */
			CMD_EXEC;
			if (number > 0)
				skip = number;
			else
				skip = 1;
			if (first_cmd)
				jump_back(1);
			forward(skip, 1);
			break;
		case A_B_SCROLL:	/* backward N lines */
			CMD_EXEC;
			if (number > 0)
				scroll = number;
			backward(scroll, 0);
			break;
		case A_FREPAINT:	/* flush buffers and repaint */
			if (!ispipe) {
				ch_init(0, 0);
				clr_linenum();
			}
			/* FALLTHROUGH */
		case A_REPAINT:		/* repaint the screen */
			CMD_EXEC;
			home();
			clear();
			repaint();
			break;
		case A_GOLINE:		/* go to line N, default 1 */
			CMD_EXEC;
			if (number < 1)
				number = 1;
			jump_back(number);
			break;
		case A_PERCENT:		/* go to percent of file */
			CMD_EXEC;
			if (number < 0)
				number = 0;
			else if (number > 100)
				number = 100;
			jump_percent(number);
			break;
		case A_GOEND:		/* go to line N, default end */
			CMD_EXEC;
			if (number <= 0)
				jump_forw();
			else
				jump_back(number);
			break;
		case A_STAT:		/* print file name, etc. */
			longprompt = 1;
			continue;
		case A_QUIT:		/* exit */
			quit();
		case A_F_SEARCH:	/* search for a pattern */
		case A_B_SEARCH:
			if (number <= 0)
				number = 1;
			start_mca(action, (action==A_F_SEARCH) ? "/" : "?");
			last_mca = mca;
			wsearch = 1;
			c = getcc();
			if (c == '!') {
				/*
				 * Invert the sense of the search; set wsearch
				 * to 0 and get a new character for the start
				 * of the pattern.
				 */
				start_mca(action, 
				    (action == A_F_SEARCH) ? "!/" : "!?");
				wsearch = 0;
				c = getcc();
			}
			goto again;
		case A_AGAIN_SEARCH:		/* repeat previous search */
			if (number <= 0)
				number = 1;
			if (wsearch)
				start_mca(last_mca, 
				    (last_mca == A_F_SEARCH) ? "/" : "?");
			else
				start_mca(last_mca, 
				    (last_mca == A_F_SEARCH) ? "!/" : "!?");
			CMD_EXEC;
			(void)search(mca == A_F_SEARCH, (char *)NULL,
			    number, wsearch);
			break;
		case A_REV_AGAIN_SEARCH: /* repeat prev. search in reverse */
			if (number <= 0)
				number = 1;
			if (wsearch)
				start_mca(last_mca, 
				    (last_mca != A_F_SEARCH) ? "/" : "?");
			else
				start_mca(last_mca, 
				    (last_mca != A_F_SEARCH) ? "!/" : "!?");
			CMD_EXEC;
			(void)search(mca != A_F_SEARCH, (char *)NULL,
			    number, wsearch);
			break;
		case A_HELP:			/* help */
			lower_left();
			clear_eol();
			putstr(MSGSTR(HELP, "help"));
			CMD_EXEC;
			help();
			break;
		case A_TAGFILE:			/* tag a new file */
			CMD_RESET;
			start_mca(A_TAGFILE, MSGSTR(PTAG, "Tag: "));
			c = getcc();
			goto again;
		case A_FILE_LIST:		/* show list of file names */
			CMD_EXEC;
			showlist();
			repaint();
			break;
		case A_EXAMINE:			/* edit a new file */
			CMD_RESET;
			start_mca(A_EXAMINE, MSGSTR(EXAMINE, "Examine: "));
			c = getcc();
			goto again;
		case A_VISUAL:			/* invoke the editor */
			if (ispipe) {
				error(MSGSTR(NOSTDIN, "Cannot edit standard input"));
				break;
			}
			CMD_EXEC;
			editfile();
			ch_init(0, 0);
			clr_linenum();
			break;
		case A_NEXT_FILE:		/* examine next file */
			if (number <= 0)
				number = 1;
			next_file(number);
			break;
		case A_PREV_FILE:		/* examine previous file */
			if (number <= 0)
				number = 1;
			prev_file(number);
			break;
		case A_SHELL:
			/*
			 * Shell Escape
			 */
			CMD_RESET;
			start_mca(A_SHELL, "!");
			c = getcc();
			goto again;
			break;
		case A_SETMARK:			/* set a mark */
			lower_left();
			clear_eol();
			start_mca(A_SETMARK, MSGSTR(MARK, "mark: "));
			c = getcc();
			if (c == erase_char || c == kill_char)
				break;
			setmark(c);
			break;
		case A_GOMARK:			/* go to mark */
			lower_left();
			clear_eol();
			start_mca(A_GOMARK, MSGSTR(GMARK, "goto mark: "));
			c = getcc();
			if (c == erase_char || c == kill_char)
				break;
			gomark(c);
			break;
		case A_PREFIX:
			/*
			 * The command is incomplete (more chars are needed).
			 * Display the current char so the user knows what's
			 * going on and get another character.
			 */
			if (mca != A_PREFIX)
				start_mca(A_PREFIX, "");
			if (CONTROL_CHAR(c)) {
				putchr('^');
				c = CARAT_CHAR(c);
			}
			putchr(c);
			c = getcc();
			goto again;
		default:
			if (verbose)
				error(MSGSTR(PRESSH, "[Press 'h' for instructions.]"));
			else
				bell();
			break;
		}
	}
}

static void
editfile(void)
{
	extern char *current_file;
	extern int curr_pos;
	static int dolinenumber;
	static char *editor;
	int c;
	char buf[MAXPATHLEN * 2 + 20];

	if (editor == NULL) {
		editor = getenv("EDITOR");
		/* pass the line number to vi */
		if (editor == NULL || *editor == '\0') {
			editor = _PATH_VI;
			dolinenumber = 1;
		}
		if (taileq(editor, "vi") || taileq(editor, "ex"))
			dolinenumber = 1;
		else
			dolinenumber = 0;
	}
	if (dolinenumber && (c = currline(curr_pos)))
		(void)sprintf(buf, "%s -c %d %s", editor, c, current_file);
	else
		(void)sprintf(buf, "%s %s", editor, current_file);
	lsystem(buf);
}

static int
taileq(char *path, char *file)
{
	char *p;

	p = ((p = strrchr(path, '/')) ? p + 1 : path);
	return(!strcmp(p, file));
}


static void
showlist(void)
{
	extern int sc_width;
	extern int ac;
	extern int curr_ac;
	extern char **av;
	register int indx, width;
	int len;
	char *p;

	if (ac <= 0) {
		error(MSGSTR(NOFARGS, "No files provided as arguments."));
		return;
	}
	for (width = indx = 0; indx < ac;) {
		p = strcmp(av[indx], "-") ? av[indx] : MSGSTR(STDIN, "stdin");
		len = strlen(p) + 1;
		if (curr_ac == indx)
			len += 2;
		if (width + len + 1 >= sc_width) {
			if (!width) {
				if (curr_ac == indx)
					putchr('[');
				putstr(p);
				if (curr_ac == indx)
					putchr(']');
				++indx;
			}
			width = 0;
			putchr('\n');
			continue;
		}
		if (width)
			putchr(' ');
		if (curr_ac == indx)
			putchr('[');
		putstr(p);
		if (curr_ac == indx)
			putchr(']');
		width += len;
		++indx;
	}
	putchr('\n');
	error((char *)NULL);
}

static void
expand (char **outbuf, char *inbuf)
{
	register char *instr;
	register char *outstr;
	register char ch;
	char temp[ARG_MAX];
	extern int mb_cur_max;
	extern char *current_file;

	instr = inbuf;
	outstr = temp;
	while ((ch = *instr) != '\0') {
		int len;

		if ((len = mblen(instr, mb_cur_max)) <= 0) 
			continue;
		if (len > 1) {
			while (len-- > 0)
				*outstr++ = *instr++;
			continue;
		}
		instr++;

		switch (ch) {
		case '%':
			strcpy (outstr, current_file);
			outstr += strlen (current_file);
			break;
		case '!':
			if (!shellcmd)
				*outstr++ = ch;
			else {
				strcpy (outstr, shellcmd);
				outstr += strlen (shellcmd);
			}
			break;
		case '\\':
			if (*instr == '%' || *instr == '!') {
				*outstr++ = *instr++;
				break;
			}
		default:
			*outstr++ = ch;
		}
	}
	*outstr++ = '\0';
	if (*outbuf == NULL)
		free(*outbuf);
	*outbuf = strdup(temp);
	return ;
}
