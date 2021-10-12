static char sccsid[] = "@(#)83	1.6  src/bos/usr/bin/more/input.c, cmdscan, bos41J, 9509A_all 2/22/95 20:58:36";
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
 * High level routines dealing with getting lines of input 
 * from the file being viewed.
 *
 * When we speak of "lines" here, we mean PRINTABLE lines;
 * lines processed with respect to the screen width.
 * We use the term "raw line" to refer to lines simply
 * delimited by newlines; not processed with respect to screen width.
 */

#define _ILS_MACROS

#include <sys/types.h>
#include <ctype.h>
#include "less.h"

wchar_t *wline;		/* Point to wbuf */
static wchar_t *wcurr;		/* Point to wbuf */
static wchar_t wbuf[LINE_MAX];	/* buffer to hold wide character string */
extern int squeeze;
extern int sigs;
extern char *line;
extern int fold_opt;
extern int mb_cur_max;
extern int sc_width;
extern int bs_mode;
extern int tabstop;
extern int bo_width, be_width;
extern int ul_width, ue_width;
extern int so_width, se_width;
extern int show_all_opt, show_opt;
static int ln_state; /* Currently in normal/underline/bold/etc mode? */

/*
 * Get the next line.
 * A "current" position is passed and a "new" position is returned.
 * The current position is the position of the first character of
 * a line.  The new position is the position of the first character
 * of the NEXT line.  The line obtained is the line starting at curr_pos.
 */
off_t
forw_line(off_t curr_pos)
{
	off_t new_pos;
	register int c;
	wchar_t wc;
	int line_len;	   /* length of input string */
	int mbclen;	   /* length of a mb char */
	int wcs_width = 0; /* display width of wc string */
	int mbs_len = 0;   /* counter for bytes added to form wline */

	if (curr_pos == NULL_POSITION || ch_seek(curr_pos))
		return (NULL_POSITION);

	c = ch_forw_get();
	if (c == EOI)
		return (NULL_POSITION);

	prewind();
	for (;;)
	{
		if (sigs)
			return (NULL_POSITION);
		if (c == '\n' || c == EOI)
		{
			/*
			 * End of the line.
			 */
			new_pos = ch_tell();
			break;
		}
		/*
		 * Append the char to the line and get the next char.
		 */
		if (pappend(c))
		{
			/*
			 * The char won't fit in the line; the line
			 * is too long to print in the screen width.
			 * End the line here.
			 */
			new_pos = ch_tell() - 1;
			break;
		}
		c = ch_forw_get();
	}
	(void) pappend('\0');

	/* 
	 *  At this point, "line" is pointing to linebuf which contains 
	 *  current line just saved by pappend() 
         */
	if (squeeze && *line == '\0')
	{
		/*
		 * This line is blank.
		 * Skip down to the last contiguous blank line
		 * and pretend it is the one which we are returning.
		 */
		while ((c = ch_forw_get()) == '\n')
			if (sigs)
				return (NULL_POSITION);
		if (c != EOI)
			(void) ch_back_get();
		new_pos = ch_tell();
	}

	/* 
	 *  if mb locale is set, convert the mb char in linebuf to wchar in
	 *  wbuf for output to wchar screen
         */
	if (mb_cur_max > 1) {
		int len;
	   	wline = wcurr = wbuf;
	   	ln_state = LN_NORMAL;
		wcs_width = 0;
	   	line_len = strlen(line);
		while (*line != '\0') {
		   	if ((mbclen=mbtowc(&wc, line, mb_cur_max)) == -1) {
				perror("more");
				raw_mode(0);
				exit (1);
		   	}
			if (fold_opt) {
			/* if this wc does not fit screen, end string here */
		   	    if ((wcs_width += (((len=wcwidth(wc))==-1)?1:len) + 
				(ln_state?ue_width:0)) > sc_width)
				break;
			}
			/* 
			 * append the wc to wbuf,  process any special mode 
			 * if needed 
			 */
			if (wc_pappend(wc, &wcs_width) > 0) 
				break;
		   	mbs_len += mbclen; /* Update length of mbchar string */	
		   	line += mbclen;    /* next multibyte char */
                } 
		if ( *line == '\0')
			/* end string and exit special mode if applicable */
			wc_pappend(L'\0', &wcs_width); 
		else {
			*wcurr = L'\0';
			/* 
		 	* line is longer than screen width, truncate and 
		 	* adjust the pointer to next line 
		 	*/
			new_pos = ch_tell() - (line_len - mbs_len + 1);	
		}
	} 
	return (new_pos);
}

/*
 * Get the previous line.
 * A "current" position is passed and a "new" position is returned.
 * The current position is the position of the first character of
 * a line.  The new position is the position of the first character
 * of the PREVIOUS line.  The line obtained is the one starting at new_pos.
 */
off_t
back_line(off_t curr_pos)
{
	off_t new_pos, begin_new_pos;
	int c;

	if (curr_pos == NULL_POSITION || curr_pos <= (off_t)0 ||
		ch_seek(curr_pos-1))
		return (NULL_POSITION);

	if (squeeze)
	{
		/*
		 * Find out if the "current" line was blank.
		 */
		(void) ch_forw_get();	/* Skip the newline */
		c = ch_forw_get();	/* First char of "current" line */
		(void) ch_back_get();	/* Restore our position */
		(void) ch_back_get();

		if (c == '\n')
		{
			/*
			 * The "current" line was blank.
			 * Skip over any preceeding blank lines,
			 * since we skipped them in forw_line().
			 */
			while ((c = ch_back_get()) == '\n')
				if (sigs)
					return (NULL_POSITION);
			if (c == EOI)
				return (NULL_POSITION);
			(void) ch_forw_get();
		}
	}

	/*
	 * Scan backwards until we hit the beginning of the line.
	 */
	for (;;)
	{
		if (sigs)
			return (NULL_POSITION);
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
			new_pos = ch_tell();
			break;
		}
	}

	/*
	 * Now scan forwards from the beginning of this line.
	 * We keep discarding "printable lines" (based on screen width)
	 * until we reach the curr_pos.
	 *
	 * {{ This algorithm is pretty inefficient if the lines
	 *    are much longer than the screen width, 
	 *    but I don't know of any better way. }}
	 */
	if (ch_seek(new_pos))
		return (NULL_POSITION);
    loop:
	begin_new_pos = new_pos;
	prewind();

	do
	{
		c = ch_forw_get();
		if (c == EOI || sigs)
			return (NULL_POSITION);
		new_pos++;
		if (c == '\n')
			break;
		if (pappend(c))
		{
			/*
			 * Got a full printable line, but we haven't
			 * reached our curr_pos yet.  Discard the line
			 * and start a new one.
			 */
			(void) pappend('\0');
			(void) ch_back_get();
			new_pos--;
			goto loop;
		}
	} while (new_pos < curr_pos);

	(void) pappend('\0');

	/* copy buffer over if in a multibyte locale */

	if (mb_cur_max > 1) {
		wchar_t wc;
		int wcs_width;
		int mbclen;
		int len;

		new_pos = begin_new_pos;

mb_loop:

	   	wline = wcurr = wbuf;
	   	ln_state = LN_NORMAL;
		wcs_width = 0;
		begin_new_pos = new_pos;

		while (*line != '\0') {
			if ((mbclen=mbtowc(&wc, line, mb_cur_max)) == -1) {
				perror("more");
				raw_mode(0);
				exit (1);
		   	}

		        if ((wcs_width += (((len=wcwidth(wc))==-1)?1:len) + 
			    (ln_state?ue_width:0)) > sc_width)
				goto mb_loop;
			/* 
			 * append the wc to wbuf,  process any special mode 
			 * if needed 
			 */
			if (wc_pappend(wc, &wcs_width) > 0) 
				goto mb_loop;

			new_pos += mbclen;
		   	line += mbclen;    /* next multibyte char */
                } 
		/* end string and exit special mode if applicable */
		wc_pappend(L'\0', &wcs_width); 
	}

	return (begin_new_pos);
}

/*
 * Append a wide character to the wline buffer.
 * Expand tabs into spaces, handle underlining, boldfacing, etc.
 * Returns 0 if ok, 1 if display width could not fit the screen width.
 */

static int
wc_pappend(wc, width)
wchar_t wc;
int *width;
{
	if (wc == L'\0') {
		/*
		 * Terminate any special modes, if necessary.
		 * Append a '\0' to the end of the line.
		 */
		switch (ln_state) {
		case LN_UL_X:
			wcurr[1] = wcurr[-1];
			wcurr[-1] = ESC_CHAR;
			wcurr[0] = UE_CHAR;
			wcurr +=2;
			break;
		case LN_BO_X:
			wcurr[1] = wcurr[-1];
			wcurr[-1] = ESC_CHAR;
			wcurr[0] = BE_CHAR;
			wcurr +=2;
			break;
		case LN_UL_XB:
		case LN_UNDERLINE:
			*wcurr++ = ESC_CHAR;
			*wcurr++ = UE_CHAR;
			break;
		case LN_BO_XB:
		case LN_BOLDFACE:
			*wcurr++ = ESC_CHAR;
			*wcurr++ = BE_CHAR;
			break;
		}
		ln_state = LN_NORMAL;
		*wcurr = L'\0';
		return(0);
	}

	if (!bs_mode) {
		/*
		 * Advance the state machine.
		 */
		switch (ln_state) {
		case LN_NORMAL:
			if (wcurr <= wbuf + 1 || 
			     wcurr[-1] != L'\b')
				break;
			if (wc == wcurr[-2])
				goto enter_boldface;
			if (wc == '_' || wcurr[-2] == '_')
				goto enter_underline;
			wcurr -= 2;
			break;

enter_boldface:
			/*
			 * We have "X\bX" (including the wcurr char).
			 * Switch into boldface mode.
			 */
			if (fold_opt) 
			   if (*width + bo_width + be_width  > sc_width)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit boldface mode.
				 */
				return (1);

			if (bo_width > 0 && wcurr > wbuf + 2
			    && wcurr[-3] == ' ') {
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter boldface" sequence.
				 */
				wcurr[-1] = wcurr[-2];
				wcurr[-2] = BO_CHAR;
				wcurr[-3] = ESC_CHAR;
				*width += bo_width-1;
				wcurr++;
			} else {
				wcurr[0] = wcurr[-2];
				wcurr[-2] = ESC_CHAR;
				wcurr[-1] = BO_CHAR;
				*width += bo_width-1;
				wcurr += 2;
			}
			goto ln_bo_xb_case;

enter_underline:
			/*
			 * We have either "_\bX" or "X\b_" (including
			 * the wcurr char).  Switch into underline mode.
			 */
			if (fold_opt) 
			    if (*width + ul_width + ue_width > sc_width)
				/*
				 * Not enough room left on the screen to 
				 * enter and exit underline mode.
				 */
				return (1);
			if (ul_width > 0 && 
			    wcurr > wbuf + 2 && wcurr[-3] == ' ')
			{
				/*
				 * Special case for magic cookie terminals:
				 * if the previous char was a space, replace 
				 * it with the "enter underline" sequence.
				 */
				wcurr[-1] = wcurr[-2];
				wcurr[-2] = UL_CHAR;
				wcurr[-3] = ESC_CHAR;
				*width += ul_width-1;
				wcurr++;
			} else
			{
			/* wcurr[-2] contains the character , save it */
				wcurr[0] = wcurr[-2]; 
				wcurr[-1] = UL_CHAR;
				wcurr[-2] = ESC_CHAR;
				*width += ul_width-1;
				wcurr +=2;
			}
			goto ln_ul_xb_case;
			/*NOTREACHED*/
		case LN_UL_XB:
			/*
			 * Termination of a sequence "_\bX" or "X\b_".
			 */
			if (wc != L'_' && wcurr[-2] != L'_' && wc == wcurr[-2])
			{
				/*
				 * We seem to have run on from underlining
				 * into boldfacing - this is a nasty fix, but
				 * until this whole routine is rewritten as a
				 * real DFA, ...  well ...
				 */
				wcurr[2] = wcurr[-2];
				wcurr[-2] = ESC_CHAR;
				wcurr[-1] = UE_CHAR;
				wcurr[0] = ESC_CHAR;
				wcurr[1] = BO_CHAR;
				wcurr += 4; /* char & non-existent backspace */
				ln_state = LN_BO_XB;
				goto ln_bo_xb_case;
			}
ln_ul_xb_case:
			if (wc == L'_')
				wc = wcurr[-2];
			wcurr -= 2;
			ln_state = LN_UNDERLINE;
			(*width)--;
			break;
		case LN_BO_XB:
			/*
			 * Termination of a sequence "X\bX".
			 */
			if (wc != wcurr[-2] && (wc == L'_' || wcurr[-2] == L'_'))
			{
				/*
				 * We seem to have run on from
				 * boldfacing into underlining.
				 */
				wcurr[2] = wcurr[-2];
				wcurr[-2] = ESC_CHAR;
				wcurr[-1] = BE_CHAR;
				wcurr[0] = ESC_CHAR;
				wcurr[1] = UL_CHAR;
				wcurr += 4; /* char & non-existent backspace */
				ln_state = LN_UL_XB;
				goto ln_ul_xb_case;
			}
ln_bo_xb_case:
			wcurr -= 2;
			ln_state = LN_BOLDFACE;
			{ int len;
			*width -= (((len=wcwidth(wc))==-1)?1:len);
			}
			break;
		case LN_UNDERLINE:
			
			if (fold_opt) 
			    if (*width + ue_width + bo_width + be_width > sc_width)
				/*
				 * We have just barely enough room to 
				 * exit underline mode and handle a possible
				 * underline/boldface run on mixup.
				 */
				return (1);
			ln_state = LN_UL_X;
			break;
		case LN_BOLDFACE:
			if (wc == L'\b')
			{
				ln_state = LN_BO_XB;
				break;
			}
			if (fold_opt) 
			    if (*width + be_width + ul_width + ue_width > sc_width)
				/*
				 * We have just barely enough room to 
				 * exit underline mode and handle a possible
				 * underline/boldface run on mixup.
				 */
				return (1);
			ln_state = LN_BO_X;
			break;
		case LN_UL_X:
			if (wc == L'\b')
				ln_state = LN_UL_XB;
			else
			{
				/*
				 * Exit underline mode.
				 * We have to shuffle the chars a bit
				 * to make this work.
				 */
				wcurr[1] = wcurr[-1];
				wcurr[-1] = ESC_CHAR;
				wcurr[0] = UE_CHAR;
				*width += ue_width;
				wcurr++;
				if (ue_width > 0 && wcurr[0] == ' ')
					/*
					 * Another special case for magic
					 * cookie terminals: if the next
					 * char is a space, replace it
					 * with the "exit underline" sequence.
					 */
					(*width)--;
				else
					wcurr++;
				ln_state = LN_NORMAL;
			} 
			break;
		case LN_BO_X:
			if (wc == L'\b') 
				ln_state = LN_BO_XB;
			else
			{
				/*
				 * Exit boldface mode.
				 * We have to shuffle the chars a bit
				 * to make this work.
				 */
				wcurr[1] = wcurr[-1];
				wcurr[-1] = ESC_CHAR;
				wcurr[0] = BE_CHAR;
				*width += be_width;
				wcurr++;
				if (be_width > 0 && wcurr[0] == ' ')
					/*
					 * Another special case for magic
					 * cookie terminals: if the next
					 * char is a space, replace it
					 * with the "exit boldface" sequence.
					 */
					(*width)--;
				else
					wcurr++;
				ln_state = LN_NORMAL;
			} 
			break;
		}
	}

	if (wc == L'\t') {
		/*
		 * Expand a tab into spaces.
		 */
		do {
			(*width)++;
		} while (!show_all_opt && (*width % tabstop) != 0);
		*wcurr++ = L'\t';
		return (0);
	}

	if (wc == L'\b') {
		if (ln_state != LN_NORMAL)
			(*width)--;
		*wcurr++ = L'\b';
		return(0);
	} 

	/*
	 * Make sure non-printable don't wrap lines.
	 * Two cases:
	 * 	ctrl-X (^X)		== 2 column
	 * 	meta-X:	(M-X)		== 3 columns
	 * 	meta-cntrl-X: (M-^X)	== 4 columns
	 * 
	 * put_line() prints them out.
	 */
	if (show_opt) {
		if (!iswprint(wc)) {
			if (iswcntrl(wc) && (isascii(wc)))
				(*width)++;	/* ^X */
			else if (iswcntrl(toascii(wc)))
				*width += 3;	/* M-^X */
			else
				*width += 2;	/* M-X */
			*wcurr++ = wc;
			return(0);
		}
	}

	/*
	 * Ordinary wc character.  Just put it in the buffer.
	 */
	*wcurr++ = wc;
	return (0);
}
