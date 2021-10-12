static char sccsid[] = "@(#)84	1.4  src/bos/usr/bin/more/line.c, cmdscan, bos412, 9446B 11/15/94 20:12:35";
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS: prewind, pappend, forw_raw_line, back-raw_line.
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
 * Routines to manipulate the "line buffer".
 * The line buffer holds a line of output as it is being built
 * in preparation for output to the screen.
 * We keep track of the PRINTABLE length of the line as it is being built.
 */

#define _ILS_MACROS

#include <sys/types.h>
#include <ctype.h>
#include "less.h"

static char linebuf[LINE_MAX];	/* Buffer which holds the current output line */
static char *curr;		/* Pointer into linebuf */
static int column=0;			/* Printable length, accounting for
				   backspaces, etc. */
static int ln_state;		/* Currently in normal/underline/bold/etc 
				 * mode? All possible cases are described
 				 * in less.h */
				

char *line;			/* Pointer to the current line.
				   Usually points to linebuf. */

extern int bs_mode;
extern int tabstop;
extern int bo_width, be_width;
extern int ul_width, ue_width;
extern int sc_width, sc_height;
extern int show_all_opt, show_opt;
extern int fold_opt;
extern int mb_cur_max;

/*
 * Rewind the line buffer.
 */
void
prewind(void)
{
	line = curr = linebuf;
	ln_state = LN_NORMAL;
	column = 0;
}

/*
 * Update the display width counter when the characters added to linebuf .  
 * In the wrapping mode, if the display width does not fit the screen  
 * it will return 1, otherwise it will update the counter.
 */
#define	NEW_COLUMN(addon) \
    if ((column + addon + (ln_state ? ue_width : 0) > sc_width) && fold_opt) \
		return (1); \
    else \
		column += addon

/*
 * Append a character to the line buffer. 
 * Expand tabs into spaces, handle underlining, boldfacing, etc. 
 * Returns 0 if ok, 1 if line length couldn't fit in screen.
 * In multibyte locale, input character is saved in buffer for
 * processing later.
 */
int
pappend(int c)
{

if (mb_cur_max == 1) { 
	if (c == '\0') {
		/*
		 * Terminate any special modes, if necessary.
		 * Append a '\0' to the end of the line.
		 */
		switch (ln_state) {
		case LN_UL_X:
			curr[1] = curr[-1];
			curr[-1] = ESC_CHAR;
			curr[0] = UE_CHAR;
			curr +=2;
			break;
		case LN_BO_X:
			curr[1] = curr[-1];
			curr[-1] = ESC_CHAR;
			curr[0] = BE_CHAR;
			curr +=2;
			break;
		case LN_UL_XB:
		case LN_UNDERLINE:
			*curr++ = ESC_CHAR;
			*curr++ = UE_CHAR;
			break;
		case LN_BO_XB:
		case LN_BOLDFACE:
			*curr++ = ESC_CHAR;
			*curr++ = BE_CHAR;
			break;
		}
		ln_state = LN_NORMAL;
		*curr = '\0';
		return(0);
	}

	if (curr > linebuf + sizeof(linebuf) - 12)
		/*
		 * Almost out of room in the line buffer.
		 * Don't take any chances.
		 * {{ Linebuf is supposed to be big enough that this
		 *    will never happen, but may need to be made 
		 *    bigger for wide screens or lots of backspaces. }}
		 */
		return(1);
	if (!bs_mode) {
		/*
		 * Advance the state machine.
		 */
		switch (ln_state) {
		case LN_NORMAL:
			if (curr <= linebuf + 1
			    || curr[-1] != '\b')
				break;
			column -= 2;
			if (c == curr[-2])
				goto enter_boldface;
			if (c == '_' || curr[-2] == '_')
				goto enter_underline;
			curr -= 2;
			break;

enter_boldface:
			/*
			 * We have "X\bX" (including the current char).
			 * Switch into boldface mode.
			 */
			column--;
			if ((column + bo_width + be_width  >= sc_width) && 
				fold_opt)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit boldface mode.
				 */
					return(1);

			if (bo_width > 0 && curr > linebuf + 2
			    && curr[-3] == ' ') {
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter boldface" sequence.
				 */
				curr[-1] = curr[-2];
				curr[-2] = BO_CHAR;
				curr[-3] = ESC_CHAR;
				column += bo_width-1;
				curr++;
			} else {
				curr[0] = curr[-2];
				curr[-2] = ESC_CHAR;
				curr[-1] = BO_CHAR;
				column += bo_width;
				curr += 2;
			}
			goto ln_bo_xb_case;

enter_underline:
			/*
			 * We have either "_\bX" or "X\b_" (including
			 * the current char).  Switch into underline mode.
			 */
			column--;
			if ((column + ul_width + ue_width  >= sc_width) &&
				fold_opt)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit underline mode.
				 */
					return (1);
			if (ul_width > 0 && 
			    curr > linebuf + 2 && curr[-3] == ' ')
			{
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter underline" sequence.
				 */
				curr[-1] = curr[-2];
				curr[-2] = UL_CHAR;
				curr[-3] = ESC_CHAR;
				column += ul_width-1;
				curr++;
			} else
			{
				curr[0] = curr[-2];
				curr[-1] = UL_CHAR;
				curr[-2] = ESC_CHAR;
				column += ul_width;
				curr +=2;
			}
			goto ln_ul_xb_case;
			/*NOTREACHED*/
		case LN_UL_XB:
			/*
			 * Termination of a sequence "_\bX" or "X\b_".
			 */
			if (c != '_' && curr[-2] != '_' && c == curr[-2])
			{
				/*
				 * We seem to have run on from underlining
				 * into boldfacing - this is a nasty fix, but
				 * until this whole routine is rewritten as a
				 * real DFA, ...  well ...
				 */
				curr[2] = curr[-2];
				curr[-2] = ESC_CHAR;
				curr[-1] = UE_CHAR;
				curr[0] = ESC_CHAR;
				curr[1] = BO_CHAR;
				curr += 4; /* char & non-existent backspace */
				ln_state = LN_BO_XB;
				goto ln_bo_xb_case;
			}
ln_ul_xb_case:
			if (c == '_')
				c = curr[-2];
			curr -= 2;
			ln_state = LN_UNDERLINE;
			break;
		case LN_BO_XB:
			/*
			 * Termination of a sequence "X\bX".
			 */
			if (c != curr[-2] && (c == '_' || curr[-2] == '_'))
			{
				/*
				 * We seem to have run on from
				 * boldfacing into underlining.
				 */
				curr[2] = curr[-2];
				curr[-2] = ESC_CHAR;
				curr[-1] = BE_CHAR;
				curr[0] = ESC_CHAR;
				curr[1] = UL_CHAR;
				curr += 4; /* char & non-existent backspace */
				ln_state = LN_UL_XB;
				goto ln_ul_xb_case;
			}
ln_bo_xb_case:
			curr -= 2;
			ln_state = LN_BOLDFACE;
			break;
		case LN_UNDERLINE:
			if ((column + ue_width + bo_width + be_width >= sc_width)
				&& fold_opt)
				/*
				 * We have just barely enough room to 
				 * exit underline mode and handle a possible
				 * underline/boldface run on mixup.
				 */
					return (1);
			ln_state = LN_UL_X;
			break;
		case LN_BOLDFACE:
			if (c == '\b')
			{
				ln_state = LN_BO_XB;
				break;
			}
			if ((column + be_width + ul_width + ue_width >= sc_width)
				&& fold_opt)
				/*
				 * We have just barely enough room to 
				 * exit underline mode and handle a possible
				 * underline/boldface run on mixup.
				 */
					return (1);
			ln_state = LN_BO_X;
			break;
		case LN_UL_X:
			if (c == '\b')
				ln_state = LN_UL_XB;
			else
			{
				/*
				 * Exit underline mode.
				 * We have to shuffle the chars a bit
				 * to make this work.
				 */
				curr[1] = curr[-1];
				curr[-1] = ESC_CHAR;
				curr[0] = UE_CHAR;
				column += ue_width;
				curr++;
				if (ue_width > 0 && curr[0] == ' ')
					/*
					 * Another special case for magic
					 * cookie terminals: if the next
					 * char is a space, replace it
					 * with the "exit underline" sequence.
					 */
					column--;
				else
					curr++;
				ln_state = LN_NORMAL;
			} 
			break;
		case LN_BO_X:
			if (c == '\b')
				ln_state = LN_BO_XB;
			else
			{
				/*
				 * Exit boldface mode.
				 * We have to shuffle the chars a bit
				 * to make this work.
				 */
				curr[1] = curr[-1];
				curr[-1] = ESC_CHAR;
				curr[0] = BE_CHAR;
				column += be_width;
				curr++;
				if (be_width > 0 && curr[0] == ' ')
					/*
					 * Another special case for magic
					 * cookie terminals: if the next
					 * char is a space, replace it
					 * with the "exit boldface" sequence.
					 */
					column--;
				else
					curr++;
				ln_state = LN_NORMAL;
			} 
			break;
		}
	}

	if (c == '\t') {
		/*
		 * Expand a tab into spaces.
		 */
		do {
			NEW_COLUMN(1);
		} while (!show_all_opt && (column % tabstop) != 0);
		*curr++ = '\t';
		return (0);
	}

	if (c == '\b') {
		if (ln_state == LN_NORMAL)
			NEW_COLUMN(2);
		else
			column--;
		*curr++ = '\b';
		return(0);
	} 

	/*
	 * Make sure non-printable don't wrap lines.
	 * Two cases:
	 * 	ctrl-X (^X)		== 2 columns
	 * 	meta-X:	(M-X)		== 3 columns
	 * 	meta-cntrl-X:(M-^X)	== 4 columns
	 * 
	 * put_line() prints them out.
	 */
	if (show_opt) {
		if (!isprint(c)) {
			if (iscntrl(c))
				NEW_COLUMN(2);		/* ^X */
			else if (iscntrl(toascii(c)))
				NEW_COLUMN(4);		/* M-^X */
			else
				NEW_COLUMN(3);		/* M-X */
			*curr++ = c;
			return(0);
		}
	}
	NEW_COLUMN(1);
     } 
     /*
      * Ordinary character or part of multibyte character.  Just put it
      * in the buffer.
      * Multibyte characters will be processed in wc_pappend()
      */
     *curr++ = c;
     return (0);
}

/*
 * Analogous to forw_line(), but deals with "raw lines":
 * lines which are not split for screen width.
 * {{ This is supposed to be more efficient than forw_line(). }}
 */
off_t
forw_raw_line(off_t curr_pos)
{
	register char *p;
	register int c;
	off_t new_pos, ch_tell();

	if (curr_pos == NULL_POSITION || ch_seek(curr_pos) ||
		(c = ch_forw_get()) == EOI)
		return (NULL_POSITION);

	p = linebuf;

	for (;;)
	{
		if (c == '\n' || c == EOI)
		{
			new_pos = ch_tell();
			break;
		}
		if (p >= &linebuf[sizeof(linebuf)-1])
		{
			/*
			 * Overflowed the input buffer.
			 * Pretend the line ended here.
			 * {{ The line buffer is supposed to be big
			 *    enough that this never happens. }}
			 */
			new_pos = ch_tell() - 1;
			break;
		}
		*p++ = c;
		c = ch_forw_get();
	}
	*p = '\0';
	line = linebuf;
	return (new_pos);
}

/*
 * Analogous to back_line(), but deals with "raw lines".
 * {{ This is supposed to be more efficient than back_line(). }}
 */
off_t
back_raw_line(off_t curr_pos)
{
	register char *p;
	register int c;
	off_t new_pos, ch_tell();

	if (curr_pos == NULL_POSITION || curr_pos <= (off_t)0 ||
		ch_seek(curr_pos-1))
		return (NULL_POSITION);

	p = &linebuf[sizeof(linebuf)];
	*--p = '\0';

	for (;;)
	{
		c = ch_back_get();
		if (c == '\n')
		{
			/*
			 * This is the newline ending the previous line.
			 * We have hit the beginning of the line.
			 */
			new_pos = ch_tell() + 1;
			break;
		}
		if (c == EOI)
		{
			/*
			 * We have hit the beginning of the file.
			 * This must be the first line in the file.
			 * This must, of course, be the beginning of the line.
			 */
			new_pos = (off_t)0;
			break;
		}
		if (p <= linebuf)
		{
			/*
			 * Overflowed the input buffer.
			 * Pretend the line ended here.
			 */
			new_pos = ch_tell() + 1;
			break;
		}
		*--p = c;
	}
	line = p;
	return (new_pos);
}
