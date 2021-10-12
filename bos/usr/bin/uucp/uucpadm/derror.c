static char sccsid[] = "@(#)63	1.4  src/bos/usr/bin/uucp/uucpadm/derror.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:24";
/* 
 * COMPONENT_NAME: CMDUUCP derror.c
 * 
 * FUNCTIONS: derror 
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
#include <sys/stat.h>
#include "defs.h";

extern struct files Rules[FILES];
/*void perror();*/
void exit();

int derror (iexit,imsg)
int iexit;
char *imsg;
{
 /* clear the screen */
	erase();
	if ((move(LINES/2,0)) == ERR) {
		fprintf (stderr, "Uucpadm: Failed to write error %s to stdscr.", imsg);
		perror(" ");
		}
		
 /* highlight the presentation */
	attron(A_REVERSE);
	if (printw("Uucpadm: %s \n",imsg) == ERR) {
	fprintf (stderr, "Uucpadm: Failed to write error %s to stdscr.", imsg);
	perror(" ");
	endwin();
	exit (EX_SC);
	}
	attroff(A_REVERSE);
	refresh();
	endwin();
	exit (iexit);
}
