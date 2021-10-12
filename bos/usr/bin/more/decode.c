static char sccsid[] = "@(#)81	1.3  src/bos/usr/bin/more/decode.c, cmdscan, bos41B, 412_41B_sync 11/23/94 11:19:57";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS: noprefix, cmd_decode, cmd_search.
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
 * Routines to decode user commands.
 *
 * This is all table driven.
 * A command table is a sequence of command descriptors.
 * Each command descriptor is a sequence of bytes with the following format:
 *	<c1><c2>...<cN><0><action>
 * The characters c1,c2,...,cN are the command string; that is,
 * the characters which the user must type.
 * It is terminated by a null <0> byte.
 * The byte after the null byte is the action code associated
 * with the command string.
 *
 * The default commands are described by cmdtable.
 */

#include <sys/param.h>
#include <sys/file.h>
#include <stdio.h>
#include "less.h"

/*
 * Command table is ordered roughly according to expected
 * frequency of use, so the common commands are near the beginning.
 */
#define	CONTROL(c)		((c)&037)

static char cmdtable[] = {
	'\r',0,				A_F_LINE,
	'\n',0,				A_F_LINE,
	'j',0,				A_F_LINE,
	CONTROL('E'),0,			A_F_LINE,
	'k',0,				A_B_LINE,
	CONTROL('Y'),0,			A_B_LINE,
	'd',0,				A_F_SCROLL,
	CONTROL('D'),0,			A_F_SCROLL,
	'z',0,				A_SF_SCROLL,
	'u',0,				A_B_SCROLL,
	CONTROL('U'),0,			A_B_SCROLL,
	' ',0,				A_F_SCREEN,
	'f',0,				A_F_SCREEN,
	CONTROL('F'),0,			A_F_SCREEN,
	'b',0,				A_B_SCREEN,
	CONTROL('B'),0,			A_B_SCREEN,
	's',0,				A_SKIP,
	'R',0,				A_FREPAINT,
	'r',0,				A_REPAINT,
	CONTROL('L'),0,			A_REPAINT,
	'g',0,				A_GOLINE,
	'p',0,				A_PERCENT,
	'%',0,				A_PERCENT,
	'G',0,				A_GOEND,
	'0',0,				A_DIGIT,
	'1',0,				A_DIGIT,
	'2',0,				A_DIGIT,
	'3',0,				A_DIGIT,
	'4',0,				A_DIGIT,
	'5',0,				A_DIGIT,
	'6',0,				A_DIGIT,
	'7',0,				A_DIGIT,
	'8',0,				A_DIGIT,
	'9',0,				A_DIGIT,

	'=',0,				A_STAT,
	CONTROL('G'),0,			A_STAT,
	':','f',0,			A_STAT,
	'/',0,				A_F_SEARCH,
	'?',0,				A_B_SEARCH,
	'n',0,				A_AGAIN_SEARCH,
	'N',0,				A_REV_AGAIN_SEARCH,
	'm',0,				A_SETMARK,
	'\'',0,				A_GOMARK,
	'E',0,				A_EXAMINE,
	':','e',0,			A_EXAMINE,
	':','n',0,			A_NEXT_FILE,
	'P',0,				A_PREV_FILE,
	':','p',0,			A_PREV_FILE,
	'v',0,				A_VISUAL,
	'!',0,				A_SHELL,
	':','!',0,			A_SHELL,

	'h',0,				A_HELP,
	'Q',0,				A_QUIT,
	'q',0,				A_QUIT,
	':','Q',0,			A_QUIT,
	':','q',0,			A_QUIT,
	':','t',0,			A_TAGFILE,
	':', 'a', 0,			A_FILE_LIST,
	'Z','Z',0,			A_QUIT,
};

static char *cmdendtable = cmdtable + sizeof(cmdtable);

#define	MAX_CMDLEN	16

static char kbuf[MAX_CMDLEN+1];
static char *kp = kbuf;

/*
 * Indicate that we're not in a prefix command
 * by resetting the command buffer pointer.
 */
void
noprefix(void)
{
	kp = kbuf;
}

/*
 * Decode a command character and return the associated action.
 */
int
cmd_decode(int c)
{
	register int action = A_INVALID;

	/*
	 * Append the new command character to the command string in kbuf.
	 */
	*kp++ = c;
	*kp = '\0';

	action = cmd_search(cmdtable, cmdendtable);

	/* This is not a prefix character. */
	if (action != A_PREFIX)
		noprefix();
	return(action);
}

/*
 * Search a command table for the current command string (in kbuf).
 */
static int
cmd_search(char *table, char *endtable)
{
	register char *p, *q;

	for (p = table, q = kbuf;  p < endtable;  p++, q++) {
		if (*p == *q) {
			/*
			 * Current characters match.
			 * If we're at the end of the string, we've found it.
			 * Return the action code, which is the character
			 * after the null at the end of the string
			 * in the command table.
			 */
			if (*p == '\0')
				return(p[1]);
		}
		else if (*q == '\0') {
			/*
			 * Hit the end of the user's command,
			 * but not the end of the string in the command table.
			 * The user's command is incomplete.
			 */
			return(A_PREFIX);
		} else {
			/*
			 * Not a match.
			 * Skip ahead to the next command in the
			 * command table, and reset the pointer
			 * to the user's command.
			 */
			while (*p++ != '\0');
			q = kbuf-1;
		}
	}
	/*
	 * No match found in the entire command table.
	 */
	return(A_INVALID);
}
