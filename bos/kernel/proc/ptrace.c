static char sccsid[] = "@(#)54	1.42.1.33  src/bos/kernel/proc/ptrace.c, sysproc, bos41J, 9519B_all 5/11/95 11:29:26";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: procxmt
 *		ptattach
 *		ptdetach
 *		ptrace
 *		ptrace_attach
 *		ptrace_detach
 *		ptrace_kill
 *		pttraced
 *		
 *   ORIGINS: 27, 3, 26, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/user.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/reg.h>
#include <sys/ptrace.h>
#include <sys/seg.h>
#include <sys/pseg.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/lockl.h>
#include <sys/sleep.h>
#include <sys/adspace.h>
#include <sys/syspest.h>
#include <sys/ldr.h>
#include <sys/malloc.h>
#include <sys/id.h>
#include <sys/machine.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"
#include "sig.h"

#define V3_UBLOCK_SIZE  18580
#define PT_READ_U	3
#define PT_WRITE_U	6
#define INIT_PID        1              

extern int	ld_ldinfo();	/* fetches loader information */
extern int	fp_ufalloc();	/* converts file pointer to file descriptor */

static enum	ioflag { IO_NONE, IO_READ, IO_WRITE };
static enum	iospace { IO_SYS, IO_USR };

static struct proc *pttraced();
static int ptattach();
static int ptdetach();

int	ptrace_maxldinfo = MAXLDINFO;	/* to be able to patch */

struct tid_list {
	tid_t	tid;	
	struct ptthreads th;
};

/*
 * NAME: ptrace
 *
 * FUNCTION: This function is the interface used that allows processes to be
 *	traced by a debugger.  This system call is utilized by the debugger
 *	to pass commands to the stopped process and modify its memory image.
 *
 * DATA STRUCTURES: The process table entry is modified when a process is
 *	being traced.  The user area block can be read and/or written to.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine may page fault
 *                                                                    
 * NOTE: Ptrace() can be called by the debugger or debuggee:
 *	If called by the traced process (PT_TRACE_ME), it allocates the inter
 *	process communications area if necessary, sets up the flags, process
 *	pointers, locks, etc., and signals itself to wait in procxmt().
 *	Otherwise it is called by the debugger and validates the parameters,
 *	confirms that the requested pid is stopped, copies user data to the 
 *	IPC area, wakes up the traced processes and sleeps awaiting results.
 *	Upon awaking, status is checked and data is copied out from the IPC.
 *
 * RETURN VALUE DESCRIPTION:
 *	 0	= successful completion
 *	-1	= failed, errno indicates cause of failure
 */ 
int
ptrace(int request, int id, int *addr, int data, int *buff)
/* int		request;	requested action by the ptrace call	*/
/* int		id;		stopped thread ID 			*/
/* int		*addr;		address pointer dependent on request	*/
/* int		data;		data depending on request type	*/
/* int		*buff;		pointer to data in debugger's space	*/
{
	struct ptipc	*ipc;
	struct proc	*trp, *dbp;
	struct ld_info	*ldorgp, *ldp;	/* pointer to loader info	*/
	char		*addrp;		/* user address for I/O		*/
	int		length;		/* length of data for I/O	*/
	int		error;		/* returned errno		*/
	int		status;		/* returned value		*/
	enum ioflag	ioflag;
	int		lockval;
	struct thread	*trt;
	struct thread	*th;
	int		*pint;
	pid_t		trpid;
        tid_t		request_tid;
	int		cnt;
	int		ipri;
	struct tid_list tid_list;  
	tid_t 		*tlist;


	ioflag = IO_WRITE;
	status = error = length = 0;

	simple_lock(&ptrace_lock);

retry:

	/* 
	 * Handle validation of all data, set 
	 * copy I/O flags, set proc flags.
	 */
	switch (request) {

	case PT_TRACE_ME:	/* used ONLY by the process to be traced */
		if ((trp = U.U_procp)->p_flag & STRC) {
			/*
			 * Fail if it is already being debugged by
			 * a process that is not its parent.
			 * Succeed if it is already being debugged 
			 * by its parent (System V ptrace behaviour).
			 */
			if (trp->p_ppid == trp->p_ipc->ip_dbp->p_pid)
				goto ok;
			error = ESRCH;
			goto err;
		}
		dbp = PROCPTR (trp->p_ppid);
		goto attach;

	case PT_ATTACH:	/* attach to a process */
	case PT_REATT:	/* attach another debugger */
		/*
		 * Don't allow:
		 *	attachs with invalid pids; or
		 *	PT_ATTACH to a process that is already being traced.
		 */
		if (!(trp = VALIDATE_PID(id))   ||
		     (trp->p_flag & SKPROC)     ||
		     (trp->p_pid == INIT_PID)   ||
		     ((trp->p_flag & STRC) && request == PT_ATTACH)) {
			error = ESRCH;
			goto err;
		}
		dbp = U.U_procp;

		/*
		 * Check that debugger has permissions to atach.
		 */
		if (privcheck(BYPASS_DAC) && 
		    trp->p_suid != getuidx(ID_REAL) &&
		    trp->p_suid != getuidx(ID_EFFECTIVE)) {
			error = EPERM;
			goto err;
		}

	attach:
		if ((error = ptattach(dbp, trp)) < 0)
			goto retry;
		if (error)
			goto err;

		if (request != PT_TRACE_ME)
			pidsig(trp->p_pid, SIGTRAP);
		simple_unlock(&ptrace_lock);
		return 0;

	case PT_LDINFO:	/* get array of file desc. for loaded program */
		addrp = (caddr_t) addr;
		length = data;
		if (pttraced(id) == NULL) {	/* only symbols of traced */
			error = ESRCH;
			goto err;
		}

		/*
		 * Length is bounded because there might be other
		 * places in the kernel that assume that xmalloc from the
		 * kernel_heap does not fail (wrong!). Anyhow letting the
		 * user allocate all the memory in the kernel_heap is an
		 * invitation for trouble.
		 */
		if (length > ptrace_maxldinfo)
			length = ptrace_maxldinfo;
		ldorgp = (struct ld_info *) xmalloc (length, 2, kernel_heap);
		if (! ldorgp) {
			error = ENOMEM;
			goto err;
		}
		if (ld_getinfo(id, length, ldorgp)) {
			xmfree(ldorgp, kernel_heap);
			error = ENOMEM;	/* return failure, user should retry */
			goto err;
		}
		ldp = ldorgp;
		do {	/* always at least one entry */
			ldp->ldinfo_fd = fp_ufalloc(ldp->ldinfo_fp);
		} while ((ulong)ldp->ldinfo_next &&
			 (ldp = (struct ld_info *)
				((ulong)ldp + (ulong)ldp->ldinfo_next)));
		if (copyout((caddr_t) ldorgp, addrp, length)) {
			xmfree(ldorgp, kernel_heap);
			error = EFAULT;
			goto err;
		}
		xmfree(ldorgp, kernel_heap);
		goto ok;

	case PT_WRITE_I:	/* write instruction address space */
	case PT_WRITE_D:	/* write data address space */
	case PT_READ_I:	/* read instruction address space */
	case PT_READ_D:	/* read data address space */
		ioflag = IO_NONE;	/* set read flag for no copy */
		/* validate passed address */
		if (! ISWALIGN(addr) || ! VALID_ADDR(addr)) {
			error = EIO;
			goto err;
		}
		break;

	case PT_WRITE_U: /* write offset to uthread structure */
		if (!ISWRITEREG(addr)) {
			error = EIO;
			goto err;
		}
	case PT_READ_U:	/* read offset from uthread structure */
		ioflag = IO_NONE;	/* set read flag for no copy */
		/* validate passed address */
		if (!ISWALIGN(addr) ||
		     (int)addr < (int)0 || 
                     (int)addr > V3_UBLOCK_SIZE) {
			error = EIO;
			goto err;
		}
		break;

	case PT_WRITE_BLOCK:/* write block of data ptrace(19) */
		ioflag = IO_READ;	/* set read flag to copy in user data */
	case PT_READ_BLOCK:	/* read block of data ptrace(17) */
		/* validate passed address and size */
		if (!ISWALIGN(addr) || data < 1 || data > IPCDATA) {
			error = EIO;
			goto err;
		}
		addrp = (char *)buff;
		length = data;
		break;

	case PT_READ_GPR:	/* read general purpose register */
	case PT_WRITE_GPR:	/* write general purpose register */
		ioflag = IO_NONE;	/* set read flag for no copy */
		/* validate passed GPR */
		if (!VALID_GPR((int)addr) && !VALID_SPR((int)addr) ) { 
			error = EIO;
			goto err;
		}
		break;

	case PT_WRITE_FPR:	/* write floating point register */
		ioflag = IO_READ;	/* set read flag to copy in user data */
	case PT_READ_FPR:	/* read floating point register */
		/* validate passed FPR */
		if (!VALID_FPR(data)) {
			error = EIO;
			goto err;
		}
		addrp = (caddr_t) addr;
		length = FPR_size;
		break;

	case PTT_WRITE_FPRS:	/* write floating point registers */
		ioflag = IO_READ; /* set read flag to copy in user data */
	case PTT_READ_FPRS:	/* read floating point registers */
		addrp = (caddr_t) addr;
		length = FPR_size * NFPRS;
		break;

	case PT_CONTINUE:	/* continue process execution */
		status = data;
		length = sizeof(struct ptthreads);
		goto detach;

	case PT_KILL:		/* same as PT_DETACH with kill signal */
		data = SIGKILL;
	case PT_DETACH:		/* detach a proc to let it continue running */
detach:
		ioflag = IO_NONE;
		if (data < 0 || data > SIGMAX) {
			error = EIO;
			goto err;
		}
		break;

	case PTT_CONTINUE:	/* continue thread execution */
		if (buff == NULL)
			ioflag = IO_NONE;
		else {
			ioflag = IO_READ;
			addrp = (caddr_t)buff;
		}
		length = sizeof(struct ptthreads);
		if (data < 0 || data > SIGMAX) {
			error = EIO;
			goto err;
		}
		status = data;
		break;

	case PT_MULTI:	/* set/clear/read multi-processing */
		ioflag = IO_NONE;
		break;

	case PTT_WRITE_GPRS:
		ioflag = IO_READ;
	case PT_REGSET:	/* return entire register set to caller */
	case PTT_READ_GPRS:
		addrp = (caddr_t) addr;
		length = GPR_size * NGPRS;
		break;

	case PTT_WRITE_SPRS:
		ioflag = IO_READ; /* set read flag to copy in user data */
	case PTT_READ_SPRS:
		addrp = (caddr_t) addr;
		length = sizeof(struct ptsprs);
		break;

	default:
		error = EIO;
		goto err;
	}

again:
	/*
	 * convert id parm from process-based to thread-based
	 */
	switch(request) {
	case PT_READ_U:
	case PT_DETACH:
	case PT_CONTINUE:
	case PT_KILL:
	case PT_MULTI:
	case PT_READ_D:
	case PT_READ_I:
	case PT_READ_BLOCK:
	case PT_READ_FPR:
	case PT_READ_GPR:
	case PT_REGSET:
	case PT_WRITE_BLOCK:	
	case PT_WRITE_D:
	case PT_WRITE_FPR:
	case PT_WRITE_GPR:
	case PT_WRITE_I:
	case PT_WRITE_U:
		trpid = id;
                request_tid = 0;
		break;
	default:

		trt = VALIDATE_TID(id);
		if (trt == NULL || trt->t_state == TSIDL) {
			error = ESRCH;
			goto err;
		}
		if (trt->t_flags & TKTHREAD) {
			error = EPERM;
			goto err;
		}

		switch (request) {
		case PTT_CONTINUE:
			/* id parameter must name thread that hit the trap */
			if (!(trt->t_flags & TTRCSIG)) {
				error = EINVAL;
				goto err;
			}
			break;
                default:
                        /*
			 * id parameter must name a thread in user mode,
			 * if not traced.
			 */
                        if (trt->t_suspend && !(trt->t_flags & TTRCSIG)) {
                                error = EPERM;
                                goto err;
                        }
                        break;
		}

		trpid = trt->t_procp->p_pid;
		request_tid = id;
		break;

	} /* switch */

        /* debuggee must be stopped and this must be the debugger to respond */
        if ((trp = pttraced(trpid)) == NULL) {
                error = ESRCH;
                goto err;
        }

	/* Backward compatibility */
	switch(request) {
	case PT_READ_U :
                /* We don't provide accurate data beyond the mst */
                if ((int)addr > sizeof(struct mstsave)) {
			status = 0;
			goto ok;
                }
		break;
	case PT_READ_GPR :
	case PT_WRITE_GPR :
		/* We don't support the tid register anymore */ 
		if ((int)addr == TID) {
			status = 0;
			goto ok;
		}
		break;
	}

	if ((ipc = trp->p_ipc)->ip_flag & IPCBUSY) {
		ipc->ip_flag |= IPCWANTED;
		e_sleep_thread(&ipc->ip_event, &ptrace_lock, LOCK_SIMPLE);
		goto again;
	}
	ipc->ip_flag |= IPCBUSY;

	if (length > IPCBSZ && !(ipc->ip_flag & IPCBLKDATA)) {
		ipc->ip_flag |= IPCBLKDATA;
		ipc->ip_blkdata = 
			xmalloc(MAX(IPCDATA, sizeof(int) * (MAXTHREADS + 1)), 2, 
					pinned_heap);
	}

	/* if read flag, copyin() data from user to IPC */
	if (ioflag == IO_READ && copyin(addrp, ipc->ip_blkdata, length)) {
		u.u_error = EFAULT;
		status = -1;
		goto wake;
	}

	switch(request) {

	case PTT_CONTINUE :
		cnt = 0;
		pint = (int *)ipc->ip_blkdata;
		if (ioflag == IO_READ) {
                	/* validate tids in buf array */
                	for ( ; *pint != NULL_TID && cnt < MAXTHREADS;
                     	      pint++, cnt++) {
                        	/* must be valid tid and in the same process */
                        	if ((th = VALIDATE_TID(*pint)) == NULL ||
                            	     th->t_procp != trp) {
                                	u.u_error = ESRCH;
                                	status = -1;
                                 	goto wake;
                         	}
                	}
		}
		if (cnt < MAXTHREADS)
			*pint = NULL_TID;
		break;

	case PT_CONTINUE :
		/* 
		 * Fill out structure with tids so that the same
		 * mechanism as PTT_CONTINUE can be used to start
		 * the process within procxmt.
		 */
		cnt = 0;
		pint = (int *)ipc->ip_blkdata;
        	trt = trp->p_threadlist;
        	do {
            		if (!(trt->t_flags & TTRCSIG)) {
				*pint++ = trt->t_tid;
				cnt++;
			}
         		trt = trt->t_nextthread;
        	} while (trt != trp->p_threadlist);
		if (cnt < MAXTHREADS)
			*pint = NULL_TID;
		/* Falls through */

	default :

        	/* determine which thread took the exception for the setrq */
        	trt = trp->p_threadlist;
        	do {
            		if (trt->t_flags & TTRCSIG)
               	   		break;
         		trt = trt->t_nextthread;
        	} while (trt != trp->p_threadlist);
		break;

	}

	/* load needed IPC intructions */
	ipc->ip_req = request;
	ipc->ip_addr = addr;
	ipc->ip_data = data;
	if (request_tid)
	   	ipc->ip_id = request_tid;
	else
	   	ipc->ip_id = trt->t_tid;

	ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
	simple_lock(&proc_int_lock);
#endif

	trp->p_suspended--;
	trp->p_int &= ~SSUSP;
	setrq(trt, E_WKX_PREEMPT, RQTAIL);

	/* 
	 * Wait for each command to complete.
	 */
	e_assert_wait(&ipc->ip_event, 0);

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
#endif
	unlock_enable(ipri, &proc_base_lock);

	/* sleep on the IPC event */
	simple_unlock(&ptrace_lock); 

	e_block_thread();

	simple_lock(&ptrace_lock);

	/* ipc could be detached via exit before debugger runs. */
	if ((request != PT_DETACH) && ((ipc = trp->p_ipc) != NULL)) {

		/*
	 	 * Fail if IPC request value is not changed to
	 	 * zero or if copyout is needed and it fails.
	 	 */
		if (ipc->ip_req != 0) {
			u.u_error = EIO;
			status = -1;
		} else {
			status = ipc->ip_data;
			if (ioflag == IO_WRITE) {
				if (copyout(ipc->ip_blkdata, addrp, length)) {
					u.u_error = EIO;
					status = -1;
				}
			}
		}

wake:
		ipc->ip_flag &= ~IPCBUSY;
		if (ipc->ip_flag & IPCWANTED) {
			ipc->ip_flag &= ~IPCWANTED;
			e_wakeup(&ipc->ip_event);
		}
	}

ok:

	simple_unlock(&ptrace_lock);
	return status;

err:

	simple_unlock(&ptrace_lock);
	u.u_error = error;
	return -1;
}

/*
 * NAME: ptattach
 *
 * FUNCTION:
 *	Attach a process (trp) to a debugger (dbp).
 *
 *	This is also called when a debugger does a PT_REATT to
 *	take over a process that already has a debugger.
 *	The transition from non-traced to traced is done here
 *	(i.e. setting the STRC flag in a proc structure).
 *	No other code in the system should set the STRC p_flag.
 *
 * EXECUTION ENVIRONMENT:
 *	The ptrace_lock must be held by the caller.
 *	This routine may only be called by a process.
 *	This routine may page fault.
 *	This function might sleep on a busy ptipc structure,
 *	-1 is returned in that case to indicate that the caller
 *	has to retry the operation (because the system state
 *	could have changed).
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	0	successful completion
 *	> 0	failed (errno)
 *	< 0	slept, caller should retry
 */
static
int
ptattach(struct proc *dbp, struct proc *trp)
{
	struct ptipc	*ipc;
	register int 	ipri;
	int		smptrace=0;

	ASSERT(lock_mine(&ptrace_lock));

	/*
	 * Check processes are not exiting and that
	 * a process does not try to attach to itself.
	 */
	if (trp->p_flag & SEXIT || dbp->p_flag & SEXIT)
		return ESRCH;
	if (dbp == trp)
		return EINVAL;		/* what is the error? XXX */

	if (ipc = trp->p_ipc) {
		/*
		 * Process already has a debugger.
		 */

		if (ipc->ip_flag & IPCBUSY) {
			/*
			 * Ipc is busy, sleep on it and return
			 * -1 so that the caller can retry.
			 * This might happen if a new debugger
			 * tries to PT_REATT to a process while
			 * that process is being traced.
			 */
			ipc->ip_flag |= IPCWANTED;
			e_sleep_thread(&ipc->ip_event,&ptrace_lock,LOCK_SIMPLE);
			return -1;
		}

		if (ipc->ip_dbp == dbp)
			/*
			 * The debugger that wants to reattach
			 * to trp is already its debugger.
			 */
			return 0;

		/*
		 * Save SMPTRACE bit, ptdetach does not know
		 * that the process will continue to be debugged
		 * and that the SMPTRACE should be kept.
		 */
		smptrace = trp->p_flag & SMPTRACE;
		(void) ptdetach(trp, 1);
	}

	/*
	 * Attach to new debugger.
	 */
	ipc = (struct ptipc *)
		xmalloc(sizeof(struct ptipc), 2, pinned_heap);
	if (!ipc)
		return ENOMEM;

        /*
         * Must have both the ptrace_lock and the proc_base_lock
         * to change STRC and STRACING.  Either can be used 
	 * independently to read it.
         */
	ipri = disable_lock(INTMAX, &proc_base_lock);

	trp->p_flag |= STRC;
	if (smptrace) 
		trp->p_flag |= smptrace;

	dbp->p_flag |= STRACING;

	ipc->ip_flag = 0;
	ipc->ip_dbp = dbp;
	ipc->ip_event = EVENT_NULL;
	ipc->ip_blkdata = ipc->ip_buf;

	trp->p_ipc = ipc;
	trp->p_dbnext = dbp->p_dblist;
	dbp->p_dblist = trp;

	unlock_enable(ipri, &proc_base_lock);

	/* 
	 * loader makes private copy of text & shlib;
	 * check to ensure adspace had been initialized
	 * in new process creation.
	 */
	if (trp->p_adspace == NULLSEGVAL)
		return 0;
	else
		return(ld_ptrace(trp->p_pid));
}

/*
 * NAME: ptdetach
 *
 * FUNCTION:
 *	Dettach a process (trp) from its debugger.
 *
 *	The transition from traced to non-traced is located here
 *	(i.e. clearing the STRC flag in a proc structure).
 *	No other code in the system should clear the STRC p_flag.
 *	The owner flag indicates that the caller already made the
 *	ptipc struct busy, so that this function does not sleep on it.
 *
 * EXECUTION ENVIRONMENT:
 *	The ptrace_lock must be held by the caller.
 *	This routine may only be called by a process.
 *	This routine may page fault.
 *	This function might sleep (if !owner) on a busy ptipc
 *	structure, -1 is returned in that case to indicate that
 *	the caller has to retry the operation (because the system
 *	state could have changed).
 *
 * RETURN VALUE DESCRIPTION:
 *	0	successful completion
 *	< 0	slept, caller should retry
 */
static
int
ptdetach(struct proc *trp, int owner)
{
	struct ptipc	 *ipc;
	struct proc	 *dbp;
	struct proc	**pp;
	int   		 ipri;

	ASSERT(lock_mine(&ptrace_lock));

	/*
	 * If not the owner and the ipc structure is busy sleep
	 * on it and return -1 so that the caller can retry.
	 * This might happen if the debuggee wants to exit
	 * after the debugger did a PT_CONTINUE and the
	 * debugger did not got a chance to pick the results
	 * in the ptipc struct and make it ~IPCBUSY.
	 */
	ipc = trp->p_ipc;
	ASSERT(ipc);
	if (!owner && ipc->ip_flag & IPCBUSY) {
		ipc->ip_flag |= IPCWANTED;
		e_sleep_thread(&ipc->ip_event, &ptrace_lock, LOCK_SIMPLE);
		return -1;
	}

        ipri = disable_lock(INTMAX, &proc_base_lock);

	e_wakeup(&ipc->ip_event);

	/*
	 * Remove trp from dbp's list of traced processes.
	 */
	dbp = ipc->ip_dbp;
	pp = &dbp->p_dblist;
	while (*pp != trp)
		pp = &(*pp)->p_dbnext;
	*pp = trp->p_dbnext;

	if (!dbp->p_dblist)
		/*
		 * No more processes to debug.
		 */
		dbp->p_flag &= ~STRACING;

	trp->p_flag &= ~(STRC | SMPTRACE);

	trp->p_dbnext = NULL;
	trp->p_ipc = NULL;

        unlock_enable(ipri, &proc_base_lock);

	if (ipc->ip_flag & IPCBLKDATA)
		xmfree(ipc->ip_blkdata, pinned_heap);
	xmfree(ipc, pinned_heap);

	return 0;
}

/*
 * NAME: pttraced
 *
 * FUNCTION:
 *	Validate pid and find out if it can be debugged
 *	by the caller at this time (i.e. check that the
 *	caller is tpid's debugger).
 *
 * EXECUTION ENVIRONMENT:
 *	The ptrace_lock must be held by the caller.
 *	This routine may only be called by a process.
 *	This routine may page fault.
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	0		failed
 *	(struct proc *)	traced process proc pointer
 */
static struct proc *
pttraced(pid_t tpid)
/* pid_t	tpid;	traced process ID		*/
{
	struct proc	*trp;		/* traced process pointer */
	int intpri;

	ASSERT(lock_mine(&ptrace_lock));

	intpri = disable_lock(INTMAX, &proc_int_lock);
	if ( ((trp = VALIDATE_PID(tpid)) == NULL) ||
	     (trp->p_stat != SSTOP)               ||
	    !(trp->p_flag & STRC)                 ||
	     (trp->p_ipc->ip_dbp != U.U_procp)) {
		unlock_enable(intpri, &proc_int_lock);
		return(NULL);
	}
	unlock_enable(intpri, &proc_int_lock);
	return(trp);
}

/*
 * NAME: procxmt
 *
 * FUNCTION:
 *	Execute a debugging request on behalf of the caller's debugger.
 *
 *	This routine is called from issig() so that the traced 
 *	process can execute commands on behalf of its debugger. 
 *	This funcion is called from a loop until it returns non-
 *	zero. All addresses are relative to this process address 
 *	space. The mstsave area that is modified may be on the
 *	stack (t_scp) or in the uthread structure.
 *
 * DATA STRUCTURES:
 *	The main data structure used is the the machine state save area.  
 *
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called by a process.
 *	This routine may page fault.
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	0	traced process cannot continue running,
 *		it should be stopped until another ptrace
 *		request is made
 *	!= 0	traced process can continue running
 */
procxmt(void)
{
	struct thread	*trt = curthread;
	struct thread	*ipt;
	struct thread	*th;
	struct proc	*trp;
        struct proc	*dbp;           /* debugger process pointer     */
	struct ptipc	*ipc;		/* private copy of IPC area     */
	char		*addr;		/* user address for I/O		*/
	int		length;		/* length of user I/O		*/
	enum ioflag	ioflag;		/* IO_NONE, IO_READ, IO_WRITE	*/
	enum iospace	iospace=IO_USR;	/* IO_SYS, IO_USR               */
	int		status;		/* return status in ip_req	*/
	int		retval;		/* return value to signaler	*/
	int		lockval;	/* temporary lock value		*/
	struct ptsprs	*ptsprs_ptr;	/* pointer into ip_blkdata	*/
	struct ptsprs	temp_ptsprs;	/* scratchpad space		*/
	struct mstsave	*mstp;		/* pointer into mstsave area	*/
	int		ipri;		/* current interrupt priority	*/
	char		fpinfo;		/* temporary fpinfo holder	*/

	trp = trt->t_procp;

	simple_lock(&ptrace_lock);

	ipc = trp->p_ipc;

	assert(ipc->ip_flag & IPCBUSY); 

	ipt = THREADPTR(ipc->ip_id);
	ASSERT(ipt != NULL);

	/* set up pointer into mst structure */
	if (ipt->t_scp != NULL) {
		mstp = &ipt->t_scp->sc_jmpbuf.jmp_context;
	} else {
		ASSERT(ipt->t_uthreadp->ut_save.msr & MSR_PR);
		mstp = &ipt->t_uthreadp->ut_save;
		iospace = IO_SYS;
	}

	dbp = ipc->ip_dbp;
	ASSERT(dbp);

	addr = (char *) ipc->ip_addr;
	status = length = retval = 0;
	ioflag = IO_READ;

	/* set the address, out/in flag, data transfer length */
	switch (ipc->ip_req) {

	case PT_DETACH:		/* detach a proc to let it keep running */

		ipri = disable_lock(INTMAX, &proc_base_lock);

		/* 
		 * The debugger has the option of calling PTT_CONTINUE to 
		 * clear the current signal in the other threads before 
		 * detaching.  If it doesn't do this then the signal will 
		 * be delivered, which in the case of SIGTRAP will generate 
		 * a core file.  This is only a problem if signals are being 
		 * delivered to multiple threads concurrently.
		 */

		trt->t_cursig = ipc->ip_data;

		/* 
	 	 * Continue the process before waking the debugger in 
		 * ptdetach so that it is put back into its normal state 
		 * before the debugger has a chance to interact with it 
		 * again.  Signals behave differently if the process
		 * is partially stopped.  Suspension is also used to 
		 * serialize many intra-process events such as thread 
		 * harvesting.  
	 	 */ 
		cont(trp);

		unlock_enable(ipri, &proc_base_lock);

		/*
		 * Detach process from debugger.  If there are other 
		 * threads about to enter procxmt, they should notice
		 * that STRC has been turned off and branch around
		 * procxmt.  
		 */
		ptdetach(trp, 1);

		simple_unlock(&ptrace_lock);

		return(1);

	case PT_KILL: 		/* same as PT_CONTINUE with kill signal */
		/* Clear all signals since SIGKILL will be posted below. */
		th = trt;
		ipri = disable_lock(INTMAX, &proc_base_lock);
		SIGINITSET(trp->p_sig);
		do {
			SIGINITSET(th->t_sig);
			th->t_cursig = 0;
			th = th->t_nextthread;
		} while (th != trt);
		unlock_enable(ipri, &proc_base_lock);
		goto pt_continue;

	case PTT_CONTINUE: 	
	case PT_CONTINUE:
		/* 
		 * Start the threads listed in ip_blkdata, unless
		 * a thread is about to enter procxmt.  The difference
		 * between these two subcommands is the content of
		 * the buffer.
		 */
		ipri = disable_lock(INTMAX, &proc_base_lock);
		pt_setrun(ipc->ip_blkdata);
		unlock_enable(ipri, &proc_base_lock);

pt_continue:
                retval = 1;
                /* if passed address is != 1, set up iar address */
                if (addr == (char *)1)
                        ioflag = IO_NONE;
                else {
                        /*
                         * A copyoutx will be done below,
                         * anyhow that should not fail because
                         * it is a copyoutx to the iar
                         * in the mst in the u-block.
                         */
                        ioflag = IO_WRITE;
                        *(int *)ipc->ip_blkdata = (int)addr;
                        length = SPR_size(IAR);
                        addr = SPR_loc(mstp,IAR);
                }
                trt->t_cursig = ipc->ip_data;
                break;

	case PT_MULTI:		/* set/clear multi-processing */
		ioflag = IO_NONE;
		ipri = disable_lock(INTMAX, &proc_base_lock);
		if (ipc->ip_data == 0)
			trp->p_flag &= ~SMPTRACE;
		else if (ipc->ip_data == 1)
			trp->p_flag |= SMPTRACE;
		unlock_enable(ipri, &proc_base_lock);
		ipc->ip_data = trp->p_flag & SMPTRACE ? 1 : 0;
		break;

	case PT_WRITE_I:	/* write instruction address space */
	case PT_WRITE_D:	/* write data address space */
		*(int *)ipc->ip_blkdata = ipc->ip_data;	/* put data in array */
		ioflag = IO_WRITE;
	case PT_READ_I:		/* read instruction address space */
	case PT_READ_D:		/* read data address space */
		length = sizeof(int);
		break;

	case PT_WRITE_BLOCK:	/* write block of data ptrace(19) */
		ioflag = IO_WRITE;
	case PT_READ_BLOCK:	/* read block of data ptrace(17) */
		length = ipc->ip_data;
		break;

	case PT_WRITE_GPR:	/* write general purpose register */
		ioflag = IO_WRITE;
		if ((int)addr == MSR)
		{
			/* fpinfo can be altered via msr */
			fpinfo = UPDATE_FPINFO(ipc->ip_data);
			if (iospace == IO_USR)
			    copyout(&fpinfo, &(mstp->fpinfo), sizeof(fpinfo));
			else
			    bcopy(&fpinfo, &(mstp->fpinfo), sizeof(fpinfo));

			SANITIZE_MSR(ipc->ip_data);
		}
		*(int *)ipc->ip_blkdata = ipc->ip_data;	/* put data in array */
	case PT_READ_GPR:	/* read general purpose register */
		length = VALID_GPR((int)addr) ? GPR_size : SPR_size(addr);
		addr = VALID_GPR((int)addr) ? 
				 GPR_loc(mstp,addr) : SPR_loc(mstp,addr);
		break;

	case PT_WRITE_FPR:	/* write floating point register */
		ioflag = IO_WRITE;
	case PT_READ_FPR:
		addr = FPR_loc(mstp,ipc->ip_data);
		length = FPR_size;
		break;

	case PT_WRITE_U: /* write offset to the uthread structure */
		ioflag = IO_WRITE;
		*(int *)ipc->ip_blkdata = ipc->ip_data;	/* put data in array */
	case PT_READ_U:	/* read offset from the uthread structure */
		addr = (char *)((int)mstp + (int)addr);
		length = GPR_size;
		break;

	case PTT_WRITE_FPRS:	/* write floating point registers */
		ioflag = IO_WRITE;
	case PTT_READ_FPRS:
		addr = FPR_loc(mstp,FPR0);
		length = FPR_size * NFPRS;
		break;

	case PTT_WRITE_GPRS:
		ioflag = IO_WRITE;
		addr = GPR_loc(mstp,GPR0);
		length = GPR_size * NGPRS;
		break;

	case PT_REGSET:	/* return entire register set to caller */
	case PTT_READ_GPRS:
		addr = GPR_loc(mstp, GPR0);
		length = GPR_size * NGPRS;
		break;

	case PTT_WRITE_SPRS:
		ioflag = IO_WRITE;
		addr = SPR_loc(mstp,IAR);
		length = sizeof(struct ptsprs);

		/* do not modify reserved fields in ptsprs */
		if (iospace == IO_USR)
			copyin(addr, &temp_ptsprs, length);
		else
			bcopy(addr, &temp_ptsprs, length);
		ptsprs_ptr = (struct ptsprs *)ipc->ip_blkdata;
		ptsprs_ptr->pt_reserved_0 = temp_ptsprs.pt_reserved_0;
		bcopy(&temp_ptsprs.pt_reserved_1,&ptsprs_ptr->pt_reserved_1,44);

		/* fpinfo can be altered via msr */
		ptsprs_ptr->pt_reserved_2 = UPDATE_FPINFO(ptsprs_ptr->pt_msr);

		SANITIZE_MSR(ptsprs_ptr->pt_msr);
		break;

	case PTT_READ_SPRS:
		addr = SPR_loc(mstp, IAR);
		length = sizeof(struct ptsprs);
		break;

        default:
                retval = -1;
                ioflag = IO_NONE;
                break;
	}

	simple_unlock(&ptrace_lock);

	/* make sure of a private copy */
	if (ld_ptrace(-1)) {
		retval = -1;
	} 
	else {
		/*
		 * Copy in/out the IPC data to the addressed location.
		 * Use copyoutx to be able to write to the text and library
		 * segments and to flush instruction and data caches.
		 */
		if (ioflag == IO_READ) {
			if (iospace == IO_USR)
				status = copyin(addr,ipc->ip_blkdata,length);
			else
				/* bcopy returns void; cannot fail */
				bcopy(addr,ipc->ip_blkdata,length);

                        switch(ipc->ip_req) {
                        case PTT_READ_GPRS :
                        case PTT_READ_FPRS :
                        case PTT_READ_SPRS :
                                ipc->ip_data = 0;
                        case PT_READ_BLOCK :
                                break;
                        default :

                                ipc->ip_data = *(int *)ipc->ip_blkdata;
                        }
		} else if (ioflag == IO_WRITE) {
			if (iospace == IO_USR)
				status = copyoutx(ipc->ip_blkdata,addr,length);
			else
				/* bcopy returns void; cannot fail */
				bcopy(ipc->ip_blkdata,addr,length);
                        switch(ipc->ip_req) {
                        case PTT_WRITE_GPRS :
                        case PTT_WRITE_FPRS :
                        case PTT_WRITE_SPRS :
                                ipc->ip_data = 0;
                                break;
                        }
		}
		ipc->ip_req = status;

	}

	/*
	 * Wakeup the debugger, since we are not going to cycle 
	 * through procxmt again until the traced process receives 
	 * another signal.  Otherwise the debugger is woken in
	 * the issig loop via stop_thread.
	 */
	if (retval == 1) {
		simple_lock(&ptrace_lock);
		ipc->ip_flag &= ~(IPCBUSY|IPCWANTED);
		e_wakeup(&ipc->ip_event);
		simple_unlock(&ptrace_lock);
	}

	return retval;
}

/*
 * NAME: ptrace_attach
 *
 * FUNCTION:
 *	Attach a process to a debugger.
 *
 *	This routine should only be called from fork.
 *	The ptrace_lock should be held in fork before trp is created,
 *	in that way it is impossible for another process to attach
 *	to it before this is done.
 *
 * EXECUTION ENVIRONMENT:
 *	The ptrace_lock must be held by the caller.
 *	This routine may only be called by a process.
 *	This routine may page fault.
 *	
 * RETURN VALUE DESCRIPTION:
 *	0	attach was successful
 *	errno	something failed
 */
int
ptrace_attach(struct proc *dbp, struct proc *trp)
{
	int	error;

	ASSERT(lock_mine(&ptrace_lock));

	error = ptattach(dbp, trp);
	ASSERT(error >= 0);

	return error;
}

/*
 * NAME: ptrace_detach
 *
 * FUNCTION:
 *	Detach a process from its debugger.
 *
 * EXECUTION ENVIRONMENT:
 *	The ptrace_lock must be held by the caller.
 *	This routine may only be called by a process.
 *	This routine may page fault.
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	0	successful completion
 *	< 0	slept, caller should retry
 */
int
ptrace_detach(struct proc *trp)
{
	int rc;

	ASSERT(lock_mine(&ptrace_lock));
	rc = ptdetach(trp, 0);
	return rc;
}


/*
 * NAME: ptrace_kill
 *
 * FUNCTION:
 *	Kill all the processes that are being traced by dbp.
 *
 *	This function is called when a debugger exits. All the
 *	processes traced by dbp will be detached and killed.
 *
 * EXECUTION ENVIRONMENT:
 *	The ptrace_lock must be held by the caller.
 *	This routine may only be called by a process.
 *	This routine may page fault.
 */
void
ptrace_kill(struct proc *dbp)
{
	struct proc	*p;

	ASSERT(lock_mine(&ptrace_lock));

	while (p = dbp->p_dblist)
		if (ptdetach(p, 0) == 0)
			pidsig(p->p_pid, SIGKILL);
}
