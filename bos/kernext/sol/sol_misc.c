static char sccsid[] = "@(#)43	1.6  src/bos/kernext/sol/sol_misc.c, sysxsol, bos411, 9428A410j 12/6/91 10:07:09";
/*
 * COMPONENT_NAME: (SYSXSOL) Serial Optical Link Device Handler
 *
 * FUNCTIONS:  startup_chip, stop_sla, sol_report_status, sol_cio_get_stat,
 *	       sol_select, sol_cdt_func, sol_trace
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/dma.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/ioacc.h>
#include <sys/m_except.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/soluser.h>
#include "soldd.h"
#include "sol_extrn.h"

extern struct sol_ddi	sol_ddi;

/*
 * NAME: sol_xmit_wait
 *
 * FUNCTION: Sleeps until all transmits are complete.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called at close time from the process environment, 
 *	but it disables interrupts and can therefore not page fault.
 *
 * (NOTES:) 
 *
 * RECOVERY OPERATION:  None.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	Nothing
 */
void
sol_xmit_wait(
struct sol_open_struct	*open_ptr
)
{
	int oldpri;

	/*
	 *  Disable interrupts and sleep as long as there are xmits
	 *  outstanding.
	 */
	oldpri = i_disable(SOL_SLIH_LEVEL);
	while (open_ptr->xmit_count != 0) {
		(void) e_sleep(&open_ptr->close_event, EVENT_SHORT);
	}
	i_enable(oldpri);
}

/*
 * NAME: startup_chip
 *
 * FUNCTION:
 *	Issues the start command to the SLA
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from several diagnostic ioctl functions (process
 *	environment).  Interrupts must be disabled, so the routine can not
 *	page fault.
 *
 * NOTES:
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	-1			- unexpected error encountered
 *	SOL_SUCCESS		- successful completion                        
 *	SOL_CANT_STOP_SLA	- couldn't bring SLA to STOPPED state
 *	SOL_SLA_IO_EXCEPTION	- SLA I/O Exception encountered
 *	SOL_SLA_ALREADY_STARTED	- attempt to start SLA already in WORKING state
 *	SOL_TIMEOUT		- software timeout encountered
 */

int startup_chip(
struct slaregs volatile		*sla_ptr,
struct sol_sla_status		*status_struct,
unsigned int			page_raddr,
int				sla_num
)

{
	uint				rc, temp;
	int				processor_priority;

	processor_priority = i_disable(IMCS_INT_PRIORITY);  /* mask ints */
	rc = SOL_SUCCESS;

	switch (PUISTATE(temp = sla_ptr->sample)) {
		case PUI_NOOP:
			/* trigger sla from non operational to stopped state */
			BUS_GETLX((long *) &sla_ptr->ch_op, &temp);
			if (PUISTATE(temp) != PUI_STOPPED) {
				rc = SOL_CANT_STOP_SLA;
				break;
			}
		case PUI_STOPPED:
			/* move xmt regs, ccr, and tag table into hardware */
			rc |= BUS_PUTLX((long *)&sla_ptr->thr.thr[0],
				(long)status_struct->thr_word0);
			rc |= BUS_PUTLX((long *)&sla_ptr->thr.thr[1],
				(long)status_struct->thr_word1);
			rc |= BUS_PUTLX((long *)&sla_ptr->thr.thr[2],
				(long)status_struct->thr_word2);
			rc |= BUS_PUTLX((long *)&sla_ptr->thr.thr[3],
				(long)status_struct->thr_word3);
			rc |= BUS_PUTLX((long *)&sla_ptr->thr.thr[4],
				(long)status_struct->thr_word4);
			rc |= BUS_PUTLX((long *)&sla_ptr->ccr,
				(long)status_struct->ccr);
			if (rc) {
				ASSERT(rc == EXCEPT_IO_SLA);
				rc = SOL_SLA_IO_EXCEPTION;
				break;
			}
			if (page_raddr != 0) {
				rc |= BUS_PUTLX((long *)&sla_ptr->tcw[0],
					(long)page_raddr);
				rc |= BUS_PUTLX((long *)&sla_ptr->tcw[1],
					(long)LAST_TAG);
				if (rc) {
					ASSERT(rc == EXCEPT_IO_SLA);
					rc = SOL_SLA_IO_EXCEPTION;
					break;
				}

			} else {
				rc = BUS_PUTLX((long *)&sla_ptr->tcw[0],
					(long)LAST_TAG);
				if (rc) {
					ASSERT(rc == EXCEPT_IO_SLA);
					rc = SOL_SLA_IO_EXCEPTION;
					break;
				}
			}

			/* read status2 before starting */
			status_struct->status_2 = sla_ptr->status2;

			/* trigger sla from stopped to running state */
			BUS_GETLX((long *) &sla_ptr->ch_start, 
			&status_struct->status_1);
			break;
		case PUI_WORK1:
			rc = SOL_SLA_ALREADY_STARTED;
			break;
		default:
			rc = -1;
	}

	if (rc == 0) {
		switch (imcs_diag_sleep(sla_num)) {
			case EVENT_SUCC:
				status_struct->status_1 =
					sla_tbl.sla[sla_num].s1_save;
				status_struct->status_2 =
					sla_tbl.sla[sla_num].s2_save;
				sla_tbl.sla[sla_num].s1_save = 0;
				sla_tbl.sla[sla_num].s2_save = 0;
				rc = SOL_SUCCESS;
				break;
			case EVENT_SIG:
				/* e-sleep terminated by signal */
				/* sla running so DD sleeping */
				rc = -1;
				break;
			case 3:
				/* process already sleeping */
				rc = -1;
				break;
			case 4:
				/* wakeup is from software timeout */
				sla_tbl.sla[sla_num].s1_save = 0;
				sla_tbl.sla[sla_num].s2_save = 0;
				stop_sla(sla_ptr, status_struct);
				rc = SOL_TIMEOUT;
				break;
			default:
				rc = -1;
		}
	}

	i_enable(processor_priority);   /* restore ints */
	return rc;
}

/*
 * NAME: stop_sla
 *
 * FUNCTION:
 *	Issues command to bring SLA to STOPPED state
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called from startup_chip() and sol_buffer_access()
 *	(process environment).  Interrupts must be disabled, so it can
 *	not page fault.
 *
 * NOTES:
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	-1	- could not bring SLA to stopped state
 */

int
stop_sla(
struct slaregs volatile		*sla_ptr,
struct sol_sla_status		*status_struct
)

{
        int rc;

	status_struct->status_1 = sla_ptr->sample;
	rc = 0;

	switch (PUISTATE(status_struct->status_1)) {
		case PUI_NOOP:
			BUS_GETLX((long *) &sla_ptr->ch_op, 
			    &status_struct->status_1);
			if (PUISTATE(status_struct->status_1) != PUI_STOPPED) {
				rc = -1;
			} else {
				status_struct->status_2 = sla_ptr->status2;
			}
			break;
		case PUI_STOPPED:
			rc = 0;
			status_struct->status_2 = sla_ptr->status2;
			status_struct->status_1 = sla_ptr->status1;
			break;
		case PUI_WORK1:
			BUS_GETLX((long *) &sla_ptr->ch_stop, 
			    &status_struct->status_1);
			if (PUISTATE(status_struct->status_1) != PUI_STOPPED) {
				rc = -1;
			} else {
				status_struct->status_2 = sla_ptr->status2;
			}
			break;
		case PUI_WORK2:
			BUS_GETLX((long *) &sla_ptr->ch_stop, 
			    &status_struct->status_1);
			if (PUISTATE(status_struct->status_1) != PUI_STOPPED) {
				rc = -1;
			} else {
				status_struct->status_2 = sla_ptr->status2;
			}
			break;
		default:
			rc = -1;
	}
	return rc;
}
/*
 * NAME: sol_report_status
 *
 * FUNCTION: Sends async status to kernel user, or puts a status block
 *	on the queue for user-mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine can be called from either the process thread or the
 *	interrupt thread, and can not page fault.
 *
 * (NOTES:) 
 *
 * RECOVERY OPERATION:  None.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	Nothing
 */
void
sol_report_status(
struct sol_open_struct	*open_ptr,	/* ptr to open structure	*/
cio_stat_blk_t		*stat_blk_ptr	/* ptr to status block		*/
)

{
	int oldpri;

	SYS_SOL_TRACE("MrsB", stat_blk_ptr->code, stat_blk_ptr->option[0],
	    open_ptr);
	if (open_ptr->devflag & DKERNEL) {
		/*
		 *  For kernel-mode, just call the stat_fn.
		 */
		(*(open_ptr->stat_fn)) (open_ptr->open_id, stat_blk_ptr);
	} else {
		/*
		 *  For user-mode, add this to the status queue.
		 */
		oldpri = i_disable(SOL_SLIH_LEVEL);
		if (open_ptr->stat_que.head ==
		    open_ptr->stat_que.tail->next) {
			open_ptr->stat_full = TRUE;
			/*
			 *  Update statistics
			 */
			sol_ddi.stats.ds.sta_que_overflow++;
		} else {
			/*
			 *  Update statistics
			 */
			open_ptr->stat_count++;
			if (open_ptr->stat_count >
			    sol_ddi.stats.cc.sta_que_high) {
				sol_ddi.stats.cc.sta_que_high++;
			}
			if (open_ptr->stat_que.head == NULL) {
				open_ptr->stat_que.head =
				    open_ptr->stat_que.tail;
			} else {
				open_ptr->stat_que.tail =
				    open_ptr->stat_que.tail->next;
			}
			open_ptr->stat_que.tail->stat_blk = *stat_blk_ptr;
			SYS_SOL_TRACE("Mrsq", stat_blk_ptr->code,
			    stat_blk_ptr->option[0], open_ptr);
			if (open_ptr->select_req & POLLPRI) {
				open_ptr->select_req &= ~POLLPRI;
				selnotify(sol_ddi.devno,
				    open_ptr->chan, POLLPRI);
				SYS_SOL_TRACE("Mrsn", stat_blk_ptr->code,
				    stat_blk_ptr->option[0], open_ptr);
			}
		}
		i_enable(oldpri);
	}
	return;
}

/*
 * NAME: sol_cio_get_stat
 *
 * FUNCTION: Provides information from the device handler.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called from sol_ioctl (process environment), but
 *	interrupts must be disabled, so it can not page fault.
 *
 * NOTES:
 *	This routine can only be called by a user-mode caller.
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	EACCES	- attempted ioctl from kernel-mode
 *	EFAULT	- invalid address specified
 */

int
sol_cio_get_stat(
int	arg,			/* address of solioctl parameter block	*/
ulong	devflag,		/* kernel or user mode			*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{
	struct status_block	stat_blk;
	struct sol_sta_que_elem	*head_ptr;
	int			rc, oldpri;

	SYS_SOL_TRACE("PgsB", sol_ddi.open_ptr[chan], chan, 0);
	if (devflag & DKERNEL) {
		return EACCES;
	}
	if (sol_ddi.open_ptr[chan]->stat_que.head == NULL) {
		stat_blk.code = CIO_NULL_BLK;
	} else {
		/*
		 *  Get the first status block, and move the queue pointers.
		 */
		oldpri = i_disable(SOL_SLIH_LEVEL);
		head_ptr = sol_ddi.open_ptr[chan]->stat_que.head;
		if (head_ptr == sol_ddi.open_ptr[chan]->stat_que.tail) {
			sol_ddi.open_ptr[chan]->stat_que.head = NULL;
		} else {
			sol_ddi.open_ptr[chan]->stat_que.head = head_ptr->next;
		}
		bcopy(head_ptr, &stat_blk, sizeof(cio_stat_blk_t));
		SYS_SOL_TRACE("Pgsc", head_ptr->stat_blk.code,
		    head_ptr->stat_blk.option[0], sol_ddi.open_ptr[chan]);
		/*
		 *  If the status queue overflowed, use this status block
		 *  to report the overflow.
		 */
		if (sol_ddi.open_ptr[chan]->stat_full) {
			sol_ddi.open_ptr[chan]->stat_full = FALSE;
			head_ptr->stat_blk.code = CIO_LOST_STATUS;
			sol_ddi.open_ptr[chan]->stat_que.tail =
				sol_ddi.open_ptr[chan]->stat_que.tail->next;
		} else {
			/*
			 *  Update statistics
			 */
			sol_ddi.open_ptr[chan]->stat_count--;
		}
		i_enable(oldpri);
	}
	rc = copyout(&stat_blk, arg, sizeof(cio_stat_blk_t));
	return rc;
}

/*
 * NAME: sol_select
 *
 * FUNCTION: Determines if the specified event has occured.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called when the select or poll subroutine is used.
 *      It is called from the process environment, but interrupts must
 *	be disabled, so it can not page fault.
 *
 * NOTES: 
 *	This routine should only be called by a user mode caller.
 *	Possible event flags:
 *	POLLIN		- receive data available
 *	POLLOUT		- transmit available
 *	POLLPRI		- status available
 *	POLLSYNC	- do not send asynchronous notification
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  
 *	0	- successful completion                        
 *	ENODEV	- devno is invalid
 *	EACCES	- invalid call from a kernel user
 */

int
sol_select(
dev_t	devno,			/* major/minor device number		*/
ushort	events,			/* conditions to be checked		*/
ushort	*reventp,		/* results of condition checks		*/
chan_t	chan)			/* channel allocated by sol_mpx		*/
{

	struct sol_open_struct	*open_ptr;
	int			old_pri;

	SYS_SOL_TRACE("PslB", events, sol_ddi.open_ptr[chan]->select_req,
	    sol_ddi.open_ptr[chan]);
	(void) lockl(&(sol_ddi.global_lock), LOCK_SHORT);
	open_ptr = sol_ddi.open_ptr[chan];
	if ((minor(devno) != SOL_OPS_MINOR) || (open_ptr == NULL)) {
		unlockl(&(sol_ddi.global_lock));
		return ENODEV;
	}
	if (open_ptr->devflag & DKERNEL) {
		unlockl(&(sol_ddi.global_lock));
		return EACCES;
	}
	*reventp = 0;
	if ((events & ~POLLSYNC) == 0) { /* no events requested */
		unlockl(&(sol_ddi.global_lock));
		return 0;
	}
	if (events & POLLOUT) {
		/*
		 *  Transmit is never blocked.
		 */
		*reventp |= POLLOUT;
	}
	/*
	 *  Interrupts must be disabled at this point to prevent losing
	 *  status blocks or receive data.  For example if the status
	 *  block arrived between the time the stat_que.head ptr is
	 *  checked and the time the select_req field is set, the
	 *  status block would be lost.
	 */
	old_pri = i_disable(SOL_OFF_LEVEL);
	if ((events & POLLIN) && (open_ptr->recv_que.head != NULL)) {
		/*
		 *  Receive data is available if the head is not NULL.
		 */
		*reventp |= POLLIN;
	}
	if ((events & POLLPRI) && (open_ptr->stat_que.head != NULL)) {
		/*
		 *  Status is available if the head is not NULL.
		 */
		*reventp |= POLLPRI;
	}
	if ((*reventp == 0) && (!(events & POLLSYNC))) {
		/*
		 *  If the user did not request synchronous notification only,
		 *  Then save these events so selnotify can be called later.
		 */
		open_ptr->select_req |= events;
	}
	SYS_SOL_TRACE("PslE", *reventp, sol_ddi.open_ptr[chan]->select_req,
	    sol_ddi.open_ptr[chan]);
	i_enable(old_pri);
	unlockl(&(sol_ddi.global_lock));
	return 0;
}

/*
 * NAME: sol_cdt_func
 *
 * FUNCTION: Fills in the component dump table and returns the address.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      It is called from the interrupt environment, and can not page fault.
 *
 * NOTES: 
 *
 * RECOVERY OPERATION: If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:  Address of the component dump table structure.
 */

struct cdt *sol_cdt_func(
int	arg)
{
	struct cdt_entry	*entry_ptr;
	int			i;

	if (arg == 1) {
		/*  First dump entry is sol_ddi structure */
		entry_ptr = &sol_ddi.cdt->cdt_entry[0];
		bcopy("sol_ddi", entry_ptr->d_name, 8);
		entry_ptr->d_len = sizeof(struct sol_ddi);
		entry_ptr->d_ptr = (caddr_t) &sol_ddi;
		entry_ptr->d_segval = (int) NULL;

		/* Next SOL_TOTAL_OPENS dump entries are sol_open structures */
		for (i=0 ; i < SOL_TOTAL_OPENS ; i++) {
			entry_ptr = &sol_ddi.cdt->cdt_entry[i+1];
			bcopy("sol_opn", entry_ptr->d_name, 8);
			if (sol_ddi.open_ptr[i] == NULL) {
				entry_ptr->d_len = 0;
			} else {
				entry_ptr->d_len =
				    sizeof(struct sol_open_struct);
			}
			entry_ptr->d_ptr = (caddr_t) sol_ddi.open_ptr[i];
			entry_ptr->d_segval = (int) NULL;
		}

		/*  Next dump entry is irq_tbl structure */
		entry_ptr = &sol_ddi.cdt->cdt_entry[SOL_TOTAL_OPENS+1];
		bcopy("irq_tbl", entry_ptr->d_name, 8);
		entry_ptr->d_len = sizeof(struct irq_tbl);
		entry_ptr->d_ptr = (caddr_t) &irq_tbl;
		entry_ptr->d_segval = (int) NULL;

		/*  Next dump entry is cddq structure */
		entry_ptr = &sol_ddi.cdt->cdt_entry[SOL_TOTAL_OPENS+2];
		bcopy("cddq", entry_ptr->d_name, 5);
		entry_ptr->d_len = sizeof(struct cddq);
		entry_ptr->d_ptr = (caddr_t) &cddq;
		entry_ptr->d_segval = (int) NULL;

		/*  Next dump entry is sla_tbl structure */
		entry_ptr = &sol_ddi.cdt->cdt_entry[SOL_TOTAL_OPENS+3];
		bcopy("sla_tbl", entry_ptr->d_name, 8);
		entry_ptr->d_len = sizeof(struct sla_tbl);
		entry_ptr->d_ptr = (caddr_t) &sla_tbl;
		entry_ptr->d_segval = (int) NULL;

		/*  Next dump entry is isq structure */
		entry_ptr = &sol_ddi.cdt->cdt_entry[SOL_TOTAL_OPENS+4];
		bcopy("isq", entry_ptr->d_name, 4);
		entry_ptr->d_len = sizeof(struct isq);
		entry_ptr->d_ptr = (caddr_t) &isq;
		entry_ptr->d_segval = (int) NULL;

		/*  Next dump entry is imcs_addresses structure */
		entry_ptr = &sol_ddi.cdt->cdt_entry[SOL_TOTAL_OPENS+5];
		bcopy("imcs_ad", entry_ptr->d_name, 8);
		entry_ptr->d_len = sizeof(struct imcs_addresses);
		entry_ptr->d_ptr = (caddr_t) &imcs_addresses;
		entry_ptr->d_segval = (int) NULL;

		/*  Next dump entry is imcsdata structure */
		entry_ptr = &sol_ddi.cdt->cdt_entry[SOL_TOTAL_OPENS+6];
		bcopy("imcsdat", entry_ptr->d_name, 8);
		entry_ptr->d_len = sizeof(struct imcsdata);
		entry_ptr->d_ptr = (caddr_t) &imcsdata;
		entry_ptr->d_segval = (int) NULL;
	}
		
	return sol_ddi.cdt;
}


#ifdef DEBUG 
/*
 * NAME: sol_trace
 *
 * FUNCTION:
 *      This routine puts a trace entry into the internal device
 *      driver trace table.  It also calls the AIX trace service.
 *
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *
 * NOTES:
 *      Each trace point is made up of 4 words (ulong).  Each entry
 *      is as follows:
 *
 *              | Footprint | data | data | data |
 *
 *      The 4 byte Footprint is contructed as follows:
 *
 *              byte 1 = Type of operation
 *              byte 2&3 = function identifier
 *              byte 4 = location identifier
 *
 *      where byte 1 type of operation may be:
 *
 *              "O"     - open
 *              "C"     - close
 *              "M"     - mpx
 *              "T"     - transmit
 *              "R"     - receive
 *              "N"     - Network Recovery Mode
 *              "A"     - Adapter activation/de-activation
 *              "D"     - diagnostic
 *              "P"     - primitive routine
 *              "I"     - ioctl
 *              "c"     - configuration
 *              "o"     - oflv routines
 *              "S"     - SLIH routine
 *              "t"     - timer routines
 *
 *      where bytes 2&3 are two characters of the actual function name:
 *
 *      where byte 4 location identifier may be:
 *
 *
 *              "B"     - beginning
 *              "E"     - end
 *              "C"     - trace point continuation
 *              "1-9"   - function trace point number
 *              "a-z"     may be the characters 1 thru 9 or
 *                        lower case a thru z
 *
 *
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES:
 *      Modifies the global static SOL trace table.
 *
 * RETURNS:  none
 */ 
void
sol_trace (     register char	*str,  /* trace data Footprint */
                register uint	arg2,   /* trace data */
                register uint	arg3,   /* trace data */
                register uint	arg4,  /* trace data */
                register int	sysflg)   /* set if system trace this call */

{
        register int    i;

        /*
         * get the current trace point index
         */
        i= sol_ddi.soltrace.next;

        /*
         * copy the trace point data into the global trace table.
         */
        sol_ddi.soltrace.table[i] = *(ulong *)str;
        ++i;
        sol_ddi.soltrace.table[i] = arg2;

        ++i;
        sol_ddi.soltrace.table[i] = arg3;
        ++i;
        sol_ddi.soltrace.table[i] = arg4;
        ++i;


        if ( i < SOL_TRACE_SIZE )
        {
                sol_ddi.soltrace.table[i] = 0x21212121; /* end delimeter */
                sol_ddi.soltrace.next = i;
        }
        else
        {
                sol_ddi.soltrace.table[0] = 0x21212121; /* end delimeter */
                sol_ddi.soltrace.next = 0;
        }

        /*  Make the AIX trace point call */

        if(sysflg)
        TRCHKGT(HKWD_DD_SOL | HKTY_GT | 0, *(ulong *)str, arg2,
                        arg3, arg4, 0);

        return;
} /* end sol_trace */
#endif 

