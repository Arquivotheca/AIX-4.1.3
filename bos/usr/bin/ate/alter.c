static char sccsid[] = "@(#)99	1.5  src/bos/usr/bin/ate/alter.c, cmdate, bos411, 9428A410j 4/18/91 10:55:25";
/* 
 * COMPONENT_NAME: BOS alter.c
 * 
 * FUNCTIONS: alter, if, while 
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
/* Module name:      alter.c                                                */
/* Description:      modify word length, stop bits, parity, speed, port,    */
/*                     dialing prefix and suffix, redial pause, attempts    */
/*                     for redialing, file transfer method and pacing char. */
/*                     as input by user from keyboard.                      */
/* Functions:        alter - display alter menu, get & check user input,    */
/*                     reset port with new values                           */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c                                       */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: no commands supplied by user                           */
/*                   user interrupt                                         */
/*   Calls:          altermenu in menu.c (prints alter menu)                */
/*                   ckrate, cklength, ckstop, ckparity, ckredial,          */
/*                     cktransfer, ckpacing in check.c (checks inputs)      */
/*                   initmod in setups.c (initialize modem)                 */
/*                   message in message.c (print user errors)               */
/*   Modifies:       userin, argp's, bits, stop, parity, speed, port,       */
/*                     dialp, dials, redial, attempts, transfer, pacing     */
/*                                                                          */
/****************************************************************************/

#include "modem.h"                          /* global variable definitions */
#include "menu.h"

/* --------------------------------------------------------
   alter - length,stop,parity,speed,port,prefix,suffix,
           redial,file transfer method,pacing character
   -------------------------------------------------------- */
alter()
{
 int i,counter=0,tmpr,fast=1;
 char c,*p,*q;
 extern int lookup();

#ifdef DEBUG
kk = sprintf(ss,"Entering alter\n");
write(fe,ss,kk);
#endif DEBUG

 state = 5;                                 /* state for user interrupts */
 retcode = (int)NULL;                       /* return code */
 
 if (strcmp(argp[1],EMPTY)==0)              /* not fast path */
   fast = 0;
 else                                       /* is fast path */
   {
   fast = 1;
   i = 1;                                   /* first argument if user takes fast path */
   }

 do
 {
 
 if (!fast)                                 /* must prompt for inputs */
   {
   for (i=0; i<26; i++)                     /* clear input variables */
     strcpy(argp[i],EMPTY);
   strcpy(userin,EMPTY);
   strcat(userin,EMPTY);
   altermenu();                             /* print alter menu */
   c = getc(stdin);
   CKRET
   while (c != '\n' && counter <= 80) {
	userin[counter++] = c;
	c = getc(stdin);
	}
   tcflush(0, TCIFLUSH);                   /* discard anything past 80 chars */
   CKRET                                    /* check for user interrupt */
   argcnt = sscanf(userin,ARGS);
   if (argcnt < 0)  break;                  /* no more input */
   if (argcnt > 25) argcnt = 25;
   i=0;                                     /* first argument if user was prompted */
   }
   
#ifdef DEBUG
{
int k;
kk = sprintf(ss,"argcnt=%d\n",argcnt);
write(fe,ss,kk);
for (k=0; k<argcnt; k++)
  {
  kk = sprintf(ss,"argp[%d] = %s\n",k,argp[k]);
  write(fe,ss,kk);
  }
}
#endif DEBUG

 while (i < argcnt && retcode == (int)NULL)
   {
   p=argp[i];                               /* command */
   q=argp[(i+1)];                           /* argument */
   if (strcmp(q,EMPTY)==0)
     strcpy(argp[(i+1)],"");                /* null string for prints */
   tmpr = atoi(q);                          /* convert to a number */

   switch(lookup(altrmenu, p))
     {
     case 3 /* 'R' */: if (ckrate(q) != ERROR) speed=tmpr;
                break;

     case 0 /* 'L' */: if (cklength(q) != ERROR) bits=tmpr;
                break;

     case 1 /* 'S' */: if (ckstop(q) != ERROR) stop=tmpr;
                break;

     case 2 /* 'P' */: if (ckparity(q) != ERROR) parity=tmpr;
                break;

     case 4 /* 'D' */: strcpy(port,q);             /* port name */
                break;

     case 5 /* 'I' */: strcpy(dialp,q);            /* dialing prefix */
                break;

     case 6 /* 'F' */: strcpy(dials,q);            /* dialing suffix */
                if (strcmp(dials,"0")==0)
                  strcpy(dials,"");         /* set off */
                break;

     case 7 /* 'W' */: if (ckredial(q,'W') != ERROR) redial = tmpr;
                break;
                  
     case 8 /* 'A' */: if (ckredial(q,'A') != ERROR) attempts = tmpr;
                break;

     case 9 /* 'T' */: if (cktransfer(q) != ERROR) transfer = *q;
                break;

     case 10 /* 'C' */: if (ckpacing(q) != ERROR) pacing = *q;
                break;

     default  : sprintf(argp[25],"%s",p);
                message(3);
                i--;                      /* decr so process next user input */
                break;
     } /* end switch */
   i = i+2;
 } /* end while */
   
 if (bits==8 && parity != 0)
   {
   message(54);
   parity = 0;
   }

 }
 while (retcode == (int)NULL && !fast);

 if (lpath != ERROR)                          /* port is open */
   initmod(lpath,OFF,OFF,0,0,0,0);          /* initialize terminal parameters */
 
 retcode = (int)NULL;

#ifdef DEBUG
kk = sprintf(ss,"leaving alter\n");
write(fe,ss,kk);
#endif DEBUG
}

