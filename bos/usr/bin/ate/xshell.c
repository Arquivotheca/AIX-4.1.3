static char sccsid[] = "@(#)32        1.15  src/bos/usr/bin/ate/xshell.c, cmdate, bos411, 9428A410j 11/11/93 10:06:35";
/* 
 * COMPONENT_NAME: BOS xshell.c
 * 
 * FUNCTIONS: MSGSTR, Mxshell, READCHAR, SENDCHAR, catch, message, 
 *            setopt, xpass, xrcv, xsend 
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
#include "xshell_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_XSHELL,n,s) 

#include "cpyrght.h"

/****************************************************************************/
/*                                                                          */
/* Module name:      xshell.c                                               */
/* Description:      transmit a file using xmodem protocol                  */
/*                   This is a self-contained program which produces the    */
/*                   xmodem shell command shipped with ate.  The xrcv and   */
/*                   xsend receives are virtually identical to the ones     */
/*                   in xmodem.c, except no messages are ever printed.      */
/*                   A separate message table and message routine is used   */
/*                   in this program to try to minimize the size of the     */
/*                   code.  The code was optimized, so some things are in-  */
/*                   line which would otherwise be separate functions.      */
/*                                                                          */
/* Functions:        catch - trap alarm clock, quit and interrupt signals   */
/*                   setopt - set terminal parameters for transfer          */
/*                   main - xshell driver                                   */
/*                   xrcv - receive a file using xmodem protocol            */
/*                   xsend - send a file using xmodem protocol              */
/*                   xpass - pass a file from stdin to port                 */
/*                   message - print user msgs                              */
/*                                                                          */
/* Status:           Version 2, Release 1, Level 0                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c                                       */
/*   Receives:       nothing                                                */
/*   Returns:        nothing                                                */
/*   Abnormal exits: can't open receive/send file                           */
/*                   user cancellation                                      */
/*                   other side doesn't respond                             */
/*                   can't write to receive or read send file               */
/*   Calls:          nothing                                                */
/*   Modifies:       nothing                                                */
/*                                                                          */
/****************************************************************************/


#include <stdio.h>
#include <ctype.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>

#define ON       1
#define OFF      0
#define ERROR    (-1)
#define BUFSZ    SECSIZ+3                     /* text buffer size */
#define SECSIZ   128                          /* block size used by xmodem */
#define PBUFSIZ  (SECSIZ+3)                   /* size of xmodem packet less SOH */
#define CHARMASK 0xFF                         /* character mask */
#define ERRORMAX 9                            /* max errors per sector */

/* file transfer special characters */
#define TIMEOUT (-1)                          /* timeout character */
#define SOH    0x01                           /* start of block character */
#define EOT    0x04                           /* end of transmission */
#define ACK    0x06                           /* acknowledge block */
#define NAK    0x15                           /* error in transmission */
#define CAN    0x18                           /* ^x, cancel key */
#define SUB    0x1a                           /* end of file (a la CP/M,DOS) */
#define NUL    '\0'                           /* null pad character */

/* these two macros are provided for
   the purpose of optimizing the code */
#define READCHAR(n)   alarm(n); lnstat=read(0,&ch,1); alarm(0); \
                      if (lnstat==ERROR) c=(-1); else c=(ch & CHARMASK) 

#define SENDCHAR(x)   ch=x; write(1,&ch,1);

extern int errno;
char bufr[BUFSZ];                             /* text buffer */
char passbuf[PBUFSIZ];                        /* pass-through buffer */
int fd;                                       /* filedes of send/receive file */
char file[81];                                /* name of send file */
struct termios option;                    /* parameters when entered setopt */
struct termios initopt;                   /* parameters when entered program */
int sectnum;                                  /* sector number */

char *msgvc1;

#ifdef DEBUG
int fe;
unsigned int kk;
char ss[512];
#endif DEBUG

                     
/* --------------------------------------------------------------------------
   catch signals
   -------------------------------------------------------------------------- */
void
catch(int sig)
{
 switch(sig)
   {
   case SIGALRM : signal(SIGALRM,(void(*)(int))catch);      /* reset the trap */
                  alarm(0);                   /* turn off the alarm */
                  break;
   case SIGTERM :
   case SIGHUP  :
   case SIGQUIT :
   case SIGINT  : tcsetattr(0,TCSANOW,&initopt); /* restore term settings */
                  close(fd);                  /* close file */
                  exit(1);                    /* exit shell command */
                  break;
  default       : break;
  }
}


/* --------------------------------------------------------------------------
   setopt - set terminal parameters for transfer.  This routine is passed
   either 0 (receive and passthru), 1 (send) or 2 (messages) as the file
   descriptor.
   -------------------------------------------------------------------------- */
setopt(fildes)
int fildes;                                   /* file descriptor */
{
struct termios to;

#ifdef DEBUG
kk = sprintf(ss,"entering Setopt\n");
write(fe,ss,kk);
#endif DEBUG

 if (tcgetattr(fildes,&option) < 0) {        /* get present terminal settings */
    perror("tcgetattr");
    exit(1);
 }

 bcopy(&option, &to, sizeof(to));
 to.c_iflag  = IGNPAR;                        /* parity off */
 to.c_iflag |= BRKINT;                        /* signal interrupt on break */
 to.c_oflag  = 0;                             /* clear output flag */
 to.c_cflag |= CREAD;                         /* enable character receiver */
 to.c_cflag |= CS8;                           /* 8 bits */
 to.c_cflag &= ~PARENB;                       /* no parity */
 to.c_lflag  = 0;                             /* clear line flag */
 to.c_cc[VINTR] = -1;                         /* turn off interrupt signal */
 to.c_cc[VQUIT] = -1;                         /* turn off quit signal */
 to.c_cc[VERASE] = -1;                        /* turn off erase */
 to.c_cc[VKILL] = -1;                         /* turn off kill */
 to.c_cc[VMIN] = 1;                           /* min chars to satisfy a read */
 to.c_cc[VTIME] = 0;                 /* timeout (1/10 sec) to satisfy a read */

 if (tcsetattr(fildes,TCSANOW,&to) < 0) {     /* set new terminal settings */
    perror("tcsetattr");
    exit(1);
 }
 
#ifdef DEBUG
kk = sprintf(ss,"Leaving setopt with\n");
write(fe,ss,kk);
kk = sprintf(ss,"to.c_iflag (input)  = %o\n",to.c_iflag);
write(fe,ss,kk);
kk = sprintf(ss,"to.c_oflag (output) = %o\n",to.c_oflag);
write(fe,ss,kk);
kk = sprintf(ss,"to.c_cflag (hrdwr)  = %o\n",to.c_cflag);
write(fe,ss,kk);
kk = sprintf(ss,"to.c_lflag (line)   = %o\n",to.c_lflag);
write(fe,ss,kk);
kk = sprintf(ss,"to.c_cc    (cntl)   = %o %o %o %o %o %o\n",
  to.c_cc[0],to.c_cc[1],to.c_cc[2],to.c_cc[3],to.c_cc[4],to.c_cc[5]);
write(fe,ss,kk);
#endif DEBUG

}


/* --------------------------------------------------------------------------
   xshell program - driver for xmodem shell command.  This routine is passed
   flags and filenames:
      -r filename (receive)
      -s filename (send)
      -p          (passthrough)
   -------------------------------------------------------------------------- */
main(argc,argv)
int argc;
char *argv[];

{
int  ret = 0;

	(void)setlocale(LC_ALL,"");		/* for setlocale() */
	catd = catopen(MF_XSHELL,NL_CAT_LOCALE);

 tcgetattr(0,&initopt);                      /* get present terminal settings */

signal(SIGALRM,(void(*)(int)) catch);                        /* alarm clock */
signal(SIGINT ,(void(*)(int)) catch);                        /* interrupt */
signal(SIGQUIT,(void(*)(int)) catch);                        /* quit signal */
signal(SIGHUP ,(void(*)(int)) catch);                        /* hangup signal */
signal(SIGTERM ,(void(*)(int)) catch);                        /* hangup signal */

#ifdef DEBUG
if ((fe=open("ERRORS",O_WRONLY|O_TRUNC|O_CREAT,0766))==ERROR)
  {
  kk = sprintf(ss,"Can't open/create ERRORS debugging file.\n");
  fe=1;
  write(fe,ss,kk);
  }
kk = sprintf(ss,"entering main\n");
write(fe,ss,kk);
#endif DEBUG
  
fd = ERROR;                                   /* file descriptor */

if (strcmp(argv[1],"-r") == 0)                /* -r (receive) flag */
  {
  if (argc < 3) 
    {
    message(18);                  /* no receive file name */
    ret = 1;
    }
  else
    {
    strcpy(file,argv[2]);                     /* copy file name into file */
    setopt(0);                                /* set terminal parameters */
    if (xrcv() < 0)                          /* call xmodem receive routine */
      ret = 1;
    tcsetattr(0,TCSANOW,&option);            /* reset stdin terminal settings */
    }
  }
else if (strcmp(argv[1],"-s") == 0)           /* -s (send) flag */
  {
  if (argc < 3) 
    {
    message(23);                  /* no send file name */
    ret = 1;
    }
  else
    {
    strcpy(file,argv[2]);                     /* copy file name into file */
    setopt(1);                                /* set terminal parameters */
    if (xsend() < 0)                          /* call xmodem send routine */
      ret = 1;
    tcsetattr(1,TCSANOW,&option);            /* reset stdin terminal settings */
    }
  }
else if (strcmp(argv[1],"-p") == 0)           /* -p (passthru) flag */
  {
  setopt(0);                                  /* set terminal paramters */
  if (xpass() < 0)                            /* pass through (ate to remote-remote) */
    ret = 1;
  tcsetattr(0,TCSANOW,&option);              /* reset stdin terminal settings */
  }
else 
  {
  message(39);                             /* invalid flags */
  ret = 1;
  }

sleep(1);                                     /* give other side a moment to
                                                   recondition the line */
close(fd);                                    /* close file */
exit(ret);
}


/* ---------------------------------------------------------------------------
   xmodem receive routine
   --------------------------------------------------------------------------- */
xrcv()
{
 int firstchar;                             /* first character read */
 int sectcurr;                              /* sector number sent */
 int sectcomp;                              /* ones complement of sectcurr */
 int cksum;                                 /* checksum transmitted by remote */
 int checksum;                              /* checksum calculated */
 int errors=0;                              /* cumulative errors for sector */
 int i,j;
 register char *p;                          /* pointer to character */
 int c,lnstat;                              /* character, line status */
 char ch;                                   /* character */
 int ret = 0;				    /* return value */
 
#ifdef DEBUG
kk = sprintf(ss,"entering xrcv()\n");
write(fe,ss,kk);
#endif DEBUG
 
 if ((fd=open(file, O_WRONLY|O_APPEND|O_CREAT, 0766))==ERROR)
   {
   message(17);                              /* can't open file */
   return(-1);                               /* return to driver */
   }

 message(05);                                /* ready to receive message */

 sectnum=1;                                  /* initialize sector number */
 
 SENDCHAR(NAK);                              /* send ready signal to transmit end */
 do
   {

   do                                        /* wait for SOH, EOT or TIMEOUT */
     {
     READCHAR(10);                           /* wait 10 seconds for a character */
     firstchar=c; 
     if (firstchar==CAN) catch(SIGINT);      /* user cancelled run */
     }
   while (firstchar!=SOH && firstchar!=TIMEOUT && firstchar!=EOT);
 
   /* TIMED OUT */
   if(firstchar==TIMEOUT)                     /* nothing received from remote side */
     {
     errors++;                                /* increment error count */
     SENDCHAR(NAK);                           /* send negative acknowledgement */
     }
 
   /* GOT A START OF HEADER */
   if(firstchar==SOH)                          /* got a start of header */
     {
     i = 0;                                    /* keep reading until an entire */
     do                                        /* buffer is received--sector no., */
       {                                       /* complement of sector no., 128 */
       j = read(0,&bufr[i],BUFSZ-i);           /* of data, and check sum */
       i = i+j;
       }
     while (i < BUFSZ);

#ifdef DEBUG
write(fe,"\n",1);
write(fe,bufr,BUFSZ);
write(fe,"\n",1);
#endif DEBUG

     sectcurr=bufr[0] & CHARMASK;               /* sector number received */
     sectcomp=bufr[1] & CHARMASK;               /* sector complement received */
     cksum=bufr[BUFSZ-1] & CHARMASK;            /* checksum received */
     
     if ((sectcurr+sectcomp)!=CHARMASK)         /* bad complement */
       {
       errors++;                                /* increment error count */
       c=0;
       while(c != TIMEOUT) {READCHAR(1);}       /* wait for silence */
       SENDCHAR(NAK);                           /* send negative acknowledgement */
       }

     else if ((sectcurr < sectnum) ||           /* duplicate sector received */
       (sectnum == 1 && sectcurr == 0377))      /* check boundary too */
       {
       errors++;                                /* increment error count */
       SENDCHAR(ACK);			        /* send acknowledgement */
       }

     else if (sectcurr > sectnum)               /* sector greater than expected */
       {
       errors++;                                /* increment error count */
       c=0;
       while(c != TIMEOUT) {READCHAR(1);}       /* wait for silence */
       SENDCHAR(NAK);                           /* send negative acknowledgement */
       }

     else
       {
       checksum=0;                              /* clear checksum */
       p = &bufr[2];                            /* set pointer to beginning of data */
       while (p < &bufr[2]+SECSIZ)
         {
         *p = *p & CHARMASK;                    /* mask each character */
         checksum = (checksum + *p);            /* calculate checksum */
         p++;
         }
       checksum &= CHARMASK;                    /* mask checksum */
       
       if (checksum != cksum)                   /* checksums don't match */
         {
         errors++;                              /* increment error count */
         c=0;
         while(c != TIMEOUT) {READCHAR(1);}     /* wait for silence */
         SENDCHAR(NAK);                         /* send negative acknowledgement */
         }
       else 
         {
         errors=0;                               /* clear error count */
         sectnum = (sectnum+1) & CHARMASK;       /* increment sector number */
         if (write(fd,&bufr[2],SECSIZ)==ERROR)   /* write data to file */
           {
           close(fd);                            /* couldn't write to file */
           return(-1);                           /* return to driver */
           }
         SENDCHAR(ACK);                          /* acknowledge sector */
         } 
       }                                         /* end else sector # ok */
     }                                           /* end SOH processing */
   }      
 while (firstchar!=EOT && errors<ERRORMAX);
 
 /* GOT AN EOT */
 if (firstchar==EOT && errors<ERRORMAX)
   {
   SENDCHAR(ACK);                                /* acknowledge EOT */
   }
  else
    ret = -1;
 
 close(fd);                                      /* close receive file */

#ifdef DEBUG
kk = sprintf(ss,"leaving xrcv\n");
write(fe,ss,kk);
#endif DEBUG

  return(ret);

}


/* --------------------------------------------------------------------------
   Xmodem send routine
   -------------------------------------------------------------------------- */
xsend()
{
  int attempt=0, firstchar;
  int checksum;                                  /* calculated check sum */
  int i,j,ret;
  register char *p;                              /* pointer to character */
  int c,lnstat;                                  /* character, line status */
  char ch;                                       /* character */
 
#ifdef DEBUG
kk = sprintf(ss,"entering xsend\n");
write(fe,ss,kk);
#endif DEBUG

  if ((fd=open(file,O_RDONLY))==ERROR)           /* open send file */
    {
    message(22);                                 /* can't open send file */
    return(-1);                                  /* return to driver */
    }

  message(16);                                   /* "ready to send" message */
 
  i = 0;
  do
    {
#ifdef DEBUG
kk = sprintf(ss,"Waiting for NAK\n");
write(fe,ss,kk);
#endif DEBUG
    i++;
    READCHAR(10);                                /* wait 10 seconds on each read */
    firstchar=c;
    if (firstchar == CAN) return(-1);            /* user cancelled run */
    }
  while (firstchar != NAK && i < ERRORMAX);
  (void) tcflush(0,TCIFLUSH);             /* flush input buffer in case NAK's */
                                                 /* stacked up due to slow user start */
                                                 
  if (firstchar != NAK) return(-1);              /* receive side not ready */

  sectnum = 1;                                   /* initialize sector number */
  do                                             /* read and write out data a */
    {                                            /* block at a time */
    if ((i=read(fd,bufr,SECSIZ))==ERROR) return(-1);   /* can't read send file */
    if (i==0) break;                             /* end of file */
    if (i < SECSIZ)                              /* last sector not 128 characters */
      for (j=i; j<SECSIZ; j++) bufr[j]=SUB;      /* pad with ctrl-Z */
    checksum = 0;                                /* clear checksum */
    p = bufr;                                    /* set pointer to start of data */
    while (p < bufr+SECSIZ)
      { checksum += *p; p++; }                   /* calculate checksum */
      
    attempt = 0;

    do                                           /* Send block repeatedly until */
      {                                          /* ACK received or limit reached. */
      SENDCHAR(SOH);                             /* send start of header */
      SENDCHAR(sectnum & CHARMASK);              /* send sector number */
      SENDCHAR(~sectnum & CHARMASK);             /* send sector complement */
      write (1,bufr,SECSIZ);                     /* send data */
      (void) tcflush(0,TCIFLUSH);		/* flush input queue */
      SENDCHAR(checksum & CHARMASK);             /* send checksum */
      attempt++;
      READCHAR(10);                              /* read for ACK or NAK */
      }
    while (c != ACK && c != CAN && attempt < ERRORMAX);

    if (c==CAN) return(-1);                      /* user cancelled run */
    sectnum = (sectnum+1) & CHARMASK;            /* increment sector number */

    }
  while (attempt < ERRORMAX);

  if (attempt >= ERRORMAX) ;			/* Do nothing */
  else                                           /* we're at the end of the file */
    {
    attempt = 0;
    do                                           /* Send EOT repeatedly until ACK */
      {                                          /* received or limit reached. */
      SENDCHAR(EOT);                             /* send end of transmission */
      attempt++;
      READCHAR(10);                              /* read for ACK or NAK */
      }
    while (c != ACK && c != CAN && attempt < ERRORMAX);
  }
 
#ifdef DEBUG
kk = sprintf(ss,"leaving xsend\n");
write(fe,ss,kk);
#endif DEBUG

  return ((attempt >= ERRORMAX) ? -1: 0);
}


/* --------------------------------------------------------------------------
   xmodem pass-through routine.  This routine is used in a once-away connection
   to transfer a file from the sailboat to the remote location using xmodem.
   It is only required in this direction because the intermediate ate is looking
   for signals (^b, ^v) whose values could occur in a binary file.  Turning off
   the signals during pass-thru was rejected because it would make it impossible
   for the user to interrupt the routine.
   
   This routine reads from its stdin and writes to the port passed it via the
   ateport environmental variable.  'ateport' is set everytime the perform
   command in cmd (command.c) is used.  It returns -1 if it can't open the
   port.
   -------------------------------------------------------------------------- */
xpass()
{
  int fc = NUL;                                  /* integer first char */
  int rfd, maxlen, len;                          /* remote fildes */
  char cfc;                                      /* character first char */
  int c, lnstat;                                 /* character, line status */
  char ch;                                       /* character */

  if ((rfd = open((char*)getenv("ateport"),O_RDWR))<0)  /* open remote tty (xmodem) port */
    return -1;                                   /* open failed */

  for (;;)                                       /* loop on full packets (we hope) */
    {
    /* receiver should send NAK (ready to receive) */
    alarm (15);                                  /* wait 15 seconds for NAK */
    len = read(rfd, &cfc, 1);                    /* read for NAK from port */
    alarm (0);                                   /* turn off alarm */
    if (len == 1)  write (1, &cfc, 1);           /* write NAK to sailboat */
  
#ifdef DEBUG
if(len==1)
  {
  write(fe,"\nBack-channel `",15);
  write(fe,&cfc,1);
  write(fe,"'\n",2);
  }
#endif DEBUG

    /* now proceed with sender's output */
    READCHAR(10);                                /* read SOH from stdin */
    if (c == TIMEOUT) continue;                  /* goto end of loop & try again */
    cfc = fc = c;
    write (rfd, &cfc, 1);                        /* write SOH to port */

#ifdef DEBUG
write(fe,"Forward channel:\n",17);
write(fe,&cfc,1);
#endif DEBUG

    if (fc == EOT || fc == CAN) break;           /* end of transmission or user cancellation */
    if (fc == SOH)                               /* rest of packet follows */
      {
      maxlen = PBUFSIZ;                          /* pass-thru buffer size */
      while (maxlen > 0)                         /* transfer stdin to port in */
        {                                        /* available chunks */
        alarm (10);
        len = read(0, passbuf, maxlen);          /* read from stdin */
        alarm (0);
        if (len < 0)                             /* no chars for 10 seconds */
          break;                                 /* leave this while loop */
        write (rfd, passbuf, len);               /* write to port */
#ifdef DEBUG
write(fe,passbuf,len);
#endif DEBUG
        maxlen -= len;                           /* decrement # left to read */
        }

      if (len < 0)                               /* data probably lost */
        continue;                                /* await retransmission */
      }
    }                                            /* end of for loop */
    
  if (fc == EOT)                                 /* propagate final acknowledgement */
    {
    alarm (5);
    while ((len = read(rfd, &cfc, 1)) >= 0)      /* read ACK from port */
      if (len > 0) write (1, &cfc, len);         /* write to sailboat */
    alarm (0);
    return(0);
    }
   else
    return(-1);
}      
    


/* --------------------------------------------------------------------------
   messages.  This routine is a duplicate of the routine in message.c and
   portrw.c.  It is included here with only those messages printed from the
   shell command to make the module smaller.  The messages contained in
   all three message tables (atemsg.h, portmsg.h and xshellmsg.h) should be
   identical.
   -------------------------------------------------------------------------- */
message(num)
int num;
{

#ifdef DEBUG
kk = sprintf(ss,"Entering xshell message with number = %d\n",num);
write(fe,ss,kk);
#endif DEBUG

 tcsetattr (2, TCSANOW, &initopt);/* restore terminal mode for message output */

 switch (num)
   {
    case  5 : msgvc1 = file;                     /* msg variable (char) 1 = file name */
			  printf(MSGSTR(MSG5, "ate: 0828-005 The system is ready to receive file %s.\r\n\
              Use Ctrl-X to stop xmodem.\r\n"), msgvc1); /*MSG*/
              setopt(2);                         /* set terminal parameters for connection */
              break;

    case 16 : msgvc1 = file;
			  printf(MSGSTR(MSG16, "ate: 0828-016 The system is ready to send file %s.\r\n\
              Use Ctrl-X to stop xmodem.\r\n"), msgvc1); /*MSG*/
              setopt(2);
              break;
              
    case 17 : msgvc1 = file;
			  printf(MSGSTR(MSG17, "ate: 0828-017 The Receive command cannot create or open the\r\n\
              %s file\r\n\
              because of the error shown below.\r\n"), msgvc1); /*MSG*/
              perror(MSGSTR(RECV, "Receive"));                /* print system message */ /*MSG*/
              break;
              
    case 18 : printf(MSGSTR(MSG18, "ate: 0828-018 The Receive command cannot complete because you did not\r\n\
              specify an input file name.\r\n")); /*MSG*/
              break;
              
    case 22 : msgvc1 = file;
			  printf(MSGSTR(MSG22, "ate: 0828-022 The Send command cannot open the\r\n\
              %s file\r\n\
              because of the error shown below.\r\n"), msgvc1); /*MSG*/
              perror(MSGSTR(SEND, "Send")); /*MSG*/
              break;
              
    case 23 : printf(MSGSTR(MSG23, "ate: 0828-023 The Send command cannot complete because you did not\r\n\
              specify an output file name.\r\n")); /*MSG*/
              break;
              
    case 39 : printf(MSGSTR(MSG39, "ate: 0828-039 The xmodem command cannot complete because a flag is not valid.\r\n\
              Valid flags are -r for receive, -s for send, or -p for pass-\r\n\
              through.  Enter the command again using one of these flags.\r\n"));
              break;
             
    default : printf(MSGSTR(MSGUNK, "Message number %d is not in messages.c\r\n"),num); /*MSG*/
              break;
   }

#ifdef DEBUG
kk = sprintf(ss,"Leaving xshell message\n");
write(fe,ss,kk);
#endif DEBUG

}

