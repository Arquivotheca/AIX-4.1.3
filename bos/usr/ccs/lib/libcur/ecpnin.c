static char sccsid[] = "@(#)44  1.25  src/bos/usr/ccs/lib/libcur/ecpnin.c, libcur, bos411, 9428A410j 10/22/93 13:18:40";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: ecpnin ecpninrf, ecpninli, trackloc, 
 *           save_colors, restore_colors
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"
#include        "cur05.h"
#include        "cur02.h"
#include        "term.h"

int     ecrfpn ();
int     ecpspn ();
int     nodelay ();

char    ECINSMD = FALSE;	/* Insert mode toggle with deflt */

extern
char    _mtkeybf;		/* flag - input buffer empty    */


static
int     last_loc = 0;		/* previous locator case        */
				/* - true if last locator input */
				/* - was scroll                 */

/*
 * NAME:                ecpnin
 *
 * FUNCTION:            Get the next input character from the pane,
 *                      assure cursor visible and data updated.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pn - pointer to pane for which input is
 *                           to be accepted.
 *                      trkcsr - boolean - TRUE implies track cursor
 *                           FALSE implies return cursor inputs
 *                      fcd - integer first code to be treated as
 *                           if it came from keyboard, if null not
 *                           to be used.
 *
 *   INITIAL CONDITIONS Keypad must be active, the pane passed must
 *                      be part of the top panel
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          locator movement tracked, cursor movement
 *                      tracked if trkcsr is TRUE.
 *
 *     ABNORMAL:        n/a
 *
 * EXTERNAL REFERENCES: wgetch(), eclocst(), eclocmv()
 *
 * RETURNED VALUES:     keycode for input received, if trkcsr is TRUE
 *                      this will not include cursor movement keys
 */

int     ecpnin (pn, trkcsr, fcd)/* function, two parms          */
register
	    PANE    *pn;        /* pointer to pane struct        */
char    trkcsr;			/* flag - true track cursor	 */
				/*       false - return cursor */
int     fcd;			/* first keyboard code          */
				/* if null ignore code          */
{

    register
    int     incd;		/* input data code integer	 */

    char    olddly;		/* nodelay flag on entry	 */

    int     cdx;		/* cursor delta horizontal	 */
    int     cdy;		/* - vertical (character units) */

    int     sdx;		/* scroll delta (scroll key     */
    int     sdy;		/* vertical scroll              */

    register
		WINDOW  *pw;    /* pointer to window for pane   */

    union			/* union used to convert signed */
    {				/* - short data that is not     */
	char    c[2];		/* - aligned to numeric value   */
	short int   i;
    } ciu;


    char    locflg;		/* copy of locator flags        */

    int     rlmt;		/* read limit counter to control */
#define lmtmx 5			/* - retries for double loc butn */

/*      define displacements into ESCSTR for locator access             */

#define locflgsc        0x40   /* - scroll flag (delta dev)    */
#define locflgsl        0x80   /* - select flag (delta dev)    */
#define locabsky        0xf8   /* - all button mask            */
#define locabson        0x04   /* - on tablet flag             */
#define locabssl        0x08+locabson/* - select button (tablet)     */
#define locabssc        0x10+locabson/* - scroll button (tablet)     */
#define locabsct        0x18+locabson/* - cmd term button (tablet)   */
#define locabsso    locabssc-locabson/* - scroll and off tablet      */

    pw = pn->w_win;

    do {			/* repeat read while processing */
				/* - results in no data to ret. */
				/* - read ahead in cursor and        */
				/* - locator can be expected to */
				/* - result in KEY_NOKEY and         */
				/* - must compensate.                */
	if (fcd == NULL) {	/* if first code is null        */
	    incd = wgetch(pw);	/* get next input code          */
	}
	else {
	    incd = fcd;		/* use first code as input code */
	    fcd = NULL;		/* ignore it on following reads */
	}

	while (TRUE) {		/* stay in following loop	 */
				/* - until explicit exit, i.e.   */
				/* - while input continues to be */
				/* - cursor or locator input     */

/*	process input while it is cursor movement			*/


	    if (trkcsr &&	/* if track cursor true and	 */
		    (incd == KEY_UP ||/* - input is a cursor movement */
			incd == KEY_DOWN ||
			incd == KEY_LEFT ||
			incd == KEY_RIGHT ||
			incd == KEY_SCR ||
			incd == KEY_SCL ||
			incd == KEY_SF ||
			incd == KEY_SR)) {
		cdx = cdy = 0;	/* initialize delta value	 */
		sdx = sdy = 0;

		do {		/* repeat while we get cursor	 */
		    switch (incd) {/* accumulate cursor delta	 */
			case KEY_UP: 
				/* UP - Decrement row		 */
			    cdy--;
			    break;
			case KEY_DOWN: /* DOWN - Increment row 	 */
			    cdy++;
			    break;
			case KEY_LEFT: 
				/* LEFT - Decrement column	 */
			    cdx--;
			    break;
			case KEY_RIGHT: 
				/* RIGHT - Increment column	 */
			    cdx++;
			    break;
			case KEY_SCR: /* scroll right increment col   */
			    sdx += 5;/* adjust scroll                */
			    cdx += 5;
			    break;
			case KEY_SCL: /* scroll left  decrement col   */
			    sdx -= 5;/* adjust scroll                */
			    cdx -= 5;
			    break;
			case KEY_SF: /* scroll forward incr row      */
			    sdy += 5;/* adjust scroll                */
			    cdy += 5;
			    break;
			case KEY_SR: /* scroll forward incr row      */
			    sdy -= 5;/* adjust scroll                */
			    cdy -= 5;
			    break;
		    }		/* end switch for cursor accum. */

		    if (_mtkeybf) {/* if keyboard buffer is empty  */
			incd = KEY_NOKEY;
				/* force null key code          */
		    }
		    else {
			incd = wgetch(pw);
				/* get next code from buffer    */
		    }

		} while (incd == KEY_UP ||
				/* continue to loop while cursor */
			incd == KEY_DOWN ||
			incd == KEY_LEFT ||
			incd == KEY_RIGHT ||
			incd == KEY_SCR ||
			incd == KEY_SCL ||
			incd == KEY_SF ||
			incd == KEY_SR);

		if ((sdx != 0) || (sdy != 0)) {
		    ecscpn(pn, sdy, sdx);
				/* scroll if needed             */
		}

		ecpninrf(pn, pw, cdy, cdx);
				/* go refresh the display       */
		last_loc &= ~locflgsc;/* force a break in consecutive */
				/* - locator scroll requests    */

		}		/* end - process locator input  */

/************************************************************************/
/*      process global keys not cursor or locator (insert mode, etc)    */
/************************************************************************/

	    else
		if (incd == KEY_IC || incd == KEY_EIC) {
				/* if input is insert mode      */
		    ECINSMD = ~ECINSMD;
				/* toggle global insert mode flg */

		    if (ECINSMD) {
			_tputvs(VS);
		    }		/* set alternat cursor shape    */
		    else {
			_tputvs(VE);
		    }		/* set normal cursor shape      */


		    incd = KEY_NOKEY;/* force another read           */
		    last_loc &= ~locflgsc;
				/* force a break in consecutive */
				/* - locator scroll requests    */
		}

	    else
		break;		/* not locator, cursor or insert */
				/* - exit from loop              */
	}			/* end cursor/locator loop	 */

    } while (incd == KEY_NOKEY &&/* end read until data to return */
	    _nodelay == FALSE);

    last_loc &= ~locflgsc;	/* if returning consider any    */
				/* - locator scroll as new      */

    return(incd);		/* return code read             */

}				/* end function ecpnin		 */

/*
 * NAME:                ecpninrf
 *
 * FUNCTION: local function ecpninrf, move the cursor and refresh the
 *      current panel.
 */

ecpninrf(pn, pw, dy, dx)	/* function definition          */
register
	    PANE    *pn;        /* current pane                 */
register
	    WINDOW  *pw;        /* p-space for pane             */
int     dy,
        dx;			/* movement distance for cursor */

{				/* begin function body          */

    pw->_cury += dy;		/* calculate new row coordinate */
    if (pw->_cury < 0)		/* if out of bounds set back to */
	pw->_cury = 0;		/* - valid range                */
    if (pw->_cury >= pw->_maxy)
	pw->_cury = pw->_maxy - 1;

    pw->_curx += dx;		/* calculate new col coordinate */
    if (pw->_curx < 0)		/* if out of bounds set back to */
	pw->_curx = 0;		/* - valid range                */
    if (pw->_curx >= pw->_maxx)
	pw->_curx = pw->_maxx - 1;

    ecpspn(pn);			/* adjust the view position     */

    ecrfpn(pn);			/* refresh just the pane        */
}				/* end - local function         */



/*
 * NAME:                trackloc
 *
 * FUNCTION:  _trackloc is set to reflect argument, will in
 *      turn affect locator processing in ecpnin().
 */

trackloc(keysw)                 /* function definition          */
char    keysw;			/* argument is boolean variable */

{

    _trackloc = keysw;		/* set global switch            */

    if (!TI || *TI) return (OK);/* non-VTD.                     */
    if (keysw) {                /* take care of mouse, etc.     */
	raw();
	meta();
	_tputvs (TI);
    } else
	_tputvs (TE);

    return(OK);			/* return to caller             */
}				/* end of function              */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/termio.h>

#undef ioctl

typedef int (*FUNC)(int);		/* pointer to a function        */

#ifdef OLD_STYLE /* by Tz Apr/23/91 */
char   *malloc ();
#endif

extern int  errno;


/*
 * NAME:                save_colors
 *
 * FUNCTION: save the terminal colors.
 */

save_colors() {
    char   *p,
           *end;

    return (0);                 /* No-op, due to VTD breaks rlogin.     */
}

/*
 * NAME:                restore_colors
 *
 * FUNCTION: restore the terminal colors.
 */

restore_colors() {

    if (CF[2])
	if (eciowr(1, CF[2], strlen (CF[2])) < 0)
	    return(-1);

    if (CB[0])
	if (eciowr(1, CB[0], strlen (CB[0])) < 0)
	    return(-1);

    return (0);

}
