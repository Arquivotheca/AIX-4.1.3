static char sccsid[] = "@(#)25	1.9  src/bos/usr/bin/ate/setups.c, cmdate, bos411, 9428A410j 7/23/93 14:21:40";
/* 
 * COMPONENT_NAME: BOS setups.c
 * 
 * FUNCTIONS: initmod, retopt, setopt 
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
/* Module name:      setups.c                                               */
/* Description:      set parameters on port (modem) and user's terminal     */
/*                     for a connection.                                    */
/* Functions:        setopt - set terminal parameters for connection        */
/*                   retopt - restore terminal parameters to original       */
/*                     settings                                             */
/*                   initmod - set port parameters for connection           */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      conn, digits in connect.c                              */
/*                   alter in alter.c                                       */
/*                   cmd in command.c                                       */
/*                   sigrout in signal.c                                    */
/*                   vt2 - portvt.c                                         */
/*                   hangup in hangup.c                                     */
/*                   modify in modify.c                                     */
/*                   xrcv, xsend in xmodem.c                                */
/*   Receives:       initmod receives flags                                 */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          nothing                                                */
/*   Modifies:       nothing                                                */
/*                                                                          */
/****************************************************************************/

#include "modem.h"


/* ---------------------------------------------------------------------------
   Set options routine - set terminal parameters for a connection
   --------------------------------------------------------------------------- */
setopt()

{
 struct termios to;
 
 tcgetattr(0, &to);
 to.c_iflag=BRKINT+IGNPAR;             /* intr on break, ignore
                                          parity errors, no stripping  */
 to.c_oflag=option.c_oflag&~OPOST;     /* turn off post processing ==> chars
                                          are transmitted without change */
 to.c_cflag=option.c_cflag;            /* hardware control */
 to.c_lflag=ISIG;                      /* enable intr/quit signals, disable
                                          canonical input, no echo, no flush
                                          after intr/quit */
 to.c_cc[VINTR]=retkey;                /* changes DEL interrupt to ^r */
 to.c_cc[VQUIT]= -1;                     /* quit is NULL (^space) */
 to.c_cc[VERASE]= -1;                    /* erase off */
 to.c_cc[VKILL]= -1;                     /* kill off */
 to.c_cc[VMIN]=1;                      /* min chars to satisfy read */
 to.c_cc[VTIME]=0;                     /* 10th of seconds to satisfy read */
 to.c_cc[VSUSP]= -1;                   /* suspend off */
 to.c_cc[VDSUSP]= -1;                  /* delayed suspend off */

 if (Echo) to.c_lflag |= ECHO;          /* set echo on */
 else to.c_lflag &= ~ECHO;              /* set echo off */
 
 if (xctl) to.c_iflag |= (IXON+IXOFF);  /* set xon-xoff on */
 else to.c_iflag &= ~(IXON+IXOFF);      /* set xon-xoff off */
 
 tcsetattr(0,TCSANOW,&to);              /* set stdin termios parameters */
 
#ifdef DEBUG
kk = sprintf(ss,"Leaving setopt with:\n");
write(fe,ss,kk);
kk = sprintf(ss,"   i_flag    o_flag    c_flag    l_flag     line         c_cc  \n");
write(fe,ss,kk);
kk = sprintf(ss,"  --------  --------  --------  --------  --------  --------------\n");
write(fe,ss,kk);
kk = sprintf(ss,"%10o%10o%10o%10o  %o %o %o %o %o %o\n", 
  to.c_iflag, to.c_oflag, to.c_cflag, to.c_lflag,
  to.c_cc[0],to.c_cc[1],to.c_cc[2],to.c_cc[3],to.c_cc[4],to.c_cc[5]);
write(fe,ss,kk);
#endif DEBUG

}


/* --------------------------------------------------------------------------
   Restore options routine - restore original terminal parameters
   -------------------------------------------------------------------------- */
retopt()
{
 struct termios to;

 to = option;
 to.c_cc[VINTR] = retkey;                      /* change DEL interrupt to ^r */
 to.c_cc[VQUIT] = -1;                          /* quit is NULL (^space) */
 tcsetattr(0,TCSANOW,&to);                 /* set termios with stored values */

#ifdef DEBUG
kk = sprintf(ss,"Leaving retopt with:\n");
write(fe,ss,kk);
kk = sprintf(ss,"   i_flag    o_flag    c_flag    l_flag     line         c_cc  \n");
write(fe,ss,kk);
kk = sprintf(ss,"  --------  --------  --------  --------  --------  --------------\n");
write(fe,ss,kk);
kk = sprintf(ss,"%10o%10o%10o%10o  %o %o %o %o %o %o\n", 
  to.c_iflag, to.c_oflag, to.c_cflag, to.c_lflag,
  to.c_cc[0],to.c_cc[1],to.c_cc[2],to.c_cc[3],to.c_cc[4],to.c_cc[5]);
write(fe,ss,kk);
#endif DEBUG
}


/* -------------------------------------------------------------------------
   initialize modem (set port parameters).  This routine is passed the
   following flags:
     portfd   - port file descriptor
     clocal   - 0 is a dialup, 1 is a local line
     hupcl    - 0 means don't hangup on last close, 1 means do hangup
     iflagset - 0 or 0          
     iflagclr - 0 or IXON+IXOFF  
     cflagset - 0 or CS8
     cflagclr - 0 or PARENB      
     
   The clocal setting is remote except in the digits routine in connect.c,
   which uses the local setting for opening the local device to write the
   dialing digits to the modem.
  
   The hupcl setting is always on except in the modify and alter routines
   and in the digits routine in connect.c, where you don't want to close the
   port after you've sent the digits to the modem because you'll lose the
   connection.
                        
   Xmodem passes the second iflag and cflag settings to insure the transfer is
   8-bits, no parity, no xon/xoff.  The function uses as follows:
     iflag & ~iflagset (777 or 777) | iflagclr (000 or 012000)
     cflag & ~cflagset (777 or 717) | cflagclr (000 or 000400)
   The effect on the iflag is to (a) make no change to the flag at all, or
   (b) turn on xon/xoff.  The effect on the cflag is to (a) make no change
   to the flag at all, or (b) turn off parity and turn on the 8-bit word
   length.
   ------------------------------------------------------------------------- */
initmod(portfd,clocal,hupcl,iflagset,iflagclr,cflagset,cflagclr)
int portfd,clocal,hupcl,iflagset,iflagclr,cflagset,cflagclr;

{
 struct termios to;
 int i;

 to.c_iflag  = IGNPAR;     /* ignore chars with parity errors */
 to.c_oflag  = 0;          /* clear output flag */
 to.c_cflag  = CREAD;      /* enable character receiver */
 to.c_lflag  = 0;          /* clear local flag */
 to.c_cc[VINTR]  = -1;     /* set intr to unlikely value to turn off */
 to.c_cc[VQUIT]  = -1;     /* set quit to unlikely value to turn off */
 to.c_cc[VERASE]  = -1;    /* ERASE = NULL */
 to.c_cc[VKILL]  = -1;     /* KILL = NULL */
 to.c_cc[VMIN]  = 1;       /* min chars to satisfy a read */
 to.c_cc[VTIME]  = 0;      /* timeout (1/10 seconds) to satisfy a read */
 to.c_cc[VEOL2] = -1;      /* EOL2 = NULL */
 for (i = VSUSP; i < NCCS; i++)	/* clear all other character, but skip   */
	to.c_cc[i]  = -1;	/* VSTART & VSTOP. They'll get set later */

 switch (speed)            /* set speed */
   {
   case    0   : to.c_cflag |= B0;
                 break;
   case   50   : to.c_cflag |= B50;
                 break;
   case   75   : to.c_cflag |= B75;
                 break;
   case  110   : to.c_cflag |= B110;
                 break;
   case  134   : to.c_cflag |= B134;
                 break;
   case  150   : to.c_cflag |= B150;
                 break;
   case  300   : to.c_cflag |= B300;
                 break;
   case  600   : to.c_cflag |= B600;
                 break;
   case 1200   : to.c_cflag |= B1200;
                 break;
   case 1800   : to.c_cflag |= B1800;
                 break;
   case 2400   : to.c_cflag |= B2400;
                 break;
   case 4800   : to.c_cflag |= B4800;
                 break;
   case 9600   : to.c_cflag |= B9600;
                 break;
   case 19200  : to.c_cflag |= EXTA;
                 break;
   default     : to.c_cflag |= B1200;
                 break;
   }

 switch (bits)             /* set character size */
   {
   case 7  : to.c_cflag |= CS7;
             break;
   case 8  : to.c_cflag |= CS8;
             break;
   default : to.c_cflag |= CS8;
             break;
   }

 switch (parity)           /* set parity */
   {
   case 0  : to.c_cflag &= ~PARENB;           /* no parity */
             break;                    
   case 1  : to.c_cflag |= PARENB + PARODD;   /* odd parity */
             break;
   case 2  : to.c_cflag |= PARENB;            /* even parity */
             break;
   default : break;
   }
   
 switch (stop)              /* set stop bits */
   {
   case 1  : break;
   case 2  : to.c_cflag |= CSTOPB;
             break;
   default : break;
   }

 if (xctl) {
	to.c_iflag |= (IXON+IXOFF);		/* set xon-xoff on */
	to.c_cc[VSTART] = (char)0x11;		/* set xon char ctrl-Q */
	to.c_cc[VSTOP]  = (char)0x13;		/* set xoff char ctrl-S */
 } else {
	to.c_iflag &= ~(IXON+IXOFF);	/* set xon-xoff off */
	to.c_cc[VSTART] = -1;		/* clear xon char */
	to.c_cc[VSTOP] = -1;		/* clear xoff char */
 }
 
 if(clocal) to.c_cflag|=CLOCAL;         /* local, not dialup */
 if(hupcl) to.c_cflag|=HUPCL;           /* hang up on last close */

 to.c_iflag = to.c_iflag & ~iflagclr | iflagset; /* see note at start of initmod */
 to.c_cflag = to.c_cflag & ~cflagclr | cflagset; /* These are used by xmodem */
 
 tcsetattr(portfd,TCSANOW,&to);         /* set termios parameters */
 
#ifdef DEBUG
kk = sprintf(ss,"Leaving initmod with:\n");
write(fe,ss,kk);
kk = sprintf(ss,"   i_flag    o_flag    c_flag    l_flag     line         c_cc  \n");
write(fe,ss,kk);
kk = sprintf(ss,"  --------  --------  --------  --------  --------  --------------\n");
write(fe,ss,kk);
kk = sprintf(ss,"%10o%10o%10o%10o  %o %o %o %o %o %o\n", 
  to.c_iflag, to.c_oflag, to.c_cflag, to.c_lflag,
  to.c_cc[0],to.c_cc[1],to.c_cc[2],to.c_cc[3],to.c_cc[4],to.c_cc[5]);
write(fe,ss,kk);
#endif DEBUG

}

