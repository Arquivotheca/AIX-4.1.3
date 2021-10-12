static char sccsid[] = "@(#)26	1.3  src/bos/usr/bin/ate/shared.c, cmdate, bos411, 9428A410j 4/18/91 10:58:32";
/* 
 * COMPONENT_NAME: BOS shared.c
 * 
 * FUNCTIONS: gettemp, readflags, writeflags 
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
/* Module name:      shared.c                                               */
/* Description:      create temporary file; write flag values into the file */
/*                     and read flag values from the file.                  */
/* Functions:        gettemp - get a temporary file to use during the       */
/*                     connection                                           */
/*                   writeflags - write values into the file                */
/*                   readflags - read values from the file                  */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      cmd in command.c  (writeflags)                         */
/*                   conn in connect.c (gettemp, readflags, writeflags)     */
/*                   catch, vt2, main in portrw.c (readflags, writeflags)   */
/*                   sigrout in signal.c (readflags, writeflags)            */
/*                   vt1 in vt100.c (writeflags)                            */
/*   Receives:       nothing                                                */
/*   Returns:        gettemp returns 0 or -1, others return nothing         */
/*   Abnormal exits: none                                                   */
/*   Calls:          message in message.c (print user errors)               */
/*   Modifies:       nothing                                                */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>

#define DEBUG 0
#define ON         1
#define OFF        0
#define ERR      (-1)
#define S3        81

/* common variables and value flags */
int menu;                       /* menu being written */
int keyappl;                    /* bit0 keypad appl, bit1 cursor appl ON/OFF */
int xmit;                       /* file transfer in progress */
int capture;                    /* data capture */
int xlatcr;                     /* translate CR to CR-LF */
int vt100;                      /* VT100 emulation */

int lpath;                      /* port fildes */
char kapfile[S3];               /* name of capture file (default=kapture) */
int temp;                       /* temporary shared filedes */
char tempfil[S3];               /* name of screen refresh file */
 
extern int fe;                  /* file descriptor for debug file */
int kk;
char ss[512];

/* ------------------------------------------------------------------------
   get temporary shared file - this file is created in /tmp for passing
   values between the parent and child processes.  The function returns 0
   if successful creation, and -1 if failure.  Shared memory could just as
   easily be used for this.  When this routine was written, the shared memory
   routines only lived on the models, and model time was scarce.  This
   routine could be written and tested on the pc.
   ------------------------------------------------------------------------ */
gettemp()
{
 int i;
 
 tmpnam(tempfil);                            /* create a unique temporary file name */
 if ((temp = open(tempfil,O_RDWR|O_CREAT))==ERR)
   {
   message(04);                              /* can't open/create temp file */
   return(-1);
   }
 fcntl(temp,F_SETFD,0);                      /* set temp to stay open across exec */
 for (i=0; i<129; i++) write(temp," ",1);    /* write blanks into it */
 return(0);
}


/* --------------------------------------------------------------------------
   write flags - write values into temporary shared file.
   -------------------------------------------------------------------------- */
writeflags()
{
 char s[128];
 
#if DEBUG
kk = sprintf(ss,"entering writeflags\n");
write(fe,ss,kk);
#endif DEBUG

 lseek(temp,0L,0);                           /* position pointer to beginning of file */
 
 sprintf(s,"%d %d %d %d %d %d %d %s",
   capture,keyappl,menu,vt100,xlatcr,xmit,   /* copy flags and capture file */
   lpath,kapfile);                           /* name into string s */
 write(temp,s,sizeof(s));                    /* write flags to temporary file */
  
#if DEBUG
kk = sprintf(ss,"cap kpad menu vt100 LF xmit lpath kapfile\n");
write(fe,ss,kk);
kk = sprintf(ss,"%3d %5d %5d %6d %3d %5d %6d %s\n",
   capture,keyappl,menu,vt100,xlatcr,xmit,lpath,kapfile);
write(fe,ss,kk);
#endif DEBUG

}


/* ------------------------------------------------------------------------
   read flags - read values from temporary shared file
   ------------------------------------------------------------------------ */
readflags()
{
 char s[128];

#if DEBUG
kk = sprintf(ss,"entering readflags\n");
write(fe,ss,kk);
#endif DEBUG

 lseek(temp,0L,0);                           /* move pointer to beginning of file */

 read(temp,s,sizeof(s));
 
 sscanf(s,"%d %d %d %d %d %d %d %40s",       /* read flags and capture file name */
   &capture,&keyappl,&menu,&vt100,&xlatcr,
   &xmit,&lpath,kapfile);

#if DEBUG
kk = sprintf(ss,"cap kpad menu vt100 LF xmit lpath kapfile\n");
write(fe,ss,kk);
kk = sprintf(ss,"%3d %5d %5d %6d %3d %5d %6d %s\n",
   capture,keyappl,menu,vt100,xlatcr,xmit,lpath,kapfile);
write(fe,ss,kk);
#endif DEBUG

}
