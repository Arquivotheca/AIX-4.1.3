static char sccsid[] = "@(#)03 1.13 src/bos/usr/bin/ate/command.c, cmdate, bos411, 9428A410j 3/11/94 16:15:33";
/* 
 * COMPONENT_NAME: BOS command.c
 * 
 * FUNCTIONS: cmd 
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
/* Module name:      command.c                                              */
/* Description:      print connected and unconnected main menu, get user    */
/*                     input, execute command.                              */
/* Functions:        cmd - display menu, get user input, execute command.   */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      conn in connect.c, main in main.c                      */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*                                                                          */
/*   Abnormal rets:  no commands supplied by user                           */
/*                   cmdkey or capkey entered while in connected main menu  */
/*                   user interrupt at various points                       */
/*                   no send/receive file name provided by user             */
/*                                                                          */
/*   Calls:          retopt in setups.c (restore terminal settings)         */
/*                   mainmenu, prompt in menu.c (print main menu)           */
/*                   conn in connect.c (make & maintain a connection)       */
/*                   direc in directory.c (dialing directory)               */
/*                   message in message.c (print user messages)             */
/*                   help in help.c (display help panels)                   */
/*                   hangup in hangup.c (terminate connection)              */
/*                   modify in modify.c (modify menu)                       */
/*                   alter in alter.c (alter menu)                          */
/*                   xrcv, xsend in xmodem.c (file transfer)                */
/*                   prcv, psend in pacing.c (file transfer)                */
/*                   cls in cls.c (clear scren)                             */  
/*                                                                          */
/*   Modifies:       userin, argp's, argcnt, state, retcode, menu, number,  */
/*                     port, dirfile, rcvfile, sndfile, xmit                */
/*                                                                          */
/****************************************************************************/

#include "modem.h"
 
extern struct list *mmenu;
extern int lookup();

/* ----------------------------------------------------------
   cmd -  Command routine.  Prints the connected and unconnected
   main menus, gets and processes user's command.
   ---------------------------------------------------------- */
cmd()
{
 int i,count,counter=0,rc;
 char c;
 static char shellcmd[sizeof(userin)+sizeof(port)+13];

#ifdef DEBUG
 kk = sprintf(ss,"Entering command routine.\n");
 write(fe,ss,kk);
#endif DEBUG

 for (i=0; i<26; i++)            /* clear argument pointers */
   strcpy(argp[i],EMPTY);
 strcpy(userin,EMPTY);            /* clear input string */
 strcat(userin,EMPTY);
 
 retopt();                       /* restore terminal settings */
 state=1;
 retcode=(int)NULL;

 mainmenu();                      /* write main menu */

 if ((c = getc(stdin)) == EOF) {
   retcode = QUIT;
   return;
 }

 while (c != '\n' && counter <= 80) {
   userin[counter++] = c;
   if ((c = getc(stdin)) == EOF) {
     retcode = QUIT;
     return;
   }
 }
 tcflush(0, TCIFLUSH);            /* discard anything past 80 chars */

 CKRET                            /* check for user interrupt */
 
 argcnt=sscanf(userin,ARGS);       /* convert input string to arguments */

#ifdef DEBUG
 kk = sprintf(ss,"argcnt=%d\n",argcnt);
 write(fe,ss,kk);
 for (i=0; i<argcnt; i++) {
   kk = sprintf(ss,"argp[%d] = %s\n",i,argp[i]);
   write(fe,ss,kk);
 }
#endif DEBUG

 if (argcnt < 0)  return(0);         /* if input is Enter, return  to conn() or main() */
 if (argcnt > 25) argcnt = 25;    /* only accept 25 arguments */
 
 if (connect && (*argp[0]==cmdkey || *argp[0]==capkey))
   {
   c = *argp[0];                  /* send a capture or command key */
   write(lpath,&c,1);             /* over the connection */
   if (*argp[0]==cmdkey)          /* user assumed to be in a once-away conn */
     menu=(-1);                   /* & calling intermediate menu.  The end */
                                  /* machine will not refresh with menu=-1. */
   return;                        /* return to conn() */
   }

 else
 {
 strcpy(argp[25],argp[0]);        /* store user input for msg print */

#ifdef DEBUG
kk = sprintf(ss,"processing command %s\n",argp[0]);
write(fe,ss,kk);
#endif DEBUG

 switch(lookup(mmenu, argp[0]))
   {
   case 0 : if(connect==OFF)			  /* Connect */
                {
                if (strcmp(argp[1],EMPTY)==0)
                  prompt(1);              /* get number, port */
                if (retcode != (int)NULL)      /* user interrupt */
                  return;                 /* return to main() */
                if (isdigit(*argp[1]))    /* have a number to dial */
                  {
                  strcpy(number,argp[1]); /* store number */
                  if (strcmp(argp[2],EMPTY) != 0)
                    strcpy(port,argp[2]); /* store port name */
                  }
                else if (strcmp(argp[1],EMPTY) != 0)
                  strcpy(port,argp[1]);   /* store port name */
                if (retcode != (int)NULL) return;
                conn();                   /* establish connection */
                if (attempts && isdigit(*argp[1])) /* redial */
                  {
                  count = 0;              /* number of attempted redials */
                  while (retcode==(int)NULL && count<attempts)
                    {
		    sleep((unsigned int)redial);
                    count++;
                    if (retcode==(int)NULL) conn();
                    }
		  message(9);             /* can't connect message */
                  }
                }
              else message(3);            /* bad command when connected */
              break;

   case 1 : if(connect==OFF)			   /* Directory */
                {
                if (strcmp(argp[1],EMPTY)!=0)
                  strcpy(dirfile,argp[1]); /* store dialing directory name */
                else prompt(2);
                if (retcode != (int)NULL) return;
                direc();                  /* print directory, get entry */
                }
              else message(3);            /* bad command when connected */
              break;

   case 6 : help();                       /* print help panels */
              break;

   case 5 : if (connect)				  /* Terminate */
                {
                hangup();                 /* terminate connection */
                retcode=(int)EXIT;
                }
              else message(3);            /* bad command if not connected */
              break;

   case 7 : modify();    /* name capture file; set LF,echo,printer,vt100,
                              capture, xon flags */
              break;

   case 8 : 
              alter();     /* set bits,stop,parity,speed,port,dialp,dials,
                              redial,attempts,transfer,pacing */
              break;

   case 9 : if (strcmp(argp[1],EMPTY)==0) prompt(3);		/* Perform */
              else			/* copy blanks over argp[0] */
                {
                for (i = 0; i<sizeof(userin) && userin[i] != ' '; i++) 
		   userin[i] = ' ';
                }
              strcpy (shellcmd, "ateport=/dev/");
              strcat (shellcmd, port);    /* port is prepended to shell cmd */
              strcat (shellcmd, " ");     /* for xmodem pass-through use */
              strcat (shellcmd, userin);
              CKRET

              {
	      /*-------------------------------------------------------------*/
	      /* This code duplicates the C system() call.  Setting gid and  */
	      /* uid were added to prevent a process from doing more than    */
	      /* they are otherwise allowed, since ATE is running as root.   */
	      /*-------------------------------------------------------------*/
	      gid_t gid ;
	      uid_t uid;
	      int status, pid, w;
	      register void (*istat)(int), (*qstat)(int);

	      gid = getgid();		/* get group id */
	      uid = getuid();		/* get user id */
	      cls();
	      if((pid = fork()) == 0) 
                {
		(void) setgid(gid);
	        (void) setuid(uid);
#ifdef DEBUG
kk = sprintf(ss,"Child processing command %s\n",shellcmd);
write(fe,ss,kk);
#endif DEBUG
		(void) execl("/usr/bin/ksh", "sh", "-c", shellcmd, 0);
		_exit(127);
	        }
	      istat = signal(SIGINT, SIG_IGN);
	      qstat = signal(SIGQUIT, SIG_IGN);
	      while((w = wait(&status)) != pid && w != -1)
		;
	      (void) signal(SIGINT, (void(*)(int)) istat);
	      (void) signal(SIGQUIT, (void(*)(int)) qstat);
	      if (w == -1) message(62);
              }
              message(40);                /* Press Enter msg */
              break;

   case 10 : hangup();
              retcode=QUIT;               /* quit */
              break;

   case 3 : if(connect)					  /* Receive */
                {
                if (strcmp(argp[1],EMPTY)!=0)
                  strcpy(rcvfile,argp[1]); /* store name of file to receive */
                else prompt(5);
                CKRET                     /* check for user interrupt */
                if (strcmp(rcvfile,"")==0)
                  {
                  message(18);            /* need a file name */
                  return;                 /* return to conn() */
                  }
                xmit=ON;                  /* file transmission ON */
		if (transfer == 'x')
                  xrcv();                 /* xmodem receive a file */
		if (transfer == 'p')
                  prcv();                 /* pacing receive a file */
                xmit=OFF;                 /* file transmission OFF */
		}
              else message(3);            /* bad command if not connected */
              break;

   case 2 : if(connect)					  /* Send */
		{
                if (strcmp(argp[1],EMPTY)!=0)
                  strcpy(sndfile,argp[1]); /* store name of file to send */
                else prompt(4);
                CKRET                      /* check for user interrupt */
                if (strcmp(sndfile,"")==0)
                  {
                  message(23);             /* need a file name */
                  return;                  /* return to conn() */
                  }
		if (transfer == 'x')
                  xsend();                 /* xmodem send a file */
		if (transfer == 'p')
                  psend();                 /* pacing send a file */
                xmit=OFF;                  /* file transmission OFF */
		}
              else message(3);             /* bad command if not connected */
              break;

   case 4 : if (connect)				  /* Break */
                {
                tcsendbreak(lpath, 0);    /* send zero bits for .25 seconds */
                sleep(1);                 /* timing wait for intr handling */
                }                         /*   in child process */
              else message(3);            /* bad command if not connected */
              break;

   default  : message(3);                 /* invalid command */
              break;
   } /* end of switch */
 } /* end of else */
   
tcflush(0,TCIFLUSH);                   /* flush the input que of extra chars */

#ifdef DEBUG
kk = sprintf(ss,"leaving command\n");
write(fe,ss,kk);
#endif DEBUG

}
