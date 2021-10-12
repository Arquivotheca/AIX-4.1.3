static char sccsid[] = "@(#)65	1.3  src/bos/usr/bin/uucp/uucpadm/dhelp.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:34";
/* 
 * COMPONENT_NAME: CMDUUCP dhelp.c
 * 
 * FUNCTIONS: dhelp 
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

extern WINDOW *helpwin;
extern WINDOW *active;

int derror();

int dhelp ()
{
 /* clear the screen */
	touchwin(helpwin);
	clear();
	refresh();
	wrefresh(helpwin);
	touchwin(active);
	return(EX_OK);
}
