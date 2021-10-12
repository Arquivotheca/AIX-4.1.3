static char sccsid[] = "@(#)79	1.1  src/bos/usr/bin/uucp/uucpadm/do_dialers.c, cmduucp, bos411, 9428A410j 8/3/93 16:04:09";
/* 
 * COMPONENT_NAME: CMDUUCP do_dialers.c
 * 
 * FUNCTIONS: do_dialers
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
#include <sys/stat.h>
#include "defs.h"

extern struct files Rules[FILES];
extern int cury[MAXENT+1], curx[MAXENT+1];
extern char  File[FILES][MAXFILE];
extern char  entries[MAXENT][MAXLINE];
extern WINDOW *active;


int bbase();
int bmenu();
int count_fields();
int cleanu();
int dactive();
int do_vi();
int dmenu();
int update();
int zaps();
char *build();
char *input();
char *malloc();
char *srch();

int do_dialers (menu)
struct menu_shell  *menu;
{
int  num;
int  i,j,k;
int  count;
int  fldcnt;
int  fldcpy[80];
int  new;
char *phold = "";
int  len;
char *buffer;
char linecopy[240];
char *lptr;
char *temptr;
char tempstr[80];
char *hold;
char *mem[MEMSIZE];
char typestr[MAXLINE];
char substr[MAXLINE];
char expectstr[MAXLINE];
char dialstr[MAXLINE];

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
	buffer = input("dodialers",1,entries[0],0,0);
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
		(void) do_vi(&Rules[Dialers],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Dialers],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Dialers],mem[num],buffer,0);
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
	if (new) {
	    count = 0;
	    len = strlen(entries[0]);
	    lptr = File[Dialers];
	    if ((lptr = srch(entries[0],len,lptr,0,1,&Rules[Dialers])) 
								!= NULL) 
	    {
		/* Get a copy of the line for manipulation */
		(void) strcpy(linecopy, lptr);

		/* Assign the field entries to a specific  */
		/* holding buffer for each entry.          */
		i = remove_field(linecopy, typestr, 1);
		if ( i < 0 )
			derror(EX_SOFTWARE,"Field processing error");

		i = remove_field(linecopy, substr, 1);
		if ( i < 0 )
			derror(EX_SOFTWARE,"Field processing error");

		(void) strcpy(expectstr, linecopy);

		mem[count] = lptr;   /* Log hit -- the update routine uses */
				     /* mem[num] to reflect the current    */
				     /* size of the current entry.	   */
		count++ ;
		if (count > MEMSIZE -1)
			derror(EX_SOFTWARE,"internal memory overflow");
		lptr += strlen(lptr) + 1; /* Onward into the . . . */
	    }
	    else {
		(void) strcpy(substr, "");
		(void) strcpy(expectstr, "");
	    }
	    new = FALSE;
	    num = 0;
	}
 again:
	if (count) {
		(void) strcpy(entries[0], typestr);
		(void) strcpy(entries[1], substr);
		(void) strcpy(entries[2], expectstr);
	}
		
/* OK, do simple check of type validity. */
	
	/* Update the menu with the current entry info */
	len = strlen(entries[0]);
	(void) bline(cury[1]+1,curx[1]);
	(void) dmenu(count,num,menu);

	/*******************************************/
	/* Wait for input on the substitute field. */
	/*******************************************/
	buffer = input("substitute",2,entries[1],0,0);
	(void) bline(cury[2]+1,curx[2]-5);

	/* Do different things, depending on what the input was. */
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Dialers],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		/* Keep substr current for CLTU's on other fields */
		if (buffer[0] != CTLU) {
			/* Only retain the buffer contents */
			/* if the user typed something.    */
			(void) strcpy(substr, buffer);
		}
		else {
			(void) strcpy(buffer, substr);
		}
		if (temptr=strchr(substr, CTLU)) {
			*temptr = '\0';
		}
		if ( ( (strlen(substr) % 2) != 0 ) || strlen(substr) > 4 ) {
			(void) bline(cury[2]+1,curx[2]-5);
			if ((mvwprintw (active,cury[2]+1,curx[2]-5, 
				  "<Warning:This field should contain 2 or 4 characters.>",
				   hold)) == ERR)
			{
			     derror(EX_SC, "Error on write to active screen!");
			}
			wrefresh(active);
			sleep(1);
			(void) bline(cury[2]+1,curx[2]-5);
		}

		(void) update(&Rules[Dialers],mem[num],buffer,1);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
	}

	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Dialers],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		if ( ( (strlen(buffer) % 2) != 0 ) || strlen(buffer) > 4 ) {
			(void) bline(cury[2]+1,curx[2]-5);
			if ((mvwprintw (active,cury[2]+1,curx[2]-5, 
				  "<Warning:This field should contain 2 or 4 characters.>",
				   hold)) == ERR)
			{
			     derror(EX_SC, "Error on write to active screen!");
			}
			wrefresh(active);
		}
		(void) strcpy(substr, buffer);
		(void) strcpy(entries[1], buffer);
	}

	/********************************************/
	/* Wait for input on the Expect-send field. */
	/********************************************/
	buffer = input("expect",3,entries[2],1,0);
	(void) bline(cury[3]+1,curx[3]);

	/* Do different things, depending on what the input was. */
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Dialers],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		/* Keep expectstr current for CLTU's on other fields */
		if (buffer[0] != CTLU) {
			/* Only retain the buffer contents */
			/* if the user typed something.    */
			strcpy(expectstr, buffer);
		}
		else {
			strcpy(buffer, expectstr);
		}
		if (temptr=strchr(expectstr, CTLU)) {
			*temptr = '\0';
		}
		fldcnt = count_fields(buffer);
		/* If an odd number of fields are present, weed out null   */
		/* strings before alerting the user to a possible mismatch */
		k = fldcnt;
		if ( (fldcnt % 2) != 0 ) {
			strcpy(linecopy, buffer);
			for (i=1; i <= fldcnt; i++) {
				j = remove_field(linecopy, fldcpy, 1);
				if ( j < 0 )
					derror(EX_SOFTWARE,"Field processing error");
				if ( strcmp(fldcpy, "\"\"") == 0 )
					k--;
			}
			if ( (k % 2) != 0 ) {
				(void) bline(cury[3]+1,curx[3]);
				if ((mvwprintw (active,cury[3]+1,curx[3], 
				   "<Warning:Odd number of fields specified!>",
				    hold)) == ERR)
				{
					derror(EX_SC,
					   "Error on write to active screen!");
				}
				wrefresh(active);
			}
		}
		/*(void) strcpy(expectstr, buffer);*/
		count++;

		(void) update(&Rules[Dialers],mem[num],buffer,2);
		(void) cleanu(menu,mem,phold);
		new = TRUE;

		continue;
	}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Dialers],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
	}
	if (buffer != NULL) {
		fldcnt = count_fields(buffer);
		/* If an odd number of fields are present, weed out null   */
		/* strings before alerting the user to a possible mismatch */
		k = fldcnt;
		if ( (fldcnt % 2) != 0 ) {
			strcpy(linecopy, buffer);
			for (i=1; i <= fldcnt; i++) {
				j = remove_field(linecopy, fldcpy, 1);
				if ( j < 0 )
					derror(EX_SOFTWARE,"Field processing error");
				if ( strcmp(fldcpy, "\"\"") == 0 )
					k--;
			}
			if ( (k % 2) != 0 ) {
				(void) bline(cury[3]+1,curx[3]);
				if ((mvwprintw (active,cury[3]+1,curx[3], 
				   "<Warning:Odd number of fields specified!>",
				    hold)) == ERR)
				{
					derror(EX_SC,
					   "Error on write to active screen!");
				}
				wrefresh(active);
				sleep(1);
			}
		}
		(void) strcpy(expectstr, buffer);
		count++;
	}

	if (count) {
		(void) bline(cury[3]+2,curx[3]);
		if ((mvwprintw (active,cury[3]+2,curx[3], "To next entry? (y or n) ==> "))== ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		while ((i = wgetch(active)) != 'y' && i != 'n')
					;
		(void) bline(cury[3]+2,curx[3]);
		if (i == 'y') {
		     num++;
		     if (num < count) 
		     {
			     (void) bmenu(menu);
		     }
		     else 
		     {
                          /* Going back to top set pointer back to last      */
                          /* record in case user wants to vi, add or delete. */
		          num--;
			  continue;
		     }
		}
		strcpy(typestr, entries[0]);
		goto again;
	}
    }  /* End while */
}
