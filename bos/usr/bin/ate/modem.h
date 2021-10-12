/* @(#)17	1.9  src/bos/usr/bin/ate/modem.h, cmdate, bos411, 9437A411a 8/26/94 16:04:16 */
/* 
 * COMPONENT_NAME: BOS modem.h
 * 
 * FUNCTIONS: 
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



/* -------------------------
   Modem Program header file
   ------------------------- */

#include <stdio.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>

#define ON         1
#define OFF        0
#define ERROR    (-1)

#define EMPTY "                                        "

/* control keys */
#define ESC      0x1B           /* Escape for VT100 emulation sequence */
#define LF       0x0A		/* line feed = newline */

/* module return codes */
/*      NULL     0x00              defined elsewhere */
#define QUIT     0x04           /* quit */
#define EXIT     0x05           /* exit */

#define CKRET if (retcode != (int)NULL) {retcode = (int)NULL; return;}

/* Command control */
#define ARGS  "%40s %40s %40s %40s %40s %40s %40s %40s %40s %40s %40s %40s\
%40s %40s %40s %40s %40s %40s %40s %40s %40s %40s %40s %40s %40s %40s",\
argp[0],argp[1],argp[2],argp[3],argp[4],argp[5],\
argp[6],argp[7],argp[8],argp[9],argp[10],argp[11],argp[12],argp[13],argp[14],\
argp[15],argp[16],argp[17],argp[18],argp[19],argp[20],argp[21],argp[22],\
argp[23],argp[24],argp[25]

char userin[81];      /* read arguments in */
char *argp[26];       /* argument pointers */
char arg0[41],arg1[41],arg2[41],arg3[41],arg4[41],arg5[41],arg6[41],
     arg7[41],arg8[41],arg9[41],arg10[41],arg11[41],arg12[41],arg13[41],
     arg14[41],arg15[41],arg16[41],arg17[41],arg18[41],arg19[41],
     arg20[41],arg21[41],arg22[41],arg23[41],arg24[41],arg25[41];
int argcnt;           /* number of arguments passed to funcs */

extern int errno;
struct termios option;          /* input,output,cntl,local modes */
char disp_buff[TTNAMEMAX];	/* buffer to store shell termio discipline */

/* common variables and value flags */
int mb_cur_max;			/* max amount of bytes for a char in locale */
int speed;                      /* baud rate */
int bits;                       /* bits per character */
int stop;                       /* number of stop bits */
int parity;                     /* none, odd or even parity */
int redial;                     /* redialing interval */
int attempts;                   /* number of redialing attempts */
int locfile;                    /* lockfile created (true or false) */
int connect;                    /* connection made (true or false) */
int state;                      /* program state */
int keyappl;                    /* bit0 keypad appl  ON |0x0001   OFF &0xFFFE */
				/* bit1 cursor appl  ON |0x0002   OFF &0xFFFD */
                                /* bit2 vt52 mode    ON |0x0004   OFF &0xFFFB */
int xmit;                       /* file transfer in progress */
                                /* xmit is not currently being checked by any */
                                /* modules */
int retcode;                    /* return code */
int menu;			/* menu being displayed (true or false) */
int sectnum;                    /* sector number from xmodem */
int Echo;                       /* echo characters */
int xctl;                       /* xon/xoff control */
int capture;                    /* data capture */
int xlatcr;                     /* translate CR to CR-LF */
int vt100;                      /* VT100 emulation */
int awrap;			/* VT100 autowrap */

char capkey;                     /* ^b, ascii VT  - capture toggle */
char cmdkey;                     /* ^v, ascii SYN - command menu */
char retkey;                     /* ^r, ascii DC2 - return one levels */

char kbdata;                    /* characters from the keyboard */
char moddata;                   /* characters from the port (modem) */
char transfer;                  /* file transfer method */
char pacing;                    /* pacing character or time */
char dialp[41];                 /* modem prefix, such as "AT DT" */
char dials[41];                 /* modem suffix */
char number[41];                /* telephone number to dial */
char machine[_SYS_NMLN];        /* system identification */
int termrc;			/* return code from setupterm */

/* file names and descriptors */
int lpath;                      /* primary port fildes */
int lpath2;                     /* secondary port fildes, used in digits() */
char port[41];                  /* port, such as tty0 */
int df;                         /* default file filedes */
int fk;                         /* capture filedes */
char kapfile[41];               /* name of capture file (default=kapture) */
FILE *fd;                       /* directory filedes */
char dirfile[41];               /* name of directory file (default=dir) */
int temp;                       /* temporary shared filedes */
char tempfil[41];               /* name of screen refresh file */
                                /* tempfil is not currently being used */  
                                /* anywhere */
int fs;                         /* send filedes */
char sndfile[41];               /* name of send file */
int fr;                         /* receive filedes */
char rcvfile[41];               /* name of receive file */

int fe;                         /* file descriptor for debug file */
int kk;
char ss[512];

#define LNSZ	4096		/* size of line data buffer */
char lndata[LNSZ+1];		/* line data from the port */
int lnstat;			/* return status from port read */
char *ptr;			/* position in array of incoming data */

#define BUFSZ  16384            /* capture buffer size */
unsigned int txti;              /* position in capture buffer */
char bufr[BUFSZ];               /* capture buffer */

char *sbuf;			/* screen buffer memory pointer */
