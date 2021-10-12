static char sccsid[] = "@(#)75	1.14.1.4  src/bos/kernel/io/machdd/pgs_bump_pin.c, machdd, bos41J, bai15 4/11/95 14:42:47";
/*
 * COMPONENT_NAME:  (MACHDD) Machine Device Driver
 *
 * FUNCTIONS:  PEGASUS BUMP interface
 *
 *	mdbumpcmd_bwait:	Isues a BUMP request and "busy waits" for the
 *					response
 *	mdbumpcmd:		Makes a transaction on BUMP
 *	mdbumpepow:		Redirection of BUMP epow interrupts to the
 *					normal EPOW handler
 *	mdbumpintr:		BUMP off-level interrupt handler
 *	mdkeyconnect:		Connects current process to key BUMP interrupt
 *	mdkeydisconnect:	Disconnects current process from key BUMP
 *					interrupt
 *	pgs_rtc_start:		Starts simultaneously the RTC or TB of all the
 *					processors
 *	pgs_rtc_stop:		Stops simultaneously the RTC or TB of all the
 *					processors
 *	mdreboot:		PEGASUS soft IPL
 *
 * ORIGINS: 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#ifdef _RS6K_SMP_MCA

#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/intr.h>
#include <sys/sleep.h>
#include <sys/lock_def.h>
#include <sys/sys_resource.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/errids.h>
#include <sys/err_rec.h>
#include <sys/systemcfg.h>
#include <sys/mpc.h>
#include <sys/iocc.h>

#include "pgs_novram.h"
#include "pgs_bump.h"
#include "pegasus.h"
#include "intr_hw.h"


int pgs_tpevent = EVENT_NULL;	/* event list used for synchronization */
Simple_lock pgs_bump_lock;	/* lock for serializing
				   BUMP accesses with BUMP interrupt handler */
mpc_msg_t mdbumpepow_mpc;	/* mpc to redirect BUMP epow interrupts
				   to normal EPOW handler */
struct  epowerr warning_log = { ERRID_EPOW_SUS,"SYSIOS ", 0 };
extern struct intr *epowhandler;/* epow handlers list */

volatile struct tp_fifo_header *pgs_pfifo;
uchar bump_cur_tid = 0;
pgs_bump_msg_t pgs_last_status;	/* A response for a waiting thread is stored */
				/*	temporarily here */
pgs_bump_msg_t *pgs_fifo_cmd, *pgs_fifo_stat;
struct intr mdd_intr;
uchar pgs_bump_init_done = 0;

pid_t	md_key_int_pid = 0;	/* pid of process connected to key switch BUMP interrupt */
int	md_key_int_signo;	/* signal no sent upon key switch BUMP interrupt */

int pgs_Flash_lvl  = 0;		/* WARNING: these variables are referenced  */
int pgs_IPL_lvl    = 0;		/* 	before vmm init, so they MUST NOT   */
int pgs_SSGA_lvl   = 0;		/*	reside in BSS			    */
int pgs_IONIAN_lvl = 0;		/*	reside in BSS			    */
int pgs_CCA2_lvl   = 0;		/*					    */
int pgs_601_lvl    = 0;		/*					    */
int pgs_DCB_lvl    = 0;		/*					    */
int is_a_Junior    = 0;		/*					    */
int OldParityError = 0;		/*	Temporary error counter		    */

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 */
int intbump	   = INTOFFL0;	/* For BUMP serialization		    */
#define DISKETTE_INT_LVL	INTCLASS3
extern int fd_mutex;
extern void d_abort_fd();

struct errors errors = {	/* Memory parity error log recod */
	ERRID_PEGA_PARITY_ERR,
	"sysplanar0"
};


/*
 * NAME: mdbumpcmd_bwait
 *
 * FUNCTION:
 *	Isue a BUMP request and "busy wait" for the response
 *
 * EXECUTION ENVIRONMENT:
 *	Should be called at INTMAX.
 *
 * NOTES:
 * 	Driver is mp-safe:
 *		accesses to BUMP are serialized with BUMP interrupt handler 
 *		using "disable_lock(INTBUMP, &pgs_bump_lock)"
 *	While waiting for the BUMP response to this request, all
 *		other BUMP messages are thrown away
 *
 * RETURN VALUE:
 *	None.
 */

void
mdbumpcmd_bwait(
	pgs_bump_msg_t *cmd,		/* Pointer to command buffer */
	pgs_bump_msg_t *stat)		/* Pointer to status buffer */
{
	register pgs_bump_msg_t *stp;
	int ipri, i;

	ipri = disable_lock(INTBUMP, &pgs_bump_lock);

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 */
        if (pgs_SSGA_lvl == 2 && fd_mutex)
                d_abort_fd();

	/*
	 * cmd fifo should never be full, since accesses to machdd are
	 * serialized
	 */
	assert((pgs_pfifo->command_in + 1) % pgs_pfifo->length !=
						pgs_pfifo->command_out);
	cmd->cpu_portid = CPU_CMD_PORTID;
	cmd->tid = ++bump_cur_tid;
	bcopy(cmd, pgs_fifo_cmd + pgs_pfifo->command_in,
						sizeof(pgs_bump_msg_t));

	__iospace_eieio();		/* preserve order */
	pgs_pfifo->command_in = (pgs_pfifo->command_in + 1) % pgs_pfifo->length;
	__iospace_eieio();		/* preserve order */

					/* send an interrupt to BUMP */
	((struct pgs_sys_spec*)(sys_resource_ptr->sys_specific_regs))->
								set_int_bu = 0;
	__iospace_sync();		/* make sure seen */
	
	for (;;){
		while(pgs_pfifo->status_out == pgs_pfifo->status_in)
			io_delay(100);		/* busy wait */
		stp = pgs_fifo_stat + pgs_pfifo->status_out;
		if (stp->cpu_portid == CPU_CMD_PORTID){
			/* status message associated to the last command */
			if (stp->tid == bump_cur_tid)
				break;
		}
		pgs_pfifo->status_out =
			(pgs_pfifo->status_out + 1) % pgs_pfifo->length;
	}
	bcopy(stp, stat, sizeof(pgs_bump_msg_t));
	pgs_pfifo->status_out = (pgs_pfifo->status_out + 1) % pgs_pfifo->length;
	unlock_enable(ipri, &pgs_bump_lock);
}


/*
 * mdbumpcmd: makes a transaction on BUMP
 *
 *	NB: 	Driver is mp-safe:
 *		. accesses to BUMP are serialized with BUMP interrupt handler 
 *			using a simple lock.
 *		As there are no performance requirements:
 *         	. driver is serialized at the thread level by machdd 
 *			serialization lock: only one transaction on BUMP 
 *			occurs at a time.
 * RETURN VALUE:
 *		0 if no error
 * 
 */

int mdbumpcmd(pgs_bump_msg_t *cmd, pgs_bump_msg_t *stat)
{
	int ipri, i;

	ipri = disable_lock(INTBUMP, &pgs_bump_lock);

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 */
        if (pgs_SSGA_lvl == 2 && fd_mutex)
                d_abort_fd();

	/* cmd fifo should never be full, since accesses to 
	   machdd are serialized */

	assert((pgs_pfifo->command_in + 1) % pgs_pfifo->length != pgs_pfifo->command_out);

	cmd->cpu_portid = CPU_CMD_PORTID;
	cmd->tid = ++bump_cur_tid;

	bcopy(cmd,pgs_fifo_cmd + pgs_pfifo->command_in,sizeof(pgs_bump_msg_t));
	__iospace_eieio();		/* preserve order */
	pgs_pfifo->command_in = (pgs_pfifo->command_in + 1) % pgs_pfifo->length;
	__iospace_eieio();		/* preserve order */

	/* send an interrupt to BUMP */

	((struct pgs_sys_spec*)(sys_resource_ptr->sys_specific_regs))
		->set_int_bu = 0;
	__iospace_sync();		/* make sure seen */

	/* sleep in interruptible mode */
	
	if (e_sleep_thread(&pgs_tpevent, &pgs_bump_lock,
			   LOCK_HANDLER | INTERRUPTIBLE) == THREAD_INTERRUPTED) {
		unlock_enable(ipri, &pgs_bump_lock);
		return EINTR;
	}
	
	bcopy (&pgs_last_status, stat, sizeof pgs_last_status);
	unlock_enable(ipri, &pgs_bump_lock);
	return 0;
}

/*
 * Redirection of BUMP epow interrupts to the normal EPOW handler
 */

mdbumpepow()
{
	int ipri = i_disable(INTEPOW);
	i_epow(epowhandler);
	i_enable(ipri);
}


/*
 * NAME: mdbumpintr
 *
 * FUNCTION:
 *	BUMP off-level interrupt handler (scheduled by misc interrupt handler)
 *
 * EXECUTION ENVIRONMENT:
 *	Executes at BUMP's off-level interrupt level -
 *	this routine cannot page fault.
 *
 * NOTES:
 *    
 * RETURN VALUE:
 *	None.
 */

void
mdbumpintr()
{
	register pgs_bump_msg_t *stat;
	register uint power_status;
	register uint behavior;
	extern uint get_pksr();

/* NVRAM WA */
	int intpri;

/*
 * Pegasus NVRAM WA - See dma_ppc.c for details.
 *
 */
        if (pgs_SSGA_lvl == 2) {
		intpri = disable_lock(DISKETTE_INT_LVL,&pgs_bump_lock);
		if (fd_mutex)
			d_abort_fd();
	}
	else
		simple_lock(&pgs_bump_lock);

	while(pgs_pfifo->status_out != pgs_pfifo->status_in) {
		stat = pgs_fifo_stat + pgs_pfifo->status_out;
		if (stat->cpu_portid == CPU_CMD_PORTID) {
			/* status message associated to the last command */
			if (stat->tid == bump_cur_tid) {
				bcopy (stat, &pgs_last_status,
							sizeof pgs_last_status);
				e_wakeup(&pgs_tpevent);
			}
		} else {
			/* unsollicited status message */
			assert(stat->cpu_portid == CPU_UNSOL_PORTID);
			switch(stat->data.misc.cmd_status) {
			case POWER_INTERRUPT :
			case SWITCH_INTERRUPT :
				pgs_pfifo->status_out =
				  (pgs_pfifo->status_out +1)% pgs_pfifo->length;
				simple_unlock(&pgs_bump_lock);
				power_status = get_pksr();
				behavior = (power_status >> BEHAVIOR_SHIFT) 
								& BEHAVIOR_MASK;
				warning_log.psr = power_status;
				if (behavior == NULL) {
				  /* no behavior set, just return if not running
				   * on battery. If swichting on battery, start
				   * rc.powerfail.
				   */
				  if (power_status & BATTERY)
				    pidsig(1, SIGPWR);
				} else if ((behavior == WARN_COOLING) ||
					   (behavior == WARN_POWER)) {
				  /* start rc.powerfail to report the warning
				   * condition, and log an ERRID_EPOW_SUS message
				   * in the errlog file.
				   */
				  pidsig(1, SIGPWR);
				  errsave(&warning_log, sizeof(warning_log));
				} else {
				  /* All others behaviors are handled in the
				   * EPOW interrupt handler.
				   */
				  if (PPDA->cpuid == MP_MASTER)
				    mdbumpepow();
				  else
				    mpc_send(MP_MASTER, mdbumpepow_mpc);
				}
				simple_lock(&pgs_bump_lock);
				continue;
			case KEY_INTERRUPT :
				if (md_key_int_pid)
					pidsig(md_key_int_pid, md_key_int_signo);
				break;
			case MEMORY_INTERRUPT :
				OldParityError++;		/* Temporary */
				/* Single bit memory parity not logged */
				break;
			case SINGLE_PARITY_ERR_NEW:
				/*
				 * New single bit memory parity error interrupt
				 *
				 * The error log record has to be packed, it may
				 * not contain reserved fields.
				 * Alignment interrupts should be avoided.
				 */
				errors.perror.cmd_status =
						stat->data.perror.cmd_status;
				errors.perror.cmd_detstat =
						stat->data.perror.cmd_detstat;
				errors.perror.cmd_board =
						stat->data.perror.cmd_board;
				errors.perror.cmd_mask[0] =
						stat->data.perror.cmd_mask[0];
				errors.perror.cmd_mask[1] =
						stat->data.perror.cmd_mask[1];
				errors.perror.cmd_addr[0] = * (uchar *)
						&stat->data.perror.cmd_addr;
				errors.perror.cmd_addr[1] = * ((uchar *)
					&stat->data.perror.cmd_addr + 1);
				errors.perror.cmd_addr[2] = * ((uchar *)
					&stat->data.perror.cmd_addr + 2);
				errors.perror.cmd_addr[3] = * ((uchar *)
					&stat->data.perror.cmd_addr + 3);
				errsave(&errors, sizeof errors);
				break;
#ifdef DEBUG
			default:
				printf("Unsollicited BUMP status message: 0x%x ignored\n", stat->data.misc.cmd_status);
#endif /* DEBUG */
			}
		}
		pgs_pfifo->status_out = (pgs_pfifo->status_out + 1) % pgs_pfifo->length;
	}
        if (pgs_SSGA_lvl == 2)
		unlock_enable(intpri,&pgs_bump_lock);
	else
		simple_unlock(&pgs_bump_lock);
}

/*
 * NAME: mdkeyconnect
 *
 * FUNCTION: Connect current process to key BUMP interrupt.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can not page fault.
 *
 * NOTES:
 *        signo:	signal to be sent to process 
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdkeyconnect(int signo)
{
	struct proc *p;
	int ipri;

	ipri = disable_lock(INTBUMP, &pgs_bump_lock);
	if (md_key_int_pid && (p = VALIDATE_PID(md_key_int_pid))
	    && p->p_stat != SNONE) {
		unlock_enable(ipri, &pgs_bump_lock);
		return EBUSY;
	}
	md_key_int_signo = signo;
	md_key_int_pid = u.u_procp->p_pid;
	unlock_enable(ipri, &pgs_bump_lock);
	return 0;
}

/*
 * NAME: mdkeydisconnect
 *
 * FUNCTION: Disconnect current process from key BUMP interrupt.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can not page fault.
 *
 * NOTES:
 *    
 * RETURN VALUE:
 *         0 : if no error
 */
int mdkeydisconnect()
{
	int ipri;

	ipri = disable_lock(INTBUMP, &pgs_bump_lock);
	if (md_key_int_pid != u.u_procp->p_pid) {
		unlock_enable(ipri, &pgs_bump_lock);
		return ESRCH;
	}
	md_key_int_pid = 0;
	unlock_enable(ipri, &pgs_bump_lock);
	return 0;
}

/*
 * NAME: pgs_rtc_start
 *
 * FUNCTION: start simultaneously the RTC or TB of all the processors
 *
 */
void
pgs_rtc_start()
{
        volatile uchar s, *sp;

        sp = &(((struct pgs_sys_spec*)
                (sys_resource_ptr->sys_specific_regs))->start_rtc);
#ifdef _POWER_601
	if (__power_601())
		*sp = 0; /* HW: RTC start = store byte */
	else
#endif /* _POWER_601 */
		s = *sp; /* HW: RTC start = load byte */
}

/*
 * NAME: pgs_rtc_stop
 *
 * FUNCTION: stop simultaneously the RTC or TB of all the processors
 *
 */
void
pgs_rtc_stop()
{
        volatile uchar s, *sp;

        sp = &(((struct pgs_sys_spec*)
                (sys_resource_ptr->sys_specific_regs))->start_rtc);
#ifdef _POWER_601
	if (__power_601())
		s = *sp; /* HW: RTC stop = load byte */
	else
#endif /* _POWER_601 */
		*sp = 0; /* HW: RTC stop = store byte */
}

/*
 * NAME: mdreboot
 *
 * FUNCTION: PEGASUS soft IPL.
 *
 * EXECUTION ENVIRONMENT:  Process; this routine can page fault.
 *
 * NOTES:
 *    
 * RETURN VALUE:
 *         does not return if no errot
 */
int mdreboot()
{
	extern ulong ipl_cb;			/* ipl control block real address */
	pgs_bump_msg_t cmd, stat;
	register int rc;

	cmd.bump_portid = BUMP_SYSTEM_PORTID;
	cmd.command = REBOOT;
	cmd.data.reboot.ipl_cb = ipl_cb;
	
	if ((rc = mdbumpcmd(&cmd, &stat)) != 0)
	   return(rc);

	while (1);	/* no return */
}

#endif /* _RS6K_SMP_MCA */

