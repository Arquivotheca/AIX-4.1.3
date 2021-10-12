static char sccsid[] = "@(#)34	1.9  src/bos/usr/ccs/lib/libcur/box.c, libcur, bos411, 9428A410j 7/17/92 15:06:58";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: superbox
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

/*
 * NAME:                supperbox
 *
 *
 * FUNCTION: This routine draws a box on the given
 *      window, with specifications for starting coordinates, depth,
 *      width, and the characters to use for the box itself.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      superbox(win, topy, topx, numl, numc, vert, hor, topl,
 *      topr, botl, botr), where 'win' is a pointer to the window,
 *      (topy,topx) are the starting coordinates, 'numl' and 'numc' are
 *      the depth and width, 'vert' is used as the vertical delimiting
 *      character, 'hor' as the horizontal one, and 'topl', 'topr',
 *      'botl', & 'botr' as the respective corners.
 *
 * EXTERNAL REFERENCES: waddch(), wmove()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

superbox(win, topy, topx, numl, numc, vert, hor, topl, topr, botl, botr)
register    WINDOW  *win;
register int    topy,
                topx,
                numl,
                numc;
NLSCHAR vert, hor, topl, topr, botl, botr;
{
    register int    i,
                    x,
                    y,
                    endy,
                    endx,
                    retval;
    char    win_scroll;

    retval = OK;		/* initialize return code */
    getyx(win, y, x);		/* set (save) x & y to the current
				   location */

    endy = topy + numl - 1;	/* find ending coordinates of box */
    endx = topx + numc - 1;
#define	IS_GRAPHIC_SUPPORTED	MB_CUR_MAX > 1
        /* Is graphic charcter supported?
	 * When MB_CUR_MAX > 1 , it means Multi byte characters
	 * Apr/19/91 
	 */
    if( IS_GRAPHIC_SUPPORTED ){
	/* All characters must be one column graphics.  If wide characters,
	 * indicate error but make reasonable substitutions and forge
	 * ahead. */
#define ADJUST(ch,sub)  if (ch > 0xff) { ch = sub; retval = ERR; }
	 ADJUST(vert,'|')
	 ADJUST(hor,'-')
         ADJUST(topl,'+')
         ADJUST(topr,'+')
         ADJUST(botl,'+')
         ADJUST(botr,'+')
    }

    for (i = topy + 1; i <= endy - 1; i++) {/* add the sides */
	if (mvwaddch(win, i, topx, vert) == ERR)
	    retval = ERR;
	if (mvwaddch(win, i, endx, vert) == ERR)
	    retval = ERR;
    }
    for (i = topx + 1; i <= endx - 1; i++) {/* add the top & bottom */
	if (mvwaddch(win, topy, i, hor) == ERR)
	    retval = ERR;
	if (mvwaddch(win, endy, i, hor) == ERR)
	    retval = ERR;
    }
				/* add the corners */
    if (mvwaddch(win, topy, topx, topl) == ERR)
	retval = ERR;
    if (mvwaddch(win, topy, topx + numc - 1, topr) == ERR)
	retval = ERR;
    if (mvwaddch(win, topy + numl - 1, topx, botl) == ERR)
	retval = ERR;
    win_scroll = win->_scroll;
    win->_scroll = FALSE;
    if (mvwaddch(win, topy + numl - 1, topx + numc - 1, botr) == ERR)
	retval = ERR;
    win->_scroll = win_scroll;

    wmove(win, y, x);		/* change the current location back to
				   what it was */
    return retval;
}

/* This function is here because we need the following capability:
 * if superbox() used from box() and fullbox() don't touch attributes
 * if superbox() used from cbox() and drawbox() use batt1 from terminfo
 * if superbox() used from cboxalt() and drawboxalt() use batt2 
 * see macros in cur01.h for defs 
 */

superbox1(win, topy, topx, numl, numc, vert, hor, topl, topr, botl, botr, attr)
register    WINDOW  *win;
register int    topy,
                topx,
                numl,
                numc;
NLSCHAR vert, hor, topl, topr, botl, botr;
ATTR	attr;
{
    ATTR    save_mode;
    short   save_flags;
    int     mode_changed = FALSE;
    int	    ret;


    if (attr)  /* batt1 or batt2 is defined */ 
    {				  
  	save_mode = win->_csbp;    /* save and restore the old one  */   
        save_flags = win->_flags;
	win->_flags |= _STANDOUT;
        win->_csbp |= attr;  /* change to new one */
        mode_changed = TRUE;
    }
    
    ret =
      superbox(win, topy, topx, numl, numc, vert, hor, topl, topr, botl, botr);

    if (mode_changed == TRUE)     /* restore */
    {
        win->_flags = save_flags;
        win->_csbp = (ATTR) save_mode;
    }
    return(ret);
}
/*** end of superbox1() ***/

