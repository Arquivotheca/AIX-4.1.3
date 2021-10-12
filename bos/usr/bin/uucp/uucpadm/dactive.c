static char sccsid[] = "@(#)61	1.3  src/bos/usr/bin/uucp/uucpadm/dactive.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:16";
/* 
 * COMPONENT_NAME: CMDUUCP dactive.c
 * 
 * FUNCTIONS: dactive 
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <curses.h>
#include "defs.h";

extern WINDOW *active;

int dactive ()
{
/* clear the screen */
	touchwin(active);
	clear();
	refresh();
	wrefresh(active);
	return(EX_OK);
}
