static char sccsid[] = "@(#)90	1.18  src/bos/usr/bin/more/main.c, cmdscan, bos41B, 412_41B_sync 11/23/94 10:39:00";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS: edit, next_file, prev_file, *save, quit, cat_file.
 *
 * ORIGINS: 85, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 *
 * OSF/1 1.2
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

/*
 * Entry point, initialization, miscellaneous routines.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "less.h"

int	ispipe;
static int	new_file;
int	is_tty;
static char	*previous_file;
char	*current_file, *current_name, *next_name;
char	*first_cmd = NULL;
char	*every_first_cmd = NULL;
char 	*cmdname; 	/* more or page or less command */
static off_t	prev_pos;
int	any_display;
int	scroll;
int	ac;
char	**av;
int	curr_ac;
int	quitting;
int	exit_status = 0;
int	mb_cur_max;
nl_catd	catd;

static void	cat_file(void);

extern int	file;
extern int	cbufs;
extern int	errmsgs;

extern char	*tagfile;
extern int	tagoption;
extern int	top_scroll;
extern int 	wait_opt;

/*
 * Edit a new file.
 * Filename "-" means standard input.
 * No filename means the "current" file, from the command line.
 */
int
edit(register char *filename)
{
	register int f;
	register char *m;
	off_t initial_pos;
	static int didpipe;
	char message[PATH_MAX*2], *p;

	initial_pos = NULL_POSITION;
	if (filename == NULL || *filename == '\0') {
		if (curr_ac >= ac) {
			error(MSGSTR(NOCURR, "No current file"));
			return(0);
		}
		filename = save(av[curr_ac]);
	}
	else if (strcmp(filename, "#") == 0) {
		if (*previous_file == '\0') {
			error(MSGSTR(NOPREV, "No previous file"));
			return(0);
		}
		filename = save(previous_file);
		initial_pos = prev_pos;
	} else
		filename = save(filename);

	/* use standard input. */
	if (!strcmp(filename, "-")) {
		if (didpipe) {
			error(MSGSTR(NOVIEW, "Can view standard input only once"));
			return(0);
		}
		f = 0;
	}
	else if ((m = bad_file(filename, message, sizeof(message))) != NULL) {
		error(m);
		free(filename);
		exit_status = 1;
		return(0);
	}
	else if ((f = open(filename, O_RDONLY, 0)) < 0) {
		(void)sprintf(message, "%s: %s", filename, strerror(errno));
		error(message);
		free(filename);
		exit_status = 1;
		return(0);
	}

	if (isatty(f)) {
		/*
		 * Not really necessary to call this an error,
		 * but if the control terminal (for commands)
		 * and the input file (for data) are the same,
		 * we get weird results at best.
		 */
		error(MSGSTR(NOTERM, "Can't take input from a terminal"));
                fprintf(stderr, MSGSTR(USAGE,
"usage: %s [-Ncdeisuvz] [-t tag] [-x tabs] [-p command] [-n number]\n\t    [-W option] [file ...]\n"), cmdname);

		if (f > 0)
			(void)close(f);
		(void)free(filename);
		raw_mode(0);
		exit (1);
	}

	/*
	 * We are now committed to using the new file.
	 * Close the current input file and set up to use the new one.
	 */
	if (file > 0)
		(void)close(file);
	new_file = 1;
	if (previous_file != NULL)
		free(previous_file);
	previous_file = current_file;
	current_file = filename;
 	pos_clear(); 
	prev_pos = position(TOP);
	ispipe = (f == 0);
	if (ispipe) {
		didpipe = 1;
		current_name = MSGSTR(STDIN, "stdin");
	} else
		current_name = (p = rindex(filename, '/')) ? p + 1 : filename;
	if (curr_ac >= ac)
		next_name = NULL;
	else
		next_name = av[curr_ac + 1];
	file = f;
	ch_init(cbufs, 0);
	init_mark();

	/* if specified, -p options are applied to all files */
	if (every_first_cmd != NULL)
		first_cmd = every_first_cmd;

	if (is_tty) {
		int no_display = !any_display;
		any_display = 1;
		if (no_display && errmsgs > 0) {
			/*
			 * We displayed some messages on error output
			 * (file descriptor 2; see error() function).
			 * Before erasing the screen contents,
			 * display the file name and wait for a keystroke.
			 */
			error(filename);
		}
		/*
		 * Indicate there is nothing displayed yet.
		 */
		if (initial_pos != NULL_POSITION)
			jump_loc(initial_pos);
		clr_linenum();
	}
	return(1);
}

/*
 * Edit the next file in the command line list.
 */
void
next_file(int n)
{
	extern int quit_at_eof;
	off_t position();
	int rc = 0, old_ac = curr_ac;

	curr_ac += n;
	while(!rc) {
		if (curr_ac >= ac) {
			curr_ac = old_ac;
			if (!quit_at_eof || position(TOP) == NULL_POSITION)
				quit();
			error(MSGSTR(NONTHF, "No (N-th) next file"));
			return;
		}
		rc = edit(av[curr_ac]);
		if (!rc)
			curr_ac++;
	}
}

/*
 * Edit the previous file in the command line list.
 */
void
prev_file(int n)
{
	int rc = 0, old_ac = curr_ac;

	curr_ac -= n;
	while(!rc) {
		if (curr_ac < 0) {
			curr_ac = old_ac;
			error(MSGSTR(NONTHF2, "No (N-th) previous file"));
			return;
		}
		rc = edit(av[curr_ac]);
		if (!rc)
			curr_ac--;
	}
}

/*
 * copy a file directly to standard output; used if stdout is not a tty.
 */
static void
cat_file(void)
{
	register int c;

	while ((c = ch_forw_get()) != EOI)
		putchr(c);
	flush();
}

int
main(int argc, char **argv)
{
	int envargc, argcnt;
#define	MAXARGS		255			/* should be enough */
	char *envargv[MAXARGS+1];
	char *p;

	setlocale( LC_ALL, "");
	catd = catopen(MF_MORE,NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
	
	if ((p = strrchr(argv[0], '/')) == NULL)
		p = argv[0];
	else
		p++;
	cmdname=p;
	if (strcmp(p, "page") == 0)
		top_scroll = 1;

	/*
	 * Process command line arguments and MORE environment arguments.
	 * Command line arguments override environment arguments.
	 */
	if (p = strdup(getenv("MORE"))) {
		int i = 2;

		envargv[0] = "more";
		envargv[1] = strtok(p, "\t ");
		while ((p = strtok(NULL, "\t ")) != NULL && i < MAXARGS)
			envargv[i++] = p;
		envargv[i] = NULL;
		envargc = i;
		(void)option(envargc, envargv);
	}
	argcnt = option(argc, argv);
	argv += argcnt;
	argc -= argcnt;

	/*
	 * Set up list of files to be examined.
	 */
	ac = argc;
	av = argv;
	curr_ac = 0;

	/*
	 * Set up terminal, etc.
	 */
	is_tty = isatty(1);
	if (!is_tty) {
		/*
		 * Output is not a tty.
		 * Just copy the input file(s) to output.
		 */
		if (ac < 1) {
			(void)edit("-");
			cat_file();
		} else {
			do {
				(void)edit((char *)NULL);
				if (file >= 0)
					cat_file();
			} while (++curr_ac < ac);
		}
		exit(exit_status);
	}

	raw_mode(1);
	get_term();
	open_getchr();
	init();
	init_signals(1);

	/* select the first file to examine. */
	if (tagoption) {
		/*
		 * A -t option was given; edit the file selected by the
		 * "tags" search, and search for the proper line in the file.
		 */
		if (!tagfile || !edit(tagfile) || tagsearch())
			quit();
	}
	else if (ac < 1)
		(void)edit("-");	/* Standard input */
	else {
		/*
		 * Try all the files named as command arguments.
		 * We are simply looking for one which can be
		 * opened without error.
		 */
		do {
			(void)edit((char *)NULL);
		} while (file < 0 && ++curr_ac < ac);
	}

	if (file >= 0)
		commands();
	quit();
	/*NOTREACHED*/
}

/*
 * Copy a string to a "safe" place
 * (that is, to a buffer allocated by malloc).
 */
char *
save(char *s)
{
	char *p;

	p = malloc(strlen(s)+1);
	if (p == NULL)
	{
		error(MSGSTR(NOMEM, "cannot allocate memory"));
		quit();
	}
	return(strcpy(p, s));
}

/*
 * Exit the program.
 */
void
quit(void)
{
	extern char *help_file;
	/*
	 * Put cursor at bottom left corner, clear the line,
	 * reset the terminal modes, and exit.
	 */
	quitting = 1;
	remove(help_file);
	lower_left();
	clear_eol();
	deinit();
	flush();
	raw_mode(0);
	exit(exit_status);
}
