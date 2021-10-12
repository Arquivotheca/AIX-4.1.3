/* @(#)87	1.8.1.2  src/bos/kernel/sys/lldebug.h, sysdb, bos411, 9428A410j 1/28/94 11:19:30 */

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_LLDEBUG
#define _H_LLDEBUG

/* debugger available flags - set by mkboot */

#define	DO_TRAP 	 0x00000000	/* Do invoke at bootup		*/
#define	DONT_TRAP 	 0x00000001	/* Don't invoke at boot,but 	*/
					/* debugger is still invokable  */
#define	NO_LLDB 	 0x00000002	/* debugger is not ever to be called */
#define	LLDB_MASK	 0x0000ffff	/* Mask debugger flags in dbg_avail */
#define EXPAND_RAMD	 0x00010000	/* bootexpand moved RAM disk around */
#define EXPAND_BCFG	 0x00020000	/* bootexpand moved config around */
#define LOCK_INSTR_AVAIL 0x80000000	/* set instr_avail (lock instrumentation) */


/* Debugger return codes. */
#define OK      0       /* all is ok */
#define DBERR   -1      /* all is not ok */
#define INITRET -2      /* can't initialize something */
#define NOTTY   -3      /* can't open a tty */
#define NOTMYTRAP 1     /* this trap belongs to some other debugger */
#define QUITRET   3
#define NODEBUGRET 4	/* the debugger is not invokable */
#define DUMPRET 8


#endif /* _H_LLDEBUG */
