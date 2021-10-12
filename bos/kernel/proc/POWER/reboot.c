static char sccsid[] = "@(#)29  1.8.1.14  src/bos/kernel/proc/POWER/reboot.c, sysproc, bos41J, 9514A_all 4/4/95 16:59:17";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: reboot
 *		turn_power_off
 *
 *   ORIGINS: 3,27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/intr.h>
#include <sys/priv.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/reboot.h>
#include <sys/machine.h>
#include <sys/errids.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#if PM_SUPPORT
#include <sys/pm.h>

extern struct _pm_kernel_data	pm_kernel_data;
#endif /* PM_SUPPORT */

extern int copyin();

/*
 *
 * NAME:        turn_power_off
 *
 * FUNCTION:    Powers Down the system
 *
 * Write to power control registers which should turn
 * the power off.  Then read the power status register to generate 
 * the interrupt to the power supply.
 *
 * NOTE: power will only be turned off on RACK models.
 *
 * CALLED BY:	reboot()
 */
void 
turn_power_off(int howto)
{						
#ifdef _POWER_RS

	caddr_t ioccaddr;
	caddr_t	*ioccptr;
	volatile ulong psr;
	int i;
	caddr_t	ioccaddr1;
	volatile ulong *ocscmd_ptr;


	if (__power_rs()) {
		/*
		 * get access to the IOCC 
		 */
		ioccaddr = (caddr_t)io_att(IOCC_BID, 0);
		if (!__power_rsc()) {	
			/*
			 * provided this isn't an RSC box, write the
			 * power control reset register.
			 * 
			 */
			ioccptr = (caddr_t *)(ioccaddr + PCRR_ADDRESS);
			*((short *)ioccptr) = 0xFFFF;		
		}					
		/*
		 * Now loop forever reading the Power Status Register
		 */
		ioccptr = (caddr_t *)(ioccaddr + PSR_ADDRESS);
		ocscmd_ptr = (ulong *)(ioccaddr + IOCC_DELAY);
		for (;;) {
			for (i=0; i<500000; i++)
				iocc_delay(ocscmd_ptr,4);
			psr = *((volatile ulong *) ioccptr);
		}
	}
#endif /* _POWER_RS */

#ifdef _POWER_PC
	if (__power_pc()) {

#ifdef _RS6K_SMP_MCA
		if (__rs6k_smp_mca()) {
			if (howto == RB_HARDIPL) {
				/* A write to the Power On Reset Control
				   Register will generate a reset to all
				   processors */
				*prcr_addr = 0xFFFF;
			} else {
				/* A write to the Power Off Control Register
				   will create a software power-off */
				*spocr_addr = 0xFFFF;
			}
			__iospace_sync();
		}
#endif /* _RS6K_SMP_MCA */

		/* NOTE:  The AWS PowerPC System Architecture defines a 
		 * software Power Off Control Register.  At this point only
		 * Pegasus uses it.  Here is how to access it:
		 * sys_resource_ptr->sys_regs.pwr_off_cr = 0xFFFF;
		 */

#ifdef PM_SUPPORT
		pm_kernel_data.turn_power_off
			= pm_kernel_data.turn_power_off_pm_core;
		if (pm_kernel_data.turn_power_off != NULL) {
			(*(pm_kernel_data.turn_power_off))();
		}
#endif /* PM_SUPPORT */
		/*
		 * Now loop forever reading the Power Status Register
		 */
#ifdef _RS6K_SMP_MCA
		for (;;) psr = *pksr_addr;
#else
		for (;;) psr = sys_resource_ptr->sys_regs.pwr_key_status;
#endif /* _RS6K_SMP_MCA */
	}
#endif /* _POWER_PC */

}


/*
 *
 * NAME:        reboot
 *
 * FUNCTION:    Reboots the system
 *
 * INPUT:	boot device
 *
 * OUTPUT:	-1=error, otherwise we're gone
 *
 * WARNING!!!!
 *	User's of this system call are NOT portable.
 *
 */

int
reboot(int howto, void *time)
{
	struct timestruc_t 	ct, bt;
	time_t			htime;
	int			ipri;
	struct errstruct {
				struct err_rec0 hdr;
				int	uid;
				int	howto;
				int	time;	
			 } err = { ERRID_REBOOT_ID, "SYSPROC", 0, 0 };


	/* Check priviledge before stopping the machine */
	if (privcheck(SYS_CONFIG) == EPERM)
	{
		u.u_error = EPERM;
		return -1;
	}

	ipri = disable_lock(INTMAX, &tod_lock);

	/*  
	 * Update the time of day chip at shutdown.  The processor clock is 
	 * read every clock tick and is used to update the global variable 
	 * tm.sec, which is used to update the time of day chip.  The 
	 * processor clock is more accurate than the time of day clock.
	 * Update the clock at second boundaries.  Some clocks do not have 
	 * a finer granularity than one second. 
	 */
	curtime(&bt);
	do { 
		curtime(&ct); 
	} while ( bt.tv_sec == ct.tv_sec );
	write_clock();

	/* log shutdown event */
	err.uid = curproc->p_uid;
	err.howto = howto;
	err.time = 0; 

	switch (howto) {
		case RB_SOFTIPL : 
			/* log the shutdown of the system */
			errsave(&err, sizeof(err));
			/* Call the system reset slih to send us back to ROS 
			 * the parameter of 1 tells sr_slih() that we came
			 * from reboot()
                         */
			sr_slih(1); 
		case RB_HALT : 
		case RB_HARDIPL :
			/* log the shutdown of the system */
			errsave(&err, sizeof(err));
			turn_power_off(howto);
		case RB_POWIPL : 
			unlock_enable(ipri, &tod_lock);
			if (copyin((caddr_t)time,(caddr_t)&htime,sizeof(htime)))
			{
				u.u_error = EFAULT;
				return(-1);
			}
			ipri = disable_lock(INTMAX, &tod_lock);
			/* log the shutdown of the system */
			err.time = htime; 
			errsave(&err, sizeof(err));
			/* 
			 * Program time of day clock so that the system will 
			 * power on automatically.  Then turn the power off so
			 * the alarm interrupt will work at the designated
			 * time.  The alarm and power off functions are only
			 * supported on racks.
			 */
			set_alarm(htime);
			turn_power_off(howto);
		default : 
			u.u_error = EINVAL;
			unlock_enable(ipri, &tod_lock);
			return(-1);
	}
}
