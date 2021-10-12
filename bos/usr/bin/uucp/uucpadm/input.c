static char sccsid[] = "@(#)74	1.6  src/bos/usr/bin/uucp/uucpadm/input.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:18";
/* 
 * COMPONENT_NAME: CMDUUCP input.c
 * 
 * FUNCTIONS: input 
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
#ifndef lint
static char aix_sccsid[] = "com/cmd/uucp/uucpadm,3.1.1,9021 AIX 6/16/90 00:06:11";
#endif
/*
 *  input
 *
 *  Copyright (c) 1988 IBM Corporation.  All rights reserved.
 *
 *
 */


#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <ctype.h>
#include "defs.h"

extern struct termios sg;

extern WINDOW *active;
extern WINDOW *helpwin;

extern int cury[MAXENT+1],curx[MAXENT+1];

int dactive();
int dhelp();
int bline();
int derror();

char *input(what,num,old,spaces,booleans)
char *what;
int num;
char *old;
int spaces;
int booleans;
{
	int i;
	int ierror;
	static char target[MAXLINE];
	char *lp;
	ierror = FALSE;
	
	while(1) {
        	bzero(target,MAXLINE);
/* Mark end of buffer. We will not allow overflow. */
		target[MAXLINE-1] = '\r';
		lp = target;
		if ((wmove(active,cury[num],curx[num])) == ERR)
			derror(EX_SC,"Error on move to active screen!");
  		wrefresh(active);
		if (ierror) {
			(void) bline(cury[num],curx[num]);
  			wrefresh(active);
			(void) bline(cury[num]+1,curx[num]);
			if ((wmove(active,cury[num],curx[num])) == ERR)
				derror(EX_SC,"Error on move to active screen!");
			ierror = FALSE;
		}
		flushinp();
		while (i = wgetch(active))  {
			if (i == ERR)
				derror(EX_SC,"Error on read from screen!");
			if (i == CTLD || i == CTLX || i == CTLU) {
				*lp = i;
				target[MAXLINE-1] = '\0';
				return(target);
			}
			if (i == '?') {
				prhelp(what);
				dhelp();
				(void) wgetch(helpwin);
/* All this effort just so we can restore what the user
*  had before he requested help. 
*/
				if (strlen(old) != 0) {
					if ((mvwprintw(active,cury[num],curx[num],"%s ",old)) == ERR)
						derror(EX_SC,"Error on read from screen!");
				} else {
					if ((mvwprintw(active,cury[num],curx[num]," ")) == ERR)
						derror(EX_SC,"Error on read from screen!");
				}	
				break;
			}
			if (i == '\r') {
				*lp = '\0';
				target[MAXLINE-1] = '\0';
				if (strlen(target) > 0)	
					return(target);
				else 
					return(NULL);
			}
			if (lp == target) {
				bline(active->_cury,active->_curx);
				wrefresh(active);
			}
			if (isspace(i) && !spaces) {
				ierror = TRUE;
				(void) bline(cury[num],curx[num]);
				(void) bline(cury[num]+1,curx[num]);
				if ((mvwprintw(active,cury[num]+1,curx[num],"< No spaces permitted!>  ")) == ERR)
			 		derror(EX_SC,"Error on write to active screen!");
				break;
			}
			if (!isprint(i)) {
				ierror = TRUE;
				(void) bline(cury[num]+1,curx[num]);
				if ((mvwprintw(active,cury[num]+1,curx[num],"< Invalid control character!>  ")) == ERR)
			 		derror(EX_SC,"Error on write to active screen!");
				break;
			}
			if (i == '~') {
				*lp = i;
				target[MAXLINE-1] = '\0';
				return(target);
			}
			if (active -> _curx < curx[num]) {
				if ((wmove(active,cury[num],curx[num])) == ERR)
					derror(EX_SC,"Error on move to active screen!");
				break;
			}
			waddch(active,i);
			wrefresh(active);
			if (i == ERASE) {
				if (waddch(active,' ') == ERR)
					derror(EX_SC,"Error on read from screen!");
				active -> _curx --;
				wrefresh(active);
				lp--;
				*lp='\0';
			}
			if (lp == target && (booleans)) {
				if (i == 'y') {
					(void) strcpy(target,"yes");
					waddstr(active,"es");
					wrefresh(active);
					return(target);
				}
				if (i == 'n') {
					(void) strcpy(target,"no");
					waddstr(active,"o");
					wrefresh(active);
					return(target);
				}
				ierror = TRUE;
				(void) bline(cury[num],curx[num]);
				(void) bline(cury[num]+1,curx[num]);
				if ((mvwprintw(active,cury[num]+1,curx[num],"< Entry must be yes or no!>  ")) == ERR)
		 			derror(EX_SC,"Error on write to active screen!");
				break;
			}
			if (i != ERASE)
				*lp++ = i;
			if (*lp != '\0') {
				(void) bline(cury[num],curx[num]);
				wattron(active,A_REVERSE);
				if ((mvwprintw(active,cury[num]+1,curx[num],"Input too long use Editor")) == ERR)
				 	derror(EX_SC,"Error on write to active screen!");
				wattroff(active,A_REVERSE);
				ierror = TRUE;
				break;
			}
		}
	}
}
