static char sccsid[] = "@(#)67	1.6  src/bos/usr/bin/uucp/uucpadm/do_devices.c, cmduucp, bos411, 9428A410j 8/3/93 16:14:47";
/* 
 * COMPONENT_NAME: CMDUUCP do_devices.c
 * 
 * FUNCTIONS: do_devices 
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
char *search();

int do_devices (menu)
struct menu_shell  *menu;
{
int  num;
int  i;
int  count;
int  new;
char *phold = "-";
int  len;
char *buffer;
char *lptr;
char *hold;
char *path = "/dev/";
char *mem[MEMSIZE];
struct stat dummy;

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
	buffer = input("type",1,entries[0],0,0);
	bline(cury[1]+1,curx[1]);
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
		(void) do_vi(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Devices],mem[num],buffer,0);
		(void) cleanu(menu,mem,phold);
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
		new = FALSE;
		if ((hold = malloc((unsigned) strlen(entries[0]) +1)) == NULL)
			derror(EX_SOFTWARE,"Unable to malloc storage");
		(void) strcpy(hold,entries[0]);
		(void) cleanu(menu,mem,phold);
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
		(void) cleanu(menu,mem,phold);
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
	lptr = File[Devices];
	while ((lptr = srch(entries[0],len,lptr,0,1,&Rules[Devices])) != NULL) {
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
		(void) cypher(mem[num],&Rules[Devices],menu->counts);
/* OK, do simple check of type validity. */
	len = strlen(entries[0]);
	bline(cury[1]+1,curx[1]);
	if (strcmp(entries[0],"Direct") && strncmp(entries[0],"TCP",(size_t) 3) && strcmp(entries[0],"ACU"))
	{
		if ((search(entries[0],Systems,3) == NULL) && \
		    (search(entries[0],Dialers,1) == NULL)) {
		bline(cury[1]+1,curx[1]);
			if ((mvwprintw (active,cury[1]+1,curx[1], "<Warning: %s not found in Systems or Dialers!>",entries[0])) == ERR)
				derror(EX_SC,"Error on write to active screen!");
			}
	}
	if (!strncmp(entries[0],"TCP",(size_t) 3))
		{
		bline(cury[1]+1,curx[1]);
		if ((mvwprintw (active,cury[1]+1,curx[1], "<Note: make certain uucpd is enabled.>"))== ERR)
				derror(EX_SC,"Error on write to active screen!");
		}
	(void) dmenu(count,num,menu);
	buffer = input("line1",2,entries[1],0,0);
	bline(cury[2]+1,curx[2]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Devices],mem[num],buffer,1);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[1],buffer);
		}
	if(strncmp(entries[0],"TCP",(size_t) 3) == 0 && strcmp(entries[1],phold) != 0) {
		bline(cury[2]+1,curx[2]);
		if ((mvwprintw (active,cury[2]+1,curx[2], "<Warning: %s not correct when type is TCP!>",entries[1])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
			}
	if(strncmp(entries[0],"TCP",(size_t) 3) && strcmp(entries[1],phold)) {
		if ((hold = malloc((unsigned) (strlen(entries[1]) + 8))) == NULL)
			derror(EX_SOFTWARE,"Unable to malloc storage");
		(void) strcpy(hold,path);
		(void) strcat(hold,entries[1]);
/* OK, do simple check for the device */
		errno = 0;
		if (stat(hold,&dummy) != 0) {
			bline(cury[2]+1,curx[2]);
			if ((mvwprintw (active,cury[2]+1,curx[2],"<Warning[%d]: %s not found!>",errno,hold)) == ERR)
				derror(EX_SC,"Error on write to active screen!");
			free((void *) hold);
			}
		}
	buffer = input("line2",3,entries[2],0,0);
	bline(cury[3]+1,curx[3]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Devices],mem[num],buffer,2);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[2],buffer);
		}
	if(strncmp(entries[0],"TCP",(size_t) 3) == 0 && strcmp(entries[2],phold) != 0) {
		bline(cury[3]+1,curx[3]);
		if ((mvwprintw (active,cury[3]+1,curx[3], "<Warning: %s not correct when type is TCP!>",entries[2])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
			}
	if(strncmp(entries[0],"TCP",(size_t) 3) && strcmp(entries[2],phold)) {
		if ((hold = malloc((unsigned) (strlen(entries[2]) + 8))) == NULL)
			derror(EX_SOFTWARE,"Unable to malloc storage");
		(void) strcpy(hold,"/dev/");
		(void) strcat(hold,entries[2]);
/* OK, do simple check for the device */
		errno = 0;
		if (stat(hold,&dummy) != 0) {
			bline(cury[3]+1,curx[3]);
			if ((mvwprintw (active,cury[3]+1,curx[3],"<Warning[%d]: %s not found!>",errno,hold)) == ERR)
				derror(EX_SC,"Error on write to active screen!");
			free((void *) hold);
			}
		}
	buffer = input("classd",4,entries[3],0,0);
	bline(cury[4]+1,curx[4]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Devices],mem[num],buffer,3);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[3],buffer);
		}
	if(strncmp(entries[0],"TCP",(size_t) 3) == 0 && strcmp(entries[3],phold) != 0) {
		bline(cury[4]+1,curx[4]);
		if ((mvwprintw (active,cury[4]+1,curx[4], "<Warning: %s not correct when type is TCP!>",entries[3])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
			}
/* Simple consistency check. Allow only one non-digit */
	if ((i = strlen(entries[3]) - strspn(entries[3],"0123456789")) > 1) {
		bline(cury[4]+1,curx[4]);
		if ((mvwprintw (active,cury[4]+1,curx[4], "<Warning: Bad format to %s !>",entries[3])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
	}
	buffer = input("dialersd",5,entries[4],1,0);
	bline(cury[5]+1,curx[5]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Devices],mem[num],buffer,4);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Devices],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[4],buffer);
		}
	if(strncmp(entries[0],"TCP",(size_t) 3) == 0 && strcmp(entries[4],entries[0]) != 0) {
		bline(cury[5]+1,curx[5]);
		if ((mvwprintw (active,cury[5]+1,curx[5], "<Warning: %s not correct when type is TCP!>",entries[4])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
			}
	len = strcspn(entries[4],Rules[Dialers].delimit);
	if (search(entries[4],Dialers,1) == NULL) {
		bline(cury[5]+1,curx[5]);
		if ((mvwprintw (active,cury[5]+1,curx[5], "<Warning: Dialer %s not found in Dialers!>",entries[4])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	if (count) {
		bline(cury[5]+2,curx[5]);
		if ((mvwprintw (active,cury[5]+2,curx[5], "To next entry? (y or n) ==> "))== ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		while ((i = wgetch(active)) != 'y' && i != 'n')
					;
		bline(cury[5]+2,curx[5]);
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
