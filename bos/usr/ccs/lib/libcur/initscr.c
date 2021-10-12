static char sccsid[] = "@(#)77	1.15  src/bos/usr/ccs/lib/libcur/initscr.c, libcur, bos411, 9428A410j 6/16/90 01:40:06";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: initscr
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
#include        "term.h"

#include       <signal.h>
#include       <sys/limits.h>

extern char *getenv ();

extern struct term _first_term; /* terminfo data        */
extern struct term *cur_term = &_first_term;/* ptr to terminfo data */
char   *strcpy ();

/*
 * NAME:                initscr
 *
 * FUNCTION: This routine initializes the current and
 *      standard screens (curscr & stdscr), as well as a few miscellaneous
 *      variables used in other routines.
 *
 * EXTERNAL REFERENCES: setterm(), issatty(), gettmode(), getenv(),
 *                     _puts(), signal(), delwin(), newwin(), strcpy()
 * DATA STRUCTURES:    WINDOW (struct _win_st)
 *
 * RETURNS:            normal -> (stdscr)      error -> ERR
 */

WINDOW *
initscr() {
    register char  *sp;

#ifdef SIGTSTP
    int     tstp ();
#endif

    if (isatty(2))
	_tty_ch = 2;
    else {
	for (_tty_ch = 0; _tty_ch < OPEN_MAX; _tty_ch++)
	    if (isatty(_tty_ch))
		break;
    }
    gettmode();
    if (My_term)		/* If user termid               */
	sp = Def_term;		/* set pointer to use that id   */

    else
	if ((sp = getenv("TERM")) == NULL)
				/* if environment specified id  */
	    sp = Def_term;	/* us it, else use default      */
    setterm(sp);

/* SAVE THE CURRENT TERMINAL STATE TO RESTORE ON EXIT */

    if (do_colors) {
	save_colors();
    }

 /* send out any terminal initialization sequences necessary */

    _no_line();			/* force normal line mode       */
				/* - i.e. clear dosedit mode    */
    _no_echo();			/* set stty no-echo mode        */

    if (TI && *TI)              /* if non-vtd                   */
	_tputvs(TI);            /* write initialize string      */

    if (do_cursor)
	_tputvs(VE);            /* initialize cursor shape      */
				/* (HFT opost has to be off)    */

#ifdef SIGTSTP
    signal((int)SIGTSTP, ((void (*)(int))(int) tstp));
#endif

    if (curscr != NULL)		/* if this routine has already been
				   called, or	 */
	delwin(curscr);		/* if the user has used this variable by
				   mistake */
    if ((curscr = newwin(LINES, COLS, 0, 0)) == ERR)
	return ERR;
    clearok(curscr, TRUE);	/* first refresh will cause the screen to
				   clear */

    if (stdscr != NULL)		/* if this routine has already been
				   called, or	 */
	delwin(stdscr);		/* if the user has used this variable by
				   mistake */
    if ((stdscr = newwin(LINES, COLS, 0, 0)) == ERR) {
	delwin(curscr);
	return ERR;
    }

    if (!ME && (SE || UE))
	if (SE)
	    ME = SE;
	else
	    ME = UE;

    if (!SO && US) {		/* if no standout, try underscore... */
	SO = US;
	SE = UE;
    }

    setup_attr();		/* figure out where requested/supported
				   attrs go */
    if (do_colors)              /* reset all switch attributes   */
	chg_attr_mode(~NORMAL, NORMAL);
    else _puts(ME)

/* calculate approximate cost of certain useful operations... */
    if (CE)			/* clear to end of line */
	CE_cost = strlen(CE);
    else			/* else make it very expensive (i.e.
				   impossible) */
	CE_cost = 500;

    if (EC)			/* erase characters */
	EC_cost = strlen(EC);
    else			/* else make it very expensive (i.e.
				   impossible) */
	EC_cost = 500;

    return(stdscr);
}


