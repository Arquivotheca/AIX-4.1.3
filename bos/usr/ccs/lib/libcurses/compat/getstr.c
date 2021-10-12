static char sccsid[] = "@(#)34  1.7  src/bos/usr/ccs/lib/libcurses/compat/getstr.c, libcurses, bos411, 9428A410j 10/28/90 12:38:07";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   wgetstr
 *
 * ORIGINS: 3, 10, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"

#ifdef NLS
static char conv[2];
static short convret;
#endif

/*
 * NAME:        wgetstr
 *
 * FUNCTION:
 *
 *      This routine gets a string starting at (_cury,_curx)
 */

wgetstr(win,str)
WINDOW	*win; 
char	*str;
{
	char myerase, mykill;
	char rownum[256], colnum[256];
	int doecho = SP->fl_echoit;
	int savecb = SP->fl_rawmode;
	register int cpos = 0;
	int rc = 0;
#ifdef NLS
	NLchar ch;
#else
	register int ch;
#endif
	register char *cp = str;

#ifdef DEBUG
	if (outf) fprintf(outf, "doecho %d, savecb %d\n", doecho, savecb);
#endif

	myerase = erasechar();
	mykill = killchar();
	noecho(); crmode();

	for (;;) {
		rownum[cpos] = win->_cury;
		colnum[cpos] = win->_curx;
		if (! (win->_flags&_ISPAD))
			wrefresh(win);
#ifdef NLS
		conv[0] = rc = wgetch(win);
		if ((int) rc == ERR) break;

		if (NCisshift(conv[0])) {
			conv[1] = rc = wgetch(win);
			if ((int) rc == ERR) break;
		}
		convret = NCdecode(conv, &ch);
#else
		ch = wgetch(win);
#endif
		if ((int) ch <= 0 || (int) ch == ERR ||
					ch == '\n' || ch == '\r')
			break;
		if (ch == myerase || ch == KEY_LEFT || ch == KEY_BACKSPACE) {
			if (cpos > 0) {
				cp--; cpos--;
				if (doecho) {
					wmove(win, rownum[cpos],
							colnum[cpos]);
					wclrtoeol(win);
				}
			}
		} else if (ch == mykill) {
			cp = str;
			cpos = 0;
			if (doecho) {
				wmove(win, rownum[cpos], colnum[cpos]);
				wclrtoeol(win);
			}
		} else {
#ifdef NLS
			*cp++ = conv[0];
			if (convret == 2)
				*cp++ = conv[1];
#else
			*cp++ = ch;
#endif
			cpos++;

			if (doecho) {
#ifdef NLS
				waddch(win, conv[0]);
				if (convret == 2)
					waddch(win, conv[1]);
#else
				waddch(win, ch);
#endif
			}
		}
	}

	*cp = '\0';

	if (doecho)
		echo();
	if (!savecb)
		nocrmode();
	waddch(win, '\n');
	if (win->_flags & _ISPAD);
		wrefresh(win);
	if ((int) ch == ERR || (int) rc == ERR)
		return ERR;
	return OK;
}
