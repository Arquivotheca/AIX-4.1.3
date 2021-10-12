static char sccsid[] = "@(#)92	1.18  src/bos/kernel/si/globals.c, syssi, bos411, 9428A410j 5/5/94 11:25:56";
/*
 * COMPONENT_NAME: (SYSSI) System Initialization
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/cblock.h>
#include <sys/types.h>
#include <sys/systemcfg.h>

/* defining instances for externs - for now anyhow */
	int	runout;      	/*systm.h - sched.c*/
	int	kernel_heap;	/*malloc.h*/
	int	init_complete;	/*oSwtrmf.c*/
	int	view_vidx;	/*vdb/dbscreen.c*/
	struct chead cfreelist;	/*tty.h*/
	int	vrm_timer[6];	/*error.c*/
	int	hbuf;		/*ios/buf.h*/
	int	mach_model = 0;	/* machine.h */
        int     fp_ie_impl = 0; /* machine.h */
#ifdef _POWER_MP
	uint number_of_cpus = 1;/* number of running cpus */
	uint mproc_physid = 0;/* master processor physical id */
#endif /* _POWER_MP */

	ulong	misc_intrs = 0;	/* dec_flih.s */

#ifdef _RS6K_SMP_MCA

__pegasus() 
{
	return (__rs6k_smp_mca());
}
#endif /* _RS6K_SMP_MCA */

