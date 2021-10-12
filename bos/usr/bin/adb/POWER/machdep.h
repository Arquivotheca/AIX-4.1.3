/* @(#)21	1.8  src/bos/usr/bin/adb/POWER/machdep.h, cmdadb, bos411, 9428A410j 6/15/90 16:57:18 */
#ifndef _H_MACHDEPP
#define _H_MACHDEPP
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS:  No functions.
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  Common definitions
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/errno.h>
#ifndef _KERNEL
#define _KERNEL
#include <sys/reg.h>
#undef _KERNEL
#endif
#include <sys/debug.h>

#define	FLOAT	1	

#define	AOcanon(h)	(h->f_magic == 0x01df)

#define NREGS		32
#define NKREGS	 	 9
#define MAXFPREGS  	32
#define NREGSYNONYMS	 6

#define REGNO(regno)	 ((regno < NREGS) ? regno : regno - IAR + NREGS)
#define SYSREGNO(regno)	 (regno - IAR + NREGS)
#define FPREGNO(regno)	 (regno - FPR0)
#define REGTOSYS(regno)	 ((regno < NREGS) ? regno : regno + IAR - NREGS)
#define FREGTOSYS(regno) (regno + FPR0)
#endif  /*  _H_MACHDEPP */
