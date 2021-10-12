static char sccsid[] = "@(#)50	1.123.1.50  src/bos/kernel/proc/exit.c, sysproc, bos41J, 9519B_all 5/11/95 15:02:23";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: _exit
 *		freeuspace
 *		kexitx
 *		kwaitpid
 *                                                                    
 *   ORIGINS: 3, 26, 27, 83
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


#include <sys/param.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/proc.h>
#include <sys/lockl.h>
#include <sys/lock_alloc.h>
#include <sys/audit.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <sys/intr.h>
#include <sys/pseg.h>
#include <sys/seg.h>
#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/lock.h>
#include <sys/trchkid.h>
#include <sys/malloc.h>
#include <sys/low.h>
#include <sys/adspace.h>
#include <sys/atomic_op.h>
#include <mon.h>
#include "swapper_data.h"
#include "ld_data.h"
#include "sig.h"

/*                                                                   
 * EXTERNAL PROCEDURES CALLED: 
 */
extern void	pidsig();
extern void	pgsignal();
extern int	lockl();
extern void	unlockl();
extern void	swtch();
extern void	disown_fp();
extern void	ruadd();
extern struct	proch *proch_anchor;
extern int      proch_gen;
extern void	prof_off();		            /* turns off profiling */

int	*proc_event = (int *) EVENT_NULL;	    /* anchor of wait list */
pid_t 	kwaitpid( int *stat_loc, pid_t pid, int options,
	struct rusage *ru_loc );    /* common interface for all waitxxx    */
void	kexit(int wait_stat);       /* common interface for user and kernel*/
void  	freeuspace(struct user *up);/* free large data segments		   */
void	update_proc_slot();         /* check to put proc slot on free list */
void	orphan_pgrp();	            /* may orphan exiting process's pgrp   */
void	orphan_child_pgrp();        /* may orphan children's pgrp          */
void	schedexit();		    /* removes from to be suspended q      */

#define	INIT_PID	1		/* default parent pid		   */
#define NANOTOMICRO	1000

/*
 * NAME: _exit()
 *                                                                    
 * FUNCTION: 
 *	This entry point is the user system call for process exit.
 *	It picks handles auditing and transfers control	to kexit().
 *
 * EXECUTION ENVIRONMENT: 
 *	This routine may only be called by a process
 *      This routine may page fault
 *                                                                    
 * RETURN VALUE DESCRIPTION: 
 *	This code does not return.
 */  

void
_exit(int status)
/* int	status;			return value to parent routine	*/
{
	kexit((status & 0xff) << 8);
	/* control does not return */
}

/*
 * NAME: kexitx()
 *
 * FUNCTION: 
 *	This is the common exit code for process termination.
 *	It can be called directly from signal code, and it is called
 *	by the system call _exit().
 *
 * EXECUTION ENVIRONMENT: 
 *	This routine may only be called by a process
 *      This routine may page fault
 *
 * NOTES: 
 *	This subroutine will release the process' resources,
 *	enter the Zombie state, wake up the parent and init processes,
 *	and dispose of children.
 *
 * RETURN VALUE DESCRIPTION: 
 *	This routine does NOT return.
 */

void
kexitx(int wait_stat)
/* int wait_stat;		return value to parent		*/
{
	struct proc	*p, *q, *r;	/* proc table pointers		*/
	struct thread	*t, *th, *tzomb=NULL;/* thread table pointers	*/
	struct proch	*proch;
	long		ixrss;		/* shared memory size		*/
	long		idrss;		/* unshared data size		*/
	int		dbgpid;		/* debugger if not parent	*/
	int		old_gen;	/* saved proch_gen */
	int		ipri;
	static int	svcnum = 0;

	t = curthread;			/* get current thread pointer	*/
	p = t->t_procp;			/* get current proc pointer	*/

        /* We shouldn't be on an event list or have any locks */ 
        assert(t->t_eventlist == NULL);
        assert(t->t_lockcount == 0);

	/* Await death of all other threads belonging to the process */
	if (p->p_threadcount > 1 || t->t_uthreadp != &uthr0 ) {

		ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif
        	/*
         	 * fork, exec, and exit are serialized by the forkstack.  If
         	 * we have gotten this far, then a pending exec/exit in the
	 	 * same process will be negated by setting STERM.
         	 */
		p->p_int |= STERM;
		p->p_flag |= SEXIT;

		if (p->p_active > 1)
		{
			/* The process may be partially stopped/suspended */
			cont(p);

			/* The process may be partially swapped out */
                        schedsig(p);

			t->t_flags |= TTERM;

			e_wakeup((int *)&p->p_synch);	/* STERM set */
			e_wakeup((int *)&t->t_synch);   /* TTERM set */

			/* Wake up the threads sleeping interruptibly */
			th = t->t_nextthread; /* skip oneself */
                        while (th != t) {
                                switch (th->t_wtype) {
                                case TWCPU :
                                case TNOWAIT :
                                        break;
                                case TWPGIN :
                                        vcs_interrupt(th);
                                        break;
                                default :
                                        if (th->t_flags & TWAKEONSIG) {
                                           if (th->t_state == TSSLEEP)
                                               setrq(th, E_WKX_PREEMPT, RQHEAD);
                                           else
                                               th->t_wtype = TNOWAIT;
                                        }
                                        break;
                                }
                                th = th->t_nextthread;
                        }

			/* Wait for the other threads to terminate themselves */
			p->p_suspended++;
			while (p->p_suspended < p->p_active) {
#ifdef _POWER_MP
				simple_unlock(&proc_int_lock);
#endif
				e_sleep_thread((int *)&p->p_synch,
					&proc_base_lock, LOCK_HANDLER);
#ifdef _POWER_MP
				simple_lock(&proc_int_lock);
#endif
			}
			p->p_suspended--;
			ASSERT(p->p_active == 1);
		}

		/* Orphan all other threads belonging to the process */
		if (p->p_threadcount > 1) {
			tzomb = NULL;

			/* Keep anchors */
			p->p_threadlist = t;
			tzomb = t->t_nextthread;

			/* Remove oneself from the fellow zombies */
			tzomb->t_prevthread = t->t_prevthread;
			t->t_prevthread->t_nextthread = tzomb;

			/* Remove the zombies from the threadlist */
			t->t_nextthread = t->t_prevthread = t;

#ifdef DEBUG
			/* Check the zombies */
			th = tzomb;
			do {
				switch (th->t_state) {
				case TSZOMB:
				case TSIDL:
		    			p->p_threadcount--;
					break;
				default :
					assert(FALSE);
				}
				th = th->t_nextthread;
			} while (th != tzomb);
			assert(p->p_threadcount == 1);
#else
			p->p_threadcount = 1;
#endif
		}

		/* Switch to the standard uthread area if not already there */
		if (t->t_uthreadp != &uthr0)
		{
			/* copy the current uthread */
			bcopy(t->t_uthreadp, &uthr0, sizeof(struct uthread));
			/* switch */
			t->t_uthreadp = &uthr0;
			SET_CSA(&uthr0.ut_save);

		}

		/* Reset so that subsequent sleeps will not return early. */
		p->p_int &= ~STERM;
		t->t_flags &= ~TTERM;

#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
		unlock_enable(ipri, &proc_base_lock);

	} else {
		p->p_flag |= SEXIT;
	}

	/* Clear signal state since there are several places that may sleep */
	SIGINITSET(p->p_sig);
	SIGINITSET(t->t_sig);
	t->t_flags &= ~TSIGAVAIL;
	 
	/* Release uthread table control block and kernel stacks segment. */
	pm_release(&U.U_uthread_cb);
	if (p->p_kstackseg != NULLSEGVAL)
	{
                pm_release(&U.U_cancel_cb);
		vms_delete(SRTOSID(p->p_kstackseg));
		p->p_kstackseg = NULLSEGVAL;
		uthr0.ut_kstack = (char *)&__ublock;
	}

	/*
	 * Clean up the zombies.
	 *
	 * Note: While all other state transitions are protected by the
	 * proc_int_lock, TSZOMB->TSNONE is protected by the proc_tbl_lock
	 * for the scheduler.
	 */
	if (tzomb != NULL) {
		struct thread *thr;

		simple_lock(&proc_tbl_lock);
		th = tzomb;
		do {
			thr = th;
			th = th->t_nextthread;
			freethread(thr);
		} while (th != tzomb);
		simple_unlock(&proc_tbl_lock);
	}

	TRCHKGT_SYSC(_EXIT,wait_stat,t->t_tid,t->t_suspend,NULL,NULL);

	if(p->p_pid == INIT_PID)
		assert((wait_stat == 0) && (p->p_child == NULL));

        /* 
	 * Pick this up regardless of possible audit
	 * suspension 
	 */
	if (t->t_uthreadp->ut_audsvc)
		t->t_uthreadp->ut_audsvc->svcnum = 1;

        if(audit_flag && audit_svcstart("PROC_Delete", &svcnum, 1, p->p_pid)){

		/* 
		 * Commit immediately
		 */

		audit_svcfinis();
                auditscall();
	}

        /*
         * Loop through the registered resource handlers
         * Start over if the list changes.  The serialization of this
         * list is strange because the resource handlers can sleep
         * causing deadlock problems if any other lock but the kernel
         * lock is used.  Since the kernel lock may be given up, the
         * list can change which is why the generation count is used.
         * Both of these techniques must be used to ensure list integrity.
         */
        do {
		(void) lockl(&kernel_lock, LOCK_SHORT);
                old_gen = proch_gen;
                for (proch = proch_anchor; proch != NULL; proch = proch->next) 
		{
                        /* call resource handler with parms */
                        (proch->handler)(proch, PROCH_TERMINATE, p->p_pid);
			(proch->handler)(proch, THREAD_TERMINATE, t->t_tid);
                        if (proch_gen != old_gen)
                                /* sombody changed the list, don't continue */
                                break;
		}
	} while (proch_gen != old_gen);
        unlockl(&kernel_lock); 	       		/* unlock kernel lock   */

        ASSERT(t->t_lockcount == 0);

	/* Remove shared memory segments */
	if (p->p_int & SPSMKILL)
		psrmshm (p);

	/*
	 * Perform uprintf termination stuff. This can sleep so
	 * don't call this routine while holding locks. It contains
	 * its own serialization.
	 */
	upfexit(p);

	disown_fp(t->t_tid);	/* Release floating point H/W		*/

	/* unlock text and data if locked */
	if (U.U_lock & (PROCLOCK | DATLOCK | TXTLOCK))
		plock(UNLOCK);

	fs_exit();		/* close open files			*/

        ASSERT(t->t_lockcount == 0);

	/* release the audit buffers */
	if (t->t_uthreadp->ut_audsvc) {
		if (t->t_uthreadp->ut_audsvc->audbuf)
			free(t->t_uthreadp->ut_audsvc->audbuf);
		xmfree((void *)t->t_uthreadp->ut_audsvc, kernel_heap);
		t->t_uthreadp->ut_audsvc = NULL;
	}

	/* if process was profiling, free profiling resources */
	if (U.U_prof.pr_scale)
		prof_off();
				
	/* release mmap resources -- must be done before shmex() */
	if (U.U_map) vm_map_deallocate(U.U_map);

	shmex();		/* detach shared memory			*/
	semexit();		/* release semaphores			*/
	acctexit(wait_stat);	/* disables accounting routines		*/

	/*
	 * Clean up outstanding timer requests
	 */
	texit(&U, &uthr0);

	/* update ru_maxrss resource */
	idrss = 4 * vms_rusage(U.U_adspace.srval[PRIVSEG]);
	idrss += 4 * bd_rusage(&U);
	ixrss = 4 * vms_rusage(U.U_adspace.srval[TEXTSEG]);

 	/* Only update if no error from vms_rusage */
	if (idrss >= 0 && ixrss >= 0 && (idrss + ixrss) > U.U_ru.ru_maxrss)
			U.U_ru.ru_maxrss = idrss + ixrss;

	/*
	 * Save needed info from the user structure into process table.
	 * - store termination or signal status in the proc table	
	 * - load parent's and add child's resource usage
	 */
	p->xp_stat = wait_stat;
	p->xp_ru = U.U_ru;
	p->xp_ru.ru_majflt = p->p_majflt;
	p->xp_ru.ru_minflt = p->p_minflt;

	ruadd(&p->xp_ru, &U.U_cru);

	/*
	 * Free memory.
	 *   ld_usecount ----> text, files, and shared libraries 
         *   freeuspace  ----> user data in big data segments
	 *   vm_releasep ----> user data in process private segment.
	 *		       user stack cannot be deleted because kprocs
	 *		       run on the user stack.  This call is the only 
	 *		       one that necessarily frees memory, which is 
	 *		       needed to minimize the number of jobs that 
	 *		       get killed when paging space is low.
	 */
	ld_usecount(-1);		
        ASSERT(t->t_lockcount == 0);
	freeuspace(NULL);
	vm_releasep(p->p_adspace, 0, U.U_dsize/PAGESIZE);  

	/* if exiting proc is a debugger, kill processes it is tracing  */
        simple_lock(&ptrace_lock);
	if (p->p_flag & STRACING)
		ptrace_kill(p);

        dbgpid = 0;
retry:
        if (p->p_flag & STRC) {
                dbgpid = p->p_ipc->ip_dbp->p_pid;
                if (p->p_ppid == dbgpid) {
                        /*
                         * Parent is the debugger, detach from it.
                         * If being debugged by non-parent
                         * kwaitpid() will do the detach.
                         * A negative return indicates that we
                         * sleept in ptrace_detach, the system
                         * state could have changed, need to retry
                         * the whole operation.
                         */
                        dbgpid = 0;
                        if (ptrace_detach(p) < 0)
                                goto retry;
                }
        }
        simple_unlock(&ptrace_lock);

	if (U.U_cred) crfree(U.U_cred);	/* discard credentials struct	*/

	/*
	 * Disable to ensure that the clock won't interrupt us, which
	 * allows us to free the locks in the ublock.
	 */
	ipri = i_disable(INTMAX);

	lock_free(&U.U_handy_lock);
	lock_free(&U.U_timer_lock);

	/*
	 * The proc_base_lock/proc_int_lock is acquired here to avoid 
	 * getting and releasing them below in pgsignal, pidsig, and 
	 * orphan_pgrp.  In addition, there is a reference to p_sigignore.
	 */
#ifdef _POWER_MP
	simple_lock(&proc_base_lock);
	simple_lock(&proc_int_lock);
#endif

	/* 
	 * session leader relinquishing the controlling tty sends a SIGHUP
	 * to the foreground process group and frees the controlling terminal.
	 */
	if (p->p_pid == p->p_sid) {
		if (U.U_ttyp && *U.U_ttysid == p->p_sid) {

			/* signal foreground proc group */
			if (*U.U_ttyp)
				pgsignal(*U.U_ttyp, SIGHUP);
		
			/*
			 * fs_exit() does not relinquish the controlling
			 * tty if any process has the tty open.
			 */
			if (*U.U_ttysid == p->p_sid) {
				/* fg pgrp before relinquishing tty; store 0 */
				*((volatile pid_t *)U.U_ttyp) = (pid_t)0;
		
				/* relinquish control of tty.  store 0 */
				*((volatile pid_t *)U.U_ttysid) = (pid_t)0;
			}
		}
	}
	if (p->p_sid) {

		/*
		 * Send SIGHUP followed by SIGCONT to newly orphaned
		 * process groups, if any member is stopped.  Also
		 * the exiting process is removed from the p_pgrpl 
		 * chain by the parent in waitpid().  
		 */
		orphan_pgrp(p);		/* may orphan process group	*/
		orphan_child_pgrp(p);	/* may orphan a child's pgrp	*/
	}

	/* reassign all children, child replaced by siblings each pass	*/
	for (r = PROCPTR(INIT_PID), q = p->p_child; q != NULL; q = p->p_child)
	{
		/* bequeath this child to INIT */

		q->p_ppid = INIT_PID;
		p->p_child = q->p_siblings;
		q->p_siblings = r->p_child;
		r->p_child = q;

		/* if child is a zombie, signal INIT to cleanup	*/
		if (q->p_stat == SZOMB) {
			pidsig(INIT_PID, SIGCHLD);
			ep_post(EVENT_CHILD, INIT_PID);
		}

		/* if child stopped/stopping, send it a hangup and continue */
		if (q->p_stat == SSTOP || q->p_int & SSUSP)
		{
			pidsig(q->p_pid, SIGHUP);
			pidsig(q->p_pid, SIGCONT);
		}
	}

	/* parent ignoring death of child */
	if (SIGISMEMBER((PROCPTR(p->p_ppid))->p_sigignore, SIGCHLD))
	{
		/* remove this process from parent's children list */
		if ((q = (PROCPTR(p->p_ppid))->p_child) == p)
			(PROCPTR(p->p_ppid))->p_child = p->p_siblings;
		else
			for (r = q, q = q->p_siblings; q != NULL; r = q,
							q = q->p_siblings)
				if (q == p)
				{
					r->p_siblings = q->p_siblings;
					break;
				}
		/*
		 * The SLOAD flag will be checked by the scheduler
		 * when scanning the process table to see if a 
		 * zombie's resources can be freed up.
		 */
		p->p_flag &= ~SLOAD;
	}

	/* send parent death of child signal. */
	pidsig(p->p_ppid, SIGCHLD);
	ep_post(EVENT_CHILD, p->p_ppid);

        if (dbgpid) {
                /*
                 * Post the debugger in the event that it is not
                 * the parent.  This allows it to get the exit status.
                 */
                pidsig(dbgpid, SIGCHLD);
                ep_post(EVENT_CHILD, dbgpid);
        }

	/* 
	 * Removes from to_be_suspended_q, which is necessary because
	 * signals are delivered before a process is suspended.
	 */
	schedexit(p);			

	/* Change state */
	t->t_state = TSZOMB;			/* mark as zombie */
	p->p_active--;
	ASSERT(p->p_active == 0);
	p->p_stat = SZOMB;			/* mark as zombie */

	/* Increment kernel process exit counter */
        if (p->p_flag & SKPROC)
	    sysinfo.kexit++;

#ifdef _POWER_MP
	simple_unlock(&proc_int_lock);
        assert(t->t_lockcount == 1); /* 1 to account for the proc_base_lock */
        locked_swtch();                 /* choose another thread to run */
#else
        assert(t->t_lockcount == 0);
        swtch();                        /* choose another thread to run */
#endif
	/* Does not return */
}

/*
 * NAME: kwaitpid()
 *                                                                    
 * FUNCTION: 
 *	This function incorporates all the functionality of wait(),
 *	wait3(), and waitpid().  It is identical to the waitpid() call
 *	with the addition of the 4th argument "ru_loc" to include the
 *	wait3() function for copying out the resource usage information.
 *	The rusage argument is only utilized when a zombie process is 
 *	found and the argument is not NULL. In this case, the usage info 
 *	is copied to the user supplied structure location, "ru_loc".
 *	See the waitpid() FUNCTION description for complete details.
 *
 * EXECUTION ENVIRONMENT: 
 *	This routine may only be called by a process
 *      This routine disables interrupts and cannot page fault. 
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *	 0	= Terminating or stopped process not found
 *	 number = Process ID of terminating or stopped process
 *	-1	= failed, errno indicates cause of failure
 */ 

pid_t
kwaitpid( int *stat_loc, pid_t pid, int options, struct rusage *ru_loc )
/* int		*stat_loc;	user location for returned status	*/
/* pid_t	pid;		pid value, -1, 0, -process group pid	*/
/* int		options;	options to vary function, see wait.h	*/
/* struct rusage *ru_loc;	pointer to child resource usage area	*/
{
	struct proc 	*p, *valid_sib;	/* proc table pointers		*/
	pid_t		retval;		/* routine return value		*/
	int		wstat;		/* used for termination status	*/
	int		lockt;		/* original lock return value	*/
	int		wait_satisfied;	/* wait loop flag		*/
	int		found_child;	/* found valid child pid	*/
	int		sleep_rtn;	/* sleep call return value	*/
	int		ipri;		/* saved interrupt priority	*/
	struct rusage   tmp_ru;         /* temp storage                 */
	struct proc	*dbp;		/* debugger pid			*/
	struct proc	*cp;		/* current process		*/
	struct thread	*t;		/* current thread		*/

	retval = 0;			/* set default return code	*/
	wstat = 0;			/* set default status code	*/

	t = curthread;
	cp = t->t_procp;

	/* check for valid option flags					*/
	if ( options & ~(WNOHANG | WUNTRACED) )
	{
		u.u_error = EINVAL;
		return ( -1 );
	}

	wait_satisfied = 0;		/* default: no stopped child	*/

	/* search through children, look for zombies or traced children	*/

	ipri = disable_lock(INTMAX, &proc_base_lock);

	for (;;)	/* repeat until: found, no children, no WNOHANG	*/
	{
		found_child = 0;	/* default: no valid child	*/

		/* If this process is a debugger, look at those processes
		 * it is debugging first. This may include children, but
		 * may not.  exit() will take the exiting traced child 
		 * process off the p_debugl chain iff the parent is the 
		 * debugger.  Therefore, the zombie process will be found 
		 * by the parent (in this case the debugger) in the second 
		 * loop and will be cleaned up.  Otherwise, (the debugger 
		 * is not the parent) we want to return status to the 
		 * debugger in the case the child has exited or stopped.
		 */
		if (cp->p_flag & STRACING) {

			for (p = cp->p_dblist; p; p = p->p_dbnext)	
			{
				/*skip children which don't match pid function*/
	
				if (pid > 0 && p->p_pid != pid)
					continue;
				if (pid < -1 && p->p_pgrp != -pid) 
					continue;
				if (pid == 0 && cp->p_pgrp != p->p_pgrp)
					continue;

				/* successful search */
				found_child = 1;

				/* found zombie */
				if(p->p_stat == SZOMB)
				{
					tmp_ru = p->xp_ru;
					wstat = p->xp_stat;
					retval = p->p_pid;

                                        /* don't cleanup proc tbl entry */
					wait_satisfied = 2;
					goto leave;
				}

				/* stopped and not reported */
				if(p->p_stat == SSTOP && !(p->p_atomic & SWTED))
				{
#ifdef _POWER_MP
					fetch_and_or((int *)&p->p_atomic, SWTED);
#else
					p->p_atomic |= SWTED;
#endif

					wstat = (p->xp_stat << 8) | W_STOPPED;
					ASSERT(p->p_flag & STRC);

					if (p->p_atomic & SFWTED)
						wstat |= W_SFWTED;
					else if (p->p_atomic & SEWTED)
						wstat |= W_SEWTED;
					else if (p->p_atomic & SLWTED)
						wstat |= W_SLWTED;
					else
						wstat |= W_STRC;

#ifdef _POWER_MP
					fetch_and_and((int *)&p->p_atomic,
						~(SEWTED|SFWTED|SLWTED));
#else
					p->p_atomic &= ~(SEWTED|SFWTED|SLWTED);
#endif

					tmp_ru = p->xp_ru;
					retval = p->p_pid;

					/* don't cleanup proc tbl entry */
					wait_satisfied = 2;
					goto leave;
				}
			}
		}

		/* 
		 * look at proc's that are children of this process	
		 */
		for (p = cp->p_child; p != NULL; valid_sib= p, p= p->p_siblings)
		{
			/* skip children which don't match pid function	*/

			if (pid > 0 && p->p_pid != pid)
				continue;
			if (pid < -1 && p->p_pgrp != -pid) 
				continue;
			if (pid == 0 && cp->p_pgrp != p->p_pgrp)
				continue;

			found_child = 1;	/* successful search	*/

			/* 
			 * Found zombie that has not been reported to
			 * another thread in the same process.  
			 */
			if (p->p_stat == SZOMB)
			{
				/*
				 * Accumulate child's statistics
				 */
				ruadd(&U.U_cru, &p->xp_ru );
				curthread->t_cpu = MIN(T_CPU_MAX,
							curthread->t_cpu +
/*???*/							p->p_threadlist->t_cpu);

				/*
				 * Separate child from siblings
				 */
				if (cp->p_child == p)
					cp->p_child = p->p_siblings;
				else
					valid_sib->p_siblings = p->p_siblings;

				if (p->p_flag & STRC) {
					/*
					 * Clear child's statistics so that
					 * they don't get accounted twice
					 * (do we really care?).
					 */
					bzero(&p->xp_ru, sizeof(p->xp_ru));
/*???*/					p->p_threadlist->t_cpu = 0;

                                        /*
                                         * Process remains on debugger list.
                                         * Scheduler cleans up process after
                                         * debugger detaches from it.
                                         */
                                        p->p_flag &= ~SLOAD;

					/* don't cleanup proc tbl entry */
					wait_satisfied = 2;
				}

				else {
					/* cleanup proc table entry */
					wait_satisfied = 1;
				}

				tmp_ru = p->xp_ru;
				wstat  = p->xp_stat;
				retval = p->p_pid;
				break;

			}	/* end, zombie found			*/
			
			/*
			 * Process has been stopped and has not been reported
			 * by a previous waitpid call.
			 *
			 * SWTED is used to communicate between stop() and
			 * kwaitpid().  Have we looked at this pid already.
			 */
			if ((p->p_stat == SSTOP) && (options & WUNTRACED) &&
			    !(p->p_atomic & SWTED) && !(p->p_flag & STRC))
			{
#ifdef _POWER_MP
				fetch_and_or((int *)&p->p_atomic, SWTED);
#else
				p->p_atomic |= SWTED;
#endif

				wstat = (p->xp_stat << 8) | W_STOPPED;
				tmp_ru = p->xp_ru;
				retval = p->p_pid;

				/* don't cleanup proc_tbl_entry */
				wait_satisfied = 2;
				break;
			}
		}	/* end of child search for loop			*/

leave:		
		/* if no children matching pid parameter, error exit	*/
		if (found_child == 0)
		{
			unlock_enable(ipri, &proc_base_lock);
			u.u_error = ECHILD;
			return(-1);
		}

		/* determine if this is the last pass			*/
		if (wait_satisfied || (options & WNOHANG))
		{
			break;
		}

                /*
                 * record local threads sleeping
                 */
                if (t->t_flags & TLOCAL)
                        cp->p_local--;
                ASSERT(cp->p_local >= 0);

		/*
		 * if no zombies, no stopped children and no traced children,
		 * sleep until awakened by ptrace(), or exit().  sleep till
		 * awakened by a signal, bail out.  otherwise, reacquire the
		 * proc_base_lock and go zombie hunting again.  
		 */
		t->t_flags |= TWAKEONSIG;
        	t->t_wevent |= EVENT_CHILD;
        	t->t_wtype = TWEVENT;

		/*
		 * The proc_base_lock needs to be held into the dispatcher
		 * for interruptible sleeps to forestall intervening SIGSTOPs
		 * and SIGCONTs to the process.  Otherwise the thread will
		 * be put onto the runqueue twice.  The first time by cont()
		 * the second time by the dispatcher.
		 */
#ifdef _POWER_MP
                if (t->t_flags & TWAKEONSIG) 
                        locked_swtch();
                else {
                        simple_unlock(&proc_base_lock);
                        swtch();
                }
                simple_lock(&proc_base_lock);
#else   
                swtch(); 
#endif

		t->t_flags &= ~TWAKEONSIG;
        	t->t_wevent &= ~EVENT_CHILD;

                if (t->t_flags & TLOCAL)
                        cp->p_local++;

		if (SIG_MAYBE(t,cp) && check_sig(t, 1)) 
		{
			u.u_error = ERESTART;
			retval = -1;
			break;
		}
	}	/* end, for loop until satisfied			*/

	unlock_enable(ipri, &proc_base_lock);

        if (wait_satisfied || (options & WNOHANG))
        {
		if ((cp->p_flag & STRACING) && 
               	    (p->p_flag & STRC) && (p->p_stat == SZOMB))
                {
                       	/*
                         * Assert ptrace_detach() will not fail:
                         *  - the debugger is running (curproc)
                         *  - traced process is zombie
                         * The only reason for sleeping in
                         * ptrace_detach would be if the ptipc
                         * struct was busy, the only way for it
                         * to be busy at this time would be by
                         * a PT_REATT by a third process. That
                         * request would fail before making it
                         * busy because the debuggee is exiting.
                         */
                        simple_lock(&ptrace_lock);
                        assert(ptrace_detach(p) == 0);
                        simple_unlock(&ptrace_lock);
                }

                if (wait_satisfied == 1)
                {
                        if (p->p_stat == SZOMB) {
                                unsigned long srval;

                                srval = NULLSEGVAL;
                                simple_lock(&proc_tbl_lock);
                                if (p->p_stat == SZOMB) {
                                        if (p->p_pgrp)
                                                update_proc_slot(p);
                                        srval = p->p_adspace;
                                        p->p_adspace = NULLSEGVAL;
                                        freeproc(p);
                                }
                                simple_unlock(&proc_tbl_lock);

                                if (srval != NULLSEGVAL)
                                        vms_delete(SRTOSID(srval));
                        }
                }
       	}

	if (stat_loc != NULL) {
		if (copyout((caddr_t)&wstat, (caddr_t)stat_loc, sizeof(wstat)))
		{
			u.u_error = EFAULT;
			retval = -1;
		}
	}

	if (ru_loc != NULL) {
		tmp_ru.ru_utime.tv_usec /= NANOTOMICRO;
		tmp_ru.ru_stime.tv_usec /= NANOTOMICRO;
		if (copyout( (caddr_t)&tmp_ru, (caddr_t)ru_loc,
							sizeof(struct rusage)))
		{
			u.u_error = EFAULT;
			retval = -1;
		}
	}

 	TRCHKGT_SYSC(WAIT,retval,pid,p->p_flag,wstat,NULL);

	return(retval);
}

/*
 * NAME: freeuspace()
 *                                                                    
 * FUNCTION: Destroy secondary working storage segments.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine may page fault
 *
 * NOTE: The process private segment is destroyed in the parent process
 *      (or init) from wait or in the swapper process. It cannot be destroyed
 *      in the context of the caller since the ublock and kstack would vanish.
 */

void
freeuspace(struct user *up)
{
	int i;
        struct segstate *sp;
	struct loader_anchor *la;
	ulong segmentarray[NSEGS];
	ulong *segments;

	segments = segmentarray;

        if (!up)
                up = &U;

	simple_lock(&proc_tbl_lock);

        /* loop through candidates for destruction */
        for (i = BDATASEG, sp = &up->U_segst[i]; i <= BDATASEGMAX; i++, sp++)
        {
                /* terminate on first non-working segment */
                if (!(sp->segflag & SEG_WORKING))
                        break;

                /*
                * Detach from the segment in the user's address space
                * descriptor and delete the segment.
                */
                if (up->U_adspace.alloc & ((unsigned)0x80000000>>i))
                        as_det(&up->U_adspace,i<<SEGSHIFT);
		if (sp->ss_srval) {
			up->U_adspace.srval[i] = NULLSEGVAL;
			*segments = SRTOSID(sp->ss_srval);
			segments++;
		}

                sp->ss_srval = 0;
                sp->segflag = 0;
                sp->num_segs = 0;
        }

        /* Check for existence of a per-process shared library data segment.
         * If one exists,  delete the segment.  This segment register will
         * not change because we are single threaded.
         */
        if (up->U_adspace.alloc & ((unsigned)0x80000000 >> SHDATASEG)) {
                vmhandle_t srval = as_getsrval(&up->U_adspace, SHDATAORG);
                as_det(&up->U_adspace, SHDATAORG);
		up->U_adspace.srval[SHDATAORG] = NULLSEGVAL;
		*segments = SRTOSID(srval);
		segments++;
        }

        /* Delete overflow segment if one exists */
        la = (struct loader_anchor *)up->U_loader;
        if (OVFL_EXISTS(la)) {
		*segments = SRTOSID(la->la_ovfl_srval);
		segments++;
                la->la_ovfl_srval = NULLSEGVAL;
                la->la_save_srval = NULLSEGVAL;
                la->la_flags &= ~(LA_OVFL_HEAP|LA_OVFLSR_ALLOCED|LA_OVFL_ATT);
        }

	simple_unlock(&proc_tbl_lock);

	/*
	 * Delete segments after releasing lock.
	 */
	segments--;
	for (; segments >= segmentarray; segments--) 
		vms_delete(*segments);
}
