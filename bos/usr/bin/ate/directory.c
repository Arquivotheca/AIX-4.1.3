static char sccsid[] = "@(#)06  1.8  src/bos/usr/bin/ate/directory.c, cmdate, bos411, 9428A410j 6/10/91 16:26:07";
/* 
 * COMPONENT_NAME: BOS directory.c
 * 
 * FUNCTIONS: MSGSTR, ckentry, direc, readfile, token 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <nl_types.h>
#include "ate_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ATE,n,s) 

#include <stdlib.h>
/****************************************************************************/
/*                                                                          */
/* Module name:      directory.c                                            */
/* Description:      read a user's dialing directory file, display it on    */
/*                     the screen, check validity of items in the entry     */
/*                     selected by the user, call conn() in connect.c to    */
/*                     establish a connection, and redial as requested.     */
/* Functions:        direc - display dialing directory, get user's entry    */
/*                     selection, check items for validity, call conn()     */
/*                     and redial as needed                                 */
/*                   readfile - read a new dialing directory                */
/*                   ckentry - is entry user selected a valid number?       */
/*                   token - get each token out of directory file line      */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c                                       */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: user input is Enter or 'e' or 'E'                      */
/*                   user interrupt                                         */
/*                   bad directory file (doesn't exist, is a directory)     */
/*                   bad item in a selected directory entry                 */
/*   Calls:          cls in cls.c (clears screen)                           */
/*                   ckrate, cklength, ckstop, ckparity, cktoggle in        */
/*                     check.c (checks inputs)                              */
/*                   message in message.c (print user errors)               */
/*                   conn in connect.c (establish a connection)             */
/*   Modifies:       userin, argp's, speed, bits, stop, parity, xlatcr,     */
/*                     retcode, state, errno, Echo                          */
/*                                                                          */
/****************************************************************************/

#include "modem.h"

#define DIRMSZ 20                           /* directory maximum size */

extern char *direntr, *exitstr;		    /* menu entry pointers */

static char oldfile[81];                    /* name of dialing directory */
static dirsz;                               /* directory size */
static struct dirs                          /* directory structure */
  {
  char dire[81];                            /* directory entry */
  char name[21];                            /* name */
  char tele[41];                            /* telephone number */
  char spd[6];                              /* speed */
  char bits[2];                             /* bits (word size) */
  char stop[2];                             /* stop bits */
  char par[2];                              /* parity */
  char Echo[2];                             /* echo */
  char lfs[2];                              /* line feeds */
  } dira[DIRMSZ];

/* ------------------------------------------------------------------------
   direc().  This routine compares the dialing directory name stored in
   dirfile with the most recent directory name (in oldfile).  If the name
   has changed, it reads the new file.  Otherwise it uses the data it stored
   when it last read the file.
   
   Notice that if a user has bad data in their file, and they use the perform
   command from the main menu to go into an editor and correct it, the new
   data will NOT be read unless they also change the file name.
   ------------------------------------------------------------------------ */
direc()
{
 int i,j,count;

#ifdef DEBUG
kk = sprintf(ss,"entering direc with directory=%s\n",dirfile);
write(fe,ss,kk);
#endif DEBUG

 state=2;
 retcode=(int)NULL;

 if (strcmp(oldfile,dirfile) != 0)          /* new file to read */
   readfile();

 while(retcode==(int)NULL)
   {
   tcflush(0, TCIFLUSH);                    /* read extraneous characters up */
   for (i=0; i<26; i++)
     strcpy(argp[i],EMPTY);
     
   cls();                                   /* clear screen */
   
   printf(MSGSTR(DIRHDR, " #        NAME           TELEPHONE (first digits)  RATE  LEN STOP PAR ECHO LF's\n")); /*MSG*/
   printf(MSGSTR(DIRDASH, "-- -------------------- -------------------------- ----- --- ---- --- ---- ----\n")); /*MSG*/
   
   for(i=0; i<dirsz; i++)                   /* print the directory */
     {
     write(1,dira[i].dire,sizeof(dira[i].dire));
     }
   
   printf("%s%s", direntr, exitstr);	    /* print user prompt */
   fflush(stdout);

   strcpy(userin,EMPTY);                     /* clear input variable */
   strcat(userin,EMPTY);
   read(0,userin,40);
   tcflush(0, TCIFLUSH);                    /* throw away chars over 40 */
   i = sscanf(userin,"%40s",argp[0]);
   if (i < 0) return;                       /* if user input is Enter, leave */

   if (mb_cur_max > 1) {  		/* multibyte code */ 
	wchar_t wc1, wc2;
	int mbcount1, mbcount2;
	
	mbcount1 = mbtowc(&wc1, exitstr[0], mb_cur_max);
	mbcount2 = mbtowc(&wc2, userin[0], mb_cur_max);
	if (mbcount1 < 0 || mbcount2 < 0) {
		printf(MSGSTR(DIR_MB, "direc: error in converting to widechar\n")); 
		return;
	}
	if (towupper((wint_t) wc1) == towupper((wint_t) wc2))
		return;			/* if user input is 'e', leave */
   }	 
   else {				/* single byte code */
	if (toupper(exitstr[0]) == toupper(userin[0]))
		return;                /* if user input is 'e', leave */
   }	 	
	
   strcpy(userin,argp[0]);
   CKRET                                    /* check for user interrupt */
     
   if ((i=ckentry(userin))==ERROR) continue;   /* bad entry - goto bottom of loop */
   
   j = 0;                                   /* check all items before msg */
   if (ckrate(dira[i].spd)   ==ERROR) j=ERROR;  /* bad baud rate */
   if (cklength(dira[i].bits)==ERROR) j=ERROR;  /* bad word length */
   if (ckstop(dira[i].stop)  ==ERROR) j=ERROR;  /* bad stop bits */
   if (ckparity(dira[i].par) ==ERROR) j=ERROR;  /* bad parity */
   if (cktoggle(dira[i].Echo,'E')==ERROR) j=ERROR; /* bad echo */
   if (cktoggle(dira[i].lfs,'L') ==ERROR) j=ERROR; /* bad linefeeds */
   if (j == ERROR)
     {
     message(49);                           /* bad entry.  Correct directory */
     return;                                /* return user to main menu to quit */
     }                                      /*   or manually dial */
   
   strcpy(argp[1],dira[i].tele);            /* telephone number */
   strcpy(number,dira[i].tele);             /* telephone number */
   speed  = atoi(dira[i].spd);              /* baud rate */
   bits   = atoi(dira[i].bits);             /* character size */
   stop   = atoi(dira[i].stop);             /* stop bits */
   parity = atoi(dira[i].par);              /* parity */
   Echo   = atoi(dira[i].Echo);             /* echo */
   xlatcr = atoi(dira[i].lfs);              /* line feeds */

   if (bits == 8 && parity != 0)
     {
     message(54);                           /* parity off when using 8 bits */
     parity = 0;
     }
   
   if (retcode==(int)NULL) conn();               /* establish connection */
   
   count = 0;                               /* number of redial attempts */
   while (retcode==(int)NULL && count<attempts)
     {
     sleep((unsigned int)redial);           /* delay between redialing tries */
     count++;
     if (retcode==(int)NULL) conn();             /* check for user interrupt */
     }
     
   }                                        /* end of "while retcode == NULL" */
   
#ifdef DEBUG
kk = sprintf(ss,"leaving directory\n");
write(fe,ss,kk);
#endif DEBUG

}

/*----------------------------------------------------------------------*/
/* get each token out of directory file line.				*/
/*----------------------------------------------------------------------*/
char *
token(p)
	char *p;
{
  char *q;

  while (isspace(*p)) p++;	/* skip over leading white space */
  if (*p==(char)NULL) return(NULL);	/* end of buffer */
  q=p;
  while (! isspace(*q)) q++;	/* move to next white space (end of token) */
  *q='\0';			/* put an EOS at token end */
  return(p);
}


/* --------------------------------------------------------------------------
   read directory file.  This routine first opens the file so that if there are
   problems, perror can be used to inform the user what they are.  It then
   checks the file status to be sure it isn't a directory.  If all is well,
   it closes the file and reopens it as a file stream.
   
   The file is read a line at a time.  The first item, the name, is then read
   from the line a character at a time until a blank is encountered.  This
   allows the user to exceed the 20-character limit without affecting the
   remaining items.  The rest of the line is taken apart using a format, and
   items longer than the format overflow into the next item.
   
   The disassembled line is then reassembled into a formatted entry for
   displaying on the screen.
   -------------------------------------------------------------------------- */
readfile()
{
 
 char *cp;
 int i,j;
 struct stat buf;
 
#ifdef DEBUG
kk = sprintf(ss,"Entering readfile\n");
write(fe,ss,kk);
#endif DEBUG

 for (i=0; i<=81; i++)              /* clear out first entry of directory */
   dira[0].dire[i]=0;

 j = (stat (dirfile,&buf));                 /* get the file status */
 if (j == ERROR ||                            /* status failed */
    (i=open(dirfile,O_RDONLY))==ERROR)        /* open failed */
   {
   if (j == ERROR) errno=2;                   /* system msg=no such file */
   message(12);                             /* can't open dialing directory */
   retcode=EXIT;                            /* set retcode to leave direc() */
   return;                                  /* return to direc() */
   }
 else if (buf.st_mode & S_IFDIR)            /* if it's a directory, inform user */
   {
   errno=21;                                /* system msg=is a directory */
   message(12);                             /* can't open dialing directory */
   retcode=EXIT;                            /* set retcode to leave direc() */
   return;                                  /* return to direc() */
   }
 else                                       /* all is well */
   {
   close(i);                                /* close dialing directory */
   fd=fopen(dirfile,"r");                   /* re-open as a file stream */
   
#ifdef DEBUG
kk = sprintf(ss,"reading %s\n",dirfile);
write(fe,ss,kk);
#endif DEBUG

   strcpy(oldfile,dirfile);                 /* save file name */
   dirsz=0;

   for(i=0;i<=DIRMSZ;i++)                   /* read the directory entries */
     {

     if ((cp=fgets(userin,sizeof(userin),fd)) != NULL) /*not EOF*/
       {
       if (i == DIRMSZ)                     /* directory has > 20 entries */
         {
         message(57);                       /* only first 20 will be used */
         break;                             /* stop reading file */
         }
       strcpy(dira[i].name,"                    ");
       strcpy(dira[i].tele,EMPTY);
       strcpy(dira[i].spd, "     ");        /* clear variables */
       strcpy(dira[i].bits," ");
       strcpy(dira[i].stop," ");
       strcpy(dira[i].par, " ");
       strcpy(dira[i].Echo," ");
       strcpy(dira[i].lfs, " ");
       dirsz=i+1;

        userin[strlen(userin)]=' ';	/* add space on end for whitespace */
        userin[strlen(userin)+1]='\0';	/* add EOS to terminate string */

	if (userin[0]=='#') 		/* this is a comment line */
  	   {
	   i--;				/* don't count this as an entry */
	   continue;			/* go back to for loop */
	   }

	cp=token(cp);
	if (cp==NULL) 			/* must be a blank line */
 	   {
	   i--;				/* don't count this as an entry */
	   continue;			/* go back to for loop */
	   }
	strncpy(dira[i].name,cp,20);	/* directory name */
 	cp+=strlen(cp)+1;		/* move to next token */

	cp=token(cp);
	if (cp==NULL) goto printline;	/* ran out of tokens */
	strncpy(dira[i].tele,cp,40);	/* telephone number */
	cp+=strlen(cp)+1;		/* move to next token */

	cp=token(cp);
	if (cp==NULL) goto printline;	/* ran out of tokens */
	strncpy(dira[i].spd,cp,5);	/* speed */
	cp+=strlen(cp)+1;		/* move to next token */

	cp=token(cp);
	if (cp==NULL) goto printline;	/* ran out of tokens */
	strncpy(dira[i].bits,cp,1);	/* bits */
	cp+=strlen(cp)+1;		/* move to next token */

	cp=token(cp);
	if (cp==NULL) goto printline;	/* ran out of tokens */
	strncpy(dira[i].stop,cp,1);	/* stop bits */
	cp+=strlen(cp)+1;		/* move to next token */

	cp=token(cp);
	if (cp==NULL) goto printline;	/* ran out of tokens */
	strncpy(dira[i].par,cp,1);	/* parity */
	cp+=strlen(cp)+1;		/* move to next token */

	cp=token(cp);
	if (cp==NULL) goto printline;	/* ran out of tokens */
	strncpy(dira[i].Echo,cp,1);	/* echo */
	cp+=strlen(cp)+1;		/* move to next token */

	cp=token(cp);
	if (cp==NULL) goto printline;	/* ran out of tokens */
	strncpy(dira[i].lfs,cp,1);	/* line feeds */

printline: 				/* reassemble for printing */
	sprintf(dira[i].dire, "%2d %-20.20s %-26.26s %5.5s %3.3s %4.4s %3.3s %4.4s %4.4s\n",i,
 	dira[i].name, dira[i].tele, dira[i].spd, dira[i].bits,
        dira[i].stop, dira[i].par, dira[i].Echo, dira[i].lfs);
       }
     else                                   /* end of directory */
         break; 
     }                                      /* end for */
   fclose(fd);                              /* close the directory */
   }                                        /* end else */
   
#ifdef DEBUG
kk = sprintf(ss,"Leaving readfile\n");
write(fe,ss,kk);
#endif DEBUG

}



/* -------------------------------------------------------------------------
   check entry.  This routine checks that any entry number is valid.  It is
   passed a pointer to the user's input string and returns ERROR or the valid
   entry integer.
   ------------------------------------------------------------------------- */
ckentry(q)
char q[];
{
 int tmpr,error=0;
 char *p;

#ifdef DEBUG
kk = sprintf(ss,"entering ckentry\n");
write(fe,ss,kk);
#endif DEBUG

 p = q;                                     /* p = user input */
 while (*p != '\0' && *p != ' ')            /* step thru input, char x char */
   {
   if (isdigit(*p) != 0) p++;               /* if it's a digit, continue */
   else
     {
     error = 1;                             /* otherwise set error to TRUE */
     break;                                 /* goto end of while */
     }
   }

 tmpr = atoi(q);                            /* store input as a digit */
 if (tmpr < 0 || tmpr >= dirsz || strcmp(q,"") == 0)  /* valid number ? */
   error = 1;                               /* if not, set error to TRUE */

 if (error)
   {
   strcpy(argp[25],q);                      /* store input for message print */
   message(13);                             /* invalid entry number */
   return(ERROR);
   }
 else return(tmpr);                         /* return good integer */
}
