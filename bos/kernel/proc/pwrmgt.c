static char sccsid[] = "@(#)74  1.3  src/bos/kernel/proc/pwrmgt.c, sysproc, bos41J, 9523C_all 6/9/95 17:26:20";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: pm_proc_start
 *              pm_proc_stop
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/errno.h>
#include <sys/param.h>
#include <sys/intr.h>
#include <sys/low.h>
#include <sys/proc.h>
#include <sys/pri.h>
#include <sys/priv.h>
#include <sys/pm.h>
#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/var.h>
#include "swapper_data.h"

/*
 * NAME: pm_proc_stop
 *
 * FUNCTI0N: Stop threads within the context of power management.
 *
 *	This is an undocumented kernel service provided for power
 *	management.  It is used within the course of shutting down 
 *	the system.  The first time it is called, user processes
 *	are suspended.  This quarantees that file I/O will only be
 * 	initiated by the caller, allowing all data to be successfully
 *	synch'd to the disk.  The second time this routine is 
 * 	called, kprocs are suspended.  This functionality is split 
 * 	because kprocs may be needed to perform the sync or resolve 
 *	page faults. 
 *
 * EXECUTI0N ENVIR0NMENT:
 *
 *     	This routine can only be called from the process environment.
 *     	It cannot page fault after disabling interrupts.
 */

void
pm_proc_stop(ulong flags)
{
	struct thread *ct;
	struct thread *t;
	int need_to_wait;
	int ipri;

	/*
	 * Record thread that initiated this mess.  It must be
	 * exempted from suspension to complete the shutdown.
	 */
	ct = curthread;

	assert((ipri = disable_lock(INTMAX, &proc_int_lock)) == INTBASE);

	do {
		need_to_wait = FALSE;

		for (t = &thread[0]; t < (struct thread *)v.ve_thread; t++) {
	
			/* skip this thread, if it is ... */
			if ((t->t_state == TSNONE) ||	  /* an empty slot */
			    (t->t_state == TSIDL)  ||	  /* being created */
			    (t->t_state == TSZOMB) ||	  /* terminated */
			    (t->t_flags & TPMSTOP) ||     /* already stopped */
			    (t->t_pri == PIDLE)    ||     /* waitproc */
			    (t == ct))	         	  /* the caller */
				continue;

			/* Ensure that thread t is a candidate to be stopped */ 
			switch (flags) {
			case PM_STOP_KOTHREADS :
				if (t->t_flags & TKTHREAD)
					break;
				continue;
			default :
				if (t->t_flags & TKTHREAD)
					continue;
				break;
			}
	
			t->t_flags |= TPMREQSTOP;
				
			switch(t->t_wtype) {
			case TWCPU :
                        	/* 
			 	 * If thread is on runq and in user mode,
			 	 * remove it from the queue and set flag 
			 	 * indicating that it has been pm stopped.
			 	 */
                        	if (t->t_suspend == 0) {
                                	remrq(t);
					t->t_flags |= TPMSTOP;
					break;
                        	}
				need_to_wait = TRUE;
				break;
			case TWMEM :
				/* 
				 * thread is swapped. This occurs as the
				 * thread returns to user mode.
				 */
				t->t_flags |= TPMSTOP;
				break;
			case TNOWAIT :
				/* 
				 * Could be a stopped thread or another
				 * thread running on another processor.
			 	 */
				if ((t->t_state == TSSTOP) && 
				    (t->t_lockcount == 0))
					t->t_flags |= TPMSTOP;
				else
					need_to_wait = TRUE;
				break;
			default :
				/* 
				 * Mark sleepers.  The sleep state is only
				 * seen after the thread has passed through
				 * the dispatcher.  It is not running.
				 */ 
				if (((t->t_state == TSSTOP) || 
					(t->t_state == TSSLEEP)) && 
				    (t->t_lockcount == 0))
					t->t_flags |= TPMSTOP;
				else
					need_to_wait = TRUE;
			}
		}

		if (need_to_wait) {
			unlock_enable(ipri, &proc_int_lock);
			delay(2);
			ipri = disable_lock(INTMAX, &proc_int_lock);
		}

	} while (need_to_wait);

	unlock_enable(ipri, &proc_int_lock);
}

/*
 * NAME: pm_proc_start
 *
 * FUNCTI0N: 
 *	Starts threads that have been previously suspended by 
 *	pm_proc_stop.  This is used during the boot sequence.
 *
 * EXECUTI0N ENVIR0NMENT:
 *
 *      This routine may be called from the process or interrupt environments.
 *      It cannot page fault.
 */

void
pm_proc_start()
{
	struct thread *t;
	int ipri;

        ipri = disable_lock(INTMAX, &proc_int_lock);

        for (t = &thread[0]; t < (struct thread *)v.ve_thread; t++) {
	
		/* 
		 * If the thread has been stopped, then put it
		 * back into the appropriate state.
		 */
		if (t->t_flags & TPMSTOP) {

			/* Turn off flags */
			t->t_flags &= ~TPMREQSTOP;
			t->t_flags &= ~TPMSTOP;

			switch(t->t_wtype) {
			case TNOWAIT :
				if (t->t_state == TSRUN)
					setrq(t, E_WKX_NO_PREEMPT, RQTAIL);
			}
		}
	}

        unlock_enable(ipri, &proc_int_lock);
}
