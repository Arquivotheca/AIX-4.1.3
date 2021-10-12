static char sccsid[] = "@(#)31	1.25  src/bos/kernel/proc/psmon.c, sysproc, bos41J, 9512A_all 3/20/95 18:53:06";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: chkpskill
 *		pgspkill
 *		pgspwarn
 *
 *
 *   ORIGINS: 27, 83
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
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#include <sys/types.h>
#include <sys/signal.h>
#include <sys/proc.h>
#include <sys/vmker.h>
#include <sys/user.h>
#include <sys/seg.h>
#include <sys/errids.h>
#include <sys/intr.h>
#include <sys/atomic_op.h>

#define	INIT_PID 1

/* error log structure for logging when a process is signalled
 * to be killed due to a low paging space condition.  the size
 * of 32 for 'cmd' corresponds to MAXCOMLEN but the define itself
 * is not used since this size is used in the formatting template
 * (heading PGSP_KILL in com/cmd/errlg/errids.desc) which would
 * need to change if MAXCOMLEN was changed.
 */
struct pgsperr {
	struct err_rec0		hdr;		/* header	*/
	char			cmd[32];	/* command name */
	pid_t			pid;		/* process ID	*/
	int			amt;		/* pgsp in 1K blks */
} pgsplog = { ERRID_PGSP_KILL, "SYSVMM", "", 0, 0 };

/*
 * NAME:     pgspwarn()
 *
 * FUNCTION: called from the VMM when paging space has reached 
 *	     a threshold. All active processes are sent a 
 *	     SIGDANGER
 *
 * EXECUTION ENVIRONMENT: 
 *	     called from a vmm critical section with a fixed stack
 *
 * RETURNS:  NONE
 */

void *
pgspwarn()
{
	struct proc *p;

	/* send danger signal.
	 */
	for( p = &proc[1]; p < max_proc; p++)
	{
		if (p->p_stat != SNONE && p->p_stat != SIDL &&
		    p->p_stat != SZOMB && !(p->p_flag & SKPROC))
		{
			pidsig( p->p_pid, SIGDANGER);
		}
	}
}
	
/*
 * NAME:     pgspkill()
 *
 * FUNCTION: called from the VMM when paging space has reached 
 *	     a threshold. The youngest eligible process
 *	     be sent a SIGKILL
 *
 * EXECUTION ENVIRONMENT: 
 *	     called from a vmm critical section with a fixed stack
 *
 * RETURNS:  
 *	pid 	- the process id of the process sent SIGKILL
 *
 *	-1	- returned if no process found to kill.
 */


pgspkill()
{
	int	ps;
	int	shmps;
	int	ipri;
	struct 	proc *p;
	struct 	proc *killp;
	pid_t	pid;		
	struct	user *uaddr;
        time_t  youngest;


	/* scan through the process table beginning with
	 * proc 1 and ending with the high water mark.
	 */
	killp = NULL;
	pid = -1;
        youngest = 0;

	for( p = &proc[1]; p < max_proc; p++)
	{
		/* if a process has already been killed and has
		 * not yet exited, don't kill another one.
		 */
		if ( (p->p_int & SPSMKILL) && (p->p_stat != SNONE) &&
		     (p->p_stat != SIDL) && (p->p_stat != SZOMB) ) {
			return(pid); /* pid is still set to -1 */
		}

		/* check if the process may be sent SIGKILL.
		 */
		if (chkpskill(p))
		{
		    uaddr = (struct user *) vm_att(p->p_adspace, &U);
		    if (uaddr->U_start > youngest)
		    {
			youngest = uaddr->U_start;
                        killp = p;
                    }
                    vm_det(uaddr);
		}
	}

	/* send SIGKILL to the process if an eligible process
	 * was found.
	 */
	if (killp)
	{
		ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif

		killp->p_int |= SPSMKILL;

		pid = killp->p_pid;
		pidsig(pid,SIGKILL);

                p=PROCPTR(killp->p_ppid);

		/*
		 * Temporaritly raise the priority of the victim
		 * and its parent.
		 */
		{
			struct thread *t = killp->p_threadlist;
			do {
				if (t->t_state != TSNONE &&
				    t->t_state != TSIDL &&
				    t->t_state != TSZOMB)
					prio_requeue(t,1);
				t = t->t_nextthread;
			} while (t != killp->p_threadlist);

			t = p->p_threadlist;
			do {
				if (t->t_state != TSNONE &&
				    t->t_state != TSIDL &&
				    t->t_state != TSZOMB)
					prio_requeue(t,2);
				t = t->t_nextthread;
			} while (t != p->p_threadlist);
		}
                shmps = shmpsusage(killp);
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
		unlock_enable(ipri, &proc_base_lock);
                ps = v_pscount(killp->p_adspace) + shmps;

		/* log the fact that a process was signalled to be
		 * killed due to a low paging space condition.
		 * the amount of paging space in use by the process
		 * is recorded as number of 1K blocks since that is
		 * how the 'ps' command displays it for a process.
		 */
		uaddr = (struct user *) vm_att(killp->p_adspace, &U);
		bcopy(uaddr->U_comm, pgsplog.cmd, sizeof(pgsplog.cmd));
		vm_det(uaddr);
		pgsplog.pid = pid;
		pgsplog.amt = ps * 4;
		errsave(&pgsplog, sizeof(pgsplog));
	}

	return(pid);
}

/*
 * chkpskill(p)
 *
 * checks if a process may be sent the SIGKILL signal.
 * returns 1 if the process can be
 * sent the signal, otherwise, returns 0.
 *
 * description of checks:
 * SIGDANGER - processes which catch this are exempt from being killed
 * SRMSHM - make sure we don't select a process we selected previously
 * kprocs and the init process are exempt
 */

chkpskill(struct proc *p)
{

	if (p->p_stat != SNONE && p->p_stat != SIDL && p->p_stat != SZOMB && 
	    !(p->p_flag & (SPSEARLYALLOC|SKPROC)) && 
	    !SIGISMEMBER(p->p_sig,SIGKILL) && 
 	    !SIGISMEMBER(p->p_sigcatch,SIGDANGER) && 
	    (p->p_pid != INIT_PID) && !(p->p_int & SPSMKILL))
	{
		return(1);
	}

	return(0);
}

/*
 * shmpsusage
 *
 * return the paging space usage for shared memory segments
 */
static int
shmpsusage (struct proc *p)
{
	struct ublock	*ubarea;
	int		seg;
	int		shmps;

	if (! (p->p_flag & SLOAD))
		return 0;

	ubarea = vm_att (p->p_adspace, &__ublock);
	seg = 0;
	shmps = 0;
	while (seg < NSEGS)
	{
		if (ubarea->ub_user.U_segst[seg].segflag & (SEG_SHARED|SEG_WORKING))
			/* cannot access uarea->segst[seg].shm_ptr->shm_handle
			 * because the shared memory descriptors are not pinned,
			 * so use the sr found in the mstsave area instead.
			 * In the case of SEG_WORKING, the shm_ptr field has
			 * some other meaning; the code below works anyway.
			 */
			/* in the multithreaded case, each user thread shares
			 * the same address space, therefore we need only to
			 * find one of them.
			 */
		{
			struct thread *t = NULL;
			struct thread *th = p->p_threadlist;

			do {
			    if (!(th->t_flags & TKTHREAD)) {
				t = th;
				break;
			    }
			    th = th->t_nextthread;
			} while (th != p->p_threadlist);

			if (!t)
			    t = p->p_threadlist;

			if (t->t_uthreadp == &uthr0)
			    shmps+=v_pscount(SRTOSID(ubarea->
				ub_uthr0.ut_save.as.srval[seg]));
			else
			    shmps+=v_pscount(SRTOSID(ubarea->
				ub_uthr[th->t_uthreadp - &__ublock.ub_uthr[0]].
							ut_save.as.srval[seg]));
		}
		++seg;
	}
	vm_det (ubarea);

	return shmps;
}
