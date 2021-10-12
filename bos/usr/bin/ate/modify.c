static char sccsid[] = "@(#)19	1.7  src/bos/usr/bin/ate/modify.c, cmdate, bos411, 9437A411a 9/12/94 15:29:06";
/* 
 * COMPONENT_NAME: BOS modify.c
 * 
 * FUNCTIONS: modify, needprompt 
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
/* Module name:      modify.c                                               */
/* Description:      modify capture file name and toggles for echo, write,  */
/*                     linefeeds, vt100 and xon/xoff control                */
/* Functions:        modify - display modify menu, get & check user input,  */
/*                     reset port if new xon/xoff value                     */
/*                   needprompt - see if prompt required when name command  */
/*                     is issued                                            */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c                                       */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: no commands provided by user                           */
/*                   user interrupt                                         */
/*   Calls:          modifymenu in menu.c (prints modify menu)              */
/*                   prompt in menu.c (prompt user for data)                */
/*                   initmod in setups.c (initialize modem)                 */
/*                   retopt in setups.c (restore terminal parameters to     */
/*                     original settings)                                   */
/*                   message in message.c (print user errors)               */
/*                   vt3 and vt5 in vt100.c (remap keyboard)                */
/*                   openfk in main.c (open capture file)                   */
/*                   vtclose in portvt.c (close vt100 screen)               */
/*                   vtinit in portvt.c (initialize vt100 screen image      */
/*                     variables)                                           */
/*   Modifies:       userin, argp's, kapfile, capture, xlatcr, vt100,       */
/*                     xctlEcho, retcode, state                             */
/*                                                                          */
/****************************************************************************/

#include "modem.h"
#include "menu.h"

/* --------------------------------------------------------------------------
   modify - printer name, capture file name, linefeeds, echo, printer, vt100,
   capture, xon/xoff
   -------------------------------------------------------------------------- */
modify()
{
 int counter=0,i,fast;
 char c,*p,*q;
 extern int lookup();

#ifdef DEBUG
kk = sprintf(ss,"Entering modify\n");
write(fe,ss,kk);
#endif DEBUG

 retcode = NULL;
 state = 5;

 if (strcmp(argp[1],EMPTY)==0) 
   fast = 0;                                /* not fast path */
 else 
   {
   fast = 1;                                /* fast path */
   i = 1;                                   /* first argument if user takes fast path */
   }

 do
 {

 if (!fast)                                 /* prompt for inputs */
   {
   for (i=0; i<26; i++)                     /* clear argp's */
     strcpy(argp[i],EMPTY);
   strcpy(userin,EMPTY);                     /* clear input */
   strcat(userin,EMPTY);
   modifymenu();                            /* print modify menu */
   c = getc(stdin);
   CKRET
   while (c != '\n' && counter <= 80) {
	userin[counter++] = c;
	c = getc(stdin);
	}
   tcflush(0,TCIFLUSH);                     /* throw out any chars over 80 */
   CKRET                                    /* check for user interrupt */
   argcnt = sscanf(userin,ARGS);
   if (argcnt < 0)  return;                 /* user pressed Enter */
   if (argcnt > 25) argcnt = 25;            /* only 25 arguments allowed */
   i=0;                                     /* first argument if user was prompted */
   }
   
 while (i < argcnt && retcode == NULL)
   {
   p=argp[i];
   q=argp[(i+1)];
   
   switch(lookup (modmenu, p))
     {
     case 0 /* 'N' */: if (needprompt(q) == 1)     /* no value follows */
                  {
                  prompt(7);                /* get capture file name */
                  if (retcode != NULL) 
                    return;                 /* check for user interrupt */
                  }
                else
                  {
                  strcpy(kapfile,q);        /* store name of capture file */
                  i++;                      /* increment over file name */
                  }
		if (capture) openfk();      /* open new capture file */
                break;

     case 4 /* 'W' */: if (capture) 		    /* toggle capture flag */
		  {
		  capture=OFF;   	    /* file closed in hangup or when */
		  }			    /* name changes (see openfk) */
                else 
		  {
		  capture=ON;
		  openfk();
		  }
                break;

     case 2 /* 'E' */: if (Echo) Echo=OFF;         /* toggle echo flag */
                else Echo=ON;
                break;

     case 1 /* 'L' */: if (xlatcr) xlatcr=OFF; /* toggle lf (CR-->CR-LF) */
                else xlatcr=ON;
                break;
                 
     case 3 /* 'V' */: if (vt100)                  /* toggle vt100 flag */
                  {
                  vt100=OFF;
#ifdef HFT
                  vt5();                    /* map keypad to base state */
#endif /* HFT */
		  vtclose();		    /* close vt100 screen */
                  }
                else
                  {
                  vt100=ON;
		  vtinit();		    /* initialize vt100 screen */
#ifdef HFT
                  vt3();                    /* map keypad to shifted state */
#endif /* HFT */
                  }
		retopt();		    /* vtclose & vtinit reset term */
					    /* state.  Put back to original */
                break;
		 
     case 6 /* 'A' */:                      /* vt100 autowrap flag */
        if (awrap) {
            awrap = OFF; /* vtwrap(); */
        } else {
            awrap = ON;
        }
        break;

     case 5 /* 'X' */: if (xctl) xctl=OFF;         /* toggle xctl (xon/xoff control) */
                else xctl=ON;
                if (lpath != ERROR)           /* reset port with new value */
                  initmod(lpath,OFF,OFF,0,0,0,0);
                break;
		 
     default  : strcpy(argp[25],p);         /* store user input for msg print */
                message(3);                 /* invalid command */
                break;
     } /* end switch */
   i++;                                     /* increment to next argument */
   } /* end while */
   
 }
 while (retcode == NULL && !fast);          /* reprint menu if not fast path */

 retcode = NULL;

#ifdef DEBUG
kk = sprintf(ss,"leaving modify\n");
write(fe,ss,kk);
#endif DEBUG

}

                  
/* --------------------------------------------------------------------------
   Need prompt routine.  This routine receives the next argument in the users
   list and checks to see if it is a valid toggle argument.  If it is, or it
   is empty, the function returns a 1 and the user is prompted for a file name.
   Otherwise it returns a 0 and it is assumed that the next argument provided
   by the user is the capture filename.
   -------------------------------------------------------------------------- */
needprompt(q)
char *q;
{
 char k[41];

#ifdef DEBUG
kk = sprintf(ss,"entering needprompt\n");
write(fe,ss,kk);
#endif DEBUG

 strcpy(k, q);
 if (strcmp(k,EMPTY) == 0) return(1);       /* this is empty */

#ifdef DEBUG
kk = sprintf(ss,"leaving needprompt\n");
write(fe,ss,kk);
#endif DEBUG
 return(0);

/* return (lookup (modmenu, k) != ERROR);*/
}
