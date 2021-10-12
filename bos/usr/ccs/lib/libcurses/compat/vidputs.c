static char sccsid[] = "@(#)12  1.2  src/bos/usr/ccs/lib/libcurses/compat/vidputs.c, libcurses, bos411, 9428A410j 10/29/93 17:49:51";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS:   vidputs
 *
 * ORIGINS: 3, 10, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

static int oldmode = 0;	/* This really should be in the struct term */
char *tparm();

/* nooff: modes that don't have an explicit "turn me off" capability */
#define nooff	(A_PROTECT|A_INVIS|A_BOLD|A_DIM|A_BLINK|A_REVERSE)
/* hilite: modes that could be faked with standout in a pinch. */
#define hilite	(A_UNDERLINE|A_BOLD|A_DIM|A_BLINK|A_REVERSE)

/*
 * NAME:        vidputs
 */

vidputs(newmode, outc)
int newmode;
int (*outc)();
{
	int curmode = oldmode;

#ifdef DEBUG
	if (outf) fprintf(outf,
		"vidputs oldmode=%o, newmode=%o\n",oldmode,newmode);
#endif

	if (newmode || !exit_attribute_mode) {
		if (set_attributes) {
			int	hi;
			if(hi=newmode&hilite)
				{newmode=(newmode & ~hilite);
				 if(A_UNDERLINE & hi)
					 newmode|=(enter_underline_mode ?
							A_UNDERLINE : A_STANDOUT);
				 if(A_BOLD & hi)
					 newmode|=(enter_bold_mode ?
							A_BOLD : A_STANDOUT);
				 if(A_DIM & hi)
					 newmode|=(enter_dim_mode ?
							A_DIM : A_STANDOUT);
				 if(A_BLINK & hi)
					 newmode|=(enter_blink_mode ?
							A_BLINK : A_STANDOUT);
				 if(A_REVERSE & hi)
					 newmode|=(enter_reverse_mode ?
							A_REVERSE : A_STANDOUT);
				}
			tputs(tparm(set_attributes,
					newmode & A_STANDOUT,
					newmode & A_UNDERLINE,
					newmode & A_REVERSE,
					newmode & A_BLINK,
					newmode & A_DIM,
					newmode & A_BOLD,
					newmode & A_INVIS,
					newmode & A_PROTECT,
					newmode & A_ALTCHARSET),
				1, outc);
			curmode = newmode;
		} else {
			if (((oldmode&nooff) > (newmode&nooff)) || 
					ceol_standout_glitch) {
				if (exit_attribute_mode) {
					tputs(exit_attribute_mode, 1, outc);
				/* } else if (oldmode == A_UNDERLINE &&
							exit_underline_mode) {
					tputs(exit_underline_mode, 1, outc); */
				} else if (exit_standout_mode) {
					tputs(exit_standout_mode, 1, outc);
				}
				curmode = oldmode = 0;
			}
			if (!(newmode&A_ALTCHARSET) && (oldmode&A_ALTCHARSET)) {
				tputs(exit_alt_charset_mode, 1, outc);
				curmode &= ~A_ALTCHARSET;
			}
			if (!(newmode&A_UNDERLINE) && (oldmode&A_UNDERLINE)) {
				tputs(exit_underline_mode, 1, outc);
				curmode &= ~A_UNDERLINE;
			}
			if (!(newmode&A_STANDOUT) && (oldmode&A_STANDOUT)) {
				tputs(exit_standout_mode, 1, outc);
				curmode &= ~A_STANDOUT;
			}
			if ((newmode&A_ALTCHARSET) && !(oldmode&A_ALTCHARSET)) {
				tputs(enter_alt_charset_mode, 1, outc);
				curmode |= A_ALTCHARSET;
			}
			if ((newmode&A_PROTECT) && !(oldmode&A_PROTECT)) {
				tputs(enter_protected_mode, 1, outc);
				curmode |= A_PROTECT;
			}
			if ((newmode&A_INVIS) && !(oldmode&A_INVIS)) {
				tputs(enter_secure_mode, 1, outc);
				curmode |= A_INVIS;
			}
			if ((newmode&A_BOLD) && !(oldmode&A_BOLD))
				if (enter_bold_mode) {
					curmode |= A_BOLD;
					tputs(enter_bold_mode, 1, outc);
				}
			if ((newmode&A_DIM) && !(oldmode&A_DIM)) 
				if (enter_dim_mode) {
					curmode |= A_DIM;
					tputs(enter_dim_mode, 1, outc);
				}
			if ((newmode&A_BLINK) && !(oldmode&A_BLINK)) 
				if (enter_blink_mode) {
					curmode |= A_BLINK;
					tputs(enter_blink_mode, 1, outc);
				}
			if ((newmode&A_REVERSE) && !(oldmode&A_REVERSE)) 
				if (enter_reverse_mode) {
					curmode |= A_REVERSE;
					tputs(enter_reverse_mode, 1, outc);
				}
			if ((newmode&A_UNDERLINE) && !(oldmode&A_UNDERLINE)) 
				if (enter_underline_mode) {
					curmode |= A_UNDERLINE;
					tputs(enter_underline_mode,1,outc);
				}
			if ((newmode&A_STANDOUT) && !(oldmode&A_STANDOUT)) 
				if (enter_standout_mode) {
					curmode |= A_STANDOUT;
					tputs(enter_standout_mode,1,outc);
				}
		}
	} else {
		if (exit_attribute_mode)
			tputs(exit_attribute_mode, 1, outc);
		else if (oldmode == A_UNDERLINE && exit_underline_mode)
			tputs(exit_underline_mode, 1, outc);
		else if (exit_standout_mode)
			tputs(exit_standout_mode, 1, outc);
		curmode = 0;
	}
	/*
	 * If we asked for bold, say, on a terminal with only standout,
	 * and we aren't already in standout, we settle for standout.
	 */
	if ((newmode&hilite) && curmode!=newmode && (curmode&A_STANDOUT)==0) {
		tputs(enter_standout_mode, 1, outc);
		curmode |= A_STANDOUT;
	}
	oldmode = curmode;
}
