static char sccsid[] = "@(#)70	1.5  src/bos/usr/bin/uucp/uucpadm/do_perms.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:00";
/* 
 * COMPONENT_NAME: CMDUUCP do_perms.c
 * 
 * FUNCTIONS: do_perms, paste 
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
#include <pwd.h>
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
int cypherp();
int derror();
char *build();
char *input();
char *srch();

int do_perms (menu)
struct menu_shell  *menu;
{
int  num;
int  i;
int  count;
int  new;
int  len;
int paste();
char trash[MAXLINE];
char *buffer;
char *lptr;
char *mem[MEMSIZE];
char *Mstr = "MACHINE=";
char *Lstr = "LOGNAME=";
/*struct passwd *getpwnam();*/

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
	buffer = input("lmP",1,entries[0],0,0);
	bline(cury[1]+1,curx[1]);
	if (wmove(active,cury[2],curx[2]) == ERR)
		derror(EX_SOFTWARE,"Move on active screen failed!");
	wrefresh(active);
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
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,0);
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
		(void) strcpy(trash,entries[0]);
		(void) cleanu(menu,mem,NULL);
		new = FALSE;
		(void) strcpy(entries[0],trash);
		count = 0;
		num = 0;
	}
	if (buffer != NULL) {
/* Blowout any invalid entries. */
	if ((strncmp(buffer,Lstr,(size_t) 8) != 0) && (strncmp(buffer,Mstr,(size_t) 8) != 0)) {
		bline(cury[1]+1,curx[1]);
		if ((mvwprintw(active,cury[1]+1,curx[1],"< L/M must start with LOGNAME= or MACHINE= !>")) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		continue;
			}			
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
/* Check login id or system entry. */
	if (!strncmp(entries[0],Mstr,(size_t) 8)) {
		buffer = entries[0];
		buffer += 8;
		len = strlen(buffer);
		if (search(buffer,Systems,1) == NULL) {
		bline(cury[1]+1,curx[1]);
		if ((mvwprintw (active,cury[1]+1,curx[1], "<Warning: MACHINE %s not found in Systems!>",entries[0])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
			}
		}
	if (!strncmp(entries[0],Lstr,(size_t) 8)) {
		buffer = entries[0];
		buffer += 8;
		if (getpwnam(buffer) == NULL) {
		bline(cury[1]+1,curx[1]);
		if ((mvwprintw (active,cury[1]+1,curx[1], "<Warning: LOGNAME %s not found!>",buffer)) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
			}
		}
/* Have some input. Get oriented. First step is to scan for existing
*  record. We start at zero.
*/
	if (new) {
	count = 0;
	len = strlen(entries[0]);
	lptr = File[Permissions];
	while ((lptr = srch(entries[0],len,lptr,0,1,&Rules[Permissions])) != NULL) {
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
		(void) cypherp(mem[num],&Rules[Permissions]);
/* strip prefixes for dmenu. We will add them back on update. */
	if (strlen(entries[1])) {
		buffer = strpbrk(entries[1],"=");
		buffer ++;
		(void) strcpy(entries[1],buffer);
		}
	if (strlen(entries[2])) {
		buffer = strpbrk(entries[2],"=");
		buffer ++;
		(void) strcpy(entries[2],buffer);
		}
	if (strlen(entries[2])) {
		buffer = strpbrk(entries[2],"=");
		buffer ++;
		(void) strcpy(entries[2],buffer);
		}
	if (strlen(entries[3]) && strncmp(entries[3],"NOREAD=",(size_t) 7)) {
		buffer = strpbrk(entries[3],"=");
		buffer ++;
		(void) strcpy(entries[3],buffer);
		}
	if (strlen(entries[4]) && strncmp(entries[4],"NOWRITE=",(size_t) 8)) {
		buffer = strpbrk(entries[4],"=");
		buffer ++;
		(void) strcpy(entries[4],buffer);
		}
	if (strlen(entries[5])) {
		buffer = strpbrk(entries[5],"=");
		buffer ++;
		(void) strcpy(entries[5],buffer);
		}
	if (strlen(entries[6])) {
		buffer = strpbrk(entries[6],"=");
		buffer ++;
		(void) strcpy(entries[6],buffer);
		}
	if (strlen(entries[7])) {
		buffer = strpbrk(entries[7],"=");
		buffer ++;
		(void) strcpy(entries[7],buffer);
		}
	if (count > 1) {
		bline(cury[1]+1,curx[1]);
		if (mvwprintw(active,cury[1]+1,curx[1],"<Warning: Multiple entries exist for %s!>",entries[0]) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	(void) dmenu(count,num,menu);
	buffer = input("request",2,entries[1],0,1);
	bline(cury[2]+1,curx[2]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,1);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) 
		(void) strcpy(entries[1],buffer);
	buffer = input("sendfiles",3,entries[2],0,1);
	bline(cury[3]+1,curx[3]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,2);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) 
		(void) strcpy(entries[2],buffer);
	if (strlen(entries[2]) && !strncmp(entries[0],Mstr,(size_t) 8)) {
		bline(cury[3]+1,curx[3]);
		if ((mvwprintw (active,cury[3]+1,curx[3], "<Warning: Sendfiles ignored with MACHINE entries!>")) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
			}
	buffer = input("read",4,entries[3],0,0);
	bline(cury[4]+1,curx[4]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,3);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) 
				(void) strcpy(entries[3],buffer);
	buffer = input("write",5,entries[4],0,0);
	bline(cury[5]+1,curx[5]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,4);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) 
				(void) strcpy(entries[4],buffer);
	buffer = input("callback",6,entries[5],0,1);
	bline(cury[6]+1,curx[6]);
 	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,5);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		if (!strncmp(entries[0],Mstr,(size_t) 8)) {
			bline(cury[6],curx[6]);
			bline(cury[6]+1,curx[6]);
			if ((mvwprintw (active,cury[6]+1,curx[6], "<Warning: Callback not valid with MACHINE entry!>")) == ERR)
				derror(EX_SC,"Error on write to active screen!");
			wrefresh(active);
			(void) strcpy(entries[5],(char *) NULL);
				}
		else 
			(void) strcpy(entries[5],buffer);
		}
	buffer = input("commands",7,entries[6],0,0);
	bline(cury[7]+1,curx[7]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,6);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		if (!strncmp(entries[0],Lstr,(size_t) 8)) {
			bline(cury[7],curx[7]);
			bline(cury[7]+1,curx[7]);
			if ((mvwprintw (active,cury[7]+1,curx[7], "<Warning: Commands not valid with LOGNAME entry!>")) == ERR)
				derror(EX_SC,"Error on write to active screen!");
			wrefresh(active);
		    (void) strcpy(entries[6],(char *) NULL);
				}
		else 
			(void) strcpy(entries[6],buffer);
		}
	buffer = input("validate",8,entries[7],0,0);
	bline(cury[8]+1,curx[8]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) paste();
		(void) update(&Rules[Permissions],mem[num],buffer,7);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Permissions],mem[num]);
		(void) cleanu(menu,mem,NULL);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		if (!strncmp(entries[0],Mstr,(size_t) 8)) {
			bline(cury[8],curx[8]);
			bline(cury[8]+1,curx[8]);
			if ((mvwprintw (active,cury[8]+1,curx[8], "<Warning: Validate not valid with MACHINE entry!>")) == ERR)
				derror(EX_SC,"Error on write to active screen!");
			wrefresh(active);
			(void) strcpy(entries[7],(char *) NULL);
				}
		else 
			(void) strcpy(entries[7],buffer);
		}
	if (count) {
		bline(cury[8]+2,curx[8]);
		if ((mvwprintw (active,cury[8]+2,curx[8], "To next entry? (y or n) ==> "))== ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		while ((i = wgetch(active)) != 'y' && i != 'n')
					;
		bline(cury[8]+2,curx[8]);
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
int paste()
{
char trash[MAXLINE];
char *Qstr = "REQUEST=";
char *Sstr = "SENDFILES=";
char *Rstr = "NOREAD=";
char *Wstr = "NOWRITE=";
char *Bstr = "CALLBACK=";
char *Cstr = "COMMANDS=";
char *Vstr = "VALIDATE=";
char *lptr;
	
	if (strlen(entries[1])) {
		(void) strcpy(trash,entries[1]);
		(void) strcpy(entries[1],Qstr);
		(void) strcat(entries[1],trash);
		}
	if (strlen(entries[2])) {
		(void) strcpy(trash,entries[2]);
		(void) strcpy(entries[2],Sstr);
		(void) strcat(entries[2],trash);
		}
	if (strlen(entries[3]) && strncmp(entries[3],Rstr,strlen(Rstr))) {
		(void) strcpy(trash,entries[3]);
		lptr = Rstr; lptr += 2;
		(void) strcpy(entries[3],lptr);
		(void) strcat(entries[3],trash);
		}
	if (strlen(entries[4]) && strncmp(entries[4],Wstr,strlen(Wstr))) {
		(void) strcpy(trash,entries[4]);
		lptr = Wstr; lptr += 2;
		(void) strcpy(entries[4],lptr);
		(void) strcat(entries[4],trash);
		}
	if (strlen(entries[5])) {
		(void) strcpy(trash,entries[5]);
		(void) strcpy(entries[5],Bstr);
		(void) strcat(entries[5],trash);
		}
	if (strlen(entries[6])) {
		(void) strcpy(trash,entries[6]);
		(void) strcpy(entries[6],Cstr);
		(void) strcat(entries[6],trash);
		}
	if (strlen(entries[7])) {
		(void) strcpy(trash,entries[7]);
		(void) strcpy(entries[7],Vstr);
		(void) strcat(entries[7],trash);
		}
	return(EX_OK);
}
