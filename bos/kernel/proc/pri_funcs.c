static char sccsid[] = "@(#)60	1.7.1.16  src/bos/kernel/proc/pri_funcs.c, sysproc, bos41J, 9512A_all 3/20/95 18:51:21";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: donice
 *		getpri
 *		prio_requeue
 *		setpri
 *		yield
 *
 *   ORIGINS: 27, 83
 *
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


#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/pri.h>
#include <sys/prio_calc.h>
#include <sys/param.h>
#include <sys/intr.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/sched.h>
#include <sys/lockl.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include <sys/id.h>
#include <sys/sleep.h>
#include <sys/atomic_op.h>
#include "swapper_data.h"

extern uid_t getuidx(int); 
extern void setrq();


/*
 * NAME: prio_requeue
 *                                                                    
 * FUNCTION: set the priority of a thread and place it on a new ready 
 *           queue if:
 *                 a. already on a ready queue (i.e. t_state = TWCPU)
 *                                 - AND -
 *                 b. the new priority differs from the current one
 *
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *           This function cannot page fault and must be called 
 *           with interrupts disabled.
 *                                                                   
 * INPUTS: 
 * 	     t    - address of thread block   
 *           prio - new priority of thread
 *
 * RECOVERY OPERATION: 
 *           None.
 *
 * DATA STRUCTURES: 
 *           None.
 *
 * RETURNS:  NONE
 */  

void
prio_requeue(struct thread *t, int prio)
{
      int i, ncpus;

      ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
      ASSERT(lock_mine(&proc_int_lock));
#endif 
    
      switch(t->t_wtype) {
      case TWCPU: 			/* thread is on a ready queue	*/
	   if (t->t_boosted) {
		remrq(t);		/* remove thrd from ready queue */
		t->t_pri = prio;	/* set the new priority		*/
		setrq(t, E_WKX_PREEMPT, RQHEAD); 
	   }
	   else if (t->t_pri != prio) {
		remrq(t);		/* remove thrd from ready queue */
		t->t_pri = prio;	/* set the new priority		*/
		setrq(t, E_WKX_PREEMPT, RQTAIL); 
	   }
	   break;
      case TWMEM:			/* thread is swapped out	*/
	   if (prio < t->t_procp->p_sched_pri)
		t->t_procp->p_sched_pri = prio;
	   t->t_pri = prio;
	   break;
      case TNOWAIT:
	   /*
	    * If the priority of a thread is being lowered and it is  
	    * currently running, then flag the dispatcher on that 
	    * processor.  Don't use the global flag because the
	    * wrong processor may enter the dispatcher and the 
	    * thread is not on the runq. 
	    */
	   if ((t->t_pri < prio) && (t->t_state == TSRUN)) {
#ifdef _POWER_MP
		ncpus = NCPUS();
                for (i = 0; i < ncpus; i++) {
                        if (ppda[i]._curthread == t) {
				ppda[i]._ficd = 1;
                                break;
                        }
                }
#else
		ppda[0]._ficd = 1;
#endif
	   }
	   /* Falls through */
      default:
	   t->t_pri = prio;
      }
}


/*
 * NAME: donice()
 *
 * FUNCTION:    Resets the process's nice value to "n" if "n" is
 *		within limits. If "n" > P_NICE_MAX or "n" < P_NICE_MIN
 *              then "n" is set to the upper/lower bound
 *
 * DATA STRUCTURES USED:
 *		Process structure
 *
 * EXECUTION ENVIRONMENT:
 *              This thread cannot page fault since it disables all 
 *              interrupts.
 *
 * RETURN VALUE DESCRIPTION:
 *              -1 - process's nice value was not set because the nice
 *                   value was being lowered (i.e. more favorable) and  
 *                   caller did not have super user authority (u_error
 *                   is set to EACCES).
 *                                       - OR -
 *                   the caller did not have access rights to the process
 *                   being changed (u_error is set to EPERM).
 *                                       - OR -
 *                   the process has a fixed priority or is not an
 *                   'real' process (u_error is set to ESRCH).
 *
 *                   In all cases, u_error has been set.
 *
 *               0 - process's nice value has been set, and its 
 *                   priority has been recalculated.
 */

int
donice(struct proc *p, int n, struct ucred *crp)
/*
 *       p: Process whose nice value is to be changed.
 *       n: New nice value. 
 */
{
        register save_ipri;          /* saved interrupt level           */
	register hadintlock;	     /* had proc_int_lock on input      */
	register hadbaselock;	     /* had proc_base_lock on input     */
	int ret_value = 0;           /* return value                    */
	  
	/*
	 * Check to make sure the caller has access rights to the
	 * process being changed.
	 */

	if (privcheck_cr(BYPASS_DAC_WRITE,crp) &&
            (crp->cr_uid != p->p_uid) &&
            (crp->cr_ruid != p->p_uid))
	{
		u.u_error = EPERM;
		return(-1);
	}

	/*
	 * If the caller specified something greater than the maximum 
	 * allowable "nice" value, change the input parameter to the 
	 * system's maximum value.
	 */

	if (n > P_NICE_MAX)
		n = P_NICE_MAX;

	/*
	 * If the caller specified something less than the minimum 
	 * allowable "nice" value, change the input parameter to the 
	 * system's minimum value.
	 */

	if (n < P_NICE_MIN)
		n = P_NICE_MIN;

	/*
	 * If the caller is trying to set the "nice" value to a more
	 * favorable position for the process, we must make sure the
	 * caller has the right to do so, as this will affect system wide
	 * process scheduling.
	 */

	if (n < EXTRACT_NICE(p) && (privcheck_cr(SET_PROC_RAC, crp) == EPERM)) {
		u.u_error = EACCES;
		return(-1);
	}

	/*
	 * If we have checked out ok up to this point, we can go ahead
	 * and alter the process's "nice" value and notify the scheduler's
	 * priority altering mechanism.
	 */
	  
        if ( p->p_flag & SFIXPRI )  {      /* process has a fixed priority */
		u.u_error = ESRCH;
		ret_value = -1 ;
	}
	else  {
		save_ipri = i_disable(INTMAX); 

#ifdef _POWER_MP
		if (!(hadbaselock = lock_mine(&proc_base_lock)))
			simple_lock(&proc_base_lock);
		if (!(hadintlock = lock_mine(&proc_int_lock)))
			simple_lock(&proc_int_lock);
#endif

		/*
		 * still need to check whether this process is 'real'
		 */

	        if ( p->p_stat != SIDL && 
	             p->p_stat != SZOMB && 
	             p->p_stat != SNONE ) {
	                    SET_NICE(p,n);
			    {
				struct thread *t = p->p_threadlist;

				/*
				 * Since nice has been changed, p_sched_pri
				 * may actually increase, therefore we need
				 * to recompute it. To do that, resetting it
				 * is sufficient because prio_requeue will
				 * update it.
				 * Note: p_sched_pri == PIDLE means that no
				 * thread has been swapped, so we can't reset it
				 * to that value but PIDLE-1 will do just fine.
				 */
				if (p->p_sched_pri != PIDLE)
					p->p_sched_pri = PIDLE - 1;

				do {
					if (t->t_state != TSIDL &&
					    t->t_state != TSZOMB &&
					    t->t_state != TSNONE)
						prio_requeue(t,prio_calc(t));
					t = t->t_nextthread;
				} while (t != p->p_threadlist);
			    }
                }
		else {
		            u.u_error = ESRCH;
		            ret_value = -1 ;
                }

#ifdef _POWER_MP
		if (!hadintlock)
			simple_unlock(&proc_int_lock);
		if (!hadbaselock)
			simple_unlock(&proc_base_lock);
#endif

		i_enable(save_ipri);
	}
	return(ret_value);
}


/*
 * NAME: getpri
 *
 * FUNCTION: returns the priority of a specified process.
 *           If the pid number is 0, supplied by caller, then the current
 *           running process's priority is returned.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine will not page fault
 *
 * NOTES: 
 *
 * RETURN VALUES:
 *	-1	failed, errno indicates cause of failure. errno can be:
 *		   - EPERM - the requesting process does not have 
 *			     permission to alter the priority of any of
 *			     the affected processes.
 *		   - ESRCH - No process can be found corresponding to
 *                            that specific pid or it was not a fixed 
 *                            priority process.
 *	process's priority
 */

int
getpri(pid_t pid) 
{
	register struct proc *p;
	register ipri;
	register pri;

	if (pid == 0)
		return(U.U_procp->p_threadlist->t_pri);

	if (pid < 0) {
		u.u_error = ESRCH;
		return(-1);
	}

	p = VALIDATE_PID(pid);

	if (p == NULL  ||             /* pid number was invalid */
	    p->p_stat == SIDL  ||     /* process just being created */
	    p->p_stat == SZOMB ||     /* have a zombie process */
	    p->p_stat == SNONE )  {   /* free process entry */
		u.u_error = ESRCH;
		return(-1);
	}

	if ((getuidx(ID_EFFECTIVE) == p->p_uid) ||  /* eff user id match */
            (getuidx(ID_REAL) == p->p_uid)      ||  /* or real user id match */
	    (privcheck(SET_PROC_RAC) == 0) ) {      /* or user has capability of
						       setting usage parms   */
		ipri = disable_lock(INTMAX, &proc_base_lock);
		if (p->p_pid != pid ||
		    p->p_stat == SZOMB ||
		    p->p_stat == SNONE )  {
			unlock_enable(ipri, &proc_base_lock);
			u.u_error = ESRCH;
			return(-1);
		}
		pri = p->p_threadlist->t_pri;
		unlock_enable(ipri, &proc_base_lock);
		return(pri);
	}

	u.u_error = EPERM;       /* requesting process does not have 
				    permission to alter/view the 
				    priority of the specificied process */
	return(-1);
}


/*
 * NAME: setpri
 *
 * FUNCTION: Enables a process to run with a fixed priority.
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine cannot page fault, it disables
 *                        all interrupts.
 *
 * NOTES: 
 *       The caller of this function must have SET_PROC_RAC privileges.
 *
 * RETURN VALUES:
 *	-1	failed, errno indicates cause of failure. errno can be:
 *		   - EINVAL - the requested priority is outside the
 *                            range of acceptable priorities.
 *		   - EPERM  - the requesting process does not have 
 *			      permission to alter the priority of any of
 *			      the affected processes.
 *		   - ESRCH - No process can be found corresponding to
 *                            that specific pid.
 *	former priority of specified process.
 */

int 
setpri(pid_t pid,int new_pri) 
{
     register save_intr;
     register struct proc *p;
     register struct thread *t;
     int former_pri;
     static int svcnum = 0;

     if(audit_flag && audit_svcstart("PROC_Setpri", &svcnum, 1, new_pri)){
     	audit_svcfinis();
     }

     TRCHKGT_SYSC(SETPRI, pid, new_pri, NULL, NULL, NULL);

     /*
      * first check if the caller has required privilege to call
      * this function.
      */
     if (privcheck(SET_PROC_RAC) == EPERM) {
	   u.u_error = EPERM;
	   return(-1);
     }

     /*
      * now check if the caller's priority is valid
      */
     if ( new_pri < PRIORITY_MIN ||
	  new_pri > PRIORITY_MAX ) {
	    u.u_error = EINVAL;
	    return(-1);
     }

     /* now check the pid number supplied by caller.
      * if the pid is:
      *     0 - change the priority of the current running process
      *     less than 0 - return ESRCH errno
      *     otherwise - validate that a process is active with that
      *                 pid number.
      * 
      */
     if ( pid == 0 ) 
	 p = curproc;              /* pid = 0, set current running process's
				      priority */
     else {
	 if ( pid < 0  )
	     p = NULL;
	 else
	     p = VALIDATE_PID(pid); /* check that pid is valid */

         /*
          * need to check if process is valid. 
          */

         if ( p == NULL ||
	      p->p_stat == SIDL  ||
	      p->p_stat == SZOMB ||
	      p->p_stat == SNONE ) {   /* no process found for that pid */
		u.u_error = ESRCH;
		return(-1);
	 }
     }

     /* 
      * now make process run with a fixed priority and set the process's
      * priority to 'pri'
      */
     save_intr = disable_lock(INTMAX, &proc_base_lock); 
     if ( pid && (p->p_pid != pid ||
		 p->p_stat == SZOMB ||
		 p->p_stat == SNONE) ) {
	unlock_enable(save_intr, &proc_base_lock); 
	u.u_error = ESRCH;
	return(-1);
     }

#ifdef _POWER_MP
     simple_lock(&proc_int_lock);
#endif

     t = p->p_threadlist;

     /*
      * This system call returns a priority value from
      * any thread in the process.  This is OK in
      * the multi-threaded case, because SFIXPRI affects
      * every threads and they thus share the same priority.
      */
     former_pri = t->t_pri;			/* save current priority */
     p->p_flag |= (SFIXPRI | SNOSWAP);		/* set fixed priority flag 
							and 'no swap' flag */
     p->p_nice = new_pri;            		/* save the new priority */

     /*
      * Since nice has been changed, p_sched_pri may actually increase,
      * therefore we need to recompute it. To do that, resetting it is
      * sufficient because prio_requeue will update it.
      * Note: p_sched_pri == PIDLE means that no thread has been swapped,
      * so we can't reset it to that value but PIDLE-1 will do just fine.
      */
     if (p->p_sched_pri != PIDLE)
	p->p_sched_pri = PIDLE - 1;

     do {
	if (t->t_state != TSZOMB && t->t_state != TSNONE)
	{
		t->t_sav_pri = new_pri;
		t->t_policy = SCHED_RR;
		prio_requeue(t,prio_calc(t));
	}
	t = t->t_nextthread;
     } while (t != p->p_threadlist);

#ifdef _POWER_MP
     simple_unlock(&proc_int_lock);
#endif
     unlock_enable(save_intr, &proc_base_lock);

     return(former_pri);
}


/*
 * NAME: yield
 *
 * FUNCTION: Forces the process to yield the CPU. 
 *
 * EXECUTION ENVIRONMENT: This routine may Pagefault.
 *
 * NOTE: Posix system call.
 *
 * RETURN VALUES: none.
 *		
 */

void
yield(void)
{
	curthread->t_uthreadp->ut_flags |= UTYIELD;
	swtch();
}
