static char sccsid[] = "@(#)33	1.13  src/bos/usr/ccs/lib/libcur/makech.c, libcur, bos411, 9428A410j 6/27/92 20:20:38";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: makech, domvcur, exit_standout, fix_current
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

#include        "cur99.h"
#include	"locale.h"
#include	"limits.h"

extern short    _ly,
                _lx;
extern int fake_invis;
extern char _curwin;
extern char *tgoto ();

/*
 * NAME:                makech
 *
 * FUNCTION: This routine does the glass updating of a
 *      single line, calculating the changes which need to be made to
 *      curscr (glass image) relative to 'win', and making the changes as
 *      efficiently as it knows how.  It tries to take advantage of line
 *      oriented features available from the terminal; however, right now
 *      these include only CE & EC (clear to end-of-line & erase
 *      characters).  If EC is available, the routine ignores CE.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      makech(win, wy), where 'win' is a pointer to the window,
 *      and 'wy' is the line the update is to be made from.
 *
 * EXTERNAL REFERENCES: mvcur(), tgoto(), _puts()
 *
 * DATA STRUCTURES:     WINDOW (struct _win_st)
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

makech(win, wy)
register    WINDOW  *win;       /* win is the window refresh() was called
				   on */
register short  wy;		/* wy is the line in the window we are
				   checking */
{
    register    ATTR    *natt,
			*catt;
    register    NLSCHAR *nsp,
			*csp;
    register short  wx,
                    lch,
                    y;
    register char  *ce,
                   *ec;
    int     ce_start,
            ce_len;		/* start and size for CE        */
    int     si;			/* save start point, integer    */
    int     i,
            j;			/* work integers                */
    short   blnk_cnt = 0;
    short   cur_blnk_cnt = 0;
    struct {
	short   pos,
	        cnt;
    }       b_list[10], *blp;

    char s[MB_LEN_MAX], *sptr;
    int	 ret;

    NLSCHAR first, last; /* first and last character in a subwindow */
    int rep_first = FALSE, rep_last = FALSE; /* flags indicating that we
					    modified the first and last
					    character in the subwindow */
    int     chsize;

    wx = win->_firstch[wy];	/* wx is the position in the line we're
				   checking */
    lch = win->_lastch[wy];	/* lch is last char in line we need to
				   check */
    y = wy + win->_begy;	/* y is actual (glass) line location */

/* replace first display position in a subwindow with partial character
   indicator if it is the second byte of a two byte character */
    if ((win->_flags & _SUBWIN) && wx == 0 &&
	is_second_byte(win->_y[wy][0])) {
	rep_first = TRUE;
	first = win->_y[wy][0];
	win->_y[wy][0] = PART_CHAR;
    }

/* replace last display position in a subwindow with partial character
   indicator if it is the first byte of a two byte character */
    if ((win->_flags & _SUBWIN) && lch == win->_maxx - 1 &&
	is_first_byte (win->_y[wy][lch])) {
	rep_last = TRUE;
	last = win->_y[wy][lch];
	win->_y[wy][lch] = PART_CHAR;
    }

    if (y == LINES - 1 &&	/* if working on last line      */
	    AM &&		/* - and have automatic margin  */
	    lch + win->_begx >= COLS - 1) {
				/* - and last change is past    */
				/* - last display column        */
	    lch = COLS - 2 - win->_begx;
    }				/* obviate the need for further */
				/* checking by adjusting last   */
				/* column to be considered so as */
				/* to not include lower right   */
				/* display, hence no automatic  */
				/* (unwanted) scrolling         */

    if (_curwin) {		/* if we're refreshing curscr, then update
				   non-blanks... */
	csp = (NLSCHAR *) " ";
	catt = (ATTR *) " ";
	catt[0] = NORMAL;
    }
    else {
	csp = &(curscr->_y[y][wx + win->_begx]);
	catt = &(curscr->_a[y][wx + win->_begx]);
    }
    nsp = &(win->_y[wy][wx]);
    natt = &(win->_a[wy][wx]);

/************************************************************************/
/*                                                                      */
/*      The following logic will consider the use of EC - erase char    */
/*      and CE.  The logic is basically the following :                 */
/*                                                                      */
/*      - locate a change to blank with normal attributes               */
/*      - determine the length of the string of consecutive blanks      */
/*      - if that string extends to the end of the line in curscr       */
/*          - compare the CE cost to the length of the string and       */
/*            setup for using CE if the CE cost is less                 */
/*      - if the string does not reach the right edge of curscr         */
/*          - compare the length of the string to the EC cost           */
/*            the EC cost is the size of the EC string plus the cost    */
/*            to move the cursor past the new blank string.  Since      */
/*            the cursor will have to be moved only if there are changes*/
/*            that follow the end of the blank string, the cost of      */
/*            moving the cursor is conditional.                         */
/*          - if the comparison is favorable, save the start and length */
/*            of the blank string for later use.                        */
/*                                                                      */
/*      The logic outlined above will not always give the best possible */
/*      string for the following reasons.                               */
/*                                                                      */
/*      - if changes in the active window following the blank string    */
/*        do not require real changes (i.e. equal current contents of   */
/*        curscr) the cost of moving the cursor is still included in    */
/*        the EC cost.                                                  */
/*      - the cost for moving the cursor is not accurately calculated   */
/*        rather it is approximated by assuming that it is equal to     */
/*        the cost of the EC itself.                                    */
/*      - if the changes in the active window do not extend to the      */
/*        right edge of the real display, and the real display is       */
/*        in fact blank to the right edge, this is not recognized and   */
/*        a CE will not be considered.                                  */
/*      - there are probably other cases that have eluded me so far.    */
/*                                                                      */
/*      Except for very slow transmission rates it does not seem to     */
/*      to be worth the cycles and complexity it would cost to detect   */
/*      these cases.                                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/

    ec = NULL;			/* default ec flag to null      */
    ce = NULL;			/* default ce flag to null      */

    if (!_curwin) {		/* if not working on curscr     */
	if (EC || CE) {		/* if erase characters or clear */
				/* - to end of line available   */
	    register
		ATTR    *wap,
			*cap;   /* local pointer variables      */
	    register
		NLSCHAR *wdp,
			*cdp;   /* local pointer variables      */

				/* init pointers for data scan  */
	    wdp = nsp;		/* new window data pointer      */
	    wap = natt;		/* new window attr pointer      */
	    cdp = &(curscr->_y[y][wx + win->_begx]);
				/* curscr data pointer          */
	    cap = &(curscr->_a[y][wx + win->_begx]);
				/* curscr attr pointer          */

	    for (i = wx; i <= lch; i++) {
				/* scan until end of mod data
				   search for real differences */
		if (*wdp != *cdp || *wap != *cap) {
				/* if difference found between
				   window and curscr          */
		    if (*wdp == NLSBLANK && *wap == NORMAL) {
				/* if new data is blank/normal  */
			si = i;	/* save start point (col num)   */
			for (j = 1, i++, wdp++, wap++, cdp++, cap++;
				i <= lch && *wdp == NLSBLANK && *wap ==
								NORMAL;
				j++, i++, wdp++, wap++, cdp++, cap++) {
			    continue;
			}	/* this loop will count adjacent */
				/* blanks (in j) which are in   */
				/* the part of win marked as    */
				/* changed. keep all pointers   */
				/* together.                    */
				/* on exit j is count of blanks */
				/* and all pointers are at the  */
				/* location of either the first */
				/* non-blank or the first char  */
				/* after the changed section    */
			if (i + win->_begx == curscr->_maxx &&
				CE) {
				/* if clear to end line and ce
				*/
			    if (j > CE_cost) {
				/* if CE is cheaper than write  */
				ce_start = si;
				/* save start point             */
				ce_len = j;
				/* save length for clear        */
				ce = CE;
				/* flag ce should be used       */
			    }	/* end - CE should be used      */
			}	/* end CE could be used         */
			else
			    if (EC &&/* else if ec available and     */
				    j > EC_cost +
				/* string is longer than ec cost */
				    (i > lch ? 0 : EC_cost)) {
				/* ec cost is cost of string +  */
				/* cost to move cursor to end   */
				/* must move if changes exist   */
				/* after the blank string       */
				/* (i > lch). the cost to move  */
				/* the cursor is estimated here */
				/* by adding EC_cost again      */
				b_list[blnk_cnt].pos = si;
				/* save start position          */
				b_list[blnk_cnt].cnt = j;
				/* save blank string length     */
				if (++blnk_cnt >= 10)
				    break;
				/* if table full quit search    */
				ec = EC;
				/* set indication ec used       */
			    }	/* end ec should be used        */
		    }		/* end change to blank/normal   */
		}		/* end difference found         */
		wdp++, wap++, cdp++, cap++;
				/* step all pointers            */
				/* i steped by for-loop logic   */
				/* if change to blank found will */
				/* be stepping past nonblank at */
				/* end of the sequence          */
	    }			/* end scan for real changes    */
	}			/* end - if EC or CE avail      */
    }				/* end - if not curscr          */

/************************************************************************/
/*                                                                      */
/*      The following loop will actually update the display. The        */
/*      logic is basically the following:                               */
/*                                                                      */
/*      - locate, within the changed portion of the argument window,    */
/*        any data which differs from the contents of curscr.           */
/*      - move the cursor to that location and set the display          */
/*        attributes for the new data                                   */
/*      - if the above logic determined that EC could be used           */
/*        at this point, put out the appropriate control string         */
/*      - otherwise put out the data character(s) for the new data.     */
/*      - in either case be sure that curscr matches the new data on    */
/*        the display.                                                  */
/*                                                                      */
/*      After the update loop, if CE was indicated, do it.              */
/*                                                                      */
/************************************************************************/

    if (ce != NULL && blnk_cnt)
	    blp = b_list;
    else    blp = NULL;
    while (wx<=lch) {
	chsize = is_first_byte(*nsp) + 1;

	if (*nsp != *csp || *natt != *catt
	    || (chsize==2 && nsp[1] != csp[1]) ) {

	    fix_current(y, wx+win->_begx, *natt);
					/* erase characters?    */
	    if (blp != NULL && wx == blp->pos) {
		j = blp->cnt;
		ec = tgoto(EC, 0, j);   /* erase the screen             */
		_puts(ec)
		wx += j;
		while (j-- > 0) {       /* and blank the buffer         */
		    *csp++ = *nsp++;
		    *catt++ = *natt++;
		}
		if (++blp >= &b_list[blnk_cnt])
		    blp = NULL;         /* at end of list               */
	    } else {                    /* repaint character            */
		if (!_curwin) {         /* if not writing from curscr   */
		    *csp++ = *nsp;
		    *catt++ = *natt;

		    if (chsize==2) {
			*csp++ = nsp[1];
			*catt++ = natt[1];
		    }

		}

		if( (ret = wctomb(s, *nsp)) < 0 )
		     return ERR;

		/* if invisible and faking it, turn the characters to blanks */
		if ((*natt & INVISIBLE) && fake_invis) {
			memset(s, ' ', ret);
		}
		sptr = s;
		eciopc(*sptr++);
		nsp++;
		natt++;
		wx++;

		ret-- ; /* the first byte has been displayed */
		if (chsize==2) {
		     if( *nsp!=WEOF ){
  			  while (ret-- > 0)
			     eciopc(*sptr++); /* display as many chars as wctomb returned */
			  nsp++;
			  natt++;
			  wx++;
		     }
		}
                else /* if the char is 1 byte long but its file code contains more than 1 byte */
                     {
                     while (ret-- > 0)
                        eciopc (*sptr++) ; /* display the rest of the chars */
                     }

		_lx = wx + win->_begx;
	    }
	} else {                        /* no change here; just move    */
	    nsp += chsize;
	    natt += chsize;
	    wx += chsize;
	    if (!_curwin) {             /* if not writing from curscr   */
		    csp += chsize;
		    catt += chsize;
	    }
	}
    }

/* If we decided on an erase-to-end, set position/attribute and do it. */
    if (ce != NULL) {
	fix_current(y, ce_start+win->_begx, NORMAL);
	j = ce_start - wx;
	csp += j;
	catt += j;
	while (ce_len-- > 0) {
	    *csp++ = NLSBLANK;
	    *catt++ = NORMAL;
	}
	_puts(CE)
    }

/* restore any characters replaced by the partial character indicator */
    if (rep_first)
	win->_y[wy][0] = first;
    if (rep_last)
	win->_y[wy][lch] = last;

    return OK;
}                                       /* end makech                   */

/*
 * NAME:                domvcur
 *
 * FUNCTION: perform a mvcur, leaving standout mode if necessary.
 */

domvcur(oy, ox, ny, nx)
register int    oy,
                ox,
                ny,
                nx;
{
    if ((curscr->_flags & _STANDOUT) && !MS &&
	!(curscr->_csbp == curscr->_a[oy][ox+1])) {
	exit_standout();
    }
    mvcur(oy, ox, ny, nx);
}


/*
 * NAME:                exit_standout
 *
 * FUNCTION: leave standout mode (physically).
 */

exit_standout() {
    chg_attr_mode(curscr->_csbp, NORMAL);
				/* change attribute to normal   */
    curscr->_csbp = NORMAL;	/* update curscr structure      */
    curscr->_flags &= ~_STANDOUT;
}

/*
 * NAME:                fix_current
 *
 * FUNCTION:    Adjust current position, attribute.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      If cursor not at y,x, put it there and adjust "last" position.
 *      Then, if attribute not att, change it.
 */

static
fix_current(y,x, att)
register short  y, x;
register ATTR   att;
{
    if (_ly != y || _lx != x) {         /* need cursor adjustment?      */
	    domvcur(_ly,_lx, y,x);
	    _ly = y;
	    _lx = x;
    }
    if (att != curscr->_csbp) {         /* need attribute correction?   */
	    chg_attr_mode(curscr->_csbp, att);
	    curscr->_csbp = att;
    }
}
