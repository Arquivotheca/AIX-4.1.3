static char sccsid[] = "@(#)66	1.4  src/bos/usr/bin/uucp/uucpadm/dmenu.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:44";
/* 
 * COMPONENT_NAME: CMDUUCP dmenu.c
 * 
 * FUNCTIONS: dmenu 
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
#include <string.h>
#include "defs.h"

extern int cury[MAXENT+1], curx[MAXENT+1];
extern char entries[MAXENT][MAXLINE];
extern WINDOW *active;

int dmenu(count,num,menu)
int count;
int num;
struct menu_shell *menu;
{
	int i;
	wattron(active,A_REVERSE);
	if (count) {
		if ((mvwprintw(active,cury[0],curx[0],"CHANGE")) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		}
	else {
		if ((mvwprintw(active,cury[0],curx[0],"  ADD ")) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		}
	wattroff(active,A_REVERSE);
	for (i=0;i < menu->counts; i++) {
		if (strcmp(entries[i],(char *) NULL) != 0) 
			if ((mvwprintw(active,cury[i+1],curx[i+1],"%s",entries[i])) == ERR)
				derror(EX_SC,"Error on write to active screen!");
		}
	if (count) {
		wattron(active,A_REVERSE);
		if ((mvwprintw(active,cury[i]+2,curx[0]-3,"Entry: %d of %d",num + 1, count)) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wattroff(active,A_REVERSE);
			}
	return(EX_OK);
}
