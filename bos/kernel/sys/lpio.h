/* @(#)13 1.24 src/bos/kernel/sys/lpio.h, sysxprnt, bos411, 9428A410j 5/6/94 09:46:38 */
/*
 * COMPONENT_NAME: (sysxprnt) Include file for use with 'lp' special file
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27, 83
 *
 */
/*
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_LPIO
#define _H_LPIO

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* include file for use with the 'lp' special file                         */  
/* For more detailed information, see the AIX Technical Reference          */  
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <sys/termio.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/* extended open parms used for the extend operand in the open.           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */

#define LPDIAGMOD 1             /* Request diagnostic mode on open        */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* ioctl constants                                                         */  
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define LPR     ('l'<<8)
#define LPRGET  (LPR|01)	/* Get margin parameters                   */
#define LPRSET  (LPR|02)	/* Set margin parameters                   */
#define LPRGETA (LPR|17)	/* Get RS232 parameters                    */
#define LPRSETA (LPR|18)	/* Set RS232 parameters                    */
#define LPRGTOV (LPR|19)	/* Get the time-out value, seconds         */
#define LPRSTOV (LPR|20)	/* Set the time-out value, seconds         */
#define LPQUERY	(LPR|23)	/* Query Command		           */
#define LPDIAG	(LPR|26)	/* Parallel printer diagnostics            */
#define LPRMODG	(LPR|27)	/* Get printer modes  		           */
#define LPRMODS	(LPR|28)	/* Set printer modes  		           */
				/* controlling a write request		   */
#define	LPWRITE_REQ	(LPR|30) /* in the Streams Based sptr module	   */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*	Declarations of structures used with lp ioctls                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* margin parameters structure used with LPRGET and LPRSET                 */
struct lprio {
	int	ind;		/* indent size			           */
	int	col;		/* columns per line		           */
	int	line;		/* lines per page		           */
};


/* printer mode structure used with LPRMODG and LPRMODS                    */
/* Note: The default value for all modes is off.                           */
struct lprmod {
       int  modes;         /* line printer modes                           */
#define PLOT      0x0001   /* if on, bypass all formatting/interpretation  */
                           /* modes.  NOFF, NONL, NOCL, NOTAB, NOBS, NOCR  */
                           /* CAPS and WRAP are ignored.                   */
#define NOFF      0x0002   /* if on, simulate form feed function with      */
                           /* line feeds, based on line value above.       */
#define NONL      0x0004   /* if on, substitute carriage returns for any   */
                           /* line feeds                                   */
#define NOCL      0x0008   /* if on, do not insert a carriage return after */
			   /* each line feed                               */
#define NOTB      0x0010   /* if on, do not expand tabs. if off, simulate  */
                           /* 8 position tabs with spaces.                 */
#define NOBS      0x0020   /* if on, ignore backspace characters           */
#define NOCR      0x0040   /* if on, substitute line feeds for any         */
                           /* carriage returns                             */
#define CAPS      0x0080   /* if on, map lower-case alphabetics to upper   */
                           /* case                                         */
#define WRAP      0x0100   /* if on, print characters beyond the page      */
                           /* width on the next line. If off, truncate     */
                           /* characters extending beyond page width.      */
#define FONTINIT  0x0200   /* if on, fonts have been initialized           */
#define RPTERR    0x0400   /* if on, errors are reported back to the       */
                           /* application.  If off, the device driver will */
                           /* not return until the error is cleared or     */
                           /* a cancel signal is received.                 */
#define IGNOREPE  0x0800   /* Ignore the paper end line                    */
/* Temporary Definitions and Declarations */
#define DOALLWRITES	01000000
};


/* timeout structure used with LPRGTOV and LPRSTOV                         */
struct lptimer {  
	int      v_timout;      /* time out value in seconds               */
};


struct lpquery {
	int	status;		/* device status	                   */
#define LPST_POUT	0x01	/* paper out condition		           */
#define LPST_TOUT	0x02	/* printer timeout - intervention required */
#define LPST_ERROR	0x04	/* unspec. error, intervention required	   */
#define LPST_BUSY	0x08	/* printer busy, may not be timeout yet	   */
#define LPST_NOSLCT	0x020	/* printer is not selected		   */
#define LPST_SOFT	0x040	/* software error			   */
#define LPST_OFF	0x080	/* power off 				   */
	int	tadapt;		/* adapter type		                   */
#define LPAD_R1         1       /* releas one nio device                   */
#define LPAD_SA         2       /* relase two NIO device with DMA          */
#define LPAD_NIO        3       /* new adapter high speed DMA              */
#define LPAD_IOD        4       /* release IOD device                      */
	int	reccnt;		/* number of bytes in rec buffer           */
	char	rsvd[12];	/* reserved, set to binary value of 0      */
};


/* diagnostics structure used with LPDIAG, parallel printers only            */
struct lpdiag {
	int	cmd;		/* command to perform	                     */
#define LP_R_STAT	0x01	/* put contents of status reg in value field */
#define LP_R_CNTL	0x02	/* put contents of contrl reg in value field */
#define LP_W_CNTL	0x03	/* put contents of value field in contrl reg */
#define LP_R_DATA       0x04    /* put contents of data reg in value field   */
#define LP_W_DATA       0x05    /* put contents of value field in data reg   */
#define LP_WATCHINT     0x06    /* Watch for an interrupt to occur           */
#define LP_DIDINTOCC    0x07    /* Did an interrupt occur                    */
	int	value;		/* data read or to be written, 0 - 255	     */
#define LP_INTDIDOCC    0x01    /* Interrupt did occur                       */
#define LP_INTNOTOCC    0x02    /* Interrupt did not occur                   */
	char	rsvd[12];	/* reserved, set to binary value of 0	     */
};


/* RS232 parameter change structure for LPRGETA and LPRSETA */
struct lpr232 {
	unsigned c_cflag;
};
typedef volatile unsigned char REGTYPE; /* register data type */

/* Declarations for the "Write Interface" of the Streams Based sptr module : */

/* This macro declares an "strioctl" structure */
#define  SP_WRITE_DECL  struct strioctl lpwrite_str   ;      

/* This macro initialises the declared "strioctl" structure */
#define SP_WRITE_INIT				\
	lpwrite_str.ic_cmd	= LPWRITE_REQ;	\
	lpwrite_str.ic_timout	= -1;	\
	lpwrite_str.ic_len	= 0;	\
	lpwrite_str.ic_dp	= NULL;           

/* Write Interface Macro */
#define	SP_WRITE(fd, buf, len)					\
	( !isastream(fd) ? write(fd, buf, len) :		\
	  ( write(fd, buf, len) < 0 ? -1 :			\
	    ( ioctl(fd, I_STR, &lpwrite_str) == 0 ? len : -1 )	\
	  )							\
	)

/* Declarations for the "Write Interface" of the Streams Based sptr module . */

#endif    /* _H_LPIO */
