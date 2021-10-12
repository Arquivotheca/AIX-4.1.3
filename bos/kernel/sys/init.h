/* @(#)72	1.54.1.13  src/bos/kernel/sys/init.h, syssi, bos41J, 9515A_all 4/3/95 09:41:28 */
/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#ifndef _H_INIT  
#define _H_INIT  

/*----------------------------------------------------------------------*/
/*  Array containing the addresses of the various initializing		*/
/*  routines executed at boot time.  Any initialization not required    */
/*  can be commented out of the initialization array.			*/
/*----------------------------------------------------------------------*/

extern void     init_ldr(),     clkstart(),     cinit();
/* extern void	trcinit(),	credinit(); */
extern void	credinit(); 

/* Trace is a subsystem */
void    trcconfig_dmy()
{
}

extern void	binit(),	errinit(),	finit();
extern void	utsinit(),	vfsinit(),	flckinit();
extern void	audinit(),	iost_init(),	netinit();
extern void	tinit(),	devsw_init(),	dmpinit();
extern void 	init_mname();
extern void	privinit();
extern void	vm_init();
extern void	cs_mpc_init();
extern void	scrubinit();
extern void	upfinit();
extern void	csinit();
extern void	epowinit();
#ifdef _POWER_MP
extern void	selpollinit();
extern void	uphysinit();
extern void	uio_init();
extern void	ipc_lock_init();
#endif /* _POWER_MP */
#ifdef _RSPC
extern void	rminit();
extern void	hdlight_init();
#endif /* _RSPC */
#ifdef _POWER
extern void 	mdinit();
#endif /* _POWER */
#ifdef PM_SUPPORT
extern void	pm_kernel_init();
#endif /* PM_SUPPORT */

/* functions #if'ed out below until needed or proven to work */

void (*init_tbl[])() = {
	vm_init,			/* finish vmm initialization	  */
	init_mname,			/* machine name initialization    */
	credinit,
	init_ldr,			/* loader tables                  */
	csinit,				/* initilize cs() call		  */

#if 0
	clkstart,			/* clock                          */
#endif /* 0 */
	devsw_init,			/* initialize devsw table	  */
	iost_init,			/* initialize iostat structure	  */
#ifdef _RSPC
	rminit,				/* initialize contig real mem heap*/
	hdlight_init,			/* initialize hard disk light	  */
#endif /* _RSPC */
#ifdef _POWER
	mdinit,				/* machine device driver          */
#endif /* _POWER */
	errinit,			/* error map table                */
	cinit,				/* clist blocks                   */
	binit,				/* buffer cache                   */
	trcconfig_dmy,			/* trace : trcinit -> trcconfig_dmy */
	dmpinit,			/* dump                           */
	tinit,				/* timer intialization		  */
	epowinit,			/* EPOW handler registration	  */
	netinit,			/* network intialization	  */
	upfinit,			/* uprintf intialization	  */
	finit,				/* file system                    */
	utsinit,			/* uname structure                */
	vfsinit,			/* virtual file system            */
	flckinit,			/* file lock table                */
	audinit,			/* audit system                   */
	privinit,			/* privilege system               */
	scrubinit,			/* memory scrubbing               */
#ifdef _POWER_MP
	cs_mpc_init,			/* vmm SID shootdown MPC startup  */
	selpollinit,			/* select/poll service lock init  */
	uphysinit,			/* uphysio service lock init      */
	uio_init,			/* pinu/unpinu service lock init  */
	ipc_lock_init,                  /* init ipc locks                 */
#endif /* _POWER_MP */
#ifdef PM_SUPPORT
	pm_kernel_init,
#endif /* PM_SUPPORT */
	0				/* end of function list           */
};

#endif	/* _H_INIT  */
