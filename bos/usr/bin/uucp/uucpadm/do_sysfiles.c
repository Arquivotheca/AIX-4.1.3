static char sccsid[] = "@(#)80	1.1  src/bos/usr/bin/uucp/uucpadm/do_sysfiles.c, cmduucp, bos411, 9428A410j 8/3/93 16:04:12";
/* 
 * COMPONENT_NAME: CMDUUCP do_sysfiles.c
 * 
 * FUNCTIONS: do_sysfiles
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
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
#include <fcntl.h>
#include <sys/stat.h>
#include "defs.h"

extern struct files Rules[FILES];
extern int cury[MAXENT+1], curx[MAXENT+1];
extern int sysfile_present;
extern char  File[FILES][MAXFILE];
extern char  entries[MAXENT][MAXLINE];
extern char  *sysfiles_image[];
extern WINDOW *active;


int bbase();
int bmenu();
int cleanu();
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

int do_sysfiles (menu)
struct menu_shell  *menu;
{
int  num;
int  i;
int  count;
int  new;
int  sysfile_index;
char *phold = "-";
int  len;
char *buffer;
char *lptr;
char *temptr;
char tempstr[90];
char *hold;
char *mem[MEMSIZE];
char svcstr[MAXLINE];
char systr[MAXLINE];
char devstr[MAXLINE];
char dialstr[MAXLINE];

(void) bbase(menu);
for (i=0;i < MEMSIZE;i++)
	mem[i] = NULL;
for (i=0;i < MAXENT;i++)
	entries[i][0] = '\0';

if (!sysfile_present) {
	
	if (mvwprintw(active,cury[1]+1,curx[1],
		"< /etc/uucp/Sysfiles not found--creating...>") == ERR)
 			derror(EX_SC,"Error on write to active screen!");
	wrefresh(active);
	sleep(1);
	(void) bline(cury[1]+1,curx[1]);
	Rules[Sysfiles].fd = open(Rules[Sysfiles].name, O_RDWR | O_CREAT); 
 	if (Rules[Sysfiles].fd < 0){
		if (mvwprintw(active,cury[1]+1,curx[1],
			"< Can't create Sysfiles file! >") == ERR)
 			derror(EX_SC,"Error on write to active screen!");
 		wrefresh(active);
 	}
 	else {
		sysfile_index = 0;
		while (sysfiles_image[sysfile_index][0] != (char) 0) {
			write(Rules[Sysfiles].fd,sysfiles_image[sysfile_index], 
					strlen(sysfiles_image[sysfile_index]));
			sysfile_index++;
		}
		/* Set proper owner and perms */
		(void) bline(cury[1]+1,curx[1]);
		if (i = fchmod (Rules[Sysfiles].fd, 0444) < 0 ) {
 			if (mvwprintw(active,cury[1]+1,curx[1],
				"< Can't set file permissions! >") == ERR)
 				derror(EX_SC,
					"Error on write to active screen!");
 			wrefresh(active);
		}
		if (i = fchown (Rules[Sysfiles].fd, 5, 5) < 0 ) {
			(void) bline(cury[1]+1,curx[1]);
			if (mvwprintw(active,cury[1]+1,curx[1],
				"< Can't set file ownership! >") == ERR)
 				derror(EX_SC,
					"Error on write to active screen!");
 			wrefresh(active);
		}
		if ((i = bload(&Rules[Sysfiles])) != EX_OK)
			derror (i,"Can't load Sysfiles file");
		sysfile_present = 1;
	}
}

new = TRUE;
count = 0;
num = 0;
while (1)
    {
	buffer = input("service",1,entries[0],0,0);
	(void) bline(cury[1]+1,curx[1]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if ((strchr(buffer,CTLU) != NULL || strchr(buffer,CTLX) != NULL) && strlen(entries[0]) == 0) {
		(void) bline(cury[1]+1,curx[1]);
		if (mvwprintw(active,cury[1]+1,curx[1],"< First entry required before update/delete! >") == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Sysfiles],mem[num],buffer,0);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
	}
	if (buffer == NULL) {
/* No entry. Check if we have cycled around. */
		if (strlen(entries[0]) == 0) {
			/* No entry key. Exit with message. */
			(void) bline(cury[1]+1,curx[1]);
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
	/*  We have some input. Check if new key is really new. If so
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
/*	if (new) {*/
	/* Check for valid service name */
	if (strcmp(buffer, "cu") && strcmp(buffer, "uucico")) {
	    if ((mvwprintw (active,cury[1]+1,curx[1], "<Service entry must be cu or uucico!>",entries[0])) == ERR)
	    	derror(EX_SC,"Error on write to active screen!");
		continue;
	    }

	if (new) {
	    /* Save the raw service name for later use */
	    (void) strcpy(svcstr, buffer);

	    /* Prepend "service=" to the service name */
	    /* to enable srch() to function correctly.*/
	    (void) strcpy(buffer,"service=");
	    (void) strcat(buffer, entries[0]);
	    (void) strcpy(entries[0], buffer);
	    count = 0;
	    len = strlen(entries[0]);
	    lptr = File[Sysfiles];
	    if ((lptr = srch(entries[0],len,lptr,0,1,&Rules[Sysfiles])) 
								!= NULL) 
	    {

		/* Assign each of the field entries to a specific holding */
		/* buffer for each entry.  We can't rely on input order   */
		/* from Sysfiles to dictate which entry is systems,       */
		/* devices or dialers.                                    */

		if (temptr = strstr(lptr, "systems=") ) {
			i = 0;
			temptr += strlen("systems=");
			while (*temptr != ' ' && *temptr != '\t' &&
			       *temptr != '\n' && *temptr != '\\' &&
			       *temptr != '\0'  && i < MAXLINE-1)
			{
				systr[i++] = *temptr++;
			}
			systr[i] = '\0';
		}
		else {
			/* No explicit Systems entry. Use the default. */
			(void) strcpy(systr, "Systems");
		}
		if (temptr = strstr(lptr, "devices=") ) {
			i = 0;
			temptr += strlen("devices=");
			while (*temptr != ' ' && *temptr != '\t' &&
			       *temptr != '\n' && *temptr != '\\' &&
			       *temptr != '\0'  && i < MAXLINE-1)
			{
				devstr[i++] = *temptr++;
			}
			devstr[i] = '\0';
		}
		else {
			/* No explicit Devices entry. Use the default. */
			(void) strcpy(devstr, "Devices");
		}
		if (temptr = strstr(lptr, "dialers=") ) {
			i = 0;
			temptr += strlen("dialers=");
			while (*temptr != ' ' && *temptr != '\t' &&
			       *temptr != '\n' && *temptr != '\\' &&
			       *temptr != '\0' && i < MAXLINE-1)
			{
				dialstr[i++] = *temptr++;
			}
			dialstr[i] = '\0';
		}
		else {
			/* No explicit Dialers entry. Use the default. */
			(void) strcpy(dialstr, "Dialers");
		}

		mem[count] = lptr;   /* Log hit -- the update routine uses */
				     /* mem[num] to reflect the current    */
				     /* size of the current entry.	   */
		count++ ;
		if (count > MEMSIZE -1)
			derror(EX_SOFTWARE,"internal memory overflow");
		lptr += strlen(lptr) + 1; /* Onward into the . . . */
	    }
	    else {
		/* srch couldn't come up with anything. */
		/* Initialize and wait for some input.  */
		(void) strcpy(systr, "Systems");
		(void) strcpy(devstr, "Devices");
		(void) strcpy(dialstr, "Dialers");
	    }
	    new = FALSE;
	    num = 0;
	}
 again:
	if (count) {
		(void) strcpy(entries[0], svcstr);
		(void) strcpy(entries[1], systr);
		(void) strcpy(entries[2], devstr);
		(void) strcpy(entries[3], dialstr);
	}

/* OK, do simple check of type validity. */
	
	/* Update the menu with the current entry info */
	len = strlen(entries[0]);
	(void) bline(cury[1]+1,curx[1]);
	(void) dmenu(count,num,menu);

	/****************************************/
	/* Wait for input on the Systems field. */
	/****************************************/
	buffer = input("systems",2,entries[1],0,0);
	(void) bline(cury[2]+1,curx[2]);

	/* Do different things, depending on what the input was. */
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		/* Keep systr current for CLTU's on other fields */
		if (buffer[0] != CTLU) {
			/* Only retain the buffer contents */
			/* if the user typed something.    */
			(void) strcpy(systr, buffer);
		}
		else {
			(void) strcpy(buffer, systr);
		}
		if (temptr=strchr(systr, CTLU)) {
			*temptr = '\0';
		}

		/* Re-add the "attribute=" stuff to the  */
		/* file entries before the update occurs */
		(void) strcpy(entries[0], "service=");
		(void) strcat(entries[0], svcstr);

		(void) strcpy(tempstr, "\tsystems=");
		(void) strcat(tempstr, buffer);
		if (temptr=strchr(tempstr, CTLU)) {
			*temptr = '\0';
		}
		(void) strcat(tempstr, " \\\n");
		(void) strcpy(buffer, tempstr);

		(void) strcpy(entries[2], "\t\tdevices=");
		(void) strcat(entries[2], devstr);
		(void) strcat(entries[2], " \\\n");
		(void) strcpy(entries[3], "\t\tdialers=");
		(void) strcat(entries[3], dialstr);

		(void) update(&Rules[Sysfiles],mem[num],buffer,1);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		/* After the screen's been updated, */
		/* remove all the "attribute=" stuff*/
		(void) strcpy(entries[0], svcstr);
		(void) strcpy(entries[1], systr);
		(void) strcpy(entries[2], devstr);
		(void) strcpy(entries[3], dialstr);
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(systr, buffer);
	}

	/****************************************/
	/* Wait for input on the Devices field. */
	/****************************************/
	buffer = input("devices",3,entries[2],0,0);
	(void) bline(cury[3]+1,curx[3]);

	/* Do different things, depending on what the input was. */
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		/* Keep devstr current for CLTU's on other fields */
		if (buffer[0] != CTLU) {
			/* Only retain the buffer contents */
			/* if the user typed something.    */
			strcpy(devstr, buffer);
		}
		else {
			(void) strcpy(buffer, devstr);
		}
		if (temptr=strchr(devstr, CTLU)) {
			*temptr = '\0';
		}

		/* Re-add the "attribute=" stuff to the  */
		/* file entries before the update occurs */
		(void) strcpy(entries[0], "service=");
		(void) strcat(entries[0], svcstr);
		(void) strcpy(entries[1], "\tsystems=");
		(void) strcat(entries[1], systr);
		(void) strcat(entries[1], " \\\n");

		(void) strcpy(tempstr, buffer);
		(void) strcpy(buffer, "\t\tdevices=");
		(void) strcat(buffer, tempstr);
		if (temptr=strchr(buffer, CTLU)) {
			*temptr = '\0';
		}
		(void) strcat(buffer, " \\\n");

		(void) strcpy(entries[3], "\t\tdialers=");
		(void) strcat(entries[3], dialstr);

		(void) update(&Rules[Sysfiles],mem[num],buffer,2);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		/* After the screen's been updated, */
		/* remove all the "attribute=" stuff*/
		(void) strcpy(entries[0], svcstr);
		(void) strcpy(entries[1], systr);
		(void) strcpy(entries[2], devstr);
		(void) strcpy(entries[3], dialstr);
		continue;
	}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
	}
	if (buffer != NULL) {
		(void) strcpy(devstr, buffer);
	}

	/****************************************/
	/* Wait for input on the Dialers field. */
	/****************************************/
	buffer = input("dialers",4,entries[3],0,0);
	(void) bline(cury[4]+1,curx[4]);

	/* Do different things, depending on what the input was. */
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		/* Keep devstr current for CLTU's on other fields */
		if (buffer[0] != CTLU) {
			/* Only retain the buffer contents */
			/* If the user typed anything.     */
			(void) strcpy(dialstr, buffer);
		}
		else {
			(void) strcpy(buffer, dialstr);
		}
		if (temptr=strchr(dialstr, CTLU)) {
			*temptr = '\0';
		}

		/* Re-add the "attribute=" stuff to the  */
		/* file entries before the update occurs */
		(void) strcpy(entries[0], "service=");
		(void) strcat(entries[0], svcstr);
		(void) strcpy(entries[1], "\tsystems=");
		(void) strcat(entries[1], systr);
		(void) strcat(entries[1], " \\\n");
		(void) strcpy(entries[2], "\t\tdevices=");
		(void) strcat(entries[2], devstr);
		(void) strcat(entries[2], " \\\n");

		(void) strcpy(tempstr, buffer);
		(void) strcpy(buffer, "\t\tdialers=");
		(void) strcat(buffer, tempstr);

		(void) update(&Rules[Sysfiles],mem[num],buffer,3);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		/* After the screen's been updated, */
		/* remove all the "attribute=" stuff*/
		(void) strcpy(entries[0], svcstr);
		(void) strcpy(entries[1], systr);
		(void) strcpy(entries[2], devstr);
		(void) strcpy(entries[3], dialstr);
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Sysfiles],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(dialstr, buffer);
	}

	if (count) {
		(void) bline(cury[4]+2,curx[4]);
		if ((mvwprintw (active,cury[4]+2,curx[4], "To next entry? (y or n) ==> "))== ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		while ((i = wgetch(active)) != 'y' && i != 'n')
					;
		(void) bline(cury[4]+2,curx[4]);
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
