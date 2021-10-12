static char sccsid[] = "@(#)09	1.4  src/bos/usr/bin/ate/help.c, cmdate, bos411, 9428A410j 4/18/91 10:56:36";
/* 
 * COMPONENT_NAME: BOS help.c
 * 
 * FUNCTIONS: help 
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
/* Module name:      help.c                                                 */
/* Description:      print help panels                                      */
/* Functions:        help - display help panels                             */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c                                       */
/*   Receives:       nothing (values are passed in argp's)                  */
/*   Returns:        nothing                                                */
/*   Abnormal exits: user interrupt                                         */
/*   Calls:          msghelp 						    */ 
/*                   cls in cls.c (clear screen)                            */
/*                   message in message.c (print prompts)                   */
/*   Modifies:       nothing                                                */
/*                                                                          */
/****************************************************************************/

#include "modem.h"

#define PAUSE CKRET                            /* check for user interrupt */ \
              message(41);                     /* "More.  Press Enter." */
              
#define STOP  CKRET                            /* check for user interrupt */ \
              message(40);                     /* "Press Enter." */ \
              break;

extern struct list *mmenu;
              
/* -----------------------------------------------------------------------
   help - print ate help panels.  Some help panels
   are two screens; the rest are only one.
   ----------------------------------------------------------------------- */
help()
{
int i=1;

#ifdef DEBUG
kk = sprintf(ss,"entering help\n");
write(fe,ss,kk);
#endif DEBUG

 if (argcnt == 1)                              /* no specific panels requested */
  {
  strcpy(argp[1], argp[0]);                    /* give them the generic "Help" one */
  argcnt = 2;
  }

 while (i<argcnt && retcode==(int)NULL)
   {
   cls();                                      /* clear screen */
   switch(lookup(mmenu, argp[i]))
     {
     case 6 : msghelp(1);  /* help */
              STOP
     case 0 : msghelp(2);  /* connect */
              PAUSE
              cls();
              msghelp(3);
              STOP
     case 1 : msghelp(4);  /* directory */
              PAUSE
              cls();
              msghelp(5);
              STOP
     case 7 : msghelp(6);  /* modify */
              PAUSE
              cls();
              msghelp(7);
              STOP
     case 8 : msghelp(8);  /* alter */
              PAUSE
              cls();
              msghelp(9);
              STOP
     case 9 : msghelp(10);  /* perform */
              STOP
     case 10: msghelp(11);  /* quit */
              STOP
     case 2 : msghelp(12);  /* send */
              STOP
     case 3 : msghelp(13);  /* receive */
              STOP
     case 5 : msghelp(14);  /* terminate */
              STOP
     case 4 : msghelp(15);  /* break */
              STOP
     default: strcpy(argp[25],argp[i]);  /* store letter for msg print */
              message(14);               /* invalid help option */
              break;
         } /* end of switch */
   i++;
   } /* end while */
   
#ifdef DEBUG
kk = sprintf(ss,"leaving help\n");
write(fe,ss,kk);
#endif DEBUG

retcode = (int)NULL;

}
