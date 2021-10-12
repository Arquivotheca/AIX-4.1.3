static char sccsid[] = "@(#)14  1.9  src/bos/usr/bin/ate/menu.c, cmdate, bos411, 9437A411a 8/26/94 16:02:50";
/* 
 * COMPONENT_NAME: BOS menu.c
 * 
 * FUNCTIONS: MSGSTR, altermenu, mainmenu, modifymenu, prompt 
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

/****************************************************************************/
/*                                                                          */
/* Module name:      menu.c                                                 */
/* Description:      display connected and unconnected main menu, alter     */
/*                     menu, and modify menu.  Prompts user for inputs when */
/*                     necessary.  See menu.h for menu formats.             */
/* Functions:        mainmenu - display main menu (connected & unconnected) */
/*                   altermenu - display alter menu                         */
/*                   modifymenu - display modify menu                       */
/*                   prompt - print user prompts                            */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c (mainmenu,prompts 1-5)                */
/*                   alter in alter.c (altermenu)                           */
/*                   modify in modify.c (modifymenu,prompt 7)               */
/*   Receives:       prompt number                                          */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          cls in cls.c (clear screen)                            */
/*   Modifies:       userin, argp's, dirfile, sndfile, rcvfile, kapfile     */
/*                                                                          */
/****************************************************************************/

#include "modem.h"
#include "menu.h"
#include <stdlib.h>

static char nl = '\n';                      /* new line */

extern struct list *mmenu;

extern char *cline;
extern char *dline;
extern char *sline;
extern char *rline;
extern char *bline;
extern char *xline;
extern char *hline;
extern char *mline;
extern char *aline;
extern char *eline;
extern char *qline;

extern char *node, *mainh1, *mainh2, *item, *line, *command, *command2;
extern char *alterh, *modh, *enter1;

extern char *enter2, *enter3;

/* -------------------------------------------------------------------------
   print connected and unconnected main menu
   ------------------------------------------------------------------------- */
mainmenu()
{
 char s[81];
 unsigned int i;
 char fline[512];
 int rc;
 
#ifdef DEBUG
kk = sprintf(ss,"In mainmenu routine\n");
write(fe,ss,kk);
#endif DEBUG

rc = sprintf(fline,MSGSTR(KEYS,"     The following keys may be used during a connection:\n\
         ctrl %c start or stop recording display output\n\
         ctrl %c display main menu to issue a command\n\
     Use ctrl %c to return to a previous screen at any time\n"),
  capkey+0x60,cmdkey+0x60,retkey+0x60);

 cls();                                     /* clear screen */

 if (connect==OFF)                          /* unconnected header */
   {
   printf("%s%s%s", node, machine, mainh1);
   }
 else                                       /* connected header */
   {
   printf("%s%s%s", node, machine, mainh2);
   }

   printf("%s",line);                		/* line of dashes */
   printf("%s",command);          		/* header line */
 
 if (connect==OFF)                          /* no connection yet */
   {
   printf("     %s\t  %s", mmenu->items[0], cline);  /* connect line */
   printf("     %s\t  %s", mmenu->items[1], dline);    /* directory line */
   }
 else
   {
   printf("     %s\t  %s", mmenu->items[2], sline);  /* send line */
   printf("     %s\t  %s", mmenu->items[3], rline);  /* receive line */
   printf("     %s\t  %s", mmenu->items[4], bline);  /* break line */
   printf("     %s\t  %s", mmenu->items[5], xline);  /* terminate line */
   }
 fflush(stdout);
 write(1,&nl,1);                              /* new line */
 printf("     %s\t  %s", mmenu->items[6], hline);  /* help line */
 printf("     %s\t  %s", mmenu->items[7], mline);  /* modify line */
 printf("     %s\t  %s", mmenu->items[8], aline);  /* alter line */
 printf("     %s\t  %s", mmenu->items[9], eline);  /* perform (execute) line */
 printf("     %s\t  %s", mmenu->items[10], qline); /* quit line */
 fflush(stdout);
 write(1,line,strlen(line));                /* line of dashes */
 write(1,fline,rc);              	        /* function keys */
 write(1,line,strlen(line));                /* line of dashes */
 write(1,enter1,strlen(enter1));            /* "type command & enter" */

#ifdef DEBUG
kk = sprintf(ss,"leaving mainmenu\n");
write(fe,ss,kk);
#endif DEBUG
 
}
 
/* --------------------------------------------------------------------------
   print alter menu
   -------------------------------------------------------------------------- */
altermenu()
{
 unsigned int i;
 char s[1024];
 
 cls();
 printf("%s%s%s", node, machine, alterh);	/* alter header */
 printf(line);                			/* line of dashes */
 printf (command2);
 fflush(stdout);
 
 /* length, stop, parity, speed */
 printf ("%s\t   ", altrmenu->items[0]);
 printf (alterdesc->items[0], bits);
 printf ("%s\t   ", altrmenu->items[1]);
 printf (alterdesc->items[1], stop);
 printf ("%s\t   ", altrmenu->items[2]);
 printf (alterdesc->items[2], parity);
 printf ("%s\t   ", altrmenu->items[3]);
 printf (alterdesc->items[3], speed);

 /* port, prefix, suffix, wait, attempts */
 printf ("%s\t   ", altrmenu->items[4]);
 printf (alterdesc->items[4], port);
 printf ("%s\t   ", altrmenu->items[5]);
 printf (alterdesc->items[5], dialp);
 printf ("%s\t   ", altrmenu->items[6]);
 printf (alterdesc->items[6], dials);
 printf ("%s\t   ", altrmenu->items[7]);
 printf (alterdesc->items[7], redial);
 printf ("%s   ", altrmenu->items[8]);
 printf (alterdesc->items[8], attempts);

 /* transfer, pacing character */

 printf ("%s   ", altrmenu->items[9]);
 printf (alterdesc->items[9], transfer);
 printf ("%s  ", altrmenu->items[10]);
 printf (alterdesc->items[10], pacing);

 write(1,line,strlen(line));
 
 write(1,enter2,strlen(enter2));            /* "type command & Enter" */
}


/* -------------------------------------------------------------------------
   print modify menu
   ------------------------------------------------------------------------- */
 modifymenu()
 {
  cls();                                        /* clear screen */
  printf("%s%s%s", node, machine, modh);	/* modify header */
  printf("%s",line);                		/* line of dashes */
  printf (command2);

  printf ("%s\t   ", modmenu->items[0]);
  printf (moddesc->items[0], kapfile);
 
  printf ("%s  ", modmenu->items[1]);
  printf (moddesc->items[1], xlatcr ? on : off);
  printf ("%s\t   ", modmenu->items[2]);
  printf (moddesc->items[2], Echo ? on : off);
  printf ("%s\t   ", modmenu->items[3]);
  printf (moddesc->items[3], vt100 ? on : off);
  printf ("%s\t   ", modmenu->items[4]);
  printf (moddesc->items[4], capture ? on : off);
  printf ("%s   ", modmenu->items[5]);
  printf (moddesc->items[5], xctl ? on : off);
  printf ("%s   ", modmenu->items[6]);
  printf (moddesc->items[6], awrap ? on : off);

  write(1,line,strlen(line));
 
  write(1,enter3,strlen(enter3));           /* "enter command & Enter */
}

/* --------------------------------------------------------------------------
   prompt user for needed inputs if not provided via "fast path".  These used
   to be prints, but were changed to writes when the prints didn't come out
   correctly.  It's akward, but it works.
   -------------------------------------------------------------------------- */
prompt(i)
int i;
{
int j,m,counter=0;
unsigned int k;
char c,s[NL_TEXTMAX];                        /* string for writes */

 cls();                                     /* clear screen */
 strcpy(userin,EMPTY);
 strcat(userin,EMPTY);
 
 switch (i)
   {
   case 1 : k = sprintf(s,MSGSTR(PHNUM, "Type the phone number of the connection for auto dialing, or the name\n\
 of the port for direct connect, and press Enter.  To manually dial a\n\
 number, just press Enter.  To redial the last number (%s), type 'r'\n\
 and press Enter.\n"),number); /*MSG*/
            write(1,s,k);
            k = sprintf(s,"> ");
            write(1,s,k);
	    c = getc(stdin);
            if (retcode != NULL) return;    /* user interrupt */
	    while (c != '\n' && counter <= 80) {
		userin[counter++] = c;
		c = getc(stdin);
		}
            tcflush(0,TCIFLUSH);            /* throw out over 80 chars */
            if (retcode != NULL) return;    /* user interrupt */
            sscanf(userin,"%40s %40s",argp[1],argp[2]);

	    if (mb_cur_max > 1) {     	/* multibyte code */
		wchar_t wc;
		int mbcount;
	
		mbcount = mbtowc(&wc, argp[1], mb_cur_max);
		if (mbcount < 0) 
		    printf(MSGSTR(MENU_MB, "menu: Error in converting to widechar.\n"));
		else {
		    if (towupper((wint_t) wc) == L'R')	/* redial */
			strcpy(argp[1], number);/*store old number in argp[1]*/ 
		}
	    }	
  	    else {			/* single byte code */
		if (toupper(*argp[1]) == 'R') 	/* redial */
			strcpy(argp[1], number);/*store old number in argp[1]*/
	    }
            break;
            
   case 2 : k = sprintf(s,MSGSTR(DIRNAME, "Type the file name of the directory you want to display and press Enter.\n")); /*MSG*/
            write(1,s,k);
            k = sprintf(s,MSGSTR(DIRNAME2, "To use the current directory (%s), just press Enter.\n"),dirfile); /*MSG*/
            write(1,s,k);
            k = sprintf(s,"> ");
            write(1,s,k);
	    c = getc(stdin);
            if (retcode != NULL) return;    /* user interrupt */
	    while (c != '\n' && counter <= 80) {
		userin[counter++] = c;
		c = getc(stdin);
		}
            tcflush(0,TCIFLUSH);            /* throw out over 80 chars */
            if (retcode != NULL) return;    /* user interrupt */
            j = sscanf(userin,"%40s",argp[1]);
            if (j>0) strcpy(dirfile,argp[1]); /* store new dirfile name */
            break;
            
   case 3 : k = sprintf(s,MSGSTR(OSCMD, "Type an operating system command and press Enter.\n")); /*MSG*/
            write(1,s,k);
            k = sprintf(s,"> ");
            write(1,s,k);
	    c = getc(stdin);
            if (retcode != NULL) return;    /* user interrupt */
	    while (c != '\n' && counter <= 80) {
		userin[counter++] = c;
		c = getc(stdin);
		}
            tcflush(0,TCIFLUSH);            /* throw out over 80 chars */
            if (retcode != NULL) return;    /* user interrupt */
            break;
            
   case 4 : k = sprintf(s,MSGSTR(FNAME, "Type the name of the file you wish to send and press Enter.  To use\n\
the last file name (%s), just press Enter.\n"),sndfile); /*MSG*/
            write(1,s,k);
            k = sprintf(s,"> ");
            write(1,s,k);
	    c = getc(stdin);
            if (retcode != NULL) return;    /* user interrupt */
	    while (c != '\n' && counter <= 80) {
		userin[counter++] = c;
		c = getc(stdin);
		}
            tcflush(0,TCIFLUSH);            /* throw out over 80 chars */
            if (retcode != NULL) return;    /* user interrupt */
            j = sscanf(userin,"%40s",argp[1]);
            if (j>0) strcpy(sndfile,argp[1]); /* store new sendfile name */
            break;

   case 5 : k = sprintf(s,MSGSTR(FNAMES, "Type the name of the file you wish to store the received data in and\n\
press Enter.  To use the last file name (%s), just\n\
press Enter.\n"),rcvfile); /*MSG*/
            write(1,s,k);
            k = sprintf(s,"> ");
            write(1,s,k);
	    c = getc(stdin);
            if (retcode != NULL) return;    /* user interrupt */
	    while (c != '\n' && counter <= 80) {
		userin[counter++] = c;
		c = getc(stdin);
		}
            tcflush(0,TCIFLUSH);            /* throw out over 80 chars */
            if (retcode != NULL) return;    /* user interrupt */
            j = sscanf(userin,"%40s",argp[1]);
            if (j>0) strcpy(rcvfile,argp[1]); /* store new rcvfile name */
            break;
            
   case 7 : k = sprintf(s,MSGSTR(KFILE, "Type the name of the file you wish to store the captured data in and\n\
press Enter.  To use the last file name (%s), just\n\
press Enter.\n"), kapfile); /*MSG*/
            write(1,s,k);
            k = sprintf(s,"> ");
            write(1,s,k);
            strcpy(argp[0],EMPTY);
	    c = getc(stdin);
            if (retcode != NULL) return;    /* user interrupt */
	    while (c != '\n' && counter <= 80) {
		userin[counter++] = c;
		c = getc(stdin);
		}
            tcflush(0,TCIFLUSH);            /* throw out over 80 chars */
            if (retcode != NULL) return;    /* user interrupt */
            j = sscanf(userin,"%40s",argp[0]);
            if (j < 0) return; 
            strcpy(kapfile,argp[0]);        /* store new capture file name */
            break;
            
   default: printf(MSGSTR(INVPR, "Invalid prompt number %d passed to prompt routine\n"),i); /*MSG*/
            break;
   }
}
