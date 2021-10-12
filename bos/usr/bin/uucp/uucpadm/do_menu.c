static char sccsid[] = "@(#)69	1.4  src/bos/usr/bin/uucp/uucpadm/do_menu.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:55";
/* 
 * COMPONENT_NAME: CMDUUCP do_menu.c
 * 
 * FUNCTIONS: do_menu 
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
 *  Copyright (c) 1987 IBM Corporation.  ll rights reserved.
 *
 *
 */

/*
 *  do_menu - perform a menu operation
 */

#include <curses.h>
#include "defs.h"

extern WINDOW *active;
extern WINDOW *helpwin;

int dactive();
int dhelp();
int derror();

int do_menu (menu)
struct menu  *menu;
{
    while (1)
    {
	struct menu_ent *p;
	int  num;
	int  err;
	int  i;
	int  imax, center, line;
	char buff[2];

/* We start at two, leaving room for border. */
	line = 2;
	
/* erase the active menu. */
	werase(active);

 /*   First, determine column size for centering. */
	imax = strlen(menu->title);

	for (num=1; num<=menu->count; num++)
	{
		if ((strlen((menu->ent[num-1]->name)) +5) > imax)
			imax = strlen((menu->ent[num-1]->name))+5;
	}

/*    Leave room for border & error messages    */
	imax += 4;
	if (imax > COLS)
		imax = COLS;
	center = (COLS - imax) / 2;

/*    Write out menu           */
	if ((mvwprintw (active,line,center + (strlen(menu->title) / 2), "%s\n", menu->title)) == ERR)
		derror(EX_SC,"Error on write to active screen!");
	line += 2;
	num=0;

	for (num=0; num < menu->count; num++)
	{
		if ((mvwprintw (active,line,center + 3, "%3d) %s", num,
				(menu->ent[num])->name)) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		line += 2;
	}
	if ((mvwprintw (active,line,center + 3, "  ?) Help")) == ERR)
		derror(EX_SC,"Error on write to active screen!");
	line += 2;

	if ((mvwprintw (active,line,center + 3, "Selection: ")) == ERR)
		derror(EX_SC,"Error on write to active screen!");
	line ++;

	/* border the active menu */

	wattron(active,A_REVERSE);
	for(i=center; i <= imax + center; i++) {
		if ((mvwaddch(active,1,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		if ((mvwaddch(active,line,i,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
	}
	for(i=1; i <= line; i++) {
		if ((mvwaddch(active,i,center,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		if ((mvwaddch(active,i,center + imax,' ')) == ERR)
			derror(EX_SC,"Error on write to active screen!");
	}
 	while(1) {
		wattroff(active,A_REVERSE);
		if ((wmove(active,line-1,center + 14)) == ERR)
			derror(EX_SC,"Error on move to active screen!");
		wattroff(active,A_REVERSE);
		dactive();
		flushinp();
		if ((i = wgetch (active)) == ERR)
			derror(EX_SC,"Error on read from active screen!");
		*buff = i;
		buff[1]='\n';
		if(i == '?') {
		prhelp (menu->tok);
		dhelp();
		(void) wgetch(helpwin);
		continue;
		}
		if (i == CTLD) {
			touchwin(active);
			continue;
		}

		i = sscanf(buff,"%d",&num);
		if (i < 0 || num >= menu->count) {
			wattron(active,A_REVERSE);
			if ((mvwprintw(active,line-1,center + 17,"invalid entry")) == ERR)
				derror(EX_SC,"Error on move to active screen!");
			wattroff(active,A_REVERSE);
			touchwin(active);
			continue;
			}
		break;
		}
	/*
	 *  Process selection.
	 */
		p = menu->ent[num];
		err = (*p->proc) (p->parm1, p->parm2, p->parm3, p->parm4);
		return (err);
		}
}
