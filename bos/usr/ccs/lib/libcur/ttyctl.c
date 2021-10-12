static char sccsid[] = "@(#)31  1.19  src/bos/usr/ccs/lib/libcur/ttyctl.c, libcur, bos411, 9428A410j 3/16/91 02:44:44";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: raw, noraw, crmode, nocrmode, echo, noecho, _no_echo, nl,
 *            nonl, _no_line, mata, nomata, nodelay, savetty, resetty,
 *            csavetty, cresetty
 *
 * ORIGINS: 3, 10, 27
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

#include	"fcntl.h"
#include        "cur02.h"

#if !(IS1|IS2|V7)
#include        "cur99.h"

/*
 * NAME:                raw, noraw, crmode, nocrmode, echo,
 *                      noecho, nl, nonl
 *
 * FUNCTION:    These routines are used to control the
 *      terminal modes using either "stty" or "ioctl" system routines.
 *
 * EXTERNAL REFERENCES: Stty()
 *
 * RETURNS:              normal -> OK            error -> ERR
 */

#define rdfdes 0

raw() {
    _tty.c_iflag &= ~(INLCR | ICRNL | IUCLC);
    _tty.c_oflag &= ~(OPOST);
    _tty.c_lflag &= ~(ISIG | ICANON);
    _tty.c_cc[VMIN] = 1;
    _tty.c_cc[VTIME] = 1;
    _pfast = _rawmode = TRUE;
    Stty(_tty_ch, &_tty);
}

noraw() {
    _tty.c_iflag |= _res_flg.c_iflag & (INLCR | ICRNL | IUCLC);
    _tty.c_oflag |= OPOST;
    _tty.c_lflag |= _res_flg.c_lflag & (ISIG | ICANON);
    _tty.c_cc[VEOF] = _res_flg.c_cc[VEOF];
    _tty.c_cc[VEOL] = _res_flg.c_cc[VEOL];
    _pfast = _rawmode = FALSE;
    Stty(_tty_ch, &_tty);
}

crmode() {
    _tty.c_lflag &= ~(ICANON);
    _tty.c_cc[VMIN] = 1;
    _tty.c_cc[VTIME] = 1;
    _rawmode = TRUE;
    Stty(_tty_ch, &_tty);
}

nocrmode() {
    _tty.c_lflag |= (ICANON);
    _tty.c_cc[VEOF] = _res_flg.c_cc[VEOF];
    _tty.c_cc[VEOL] = _res_flg.c_cc[VEOL];
    _rawmode = FALSE;
    Stty(_tty_ch, &_tty);
}

echo() {			/* control echo by getch        */
    _echoit = TRUE;
}

noecho() {			/* turn off echo by getch       */
    _echoit = FALSE;
}

_no_echo() {			/* turn off stty echo mode      */
    _tty.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    Stty(_tty_ch, &_tty);
}

nl() {
    _tty.c_iflag |= _res_flg.c_iflag & (INLCR | ICRNL | IGNCR);
    _tty.c_oflag |= OPOST;
    _pfast = FALSE;
    Stty(_tty_ch, &_tty);
}

nonl() {
    _tty.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    _tty.c_oflag &= ~(OPOST);
    _pfast = TRUE;
    Stty(_tty_ch, &_tty);
}
#endif

/*
 * NAME:                _no_line
 *
 * FUNCTION:    Clear the line mode field in the terminal interface
 *      used to control the dosedit mode for the terminal.
 *      which does bad things to this package
 */

_no_line() {
    _tty.c_line = 0;		/* clear the line mode          */
    Stty(_tty_ch, &_tty);	/* update the interface         */
}				/* end function _no_line        */

/*
 * NAME:                mata
 *
 * FUNCTION:    Allow full 8 bit input.
 */

meta() {
    if (KM) {			/* no action if not in termcap	 */
	_tty.c_iflag &= ~(ISTRIP);/* clear strip to 7 bits flag	 */
	Stty(_tty_ch, &_tty);	/* update processing in kernel	 */
	_puts(MM)               /* if needed send enable string  */
    }
}				/* end meta function		 */

/*
 * NAME:                nomata
 *
 * FUNCTION:    Suppress full 8 bit input.
 */

nometa() {
    if (KM) {			/* no action if not in termcap	 */
	_tty.c_iflag |= (_res_flg.c_iflag & ISTRIP);
    /* reset original strip status	 */
	Stty(_tty_ch, &_tty);	/* update kernel flags		 */
	_puts(MO)               /* if needed send disable string */
    }
}				/* end of nometa function	 */


/*
 * NAME:                nodelay
 *
 * FUNCTION: The nodelay flag is now just a software curses concept
 *      which is implemented in _ndelay_read.  This prevents making
 *      stdout nodelay when stdin is made nodelay.  If stdout is made
 *      nodelay, then output is trashed on slow terminals.
 */

nodelay(delayfg)		/* set nodelay mode		 */
char    delayfg;		/* argument is boolean		 */
{

    _nodelay = delayfg;
    return OK;
}

/*
 * NAME:                savetty
 *
 * FUNCTION:    Save application tty state.
 */

savetty() {			/* save tty state, no parms     */
    Gtty(_tty_ch, &_tty);	/* save the current tty state   */
    Gtty(_tty_ch, &_res_flg);	/* save copy for restore        */
    _fcntl = fcntl(rdfdes, F_GETFL, 0);/* save the nodelay flag also   */
}				/* end function savetty         */


/*
 * NAME:                resetty
 *
 * FUNCTION:    Reset application tty state
 *      parm is boolean, true ==> clear screen also
 */

resetty(clr)			/* reset application tty state  */
char    clr;			/* clear option flag            */

{
    if (curscr->_csbp != NORMAL) {/* if not in normal attr state*/
	chg_attr_mode(curscr->_csbp, NORMAL);
				/* change attribute state       */
	curscr->_csbp = NORMAL;	/* reflect in curscr also       */
    }

    if (clr)                    /* if clear requested           */
	_puts(CL)               /* write clear screen           */

    if (do_cursor)
	_tputvs(VE);            /* set normal cursor            */

    _tputvs(TE);		/* write terminal reset string  */

    eciofl(stdout);		/* flush any pending output     */

    flttyin();			/* flush any pending input      */

    Stty(_tty_ch, &_res_flg);	/* reset term attributes        */
    fcntl(rdfdes, F_SETFL, _fcntl);/* reset nodelay mode        */
}				/* end function resetty         */

/*
 * NAME:                csavetty
 *
 * FUNCTION:    Save curses tty state
 *      parm is boolean, true ==> set curses modes before save
 */

csavetty(setc)			/* save curses tty state        */
char    setc;                   /* initialize curses modes first*/

{

    if (setc) {			/* if initialize requested      */
	trackloc(_trackloc);    /* take care of mouse, etc.     */
	crmode();		/* turn off canonical processing */
	noecho();		/* turn off echo of input       */
	meta();			/* enable all input             */
	nonl();			/* turn off input/output transl */
	keypad(TRUE);		/* enable input mapping         */
    }

    Gtty(_tty_ch, &_ctty);	/* save curses term state       */
    _cfcntl = fcntl(rdfdes, F_GETFL, 0);
				/* save nodelay state           */
}				/* end csavetty                 */

/*
 * NAME:                cresetty
 *
 * FUNCTION:    Reset curses tty state
 *      parm is boolean, true ==> refresh display from curscr also
 */

cresetty(rshw)			/* reset curses tty state       */
char    rshw;			/* reshow from curscr ?         */

{
    trackloc (_trackloc);       /* take care of mouse, etc.     */
				/* (HFT opost has to be off)    */
    Stty(_tty_ch, &_ctty);	/* reset tty modes              */
    fcntl(rdfdes, F_SETFL, _cfcntl);/* reset nodelay flag       */

    if (TI && *TI)              /* if non-vtd                   */
	_tputvs(TI);            /* write initialize string      */

    if (curscr->_csbp != NORMAL) {/* if not in normal attr state*/
	chg_attr_mode(curscr->_csbp, NORMAL);
				/* change attribute state       */
	curscr->_csbp = NORMAL;	/* reflect in curscr also       */
    }

    if (rshw) {			/* if request to reshow         */
	clearok(curscr, TRUE);	/* allow for clear of curscr    */
	wrefresh(curscr);	/* refresh the data to the disp */
    }

}				/* end cresetty                 */

