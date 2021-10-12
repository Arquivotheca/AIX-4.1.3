static char sccsid[] = "@(#)10        1.128.1.12  src/bos/kernel/ios/conf.c, sysios, bos411, 9428A410j 7/3/94 16:10:34";

/*
 * COMPONENT_NAME: (SYSIOS) Device Configuration
 *
 * ORIGINS: 26, 27, 83
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
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * LEVEL 1, 12 Years Bull Confidential Information
*/

#ifdef _POWER
#define DYNAMIC
#endif

/* utsname */

/* SYS macro modified by Michael Winestock (MJW) 04/02/93.                   */
/* SYS macro modified by aubertine, 7/02/94.  _GOLD defined in ios/Makefile  */

#define NID (0x20ffffff)

#ifdef _GOLD
#define SYS "AIX"
#else
#define SYS "AIX __default_build__"
#endif

#define NODE "nodename"
#define VER "4"
#define REL "1"
#define LEVL "0"

#define NDBUF 64
#define SCHEDHZ 20
#define PSLOTPANIC 100
#define PSLOTKILL 200
#define PSLOTWARN 350

#define NKPROC 10
#define NNCB 25
#define MAXNODE 20
#define FLOAT 1
#define NPBUF 8
#define INETLEN 576
#define POWER 0
#define NSABUF 0
#define NTEXT 40
#define NSHLIB 30
#define NCALL 50
#define KMAPSIZ 100
#define SMAPSIZ 75
#define KPROC 1
#define NCLIST (16 * 1024)      /* The size of a cblock is 64 bytes so  */
	                        /* a 16K cblock array will take 1MB     */

extern nulldev();
extern nodev();

/* externs for ram disk driver */
extern ram_open();
extern ram_close();
extern ram_strategy();
extern ram_read();
extern ram_write();
extern ram_ioctl();

/* externs for sys (/dev/tty) driver */
extern syopen();
extern syread();
extern sywrite();
extern syioctl();
extern syselect();

#ifdef _POWER
/* externs for machine driver */
extern mdopen();
extern mdclose();
extern mdioctl();
extern mdinit();
extern mdread();
extern mdmpx();
#endif

/* externs for static console device driver */
extern conopen();
extern conclose();
extern conioctl();
extern conconfig();
extern conselect();
extern conwrite();
extern conread();
extern conrevoke();
extern conmpx();

/* externs for mem, kmem, and null driver */
extern mmread();
extern mmwrite();

/* externs for dump driver */
extern dmpioctl();

/* externs for trace driver */
/* extern trcopen();  */
/* extern trcclose(); */
/* extern trcread(); */
/* extern trcwrite(); */
/* extern trcioctl(); */

/* externs for error driver */
extern erropen();
extern errclose();
extern errread();
extern errwrite();
extern errioctl();


/* externs for audit driver */
extern  auditopen();
extern  auditclose();
extern  auditread();
extern  auditioctl();
extern  auditmpx();

#include "bio.h"
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/space.h>
#include <sys/io.h>
#include <sys/utsname.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/seg.h>

int pwr_cnt;
int pwr_act;
int schedhz = SCHEDHZ;
int pslotpanic = PSLOTPANIC;
int pslotkill = PSLOTKILL;
int pslotwarn = PSLOTWARN;
int lp_cnt = 1;
int inetlen = INETLEN;
int (*dump)() = nulldev;
int dump_addr = 0;

/*
 * These pointers are to an edge vector whose elements point to tty
 * structures that have been sent to ttyinit but have not yet been
 * sent to ttyfree.  The common tty (tty.c) maintains these pointers
 * and the debugger uses them with the ``tty'' command.
 */
struct tty **tty_array, **tty_array_end;

#if 0
int ncwsegs = NCWSEGS;
struct cw_ds cw_ds[NCWSEGS];
#endif 0

#ifndef MACHNAME
#define MACHNAME "unknown"
#endif

struct utsname utsname = {
	SYS,
	NODE,
	REL,
	VER,
	MACHNAME,
};

struct xutsname xutsname = {
	NID,
};

int     (*pwr_clr[])() =
	{
	        (int (*)())0
	};

int     (*dev_init[])() =
	{
	        (int (*)())0
	};

/* Static Device Switch Table: This table defines the major numbers of the
 * device drivers that are statically bound with the kernel. This table is
 * copied to the dynamically allocated device switch table at kernel 
 * initialization time by devsw_init. Dynamically allocated major numbers 
 * are assigned by the configuration manager starting at STATIC_DEVCNT. 
 */

struct devsw static_devsw[STATIC_DEVCNT] = {

/* Major # 0 - Ram disk psuedo device driver */
/* 0 */ { ram_open, ram_close, ram_read, ram_write, ram_ioctl, ram_strategy,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 },

#ifdef _POWER_MP
/* Major # 1 - sys (dev/tty) psuedo device driver */
/* 1 */ { syopen, nulldev, syread, sywrite, syioctl, nodev,
	 0, syselect, nodev, nodev, nodev, nodev, nodev, 0, 0, DEV_MPSAFE },
#else
/* Major # 1 - sys (dev/tty) psuedo device driver */
/* 1 */ { syopen, nulldev, syread, sywrite, syioctl, nodev,
	 0, syselect, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 },
#endif /* _POWER_MP */

#ifdef _POWER_MP
/* Major # 2 - mem (dev/null, dev/mem, dev/kmem) psuedo driver */
/* 2 */ { nulldev, nulldev, mmread, mmwrite, nodev, nodev,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, DEV_MPSAFE },
#else
/* Major # 2 - mem (dev/null, dev/mem, dev/kmem) psuedo driver */
/* 2 */ { nulldev, nulldev, mmread, mmwrite, nodev, nodev,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 },
#endif /* _POWER_MP */

/* Major # 3 - machine  (dev/bus0, dev/nvram) device driver */
/* 3 */ { mdopen, mdclose, mdread, nodev, mdioctl, nodev,
	 0, nodev, mdinit, nodev, nodev, mdmpx, nodev, 0, 0, 0 },

/* Major # 4 - console (dev/console) psuedo device driver */
/* 4 */ { conopen, conclose, conread, conwrite, conioctl, nodev,
	0, conselect, conconfig, nodev, nodev, conmpx, conrevoke, 0, 0,
	 CONS_DEFINED },

/* Major # 5 - system trace (dev/systrc) psuedo device driver */
/* 5  { trcopen, trcclose, trcread, trcwrite, trcioctl, nodev, */
/*	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 }, */
/* 5 */ { nodev, nodev, nodev, nodev, nodev, nodev,
         0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 },

#ifdef _POWER_MP
/* Major # 6 - system error log (sys/error) psuedo device driver */
/* 6 */ { erropen, errclose, errread, errwrite, errioctl, nodev,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, DEV_MPSAFE },
#else
/* Major # 6 - system error log (sys/error) psuedo device driver */
/* 6 */ { erropen, errclose, errread, errwrite, errioctl, nodev,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 },
#endif /* _POWER_MP */

#ifdef _POWER_MP
/* Major # 7 - system dump (dev/sysdump) psuedo device driver */
/* 7 */ { nulldev, nulldev, nodev, nodev, dmpioctl, nodev,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, DEV_MPSAFE },
#else
/* Major # 7 - system dump (dev/sysdump) psuedo device driver */
/* 7 */ { nulldev, nulldev, nodev, nodev, dmpioctl, nodev,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 },
#endif /* _POWER_MP */

/* Major #8 - system audit (dev/audit) psuedo device driver */
/* 8 */ { auditopen, auditclose, auditread, nodev, auditioctl, nodev,
	 0, nodev, nodev, nodev, nodev, auditmpx, nodev, 0, 0, 0 },

/* Major #9 - unused static device driver entry */
/* 9 */ { nodev, nodev, nodev, nodev, nodev, nodev,
	 0, nodev, nodev, nodev, nodev, nodev, nodev, 0, 0, 0 },

/* Major #(STATIC_DEVCNT) - FIRST DYNAMICALLY ASSIGNED MAJOR NUMBER */

};

int devcnt = (int) STATIC_DEVCNT;



