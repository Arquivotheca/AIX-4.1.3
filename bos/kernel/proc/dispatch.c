static char sccsid[] = "@(#)46	1.62.1.12  src/bos/kernel/proc/dispatch.c, sysproc, bos41J, 9519A_all 5/5/95 15:55:43";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: bindprocessor
 *		dispatch
 *		mycpu
 *		remrq
 *		setrq
 *		switch_cpu
 *		waitproc
 *
 *   ORIGINS: 27, 83
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
#include <sys/proc.h>
#include <sys/pri.h>
#include <sys/var.h>
#include <sys/sched.h>
#include <sys/sysinfo.h>
#include <sys/syspest.h>
#include <sys/systm.h>
#include <sys/intr.h>
#include <sys/trchkid.h>
#include <sys/user.h>
#include <sys/sleep.h>
#include <sys/systemcfg.h>
#include <sys/errno.h>
#include <sys/id.h>
#ifdef PM_SUPPORT
#include <sys/pm.h>
#endif /* PM_SUPPORT */
#include "swapper_data.h"

struct ready_queue_flags run_mask;	/* ready queue flags	*/
struct thread *thread_run[PMASK+1];	/* ready to run queue	*/

#ifdef _POWER_MP
int affinity_priodelta = 0;
int affinity_scandelta = 0;
int affinity_skipboosted = FALSE;
#endif


/*
 * NAME: setrq
 *
 * FUNCTI0N: Chain a thread structure into the dispatcher ready queue, based
 *	     on the thread priority.
 *
 * EXECUTI0N ENVIR0NMENT:
 *
 *           This routine can only be called from within a disabled critical
 *           section.
 *           This function cannot page fault.
 *
 * N0TES:
 *           The thread that is to be put on the ready queue must not have
 *           t_wtype == SWCPU.
 *
 * REC0VERY 0PERATI0N:
 *           None.
 *
 * DATA STRUCTURES:
 *           runrun is set to 1, which forces a call to dispatch().
 *           modifies thread_run array
 *
 *
 * RETURNS:  N0NE
 */

void
setrq(struct thread *t, int flags, int where)
{
	register struct thread	**rq;	/* ready queue head */
	register int		lvl;	/* ready thread's priority level */
#ifdef _POWER_MP
	register int i;
	int ncpus = NCPUS();		/* number of running cpus */
#endif

	/*
	 * trace call to setrq()
	 */
	TRCHKL4T(HKWD_KERN_SORQ,t->t_procp->p_pid,t->t_tid,t->t_pri,
		 t->t_policy);

	ASSERT( t->t_wtype != TWCPU );
	ASSERT( t->t_procp->p_flag & SLOAD );
	ASSERT( csa->intpri == INTMAX );
#ifdef _POWER_MP
	ASSERT( lock_mine(&proc_int_lock) );
#ifdef DEBUG
	for (i = 0; i < ncpus; i++) {
		assert(t != ppda[i]._curthread || i == CPUID);
	}
#endif
#endif

	/* Assumption : if at least one of your thread is put on a run queue,
		then you can no longer be stopped or swapped out */
	t->t_procp->p_stat = SACTIVE;	/* process is active */
	t->t_state = TSRUN;		/* thread is runnable */

        /*
         * When power management is active, don't put threads on the
         * runqueue.  Instead change state to TNOWAIT.
         */
        if ((t->t_flags & TPMREQSTOP) && (t->t_suspend == 0)) {
                t->t_flags |= TPMSTOP;
                t->t_wtype = TNOWAIT;
        }
        else {
                t->t_wtype = TWCPU;             /* waiting for CPU */

                lvl = t->t_pri & PMASK;      /* ready thread's priority level */
                rq = &thread_run[lvl];            /* ready queue head pointer */

                if (*rq) {                   /* queue not empty? Add new thread
                                                between tail and head */
                        t->t_prior = *rq;                /* put it after tail */
                        t->t_next = (*rq)->t_next;      /* put it before head */

                        (t->t_prior)->t_next = t;             /* update links */
                        (t->t_next)->t_prior = t;

                        if (where == RQTAIL)            /* was to be put last */
                                *rq = t;             /* therefore is new tail */
                                                     /* otherwise is new head */
                }
                else {                     /* first ready thread on this list */
                        t->t_next = t;              /* this is the only entry */
                        t->t_prior = t;
                        *rq = t;
                        run_mask.word[lvl/BITS_PER_WORD]|=MASKBIT(lvl);
                }

                /*
                 * Set "runrun" flag if this event is interesting:
                 * i.e., if the priority of the newly-ready thread
                 * is more favored than that of one of the current
                 * thread.  0therwise, dispatch() will select
                 * curthread anyway, so don't waste time calling it.
                 */
#ifdef _POWER_MP
                if ((flags != E_WKX_NO_PREEMPT)  && (runrun < ncpus)) {
                        for (i = 0; i < ncpus; i++) {
                              if ((ppda[i]._curthread->t_pri & PMASK) > lvl) {
                                  runrun++;    /* INC_RUNRUN(1) to dispatch() */
                                  break;
                              }
                        }
                }
#else
                if (((curthread->t_pri & PMASK) > lvl) &&
                     (flags != E_WKX_NO_PREEMPT))
                        INC_RUNRUN(1);            /* force call to dispatch() */
#endif
        }                                                     /* if pm_active */
}


/*
 * NAME: remrq
 *
 * FUNCTI0N: Remove thread entry from ready queue.
 *
 * EXECUTI0N ENVIR0NMENT:
 *
 *           This procedure can be called by a thread or an interrupt handler
 *           with interrupts disabled.
 *           This function cannot page fault.
 *
 * N0TES:
 *           None.
 *
 * REC0VERY 0PERATI0N:
 *           None.
 *
 * DATA STRUCTURES:
 *           None.
 *
 * RETURNS:  N0NE
 */

void
remrq(struct thread *t)
{
	register struct thread	**rq;	/* ready queue head */
	register int		lvl;	/* ready thread's priority level */

	ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
	ASSERT( lock_mine(&proc_int_lock) );
#endif

	if (t->t_wtype == TWCPU) {	/* thread on ready queue? */

		t->t_wtype = TNOWAIT;	/* no longer waiting for CPU */
		lvl = t->t_pri & PMASK;	/* thread's priority level */
		rq = &thread_run[lvl];	/* ready queue head pointer */
		ASSERT(*rq);		/* should be something there */

		if (t->t_next == t) {	/* only one process in this ready q */
			run_mask.word[lvl/BITS_PER_WORD] &= ~MASKBIT(lvl);
			*rq = NULL;
		}
		else {
			(t->t_prior)->t_next = t->t_next;
			(t->t_next)->t_prior = t->t_prior;
			if (*rq == t)
				*rq = t->t_prior;
		}
	}

}
 

/*
 * NAME: dispatch
 *
 * FUNCTION: Select the higest priority ready thread and dispatch it,
 *           ie. make it the current running thread.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           This routine must be called with interrupts disabled and
 *           the stack in memory, ie. a critical section.
 *
 * NOTES:
 *           This function is called from the various first level interrupt
 *           handlers and from unready(), all in low.s, to select a new
 *           thread.
 *
 *           dispatch() returns to its caller, passing a pointer to the
 *           process structure of the process owning the selected thread.
 *           The "resume" function in low.s will cause the selected thread
 *           to run.
 *
 *           When dispatch() is called, it needs to put the current thread
 *           back on the ready queue if it is still in the SRUN state.
 *           The current thread may still be the highest priority ready
 *           thread, but it's easier to simply let things flow than bother
 *           with exception checking for this case.
 *
 *	     If the current thread is no longer in the TSRUN state, it can
 *	     be in the TSSLEEP or TSSTOP state, from which it won't move
 *	     as long as we are in the critical region (INTMAX/proc_int_lock).
 * 	     However, it can also be in the TSZOMB state from which it can
 *	     be moved without the proc_int_lock.  The freeproc/freethread 
 *	     are serialized with the proc_tbl_lock.  On a UP machine, this
 *	     will not happen, because of the INTMAX critical section.  On
 *	     an MP machine the TSNONE state may be seen, but state management 
 *	     in general only occurs under the context of the thread so this
 *	     exception is considered incidental as long it is reinialized
 * 	     when the thread is started for the first time.  For example,
 *	     t_lockcount may be decremented to -1, but it is only checked 
 *	     under the thread, so it doesn't hurt anything.  However,
 *	     state management is resynchronized with the transition from
 *	     TSNONE to TSIDL which is managed through the proc_int_lock.  
 *	     Note this problem does not exist for multi threaded processes, 
 *	     because the proc_int_lock is required to harvest the thread.  It 
 *	     is used when separating the TSZOMB thread from the process' 
 *	     thread list.  This coupled with the fact that the proc_int_lock 
 *	     is held across the termination routine into dispatch prevents 
 *	     this from happening to multi threaded processes.  
 *
 * RECOVERY OPERATION:
 *           None.
 *
 * DATA STRUCTURES:
 *           runrun is decremented
 *
 * RETURNS: 
 *           Pointer to (struct proc) of process owning the selected thread.
 */

struct proc *
#ifdef _POWER_MP
dispatch(register struct proc *old_p, int no_proc_base_lock)
#else
dispatch(register struct proc *old_p)
#endif
{
	register int lvl;		/* Priority level */
	register struct thread *t;	/* new thread to be dispatched */
	register struct thread *old_t;	/* current running thread */
	register struct thread *ownt;	/* lock owner */
	register struct ppda *myppda;	/* my ppda pointer */
	register struct thread **rq;	/* ready queue head */
	register int boosting;		/* current thread is boosting another */
#ifdef _POWER_MP
	register struct thread *h, *tt;
	struct ready_queue_flags lrunmask; /* A local copy */
#endif

	ASSERT(csa->prev != NULL && csa->prev->prev == NULL);
	ASSERT(csa->intpri == INTMAX);

	myppda = PPDA;
	old_t = myppda->_curthread;

#ifdef _POWER_MP
	ASSERT(!lock_mine(&proc_int_lock));
	simple_lock(&proc_int_lock);
#endif

	/*
	 * Increment number of dispatches.
         * We should test for SZOMB and SNONE, but t_dispct is just
         * a statistical field and its being off by 1 is not important.
	 */
	old_t->t_dispct++;

	if (old_t->t_state == TSRUN) {	/* Current thread still runnable? */

                /*
                 * Complete state transition for going to sleep.
                 */
                if (old_t->t_wtype != TNOWAIT) {

			switch(old_t->t_wtype) {
			case TWLOCK :
			case TWLOCKREAD :
				/* 
				 * The current thread is about to sleep on
				 * a lock.  Boost the lock owner's priority
				 * if necessary.  At a minimum requeue it to
				 * the front of the queue if it has the same
				 * priority as the current thread.  Its 
				 * priority is unboosted, when it returns to 
				 * user mode in the system call handler or 
				 * in the system clock handler - sys_timer.
				 */
				ownt = old_t->t_lockowner;
                		if (ownt->t_pri >= old_t->t_pri) {
					boosting = (ownt->t_pri > old_t->t_pri);
                                	ownt->t_boosted++;
					ASSERT(ownt->t_boosted > 0);
                        		ownt->t_lockpri = old_t->t_pri;
                        		prio_requeue(ownt, old_t->t_pri);
					if (!boosting)
                                		ownt->t_boosted--;
					ASSERT(ownt->t_boosted >= 0);
                		}
				if (*(int *)(old_t->t_wchan1) & INSTR_ON) {
					int *l = *((int **)(old_t->t_wchan1));
					fetch_and_and((atomic_p)l, ~INTERLOCK);
				} else {
                			fetch_and_and((atomic_p)old_t->t_wchan1,
						~INTERLOCK);
				}

				/* Fall through */
			default :
                        	old_t->t_state = TSSLEEP;
			}
                        if (old_p->p_pid > 0)   /* Not while booting */
                                U.U_ru.ru_nivcsw += 1;/* No MP synch ! */
                        goto i_will_goto_sleep;
                }

		/*
		 *  If current thread running with interrupts disabled
		 *  then resume its execution.
		 */
		if (old_t->t_uthreadp->ut_save.intpri != INTBASE)  {
			/*
			 * Trace the thread we've selected
			 */
			TRCHKL5T(HKWD_KERN_DISPATCH, old_p->p_pid, old_t->t_tid,
				 0, (old_t->t_pri << 16), myppda->cpuid);

#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
			if (!no_proc_base_lock)
				simple_unlock(&proc_base_lock);
#endif

			/* Return pointer to process owning selected thread */
			return (old_p);
		}
		 
		/*
		 * Put the thread on a 'ready to run' queue
		 */
		ASSERT( old_t->t_wtype == TNOWAIT );
		ASSERT( old_p->p_flag & SLOAD );
		ASSERT( old_p->p_stat == SACTIVE );

		old_t->t_wtype = TWCPU;	/* waiting for CPU */

		lvl = old_t->t_pri & PMASK;	/* priority level */
		rq = &thread_run[lvl]; 	/* ready queue head pointer */

		if (*rq) {
			old_t->t_prior = *rq;		/* after tail */
			old_t->t_next = (*rq)->t_next;	/* before head*/
			(old_t->t_prior)->t_next = old_t;
			(old_t->t_next)->t_prior = old_t;
		}
		else {
			old_t->t_next = old_t;
			old_t->t_prior = old_t;
			run_mask.word[lvl/BITS_PER_WORD]|=MASKBIT(lvl);
		}

		if ((old_t->t_policy == SCHED_OTHER)		||
		    (old_t->t_uthreadp->ut_flags & UTYIELD)	||
		    ((old_t->t_policy == SCHED_RR) &&
			(old_t->t_ticks >= timeslice)))
		{
			old_t->t_ticks = 0;
			old_t->t_uthreadp->ut_flags &= ~UTYIELD;
			*rq = old_t;			/* RQTAIL */

		} else {
			if (*rq == NULL) *rq = old_t;	/* RQHEAD */
		}

	}
	
i_will_goto_sleep:

#ifdef _POWER_MP

	lrunmask = run_mask;	/* local copy of run_mask as we will modify it*/

        for (;;) {
		lvl = bitindex(&lrunmask);	/* 1st level of ready threads*/
		ASSERT(lvl <= PMASK);		/* Wait process always ready */
		
		h = t = thread_run[lvl]->t_next;/* Get head entry */
		ASSERT(h != NULL);		/* Must get something */
		
		do {
			if ((t->t_cpuid == PROCESSOR_CLASS_ANY) ||
			    (t->t_cpuid == myppda->cpuid))
				goto breakbreak;
			t = t->t_next;
		} while (t != h);

		lrunmask.word[lvl/BITS_PER_WORD] &= ~MASKBIT(lvl);
	}
breakbreak:

	if (t->t_affinity != myppda->cpuid) {

		int newlvl, scan;
		newlvl = lvl;
		scan = 0;

		while ((newlvl <= (lvl + affinity_priodelta)) &&
			(newlvl < PIDLE)) {

			h = thread_run[newlvl]->t_next;
			ASSERT(h != NULL);
			if (newlvl == lvl)	/* Skip uneligible threads   */
				tt = t;		/* detected by previous loop */
			else
				tt = h;

			do {
				if (++scan > affinity_scandelta)
					goto affinity_complete;

		    		if ((tt->t_affinity == myppda->cpuid) &&
				    ((tt->t_cpuid == PROCESSOR_CLASS_ANY) ||
				     (tt->t_cpuid == myppda->cpuid))) {

					t = tt;
					lvl = newlvl;
					goto affinity_complete;

				}

				switch(tt->t_policy) {
				case SCHED_OTHER:
					if (!affinity_skipboosted) {
						if (tt->t_boosted)
							goto affinity_complete;
					}
					break;
				case SCHED_RR:
				case SCHED_FIFO:
					goto affinity_complete;
				}

				tt = tt->t_next;
			} while (tt != h);

			lrunmask.word[newlvl/BITS_PER_WORD] &= ~MASKBIT(newlvl);
			newlvl = bitindex(&lrunmask);
		}

affinity_complete:
		t->t_affinity = myppda->cpuid;
	}

	/* Dequeue selected process and update run queue for this level */
	if (t == t->t_next)  {
		run_mask.word[lvl/BITS_PER_WORD] &= ~MASKBIT(lvl);
		thread_run[lvl] = NULL;
	} else {
		(t->t_prior)->t_next = t->t_next; /* Remove head */
		(t->t_next)->t_prior = t->t_prior;
		if (thread_run[lvl] == t) {
			thread_run[lvl] = t->t_prior;
		}
	}
	
#else /* POWER_MP */
	
	/*
	 *  Find highest priority non-empty ready queue list.
	 */
	lvl = bitindex(&run_mask);	/* Return index of first 1 bit */

	ASSERT(lvl <= PMASK);		/* Wait process always ready */
	
	t = thread_run[lvl]->t_next;	/* Get head entry */
	
	if (t->t_next == t) {		/* Only entry, this level ? */
		run_mask.word[lvl/BITS_PER_WORD] &= ~MASKBIT(lvl);
		thread_run[lvl] = NULL;
	}
	else {
		thread_run[lvl]->t_next = t->t_next; /* Remove head */
		(t->t_next)->t_prior = t->t_prior;
	}

#endif /* _POWER_MP */

	myppda->_ficd = 0;		/* Clear finish-int call dispatch */
	INC_RUNRUN(-1);			/* Clear thread switch requested */
	t->t_wtype = TNOWAIT;		/* Thread no longer waiting */
	
	if (t != old_t) {		/* dispatching a new thread */

                /*
                 * There is no need to test the TID since the recycling of
                 * old_t cannot have moved past the SNONE state and
                 * therefore the new thread cannot be the recycled old
                 * thread. Testing the slots is enough to guarantee a
                 * new thread has been dispatched.
                 */

		/*
		 * If last local thread, signal run-time scheduler.
                 * Even if old_t has been recycled (TSZOMB/TSNONE),
                 * TLOCAL won't be set.  Therefore there is no need to
                 * test the condition.
		 */
		if (old_t->t_flags & TLOCAL && old_p->p_local == 0) {
			pidsig(old_p->p_pid, SIGWAITING);
		}

		/* don't increment involuntary counts while we are booting */
		if ((old_t->t_state == TSRUN) && (old_p->p_pid > 0))
			U.U_ru.ru_nivcsw += 1;/* No MP synch ! */
			
		sysinfo.pswitch++;
		cpuinfo[myppda->cpuid].pswitch++;
#ifdef _POWER_MP
		/*
		 * The fprs are disowned on MP on every context switch, 
		 * because the thread may be resumed on another processor. 
		 * It is sufficent to test the slots because we are 
		 * guaranteed that the slot won't be recycled before 
		 * going through the dispatcher at least once.
		 */
		if (old_t == myppda->fpowner) {
			disown_fp(old_t->t_tid);
		}
#endif
		/*
		 * A recycled slot never has t_graphics set.
		 */
		if (old_t->t_graphics) {
			dispatch_graphics_inval(old_t->t_graphics);
		}	
	    
		/*
	 	 * Set current thread pointer.
	 	 * Don't forget to relink the mstsave chain because the 
		 * thread mstsave area (in the uthread structure) is not 
		 * at a fixed address.
	 	 */
		myppda->_csa->prev = &t->t_uthreadp->ut_save;
		SET_CURTHREAD(t);

#ifdef _POWER_MP
		/* 
		 * change ownership of proc_int_lock from 'old thread' to 
		 * 'current thread'.  old_t->t_lockcount may go to -1, if
		 * the process is harvested by the scheduler, however, it
		 * is reset to zero before the thread/process is started
		 * for the first time.
		 */
		old_t->t_lockcount--;
		ASSERT((short)old_t->t_lockcount >= -1);
		t->t_lockcount++;
		ASSERT(t->t_lockcount > 0);

		if (!no_proc_base_lock) {
                	old_t->t_lockcount--;
                	ASSERT((short)old_t->t_lockcount >= -1);
                	t->t_lockcount++;
                	ASSERT(t->t_lockcount > 0);
		}
#endif

		if (t->t_graphics) {	    
			dispatch_graphics_setup(t->t_graphics);    
		}
    	}

	/*
	 * Trace the thread we've selected
	 */
	if(PMASK == lvl)
		TRCHKL5T(HKWD_KERN_IDLE, t->t_procp->p_pid, t->t_tid, 
                 old_t->t_tid, (t->t_pri << 16 | old_t->t_pri), myppda->cpuid);

	else
		TRCHKL5T(HKWD_KERN_DISPATCH, t->t_procp->p_pid, t->t_tid, 
                 old_t->t_tid, (t->t_pri << 16 | old_t->t_pri), myppda->cpuid);

#ifdef _POWER_MP
#ifdef DEBUG
#ifdef _POWER_PC
       	/* sync is usually done in lock front end */
        if (__power_pc())
            	__iospace_sync();
#endif
	/*
	 * sunlock() is the back end of simple_unlock.  It is
	 * used because the ownership checks are in the assembler
	 * front end only when DEBUG is enabled.
	 */	
	sunlock(&proc_int_lock, -2);
	if (!no_proc_base_lock)
		sunlock(&proc_base_lock, -2);
#else
	simple_unlock(&proc_int_lock);
	if (!no_proc_base_lock)
		simple_unlock(&proc_base_lock);
#endif
#endif

	ASSERT(t->t_wchan2 == NULL);
	return(t->t_procp); /* Return ptr to process owning selected thread */
}

/*
 * NAME: waitproc
 *
 * FUNCTION: This is the waitproc code. The waitproc(s) are
 *           mostly spinning around. If runrun != 0
 *           then call swtch() to reduce latency.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:    NOT allowed to go to sleep. Stack and code must be pinned.
 *
 * RECOVERY OPERATION:
 *           None.
 *
 * DATA STRUCTURES:
 */
void
waitproc()
{
#ifdef _POWER_MP
	volatile int *runvalue = &runrun;
	volatile char *ficdvalue = &PPDA->_ficd;

	/*
	 * The scanning limit for affinity is incremented once for each
	 * processor when it its waitproc starts.
	 */
	fetch_and_add(&affinity_scandelta, 3);
#endif

	for (;;) {
#ifdef PM_SUPPORT
		/* When the processor is placed into a power saving
		 * mode, cpu_idle stops execution of this loop until
		 * the processor resumes execution from the power
		 * saving mode.
		 */
		extern struct _pm_kernel_data   pm_kernel_data;
                volatile struct _pm_kernel_data   *pmkdptr=&pm_kernel_data;
                pmkdptr->cpu_idle = pmkdptr->cpu_idle_pm_core;
                if (pmkdptr->cpu_idle != NULL) {
                        (*(pmkdptr->cpu_idle))();
		}
#endif /* PM_SUPPORT */
#ifdef _POWER_MP
		while ((*runvalue == 0) && (*ficdvalue == 0)) {}
		swtch();
#endif
	}
}

/*
 * NAME: bindprocessor
 *
 * FUNCTION: bind a process/thread to a processor
 *
 * EXECUTION ENVIRONMENT:
 *
 *           Must be called from process level.
 *
 * NOTES:
 *           Se design 53035 for a description.
 *
 * RECOVERY OPERATION:
 *           None.
 *
 * DATA STRUCTURES:
 *           Updates errno if there is an error.
 *           t_cpuid and t_scpuid in affected thread(s).
 *
 */
int
bindprocessor(int what, int who, cpu_t where)
{
  	int ipri;
  	struct thread *t;
  	struct thread *ct;
  	struct uthread *cu;
  	struct proc   *p;
	uid_t effuid;
	uid_t realuid;

	ct = curthread;
	cu = ct->t_uthreadp;

	effuid = getuidx(ID_EFFECTIVE);
	realuid = getuidx(ID_REAL);

  	/* Must have an existing processor */
  	if (((where < 0) || (where >= NCPUS())) &&
       	    (where != PROCESSOR_CLASS_ANY))
	{
    		cu->ut_error = EINVAL;
    		return -1;
  	}

  	/* What are we binding */
  	if ((what != BINDPROCESS) && (what != BINDTHREAD)){
    		cu->ut_error = EINVAL;
    		return -1;
  	}

#ifndef _POWER_MP
	return(0);
#endif

  	if (what == BINDPROCESS){
    		/* 
     		 * Bind all thread(s) in a process.
     		 */
    		p = VALIDATE_PID(who);
    		if (p == NULL || p->p_stat == SIDL || 
				 p->p_stat == SZOMB || p->p_stat == SNONE) {
      			cu->ut_error = ESRCH;
      			return -1;
    		}
		/* Must have the right privileges */
		if ((effuid  != p->p_uid)  &&
		    (realuid != p->p_uid)  &&
		    (effuid  != p->p_suid) &&
		    (realuid != p->p_suid) &&
		    (privcheck(SET_PROC_RAC) == EPERM)){
			cu->ut_error = EPERM;
			return -1;
		}
  		ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif
		/* Check whether the process is still valid */
		if(p->p_pid != who || p->p_stat == SZOMB || p->p_stat == SNONE){
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
			unlock_enable(ipri, &proc_base_lock);
			cu->ut_error = ESRCH;
			return -1;
		}
    		t = p->p_threadlist;
    		do {
      			t->t_scpuid = where;
			if (!(t->t_flags & TFUNNELLED))
				t->t_cpuid = where;
      			t = t->t_nextthread;
    		} while (t != p->p_threadlist);

    		/* If we bound ourself to another processor, call swtch(). */
    		if ((ct->t_procp == p)                    && 
		    (ct->t_scpuid != PROCESSOR_CLASS_ANY) && 
		    (ct->t_scpuid != CPUID))
		{
                        /*
                         * Set finish interrupt call dispatcher
			 * flag for the target processor.  Should 
			 * be seen at the next interrupt, however, 
			 * it is not quaranteed. 
                         */
			GET_PPDA(where)->_ficd = 1;

                        /*
                         * Unlock and enable on this processor so
                         * the dispatcher will put the current thread
                         * on the runqueue.
                         */
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
                        unlock_enable(ipri, &proc_base_lock);

                        /*
                         * state save and call dispatcher.
                         */
                        swtch();

      			return 0;
    		} 
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
      		unlock_enable(ipri, &proc_base_lock);
      		return 0;
  	} else {				/* ! BIND_PROCESS */
    		/*
     		 * Bind a thread
     		 */
    		t = VALIDATE_TID(who);
    		if (t == NULL || t->t_state == TSIDL || 
				  t->t_state == TSZOMB || t->t_state == TSNONE){
      			cu->ut_error = ESRCH;
      			return -1;
    		}
		/* Must have the right privileges */
		if ((effuid  != t->t_procp->p_uid)  &&
		    (realuid != t->t_procp->p_uid)  &&
		    (effuid  != t->t_procp->p_suid) &&
		    (realuid != t->t_procp->p_suid) &&
		    (privcheck(SET_PROC_RAC) == EPERM)){
			cu->ut_error = EPERM;
			return -1;
		}
  		ipri = disable_lock(INTMAX, &proc_base_lock);
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif
		/* Check whether the thread is still valid */
		if(t->t_tid != who || t->t_state == TSZOMB ||
					t->t_state == TSNONE){
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
			unlock_enable(ipri, &proc_base_lock);
			cu->ut_error = ESRCH;
			return -1;
		}

    		t->t_scpuid = where;
		if (!(t->t_flags & TFUNNELLED))
			t->t_cpuid = where;

    		/* If we bound ourself to another processor, call swtch(). */
    		if ((t == ct) && 
		    (t->t_cpuid != PROCESSOR_CLASS_ANY) && 
	            (t->t_cpuid != CPUID))
		{
                        /*
                         * Set finish interrupt call dispatcher
                         * flag for the target processor.  Should
                         * be seen at the next interrupt, however,
                         * it is not quaranteed.
                         */
			GET_PPDA(where)->_ficd = 1;

                        /*
                         * Unlock and enable on this processor so
                         * the dispatcher will put the current thread
                         * on the runqueue. 
                         */
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
                        unlock_enable(ipri, &proc_base_lock);

                        /*
                         * state save and call dispatcher.
                         */
                        swtch();

      			return 0;
    		}
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
      		unlock_enable(ipri, &proc_base_lock);
      		return 0;
  	}
}

/*
 * NAME: switch_cpu
 *
 * FUNCTION: move running thread to another processor,
 *           used for funneling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           Must be called from process level at INTBASE.
 *
 * RECOVERY OPERATION:
 *           None.
 *
 * DATA STRUCTURES:
 *           curthread->cpuid,
 *           curthread->scpuid.
 *
 * RETURNS:
 *           TRUE, if the caller was funneled prior to the
 *                 this call.  This only applies when using
 *                 the SET_PROCESSOR_ID option.
 *           FALSE, all other cases.
 */
int
switch_cpu(cpu_t where, int options)
{
  	int ipri;
  	struct thread *t = curthread;
        int wasfunnelled;

        wasfunnelled = t->t_flags & TFUNNELLED;

	/* 
	 * Must be called at INTBASE, otherwise the dispatcher
	 * will reschedule the current thread, since it is in
	 * a disabled critical section.
	 */ 
	ASSERT(csa->intpri == INTBASE);

  	if (options == SET_PROCESSOR_ID){
    		/* Must get an existing processor */
    		if ((where < 0) || (where >= NCPUS())){
                        return(wasfunnelled);
		}

    		ipri = disable_lock(INTMAX, &proc_base_lock);

    		t->t_cpuid = where;
		t->t_flags |= (TFUNNELLED);
    		if ((where != PROCESSOR_CLASS_ANY) && (where != CPUID))
		{

                        /*
                         * Set finish interrupt call dispatcher
                         * flag for the target processor.  Should
                         * be seen at the next interrupt, however,
                         * it is not quaranteed.
                         */
			GET_PPDA(where)->_ficd = 1;

			/* 
			 * Unlock and enable on this processor so
			 * the dispatcher will put the current thread
			 * on the runqueue.
			 */
      			unlock_enable(ipri, &proc_base_lock);

			/*
			 * state save and call dispatcher.
			 */
      			swtch();
                        return(wasfunnelled);
    		}
   	 	unlock_enable(ipri, &proc_base_lock);
                return(wasfunnelled);
  	}

  	if (options == RESET_PROCESSOR_ID){

    		ipri = disable_lock(INTMAX, &proc_base_lock);

    		t->t_cpuid = t->t_scpuid;
		t->t_flags &= ~(TFUNNELLED);
    		if ((t->t_cpuid != PROCESSOR_CLASS_ANY) && 
		    (t->t_cpuid != CPUID))
		{
                        /*
                         * Set finish interrupt call dispatcher
                         * flag for the target processor.  Should
                         * be seen at the next interrupt, however,
                         * it is not quaranteed.
                         */
			GET_PPDA(where)->_ficd = 1;

                        /*
                         * Unlock and enable on this processor so
                         * the dispatcher will put the current thread
                         * on the runqueue.
                         */
                        unlock_enable(ipri, &proc_base_lock);

			/*
			 * state save and call dispatcher.
			 */
      			swtch();
      			return(0);
    		}
    		unlock_enable(ipri, &proc_base_lock);
    		return(0);
  	}
}

/*
 * NAME: mycpu
 *
 * FUNCTION: return logical processor we are running at.
 *           used for funneling.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * NOTES:
 *           Se design 74837 for a description.
 *           This is TEMPRORARILY interface, will be removed.
 *
 * RECOVERY OPERATION:
 *           None.
 *
 * DATA STRUCTURES:
 *
 */
cpu_t
mycpu(void)
{
#ifdef _POWER_MP
  	return (PPDA->cpuid);
#else
	return(0);
#endif
}
