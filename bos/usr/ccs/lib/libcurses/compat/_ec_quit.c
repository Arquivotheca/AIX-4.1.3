static char sccsid[] = "@(#)90	1.6  src/bos/usr/ccs/lib/libcurses/compat/_ec_quit.c, libcurses, bos411, 9428A410j 6/16/90 01:43:15";
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _ec_quit
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

/*
 * NAME:        _ec_quit
 *
 * FUNCTION:
 *
 *      Emergency quit.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Called at startup only if something wrong in
 *      initializing termcap.
 */

#ifndef 	NONSTANDARD

_ec_quit(msg, parm)
char *msg, *parm;
{
#ifdef DEBUG
	if(outf) fprintf(outf, "_ec_quit(%s,%s).\n", msg, parm);
#endif
	reset_shell_mode();
	fprintf(stderr, msg, parm);
	exit(1);
}
#endif	 	NONSTANDARD
