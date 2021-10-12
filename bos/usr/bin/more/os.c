static char sccsid[] = "@(#)76  1.4  src/bos/usr/bin/more/os.c, cmdscan, bos412, 9446B 11/15/94 20:12:42";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS: lsystem, iread, intread, *mglob, *bad_file, strtcpy.
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
 * Operating system dependent routines.
 *
 * Most of the stuff in here is based on Unix, but an attempt
 * has been made to make things work on other operating systems.
 * This will sometimes result in a loss of functionality, unless
 * someone rewrites code specifically for the new operating system.
 *
 * The makefile provides defines to decide whether various
 * Unix features are present.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <paths.h>
#include "less.h"

int reading;

extern int screen_trashed;

static jmp_buf read_label;

/*
 * Pass the specified command to a shell to be executed.
 * Like plain "system()", but handles resetting terminal modes, etc.
 */
void
lsystem(char *cmd)
{
	int inp;
	char cmdbuf[ARG_MAX];
	char *shell, *getenv();

	/*
	 * Print the command which is to be executed,
	 * unless the command starts with a "-".
	 */
	if (cmd[0] == '-')
		cmd++;
	else
	{
		lower_left();
		clear_eol();
		putstr("!");
		putstr(cmd);
		putstr("\n");
	}

	/*
	 * De-initialize the terminal and take out of raw mode.
	 */
	deinit();
	flush();
	raw_mode(0);

	/*
	 * Restore signals to their defaults.
	 */
	init_signals(0);

	/*
	 * Force standard input to be the terminal, "/dev/tty",
	 * even if less's standard input is coming from a pipe.
	 */
	inp = dup(0);
	(void)close(0);
	if (open(_PATH_TTY, O_RDONLY, 0) < 0)
		(void)dup(inp);

	/*
	 * Pass the command to the system to be executed.
	 * If we have a SHELL environment variable, use
	 * <$SHELL -c "command"> instead of just <command>.
	 * If the command is empty, just invoke a shell.
	 */
	if ((shell = getenv("SHELL")) != NULL && *shell != '\0')
	{
		if (*cmd == '\0')
			cmd = shell;
		else {
		   if ((strlen(cmd)+strlen(shell)+8)>ARG_MAX) {
			error(MSGSTR(TOOLONG, "Command string is too long"));
			exit(1);
		   } else
		        sprintf(cmdbuf, "%s -c \"%s\"", shell, cmd); 
		   cmd = cmdbuf;
		}
	}
	if (*cmd == '\0')
		cmd = "/usr/bin/sh";

	(void)system(cmd);

	/*
	 * Restore standard input, reset signals, raw mode, etc.
	 */
	(void)close(0);
	(void)dup(inp);
	(void)close(inp);

	init_signals(1);
	raw_mode(1);
	init();
	screen_trashed = 1;
	/*
	 * Since we were ignoring window change signals while we executed
	 * the system command, we must assume the window changed.
	 */
	winch(0);
}

/*
 * Like read() system call, but is deliberately interruptable.
 * A call to intread() from a signal handler will interrupt
 * any pending iread().
 */
int
iread(int fd, unsigned char *buf, int len)
{
	register int n;

	if (setjmp(read_label))
		/*
		 * We jumped here from intread.
		 */
		return (READ_INTR);

	flush();
	reading = 1;
	n = read(fd, (char *)buf, len);
	reading = 0;
	if (n < 0)
		return (-1);
	return (n);
}

void
intread(void)
{
	(void)sigsetmask(0L);
	longjmp(read_label, 1);
}

/*
 * Expand a filename, substituting any environment variables, etc.
 * The implementation of this is necessarily very operating system
 * dependent.  This implementation is unabashedly only for Unix systems.
 */

char *
mglob(char *filename)
{
	FILE *f;
	char *p;
	int ch;
	char *cmd;
	static char buffer[MAXPATHLEN];

	if (filename[0] == '#')
		return (filename);

	/*
	 * We get the shell to expand the filename for us by passing
	 * an "echo" command to the shell and reading its output.
	 */
	p = getenv("SHELL");
	if (p == NULL || *p == '\0')
	{
		/*
		 * Read the output of <echo filename>.
		 */
		cmd = malloc((u_int)(strlen(filename)+8));
		if (cmd == NULL)
			return (filename);
		(void)sprintf(cmd, "echo \"%s\"", filename);
	} else
	{
		/*
		 * Read the output of <$SHELL -c "echo filename">.
		 */
		cmd = malloc((u_int)(strlen(p)+strlen(filename)+12));
		if (cmd == NULL)
			return (filename);
		(void)sprintf(cmd, "%s -c \"echo %s\"", p, filename);
	}

	if ((f = popen(cmd, "r")) == NULL)
		return (filename);
	free(cmd);

	for (p = buffer;  p < &buffer[sizeof(buffer)-1];  p++)
	{
		if ((ch = getc(f)) == '\n' || ch == EOF)
			break;
		*p = ch;
	}
	*p = '\0';
	(void)pclose(f);
	return(buffer);
}

char *
bad_file(char *filename, char *message, u_int len)
{
	struct stat statbuf;

	if (stat(filename, &statbuf) < 0) {
		(void)sprintf(message, "%s: %s", filename, strerror(errno));
		return(message);
	}
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
		(void)sprintf(message, 
				MSGSTR(ISADIR, "%s is a directory"), filename);
		return(message);
	}
	return((char *)NULL);
}
