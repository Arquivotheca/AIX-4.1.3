/* @(#)93       1.21  src/bos/kernel/sys/termio.h, cmdtty, bos411, 9428A410j 6/4/91 08:47:35 */

/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TERMIO
#define _H_TERMIO
#include <standards.h>

#include <sys/ioctl.h>
#include <sys/ttmap.h>

/* termios defines all the bits, baudrates, etc. */
#include <termios.h>

#define CBELL   '\007'                  /* ASCII BELL control */
#define CDEL    '\177'                  /* delete */
#define CEOF    '\004'                  /* ^D */
#define CERASE  '\010'                  /* ^H */
#define CESC    '\\'                    /* back-slash */
#define CFORM   '\014'                  /* ^L */
#define CINTR   '\003'                  /* ^C */
#define CKILL   '\025'                  /* ^U */
#define CNUL    '\0'                    /* null char */
#define CQUIT   '\034'                  /* ^\ */
#define CSTART  '\021'                  /* ^Q */
#define CSTOP   '\023'                  /* ^S */
#define CVT     '\013'                  /* ^K */

/*
 * Ioctl control packet
 */

#define NCC 8

struct termio {
        unsigned short c_iflag;         /* input modes */
        unsigned short c_oflag;         /* output modes */
        unsigned short c_cflag;         /* control modes */
        unsigned short c_lflag;         /* line discipline modes */
        char c_line;                    /* line discipline */
        unsigned char c_cc[NCC];        /* control chars */
};

/*
 *  Structure, defines for setting page length
 */
#define PAGE_SETL    04         /* Set Page Length (Ioctl TCSLEN) */
#define PAGE_MSK     03         /* Paging Ioctl Command Mask (TCSLEN) */
#define PAGE_ON      01         /* Enable Paging (TCSLEN) */
#define PAGE_OFF     02         /* Disable Paging (TCSLEN) */
#ifndef PAGE_ENB
#define PAGE_ENB     PAGE_ON    /* For compatibility with older version */
#endif

struct  tty_page {
        char          tp_flags;
        unsigned char tp_slen;
};

#define TIOC            ('T'<<8)
#define TCGETS          (TIOC|1)
#define TCSETS          (TIOC|2)
#define TCSETSW         (TIOC|3)
#define TCSETSF         (TIOC|4)
#define TCGETA          (TIOC|5)
#define TCSETA          (TIOC|6)
#define TCSETAW         (TIOC|7)
#define TCSETAF         (TIOC|8)
#define TCSBRK          (TIOC|9)        /* SVID interface */
#define TCSBREAK        (TIOC|10)       /* 0->.25 seconds else <arg>ms */
#define TCXONC          (TIOC|11)
#define TCFLSH          (TIOC|12)
#define TCGLEN          (TIOC|13)
#define TCSLEN          (TIOC|14)
#define TCSAK           (TIOC|15)
#define TCQSAK          (TIOC|16)
#define TCTRUST         (TIOC|17)
#define TCQTRUST        (TIOC|18)
#define TCSMAP          (TIOC|19)
#define TCGMAP          (TIOC|20)
#define TCKEP           (TIOC|21)
#define TCGSAK          (TIOC|22)
#define TCLOOP          (TIOC|23)
#define TCVPD           (TIOC|24)
#define TCREG           (TIOC|25)
#define TCGSTATUS       (TIOC|26)
#define TCSCONTROL      (TIOC|27)
#define TCSCSMAP        (TIOC|28)
#define TCGCSMAP        (TIOC|29)
#define TCMGR           TCSAK
#define TCQMGR          TCQSAK
#define TIONREAD        FIONREAD

#define TCSAKOFF        0               /* used with TCSAK and TCQSAK */
#define TCSAKON         1
#define TCUNTRUSTED     0               /* used with TCTRUST and TCQTRUCT */
#define TCTRUSTED       1

#endif /* _H_TERMIO */
