static char sccsid[] = "@(#)82	1.2  src/bos/usr/bin/uucp/uucpadm/spec_files.c, cmduucp, bos411, 9428A410j 1/15/94 11:10:29";
/* 
 * COMPONENT_NAME: CMDUUCP spec_files.c
 * 
 * FUNCTIONS: spec_files, endofentry
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
extern WINDOW *active;


int specbase();
int bmenu();
int cleanu();
int dactive();
int do_vi();
int update();
int zaps();
int derror();
char *build();
char *input();
char *malloc();
char *srch();
extern int errno;
extern struct menu_ent option_devices;
extern struct menu_ent option_systems;
extern struct menu_ent option_dialers;
extern struct menu_ent option_spec_devices;
extern struct menu_ent option_spec_systems;
extern struct menu_ent option_spec_dialers;
extern char *help_token[];
extern char *sysfiletag[];
extern char *Filename[];
extern struct menu_ent *handoff[];

int spec_files (menu)
struct spec_shell  *menu;
{
     int  num;
     int  i;
     int  count;
     int  fileindex;
     int  new;
     char *phold = "-";
     int  len;
     char *buffer;
     char *lptr;
     char filename[80];		/* Contains name of file		*/
     char *cuptr;		/* Pointer to cu Sysfiles entry 	*/
     char *cicoptr;		/* Ptr to uucico Sysfiles entry 	*/
     int  err;			/* Return code from shell menu  	*/
     int  matchfound;		/* File listed in Sysfiles? (Boolean)   */
     int  filemode;		/* Mode of created configuration file	*/
     int  tempint;		/* Temporary integer for indexing data  */
     char tempstr[60];		/* Temporary string for copying data	*/
     char *temptr;		/* Temporary pointer for copying data   */
     int  cufilecount;		/* Count of cu Systems/Dialers/Devices files*/
     char custrs[20][80];	/* Holds names of cu config files      	*/
     int  custrs_index;		/* Index for loading custrs[][] array   */
     int  cicofilecount;	/* Count of uucico files        	*/
     char cicostrs[20][80];	/* Names of uucico files        	*/
     int  cicostrs_index;	/* Index for loading cicostrs[][] array */
     struct menu_ent *p;	/* Pointer to file modification shell	*/
     char *hold;
     char *mem[MEMSIZE];
     
     if (strcmp(menu->toks, "devices_spec") == 0)
         fileindex = Devices;
     else if (strcmp(menu->toks, "systems_spec") == 0)
         fileindex = Systems;
     else if (strcmp(menu->toks, "dialers_spec") == 0)
         fileindex = Dialers;
     else
         fileindex = Poll;  /* This is a random assignment -- just setting 
				a default value so things don't explode.
				This should never be reached.		*/

     for (i=0;i < MEMSIZE;i++)
          mem[i] = NULL;
     for (i=0;i < MAXENT;i++)
          entries[i][0] = '\0' ;
     new = TRUE;
     count = 0;
     num = 0;
     
     if (sysfile_present) {
          lptr=File[Sysfiles];
          /* Find the Sysfiles entry for cu */
          len=strlen("service=cu");
          cufilecount = 0;
          if ((cuptr = srch("service=cu",len,lptr,0,1,&Rules[Sysfiles])) 
                                             != NULL) {
               /* Find the cu entry if one exists. */
               custrs_index = 0;
               if (temptr = strstr(cuptr, sysfiletag[fileindex])) {
                    temptr += strlen(sysfiletag[fileindex]);
                    /* Count and copy the cu file names	*/
                    /* into the custrs[][] array.       */
                    tempint = 0;
          
                    /* Watch out for leading trash.  */
                    /* Some sysops may try anything! */
                    if (*temptr != ':' && !endofentry(*temptr))
                         cufilecount++;
          
                    while (!endofentry(*temptr)){
                         /* Check for file name separator */
                         if (*temptr != ':') {
                              /* If no separator, continue */
                              /* with the current string   */
                              custrs[custrs_index][tempint++] = 
                                             *temptr++;
                         }
                         else {
                              /* If a separator, terminate current string */
                              custrs[custrs_index++][tempint] = '\0';
     
                              /* Reset index for next filename */
                              tempint = 0; 
     
                              /* continue after separator      */
                              temptr++;    
          
                              /* If more data after the colon,   */
                              /* increment the device file count */
                              if (!endofentry(*temptr) && 
                                        *temptr != ':')
                                   cufilecount++;
                         }
                    }
                    /* Tack a null onto the last filename copied. This   */
                    /* wasn't done in the loop body like the others were.*/
                    custrs[custrs_index][tempint] = '\0';
               }
               else {
                    /* No cu entry exists for this file type.     */
                    /* Use the default.                           */
                    (void) strcpy(custrs[cufilecount++], Filename[fileindex]);
               }
          }
          else {
               /* No cu entry exists, period.  Use the default. */
               (void) strcpy(custrs[cufilecount++], Filename[fileindex]);
          }

          /* Find the Sysfiles entry for uucico */
          len=strlen("service=uucico");
          cicofilecount = 0;
          if ((cicoptr = srch("service=uucico",len,lptr,0,1,&Rules[Sysfiles])) 
                                             != NULL) {
               /* Find the uucico entry if one exists. */
               cicostrs_index = 0;
               if (temptr = strstr(cicoptr, sysfiletag[fileindex])) {
                    temptr += strlen(sysfiletag[fileindex]);
                    /* Count and copy the uucico file names */
                    /* into the cicostrs[][] array.         */
                    tempint = 0;
          
                    /* Watch out for leading trash.  */
                    /* Some sysops may try anything! */
                    if (*temptr != ':' && !endofentry(*temptr))
                         cicofilecount++;
          
                    while (!endofentry(*temptr)) {
                         /* Check for file name separator */
                         if (*temptr != ':') {
                              /* If no separator, continue */
                              /* with the current string   */
                              cicostrs[cicostrs_index][tempint++] = 
                                                *temptr++;
                         }
                         else {
                              /* If a separator, terminate  */
                              /* current string             */
                              cicostrs[cicostrs_index++][tempint] = '\0';
                              /* Reset index for next filename */
                              tempint = 0; 
     
                              /* continue after separator      */
                              temptr++;    
          
                              /* If more data after the colon,   */
                              /* increment the device file count */
                              if (!endofentry(*temptr) && *temptr != ':')
                                   cicofilecount++;
                         }
                    }
                    /* Tack a null onto the last filename copied. This   */
                    /* wasn't done in the loop body like the others were.*/
                    cicostrs[cicostrs_index][tempint] = '\0';
               }
               else {
                    /* No uucico entry exists for this file type. */
                    /* Use the default.                           */
                    (void) strcpy(cicostrs[cicofilecount++], 
					Filename[fileindex]);
               }
          }
          else {
               /* No uucico entry exists.  Use the default. */
               (void) strcpy(cicostrs[cicofilecount++], Filename[fileindex]);
          }
     }
     else { /* No Sysfiles file exists.  Use defaults for both uucico and cu */
          (void) strcpy(cicostrs[0], Filename[fileindex]);
          (void) strcat(cicostrs[0],
		"  (This is the default--No Sysfiles file exists!)");
          (void) strcpy(custrs[0], Filename[fileindex]);
          (void) strcat(custrs[0], 
		"  (This is the default--No Sysfiles file exists!)");
          cufilecount = cicofilecount = 1;
     }

     /* Copy filenames to be displayed to the menu structure */
     for (i=0; i < cicofilecount && i < FILEMAX; i++) {

          /* entry[4]..entry[8] hold the uucico filenames display copies */
          menu->entry[4+i] = cicostrs[i];
     }
     for (i=0; i < cufilecount && i < FILEMAX ; i++) {
 
          /* entry[12]..entry[16] hold the cu filenames display copies */
          menu->entry[12+i] = custrs[i];
     }

     /* Now display the menu with all the filenames from Sysfiles */
     (void) specbase(menu);

     while (1) {
          /* Cursor input postion to follow Specify prompt */
          curx[1] = strlen(menu->entry[0]) + 5;
          /* Position error messages just below user input area. */
          curx[2] = curx[1];
          /* Wait for filename to be input */
          buffer = input(help_token[fileindex],1,entries[0],0,0);
          (void) bline(cury[1]+1,curx[1]);
          if (strchr(buffer,VIEW)) {
               (void) bline(cury[2],curx[2]);
               if ((mvwprintw(active,cury[2],curx[2],"< Tilde character invalid. >  ")) == ERR)
                     derror(EX_SC,"Error on write to active screen!");
               continue;
          }
          if (strchr(buffer,CTLU) || strchr(buffer,CTLX)) {
               (void) bline(cury[2],curx[2]);
               if ((mvwprintw(active,cury[2],curx[2],"< Invalid control character!>  ")) == ERR)
                     derror(EX_SC,"Error on write to active screen!");
               continue;
          }
          if (strchr(buffer,CTLD) != NULL)
               return(EX_OK);
          if (buffer == NULL) {
               /* No entry. Check if we have cycled around. */
               if (strlen(entries[0]) == 0) {
                    /* No entry key. Exit with message. */
                    (void) bline(cury[2],curx[2]-20);
                    if (mvwprintw(active,cury[2],curx[2],"< Entry required! >") == ERR)
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
               /* We have some input.  Set up the file for */
               /* inspection/modification and invoke the   */
               /* file manipulation routine.               */

               /* If filename has an absolute path, just use it.*/
               if (buffer[0] == '/') {
                    (void) strcpy(filename, buffer);
                    lptr = filename; /* used in cross check of Sysfiles */
               }
               else { /* Prepend default path */
                    (void) strcpy(filename, "/etc/uucp/");
                    strcat(filename, buffer);
                    lptr = buffer; /* used in cross check of Sysfiles */
               }
               Rules[fileindex].name = filename;

               /* Inform user if filename not present in Sysfiles file */
               matchfound = 0;      
               for (i=0; i < cufilecount; i++) {
                    if (strcmp(lptr, custrs[i]) == 0)
                         matchfound = 1;      
               }
               for (i=0; i < cicofilecount; i++) {
                    if (strcmp(lptr, cicostrs[i]) == 0)
                         matchfound = 1;      
               }

               if (!matchfound) {
                    (void) strcpy(tempstr, " < Warning -- ");
                    (void) strcat(tempstr, lptr);
                    (void) strcat(tempstr, " not listed in Sysfiles! >");
                    (void) bline(cury[2],curx[2]-20);
                    if (mvwprintw(active,cury[2],curx[2]-20, tempstr) == ERR) {
                         derror(EX_SC,"Error on write to active screen!");
                         wrefresh(active);
                    }
                    wrefresh(active);
                    sleep(1);
               }
                    
               Rules[fileindex].fd = open (Rules[fileindex].name, O_RDWR);
               if (Rules[fileindex].fd < 0)  {
                    if (errno == ENOENT) {
			 (void) strcpy(tempstr, "< ");
			 (void) strcat(tempstr, filename);
			 (void) strcat(tempstr, 
					" file not found--creating...>");
                         (void) bline(cury[2],curx[2]-20);
                         if (mvwprintw(active,cury[2],curx[2]-20, tempstr) 
								    == ERR) {
                                   derror(EX_SC,"Error on write to active screen!");
                                   wrefresh(active);
                         }
                         else {
                              wrefresh(active);
                              /* Wait to let user read msg */
                              sleep(1);
                         }
                         Rules[fileindex].fd = open(Rules[fileindex].name, 
                                                           O_RDWR | O_CREAT); 
                         if (Rules[fileindex].fd < 0)  {
                              (void) strcpy(tempstr, "< Can't open ");
                              (void) strcat(tempstr, Filename[fileindex]);
                              (void) strcat(tempstr, " file! >");
                              (void) bline(cury[2],curx[2]-20);
                              if (mvwprintw(active,cury[2],curx[2]-20,
                                                            tempstr) == ERR)
                                   derror(EX_SC,"Error on write to active screen!");
                                   wrefresh(active);
                                   continue;
                         }
                         else {
                              /* Set proper owner and perms */
                              (void) bline(cury[2],curx[2]-20);
                              if (fileindex == Systems)
                                    filemode = 0400;
                              else 
                                    filemode = 0444;
                              if (i = fchmod (Rules[fileindex].fd, 
                                                           filemode) < 0 ) {
                                   if (mvwprintw(active,cury[2],curx[2],"<Can't set file permissions!>") == ERR)
                                   derror(EX_SC,"Error on write to active screen!");
                                   wrefresh(active);
                                   continue;
                              }
                              if (i = fchown (Rules[fileindex].fd, 5, 5) < 0 ) {
                                   (void) bline(cury[2],curx[2]-20);
                                   if (mvwprintw(active,cury[2],curx[2],"< Can't set file ownership! >") == ERR)
                                   derror(EX_SC,"Error on write to active screen!");
                                   wrefresh(active);
                                   continue;
                              }
                              if ((err = bload(&Rules[fileindex])) != EX_OK)
                                    derror (err,"Can't load file");
                         } /* Set owner and perms */
                    } /* File does not exist */
               } /* File open for read failed */
               else {
                        if ((err = bload(&Rules[fileindex])) != EX_OK)
                                derror (err,"Can't load file");
               }
               /*
                *  Process selection.
                */
               p = handoff[fileindex];
               if (p) {
                    err = (*p->proc) (p->parm1, p->parm2, p->parm3, p->parm4);
                    return (err);
               }
               else
                    derror (-1,"Handler routine not found\n");

          } /* if buffer != NULL */
     }  /* while (1) */
}
     
int endofentry(item)                            
     char item;                                 
{                                           
     if (item != ' ' && item != '\t' &&      
         item != '\n' && item != '\\' &&     
         item != '\0' )                      
          return(0);                      
     else                                 
          return(1);                      
}                                           
