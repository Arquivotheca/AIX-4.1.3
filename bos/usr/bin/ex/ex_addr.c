static char sccsid[] = "@(#)99	1.9  src/bos/usr/bin/ex/ex_addr.c, cmdedit, bos412, 9445B412 10/26/94 15:18:38";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_addr.c
 *
 * FUNCTIONS: address, getnum, setCNL, setNAEOL, setall, setcount, setdot,
 * setdot1, setnoaddr
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
#include "ex_re.h"

/*
 * Routines for address parsing and assignment and checking of address bounds
 * in command mode.  The routine address is called from ex_cmds.c
 * to parse each component of a command (terminated by , ; or the beginning
 * of the command itself.  It is also called by the scanning routine
 * in ex_voperate.c from within open/visual.
 *
 * Other routines here manipulate the externals addr1 and addr2.
 * These are the first and last lines for the current command.
 *
 * The variable bigmove remembers whether a non-local glitch of . was
 * involved in an address expression, so we can set the previous context
 * mark '' when such a motion occurs.
 */

/*
 * Bigmove defined during linting in response to
 * undefined variable error.
 */
static	int	bigmove;

/*
 * Set up addr1 and addr2 for commands whose default address is dot.
 */
void
setdot(void)
{

	setdot1();
	if (bigmove)
		markDOT();
}

/*
 * Call setdot1 to set up default addresses without ever
 * setting the previous context mark.
 */
void
setdot1(void)
{

	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2) {
		notempty();
		error(MSGSTR(M_002, "Addr1 > addr2|First address exceeds second"), DUMMY_INT);
	}
}

/*
 * Ex allows you to say
 *	delete 5
 * to delete 5 lines, etc.
 * This is implemented by setcount.
 */
void
setcount(void)
{
	register int cnt;

	cnt_set = 0;
	pastwh();
	if (!iswdigit(peekchar())) {
		setdot();
		return;
	}
	addr2 = addr1;
	setdot();
	cnt = getnum();
	if (cnt <= 0)
		error(MSGSTR(M_003, "Bad count|Nonzero count required"), DUMMY_INT);
	addr2 += cnt - 1;
	if (addr2 > dol)
		addr2 = dol;
	nonzero();
	cnt_set = 1;
}

/*
 * Parse a number out of the command input stream.
 */
int
getnum(void)
{
	register int cnt;

	for (cnt = 0; iswdigit(peekcd());)
		cnt = cnt * 10 + ex_getchar() - '0';
	return (cnt);
}

/*
 * Set the default addresses for commands which use the whole
 * buffer as default, notably write.
 */
void
setall(void)
{

	if (addr2 == 0) {
		addr1 = one;
		addr2 = dol;
		if (dol == zero) {
			dot = zero;
			return;
		}
	}
	/*
	 * Don't want to set previous context mark so use setdot1().
	 */
	setdot1();
}

/*
 * No address allowed on, e.g. the file command.
 */
void
setnoaddr(void)
{

	if (addr2 != 0)
		error(MSGSTR(M_004, "No address allowed@on this command"), DUMMY_INT);
}

/*
 * Parse an address.
 * Just about any sequence of address characters is legal.
 *
 * If you are tricky you can use this routine and the = command
 * to do simple addition and subtraction of cardinals less
 * than the number of lines in the file.
 */
line *
address(wchar_t *in_line)
{
	register line *addr;
	register int offset, c;
	short lastsign;

	bigmove = 0;
	lastsign = 0;
	offset = 0;
	addr = 0;
	for (;;) {
		if (iswdigit(peekcd())) {
			if (addr == 0) {
				addr = zero;
				bigmove = 1;
			}
			loc1 = 0;
			addr += offset;
			offset = getnum();
			if (lastsign >= 0)
				addr += offset;
			else
				addr -= offset;
			lastsign = 0;
			offset = 0;
		}
		switch (c = getcd()) {

		case '?':
		case '/':
		case '$':
		case '\'':
		case '\\':
			bigmove++;
		case '.':
			if (addr || offset)
				error(MSGSTR(M_005, "Badly formed address"), DUMMY_INT);
		}
		offset += lastsign;
		lastsign = 0;
		switch (c) {

		case ' ':
		case '\t':
			continue;

		case '+':
			lastsign = 1;
			if (addr == 0)
				addr = dot;
			continue;

		case '^':
		case '-':
			lastsign = -1;
			if (addr == 0)
				addr = dot;
			continue;

		case '\\':
		case '?':
		case '/':
			c = compile(c, 1);
			notempty();
			savere(scanre);
			addr = dot;
			if (in_line && execute(0, dot)) {
				if (c == '/') {
					while (loc1 <= in_line) {
						if (loc1 == loc2)
							loc2++;
						if (!execute(1, (line *)0))
							goto nope;
					}
					break;
				} else if (loc1 < in_line) {
					wchar_t *last;
doques:

					do {
						last = loc1;
						if (loc1 == loc2)
							loc2++;
						if (!execute(1, (line *)0))
							break;
					} while (loc1 < in_line);
					loc1 = last;
					break;
				}
			}
nope:
			for (;;) {
				if (c == '/') {
					addr++;
					if (addr > dol) {
						if (value(WRAPSCAN) == 0)
							error(MSGSTR(M_006, "No match to BOTTOM|Address search hit BOTTOM without matching pattern"), DUMMY_INT);
						addr = zero;
					}
				} else {
					addr--;
					if (addr < zero) {
						if (value(WRAPSCAN) == 0)
							error(MSGSTR(M_007, "No match to TOP|Address search hit TOP without matching pattern"), DUMMY_INT);
						addr = dol;
					}
				}
				if (execute(0, addr)) {
					if (in_line && c == '?') {
						in_line = &linebuf[LBSIZE];
						goto doques;
					}
					break;
				}
				if (addr == dot)
					error(MSGSTR(M_008, "Fail|Pattern not found"), DUMMY_INT);
			}
			continue;

		case '$':
			addr = dol;
			continue;

		case '.':
			addr = dot;
			continue;

		case '\'':
			c = markreg(ex_getchar());
			if (c == 0)
				error(MSGSTR(M_009, "Marks are ' and a-z"), DUMMY_INT);
			addr = getmark(c);
			if (addr == 0)
				error(MSGSTR(M_010, "Undefined mark@referenced"), DUMMY_INT);
			break;

		default:
			ungetchar(c);
			if (offset) {
				if (addr == 0)
					addr = dot;
				addr += offset;
				loc1 = 0;
			}
			if (addr == 0) {
				bigmove = 0;
				return (0);
			}
			if (addr != zero)
				notempty();
			addr += lastsign;
			if (addr < zero)
				error(MSGSTR(M_011, "Negative address@- first buffer line is 1"), DUMMY_INT);
			if (addr > dol)
				error(MSGSTR(M_012, "Not that many lines@in buffer"), DUMMY_INT);
			return (addr);
		}
	}
}

/*
 * Abbreviations to make code smaller
 * Left over from squashing ex version 1.1 into
 * 11/34's and 11/40's.
 */
void
setCNL(void)
{

	setcount();
	donewline();
}

void
setNAEOL(void)
{

	setnoaddr();
	eol();
}
