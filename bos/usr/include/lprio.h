/* @(#)96	1.7  src/bos/usr/include/lprio.h, libplot, bos411, 9428A410j 6/16/90 00:11:35 */
/*
 * COMPONENT_NAME: cmdgraf
 *
 * FUNCTIONS:
 *
 * ORIGINS: 10,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* lprio.h	5.2 87/03/16 11:23:20 */
#ifndef _H_LPRIO
#define _H_LPRIO

#include <sys/termio.h>
#include <sys/ioctl.h>

struct lprio {
	int     ind;            /* indent value */
	int     col;            /* maximum character count */
	int     line;           /* maximum line count */
};

struct lprmode {
	int     modes;          /* optional line printer modes */
};

	/* option flags */
#define PLOT    01
#define NOCL    02      /* no cr/lf */
#define NOFF    0400
#define NONL    01000
#define NOTB    02000
#define NOBS    04000
#define NOCR    010000
#define CAPS    020000
#define WRAP    040000


#define LPR     ('l'<<8)
#define LPRGET  (LPR|01)
#define LPRSET  (LPR|02)
#define LPRGETV (LPR|05)
#define LPRSETV (LPR|06)

/*  IBM additional ioctl's   */

#define LPRVRMG (LPR|10)
#define LPRVRMS (LPR|11)
#define LPRUGES (LPR|12)
#define LPRUFLS (LPR|13)
#define LPRURES (LPR|14)
#define LPRGMOD (LPR|15)
#define LPRSMOD (LPR|16)
#define LPRGETA (LPR|17)
#define LPRSETA (LPR|18)
#define LPRGTOV (LPR|19)
#define LPRSTOV (LPR|20)


/* optional printer modes */
struct oprmode {
	int flags;
};
#define LPRSYNC      0x01
#define LPRALLERR    0x02
#define LPRFONTINIT  0x04
#define SLOWPRNT     0x08


/* error reporting structure */
struct LPRUDE
{       int       status;       /* error reason code */
	int       cresult;      /* current operation result :PSB */
	int       tadapt;       /* adapter type */
	int       npio;         /* number pending IO operations */
};

	/* status values - error reason codes */

#define LPRPOUT  01      /* printer out of forms - intervention req'd */
#define LPRPTIM  0400    /* timeout - intervention required */
#define LPRPERR  01000   /* unspec. internal error - intervention req'd */
#define LPRTERR  02000   /* transmission error */
#define LPRINIT  04000   /* adapter initialization failed */
#define LPRADAP  010000   /* adapter not present */
#define LPRSOFT  020000   /* software error */
#define LPRREAD  040000   /* read error */


	    /* types of adapter */

#define LPRPARALLEL        01
#define LPRSERIAL          02

/* RS232 parameter change structure for LPRGETA and LPRSETA */

struct lpr232 {
	unsigned c_cflag;
};

/* variable timeout value change structure for LPRGTOV and LPRSTOV */
struct lptimer {
	unsigned v_timout;
};

#endif /* _H_LPRIO */

