static char sccsid[] = "@(#)04	1.22.1.2  src/bos/usr/bin/ate/connect.c, cmdate, bos411, 9428A410j 4/4/94 16:32:56";
/* 
 * COMPONENT_NAME: BOS connect.c
 * 
 * FUNCTIONS: MSGSTR, ckenabled, conn, digits 
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
/* Module name:      connect.c                                              */
/* Description:      write dialing digits to port, establish connection,    */
/*                     read port and call vt2/portread to write to screen,  */
/* 		       read stdin and write to port			    */
/* Functions:        conn - establish and maintain connection               */
/*                   digits - write dialing number to port                  */
/*                   ckenabled - see if port is enabled                     */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*                   <sys/select.h>                                         */  
/*   Called by:      cmd in command.c, direc in directory.c                 */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: user interrupt                                         */
/*                   can't open local dialing port                          */
/*                   port is enabled                                        */
/*                                                                          */
/*   Calls:          initmod in setups.c (initialize modem)                 */
/*                   setopt in setups.c (set terminal up for connection)    */
/*                   cmd in command.c (process a user command)              */
/*                   message in message.c (print user msgs)                 */
/*                   vt1 in vt100.c (convert sailboat key to vt100 key)     */
/*                   hangup in hangup.c (end connection)                    */ 
/*                   portinit in portrw.c (set screen size, alloc cap buf)  */
/*                   portread in portrw.c (port read/write routine)         */ 
/*                   redraw in portrw.c (repaint the connection screen      */
/*                      after a menu)                                       */ 
/*                   vtdraw in portvt.c (refresh the screen)                */ 
/*                   vt2 in portvt.c (maintain vt100 screen image)          */
/*                   openfk in main.c (open capture file)                   */ 
/*                                                                          */
/*   Modifies:       connect, portname, attempts, retcode, state            */
/*                                                                          */
/****************************************************************************/

#include "modem.h"
#include <sys/select.h>
#include <sys/time.h>
#define MAX_RETRY 5

static char cret = '\r';    	/* carriage return */
  
int rdmask,wrmask,exmask;	/* files ready to read */
struct timeval timeout;		/* select timeout period */

/* ------------------------------------------------------------------------
   dial digits:  opens local port (/dev/ltty#) with nodelay, writes dialing
   sequence and closes port.  Sets an alarm of 45 seconds for detection of
   carrier detect.
   ------------------------------------------------------------------------ */
digits()
{
 static char portname[81];                  /* port name */
 char dialstring[100], *bufp ;
 int i,j,tries = 0 ;
 strcpy(portname,"/dev/");                 /* local port is remote port name */
 strcat(portname,port);                     /* with l prepended.  Created by devices. */
 
#ifdef DEBUG
 kk = sprintf(ss,"in digits.  Port=%s, dialing pattern=%s %s %s\n",
  portname,dialp,argp[1],dials);
write(fe,ss,kk); 
#endif DEBUG

 alarm(10);                                 /* set alarm so can't hang on open */
 if((lpath=open(portname,O_RDWR|O_NDELAY))==ERROR)
   {
   alarm(0);                                /* turn off alarm */
   strcpy(argp[25],portname);               /* store port name for msg print */
   message(8);                              /* can't open port */
   if(locfile) {
      ttyunlock(port);			    /* get rid of lock file */
      locfile=OFF;                          /* reset lockfile indicator */
     }
   retcode=EXIT;                            /* set retcode to leave conn() */
   return;                                  /* return to conn() */
   }
 else alarm(0);                             /* turn off alarm */
 lpath2 = dup(lpath);
   
 initmod(lpath,ON,OFF,0,0,0,0);             /* initialize port parameters */
 
 if (retcode != 0)                       /* check for user interrupt */
   {
   close(lpath);                            /* close port */
   return;                                  /* return to conn() */
   }
   sprintf(dialstring,"%s%s%s%c",dialp,argp[1],dials,cret);
#ifdef DEBUG
  kk=sprintf(ss,"dialpattern = %s\n",dialstring);
  write(fe,ss,kk);
#endif DEBUG
   sleep(1);				/* Wait to send dialstring to */
					/* newly opened modem port    */
   i = strlen(dialstring); j = 0 ;
   bufp = dialstring ;

 retry :

   for(; i>0; i -=j, bufp +=j) {
    if(0 >= (j = write(lpath,bufp,i))) {
	break ;
    }
   }
    if(j <= 0) {
	tries++;
	j = 0;
	if(tries > MAX_RETRY){
		message(9);
		retcode = EXIT;
	}
	else {
		sleep(2);
		goto retry;
	}
#ifdef DEBUG
  kk = sprintf(ss,"error in writing port=%d\n",errno);
  write(fe,ss,kk);
#endif DEBUG
     }
 sleep(5);
 initmod(lpath,OFF,OFF,0,0,0,0);
 close(lpath);                              /* close port */
     
 alarm((unsigned int)45);                   /* time out if no carrier detect */
 
#ifdef DEBUG
 kk = sprintf(ss,"leaving digits\n");
write(fe,ss,kk); 
#endif DEBUG
}


/* ------------------------------------------------------------------------
   conn - Connect routine.  Establish and maintain connection.  If the user
   has provided a number, they have 90 seconds for carrier detect to be
   established.  If no number is provided, a direct connect is assumed and
   will time out in 10 seconds if there is no carrier detect.
   ------------------------------------------------------------------------ */
conn()
{
static char portname[81];               /* port name */
int rc;					/* return code */

int ESCflag = 0;               /* 1 = esc received; 0 = no escape received */  

#ifdef DEBUG
kk = sprintf(ss,"Entering connect\n");
write(fe,ss,kk);
#endif DEBUG

 state=3;
 retcode=0;
 connect=OFF;
 lpath2 = 0;
 portinit();
 strcpy(portname,"/dev/");                  /* port name */
 strcat(portname,port);

 /* lock the port so other */
 /*   programs won't use it */
 if(rc=ttylock(port) == -1)		    /* create lockfile */
   {                                        
   strcpy(argp[25],portname);               /* port name for msg print */
   message(61);                             /* can't open port */
   return;                                  /* return to direc or cmd */
   }
 locfile=ON;                                /* this ate created lokfile */
 
 if(isdigit(*argp[1]))                      /* user provided a number */
   {
#ifdef DEBUG
kk = sprintf(ss,"connect.c:  user provided a number\n");
write(fe,ss,kk);
#endif DEBUG
   digits();                                /* dial number */
   CKRET                                    /* user interrupt? */
   }
 else if (strcmp(argp[1],EMPTY) == 0)       /* wait for user to dial a number */
   {
   message(7);                              /* dial number now msg */
   alarm((unsigned int)90);                 /* time out if no carrier detect */
   }
 else                                       /* no number; must be direct conn */
   {
   alarm(10);                               /* time out for direct connect */
   }

 CKRET                                      /* user interrupt? */
 
 if((lpath=open(portname,O_RDWR))==ERROR)     /* error opening port */
   {
   if(errno!=EINTR)                         /* err not time-out intr signal */
     {
     alarm(0);                              /* turn off alarm */
     strcpy(argp[25],portname);             /* store port name for msg print */
     if (lpath2) {			    /* if digits */
       initmod(lpath2,OFF,ON,0,0,0,0);
       close(lpath2);
     }
     if(locfile) {
       ttyunlock(port);			    /* get rid of lock file */
       locfile=OFF;                         /* reset lockfile indicator */
       }
     message(8);                            /* can't open port */
     return;                                /* return to direc or cmd */
     }
   else                                     /* error is time-out signal */
     {
     if (retcode == 0 && attempts==0) {
       message(9);                          /* no carrier detect sensed */
     } else {
       retcode=0;
     }
     if (lpath2) {			    /* if digits */
       initmod(lpath2,OFF,ON,0,0,0,0);
       close(lpath2);
     }
     if(locfile) {
	 ttyunlock(port);		    /* get rid of lock file */
	 locfile=OFF;                       /* reset lockfile indicator */
     }
     tcflush(0, TCIFLUSH);                  /* flush input buffer */
     return;                                /* return to direc or cmd */
     }
   }
   
 else                                       /* no error opening port */
   {
   alarm(0);                                /* turn off timeout */
   if (lpath2) close(lpath2);               /* if digits */
   CKRET                                    /* user interrupt? */
   attempts=0;                              /* successful connection */
   initmod(lpath,OFF,ON,0,0,0,0);           /* initialize port parameters */
   connect=ON;
   if (vt100) vtinit();			    /* initialize vt100 screen */
   setopt();                                /* set terminal parameters */
   message(10);                             /* connected message */
#ifdef DEBUG
kk = sprintf(ss,"successful connection!\n");
write(fe,ss,kk);
#endif DEBUG

   }

 /*---------------------------------------------------------------------*/
 /* Loop forever							*/
 /*---------------------------------------------------------------------*/

 while (connect && retcode==0)        /* while connected & no user intr */
   {
   timeout.tv_sec=2;			/* timeout value in seconds */
   timeout.tv_usec=0;			/* timeout value in micro seconds */
   rdmask=(1<<0) | (1<<lpath);	 	/* check stdin, port */
   wrmask=0;
   exmask=(1<<0) | (1<<lpath);	 	/* check stdin, port */
   rc=select(32,&rdmask,&wrmask,&exmask,&timeout);

#ifdef DEBUG
kk = sprintf(ss,"\nselect rc=%d, rdmask=%o, wrmask=%o, exmask=%o, errno=%d\n",
	rc,rdmask,wrmask,exmask,errno);
write(fe,ss,kk);
#endif DEBUG

   if(rc < 0) continue ;
     
   /*-------------------------------------------------------------------*/
   /* Check for exceptions first.					*/
   /* An exception on the port is probably loss of carrier.  Hangup.    */
   /* An exception on stdin is probably loss of carrier on the input    */
   /* side.  Connection to user is gone.  Hangup and have ate exit.     */
   /*-------------------------------------------------------------------*/
   if (exmask & (1<<lpath))		/* exception returned on port */
     {
#ifdef DEBUG
kk = sprintf(ss,"exception %d found on port.  Calling hangup.\n",errno);
write(fe,ss,kk);
#endif DEBUG
     hangup();
     }

   if ((exmask & (1<<0)) && (errno != EINTR))	/* exception returned on stdin*/
     {
#ifdef DEBUG
kk = sprintf(ss,"exception %d found on stdin.  Hangup and exit.\n",errno);
write(fe,ss,kk);
#endif DEBUG
     hangup();
     exit(ERROR);
     }

   /*-------------------------------------------------------------------*/
   /* read the keyboard first in case the user typed an interrupt and   */
   /* doesn't want to see what came in on the port.			*/
   /*-------------------------------------------------------------------*/
   if (rdmask & (1<<0))			/* standard in */
     {
#ifdef DEBUG
kk = sprintf(ss,"KEYBOARD: ");
write(fe,ss,kk);
#endif DEBUG
     alarm(1);			 	/* alarm clock for fake mask */
     rc=read(0,&kbdata,1);		/* read stdin (probably keyboard) */
     alarm(0);
     if (rc > 0)			/* read stdin (probably keyboard) */
       {

#ifdef DEBUG
if (kbdata < 0x20 || kbdata > 0x7e)
  kk = sprintf(ss," 0%o ",kbdata);
else kk = sprintf(ss,"%c",kbdata);
write(fe,ss,kk);
#endif DEBUG

       if (kbdata == 0377)
         /* do nothing */;
  
       else if (kbdata == capkey) 	 /* user typed in ^b, capture key */
         {
         if(capture) capture=OFF;
         else 
            {
            capture=ON;
	    openfk();
            }
         }
         
       else if (kbdata == cmdkey)	 /* user typed in ^v, command key */
         {		

         if (txti != 0)                  /* if there is captured data */ 
         {                               /* write it to a file in case the */
           if (write(fk,bufr,txti) == ERROR)   /* the user wants to see it */  
              message(26);                 /* can't write to capture file */
           txti = 0;                       /* capture buf array index = 0 */
         }

         cmd();                          /* print menu, process command */
         state=3;
         if (retcode==QUIT) return;      /* user wants to quit */
         CKRET                           /* check for user interrupt */
         setopt();  			/* reset term params for conn */
	 
	 if (vt100) vtdraw(); 
	 else redraw();
         }
  		  
       else switch(kbdata)               /* not a command key */
         {
         case ESC:  write(lpath,&kbdata,1);           /* write esc to port */ 
#ifdef DEBUG
kk = sprintf(ss,"\nconnect.c: have written ESC to port\n");
write(fe,ss,kk);
#endif DEBUG
                    if (vt100) 
                    { 
                       ESCflag = 1; 
                       /* set the ESC flag so that later, after the ESC
                          has been processed, vt1() can be called to 
                          handle an escape sequence                       */ 
                    } 
                    break;
                    
         default  : write(lpath,&kbdata,1);  /* write character to port */
                    break;
         } 		/* end else */
       } 		/* end if read ok */
     else		/* keyboard has EOF or error */
       hangup();
     } 			/* end if keyboard has data */

   /*-------------------------------------------------------------------*/
   /* read the port second.						*/
   /*-------------------------------------------------------------------*/

#ifdef DEBUG
kk = sprintf(ss,"\nconnect.c: read the port second");
write(fe,ss,kk);
kk = sprintf(ss,"\nconnect.c: rdmask=%d, lpath=%d, 1<<lpath=%d",rdmask, lpath, 
     1<<lpath);
write(fe,ss,kk);
kk = sprintf(ss,"\nconnect.c: rdmask & (1<<lpath))=%d", 
     (rdmask & (1<<lpath)));
write(fe,ss,kk);
#endif DEBUG

   if (rdmask & (1<<lpath))	 	/* data on the port */
     {
     ptr = lndata;			/* ptr starts at lndata */
     lnstat=0;				/* nothing read yet */
     alarm(1);			 	/* alarm clock for fake mask */
     lnstat=read(lpath,lndata,LNSZ);	/* read stdin (keyboard or ? ) */
     alarm(0);
     if (lnstat > 0)	
       {
#ifdef DEBUG
kk = sprintf(ss,"connect.c: Data on Port");
write(fe,ss,kk);
#endif DEBUG

#ifdef DEBUG
kk = sprintf(ss,"PORT: ");
write(fe,ss,kk);
write(fe,lndata,lnstat);
kk = sprintf(ss,"(%d)\n",lnstat);
write(fe,ss,kk);
#endif DEBUG

       if (vt100)              /* if in vt100 mode, process data in vt2() */
       {
          vt2();  
#ifdef DEBUG
kk = sprintf(ss,"\nconnect.c: ESCflag = %d\n",ESCflag);
write(fe,ss,kk);
#endif DEBUG
          if (ESCflag)  /* if ESC received, process escape sequence in vt1()  */
          {
             vt1();
             ESCflag = 0;         
          }
       } 
       else portread();   /* if not in vt100 mode, process data in portread() */
       }
     else {				/* port has EOF or error on read */
       alarm(0);
       hangup();
     }
     }
   else 
   {
#ifdef DEBUG
kk = sprintf(ss,"\nconnect.c: No data on Port\n");
write(fe,ss,kk);
#endif DEBUG
      if (ESCflag && vt100) /* if there is no data on the port, but you are   */
      {                     /* in vt100 mode and have received an ESC on the  */
         vt1();             /* keyboard, process the escape sequence in vt1() */
         ESCflag = 0;
      }
   }                                     /* end else of "if data on the port" */
   } 	      		           /* end while connected and retcode is good */

 
#ifdef DEBUG
kk = sprintf(ss,"\nleaving connect\n");
write(fe,ss,kk);
#endif DEBUG

 retcode = 0;
 
}


/* ------------------------------------------------------------------------
   See if port is enabled by matching port name to entries and action 
   in /etc/inittab  file.  The port name is passed in.  The function
   returns 1 if the port is enabled, and 0 if it is not.
   ------------------------------------------------------------------------ */

#define MAXLLEN 256
#define INITTAB "/etc/inittab"


int
ckenabled(prt)
char prt[];
{
	FILE *fpinit;
	char *s,linebuf[MAXLLEN];
	char	pname[16];

#ifdef DEBUG
/* kk = sprintf(ss,"entering port test routine\n");
write(fe,ss,kk); */
#endif DEBUG
	if((fpinit = fopen(INITTAB,"r"))==NULL){	/*open inttab */
   	perror(MSGSTR(PORTERR, "Error opening /etc/inittab :\n")); /*MSG*/
  	 return(1);                              /* return to conn() */
	}
	while(fgets(linebuf,MAXLLEN,fpinit)){
		if(getname(linebuf,pname)){
			if(strncmp(prt,pname,(size_t)strlen(prt))== 0){
				if(getaction(linebuf,pname)){
					if((strncmp("respawn",pname,(size_t)7) ==0) || (strncmp("ondemand",pname,(size_t)8)==0)){
			/* port is enabled */
						return(1);
					}
					else{
						return(0);
					}
				}
				else {
					printf("action not specified\n");
				}
			}
		}
	}
	fclose(fpinit);
#ifdef DEBUG
/* kk = sprintf(ss,"leaving port test routine\n");
write(fe,ss,kk); */
#endif DEBUG
	return(0);
}
int
getname(from,to)
char	*from;
char	*to;
{
	char *s = from ;
	int	i ;
	/* skip leading white spaces */
	for(; *s == ' ' || *s == '\t';s++)
	if(*s == '\n' || *s == '\0')
		return(0);
	/* copy action to "to" */
	for(i=0; *s != ':' && *s != ' ' && *s != '\t' && *s != '\n' && 
		*s != '\0'; s++,i++)
		to[i] = *s ;
	to[i] = '\0' ;
	return(i) ;
}

int
getaction(from,to)
char	*from;
char	*to;
{
	char *s = from ;
	int i;
	/* skip the name of the port */
	for(; *s != ':' && *s != '\n' && *s != '\0'; s++);
	if(*s == '\n' || *s == '\0')
		return(0);
	/* skip over run level */
	for(s++; *s != ':' && *s != '\n' && *s != '\0'; s++);
	if(*s == '\n' || *s == '\0')
		return(0);
	/* skip leading white spaces */
	for(s++; *s == ' ' || *s == '\t';s++)
	if(*s == '\n' || *s == '\0')
		return(0);
	/* copy action to "to" */
	for(i=0; *s != ':' && *s != ' ' && *s != '\t' && *s != '\n' && 
		*s != '\0'; s++,i++)
		to[i] = *s ;
	to[i] = '\0' ;
	return(i) ;
}

/* old method of checking the status of the port */
struct portstatus
{       char    ps_line[14];                /* device name */
	char    ps_stat;                    /* current status */
	char    ps_rqst;                    /* requested status */
};

ckenabled_old(prt)
char prt[];
{
 int rc,fp;                                 /* return code, file pointer */
 struct portstatus name;

#ifdef DEBUG
/* kk = sprintf(ss,"entering port test routine\n");
write(fe,ss,kk); */
#endif DEBUG

 if ((fp = open("/etc/portstatus",O_RDONLY))==ERROR)
   {
   perror(MSGSTR(PORTERR, "Error opening /etc/portstatus:\n")); /*MSG*/
   return(-1);                              /* return to conn() */
   }

 do
   {
   rc = read(fp,&name,sizeof(name));        /* read a line of /etc/portstatus */

   if (strcmp(prt,name.ps_line)==0 && name.ps_stat==001)
     {
     return(1);                             /* port is enabled */
     }
   } /* end of do-while */
 while (rc != 0);                           /* until end of file */
 
 close(fp);                                 /* close /etc/portstatus */

#ifdef DEBUG
/* kk = sprintf(ss,"leaving port test routine\n");
write(fe,ss,kk); */
#endif DEBUG

 return(0);

}
