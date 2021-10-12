/* @(#)77	1.11  src/bos/kernel/sys/pri.h, sysproc, bos411, 9428A410j 4/2/93 13:50:12 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: SLPDEF
 *		sleep
 *		
 *
 *   ORIGINS: 3,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_PRI
#define _H_PRI

/*
 * NOTES:
 *      1. processes that are not subject to scheduling should run with a 
 *         a fixed priority, and their priority should be more favored
 *         than the scheduling process's priority (i.e. PRI_SCHED).
 */
#include        <sys/proc.h>

#define PMASK   127
#define PCATCH  0x100            /* must not be equal to SWAKEONSIG. Both
				    PCATCH and SWAKEONSIG must be greater 
				    than PMASK */

#ifdef _KERNEL
/* upwardly compatible sleep macro: */
#define SLPDEF(C,D) (sleepx((C),(D)&PMASK,(D)&PCATCH|(((D)&PMASK)>PZERO?SWAKEONSIG:0)))
#define sleep(C, D)     SLPDEF(C, D)
#endif /* _KERNEL */

/*
 * sleep / sleepx priorities
 * fixed process priorities
 * should not be altered too much
 * changing a priority's relationship (< = >) to
 * PSWP, PINOD, PRIBIO, PZERO, PUSER, or PIDLE is "risky".
 */

#define PSWP            0       /* "swapping" */
#define PRI_SCHED       16      /* priority for the scheduling process.
                                   NOTE: all processes that are subject to 
                                   scheduled must run with a priority less
                                   favored than this                        */
#define PINOD           18      /* inode locks */
#define PZERO           25      /* if >PZERO signals interrupt the sleep */
				/* above comment no longer true! see
				   SWAKEONSIG and sleepx() for details   */
#define PPIPE           26
#define PMSG            27      /* ipc messages */
#define TTIPRI          30      /* tty input */
#define TTOPRI          31      /* tty output */
#define PWAIT           33
#define PUSER           40
#define PRI_LOW         PIDLE-1 /* least favored priority for a non
                                   realtime process  */
#define PIDLE           PMASK   /* wait process's priority */

/* Real Time Process Priority assignments for use with the setpri()
   system call
 */

#define MBUF_RTPRI	36	/* MBUF buffer pool expansion process */
#define NETPCL_RTPRI	37	/* Network  Protocol Process */
#define HFTSC_RTPRI	38	/* High function terminal screen manager */
#define NETTMR_RTPRI	39	/* Network Timer Processes */

#endif /* _H_PRI */
