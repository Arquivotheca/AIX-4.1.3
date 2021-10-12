static char sccsid[] = "@(#)20  1.11  src/bos/usr/ccs/lib/libcur/tstp.c, libcur, bos411, 9428A410j 3/7/91 17:54:48";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: tstp
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

#include        "cur99.h"
#include	<signal.h>
#ifdef SIGTSTP

/*
 * NAME:                tstp
 *
 * FUNCTION: This routine handles the "tstp" start and
 *      stop signals.  (tstp is a Berkeley(ism) and can ignored for all
 *      practical purposes.
 *
 * EXTERNAL REFERENCES: mvcur(), endwin(), fflush(), kill(), signal(),
 *                      stty(), wrefresh()
 * DATA STRUCTURES:     none
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

tstp() {
    SGTTY tty;
    tty = _tty;
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    kill(0, SIGTSTP);
    signal((int)SIGTSTP, ((void (*)(int))(int) tstp));
    _tty = tty;
#if (IS1|IS2|V7)
    stty(_tty_ch, &_tty);
#else
    Stty(_tty_ch, &_tty);
#endif
    wrefresh(curscr);
    return OK;
}
#endif
