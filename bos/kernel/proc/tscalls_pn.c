static char sccsid[] = "@(#)81  1.15.1.3  src/bos/kernel/proc/tscalls_pn.c, sysproc, bos412, 9446B 11/9/94 09:02:16";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: interval_end
 *   		ksettimer
 *		ksettimer_mpc
 *		rtsleep_end
 *		set_time_end
 *		wait_time_start
 *
 *   ORIGINS: 27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>		/* Always needed			*/
#include <sys/timer.h>		/* timer structure defines, flags, etc.	*/
#include <sys/errno.h>		/* To define error numbers on failure	*/
#include <sys/intr.h>		/* To define serialization constants	*/
#include <sys/proc.h>		/* Define proc struct where trb's kept	*/
#include <sys/rtc.h>		/* realtime clock defines		*/
#include <sys/syspest.h>	/* To define assert and ASSERT macros	*/
#include <sys/trchkid.h>	/* For trace hooks			*/
#include <sys/time.h>		/* To define the system wide timer, etc.*/
#include <sys/user.h>		/* Accessibility to the user structure	*/
#include <sys/events.h>		/* POSIX delivery mechanism defines	*/
#include <sys/malloc.h>		/* xmalloc defines			*/
#include <sys/low.h>		/* to define the mst save area for the..*/
#include <sys/mstsave.h>	/* asserts like ensuring called by proc.*/
#include <sys/priv.h>		/* for the security/privilege defines	*/
#include <sys/sleep.h>		/* e_sleep() return codes		*/
#include <sys/systemcfg.h>

#ifdef _POWER_MP
struct timestruc_t tod_delta;	/* time delta to be applied per processor*/
boolean_t tod_delta_negative;

int time_updates[MAXCPU] = {0};	/* n of processors yet to be updated 	 */
mpc_msg_t mpct;
#endif

extern struct trb *talloc();		/* allocate timer request blocks*/
void rtsleep_end();			/* for forward reference	*/
extern void set_time();			/* for forward reference	*/

Simple_lock time_lock;		

#ifdef _POWER_MP
/*
 * NAME:  wait_time_start
 *                                                                    
 * FUNCTION:  wait until all previous timer updates are done.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This routine may only be called under a process.
 *                                                                   
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  NONE
 *
 */  
void
wait_time_start()
{
	int ncpus = NCPUS();
	int i;

	simple_lock(&time_lock);
	
	for (i=0; i < ncpus; i++) {
		if (time_updates[i])
			i = -1;
	}

	for (i=0; i < ncpus; i++) {
		time_updates[i] = 1;
	}

	simple_unlock(&time_lock);
}

/*
 * NAME:  set_time_end
 *                                                                    
 * FUNCTION:  Set this processor's timer update.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is callable from the process and
 * 	interrupt environments only at INTTIMER. 
 *                                                                   
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  NONE
 *
 */  
void
set_time_end()
{
	time_updates[CPUID] = 0;
        __iospace_sync();
}

/*
 * NAME:  ksettimer_mpc 
 *
 * FUNCTION: This function is called by mpc when ksettimer is 
 *           done at another processor
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Called at INTTIMER. 
 *
 * RETURN VALUE DESCRIPTION:  NONE
 */
ksettimer_mpc()
{
  	struct timestruc_t now, new, delta;
  	int ipri;

	ASSERT(csa->intpri < csa->prev->intpri);

  	/*
   	 * Someone updated the time. tod_delta contains the
	 * delta to be applied to each processor except the
	 * one that issued the mpc.  Its time is updated inline.
   	 */
  	ipri = disable_lock(INTMAX, &tod_lock);
#ifdef _RS6K_SMP_MCA
        if (__rs6k_smp_mca())
                pgs_rtc_stop();
#endif /* _RS6K_SMP_MCA */
	curtime(&now);
	delta = tod_delta;

	if (tod_delta_negative){
		/* 
		 * A negative time delta should be applied.  See ksettimer.
		 */
		ntimersub(now, delta, new);
	}
	else
		ntimeradd(now, delta, new);

  	set_time(new);
#ifdef _RS6K_SMP_MCA
        if (__rs6k_smp_mca())
                pgs_rtc_start();
#endif /* _RS6K_SMP_MCA */
  	unlock_enable(ipri, &tod_lock);

  	set_time_end(); 		/* it is done at INTTIMER, I know */
}
#endif

/*
 * NAME:  ksettimer 
 *
 * FUNCTION: This interface is the kernel service counterpart to settimer.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine may be called from a kernel extension.
 *
 *	This routine may not page fault.
 *
 *	It serializes to INTMAX to ensure that the clock does not tick in
 *	the middle of updating things.
 *
 * RETURN VALUE DESCRIPTION:  
 *	success -> 0;  failure -> EINVAL: invalid timer parameter. 	 
 */

ksettimer(struct timestruc_t *nct)
{
	struct timestruc_t nnct;
	struct timestruc_t delta, actual;
	int ipri;
	int i;

	/* validate nct input parameter */
	if((nct->tv_nsec < 0) || (nct->tv_nsec >= NS_PER_SEC)){ 
		return(EINVAL);
	}

	bcopy(nct, &nnct, sizeof(struct timestruc_t));

	assert(csa->intpri == INTBASE);

#ifdef _POWER_MP
	/*
	 * This is a little complicated, we have to
	 * 1. wait for all previous time updates to finish
	 * 2. do our one update
	 * 3. update the other processors tod_delta
	 * 4. tell the other processors to do the same
	 * 5. wait until all other processors are done
	 *
	 * This assumes that an mpc is not sent to the
	 * processor that issued the mpc.
	 */
	wait_time_start(); 				/* 1 */

	ipri = disable_lock(INTMAX, &tod_lock);

#ifdef _RS6K_SMP_MCA
        if (__rs6k_smp_mca())
                pgs_rtc_stop();
#endif /* _RS6K_SMP_MCA */
	curtime(&actual);  	/* current time */
	set_time(nnct);        				/* 2 */
#ifdef _RS6K_SMP_MCA
        if (__rs6k_smp_mca())
                pgs_rtc_start();
#endif /* _RS6K_SMP_MCA */
	/*
	 * Calculate the time delta to be applied by the
	 * other processors.  Negative time deltas are
	 * flagged via tod_delta_negative.  The tv_sec
	 * field is unsigned long.
	 */
	if (ntimercmp(actual, nnct, >)) {		/* 3 */
		ntimersub(actual, nnct, tod_delta);
		tod_delta_negative = TRUE;
	}
	else {
		ntimersub(nnct, actual, tod_delta);
		tod_delta_negative = FALSE;
	}

	/* Remain disabled so that the thread waits on
	 * the processor that it updated.
	 */ 
	unlock_enable(INTIODONE, &tod_lock);

	mpc_send(MPC_BROADCAST, mpct); 			/* 4 */

	set_time_end();   				/* 5 */

	i_enable(ipri);
#else
	/*  Update the timer hardware and memory mapped time variables */
	ipri = i_disable(INTMAX);
	set_time(nnct);
	i_enable(ipri);
#endif

	return(0);
}

/*
 * NAME:  rtsleep_end
 *                                                                    
 * FUNCTION: Timeout routine for nsleep. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is called via the timer interrupt handler.
 *
 *	This routine does not page fault.
 */  
void
rtsleep_end(register struct trb *t)
{
	ASSERT(csa->prev != NULL);
	ASSERT(csa->intpri == INTTIMER);

	e_wakeup((int *)t->func_data);
}


/*
 * NAME:  interval_end
 *                                                                    
 * FUNCTION:  This is the timeout function for absinterval() and
 *	incinterval() timeout requests.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is called on the timer interrupt level.
 *                                                                   
 * RETURN VALUE DESCRIPTION:  interval_end has no returned value.
 */  
void
interval_end(register struct trb *timereq)
{
	register struct thread	*t;	/* thread to send SIGALRM1 to	 */
	register struct proc	*p;	/* process to send SIGALRM to	 */
	register ulong		id;	/* id to send SIGALRM/1 to       */
	struct timestruc_t	ct;	/* current time                  */

	ASSERT(csa->prev != NULL);
	ASSERT(csa->intpri == INTTIMER);

	id = timereq->id;
	if (MAYBE_PID(id)) {
		p = VALIDATE_PID(id);
		assert(p != NULL);
		pidsig(id, timereq->func_data);
	}
	else {
		t = VALIDATE_TID(id);
		assert(t != NULL);
		kthread_kill(id, timereq->func_data);
	}

	/*
	 *  If this is a periodic timer (i.e. if it_interval is non-zero),
	 *  resubmit it.
	 */
	if(ntimerisset(&(timereq->timeout.it_interval)))  {
		ntimeradd(timereq->timeout.it_interval, 
			  timereq->timeout.it_value, 
			  timereq->timeout.it_value);
		/*
		 *  Must validate the interval.	 Time is maintained as a 
		 *  32 bit word which will eventually wrap around.  We 
		 *  do not want to process requests that are based in the 
		 *  past.
		 */
		curtime(&ct);
		if (ntimercmp(timereq->timeout.it_value, ct, >)){
			/* current time is already included */
			timereq->flags |= T_ABSOLUTE;	
			tstart(timereq);
		}
                /*
                 *  If the end of the new interval is past due, make the
                 *  time-out request current and add the interval in again.
                 *  Validate the resultant time to ensure that it is not in
                 *  the past.
                 */
                else {
                        timereq->timeout.it_value.tv_sec = ct.tv_sec;
                        timereq->timeout.it_value.tv_nsec = ct.tv_nsec;
                        ntimeradd(timereq->timeout.it_interval,
                                  timereq->timeout.it_value,
                                  timereq->timeout.it_value);
                        if (ntimercmp(timereq->timeout.it_value, ct, >)){
                                /* current time is already included */
                                timereq->flags |= T_ABSOLUTE;
                                tstart(timereq);
                        }
                }
	}
}
