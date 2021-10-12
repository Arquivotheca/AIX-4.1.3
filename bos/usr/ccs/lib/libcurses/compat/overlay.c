static char sccsid[] = "@(#)45  1.7  src/bos/usr/ccs/lib/libcurses/compat/overlay.c, libcurses, bos411, 9428A410j 6/16/90 01:51:14";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   overlay
 *
 * ORIGINS: 10, 27
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
# include	<ctype.h>

# define        min(a,b)        ((a) < (b) ? (a) : (b))
# define        max(a,b)        ((a) > (b) ? (a) : (b))

/*
 * NAME:        overlay
 *
 * FUNCTION:
 *
 *      This routine writes win1 on win2 non-destructively.
 */

overlay(win1, win2)
register WINDOW	*win1, *win2; {

	register chtype	*sp, *end;
	register int    x, y, endy, endx, starty, startx, y_top,
			y_bot, x_left, x_rite, temp1, temp2, temp3;

# ifdef DEBUG
	if(outf) fprintf(outf, "OVERLAY(%0.2o, %0.2o);\n", win1, win2);
# endif
	/* calculate the amount of overlapping between the windows */
y_top  = max(win1->_begy, win2->_begy);
y_bot  = min((win1->_maxy + win1->_begy -1), (win2->_maxy + win2->_begy -1));
x_left = max(win1->_begx, win2->_begx);
x_rite = min((win1->_maxx + win1->_begx -1), (win2->_maxx + win2->_begx -1));

starty = y_top  - win1->_begy;
startx = x_left - win1->_begx;
endy   = y_bot  - win1->_begy;
endx   = x_rite - win1->_begx;

temp1  = win1->_begy - win2->_begy;     /* temp(s) added for performance*/
temp2  = x_left      - win2->_begx;

      /* for each line they have in "common" space do:  */
for (y = starty; y <= endy; y++) {
      end = &(win1->_y[y][endx]);
      sp  = &(win1->_y[y][startx]);
      temp3 = y + temp1;
	      /* for each character in common space, transfer non-blanks */
      for (x = 0; sp <= end; sp++, x++)
	      if (!isspace(*sp))
		      mvwaddch(win2, temp3, x + temp2, *sp);
      } /* end: for y = starty... */

}
