static char sccsid[] = "@(#)13  1.20  src/bos/usr/bin/ate/main.c, cmdate, bos411, 9437A411a 9/12/94 15:28:52";
/* 
 * COMPONENT_NAME: BOS main.c
 * 
 * FUNCTIONS: MSGSTR, Mmain, getdef, opendf, openfk 
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


#include "cpyrght.h"

#include <nl_types.h>
#include "ate_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ATE,n,s) 

/****************************************************************************/
/*                                                                          */
/* Module name:      main.c                                                 */
/* Description:      ate driver.  Initializes global variables, reads user  */
/*                     supplied defaults from ate.def if there is one, then */
/*                     continuously calls cmd in command.c until the user   */
/*                     quits.                                               */
/* Functions:        main - initialize variables, call cmd until user quits */
/*                   getdef - get default values from ate.def file          */
/*                   opendf - open default file; create if it doesn't exist */
/*                   openfk - open capture file                             */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*                   <sys/utsname.h>                                        */
/*   Called by:      user entering "ate" at shell command level             */
/*                   conn in connect.c                                      */
/*                   modify in modify.c                                     */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          sig_set in signals.c (set signal traps)                 */
/*                   cmd in command.c (execute user commands)               */
/*                   vt3 in vt100.c (map number pad to shifted state        */
/*                     i.e. numbers)                                        */
/*                   vt5 in vt100.c (remap keyboard to std 'G' keys)        */
/*                   cklength, ckstop, ckparity, ckrate, ckredial,          */
/*                     cktransfer, ckpacing, cktoggle, ckkey in check.c     */
/*                   portinit in portrw.c (initialize screen size)          */
/*                   vtinit in portvt.c (initialize vt100 screen image      */
/*                      variables)                                          */
/*                   message in message.c (display user messages)           */
/*   Modifies:       all global values are initialized here.                */
/*                                                                          */
/****************************************************************************/
#include <locale.h>
#include <stdlib.h>
#include "modem.h"
#include <sys/utsname.h>

char capchar[4],cmdchar[4],retchar[4];


/* -------------------------------------------------------------------------
   main program:  initialize global values, get user defaults from ate.def
   file (or create file) in current directory, and call cmd until user quits.
   ------------------------------------------------------------------------- */

main()
{
 struct utsname utsn;              /* current system information */

	(void)setlocale(LC_ALL,"");		/* for setlocale() */
	catd = catopen(MF_ATE,NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
#ifdef DEBUG

if ((fe=open("/tmp/DEBUG",O_WRONLY|O_TRUNC|O_CREAT,0766))==ERROR)
  {
  kk = sprintf(ss,"Can't open/create \"debug\" debugging file.\n");
  fe=1;				   /* write debug stmts to stdout */
  write(fe,ss,kk);
  }
  chmod("/tmp/DEBUG",0766) ;		/* this is to fix the bug in create*/
kk = sprintf(ss,"entering main\n");
write(fe,ss,kk);
#endif DEBUG

 speed   = 1200;                   /* baud rate */
 bits    = 8;                      /* bits per character */
 stop    = 1;                      /* number of stop bits */
 parity  = 0;                      /* no parity */
 redial  = 0;                      /* redialing interval */
 attempts= 0;                      /* redialing attempts */
 connect = OFF;                    /* no connection made */
 locfile = OFF;                    /* not using port     */
 state   = 0;                      /* program state */
 menu    = 0;                      /* no menu being written */
 keyappl = 0;                      /* no vt100 keypad application mode */
 xmit    = 0;                      /* file transfer not in progress */
 retcode = (int)NULL;              /* return code */
 sectnum = 0;                      /* sector number in file transfers */
 capkey  = 002;                    /* capture key */
 cmdkey  = 026;                    /* command key */
 retkey  = 022;                    /* return key */
 strcpy(capchar,"002");
 strcpy(cmdchar,"026");
 strcpy(retchar,"022");

 kbdata   = (int)NULL;                  /* characters from the keyboard */
 moddata  = (int)NULL;                  /* characters from the port (modem) */
 transfer = 'p';                   /* file transfer method */
 pacing   = '0';                   /* interval pacing off */
 Echo     = OFF;                   /* don't echo characters */
 xctl     = ON ;                   /* xon/xoff control turned on */
 capture  = OFF;                   /* no data capture */
 xlatcr   = OFF;                   /* don't translate CR to CR-LF */
 vt100    = OFF;                   /* no VT100 emulation */
 awrap    = ON;                    /* autowrap in VT100 emulation */

 strcpy(dialp,"ATDT");             /* modem prefix, such as "AT DT" */
 strcpy(dials," ");                /* modem suffix */
 strcpy(number,"0");               /* telephone number to dial */

 lpath = ERROR;                    /* port fildes for tty0 */
 fk    = ERROR;                    /* kapture filedes */
 fd    = NULL;                     /* directory file pointer */
 fs    = ERROR;                    /* send filedes */
 fr    = ERROR;                    /* receive filedes */
 temp  = ERROR;                    /* temporary shared file */
 
 strcpy(port,"tty0");              /* port, such as tty0 */
 strcpy(kapfile,"kapture");        /* name of capture file */
 strcpy(dirfile,"/usr/lib/dir");   /* name of directory file */
 strcpy(sndfile,"");               /* name of send file */
 strcpy(rcvfile,"");               /* name of receive file */
 strcpy(tempfil,"");               /* name of temporary shared file */
                                   /* tempfil is not currently being used */
                                   /* anywhere */
 
 uname(&utsn);                     /* get name of current system */
 strcpy(machine,utsn.nodename);    /* save machine node name for menu prints */

 argp[0] =arg0;                    /* set pointers to storage areas */
 argp[1] =arg1;
 argp[2] =arg2;
 argp[3] =arg3;
 argp[4] =arg4;
 argp[5] =arg5;
 argp[6] =arg6;
 argp[7] =arg7;
 argp[8] =arg8;
 argp[9] =arg9;
 argp[10]=arg10;
 argp[11]=arg11;
 argp[12]=arg12;
 argp[13]=arg13;
 argp[14]=arg14;
 argp[15]=arg15;
 argp[16]=arg16;
 argp[17]=arg17;
 argp[18]=arg18;
 argp[19]=arg19;
 argp[20]=arg20;
 argp[21]=arg21;
 argp[22]=arg22;
 argp[23]=arg23;
 argp[24]=arg24;
 argp[25]=arg25;

 tcgetattr(0,&option);		/* store present terminal parameters */
#ifdef TXGETLD
 ioctl(0,TXGETLD,disp_buff);	/* store line discipline parameter */
#endif /* TXGETLD */

 sig_set();			/* set signal traps */
 portinit();
 menuinit();			/* initialize menus */

 getdef();			/* get user defaults */
 if (vt100) 
    {
#ifdef HFT
    vt3();			/* map keypad to numbers */
#endif /* HFT */
    vtinit();			/* initialize vt100 screen */
    }
 
 while (retcode != QUIT) cmd();	/* execute user commands */

 close(fk);			/* close the kapture file */
 if (vt100) 
    {
#ifdef HFT
    vt5();			/* remap keypad to base state */
#endif /* HFT */
    vtclose();			/* close the vt100 screen */
    }
 tcsetattr(0,TCSANOW,&option);  /* restore terminal parameters */
#ifdef TXSETLD
 ioctl(0,TXSETLD,disp_buff);	/* restore line discipline parameter */
#endif /* TXSETLD */
 
#ifdef DEBUG
kk = sprintf(ss,"leaving main\n");
write(fe,ss,kk);
close(fe);
#endif DEBUG

 exit(0);

}


/* --------------------------------------------------------------------------
   default - read ate.def file containing user defaults.  Format is default
   name followed by new value, one entry per line.
   -------------------------------------------------------------------------- */
getdef()
{
 int i,rc;
 char *p;

#ifdef DEBUG
/* kk = sprintf(ss,"Entering getdef routine\n");
write(fe,ss,kk); */
#endif DEBUG

 opendf(); 				/* open default file */
 if (df == ERROR) return;               /* couldn't open file */

 while (ON)                             /* do forever */
   {
   i=0;
   strcpy(userin,EMPTY);
   strcat(userin,EMPTY); 
   while ( (rc=read(df,&userin[i],1)) != 0 && userin[i] != '\n') 
     {
     i++;
     }

   if (rc == ERROR)                     /* bad read */
     {
     message(11);                       /* can't create/open default file */
     break;                             /* exit loop */
     }
   if (rc == 0) break;                  /* end of file--exit while loop */

   strcpy(argp[0],EMPTY);
   strcpy(argp[1],EMPTY);
   argcnt = sscanf(userin,"%40s %40s",argp[0],argp[1]);
   if (argcnt==1) strcpy(argp[1],"");

   p = argp[0];
   if (mb_cur_max > 1) {	/* multibyte code */
	wchar_t *wc, *tmpwc;
	int mbcount, n;
	
	n = (strlen(p) + 1) * sizeof(wchar_t);
	tmpwc = wc = (wchar_t *)malloc(n);

	/*convert multibyte string to widechar */
	mbcount = mbstowcs(wc, p, n);
	if (mbcount < 0) 
		printf(MSGSTR(MAIN_MB, "main: Error in converting to widechar.\n"));
	else {
		int i;
		for (i = 0; i < mbcount && !iswspace((wint_t) *wc); i++)
		{
		   /* convert variable to all caps */
		    *wc = towupper((wint_t) *wc);		
		    wc++;
		}
		/* convert widechar back to multibyte */
		mbcount = wcstombs(p, tmpwc, n);
		if (mbcount < 0)
			printf(MSGSTR(MAIN_WC, "main: Error in converting to multibyte.\n"));
	}
   }	
   else {			/* single byte code */
  	 while (*p != '\0' && *p != ' ' && p != NULL)
   	{
     		*p = toupper(*p);    /* convert variable to all caps */
   		 p++;
     	}
   }
   /* check all entries for validity.  Don't use ones that are invalid */
   
   if (strcmp(argp[0],MSGSTR(DFLEN, "LENGTH")) == 0 && cklength(argp[1]) != ERROR) /*MSG*/
     bits = atoi(argp[1]);              /* word length */

   if (strcmp(argp[0],MSGSTR(DFSTOP, "STOP")) == 0 && ckstop(argp[1]) != ERROR) /*MSG*/
     stop = atoi(argp[1]);              /* number of stop bits */

   if (strcmp(argp[0],MSGSTR(DFPARITY, "PARITY")) == 0 && ckparity(argp[1]) != ERROR) /*MSG*/
     parity = atoi(argp[1]);            /* parity */

   if (strcmp(argp[0],MSGSTR(DFRATE, "RATE")) == 0 && ckrate(argp[1]) != ERROR) /*MSG*/
     speed = atoi(argp[1]);             /* baud rate */

   if (strcmp(argp[0],MSGSTR(DFDEVICE, "DEVICE")) == 0 && strcmp(argp[1],"") != 0) /*MSG*/
     strcpy(port,argp[1]);              /* port name */

   if (strcmp(argp[0],MSGSTR(DFINITIAL, "INITIAL")) == 0) /*MSG*/
     strcpy(dialp,argp[1]);             /* modem prefix like ATDT */

   if (strcmp(argp[0],MSGSTR(DFFINAL, "FINAL")) == 0) /*MSG*/
     strcpy(dials,argp[1]);             /* modem suffix */

   if (strcmp(argp[0],MSGSTR(DFWAIT, "WAIT")) == 0 && ckredial(argp[1],'W') != ERROR) /*MSG*/
     redial = atoi(argp[1]);            /* redial interval */
 
   if (strcmp(argp[0],MSGSTR(DFATTEMPTS, "ATTEMPTS")) == 0 && ckredial(argp[1],'A') != ERROR) /*MSG*/
     attempts = atoi(argp[1]);          /* times to redial */

   if (strcmp(argp[0],MSGSTR(DFXFER, "TRANSFER")) == 0 && cktransfer(argp[1]) != ERROR) /*MSG*/
     transfer = *argp[1];               /* file transfer type */

   if (strcmp(argp[0],MSGSTR(DFCHAR, "CHARACTER")) == 0 && ckpacing(argp[1]) != ERROR) /*MSG*/
     pacing = *argp[1];                 /* pacing character */

   if (strcmp(argp[0],MSGSTR(DFNAME, "NAME")) == 0 && strcmp(argp[1],"") != 0) /*MSG*/
     {
     strcpy(kapfile,argp[1]);           /* capture file name */
     }

   if (strcmp(argp[0],MSGSTR(DFLFS, "LINEFEEDS")) == 0 && cktoggle(argp[1],'L') != ERROR) /*MSG*/
     xlatcr = atoi(argp[1]);           	/* transfer CR to CR-LF */
     
   if (strcmp(argp[0],MSGSTR(DFECHO, "ECHO"))      == 0 && cktoggle(argp[1],'E') != ERROR) /*MSG*/
     Echo = atoi(argp[1]);              /* echo toggle */

   if (strcmp(argp[0],MSGSTR(DFVT100, "VT100"))     == 0 && cktoggle(argp[1],'V') != ERROR) /*MSG*/
     vt100 = atoi(argp[1]);             /* vt100 toggle */

   if (strcmp(argp[0],MSGSTR(DFWRITE, "WRITE"))     == 0 && cktoggle(argp[1],'W') != ERROR) /*MSG*/
     {
     capture = atoi(argp[1]);           /* capture toggle */
     if (capture) openfk();		/* open the capture file */
     }

   if (strcmp(argp[0],MSGSTR(DFXONOFF, "XON/XOFF"))  == 0 && cktoggle(argp[1],'X') != ERROR) /*MSG*/
     xctl = atoi(argp[1]);              /* xon/xoff control toggle */
 
   if (strcmp(argp[0],MSGSTR(DFDIR, "DIRECTORY")) == 0 && strcmp(argp[1],"") != 0) /*MSG*/
     strcpy(dirfile,argp[1]);           /* dialing directory file name */

   if (strcmp(argp[0],MSGSTR(DFCAPKEY, "CAPTURE_KEY")) == 0 && (rc=ckkey(argp[1])) != ERROR) /*MSG*/
     capkey = rc;                       /* capture key (^b) */

   if (strcmp(argp[0],MSGSTR(DFMMENUKEY, "MAINMENU_KEY")) == 0 && (rc=ckkey(argp[1])) != ERROR) /*MSG*/
     cmdkey = rc;                       /* command key (^v) */

   if (strcmp(argp[0],MSGSTR(DFPREVKEY, "PREVIOUS_KEY")) == 0 && (rc=ckkey(argp[1])) != ERROR) /*MSG*/
     retkey = rc;                       /* return key (^r) */

   if ((0 == strcmp(argp[0],MSGSTR(DFVT100, "AUTOWRAP"))) &&
       (ERROR != cktoggle(argp[1],'A')))
     awrap = atoi(argp[1]);             /* autowrap toggle */

   }  /* end of while */

 close(df);                             /* close default file */

#ifdef DEBUG
/* kk = sprintf(ss,"Leaving getdef\n");
write(fe,ss,kk); */
#endif DEBUG
}


#define NEWFILE "\
LENGTH       %d\n\
STOP         %d\n\
PARITY       %d\n\
RATE         %d\n\
DEVICE       %s\n\
INITIAL      %s\n\
FINAL        %s\n\
WAIT         %d\n\
ATTEMPTS     %d\n\
TRANSFER     %c\n\
CHARACTER    %c\n\
NAME         %s\n\
LINEFEEDS    %d\n\
ECHO         %d\n\
VT100        %d\n\
WRITE        %d\n\
XON/XOFF     %d\n\
DIRECTORY    %s\n\
CAPTURE_KEY  %s\n\
MAINMENU_KEY %s\n\
PREVIOUS_KEY %s\n\
AUTOWRAP     %d\n",\
bits,stop,parity,speed,port,dialp,dials,redial,attempts,transfer,\
pacing,kapfile,xlatcr,Echo,vt100,capture,xctl,dirfile,capchar,cmdchar,retchar,\
awrap


/* --------------------------------------------------------------------------
   opendf - open default file.  This routine stores either a valid file
   descriptor for the ate.def file or ERROR in the global variable df.
   -------------------------------------------------------------------------- */
opendf()
{
char s[526];
int i;

#ifdef DEBUG
/* kk = sprintf(ss,"entering opendf\n");
write(fe,ss,kk); */
#endif DEBUG

 df = ERROR;
 if ((df=open("ate.def",O_RDONLY))==ERROR)
   {
   if (errno == ENOENT)			/* file doesn't exist */
     {
     if ((df=open("ate.def",O_WRONLY|O_CREAT,
             0766))==ERROR)             /* couldn't create */
       {
       message(11);                     /* can't open/create ate.def */
       return(ERROR);
       }
     else                               /* successfully created file */
       {
       i = sprintf(s,NEWFILE);          /* write data into file */
       write(df,s,i);
       close(df);                       /* close file */
       df=ERROR;
       }
     }                                  /* end of if-file-doesn't-exist */
   else /* some other error */
     {
     message(11);                       /* can't open/create ate.def */
     return(ERROR);
     }
   }                                    /* end of if-ERROR */

#ifdef DEBUG
/* kk = sprintf(ss,"leaving opendf\n");
write(fe,ss,kk); */
#endif DEBUG

return(0);

}

                                                                  
/* --------------------------------------------------------------------------
   Open capture file.  The name of the capture file is saved in the global
   variable kapfile.
   -------------------------------------------------------------------------- */
openfk()
{
static char oldfile[41] = "kapture";    /* name of previous capture file */

   /*-------------------------------------------------------------------*/
   /* choices:  file is not open yet          - open it 		*/
   /*           capture file name has changed - close old, open new	*/
   /*           capture file is already open  - do nothing		*/
   /*-------------------------------------------------------------------*/
   if (fk == ERROR) 			/* file is not open yet */
     {
     fk=open(kapfile,O_WRONLY|O_APPEND|O_CREAT,0766);
     }
   else if (fk != ERROR &&		  /* a file is already open */
       (strcmp(oldfile,kapfile) != (int)NULL)) /* but the name has changed */
     {
     close(fk);
     fk=open(kapfile,O_WRONLY|O_APPEND|O_CREAT,0766);
     }

   /*-------------------------------------------------------------------*/
   /* if the file transfer failed, tell user.  Otherwise, save new name */
   /*-------------------------------------------------------------------*/
   if (fk == ERROR)
     {
     message(01);                        /* can't create or open file */
     capture=OFF;
     }
   else strcpy(oldfile,kapfile);

}

