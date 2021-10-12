static char sccsid[] = "@(#)68	1.5  src/bos/usr/bin/uucp/uucpadm/do_dialc.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:52";
/* 
 * COMPONENT_NAME: CMDUUCP do_dialc.c
 * 
 * FUNCTIONS: do_dialc 
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
#include <errno.h>
#include <sys/stat.h>
#include "defs.h"

extern struct files Rules[FILES];
extern int cury[MAXENT+1], curx[MAXENT+1];
extern char  File[FILES][MAXFILE];
extern char  entries[MAXENT][MAXLINE];
extern WINDOW *active;


int bbase();
int bmenu();
int cleanu();
int cypher();
int dactive();
int do_vi();
int dmenu();
int update();
int zaps();
int derror();
char *build();
char *input();
char *malloc();
char *srch();

int do_dialc (menu)
struct menu_shell  *menu;
{
int  num;
int  i;
int  count;
int  new;
int  len;
char *buffer;
char *lptr;
char *hold;
char *mem[MEMSIZE];

(void) bbase(menu);
for (i=0;i < MEMSIZE;i++)
	mem[i] = NULL;
for (i=0;i < MAXENT;i++)
	entries[i][0] = '\0';
new = TRUE;
count = 0;
num = 0;
while (1)
    {
	buffer = input("abr",1,entries[0],0,0);
	bline(cury[1]+1,curx[1]);
	if ((wmove(active,cury[1],curx[1])) == ERR)
		derror(EX_SOFTWARE,"Error on move to active screen!");
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if ((strchr(buffer,CTLU) != NULL || strchr(buffer,CTLX) != NULL) && strlen(entries[0]) == 0) {
		bline(cury[1]+1,curx[1]);
		if (mvwprintw(active,cury[1]+1,curx[1],"< First entry required before update/delete! >") == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Dialcode],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Dialcode],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Dialcode],mem[num],buffer,0);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer == NULL) {
/* No entry. Check if we have cycled around. */
		if (strlen(entries[0]) == 0) {
/* No entry key. Exit with message. */
		bline(cury[1]+1,curx[1]);
		if (mvwprintw(active,cury[1]+1,curx[1],"< First entry required! >") == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		continue;
			}
		if ((hold = malloc((unsigned) strlen(entries[0]) +1)) == NULL)
			derror(EX_SOFTWARE,"Unable to malloc storage");
		(void) strcpy(hold,entries[0]);
		(void) cleanu(menu,mem,NULL);
		new = FALSE;
		(void) strcpy(entries[0],hold);
		free((void *) hold);
		count = 0;
		num = 0;
	}
	if (buffer != NULL) {
/* We have some input. Check if new key is really new. If so
*  clear the menu & error messages.
*/
	if (strcmp(buffer,entries[0])) {
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		count = 0;
		num = 0;
		(void) strcpy(entries[0],buffer);
			}
		}
/* Have some input. Get oriented. First step is to scan for existing
*  record. We start at zero.
*/
	if (new) {
	count = 0;
	len = strlen(entries[0]);
	lptr = File[Dialcode];
	while ((lptr = srch(entries[0],len,lptr,0,1,&Rules[Dialcode])) != NULL) {
		mem[count] = lptr; 	/* Log hit */
		count ++ ;
		if (count > MEMSIZE -1)
			derror(EX_SOFTWARE,"internal memory overflow");
		lptr += strlen(lptr) + 1; /* Onward into the . . . */
		}
	new = FALSE;
	num = 0;
	}
 again:
	if (count)  
		(void) cypher(mem[num],&Rules[Dialcode],menu->counts);
	bline(cury[1]+1,curx[1]);
	if (count > 1) {
		bline(cury[1]+1,curx[1]);
		if (mvwprintw(active,cury[1]+1,curx[1],"<Warning: Multiple entries exist with this dialcode!>") == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	(void) dmenu(count,num,menu);
	buffer = input("dialcode",2,entries[1],0,0);
	bline(cury[2]+1,curx[2]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Dialcode],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Dialcode],mem[num],buffer,1);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Dialcode],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[1],buffer);
		}
	if (count) {
		bline(cury[2]+2,curx[2]);
		if ((mvwprintw (active,cury[2]+2,curx[2], "To next entry? (y or n) ==> "))== ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		while ((i = wgetch(active)) != 'y' && i != 'n')
					;
		bline(cury[2]+2,curx[2]);
		if (i == 'y') {
		num++;
/* So much for structure! */
		if (num < count) {
			(void) bmenu(menu);
						}
		else {
/* Going back to top set pointer back to last record in case user
*  wants to vi, add or delete.
*/
			num--;
			continue;
				}
			}
		goto again;
		}
	}
}
