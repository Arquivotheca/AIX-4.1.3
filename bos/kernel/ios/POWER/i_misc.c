static char sccsid[] = "@(#)33	1.23.3.21  src/bos/kernel/ios/POWER/i_misc.c, sysios, bos41J, 9516B_all 4/21/95 11:20:26";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: i_epow, epow_timer, io_exception
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/buid.h>
#include <sys/low.h>
#include <sys/signal.h>
#include <sys/except.h>
#include <sys/intr.h>
#include <sys/mstsave.h>
#include <sys/iplcb.h>
#include <sys/timer.h>
#include <sys/vmker.h>
#include <sys/ppda.h>
#include <sys/syspest.h>
#include <sys/systemcfg.h>
#include <sys/sys_resource.h>
#include "dma_hw.h"
#include "intr_hw.h"
#include "interrupt.h"
#ifdef _RS6K_SMP_MCA
#include "pegasus.h"
#include <sys/processor.h>
#endif

#ifdef _RS6K_SMP_MCA
/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 */
#define DISKETTE_INT_LVL	4
extern int fd_mutex;
extern int pgs_SSGA_lvl;
extern void d_abort_fd();
#endif /* _RS6K_SMP_MCA */
 
extern  struct ppda     ppda[];

#ifdef _POWER_MP
extern	struct i_data	i_data;	/* interrupt handler data struct*/
#endif /* _POWER_MP */

struct intr	scuhand;	/* keep in pinned part of kernel */
struct intr     epowhand;	/* keep in pinned part of kernel */
#ifdef _RS6K_SMP_MCA
struct intr     epowhand_dev;	/* keep in pinned part of kernel */
#endif
struct intr	*epowhandler;	/* keep in pinned part of kernel */
uint get_pksr();

#ifdef _RSPC
int	fw_keycase = KEY_POS_NORMAL;
#endif /* _RSPC */

/*
 * allocated, in pinned memory,  and initializes various error
 * logging structures
 */
struct	miscerr	misc_log = { ERRID_MISC_ERR,"SYSIOS ", 0, 0, 0 };

struct	scuerr	exdma_log = { ERRID_EXCHECK_DMA,"SYSIOS ", 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0 };
struct	scuerr	scrub_log  = { ERRID_EXCHECK_SCRUB,"SYSIOS ", 0, 0, 0, 
				0, 0, 0, 0, 0, 0, 0, 0 };

struct	epowerr	suspend_log = { ERRID_EPOW_SUS,"SYSIOS ", 0 };
struct	epowerr	resume_log  = { ERRID_EPOW_RES,"SYSIOS ", 0 };

int	epow_status = RESUME;	/* power status flag, initialize to normal */

struct 	trb	*epow_trb;	/* pointer to timer block for epow */


/*
 * NAME:  i_epow
 *
 * FUNCTION:
 *      This routine calls all of the registered EPOW interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can only be called by the interrupt handler
 *      that catches the EPOW interrupt on interrupt priority INTEPOW.
 *
 *      EPOW interrupt handlers are registered via i_init. They are
 *      not called by i_slih though. This is because each of them
 *      must be called, not just the first that returns INTR_SUCC.
 *
 *      This routine does not page fault.
 *
 * RETURN VALUE DESCRIPTION:
 *     			INTR_SUCC
 *
 * EXTERNAL PROCEDURES CALLED:
 *			i_disable, i_enable, io_att, io_det
 *      		All registered EPOW interrupt handlers.
 */

int
i_epow( 
	struct intr *handler)			/* EPOW interrupt handler */
{
	register ulong	power_status;		/* resume power status */
	register ulong	behavior;		/* expected behavior */
	register int  	ipri;			/* interrupt priority */
	register int  	i_flag;			/* flag for intr struct */

	/*
	 * We're running at INTEPOW
	 */

	/*
	 * Read the Power Status Register 
  	 */
	power_status = get_pksr();
	behavior = (power_status >> BEHAVIOR_SHIFT) & BEHAVIOR_MASK;
	power_status &= EPOW_MASK;

	/*
	 * Don't act on warnings which are sure to be followed by more
	 * serious errors if real, so that a transient failure does not
	 * disrupt SCSI activity. Likewise don't act on "everything's fine".
	 */
	if ((behavior == USE_EPOW_BITS) &&
	    ((power_status == PWR_OVERLOAD) ||
	     ((power_status == NORMAL_STATE) && (epow_status == RESUME))))
		return INTR_SUCC;

	/* log errors */
	suspend_log.psr = power_status;

	if ( epow_status != SUSPEND ) {
		if ( power_status == BATTERY ) {
			/* running on battery */
			i_flag = EPOW_BATTERY;
			epow_status = SUSPEND_BAT;
			pidsig(1, SIGPWR);
		} else if ( (power_status == NORMAL_STATE) &&
			    (epow_status == SUSPEND_BAT) ) {
			/*
			 * This implies that we have just gone from
			 * battery power back to main power. We will
			 * resume via the timer since it runs at INTTIMER. 
			 * Start Epow timer for immediate interrupt.
			 */
			while(tstop(epow_trb));
			epow_trb->func_data = (ulong)FROM_FLIH;
			epow_trb->timeout.it_value.tv_sec = 0;
			epow_trb->timeout.it_value.tv_nsec = 0;
			tstart(epow_trb);
			return( INTR_SUCC );
		}
		else {
			/* loss of power */
			i_flag = EPOW_SUSPEND;	
			epow_status = SUSPEND;
		}

		/*
		 *  Run the list of interrupt handlers that have
		 *  requested notification of EPOW. Call each of them.
		 */
		handler = handler->next;
		while ( handler != (struct intr *)NULL ) {
			handler->flags |= i_flag;
			(void)(*handler->handler) (handler);
			handler = handler->next;
		}

	}
	errsave(&suspend_log, sizeof(suspend_log));
	
	/*
	 * Start Epow timer for reasonable delay (allow for false EPOW)
	 * to pop if we're still alive; set func data to reflect
	 * we started the timer from the flih
	 */
	while(tstop(epow_trb));
        epow_trb->func_data = (ulong)FROM_FLIH;
	epow_trb->timeout.it_value.tv_sec = 0;
	epow_trb->timeout.it_value.tv_nsec = 50000000;
	tstart(epow_trb);

	return( INTR_SUCC );
}

/*
 * NAME:  epow_timer
 *
 * FUNCTION:
 *      This routine is the timer handler for EPOW.  It resumes normal system
 *	operation if power is resumed, and detects and reports fan faults
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can only be called by the system timer service as a
 *	result of the timer expiring at INTMAX/INTEPOW.
 *
 *      This routine does not page fault.
 *
 * RETURN VALUE DESCRIPTION:
 *     			None
 *
 * EXTERNAL PROCEDURES CALLED:
 */
void
epow_timer( 
	struct trb *t)			/* trb structure pointer */
{
	register ulong	power_status;		/* resume power status */
	register ulong  behavior;		/* expected behavior */
	register int  	ipri;			/* interrupt priority */
	register int	plvl=0;			/* Processor level */
	struct intr *handler;			/* interrupt handler pointer*/

	/* 
	 * Read Power Status Register
	 */
	power_status = get_pksr();
	behavior = (power_status >> BEHAVIOR_SHIFT) & BEHAVIOR_MASK;
	power_status &= EPOW_MASK;

	/*
	 * Fake behavior for machines not setting it
	 */
        if (behavior == USE_EPOW_BITS) {
                switch (power_status) {
                case THERMAL_WARNING :
                case PWR_FAN_FAULT :
                        behavior = FAST_SHUTDOWN;
                        break;
                case BATTERY :
                        behavior = IMMED_SHUTDOWN;
                        break;
                }
        }

#ifdef _RS6K_SMP_MCA
	if ((__rs6k_smp_mca()) && (power_status & LOPP)) {
		/* wait for the end of the EPOW transient */
		epow_trb->func_data = (ulong)FROM_FLIH;
		epow_trb->timeout.it_value.tv_sec = 0;
		epow_trb->timeout.it_value.tv_nsec = 50000000;
		tstart(epow_trb);
		return;
	}
#endif
	if (t->func_data == FROM_FLIH) {
		/*
		 * We must still be alive
		 */
		if ( epow_status != RESUME ) {

			/*
			 * If an immediate power down condition is in effect,
			 * then we'd better not resume.
			 */
			if ((behavior == IMMED_SHUTDOWN) ||
			    (behavior == IMMEDX_SHUTDOWN))
				return;

#ifdef _POWER_MP
			/* get the process level of EPOW
			 * and lock the poll array 
			 */
			plvl = i_genplvl( &epowhand );
#endif /* _POWER_MP */

			/*  
			 *  We were suspended, we can resume. 
			 * Traverse list of EPOW handlers.
			 */
			handler = epowhandler->next;
			while (handler != (struct intr *)NULL) {
				handler->flags |= EPOW_RESUME;
				handler->flags &= ~(EPOW_BATTERY |EPOW_SUSPEND);
				handler = handler->next;
			}
			epow_status = RESUME;
					
			handler = epowhandler->next;
			while (handler != (struct intr *)NULL) {
				(void)(*handler->handler) (handler);
				/*
				 * This allows any pending EPOW interrupts
				 * in.
				 */
				i_enable(INTCLASS0);
				ipri = i_disable(INTEPOW);

				if ( epow_status != RESUME )
					/* We have had another EPOW so
					 * stop resuming
					 */
					return;
				handler->flags &= ~( EPOW_RESUME |
						EPOW_BATTERY | EPOW_SUSPEND);
				handler = handler->next;
			}

			switch (behavior) {

			case FAST_SHUTDOWN:
				/*
				 * this was a Power Supply Fan Fault or
				 * a Thermal warning, start
				 * a timer for 20 seconds to allow the system
				 * a little more time to get things to disk.
				 */
				/*
				 * Make sure Epow timer isn't already running
				 * (shouldn't be possible).
				 */
				if (!(epow_trb->flags & T_ACTIVE)) {
					/*
					 * Start Epow timer for 20 seconds
					 * set func data to reflect
					 * we started the timer from the timer
					 */
			        	epow_trb->func_data = (ulong)FROM_TRB;
					epow_trb->timeout.it_value.tv_sec = 20;
					epow_trb->timeout.it_value.tv_nsec = 0;
					tstart(epow_trb);
				}
				/*
				 * signal init to take action
				 */
				pidsig(1, SIGPWR);
				break;

			case SLOW_SHUTDOWN:
				/*
				 * we still have about 10 minutes before loss
				 * of power, start a timer for 10 minutes to
				 * allow for a clean shutdown of the system.
				 */
				/*
				 * Make sure Epow timer isn't already running
				 * (shouldn't be possible).
				 */
				if (!(epow_trb->flags & T_ACTIVE)) {
					/*
					 * Start Epow timer for 10 minutes
					 * set func data to reflect
					 * we started the timer from the timer
					 */
					epow_trb->func_data = (ulong)FROM_TRB;
					epow_trb->timeout.it_value.tv_sec = 600;
					epow_trb->timeout.it_value.tv_nsec = 0;
					tstart(epow_trb);
				}
				/*
				 * signal init to take action
				 */
				pidsig(1, SIGPWR);
				break;

			case USE_EPOW_BITS:
				if (power_status == NORMAL_STATE) {
					/* 
					 * Must have been a false EPOW
					 * return to normal state
					 */
					resume_log.psr = power_status;
					errsave(&resume_log,sizeof(resume_log));
					break;
				}

			default:
				/*
				 * Must have been a secondary fan fault, 
				 * switch to battery backup, or an undefined 
				 * power status.  signal init to take action.
				 */
				pidsig(1, SIGPWR);
				break;

			}
		}  /* if (epow_suspend) ....else we were already resumed */
	} else {
		/*
		 * We were started from the Timer, which means we
		 * previously detected a power supply fan fault.
		 * See if the condition still exists, and if so
		 * suspend all registered handlers.
		 */
		if ((behavior == FAST_SHUTDOWN) ||
		    (behavior == SLOW_SHUTDOWN)) {
			/*
			 * Yip, still a problem. 
			 * Make sure we weren't already suspended
			 */
			if (epow_status != SUSPEND) {
				/*
				 * Suspend handlers
				 */
				handler = epowhandler->next;
				while ( handler != (struct intr *)NULL ) {
					handler->flags |= EPOW_SUSPEND;
					(void)(*handler->handler) (handler);
					handler = handler->next;
				}
				epow_status = SUSPEND;
			}
		}
	}

}


/*
 * NAME: io_exception
 *
 * FUNCTION: called by ds_flih when a DSI occurs in iospace.
 *
 * EXECUTION ENVIORMENT:
 *	Called from ds_flih with interrupt disabled
 *
 * RETURNS: None
 */

void
io_exception(
	ulong dar,			/* faulting address */
	ulong srval,			/* faulting segment reg. value */
	struct mstsave *mst,		/* faulting mst */
	ulong dsisr)			/* machine dsisr register value */
{
	volatile ulong *csr15;
	volatile ulong *dsirr_ptr;
        struct ipl_cb   *iplcb_ptr;             /* ros ipl cb pointer */
        struct buc_info *buc_ptr;               /* BUC info pointer */
	struct b_protect *b_prt;
	ulong ioccaddr;
	int buid;
	int num_bucs,buc_cnt;
	int dsirr;
	ulong csr_val;
	int valid_buid;
	int iocc_dev=FALSE;
	int except;
        struct ppda     *ppda_ptr;      /* PPC interrupt registers */
        volatile struct ppcint *intr;
	extern void check_bus_timeout();

	assert(!__rspc());

	buid = BID_TO_BUID(srval);

#ifdef _RS6K
	if (__rs6k()) {
		/*
		 * For PowerPC there isn't a Data Storage Interrupt Reason
		 * Register....
		 */
		dsirr = 0;
	        /* get addressability to iplinfo structure
	         */
		iplcb_ptr = (struct ipl_cb *) vmker.iplcbptr;
	        buc_ptr = (struct buc_info *)((uint)iplcb_ptr +
                                        iplcb_ptr->s0.buc_info_offset);
	        num_bucs = buc_ptr->num_of_structs;
	        for(buc_cnt=0;buc_cnt < num_bucs;buc_cnt++) {
	                /*
	                 * Find the BUC that had the exception
	                 */
	                if (buc_ptr->buid_data[0].buid_value == buid) {
				/*
				 * Make sure this is an IOCC
				 */
				if (buc_ptr->IOCC_flag)
					iocc_dev = TRUE;
				break;
			}
		
			/*
			 * Point to the next BUC
			 */
			buc_ptr = (struct buc_info *)((uint)buc_ptr + 
							buc_ptr->struct_size);
		}
	}
#endif /* _RS6K */

#ifdef _POWER_RS
	if (__power_rs()) {
		if (__power_rsc()) {
			/* the data storage interrupt reason 
			 * register can't be read on RSC.
			 */
			dsirr = 0;
			valid_buid = (buid == IOCC0_BUID || buid == SGA_BUID ||
					buid == SCU_BUID);
			check_bus_timeout(dar, srval, mst);
		} else {
			/* get data storage interrupt reason register
			 */
			dsirr_ptr = (ulong *)io_att(SCU_BID, DSIRR);
			dsirr = *dsirr_ptr;
			io_det(dsirr_ptr);
			valid_buid = !(dsirr & DSIRR_IB);
		}
	}
#endif /* _POWER_RS */

	/* store off error logging information:
	 */
	mst->except[0] = 0;
	mst->except[1] = dsisr;
	mst->except[2] = srval;
	mst->except[3] = dar;
	mst->except[4] = dsirr;
	mst->o_vaddr   = dar;

#ifdef _POWER_RS
	if (__power_rs()) {
		/*
		 * For the POWER platform, we know the hardcoded IOCC
		 * Bus IDs, and that exception status is available in
		 * channel status register 15 
		 */
		if (valid_buid && buid >= IOCC0_BUID && buid <= IOCC3_BUID) {
			/*  check that the exception was in iocc space
			 *  Setup access to the IOCC.
	 		 */
			ioccaddr = (ulong) io_att(IOCC_HANDLE(srval), 0);
	
			/* Get value of channel status register 15
			 */
			csr15 = CSR_EFF_ADDR(15);
			csr_val = *csr15;
			RESET_STATUS(csr15);
			mst->except[0] = csr_val;
			io_det(ioccaddr);
	
 			/* Re-establish access to displays controlled by CSR15
 			 * for a graphics thread.  
 			 * Make sure csr15 is set up correctly (even when this
 			 * thread does not currently own any display on the bus)
 			 */	
 			RESTORE_CSR15(buid);
		}

		/* get exception code to pas to p_slih
		 */
		switch(buid) {
			case IOCC0_BUID:
			case IOCC1_BUID:
			case IOCC2_BUID:
			case IOCC3_BUID:
				except = EXCEPT_IO;
				break;
			case MSLA0_BUID:
			case MSLA1_BUID:
			case MSLA2_BUID:
			case MSLA3_BUID:
			case MSLA4_BUID:
			case MSLA5_BUID:
			case MSLA6_BUID:
			case MSLA7_BUID:
				except = EXCEPT_IO_SLA;
				break;
			case SGA_BUID:
				except = EXCEPT_IO_SGA;
				break;
			default:
				except = EXCEPT_IO_SCU;
		}
	} 
#endif /* _POWER_RS */

#ifdef _RS6K
	if (__rs6k()) {
		/*
		 * For PowerPC, we know that only T=1 BUCs will cause
		 * direct store error interrupts, which are only IOCC type
		 * devices.  We also know that the exception status is
		 * available in the DSEIR for this processor.
		 */
		if (iocc_dev) {
			/*
			 * If a valid IOCC....
			 */
			/*
			 * !!!Determine which processor we're on and
			 * read the appropriate DSIER 
			 */
	                ppda_ptr = PPDA;
			intr = (volatile struct ppcint *)(ppda_ptr->intr);
			/*
			 * note that we make the exception status look
			 * like the old CSR 15 for device driver compatibility
			 */
			mst->except[0] = ((intr->dsier << 12) & 0xF0000000);
			except = EXCEPT_IO;
		} else {
			except = EXCEPT_IO_SCU;
		}
	}
#endif /* _RS6K */


	ASSERT(CSA != mst);

	if (mst->intpri == INTBASE) {
		/*
		 * if the exception was originated at INTBASE (graphics app),
		 * then run the exception handling at INTPAGER.  Otherwise,
		 * allow the exception handling to run at INTMAX.  NOTE: This
		 * will require the acquisition of a spin-lock for MP systems.
		 */
		i_enable(INTPAGER);
	}

	/* Call p_slih to handle the exception 
	 */
#ifndef _THREADS
	p_slih(mst, dar, except, curproc);
#else /* _THREADS */
	p_slih(mst, dar, except, curthread);
#endif
}

/*
 * NAME:  get_pksr()
 *
 * FUNCTION:  
 *	gets power status / keylock register value.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called in an interrupt context.
 *
 * NOTES:  
 *
 * RETURN VALUE DESCRIPTION:
 *	uint
 */

uint
get_pksr()
{
	uint pksr;
	volatile ulong *pow_stat_addr;
#ifdef _RS6K_SMP_MCA
        int intpri = INTMAX;
        cpu_t save_cpu;
#endif /* _RS6K_SMP_MCA */

#ifdef _POWER_RS
	if (__power_rs()) {
		/* Address of Power/Status register. */
		pow_stat_addr = (ulong *)io_att(IOCC_HANDLE(IOCC_BID),
					     PSR_ADDRESS);
		pksr = *pow_stat_addr;
		io_det(pow_stat_addr);
	}
#endif /* _POWER_RS */

#ifdef _RS6K
	if (__rs6k())
	{
#ifdef _RS6K_SMP_MCA
		pksr = *pksr_addr;
		if (pgs_SSGA_lvl == 2) {
			if (CSA->intpri == INTBASE) {
				save_cpu = CURTHREAD->t_cpuid;
				switch_cpu (MP_MASTER, SET_PROCESSOR_ID);
				intpri = i_disable (DISKETTE_INT_LVL);
			}

			if (fd_mutex)
				d_abort_fd();
		}

		if ( (__rs6k_smp_mca()) &&
		    (~(((struct pgs_sys_spec*)
		       (sys_resource_ptr->sys_specific_regs))->iod_hw_sts) & 
		     (EPOW_MCA_CAB | EPOW_MAIN_CAB)) )
		  pksr |= LOPP;

		if (intpri == INTBASE) {
			i_enable(intpri);
			switch_cpu (save_cpu,RESET_PROCESSOR_ID);
		}

#else 
		pksr = sys_resource_ptr->sys_regs.pwr_key_status;
#endif /* _RS6K_SMP_MCA */
	}
#endif /* _RS6K */

#ifdef _RSPC
	if (__rspc())
	{
		pksr = fw_keycase;
	}
#endif

	
	return(pksr);

} /* get_pksr() */


