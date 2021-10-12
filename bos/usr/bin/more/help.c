static char sccsid[] = "@(#)82	1.1  src/bos/usr/bin/more/help.c, cmdscan, bos411, 9428A410j 7/26/93 11:34:53";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS: help.
 *
 * ORIGINS: 85, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <paths.h>
#include "less.h"


static char *help_msg[] = {

"  Commands flagged with an asterisk (``*'') may be preceeded by a number.\n",
"  Commands of the form ``^X'' are control characters, i.e. control-X.\n",
"\n",
"  h              Display this help.\n",
"\n",
"  f, ^F, SPACE * Forward  N lines, default one screen.\n",
"  b, ^B        * Backward N lines, default one screen.\n",
"  j, CR, ^E    * Forward  N lines, default 1 line.\n",
"  k  ^Y        * Backward N lines, default 1 line.\n",
"  d, ^D        * Forward  N lines, default half screen or last N to d/u.\n",
"  u, ^U        * Backward N lines, default half screen or last N to d/u.\n",
"  g            * Go to line N, default 1.\n",
"  G            * Go to line N, default the end of the file.\n",
"  p, %         * Position to N percent into the file.\n",
"\n",
"  r, ^L          Repaint screen.\n",
"  R              Repaint screen, discarding buffered input.\n",
"\n",
"  m[a-z]         Mark the current position with the supplied letter.\n",
"  '[a-z]         Return to the position previously marked by this letter.\n",
"  ''             Return to previous position.\n",
"\n",
"  /pattern     * Search forward  for N-th line containing the pattern.\n",
"  /!pattern    * Search forward  for N-th line NOT containing the pattern.\n",
"  ?pattern     * Search backward for N-th line containing the pattern.\n",
"  ?!pattern    * Search backward for N-th line NOT containing the pattern.\n",
"  n            * Repeat previous search (for N-th occurence).\n",
"\n",
"  :a              Display the list of files.\n",
"  E [file]        Examine a new file.\n",
"  :e [file]       Examine a new file.\n",
"  :n, N        *  Examine the next file.\n",
"  :p, P        *  Examine the previous file.\n",
"  :t [tag]        Examine the tag.\n",
"  v               Run an editor on the current file.\n",
"\n",
"  =, ^G, :f       Print current file name and stats.\n",
"\n",
"  q, :q, or ZZ    Exit.\n",
"  !command        Invoke a shell with command.\n",
"  :!command       Invoke a shell with command.\n",
"  z               Scroll N lines, default one screen. \n",
NULL };
/*
 * Print out the help screen.
 *
 * What this used to do was invoke more on the help file,
 * but thanks to i18n, we have to get this out of a message catalog.
 * This is slow, but effective (hey, its the help case!).
 */

char *help_file = NULL;	/* name of file with help strings */

void
help(void)
{
	char 	cmd[MAXPATHLEN + 20];
	int 	i=0;
	int	fd;
	char 	*msg;

#ifdef _PATH_HELPFILE
	/*
	 * optimise the non-messsage catalog case
	 */
	msg = MSGSTR(HELP_01, "default");
	if (strcmp(msg, "default") == 0) {
		(void)sprintf(cmd, "-more %s", _PATH_HELPFILE);
		lsystem(cmd);
		return;
	}
#endif

	/*
	 * Strategy:
	 *	Get message from the catalog and write in to a temp file.
	 * 	Run more on this temp file.
	 */
	if (help_file == NULL) {
		if ((help_file = tmpnam(NULL)) == NULL ||
		    (fd = open(help_file, (O_WRONLY|O_CREAT), 0666)) == -1) {
			error("Can't create temp file for help");
			return;
		}
		while (help_msg[i] != NULL) {
			msg = MSGSTR(HELP_01 + i, help_msg[i]);
			write(fd, msg, strlen(msg));
			i++;
		}
		close(fd);
	}

	(void)sprintf(cmd, "-more %s", help_file);
	lsystem(cmd);
	error(MSGSTR(HELPEND, "End of help"));
}
