static char sccsid[] = "@(#)31	1.7  src/bos/usr/bin/ate/xmodem.c, cmdate, bos411, 9428A410j 8/10/91 19:19:07";
/* 
 * COMPONENT_NAME: BOS xmodem.c
 * 
 * FUNCTIONS: READCHAR, SENDCHAR, xrcv, xsend 
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
/* Module name:      xmodem.c                                               */
/* Description:      transfer a file using xmodem protocol                  */
/* Functions:        xrcv - receive a file                                  */
/*                   xsend - send a file                                    */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c                                       */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: can't open receive or send file                        */
/*                   user interrupt                                         */
/*                   other side isn't ready                                 */
/*   Calls:          message in message.c (print user msgs)                 */
/*                   initmod in setups.c (initialize modem)                 */
/*   Modifies:       sectnum, lpath, state, retcode                         */
/*                                                                          */
/****************************************************************************/

#include "modem.h"

#define SECSIZ   128                         /* block size used by xmodem */
#define XBUFSZ    SECSIZ+3                    /* read buffer size */
#define ERRORMAX 9                           /* maximum number of errors */
#define CHARMASK 0xFF                        /* character mask */

/* file transfer special characters */
#define TIMEOUT (-1)                         /* timeout character */
#define SOH    0x01                          /* start of block character */
#define EOT    0x04                          /* end of transmission */
#define ACK    0x06                          /* acknowledge block */
#define NAK    0x15                          /* error in transmission */
#define SUB    0x1a                          /* end of file (a la CP/M, DOS) */

/* the two macros following were created
   for optimization purposes */
   
#define READCHAR(n)    alarm(n); lnstat=read(lpath,&ch,1); alarm(0); \
                       if (lnstat==ERROR) c=(-1); else c=(ch & CHARMASK)

#define SENDCHAR(x)    ch=x; write(lpath,&ch,1)

char xbufr[XBUFSZ];                            /* text buffer */

/* ------------------------------------------------------------------------
   xmodem receive routine - receive a file from remote end.
   ------------------------------------------------------------------------ */
xrcv()
{
 int firstchar;                              /* first character read */
 int sectcurr;                               /* sector number sent */
 int sectcomp;                               /* ones complement of sectcurr */
 int cksum;                                  /* checksum transmitted by remote */
 int checksum;                               /* calculated checksum */
 int errors=0;                               /* cumulative errors on this sector */
 int i,j;
 int wait=0;
 register char *p;                           /* pointer to character in array */
 int c,lnstat;                               /* character, line status */
 char ch;                                    /* character */
 int errcount;				     /* error count in reading */
 
#ifdef DEBUG
kk = sprintf(ss,"entering xrcv()\n");
write(fe,ss,kk);
#endif DEBUG
 
 state = 7;                                  /* file receipt state */
 retcode = (int)NULL;                             /* normal retcode value */
 errcount = 0 ;

 if ((fr=open(rcvfile, O_WRONLY|O_APPEND|O_CREAT,0766))==ERROR)
   {
   message(17);                              /* can't create/open file */
   return;                                   /* return to cmd */
   }

 message(20);                                /* ready to receive message */

 sectnum=1;                                  /* initialize sector number */
 
 CKRET                                       /* check for user interrupt */
   
  /* Set 8-bit length and cancel xon/xoff and parity
     (possibly at variance with menu settings) */
 initmod(lpath,OFF,ON,0,IXON+IXOFF,CS8,PARENB);

 SENDCHAR(NAK);                              /* send "ready to receive" */
#ifdef DEBUG
kk = sprintf(ss,"sent NAK\n");
write(fe,ss,kk);
#endif DEBUG

 do
   {

   do                                        /* wait for SOH, EOT or TIMEOUT */
     {
#ifdef DEBUG
kk = sprintf(ss,"waiting for SOH,TIMEOUT,EOT\n");
write(fe,ss,kk);
#endif DEBUG
     READCHAR(10);                           /* wait 10 seconds for a character */
     firstchar=c;
     CKRET                                   /* check for user interrupt */
     }
   while (firstchar!=SOH && firstchar!=TIMEOUT && firstchar!=EOT);
 
   /* TIMED OUT */
   if(firstchar==TIMEOUT)                    /* timed out */
     {
     errors++;                               /* increment error count */
     wait++;                                 /* increment nothing received count */
     /* NOTE:  The xmodem offered by CompuServ might take as long as
               20 seconds for the file transfer to begin.  In order to
               allow for this, send a NAK only every 20 seconds instead
               of every 10 seconds */
     if ((wait%2)==0)			     /* wait is even-numbered */
     SENDCHAR(NAK);                          /* send "ready to receive" */
#ifdef DEBUG
kk = sprintf(ss,"sent another NAK\n");
write(fe,ss,kk);
#endif DEBUG
     }
   else wait=0;                              /* clear nothing received count */
 
   /* GOT A START OF HEADER */
   if(firstchar==SOH)                        /* got a start of header */
     {
#ifdef DEBUG
kk = sprintf(ss,"got a SOH\n");
write(fe,ss,kk);
#endif DEBUG
     i = 0;
     do                                      /* continue reading until an entire  */
       {                                     /* buffer is received--sector number */
       j = read(lpath,&xbufr[i],(XBUFSZ-i));   /* complement of sector number, 128  */
       i = i+j;                              /* bytes of data and a checksum      */
       if(j <= 0) errcount++ ;
       if(errcount > 5){
		message(34);
		hangup();
		return ;
       }

       CKRET                                 /* check for user interrupt */
       }
     while (i < XBUFSZ && retcode == (int)NULL);
     
#ifdef DEBUG
write(fe,"\n",1);
write(fe,xbufr,XBUFSZ);
write(fe,"\n",1);
#endif DEBUG

     sectcurr=xbufr[0] & CHARMASK;            /* sector number being sent */
     sectcomp=xbufr[1] & CHARMASK;            /* 1's complement of sector number */
     cksum=xbufr[XBUFSZ-1] & CHARMASK;         /* checksum sent by remote end */

     if ((sectcurr+sectcomp)!=CHARMASK)      /* bad complement */
       {
       message(32);                          /* sector number not correct */
       errors++;                             /* increment error count */
       c=0;
       while (c != TIMEOUT) {READCHAR(1);}   /* wait for silence */
       SENDCHAR(NAK);                        /* send negative acknowledgement */
       }

     else if ((sectcurr < (sectnum&CHARMASK)) ||
         (sectnum == 1 && sectcurr == 0377)) /* duplicate sector received */
       {
       
       /* in xmodem protocol, the sector number is 8 bits.  This means
          it "rolls over" at 256 and starts again at zero.  To keep the
          sector numbering displayed for the user continuous, the
          following formula is used:
                   sectnum
                   ------- x 256 + sectcurr
                     256                                                 */
       
       sprintf(argp[25],"%d",((int) sectnum/256)*256+sectcurr);
       message(30);                          /* sector received twice */
       errors++;                             /* increment error count */
       SENDCHAR(ACK);                        /* send acknowledgement */
       }

     else if (sectcurr > (sectnum&CHARMASK)) /* sector GT expected */
       {
       sprintf(argp[25],"%d",((int) sectnum/256)*256+sectcurr);
       message(31);                          /* wrong sector */
       errors++;                             /* increment error count */
       c=0;
       while (c != TIMEOUT) {READCHAR(1);}   /* wait for silence */
       SENDCHAR(NAK);                        /* send negative acknowledgement */
       }

     else
       {
       checksum=0;
       p = &xbufr[2];                         /* pointer to 128 chars received */
       while (p < &xbufr[2]+SECSIZ)           /* for each char in sector */
         {
         *p = *p & CHARMASK;                 /* mask it to 8 bits */
         checksum = (checksum + *p);         /* calculate the checksum */
	 p++;                                /* increment pointer */
         }
       checksum &= CHARMASK;                 /* mask the calculated checksum */
       
       if (checksum != cksum)                /* checksums don't match */
         {
         message(29);                        /* checksum error */
         errors++;                           /* increment error count */
         c=0;
         while (c != TIMEOUT) {READCHAR(1);} /* wait for silence */
         SENDCHAR(NAK);                      /* send negative acknowledgement */
         }
       else 
         {
         message(28);                        /* receiving block */
         errors=0;                           /* re-initialize error count */
         sectnum++;                          /* increment sector number */
	 if (write(fr,&xbufr[2],SECSIZ)==ERROR) /* write sector into receive file */
           {                                 /* if the user hasn't interrupted */
           if (retcode==(int)NULL) message(27);   /* can't write to receive file */
           initmod(lpath,OFF,ON,0,0,0,0);    /* put port back (xon,8-bit,parity) */
           close(fr);                        /* close receive file */
           retcode=(int)NULL;                     /* set retcode to normal */
           return;
           }
         SENDCHAR(ACK);                      /* send acknowledgement */
#ifdef DEBUG
kk = sprintf(ss,"sent an ACK\n");
write(fe,ss,kk);
#endif DEBUG
         } 
       }                                     /* end else */
     }                                       /* end SOH processing */
   }      
 while (firstchar!=EOT && errors<ERRORMAX && retcode==(int)NULL);
 
 CKRET                                       /* check for user interrupt */
   
 /* GOT AN EOT (end of transmission) */
 if (firstchar==EOT && errors<ERRORMAX)
   {
#ifdef DEBUG
kk = sprintf(ss,"got an EOT\n");
write(fe,ss,kk);
#endif DEBUG
   SENDCHAR(ACK);                            /* send acknowledgement to EOT */
   initmod(lpath,OFF,ON,0,0,0,0);            /* restore user's settings */
   message(15);                              /* file transfer complete */
   }
 else if (wait == ERRORMAX)                  /* haven't received anything */
   {
   initmod(lpath,OFF,ON,0,0,0,0);            /* restore user's settings */

   /* NOTE:  Have only been waiting for 90 seconds, not 100.  
             Not fixed due to the pub changes which would be required. */
   sectnum = 100;                            /* global variable used for msg print */
   message(2);                               /* nothing sent for 100 seconds */
   }
 else
   {
   initmod(lpath,OFF,ON,0,0,0,0);            /* restore user's settings */
   message(34);                              /* too many errors */
   }
 
 close(fr);                                  /* close receive file */

#ifdef DEBUG
kk = sprintf(ss,"leaving xrcv\n");
write(fe,ss,kk);
#endif DEBUG

}


/* --------------------------------------------------------------------------
   Xmodem send routine - send a file to remote computer
   -------------------------------------------------------------------------- */
xsend()
{
  int attempt=0, firstchar;
  int checksum;                              /* calculated checksum */
  int i,j;
  register char *p;                          /* pointer to character */
  int c,lnstat;                              /* character, line status */
  char ch;                                   /* character */
 
#ifdef DEBUG
kk = sprintf(ss,"entering xsend\n");
write(fe,ss,kk);
#endif DEBUG

  state = 6;                                 /* state for file send */
  retcode = (int)NULL;                            /* normal retcode value */
 
  if ((fs=open(sndfile,O_RDONLY))==ERROR)
    {
    message(22);                             /* can't open send file */
    return;                                  /* return to cmd */
    }

  message(24);                               /* "ready to send" message */
 
  /* Set 8-bit length and cancel xon/xoff and parity
     (possibly at variance with menu settings) */
  initmod(lpath,OFF,ON,0,IXON+IXOFF,CS8,PARENB);

  i = 0;
  do
    {
    i++;
    READCHAR(10);                            /* wait 10 seconds on each read */
    firstchar=c;      
#ifdef DEBUG
kk = sprintf(ss,"Waiting for NAK.  Got %o\n",c);
write(fe,ss,kk);
#endif DEBUG
    }
  while (firstchar != NAK && i < ERRORMAX && retcode == (int)NULL);
  (void) tcflush(lpath,TCIFLUSH);           /* flush in case NAK's stacked up */
                                             /*   due to slow user start */
  CKRET                                      /* check for user interrupt */
   
  if (firstchar != NAK) 
    {
    message(35);                             /* remote side not ready */
    initmod(lpath,OFF,ON,0,0,0,0);           /* restore user's settings */ 
    close(fs);                               /* close send file */
    return;                                  /* return to cmd */
  }

  sectnum = 1;                               /* initialize sector number */
  do
    {                                        /* read and write out data a block at a time */
    if ((i=read(fs,xbufr,SECSIZ))==ERROR)
      {
      if (retcode==(int)NULL) message(36);        /* can't read file */
      close(fs);                             /* close send file */
      retcode=(int)NULL;
      initmod(lpath,OFF,ON,0,0,0,0);         /* restore port settings */
      return;                                /* return to cmd */
      }
    if (i==0) break;                         /* end of file */
    
    if (i < SECSIZ)                          /* not a full sector at end */
      for (j=i; j<SECSIZ; j++) xbufr[j]=SUB; /* pad with ctrl-Z */

    checksum = 0;
    p = xbufr;                                /* set pointer to start of data */
    while (p < xbufr+SECSIZ)
      {
      checksum += *p;                        /* calculate checksum */
      p++;
      }

    attempt = 0;
    do                                      /* send block repeatedly until */
      {                                     /* ACK received or limit reached. */
      message (25);                         /* sending block */
      SENDCHAR(SOH);                        /* send SOH */
      SENDCHAR(sectnum & CHARMASK);         /* send sector number */
      SENDCHAR(~sectnum & CHARMASK);        /* send complement of sector number */
      write (lpath,xbufr,SECSIZ);            /* send 128 bytes of data */
      (void) tcflush(lpath,TCIFLUSH);	    /* flush input queue or bad ACK's */
      SENDCHAR(checksum & CHARMASK);        /* send checksum */
      CKRET                                 /* check for user interrupt */
      attempt++;                            /* increment attempts counter */
      READCHAR(10);                         /* read for ACK or NAK */
      }
    while (c != ACK && attempt < ERRORMAX && retcode == (int)NULL);

    sectnum++;                              /* increment sector number */

    }
  while (attempt < ERRORMAX && retcode == (int)NULL);

  CKRET                                     /* check for user interrupt */
   
  if (attempt >= ERRORMAX)
    {
    initmod(lpath,OFF,ON,0,0,0,0);          /* restore user's settings */
    close(fs);                              /* close send file */
    message(37);                            /* no acknowledgement of sector */
    }
  else                                      /* we're at the end of the file */
    {
    attempt = 0;
    do                                      /* send EOT repeatedly until ACK */
      {                                     /* received or limit reached. */
      SENDCHAR(EOT);
      attempt++;
      READCHAR(10);                         /* read for ACK or NAK */
      }
    while (c != ACK && attempt < ERRORMAX && retcode == (int)NULL);
    (void) tcflush(lpath,TCIFLUSH);	    /* flush input Q */

    CKRET                                   /* check for user interrupt */
 
    initmod(lpath,OFF,ON,0,0,0,0);          /* restore user's settings */
    close(fs);                              /* close send file */
   
    if(attempt == ERRORMAX) 
      message(38);                          /* no acknowledgement of EOT */
    else  message(15);                      /* end of transmission */
    }
 
#ifdef DEBUG
kk = sprintf(ss,"leaving xsend\n");
write(fe,ss,kk);
#endif DEBUG
}
