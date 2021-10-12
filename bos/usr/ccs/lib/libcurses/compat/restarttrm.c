static char sccsid[] = "@(#)62  1.5  src/bos/usr/ccs/lib/libcurses/compat/restarttrm.c, libcurses, bos411, 9428A410j 6/16/90 01:52:01";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   restartterm
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

#include "cursesext.h"
#include <IN/uparm.h>

extern	struct term *cur_term;

/*
 */

/*
 * NAME:        restartterm
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is useful after saving/restoring memory from a file (e.g. as
 *      in a rogue save game).  It assumes that the modes and windows are
 *      as wanted by the user, but the terminal type and baud rate may
 *      have changed.
 */

restartterm(term, filenum, errret)
char *term;
int filenum;	/* This is a UNIX file descriptor, not a stdio ptr. */
int *errret;
{
	int saveecho = SP->fl_echoit;
	int savecbreak = SP->fl_rawmode;
	int saveraw;
	int savenl;

#ifdef USG
	saveraw = (cur_term->Nttyb).c_cc[VINTR] == 0377;
	savenl = (cur_term->Nttyb).c_iflag & ICRNL;
#else
	saveraw = (cur_term->Nttyb).sg_flags | RAW;
	savenl = (cur_term->Nttyb).sg_flags & CRMOD;
#endif

	setupterm(term, filenum, errret);

	/*
	 * Restore curses settable flags, leaving other stuff alone.
	 */
	if (saveecho)
		echo();
	else
		noecho();

	if (savecbreak)
		cbreak(), noraw();
	else if (saveraw)
		nocbreak(), raw();
	else
		nocbreak(), noraw();
	
	if (savenl)
		nl();
	else
		nonl();

	reset_prog_mode();

	LINES = lines;
	COLS = columns;
}
