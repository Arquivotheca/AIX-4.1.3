static char sccsid[] = "@(#)23        1.11  src/bos/usr/bin/ate/portrw.c, cmdate, bos411, 9428A410j 6/10/91 16:27:25";
/* 
 * COMPONENT_NAME: BOS portrw.c
 * 
 * FUNCTIONS: MSGSTR, portinit, portread, redraw 
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
/* Module name:      portrw.c                                               */
/* Description:      Process port input.				    */
/*                   This process is responsible for the initializing the   */
/* 		     port, data capture, and redrawing the screen after     */
/*		     a menu display. 					    */
/*                   The PC macro was created during code optimization so   */
/*		     it would be in line.				    */
/*                                                                          */
/* Functions:        redraw - repaint the connection screen after a menu    */
/*                   portinit - initialize screen size                      */
/*                   portread - port read/write routine                     */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    xmodem.h - global variables                            */
/*   Called by:      conn in connect.c                                      */
/*                   sigrout in signal.c                                    */
/*                   main in main.c                                         */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          cls in cls.c (clear screen)                            */
/*                   message in message.c (display user messages)           */
/*   Modifies:       bufr, sbuf, txti                                       */
/*                                                                          */
/****************************************************************************/

#include "modem.h"
#include <stdlib.h>

wchar_t 
convrt(a_char)		/* convert a multibyte or byte to a widechar; */
char a_char;		/* used below */
{ 
	wchar_t wc; 
	mbtowc(&wc, &a_char, mb_cur_max); 
	return(wc); 
}

/* macro to filter incoming CR's and CR-LF's to LF's only in capture file */
static char k=0;		/* trailing char for CR-LF filtering */
#define PC        if (*ptr=='\n' && k=='\r')k='\n'; /* CR-LF => do nothing */ \
                  else { k = *ptr;            /* incr trailing ptr */ \
                    if (*ptr=='\r') bufr[txti++]='\n'; /* CR=LF */ \
		    else if(k == 010) { txti-- ; if((int)txti < 0) txti =0 ;}\
		    else if(k==025){ for(;(int)txti>0 && bufr[txti]!='\n';txti--);}\
		    else if(!(iswprint((wint_t) convrt(k)) || iswspace((wint_t) convrt(k)))); \
                    else bufr[txti++]=k; }  /* store character */

extern char  *getenv(), *termdef(), *setupterm();
struct winsize win; 		/* terminal window size */

/* screen store */
int size;			/* size of screen buffer */
/* char *sbuf                      screen buffer memory ptr - modem.h */
char swrp;                      /* screen wrap indicator */
char *scur;                     /* screen cursor position */
char *sbufe;                    /* end of screen buffer */

int Lines,Cols;			/* Lines and Columns  */
int i,j;			/* temporary variables */



/* -------------------------------------------------------------------------
   port read/write initialization routine. (initialize screen size)  
   ------------------------------------------------------------------------- */
portinit()
{

register char *lcp,*ccp;                    /* line count and col count ptrs */
 
 /*--------------------------------------------------------------------------*/
 /* termdef returns the current line size and column size of the console.    */
 /* The Lines and columns can vary based on the font selected by the user.   */
 /* If it is not called from the console, it returns the environment         */
 /* variables LINES and COLUMNS (if they are defined) or a pointer to an     */
 /* empty string (if they are not defined).                                  */
 /*--------------------------------------------------------------------------*/
#ifdef DEBUG
kk = sprintf(ss,"PORTRW:  calling termdef for columns\n");
write(fe,ss,kk);
#endif DEBUG

 if(!ioctl(0,TIOCGWINSZ, &win) && (i = win.ws_col) && (j = win.ws_row)){
	Lines = j ; Cols = i ;
 }
 else{ 
 	ccp = (char *) termdef(0,'c');  /* ptr to string with column size */
 	lcp = (char *) termdef(0,'l');  /* ptr to string with line size */
 	if (strcmp(lcp,"")!=0) Lines = atoi(lcp);
	else Lines = 24;
	if (strcmp(ccp,"")!=0) Cols  = atoi(ccp);
	else Cols = 80;
 }
 size = (Lines * Cols * 2);
#ifdef DEBUG
kk = sprintf(ss,"PORTRW: call setupterm lines= %d and cols=%d\n",Lines,Cols);
write(fe,ss,kk);
#endif DEBUG
/* setupterm (getenv("TERM"),1,&termrc); */
 setupterm (0,1,&termrc);
 reset_shell_mode();

#ifdef DEBUG
kk = sprintf(ss,"PORTRW:  Lines=%d, Cols=%d, termrc=%d\n",Lines,Cols,termrc);
write(fe,ss,kk);
#endif DEBUG

 if ((sbuf = (char *)malloc(size))==NULL)           /* get memory for screen buffer */
   {
   printf(MSGSTR(NOMEM, "No memory available for screen buffer\n")); /*MSG*/
   message(40);
   exit(-1);
   }

 scur=sbuf;                                 /* set cursor ptr screen top */
 swrp=OFF;                                  /* screen wrap is OFF */
 sbufe=sbuf+size-1;                         /* set ptr to screen buffer end */

 txti=0;                                    /* # chars in capture buffer */
 
}


/* -------------------------------------------------------------------------
   port read/write routine.  This routine receives a pointer to a string
   variable which contains the file descriptor number of the temporary file
   to be used to pass flag values between the parent routine and this child
   routine.  This routine is started by the parent (fork'd and exec'd) when
   a connection is established.  It returns nothing.  On a normal termination,
   the parent will be waiting in hangup.c for its death.
   
   It uses the lndata array to store characters read in from the port.  When
   vt100 is ON, new data may be read into the lndata array if the vt100 
   routine exhausts the current lndata contents while it is processing an
   escape sequence.  The hold array is used to store filtered characters before
   they are written to stdout.  All characters accumulated prior to a vt100
   escape sequence being processed are written out and characters start
   accumulating in the hold buffer again when the program returns from the
   vt100 function.
   ------------------------------------------------------------------------- */
portread()
{
register char *q;			/* ptr to output data */
static char hold[2*LNSZ];               /* output several chars at once */
					
#ifdef DEBUG
kk = sprintf(ss,"entering portread\n");
write(fe,ss,kk);
#endif DEBUG

 /*-------------------------------------------------------------------------*/
 /* Process characters read.						    */
 /*-------------------------------------------------------------------------*/
 q = hold; 				/* ptr to 1st char in hold array */

 /*  -----------------------------------------------------------------------*/
 /* Process the characters read in, one at a time.			    */
 /*-------------------------------------------------------------------------*/
 while (ptr < lndata+lnstat)
   {
   /*-----------------------------------------------------------------------*/
   /* Add a linefeed to carriage return if requested by user.               */
   /*-----------------------------------------------------------------------*/
   if (xlatcr && *ptr==0x0D)            /* add a LF when CR is found */
     {
     *ptr = '\n';                       /* change CR to LF */
     *q = '\r';                         /* store CR in hold buffer */
     *scur='\r'; scur++;
     if(scur>sbufe) { scur=sbuf; swrp=ON; }
     q++;                               /* increment ptr in hold array */
     }

   /*-----------------------------------------------------------------------*/
   /* Save in refresh buffer, capture file and hold buffer 		    */
   /*-----------------------------------------------------------------------*/
   *scur = *ptr; scur++;
   if(scur>sbufe) { scur=sbuf; swrp=ON; }

   if (capture)                         /* user capture flag is ON */
     {
     if (txti < BUFSZ)                  /* capture buffer isn't full */
       {PC}                             /* filter & store data */
     else                               /* buffer is full; flush first */
       {
       if (write(fk,bufr,txti) == ERROR)  /* can't write to file */
         message(26);
       txti = 0;                        /* capture buf array index = 0 */
       PC                               /* filter & store data */
       }
     }                                  /* end of capture processing */

   *q = *ptr;                           /* store current char in hold buf */
   q++;                                 /* incr pointer in hold buffer */
   ptr++;                               /* increment pointer in lndata */

   } /* end of while lndata buffer processing */

   /*-------------------------------------------------------------------------*/
   /* write the processed data to screen                                      */
   /*-------------------------------------------------------------------------*/
   write(1,hold,q-hold);	        /* flush hold buf if no menu is */
#ifdef DEBUG
kk = sprintf(ss,"flush hold buffer\n");
write(fe,ss,kk);
#endif DEBUG

}
                                                                  

/* -------------------------------------------------------------------------
   Screen redraw routine.  This routine repaints the connection screen after
   the user returns from using the menus.

   Ined doesn't use NL's--it uses escape sequences.  Therefore, the screen
   redraw routine will never count any NL's before it reaches the start ptr.
   The entire buffer will be printed.
   ------------------------------------------------------------------------- */
redraw()
{
 int i;
 char *start,*end;                          /* pointers to start,end of last Lines */

#ifdef DEBUG
/* kk = sprintf(ss,"PORTRW:  \nREFRESH BUFFER = \n");
write(fe,ss,kk);
write(fe,sbuf,size);
write(fe,"\n",1); */
#endif DEBUG
 
 
 cls();                                     /* clear screen */
#ifdef DEBUG
/* write(fe,start,scur-start); */
#endif DEBUG
   write(1,sbuf,scur-sbuf);               /* write from ptr to cursor */
}
