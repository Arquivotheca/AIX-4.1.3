static char sccsid[] = "@(#)48	1.3  src/bos/usr/bin/uucp/uucpadm/bbase.c, cmduucp, bos411, 9428A410j 8/3/93 16:13:17";
/* 
 * COMPONENT_NAME: CMDUUCP bbase.c
 * 
 * FUNCTIONS: bbase 
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
/*
 *  uucpadm
 *
 *  Copyright (c) 1988 IBM Corporation.  All rights reserved.
 *
 *
 */


#include <curses.h>
#include <string.h>
#include "defs.h"

extern int cury[MAXENT+1], curx[MAXENT+1];
extern WINDOW *active;

int derror();

int bbase (menu)
struct menu_shell  *menu;
{
	int  num;
	int  i;
	int  center, line;
/* We start at two, leaving room for border. */
	line = 2;

/* clear the active menu. */
	werase(active);

 /*   First, center. */

	if ((center = (COLS - REQ) / 2) < 0)
		center = 0;
	num = 0;

/*    Write out menu           */
	if ((mvwprintw (active,line,(COLS - strlen(menu->header)) / 2, "%s", menu->header)) == ERR)
		derror(EX_SC,"Error on write to active screen!");
		cury[num] = active ->_cury + 1;
		curx[num] = active ->_curx - ((strlen(menu->header)+6) / 2);
	line += 3;
	for (num=0; num < menu->counts; num++)
	{
	if ((mvwprintw (active,line,center + 3, "%s",menu->entry[num])) == ERR)
		derror(EX_SC,"Error on write to active screen!");
		curx[num+1] = active ->_curx;
		cury[num+1] = active ->_cury;
	line += 2;
	}

	line++;
	/* border the active menu */

	wattron(active,A_REVERSE);
	for(i=center; i <= REQ + center; i++) {
		if ((mvwaddch(active,1,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		if ((mvwaddch(active,line,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
	}
	for(i=1; i <= line; i++) {
		if ((mvwaddch(active,i,center,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		if ((mvwaddch(active,i,center + REQ,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
	}
	for(i=center; i < center + ((REQ - strlen(PANEMSG1)) / 2); i++) {
		if ((mvwaddch(active,line,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		}
	if ((mvwprintw (active,line,center + ((REQ - strlen(PANEMSG1)) /2), "%s", PANEMSG1)) == ERR)
		derror(EX_SC,"Error on write to active screen!");
	for(i=active -> _curx; i <= center + REQ; i++) {
		if ((mvwaddch(active,line,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		}
	line++;
	for(i=center; i < center + ((REQ - strlen(PANEMSG2)) / 2); i++) {
		if ((mvwaddch(active,line,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		}
	if ((mvwprintw (active,line,center + ((REQ - strlen(PANEMSG2)) /2), "%s", PANEMSG2)) == ERR)
		derror(EX_SC,"Error on write to active screen!");
	for(i=active -> _curx; i <= center + REQ; i++) {
		if ((mvwaddch(active,line,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		}
	wattroff(active,A_REVERSE);
	return(EX_OK);
}
