static char sccsid[] = "@(#)88	1.3  src/bos/usr/bin/more/position.c, cmdscan, bos41B, 412_41B_sync 12/15/94 17:34:09";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:  	position, add_forw_pos, add_back_pos, copytable, 
 *		pos_clear, onscreen.
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

/*
 * Routines dealing with the "position" table.
 * This is a table which tells the position (in the input file) of the
 * first char on each currently displayed line.
 *
 * {{ The position table is scrolled by moving all the entries.
 *    Would be better to have a circular table 
 *    and just change a couple of pointers. }}
 */

#include <sys/types.h>
#include "less.h"

static off_t *table;		/* The position table */
static int tablesize;
extern int exit_status;

extern sc_window;
/*
 * Return the starting file position of a line displayed on the screen.
 * The line may be specified as a line number relative to the top
 * of the screen, but is usually one of these special cases:
 *	the top (first) line on the screen
 *	the second line on the screen
 *	the bottom line on the screen
 *	the line after the bottom line on the screen
 */
off_t
position(int where)
{
	switch (where)
	{
	case BOTTOM:
		where = sc_window - 1;
		break;
	case BOTTOM_PLUS_ONE:
		where = sc_window;
		break;
	case MIDDLE:
		where = sc_window + 1 / 2;
	}
	return (table[where]);
}

/*
 * Add a new file position to the bottom of the position table.
 */
void
add_forw_pos(off_t pos)
{
	register int i;

	/*
	 * Scroll the position table up.
	 */
	for (i = 1;  i < sc_window + 1;  i++)
		table[i-1] = table[i];
	table[sc_window] = pos;
}

/*
 * Add a new file position to the top of the position table.
 */
void
add_back_pos(off_t pos)
{
	register int i;

	/*
	 * Scroll the position table down.
	 */
	for (i = sc_window;  i > 0;  i--)
		table[i] = table[i-1];
	table[0] = pos;
}

void
copytable(void)
{
	register int a, b;

	for (a = 0; a < sc_window + 1 && table[a] == NULL_POSITION; a++);
	for (b = 0; a < sc_window + 1; a++, b++) {
		table[b] = table[a];
		table[a] = NULL_POSITION;
	}
}

/*
 * Initialize the position table, done whenever we clear the screen.
 */
void
pos_clear(void)
{
	register int i;
	int mem_amount;
	extern char *malloc(), *realloc();

	if (table == 0) {
		tablesize = sc_window + 1;
		mem_amount = (tablesize == 0 ? sizeof (*table) : tablesize * sizeof (*table));
		if ((table = (off_t *)malloc(mem_amount)) == NULL) {
			error(MSGSTR(NOMEM, "cannot allocate memory"));
			exit_status = 1;
			quit();
		}
	} else if (sc_window + 1 >= tablesize) {
		tablesize = sc_window + 1;
		mem_amount = (tablesize == 0 ? sizeof (*table) : tablesize * sizeof (*table));
		table = (off_t *)realloc(table, mem_amount);
	}

	for (i = 0;  i < sc_window + 1;  i++)
		table[i] = NULL_POSITION;
}

/*
 * See if the byte at a specified position is currently on the screen.
 * Check the position table to see if the position falls within its range.
 * Return the position table entry if found, -1 if not.
 */
int
onscreen(off_t pos)
{
	register int i;

	if (pos < table[0])
		return (-1);
	for (i = 1;  i < sc_window + 1;  i++)
		if (pos < table[i])
			return (i-1);
	return (-1);
}
