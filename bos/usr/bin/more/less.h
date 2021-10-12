/* @(#)93	1.4  src/bos/usr/bin/more/less.h, cmdscan, bos41B, 412_41B_sync 11/23/94 11:20:07 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 *
 * @(#)$RCSfile: less.h,v $ $Revision: 1.1.2.2 $ (OSF) $Date: 1992/08/24 18:17:0
8 $ 
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
 *
 *	less.h	5.9 (Berkeley) 6/1/90
 */

#include <sys/types.h>
#include "more_msg.h"

#define	NULL_POSITION	((off_t)(-1))

#define	EOI		(-1)
#define	READ_INTR	(-2)

/*
 * Special chars used to tell put_line() to do something special.
 * Always preceded by ESC_CHAR if they are real control codes.
 * ESC_CHAR is escaped with itself if it is in input stream.
 */
#define	ESC_CHAR	'\001'		/* character preceding all above */
#define	UL_CHAR		'\002'		/* Enter underline mode */
#define	UE_CHAR		'\003'		/* Exit underline mode */
#define	BO_CHAR		'\004'		/* Enter boldface mode */
#define	BE_CHAR		'\005'		/* Exit boldface mode */

#define	CONTROL_CHAR(c)		(iscntrl(c))
#define	CARAT_CHAR(c)		((c == '\177') ? '?' : (c | 0100))

#define	TOP		(0)
#define	TOP_PLUS_ONE	(1)
#define	THIRD		(2)
#define	BOTTOM		(-1)
#define	BOTTOM_PLUS_ONE	(-2)
#define	MIDDLE		(-3)

#define	A_INVALID		-1

#define	A_AGAIN_SEARCH		1
#define	A_B_LINE		2
#define	A_B_SCREEN		3
#define	A_B_SCROLL		4
#define	A_B_SEARCH		5
#define	A_DIGIT			6
#define	A_EXAMINE		7
#define	A_FREPAINT		8
#define	A_F_LINE		9
#define	A_F_SCREEN		10
#define	A_F_SCROLL		11
#define	A_F_SEARCH		12
#define	A_GOEND			13
#define	A_GOLINE		14
#define	A_GOMARK		15
#define	A_HELP			16
#define	A_NEXT_FILE		17
#define	A_PERCENT		18
#define	A_PREFIX		19
#define	A_PREV_FILE		20
#define	A_QUIT			21
#define	A_REPAINT		22
#define	A_SETMARK		23
#define	A_STAT			24
#define	A_VISUAL		25
#define	A_TAGFILE		26
#define	A_FILE_LIST		27
#define	A_SF_SCROLL		28
#define	A_SKIP			29
#define	A_SHELL			30
#define	A_REV_AGAIN_SEARCH	31

/*
 * A ridiculously complex state machine takes care of backspaces.  The
 * complexity arises from the attempt to deal with all cases, especially
 * involving long lines with underlining, boldfacing or whatever.  There
 * are still some cases which will break it.
 *
 * There are seven possible states as dealing with ln_state:
 *      LN_NORMAL is the normal state (not in underline mode).
 *      LN_UNDERLINE means we are in underline mode.  We expect to get
 *              either a sequence like "_\bX" or "X\b_" to continue
 *              underline mode, or anything else to end underline mode.
 *      LN_BOLDFACE means we are in boldface mode.  We expect to get sequences
 *              like "X\bX\b...X\bX" to continue boldface mode, or anything
 *              else to end boldface mode.
 *      LN_UL_X means we are one character after LN_UNDERLINE
 *              (we have gotten the '_' in "_\bX" or the 'X' in "X\b_").
 *      LN_UL_XB means we are one character after LN_UL_X
 *              (we have gotten the backspace in "_\bX" or "X\b_";
 *              we expect one more ordinary character,
 *              which will put us back in state LN_UNDERLINE).
 *      LN_BO_X means we are one character after LN_BOLDFACE
 *              (we have gotten the 'X' in "X\bX").
 *      LN_BO_XB means we are one character after LN_BO_X
 *              (we have gotten the backspace in "X\bX";
 *              we expect one more 'X' which will put us back
 *              in LN_BOLDFACE).
 */

#define LN_NORMAL       0     /* Not in underline, boldface or whatever mode */
#define LN_UNDERLINE    1       /* In underline, need next char */
#define LN_UL_X         2       /* In underline, got char, need \b */
#define LN_UL_XB        3       /* In underline, got char & \b, need one more */
#define LN_BOLDFACE     4       /* In boldface, need next char */
#define LN_BO_X         5       /* In boldface, got char, need \b */
#define LN_BO_XB        6       /* In boldface, got char & \b, need same char */

extern nl_catd catd;
#define MSGSTR(Num, Str)	catgets(catd, MS_MORE, Num, Str)

/*
 * prototypes
 */

/* ch.c */
extern int 	ch_seek(off_t);
extern int	ch_end_seek(void);
extern int	ch_beg_seek(void);
extern off_t	ch_length(void);
extern off_t	ch_tell(void);
extern int	ch_forw_get(void);
extern int	ch_back_get(void);
extern void	ch_init(int, int);

/* command.c */
extern void	commands(void);

/* decode.c */
extern void	noprefix(void);
extern int	cmd_decode(int);

/* help.c */
extern void	help(void);

/* input.c */
extern off_t	forw_line(off_t);
extern off_t	back_line(off_t);

/* line.c */
extern void	prewind(void);
extern int	pappend(int);
extern off_t	forw_raw_line(off_t);
extern off_t	back_raw_line(off_t);

/* linenum.c */
extern void	clr_linenum(void);
extern void	add_lnum(int, off_t);
extern int 	find_linenum(off_t);
extern int	currline(int);

/* main.c */
extern int 	edit(char *);
extern void	next_file(int);
extern void 	prev_file(int);
extern char	*save(char *);
extern void	quit(void);

/* option.c */
extern int	option(int, char **);

/* os.c */
extern void	lsystem(char *);
extern int	iread(int, unsigned char *, int);
extern void	intread(void);
extern char	*mglob(char *);
extern char 	*bad_file(char *, char *, u_int);

/* output.c */
extern void	put_line(void);
extern void	flush(void);
extern void	purge(void);
extern void	putchr(int);
extern void	putstr(char *);
extern void	error(char *);
extern void	ierror(char *);

/* position.c */
extern off_t	position(int);
extern void	add_forw_pos(off_t);
extern void 	add_back_pos(off_t);
extern void	copytable(void);
extern void	pos_clear(void);
extern int	onscreen(off_t);

/* prim.c */
extern void	forward(int, int);
extern void	backward(int, int);
extern void	repaint(void);
extern void	jump_forw(void);
extern void	jump_back(int);
extern void	jump_percent(int);
extern void	jump_loc(off_t);
extern void	init_mark(void);
extern void	setmark(int);
extern void	gomark(int);
extern int	search(int, char *, int, int);

/* screen.c */
extern void	raw_mode(int);
extern void	get_term(void);
extern void	init(void);
extern void	deinit(void);
extern void	home(void);
extern void	add_line(void);
extern void	lower_left(void);
extern void	bell(void);
extern void	clear(void);
extern void	clear_eol(void);
extern void	so_enter(void);
extern void	so_exit(void);
extern void	ul_enter(void);
extern void	ul_exit(void);
extern void	bo_enter(void);
extern void	bo_exit(void);
extern void	backspace(void);
extern void	putbs(void);

/* signal.c */
extern void	init_signals(int);
extern void 	winch(int);
extern void	psignals(void);

/* tags.c */
extern void	findtag(char *);
extern int	tagsearch(void);

/* ttyin.c */
extern void	open_getchr(void);
extern int	getchr(void);

