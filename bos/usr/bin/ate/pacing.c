static char sccsid[] = "@(#)21	1.4  src/bos/usr/bin/ate/pacing.c, cmdate, bos411, 9428A410j 4/18/91 10:57:57";
/* 
 * COMPONENT_NAME: BOS pacing.c
 * 
 * FUNCTIONS: prcv, psend 
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



/****************************************************************************/
/*                                                                          */
/* Module name:      pacing.c                                               */
/* Description:      send or receive a file from a connection using either  */
/*                     character pacing or interval pacing                  */
/* Functions:        prcv  - receive a file                                 */
/*                   psend - send a file                                    */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c                                       */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: can't open send or receive file                        */
/*                   can't write to receive file                            */
/*                   no pacing character is received in time interval       */
/*   Calls:          message in message.c (print user errors)               */
/*   Modifies:       nothing                                                */
/*                                                                          */
/****************************************************************************/

#include "modem.h"

#define TIMEOUT     -1 		/* timeout character */
#define EOT       0x04          /* end of tranmission */

char outdat[LNSZ+1];		/* output data */
char *p, *q;	                /* array pointers */
int c;				/* current character */
char ch;			/* character form of c */

/*----------------------------------------------------------------------*/
/* pacing receive routine						*/
/*----------------------------------------------------------------------*/
prcv()
{
 int promptc = 0;			/* prompt character or interval */
 int old_c = 0;                        /* previous character received */

#ifdef DEBUG
kk = sprintf(ss,"entering prcv\r\n");
write(fe,ss,kk);
#endif DEBUG

 state=7;
 lnstat=0;				/* no chars read yet */
 p=lndata;				/* set ptr to start of buffer */
 q=outdat;

 if((fr=open (rcvfile,O_WRONLY|O_CREAT|O_APPEND,0766))==ERROR)
     {
     message(17);			/* can't open/create receive file */
     return;
     }
     
 message(20);                           /* ready to receive message */

#ifdef DEBUG
kk = sprintf(ss,"writing prompt character to port\n");
write(fe,ss,kk);
#endif DEBUG

 if (isdigit(pacing))                   /* pacing character is a digit */
   /* do nothing */ ;
 else
   {
   promptc = pacing;                    /* prompt char for controlling writes */
   write(lpath,&pacing,1);              /* send initial prompt character */
   }

#ifdef DEBUG
kk = sprintf(ss,"entering while loop\n");
write(fe,ss,kk);
#endif DEBUG

 /*---------------------------------------------------------------------*/
 /* while there are still characters to read and no end of transmission */
 /*---------------------------------------------------------------------*/
 while (lnstat != ERROR && 		/* read didn't fail */
	*p != EOT &&			/* haven't received an EOT */
        retcode == (int)NULL)   		/* user hasn't interrupted */
   {
   /*-------------------------------------------------------------------*/
   /* read characters into lndata buffer				*/
   /*-------------------------------------------------------------------*/
   if (p == lndata+lnstat)		/* end of buffer */ 
     {
     alarm(30);				/* set timer */
     lnstat = read(lpath,lndata,LNSZ);	/* read in anything on port */
     alarm(0);				/* turn off timer */
     p=lndata;				/* set ptr to start of buffer */
     q=outdat;
#ifdef DEBUG
kk = sprintf(ss,"\nREADING:  n=%d, data=",lnstat);
write(fe,ss,kk);
if (lnstat>0) write(fe,lndata,lnstat);
#endif DEBUG
     }
   /*-------------------------------------------------------------------*/
   /* process characters already in lndata buffer			*/
   /* q moves through outdat, while p moves through lndata.		*/
   /*-------------------------------------------------------------------*/
   else					/* not end of buffer yet */
     {
     /*-----------------------------------------------------------------*/
     /* process each character one at a time			   	*/
     /*-----------------------------------------------------------------*/
     for (; p<lndata+lnstat; p++,q++)   /* process everything read */
       {
       /*---------------------------------------------------------------*/
       /* End of transmission or null character?			*/
       /*---------------------------------------------------------------*/
       if (*p == (int)NULL || *p == EOT)
         {
#ifdef DEBUG
kk = sprintf(ss,"\nEOT/NULL(%d) at p=%d, lndata=%d\n",*p,p,lndata);
write(fe,ss,kk);
#endif DEBUG
         }
       *p = *p & 0x00FF;		/* mask char to 8 bits */
       /*---------------------------------------------------------------*/
       /* If this is a CR-LF combination, throw out LF.  CR will be     */
       /* converted to NL below.					*/
       /*---------------------------------------------------------------*/
       if (old_c=='\r' && *p=='\n')    	/* got a CR-LF combination */
         {
	 p++;				/* throw out LF & get next char */
	 if (p<lndata+lnstat)		/* still in buffer */
	   old_c = *p;			/* save new character */
	 else break;			/* at end of lndata buffer */
         }
       else old_c = *p;
       /*---------------------------------------------------------------*/
       /* convert CR to NL.  Then send prompt character if necessary.   */
       /*---------------------------------------------------------------*/
       if (*p=='\r') *p = '\n';		/* convert CR to LF */
       if (*p == '\n' && promptc)       /* EOL and character pacing */
         {
         write(lpath,&pacing,1);	/* send prompt character */
         }

       (*q)=(*p);
       }				/* end of for */

     /*-----------------------------------------------------------------*/
     /* write the characters out to the file & stdout.			*/
     /*-----------------------------------------------------------------*/
     if (write (fr,outdat,q-outdat) != q-outdat) 
       {
       message(27);                     /* can't write to receive file */
       return;
       }
     write(1,outdat,q-outdat);		/* echo to screen */
#ifdef DEBUG
kk = sprintf(ss,"\nWRITING:  n=%d, data=",q-outdat);
write(fe,ss,kk);
write(fe,outdat,q-outdat);
#endif DEBUG
     } /* end of else */
   } /* end of while */
   
 CKRET                                  /* check for user interrupt */

#ifdef DEBUG
kk = sprintf(ss,"\nend of while loop: lnstat=%d, *p=%d, retcode=%d\n",
   lnstat,*p,retcode);
write(fe,ss,kk);
#endif DEBUG

 if (lnstat == ERROR)                     /* didn't receive a character */
   {
   sectnum = 30;                        /* global variable used for msg print */
   message(2);                          /* timed out message */
   }
 else message(15);                      /* transfer complete message */
 close(fr);                             /* close receive file */
 
#ifdef DEBUG
kk = sprintf(ss,"leaving prcv\n");
write(fe,ss,kk);
#endif DEBUG

 retcode = (int)NULL;
 
}  /* end of prcv */


/*----------------------------------------------------------------------*/
/* pacing send routine 							*/
/*									*/   
/*----------------------------------------------------------------------*/
psend()
{
 int delay = 0;                             /* seconds to delay between lines */
 int tries;
 int rc,promptc = 0;                        /* read chars, prompt character */
 char array[512];                           /* used to read up echoed chars */
 
#ifdef DEBUG
kk = sprintf(ss,"entering psend\n");
write(fe,ss,kk);
#endif DEBUG

 state=6;
 lnstat = 0;				/* no chars read yet */
 p = lndata;				/* set ptr to start of buffer */

 if ((fs=open(sndfile,O_RDONLY))==ERROR)
   {
   message(22);				/* can't open send file */
   return;
   }

 if(isdigit(pacing))                    /* pacing char is an interval */
   delay = pacing - '0';                /* set delay between lines */
 else promptc = pacing;                 /* save prompt character */

 message(24);                           /* ready to send message */

#ifdef DEBUG
kk = sprintf(ss,"waiting 20 seconds for pacing character\n");
write(fe,ss,kk);
#endif DEBUG

 /*---------------------------------------------------------------------*/
 /* Use any character received in the next 10 seconds to get started.   */
 /* If nothing arrives, start sending anyway.				*/
 /*---------------------------------------------------------------------*/
 if (promptc)                           /* this is character pacing */
   {
   alarm(10);				/* set timer */
   rc=read(lpath,&c,1);			/* read initial pacing char */
   alarm(0);				/* turn off timer */
   }
 
#ifdef DEBUG
kk = sprintf(ss,"rc=%d, pacing character is %c\n",rc,c);
write(fe,ss,kk);
#endif DEBUG

 /*-------------------------------------------------------------------------*/
 /* while there are still characters in the file...			    */
 /*-------------------------------------------------------------------------*/
 while ((lnstat = read(fs,lndata,LNSZ))	    /* read in a block from file */
        && retcode == (int)NULL)                 /* no user interrupt */
   {
   p=q=lndata;				    /* set ptrs to start of array */

   while (p < lndata+lnstat)		    /* not at end of array */
     {
     /*---------------------------------------------------------------------*/
     /* End of line character.  Write line to port & do pacing stuff.       */
     /*---------------------------------------------------------------------*/
     if (*p == '\n')
       {
       write(1,q,p-q+1);		    /* write to screen */
       *p = '\r';			    /* convert LF to CR */
       write(lpath,q,p-q+1);		    /* write this line to port */
       q = (++p);			    /* reset q, incr p */

       /*-------------------------------------------------------------------*/
       /* Need to wait for a pacing character.				    */
       /*-------------------------------------------------------------------*/
       if (promptc)			    /* get pacing character */
         {
         ch = '0';		 	    /* prompt character read */
         tries = 0;			    /* initialize # of chars read */
         while (ch != pacing && tries<7)    /* haven't received prompt char */
           {				    /*   & chars were read from port */
           alarm(5);		   	    /* don't wait on read >5 seconds */
           rc = read(lpath,array,512);	    /* read anything on port--some */
           alarm(0);			    /*   routines echo back char sent */

           if (rc>0) ch=array[rc-1];	    /* prompt char is last one read */
           else ch='0';			    /*   or we didn't receive one */
           
           CKRET			    /* check for user interrupt */
           
           if (ch==pacing) break;	    /* exit while loop */
   	   else tries++;
           }
         if (tries==7 && ch != pacing)	    /* haven't rec'd the prompt char */
           {
#ifdef DEBUG
kk = sprintf(ss,"No pacing character received.  Leaving psend\n");  
write(fe,ss,kk);
#endif DEBUG
           message(21);			    /* time out message */
           close(fs);			    /* close send file */
           return;				
           }
         }			 	    /* end of "get pacing char" */
       /*-------------------------------------------------------------------*/
       /* Interval pacing.  Wait 'delay'.				    */
       /*-------------------------------------------------------------------*/
       else if (delay > 0) sleep(delay);    /* interval pacing delay */
       }				    /* end of newline processing */
     /*---------------------------------------------------------------------*/
     /* Normal character.                                                   */ 
     /*---------------------------------------------------------------------*/
     else p++;      
     }                                      /* end of buffer */

   if (q<p) 
     {
     write(1,q,p-q);		    	    /* write to screen */
     write(lpath,q,p-q);           	    /* write partial line to port */
     }
   CKRET                                    /* check for user interrupt */
   }

     
 /*---------------------------------------------------------------------*/
 /* This routine used to send an EOT when it encountered the end of the */
 /* send file.  However, we discovered other routines didn't expect it  */
 /* and stored it in the received fiel.  On another Unix system, it     */
 /* caused a logout.							*/
 /*---------------------------------------------------------------------*/
 /* sendchar (EOT);                             REMOVED */
 message(15);                               /* file transfer complete */
 close(fs);                                 /* close send file */
 
#ifdef DEBUG
kk = sprintf(ss,"leaving psend\n");
write(fe,ss,kk);
#endif DEBUG

retcode = (int)NULL;

}
