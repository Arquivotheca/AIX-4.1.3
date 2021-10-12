static char sccsid[] = "@(#)90	1.20  src/bos/usr/ccs/lib/libcur/cr_put.c, libcur, bos411, 9428A410j 10/27/93 14:38:34";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: mvcur, fgoto, plodput, plod, tabcol
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

extern char *tgoto ();		/* libtermcap.a routine which does the
				   substitution */
 /* of numeric values into the capabilities strings  */
static int  outcol,
            outline,		/* old cursor coordinates */
            destcol,
            destline;		/* new cursor coordinates */
int     plodput ();		/* counter or output routine for plod() */
extern  WINDOW  *_win;          /* shared variable from wrefresh() */

/*
 * NAME:                mvcur
 *
 * FUNCTION: This routine does the cursor movement
 *      optimizations; based on the capabilities of the terminal, this
 *      routine calculates the minimal string necessary to send to the
 *      terminal to move the cursor to the desired location (if possible).
 *
 * EXECUTION ENVIRONMENT:
 *
 *      mvcur(ly, lx, y, x), where (ly, lx) is the old location
 *      of the cursor, and (y, x) is the desired location of the cursor.
 *
 *      This routine is not simple. I have documented it as well
 *      as time permitted; however, I strongly recommend you think long
 *      and hard before you mess with this routine!  If you really do
 *      understand what is going on, then I recommend that the local
 *      motions routine (plod()), be updated to reflect some of the more
 *      sophisticated terminal capabilites (e.g. DO, LE, RI, & UP; these
 *      are the parameterized versions of cursor down, left, right, & up).
 *
 * EXTERNAL REFERENCES: tgoto(), tputs()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st), TERMCAP database info
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

mvcur(ly, lx, y, x)
register int    ly,
                lx,
                y,
                x;
{
    if (ly == y && lx == x)	/* optimization:  same location */
	return OK;
 /* make parameter values known to the rest of these routines */
    destcol = x;		/* newx */
    destline = y;		/* newy */
    outcol = lx;		/* oldx */
    outline = ly;		/* oldy */
    return(fgoto());
}



/*
 * NAME:                fgoto
 *
 * FUNCTION: Sync the position of the output cursor.
 *      Most work here is rounding for terminal boundaries getting the
 *      column position implied by wraparound or the lack thereof and
 *      rolling up the screen to get destline on the screen.
 */

fgoto() {
    register int    l,
                    c;

    if (destcol > COLS - 1) {	/* wrap until newx is on screen */
	destline += destcol / COLS;
	destcol %= COLS;
    }
    /*
     * Handle new line glitch by forcing cursor to wrap to 
     * the next line first column when automargin is set
     */
    if (XN != NULL && AM != NULL && outcol == _win->_maxx) {
	putchar('\r');
	putchar('\n');
    }        
    if (outcol >= COLS) {	/* if lastx not on screen... */
	if (AM == NULL) {	/* if no automatic wrap      */
	    outcol = COLS - 1;	/* assume cursor stopped at  */
	}			/* - screen boundry          */
	else {			/* else assume wrap around   */
	    outline += (outcol + 1) / COLS;
				/* with line increment for   */
	    outcol %= COLS;	/* - each full line past     */
	}
	if (outline > LINES - 1) {/* if lasty not on screen... */
	    destline -= outline - (LINES - 1);
	    outline = LINES - 1;
	}
    }				/* end - if past right edge  */

				/* The last part of the preceeding block */
				/* assumes that moving the cursor down  */
				/* at the last line will scroll the disp */
				/* and hence the destination line up    */
				/* This seems of questionable validity  */


    if (destline > LINES - 1) {	/* if newy is not on screen... */
	l = destline;
	destline = LINES - 1;	/* force newy onto screen */
	if (outline < LINES - 1) {/* if lasty is on screen */
	    c = destcol;
	    if (!_pfast && !CA)
		destcol = 0;
	    fgoto();		/* lets do this recursively... */
	    destcol = c;
	}
	while (l-- > LINES - 1) {
	    if (DO && (_pfast || *DO != '\n'))
		tputs(DO, 0, eciopc);
	    else {
		eciopc(NLSNL);
		if (!_pfast)
		    outcol = 0;
	    }
	}
    }
    if (destline < outline && !(CA || UP))/* if we can't move */
	destline = outline;	/* up the screen... */

    if (CA) {			/* if cursor addressable... */
    /* if it's cheaper to use local motions, then     */
    /* use local motions instead of cursor addressing */
	if (plod(strlen(tgoto(CM, destcol, destline))) > 0)
	    plod(0);
	else			/* else use cursor addressing */
	    tputs(tgoto(CM, destcol, destline), 0, eciopc);
    }
    else			/* else not cursor addressable... */
	plod(0);		/* use local motions */

    outline = destline;		/* update old y & x after we've made the
				   move */
    outcol = destcol;
    return OK;
}


static int  plodcnt,
            plodflg;

/*
 * NAME:                plodput
 */

plodput(c)
register    NLSCHAR c; {
    if (plodflg)		/* if plodflg is "set", then we are only  
				       */
	plodcnt--;		/* calculating the cost of local motions, 
				       */
    else
	eciopc(c);		/* else we are actually sending the
				   characters */
}


/*
 * NAME:                plod
 *
 * FUNCTION: Move (slowly) to destination.  (Using local motions...)
 *      Hard thing here is using home cursor on really deficient terminals.
 *      Otherwise just use cursor motions, tabs, overtabbing,
 *      and backspace.
 */

plod(cnt)			/* If (cnt == 0) we actually do the
				   motion, otherwise we are simply
				   calculating the cost; when
				   cnt is not 0, it is the cost of
				   direct cursor motion (CM); and if our
				   return value is greater than 0, then it
				   was cheaper to use local motions (and
				   we will by calling this routine again
				   with cnt == 0). */
register int    cnt;
{
    register int    i,
                    j;
    register int    soutcol,
                    soutline,
                    k;
    register    NLSCHAR c;

    plodcnt = plodflg = cnt;	/* when plodflg is "set", we are only */
				/* calculating the cost...              */

    soutcol = outcol;		/* Save these two variables just in case 
				*/
    soutline = outline;		/* we are only calculating the cost ...  
				*/
/*
 * Consider homing and moving down/right from there, vs. moving
 * directly with local motions to the right spot.
 */
    if (HO) {			/* HO (home) shouldn't be specified in
				   TERMCAP unless the terminal is really
				   deficient!  i is the (approx) cost
				   to home and tab/space to the right to
				   get to the proper column.	This
				   assumes ND space costs 1 char.  So
				   i+destcol is cost of motion with home. 
				*/
	if (GT)
	    i = (destcol / IT) + (destcol % IT);
	else
	    i = destcol;
    /* 
     * j is cost to move locally without homing
     */
	if (destcol >= outcol)	/* if motion is to the right */
	    if ((j = destcol / IT - outcol / IT) && GT)
		j += destcol % IT;
	    else
		j = destcol - outcol;
	else			/* leftward motion only works if we can
				   backspace */
	    if (outcol - destcol <= i && (BS || BC))
		i = j = outcol - destcol;/* cheaper to bs */
	else
	    j = i + 1;		/* make it more expensive */
    /* k is the absolute value of vertical distance */
	j += (k = outline - destline) < 0 ? -k : k;
    /* 
     * Decision.  We may not have a choice if no UP.
     */
	if (i + destline < j || (!UP && destline < outline)) {
	/* 
	 * Cheaper to home.  Do it now and pretend it's a
	 * regular local motion.
	 */
	    tputs(HO, 0, plodput);
	    outcol = outline = 0;
	}
	else
	    if (LL) {
	    /* 
	     * Quickly consider homing down and moving from
	     * there.  Assume cost of LL is 2.
	     */
		k = (LINES - 1) - destline;
		if (i + k + 2 < j && (k <= 0 || UP)) {
		    tputs(LL, 0, plodput);
		    outcol = 0;
		    outline = LINES - 1;
		}
	    }
    }				/* end if (HO)... */
/*
 * No home and no up means it's impossible.
 */
    else
	if (!UP && destline < outline)
	    return - 1;

    if (GT)
	i = destcol % IT + destcol / IT;
    else
	i = destcol;
    j = outcol - destcol;
/*
 * If we will later need a \n which will turn into a \r\n by
 * the system or the terminal, then don't bother to try to \r.
 */
    if ((!_pfast) && outline < destline)
	goto dontcr;
/*
 * If the terminal will do a \r\n and there isn't room for it,
 * then we can't afford a \r.
 */
    if (NC && outline >= destline)
	goto dontcr;
/*
 * If it will be cheaper, or if we can't back up, then send
 * a return preliminarily.
 */
    if (j > i + 1 || outcol > destcol && !BS && !BC) {
    /* This doesn't take the length of CR into account. */
	if (CR)
	    tputs(CR, 0, plodput);
	else
	    plodput((NLSCHAR) '\r');
	if (NC) {
	    if (DO)
		tputs(DO, 0, plodput);
	    else
		plodput((NLSCHAR) '\n');
	    outline++;
	}
	outcol = 0;
    }

dontcr: 

    while (outline < destline) {
	outline++;
	if (DO && (_pfast || *DO != '\n'))
	    tputs(DO, 0, plodput);
	else
	    plodput((NLSCHAR) '\n');
	if (plodcnt < 0)
	    goto out;
        if (!_pfast && (!DO || *DO == '\n'))
    	    outcol = 0;
    }
    while (outcol > destcol) {
	if (plodcnt < 0)
	    goto out;
	outcol--;
	if (BC)
	    tputs(BC, 0, plodput);
	else
	    plodput((NLSCHAR) '\b');
    }
    while (outline > destline) {
	outline--;
	tputs(UP, 0, plodput);
	if (plodcnt < 0)
	    goto out;
    }
    if (xRI && destcol - outcol > 3) {
	register char  *str;
	if (GT) {
	    k = outcol;
	    j = 0;
	    while ((i = tabcol(k, IT)) < destcol) {
		j += strlen(TA);
		k = i;
	    }
	    if (destcol - k > 4 && i < COLS && (BC || BS)) {
		j += strlen(TA);
		k = i;
		while (k > destcol) {
		    j += (BC ? strlen(BC) : 1);
		    k--;
		}
	    }			/* end if (destcol-outcol>4... */
	    j += destcol - k;
	}			/* end:  if (GT)... */
	else
	    j = destcol - outcol;
	if (strlen(str = tgoto(xRI, 0, destcol - outcol)) <= j) {
	    tputs(str, 0, plodput);
	    outcol = destcol;
	    if (plodcnt < 0)
		goto out;
	}
    }				/* end:  if xRI... */
    if (GT && destcol - outcol > 1) {
	while ((i = tabcol(outcol, IT)) < destcol) {
	    tputs(TA, 0, plodput);
	    outcol = i;
	}
	if (destcol - outcol > 4 && i < COLS && (BC || BS)) {
	    tputs(TA, 0, plodput);
	    outcol = i;
	    while (outcol > destcol) {
		outcol--;
		if (BC)
		    tputs(BC, 0, plodput);
		else
		    plodput((NLSCHAR) '\b');
	    }
	}			/* end if (destcol-outcol>4... */
    }				/* end if (GT && destcol - outcol > 1) ...
				   */

#define IS_MB_CHAR_SET	MB_CUR_MAX > 1
    while (outcol < destcol) {
	 /* We don't want to try to step over individual columns in SJIS;
	  * they might only be half-characters.  The case analysis isn't
	  * worth it; just move.
	  */

	 /* 
	  * Move one char to the right.  We don't automatically use ND space
	  * because it may be cheaper to just print what we are moving over.
	  */
	 if ( MB_CUR_MAX == 1 && _win != NULL) {
	      /* make sure in limits */
	      if (outline < _win->_begy || outcol < _win->_begx)
		   goto nondes;
	      if (_win->_a[outline - _win->_begy][outcol - _win->_begx] 
		  == curscr->_csbp) {
		   c = _win->_y[outline - _win->_begy][outcol - _win->_begx];
		   plodput(c);
		   outcol++;	/* increment position ptr */
	      }else
		   goto nondes; 
	 }		/* end if (_win != NULL)... */
	 else 
	      /* The following cases come here
	       *  -Single Column Code && win==NULL    or
               *  -Multi Column Code
	       */
nondes:
	      if (xRI && (destcol - outcol > 1 || !ND)) {
		   tputs(tgoto(xRI, 0, destcol - outcol), 0, plodput);
		   outcol = destcol;
	      }
	      else {
		   if (ND)
			tputs(ND, 0, plodput);
		   else
			plodput(NLSBLANK);
		   outcol++;
	      }
	 if (plodcnt < 0)
	      goto out;
    }				/* end while(outcol < destcol) */

out: 

    if (plodflg) {		/* if we were just calculating the cost,
				   reset these two */
	outcol = soutcol;
	outline = soutline;
    }
    return(plodcnt);
}



/*
 * NAME:                tabcol
 *
 * FUNCTION: Return the column number that results from being in column col
 *      and hitting a tab, where tabs are set set every 'ts' columns.  Works
 *      right for the case where col > COLS, even if 'ts' does not divide
 *      COLS.
 */

tabcol(col, ts)
register int    col,
                ts;
{
    register int    offset;

    if (col >= COLS) {
	offset = COLS * (col / COLS);
	col -= offset;
    }
    else
	offset = 0;
    return(col + ts - (col % ts) + offset);
}
