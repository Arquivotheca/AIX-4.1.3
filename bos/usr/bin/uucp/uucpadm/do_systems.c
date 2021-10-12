static char sccsid[] = "@(#)72	1.4.2.2  src/bos/usr/bin/uucp/uucpadm/do_systems.c, cmduucp, bos411, 9428A410j 8/3/93 16:15:09";
/* 
 * COMPONENT_NAME: CMDUUCP do_systems.c
 * 
 * FUNCTIONS: do_systems, remove_field, add_field, count_fields
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
char *getcpy();
char *getncpy();
char *malloc();
char *search();
char *srch();
char *cyphert();

int do_systems (menu)
struct menu_shell  *menu;
{
int  num;
int  i, j;
int  count;
int  new;
int  len, len2;
char *buffer;
char *lptr;
char *hold;
char *phold = "-";
char *mem[MEMSIZE];
char linecopy[400];			/* Copy of entry from Systems file */
int field_count;		/* Count of fields in a Systems entry */
char pprompt[15], lprompt[40];	/* Login and passwd prompts */
char seq_fld[20];		/* Temporary login sequence field holder */
int login_seq_cnt;		/* Number of login sequence sub-fields */

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
	buffer = input("nameS",1,entries[0],0,0);
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
		(void) do_vi(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Systems],mem[num],buffer,0);
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
	lptr = File[Systems];
	while ((lptr = srch(entries[0],len,lptr,0,1,&Rules[Systems])) != NULL) {
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
	if (count) {
		/* Take out the login and password prompt info 	  */
		/* before letting cypher build the entries struct */
		(void) strcpy(linecopy, mem[num]);
		field_count = count_fields(linecopy);
		i = remove_field(linecopy, pprompt, field_count-1);
		if ( i < 0 )
			derror(EX_SOFTWARE,"Field processing error");
		login_seq_cnt = field_count - 8;
		lprompt[0] = '\0';
		for (i=0; i < login_seq_cnt ; i++) {
			j = remove_field(linecopy, seq_fld, field_count-3-i);
			if ( j < 0 )
				derror(EX_SOFTWARE,"Field processing error");
			add_field(lprompt, seq_fld, 1);
		}
		(void) cypher(linecopy,&Rules[Systems],menu->counts);
		add_field(entries[5], lprompt, 1);
		add_field(entries[6], pprompt, 1);
	}
	bline(cury[1]+1,curx[1]);
/* Need to breakout password field will attack it with reverse
*  polish notation.
*/
	if ((buffer = strrchr(entries[menu->counts - 2],':')) != NULL) 
		if ((*--buffer) == 'd' || *buffer == 'D') {
/* Gotit! Blow the air out. */
		while(strcspn(buffer,Rules[Systems].delimit))
				buffer--;
				buffer++;
		(void) strcpy(entries[menu->counts -1],buffer);
				buffer--;
		while(strspn(buffer,Rules[Systems].delimit))
				buffer--;
		*++buffer = '\0';
				 	}

	if (count > 1) {
		bline(cury[1]+1,curx[1]);
		if (mvwprintw(active,cury[1]+1,curx[1],"<Warning: Multiple entries exist with this system name!>") == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	(void) dmenu(count,num,menu);
	buffer = input("timeS",2,entries[1],0,0);
	bline(cury[2]+1,curx[2]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Systems],mem[num],buffer,1);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[1],buffer);
		}
/* Have some input. Pass to cyphert for validity check. */
	buffer = cyphert(entries[1]);
	if((buffer == NULL || strcmp(buffer,entries[1])) && strcmp(phold,entries[1])) {
		(void) strcpy(entries[1],buffer);
		bline(cury[2],curx[2]);
		if (mvwprintw(active,cury[2],curx[2],"%s",entries[1]) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		bline(cury[2]+1,curx[2]);
		if (mvwprintw(active,cury[2]+1,curx[2],"<Warning: Invalid time entry. Valid portion displayed!>") == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	buffer = input("typeS",3,entries[2],0,0);
	bline(cury[3]+1,curx[3]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Systems],mem[num],buffer,2);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[2],buffer);
		}
/* Check for Devices entry. */
	if ((hold = malloc((unsigned) strlen(entries[2]) +1)) == NULL)
		derror(EX_SOFTWARE,"Unable to malloc storage");
	(void) strcpy(hold,entries[2]);
	if (strchr(hold,','))
		*strchr(hold,',') = '\0';
	len = strlen(hold);
	if (search(hold, Devices, 1) == NULL) {
		bline(cury[3]+1,curx[3]);
		if ((mvwprintw (active,cury[3]+1,curx[3], "<Warning:Type %s not found in Devices!>",hold)) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	buffer = input("classS",4,entries[3],0,0);
	bline(cury[4]+1,curx[4]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Systems],mem[num],buffer,3);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[3],buffer);
		}
	len2 = strlen(entries[3]);
	if (((search(hold,Devices,1) == NULL)) || \
	    ((search(entries[3],Devices,4) == NULL))) {
		bline(cury[4]+1,curx[4]);
		if ((mvwprintw (active,cury[4]+1,curx[4], "<Warning: %s %s entries not found in Devices!>",hold,entries[3])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	free((void *) hold);
	buffer = input("phoneS",5,entries[4],0,0);
	bline(cury[5]+1,curx[5]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Systems],mem[num],buffer,4);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		(void) strcpy(entries[4],buffer);
		}
	if ((strcmp(entries[3],"ACU") == 0) && strcmp(entries[4],phold)) {
		bline(cury[5]+1,curx[5]);
		if ((mvwprintw (active,cury[5]+1,curx[5], "<Warning: %s is not ACU!>",entries[3])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		}
	if ((buffer != NULL) && (sscanf(entries[4],"%[A-Za-z]",buffer) > 0)) {
		len = strlen(buffer);
		if (srch(buffer,len,File[Dialcode],0,1,&Rules[Dialcode]) == NULL) {
		bline(cury[5]+1,curx[5]);
		if ((mvwprintw (active,cury[5]+1,curx[5], "<Warning: Dialcode %s not found in Dialcodes!>",entries[4])) == ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
			}
		}
	buffer = input("loginS",6,entries[5],1,0);
	bline(cury[6]+1,curx[6]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		(void) update(&Rules[Systems],mem[num],buffer,5);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		entries[5][0] = '\0';
		if (!count) {
/* This is an add. Glue on appropriate prefix. Prefix may be
*  modified in change mode.
*/
			if (strcmp(entries[2],"ACU")) 
				(void) strcat(entries[5],"in:--in: "); 
			else
				(void) strcat(entries[5],"\"\" \\r\\d\\r\\d\\r in:--in: "); 
				}
		(void) strcat(entries[5],buffer);
		}
	buffer = input("passwdS",7,entries[6],1,0);
	bline(cury[7]+1,curx[7]);
	if (strchr(buffer,CTLD) != NULL)
		return(EX_OK);
	if (strchr(buffer,CTLX) != NULL) {
		(void) zaps(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,CTLU) != NULL) {
		if (!count) {
			/* This is an add. Glue on appropriate prefix.	*/
			/* Prefix may be modified in change mode.	*/
			add_field(buffer, "word: ", 1); 
		}
		(void) strcat(entries[6],buffer);
		(void) update(&Rules[Systems],mem[num],buffer,6);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (strchr(buffer,VIEW) != NULL) {
		(void) do_vi(&Rules[Systems],mem[num]);
		(void) cleanu(menu,mem,phold);
		new = TRUE;
		continue;
		}
	if (buffer != NULL) {
		entries[6][0] = '\0';
		if (!count) 
/* This is an add. Glue on appropriate prefix. Prefix may be
*  modified in change mode.
*/
			(void) strcat(entries[6],"word: "); 
		(void) strcat(entries[6],buffer);
		}
	if (count) {
		bline(cury[7]+2,curx[7]);
		if ((mvwprintw (active,cury[7]+2,curx[7], "To next entry? (y or n) ==> "))== ERR)
			derror(EX_SC,"Error on write to active screen!");
		wrefresh(active);
		while ((i = wgetch(active)) != 'y' && i != 'n')
					;
		bline(cury[7]+2,curx[7]);
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

int remove_field(line, field, n)
	char *line, *field;
	int n;
{
	/* remove the nth field of a newline-or-null terminated line. Fields */
	/* are separated by blanks or tabs and numbered starting from one.   */
	/* The original line is modified and a copy of the field removed     */
	/* is placed into the field string and terminated with a null char.  */
	char	*ptr;		/* Current char in line */
	char	*ptr2;		/* temporary char ptr	*/
	int	field_num;	/* Current field in line */
	int	fld_len;    	/* Length of field rmvd  */

 	/* Find start of first field */
	ptr=line;
	while (*ptr == ' ' || *ptr == '\t')  {
		ptr++;
	}

	field_num = 1;
	while (field_num != n) {
		while (*ptr != ' ' && *ptr != '\t') { /* Skip to end of field */
			ptr++;
			if (*ptr == '\n' || *ptr == '\0')
				return(-1);
		}
		while ( *ptr == ' ' || *ptr == '\t') /* Skip to next field */
		{
			ptr++;
			if (*ptr == '\n' || *ptr == '\0')
				return(-1);
		}
		field_num++;
	}
	if ( field_num == n ) {
		ptr2 = ptr; /* ptr2 will be set to next field */
		/* First find end of current field */
		while (*ptr2 != ' ' && *ptr2 != '\t') { 
			ptr2++;
			if (*ptr2 == '\n' || *ptr2 == '\0')
				break;
		}

		/* ptr2 is now pointing to the character  */
		/* following the end of the field of 	  */
		/* interest. Save the field.	 	  */
		fld_len = (int) ptr2 - (int) ptr;
		(void) strncpy(field, ptr, fld_len);
		field[fld_len] = '\0';
		
		/* Now find start of next field */
		if (*ptr2 != '\n' && *ptr2 != '\0') {
			while ( *ptr2 == ' ' || *ptr2 == '\t') {
				ptr2++;
			}
		}

		/* Now remove the field from the original line */
		if  (*ptr2 == '\n' || *ptr2 == '\0')
			*ptr = *ptr2 ;
		else {
			while ( *(ptr2-1) != '\n' && *(ptr2-1) != '\0') {
		  		*ptr++ = *ptr2++;
			}
		}
		return(0);
	}
	else
		return(-1);
}

int count_fields(line)
	char *line;
{
	char *ptr;	/* Current position in line */
	int  fld_cnt;	/* Count of fields in line  */

	/* Count the number of character fields in a record whose */
	/* fields are separated by blanks or tabs		  */

 	/* Find start of first field */
	ptr = line;
	while (*ptr == ' ' || *ptr == '\t')  {
		ptr++;
		if (*ptr == '\n' || *ptr == '\0') 
			return(0);
	}

	fld_cnt = 0;
	while ( *ptr != '\n' && *ptr !='\0' ) {       /* Process entire line  */
		while (*ptr != ' ' && *ptr != '\t') { /* Skip to end of field */
			ptr++;
			if ( *ptr == '\n' || *ptr =='\0' ) 
				break;
		}
		while ( *ptr == ' ' || *ptr == '\t') /* Skip to next field */
		{
			ptr++;
			if (*ptr == '\n' || *ptr == '\0')
				break;
		}
		fld_cnt++;
	}
	return(fld_cnt);
}

add_field(line, field, n)
	char *line, *field;
	int n;
{
	/* add field as the nth field of a newline-or-null terminated 	*/
	/* line. Fields are separated by blanks or tabs. 		*/
	char	*ptr;		/* Current char in line */
	char	tempstr[160];	/* temporary holder 	*/
	int	field_num;	/* Current field in line */

 	/* Find start of first field */
	ptr=line;
	while (*ptr == ' ' || *ptr == '\t')  {
		ptr++;
	}

	/* If nth field not found, we're outta here */
	if (*ptr == '\n' || *ptr == '\0') 
		if (n != 1)
			return(-1);
		else {
			(void) strcat(line, field);
			return(0);
		}

	field_num = 1;
	while (field_num != n) {
		while (*ptr != ' ' && *ptr != '\t') { /* Skip to end of field */
			ptr++;
			if (*ptr == '\n' || *ptr == '\0') {
				if ( n == field_num + 1 ) {
					tempstr[0] = ' ';
					tempstr[1] = '\0';
					(void) strcat(tempstr, field);
					(void) strcat(line, tempstr);
					return(0);
				}
				else {
					return(-1);
				}
			}
		}
		while ( *ptr == ' ' || *ptr == '\t') /* Skip to next field */
		{
			ptr++;
			if (*ptr == '\n' || *ptr == '\0')
				return(-1);
		}
		field_num++;
	}
	if ( field_num == n ) {
		(void) strcpy(tempstr, ptr);
		*ptr = '\0';
		(void) strcat(field," ");
		(void) strcat(ptr, field);
		(void) strcat(ptr, tempstr);
		return(0);
	}
	else
		return(-1);
}
