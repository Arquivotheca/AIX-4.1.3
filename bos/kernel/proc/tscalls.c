static char sccsid[] = "@(#)22	1.89  src/bos/kernel/proc/tscalls.c, sysproc, bos41J, 9521A_all 5/19/95 18:04:24";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: absinterval
 *		getinterval
 *		gettimer
 *		gettimerid
 *		incinterval
 *		nsleep
 *		reltimerid
 *		resabs
 *		resinc
 *		restimer
 *		settimer
 *		texit
 *		tsig
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
#include <sys/audit.h>		/* audit structures                     */

extern void interval_end();		/* abs- and incinterval timeout	*/
extern void rtsleep_end();		/* nsleep timeout		*/
extern struct trb *talloc();		/* allocate timer request blocks*/
long min_ns; 				/* global var for minimum ns    */

extern int time_adjusted;


/*
 * NAME:  getinterval (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  Gets the current value for a specified per-process timer.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called under a process.
 *
 *	This routine may page fault.
 *                                                                   
 * NOTES:  Due to race conditions, it is possible that the value returned
 *	by this process may be 0 even though it_interval is non-zero (i.e.
 *	even though the timer is a periodic timer).
 *
 * DATA STRUCTURES:  U.U_timer (the per-process array of timer
 *	information structures). ut->ut_timer (the per-thread array).
 *
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error
 *	with errno set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EINVAL:  The timerid parameter does not correspond to an id
 *		 returned by gettimerid().
 */  
int
getinterval(register timer_t timerid,
            register struct itimerstruc_t *value)
{
	struct  itimerstruc_t tt;	/* passed timer local copy      */
	struct	itimerstruc_t ct;	/* current time			*/
	struct	itimerstruc_t dt;	/* delta time			*/
	struct  thread       *th;
	struct  proc         *p;
	struct  uthread      *ut;
	struct  trb          **timer_array;
	int		      index;
	int		      ipri;
	boolean_t	      timer_locked;

	TRCHKLT_SYSC(GETINTERVAL, timerid);

	ASSERT(csa->prev == NULL);	/* make sure caller is a process*/

	th = curthread;
	p = th->t_procp;
	ut = th->t_uthreadp;

	/* Don't need to lock timers anchored by the uthread structure */ 
	if (timerid < NTIMERS) {
		if (timer_locked = (p->p_active > 1))
			ipri = disable_lock(INTTIMER, &U.U_timer_lock);
		else
			ipri = i_disable(INTTIMER);
	}

	/*  Validate input parameter.  */
	if(TIMERID_NOTVALID(timerid))  {
		if (timerid < NTIMERS) {
			if (timer_locked)
				unlock_enable(ipri, &U.U_timer_lock);
			else
				i_enable(ipri);
		}
		ut->ut_error = EINVAL;
		return(-1);
	}

        /* Determine base address of timers: ublock or uthread. */
        if ((0 <= timerid) && (timerid < NTIMERS)) {
                timer_array = &U.U_timer[0];
                index = timerid;
        }
        else {
                timer_array = &ut->ut_timer[0];
                index = timerid - TIMERID_REAL1;
        }

        /* local copy of trb */
        tt = timer_array[index]->timeout;

	/*
	 *  If this is a periodic timer, then the period will be returned to
	 *  the caller unmodified.
	 */
	dt.it_interval = tt.it_interval;

	/*  Now the time left until the next timeout needs to be calculated.  */

	if(TIMERID_ISBSD(timerid))
		ntimerclear(&(ct.it_value));
	else
		curtime(&(ct.it_value));

	if(ntimercmp(ct.it_value, tt.it_value, <))  {
		/*
		 *  The time in the timeout field is greater than the current
		 *  time (the timer has not yet expired).  The time left
		 *  until the next timeout must be calculated (i.e. the
		 *  timeout time minus the current time).
		 */
		ntimersub(tt.it_value, ct.it_value, dt.it_value);
	}
	else  {
		/*
		 *  The time in the timeout field is LESS THAN the current
		 *  time (the timer has expired) so return 0.
		 */
		ntimerclear(&(dt.it_value));
	}
        if (timerid < NTIMERS) {
                if (timer_locked)
                        unlock_enable(ipri, &U.U_timer_lock);
                else
                        i_enable(ipri);
        }

	/*  Return the timeout values to the caller.  */
	if((copyout(&dt, value, sizeof(struct itimerstruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}

	return(0);
}


/*
 * NAME:  absinterval (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  Requests notification at a specific time (i.e. sets the 
 *	value of a per-process timer to a specific time).
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called from a process.
 *
 *	This routine may page fault.
 *                                                                   
 * NOTES:  The value and ovalue parameters are pointers to itimerstruc_t
 *	structures.  The itimerstruct_t structure is made up of two
 *	timestruc_t structures.  One of these timestruc_t structures
 *	(it_value) contains the time for which the requested notification 
 *	should be scheduled.  If it_value is 0, the per-process timer is 
 *	disabled and the amount of time left before the timer would have 
 *	expired is returned in the ovalue parameter.
 *
 *	The other timestruc_t structure (it_interval) contains the 
 *	value for subsequent timeouts.  Thus, if it_interval in zero,
 *	a "one-shot" timer is requested.  Conversely, if it_interval
 *	is not zero, a "periodic" timer is requested, with the period
 *	starting at the absolute time specified by it_value.
 *
 *	If the specified time in it_value has already expired, the function
 *	will succeed and the timer event will be delivered.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error.
 *	If successful, the ovalue parameter is set to the previous timer
 *	setting (i.e. the amount of time left until the timer would have
 *	expired).  Upon error, errno is set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EINVAL:  A value structure specified a nanosecond value less than
 *		 zero or greater than or equal to one billion;
 *	EINVAL:  The timerid parameter does not correspond to an id
 *		 returned by gettimerid().
 */  
int
absinterval(register timer_t timerid,
            register struct itimerstruc_t *value, 
            register struct itimerstruc_t *ovalue)
{
	struct	 timestruc_t	ct;	/* current time			*/
	struct	 itimerstruc_t	icp;	/* copy of value parameter	*/
	struct	 itimerstruc_t	ocp;	/* copy of ovalue parameter	*/
	register struct trb	*t;	/* timer structure to submit	*/
	struct   thread         *th;
	struct   proc         	*p;
	struct   uthread        *ut;
	struct   trb            **timer_array;
	int                     index;
	int			ipri;
        boolean_t             timer_locked;

	TRCHKLT_SYSC(ABSINTERVAL, timerid);

	ASSERT(csa->prev == NULL);	/* make sure caller is a process*/

	th = curthread;
	p = th->t_procp;
	ut = th->t_uthreadp;

	/*  Initialize the local copies of input and output values.  */
	ntimerclear(&(icp.it_value));
	ntimerclear(&(icp.it_interval));
	ntimerclear(&(ocp.it_value));
	ntimerclear(&(ocp.it_interval));

	/* If "value" is NULL, the we're not setting a new time, so */
	/* skip this part. 					    */
	if(value != NULL)
	{
		/*  Get the time parameter from user space.  */
		if((copyin(value, &icp, sizeof(struct itimerstruc_t))) != 0)  {
			ut->ut_error = EFAULT;
			return(-1);
		}

		/*  Validate the time parameters.  */
		if((icp.it_value.tv_nsec >= NS_PER_SEC) ||
	   	   (icp.it_value.tv_nsec < 0)  ||
	   	   (icp.it_interval.tv_nsec >= NS_PER_SEC) ||
	   	   (icp.it_interval.tv_nsec < 0)) {
			ut->ut_error = EINVAL;
			return(-1);
		}

		/*
	 	 *  Only privileged users may submit fine granularity timers.  
		 *  If the time is non-zero AND the user is NOT privileged AND 
	         *  the timer request is for LESS THAN 10 millisecond, then 
		 *  make the timer request 10 millisecond.
	 	 */
		if(privcheck(SET_PROC_RAC) == EPERM)  {
			if((icp.it_value.tv_sec == 0) &&
		   	   (icp.it_value.tv_nsec != 0) &&
		   	   (icp.it_value.tv_nsec < (NS_PER_MSEC * 10)))  {
				icp.it_value.tv_nsec = (NS_PER_MSEC * 10);
			}
			if((icp.it_interval.tv_sec == 0) &&
		   	   (icp.it_interval.tv_nsec != 0) &&
		   	   (icp.it_interval.tv_nsec < (NS_PER_MSEC * 10 )))  {
				icp.it_interval.tv_nsec = (NS_PER_MSEC * 10);
			}
		}
	}

	/* Don't need to lock timers anchored by the uthread structure */
        if (timerid < NTIMERS) {
                if (timer_locked = (p->p_active > 1))
                        ipri = disable_lock(INTTIMER, &U.U_timer_lock);
                else
                        ipri = i_disable(INTTIMER);
        }

	/*  Check to see that the timerid is that of a valid timer.  */
	if(TIMERID_NOTVALID(timerid))  {
	        if (timerid < NTIMERS) {
	       	        if (timer_locked)
       		                unlock_enable(ipri, &U.U_timer_lock);
	                else
	                        i_enable(ipri);
	        }
		ut->ut_error = EINVAL;
		return(-1);
	}

        /* Determine base address of timers: ublock or uthread. */
        if ((timerid >= 0) && (timerid < NTIMERS)) {
                timer_array = &U.U_timer[0];
		index = timerid;
        }
        else {
                timer_array = &ut->ut_timer[0];
		index = timerid - TIMERID_REAL1;
        }

	/* local copy of trb */
	t = timer_array[index];

	if(TIMERID_ISBSD(timerid))
		ntimerclear(&ct);
	else
		curtime(&ct);

	if(t->flags & T_ACTIVE)  {
		/*
		 *  If this is not a BSD profile or virtual timer
		 *  and we are going to set a new time, then the 
		 *  timeout request must be deactivated.
		 */
		if(!(TIMERID_ISBSD(timerid)) && (value != NULL))  {
                	while (tstop(t));
		}

		/*
		 *  The value for the time remaining until the timeout must
		 *  be returned to the caller.  This value was initialized 
		 *  to zero.  If the current time is less than the time in 
		 *  the timeout structure, then zero is the correct value.
		 *  Otherwise, the difference between the value in the 
		 *  timeout structure and the current time must be calculated
		 *  and put in the structure which will be copyout'd to
		 *  the caller.
		 */
		if(ntimercmp(ct, t->timeout.it_value, <))  {
			ntimersub(t->timeout.it_value, ct, ocp.it_value);
		}
		ocp.it_interval = t->timeout.it_interval;
	}

        if (value == NULL)                      /* called for info only */
                goto abs_return;
        t->timeout = icp;
        if (!ntimerisset(&(icp.it_value)))      /* Disable timer */
                goto abs_return;

	/*
	 *  BSD profile and virtual timers are handled differently 
	 *  from the rest - they do not get placed on the system or 
	 *  process active list.  Instead, the flags are just set 
	 *  to indicate that the timer is active.
	 */
	if(TIMERID_ISBSD(timerid)) {
		t->flags = (T_ABSOLUTE | T_ACTIVE);
	}
	else  {
		t->flags = T_ABSOLUTE;

		/*
		 *  If the time specified has already occurred set the
		 *  time to the current time so that periodic timers can 
		 *  be processed correctly.  We could go into an apparent
		 *  infinite loop processing the same timer request, if 
	         *  it were set back sufficiently.  Time is maintained
		 *  in seconds and nanosecs since the EPOCH.
		 */
		if (ntimercmp(t->timeout.it_value, ct, <)) {
			t->timeout.it_value = ct;
		}

		/*
		 *  Activate the request.  Note that there is no need
		 *  to bother checking if the time has already passed
		 *  - if it has, an immediate timeout will occur.
		 */
		tstart(t);
	}

abs_return:

        if (timerid < NTIMERS) {
                if (timer_locked)
                        unlock_enable(ipri, &U.U_timer_lock);
                else
                        i_enable(ipri);
        }

        /*  Return the current timeout values to the caller if requested  */
        if (ovalue != NULL) {
                if((copyout(&ocp, ovalue, sizeof(struct itimerstruc_t))) != 0){
                         ut->ut_error = EFAULT;
                         return(-1);
                }
        }

	return(0);
}


/*
 * NAME:  incinterval (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  Requests notification after a specific interval of time
 *	(i.e. sets the value of a per-process timer to a given offset
 *	from the current timer setting).
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called from a process.
 *
 *	This routine may page fault.
 *                                                                   
 * NOTES:  The value and ovalue parameters are pointers to itimerstruc_t
 *	structures.  The itimerstruct_t structure is made up of two
 *	timestruc_t structures.  One of these timestruc_t structures
 *	(it_value) contains the offset from the current timer value
 *	for which the requested notification should be scheduled.  If
 *	it_value is 0, the per-process timer is disabled and the amount
 *	of time left before the timer would have expired is returned
 *	in the ovalue parameter.
 *
 *	The other timestruc_t structure (it_interval) contains the 
 *	value for subsequent timeouts.  Thus, if it_interval is zero,
 *	a "one-shot" timer is requested.  Conversely, if it_interval
 *	is not zero, a "periodic" timer is requested.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error.
 *	If successful, the ovalue parameter is set to the previous timer
 *	setting (i.e. the amount of time left until the timer would have
 *	expired).  Upon error, errno is set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EINVAL:  A value structure specified a nanosecond value less than
 *		 zero or greater than or equal to one billion;
 *	EINVAL:  The timerid parameter does not correspond to an id
 *		 returned by gettimerid().
 */  
int
incinterval(register timer_t timerid, 
            register struct itimerstruc_t *value, 
            register struct itimerstruc_t *ovalue)
{
	struct	 timestruc_t	ct;	/* current time			*/
	struct	 timestruc_t	rt;	/* resolved time		*/
	struct	 itimerstruc_t	icp;	/* copy of value parameter	*/
	struct	 itimerstruc_t	ocp;	/* copy of ovalue parameter	*/
	register struct trb	*t;	/* timer structure to submit	*/
	struct   thread         *th;
	struct   proc         	*p;
	struct   uthread        *ut;
	struct   trb            **timer_array;
	int			index;
	int			ipri;
        boolean_t             timer_locked;

	TRCHKLT_SYSC(INCINTERVAL, timerid);

	ASSERT(csa->prev == NULL);

	th = curthread;
	p = th->t_procp;
	ut = th->t_uthreadp;

	/*  Check to see that this is not a BSD profile or virtual timer.  */
	if(TIMERID_ISBSD(timerid))  {
		ut->ut_error = EINVAL;
		return(-1);
	}

	/* initialize local copies of input and output structures   */
	ntimerclear(&(icp.it_value));
	ntimerclear(&(icp.it_interval));
	ntimerclear(&(ocp.it_value));
	ntimerclear(&(ocp.it_interval));

	/*  
	 * Get the time parameter from user space if there is one 
	 * If there's not, then we're probably being called just 
	 * to return the time until the next timeout. 
         */
	curtime(&ct);
	if(value != NULL)
	{
		if((copyin(value, &icp, sizeof(struct itimerstruc_t))) != 0)  
		{
			ut->ut_error = EFAULT;
			return(-1);
		}

		/*  Validate the time parameters.  */
		if((icp.it_value.tv_nsec >= NS_PER_SEC) ||
	   	   (icp.it_value.tv_nsec < 0)  ||
	   	   (icp.it_interval.tv_nsec >= NS_PER_SEC) ||
	   	   (icp.it_interval.tv_nsec < 0 )) 
		{
			ut->ut_error = EINVAL;
			return(-1);
		}

		/*
	 	 *  Only privileged users may submit fine granularity timers.  
		 *  If the time is non-zero AND the user is NOT privileged AND 
		 *  the timer request is for LESS THAN 10 millisecond, then 
		 *  make the timer request 10 millisecond.
	 	 */
		if ((privcheck(SET_PROC_RAC) == EPERM) && (value != NULL))  
		{
			if((icp.it_value.tv_sec == 0) &&
		   	   (icp.it_value.tv_nsec != 0) &&
		   	   (icp.it_value.tv_nsec < (NS_PER_MSEC * 10)))  {
				icp.it_value.tv_nsec = (NS_PER_MSEC * 10);
			}
			if((icp.it_interval.tv_sec == 0) &&
		   	   (icp.it_interval.tv_nsec != 0) &&
		   	   (icp.it_interval.tv_nsec < (NS_PER_MSEC * 10)))  {
				icp.it_interval.tv_nsec = (NS_PER_MSEC * 10);
			}
		}
	
		/* 
		 *  Ensure that the resolved time of the request is in the
		 *  future.  Time is kept as a 32 bit word, which can be
		 *  truncated, if it_value is sufficiently large.
		 */
		ntimeradd(ct, icp.it_value, rt);
		if (ntimercmp(rt, ct, <))  
		{
			ut->ut_error = EINVAL;
			return(-1);
		}
	}

        /* Don't need to lock timers anchored by the uthread structure */
        if (timerid < NTIMERS) {
                if (timer_locked = (p->p_active > 1))
                        ipri = disable_lock(INTTIMER, &U.U_timer_lock);
                else
                        ipri = i_disable(INTTIMER);
        }

        /*  Check to see that the timerid is that of a valid timer.  */
        if(TIMERID_NOTVALID(timerid))  {
	        if (timerid < NTIMERS) {
	                if (timer_locked)
	                        unlock_enable(ipri, &U.U_timer_lock);
	                else
	                        i_enable(ipri);
	        }
                ut->ut_error = EINVAL;
                return(-1);
        }

        /* Determine base address of timers: ublock or uthread. */
        if ((timerid >= 0) && (timerid < NTIMERS)) {
                timer_array = &U.U_timer[0];
		index = timerid;
        }
        else {
                timer_array = &ut->ut_timer[0];
		index = timerid - TIMERID_REAL1;
        }

        /* local copy of trb */
	t = timer_array[index];

	if(t->flags & T_ACTIVE)  
	{
		/*  The timeout request must be deactivated. */
		if(value != NULL) {
                	while (tstop(t));
		}
		
		/*
		 *  The value for the time remaining until the timeout must
		 *  be returned to the caller.  This value was initialized 
		 *  to zero.  If the current time is less than the time in 
		 *  the timeout structure, then zero is the correct value.
		 *  Otherwise, the difference between the value in the 
		 *  timeout structure and the current time must be calculated
		 *  and put in the structure which will be copyout'd to
		 *  the caller.
		 */
		if(ntimercmp(ct, t->timeout.it_value, <))  {
			ntimersub(t->timeout.it_value, ct, ocp.it_value);
		}
		ocp.it_interval = t->timeout.it_interval;
	}
    	
	if (value == NULL) 			/* called for info only */
		goto inc_return;
	t->timeout = icp;
	if (!ntimerisset(&(icp.it_value))) 	/* Disable timer */
		goto inc_return;

	/*
	 *  A request needs to be submitted only if the timeout value specified
	 *  by the caller is non-zero.
	 */
	t->flags = T_INCINTERVAL;
	
	/*  Submit the trb.  */
	tstart(t);
	    
inc_return:

        if (timerid < NTIMERS) {
                if (timer_locked)
                        unlock_enable(ipri, &U.U_timer_lock);
                else
                        i_enable(ipri);
        }

	/*  Return the current timeout values to the caller if requested */
        if(ovalue != NULL)
	{
	  	if((copyout(&ocp, ovalue, sizeof(struct itimerstruc_t))) != 0)
		{
			ut->ut_error = EFAULT;
		  	return(-1);
	  	}
	}

	return(0);
}


/*
 * NAME:  gettimerid (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  This service provides a process with a unique identifier for
 *	each timer request it makes of the system.  Moreover, it allows a
 *	process to select a particular type of timer.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called from a process.
 *
 *	This routine may page fault.
 *                                                                   
 * NOTES:  Notification via signals is the only delivery mechanism supported
 *	at this time for POSIX TIMEOFDAY time_types.  Which signal is sent 
 *	depends upon the timer type.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  A timer_t upon successful completion, -1 upon
 *	error.  If successful, a unique identifier of a per-process interval 
 *	timer is returned.  Upon error, errno is set as follows:
 *	EINVAL:	The type of timer passed as a parameter to gettimerid()
 *		is not defined.
 *	EINVAL:	The type of timer passed as a parameter to gettimerid()
 *		does not allow the specified delivery mechanism.
 *	ENOSPC:	The calling process has already allocated all of the 
 *		interval timers associated with the specified timer
 *		type for this implementation.
 *	EAGAIN:	Memory could not be allocated for a timer structure.
 */  
timer_t
gettimerid(register int timerid, register int event_type)
{
	register timer_t    index;	/* index into user timer array	*/
	register heapaddr_t heap;	/* from pinned_ or kernel_heap	*/
	register struct trb *local_trb;
	struct   thread     *th;
	struct   proc       *p;
	struct   uthread    *ut;
	int		    ipri;
        struct   trb        **timer_array;
        boolean_t             timer_locked;


	ASSERT(csa->prev == NULL);

	th = curthread;
	p = th->t_procp;
	ut = th->t_uthreadp;

	/*
	 *  Notification via signals is the only delivery mechanism supported 
	 *  at this time.  Thus, the BSD routines are expected to call this
	 *  service with DELIVERY_SIGNALS.
	 */
	if(event_type != DELIVERY_SIGNALS)  {
		ut->ut_error = EINVAL;
		return((timer_t) -1);
	}

	/* Determine base address of timers: ublock or uthread. */
	if ((0 <= timerid) && (timerid <= NTIMERS)) {
		timer_array = &U.U_timer[0];
		index = 0;
	}
	else if ((TIMERID_REAL1 <= timerid) && 
		 (timerid < TIMERID_REAL1 + NTIMERS_THREAD)) {
		timer_array = &ut->ut_timer[0];
		index = -TIMERID_REAL1;
	}
	else {
            	ut->ut_error = EINVAL;
		return(-1);
	}

	/* Preallocate trb */
	local_trb = talloc();
	if (local_trb == NULL) {
                ut->ut_error = EAGAIN;
                return((timer_t) -1);
        }

	/* Don't need to lock timers anchored by the uthread structure. */
        if (timerid <= NTIMERS) {
                if (timer_locked = (p->p_active > 1))
                        ipri = disable_lock(INTTIMER, &U.U_timer_lock);
                else
                        ipri = i_disable(INTTIMER);
        }

	switch(timerid)  {
	case ITIMER_VIRTUAL:
	case ITIMER_PROF:
	case ITIMER_REAL:
	case ITIMER_REAL1:
	case ITIMER_VIRT:
		/*
		 *  If the slot is already initialized, there is no
		 *  work to do and the slot # can be returned to the
		 *  caller.  Otherwise, index is set and the slot
		 *  is intialized after the switch statement.
		 */
		index += timerid;
		if(timer_array[index] != NULL) {
		        if (timerid <= NTIMERS) {
		                if (timer_locked)
		                        unlock_enable(ipri, &U.U_timer_lock);
		                else
		                        i_enable(ipri);
		        }
			tfree(local_trb);
                        return((timer_t) timerid);
		}
                break;
	case TIMEOFDAY:
		for(index = TIMERID_TOD; 
		    index < (TIMERID_TOD + NTIMEOFDAY); 
		    index++)  {
			if(timer_array[index] == NULL)  {
				break;	/* for(index... */
			}
		}
		if(index == TIMERID_TOD + NTIMEOFDAY)  {
			/*
			 *  If index is equal to above, then  there must 
		 	 *  not have been any available timer slots.
			 */
		        if (timerid <= NTIMERS) {
		                if (timer_locked)
		                        unlock_enable(ipri, &U.U_timer_lock);
		                else
		                        i_enable(ipri);
		        }
			ut->ut_error = ENOSPC;
			tfree(local_trb);
               		return((timer_t) -1);
		}
		timerid = index;
		break;
	}

	/*
	 *  A slot is available.  Allocate the trb and fill in the func
	 *  and pid fields if it is a TIMEOFDAY type timer.
	 */
	timer_array[index] = local_trb;
        local_trb->ipri = INTTIMER;
	local_trb->id = p->p_pid;

        if(!TIMERID_ISBSD(timerid))
	{
		local_trb->func = (void(*)())interval_end;
		if (timerid == ITIMER_REAL1) {
			local_trb->func_data = SIGALRM1;
			local_trb->id = th->t_tid;
		}
		else
			local_trb->func_data = SIGALRM;
	}

        if (timerid <= NTIMERS) {
                if (timer_locked)
                        unlock_enable(ipri, &U.U_timer_lock);
                else
                        i_enable(ipri);
        }

	return((timer_t) timerid);
}


/*
 * NAME:  gettimer (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  Get the current value of the system-wide timer specified 
 *	by the timer_type parameter.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called under a process.
 *
 *	This routine may page fault.
 *                                                                   
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error 
 *	with errno set as follows:
 *	EINVAL:  The type of timer passed as a parameter to gettimer()
 *		 does not specify a known timer.
 *	EFAULT:  An argument address referenced invalid memory;
 */  
int
gettimer(register int timer_type, register struct timestruc_t *tp)
{
	struct	 timestruc_t ct;		/* to get current time	*/
	struct   uthread     *ut;

	TRCHKLT_SYSC(GETTIMER, timer_type); 

	ASSERT(csa->prev == NULL);

	ut = curthread->t_uthreadp;

	/*  Validate input parameter.  */
	if(timer_type != TIMEOFDAY)  {
		ut->ut_error = EINVAL;
		return(-1);
	}

	curtime(&ct);

	if((copyout(&ct, tp, sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}

	return(0);
}


/*
 * NAME:  settimer (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  Sets the value of a system-wide timer.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called under a process.
 *
 *	This routine may page fault.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error 
 *	with errno set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EPERM:   The requesting process does not have the appropriate 
 *		 privilege to set the system time;
 *	EINVAL:  The type of timer passed as a parameter to settimer()
 *		 does not specify a known timer.
 */  
int
settimer(register int timer_type, 
         register struct timestruc_t *tp)
{
	struct timestruc_t 	nct;	/* new current time (to set)	*/
	static int 		svcnum = 0;
	struct uthread		*ut;

  	TRCHKLT_SYSC(SETTIMER, timer_type);

	ASSERT(csa->prev == NULL);

	ut = curthread->t_uthreadp;

	if(audit_flag && audit_svcstart("PROC_Settimer", &svcnum, 0) )
		audit_svcfinis();

	/*  Only a user with authority can set the time.  */
	if(privcheck(SYS_CONFIG) == EPERM){
		ut->ut_error = EPERM;
		return(-1);
	}

	/*  Validate input parameter.  */
	if(timer_type != TIMEOFDAY)  {
		ut->ut_error = EINVAL;
		return(-1);
	}

	if((copyin(tp, &nct, sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}

	/* validate tp input parameter */
	if((nct.tv_nsec < 0) || (nct.tv_nsec >= NS_PER_SEC)){ 
		ut->ut_error = EINVAL;
		return(-1);
	}

	ksettimer(&nct);

	/* set if date changed by system call. */
	time_adjusted = 1;

	return(0);
}


/*
 * NAME:  resabs (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  Returns the resolution of the requested timer.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *	This routine may only be called under a process.
 *
 *	This routine may page fault.
 *                                                                   
 *                                                                   
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error.
 *	If successful, the values calculated as the resolution and the
 *	maximum value for the timer specified by the timer_type parameter
 *	are placed at the addresses specified by the res and max
 *	parameters, respectively.  Upon error, errno is set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EINVAL:  The type of timer passed as a parameter to resabs()
 *		 does not specify a known timer.
 */
int
resabs(register timer_t timerid,          /* Timerid for which res wanted*/
       register struct timerstruc_t *res, /* Minimum resolution of the timer*/
       register struct timerstruc_t *max) /* Maximum value of the timer	*/
{
	struct	 timestruc_t	tmrval;
	struct	 uthread	*ut;

	TRCHKLT_SYSC(RESABS,timerid);

	ASSERT(csa->prev == NULL);

	ut = curthread->t_uthreadp;

	if(TIMERID_NOTVALID(timerid)) {
		ut->ut_error = EINVAL;
		return(-1);
	}

	if(!(TIMERID_ISBSD(timerid))) {

			tmrval.tv_sec = 0;
                        tmrval.tv_nsec = min_ns;

			/*
			 *  Only privileged users may submit fine granularity 
			 *  timers.  If the user is NOT privileged AND the 
			 *  timer resolution is LESS THAN 10 millisecond, then 
			 *  make the timer resolution 10 millisecond.
			 */
			if(privcheck(SET_PROC_RAC) == EPERM)  {
				if(tmrval.tv_nsec < (NS_PER_MSEC * 10))  {
					tmrval.tv_nsec = (NS_PER_MSEC * 10);
				}
			}

	} else {
			/*  Give the user one clock tick resolution.  */
			tmrval.tv_sec = 0;
			tmrval.tv_nsec = NS_PER_SEC / HZ;
	}

	/*  Copy out minimum values to the user.  */
	if((copyout(&tmrval, res, 
	        sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}

	/*  Give the maximum values to the user.  */
	tmrval.tv_sec = MAXTODSEC;
	tmrval.tv_nsec = MAXTODNSEC;
	if((copyout(&tmrval, max, 
	        sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}
	
	return(0);
}


/*
 * NAME:  restimer (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  Returns the resolution of the requested timer.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called under a process.
 *
 *	This routine may page fault.
 *                                                                   
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error.
 *	If successful, the values calculated as the resolution and the
 *	maximum value for the timer specified by the timer_type parameter
 *	are placed at the addresses specified by the res and maxval
 *	parameters, respectively.  Upon error, errno is set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EINVAL:  The type of timer passed as a parameter to settimer()
 *		 does not specify a known timer.
 */  
int
restimer(register int timer_type,             /*Type of system timer */
         register struct timestruc_t *res,    /*Min resolution of the timer*/
         register struct timestruc_t *maxval) /*Max value of the timer */
{
	struct	 timestruc_t	tmrval;
	struct   uthread        *ut;

	ut = curthread->t_uthreadp;

	TRCHKLT_SYSC(RESTIMER,timer_type);

	switch(timer_type) {

		case TIMEOFDAY:
			tmrval.tv_sec = 0;
                        tmrval.tv_nsec = min_ns;

			/*  Give the minimum values to the user. */
			if((copyout(&tmrval, res, 
				    sizeof(struct timestruc_t))) != 0)  {
				ut->ut_error = EFAULT;
				return(-1);
			}

			/*  Give the maximum values to the user.  */
			tmrval.tv_sec = MAXTODSEC;
			tmrval.tv_nsec = MAXTODNSEC;
			if((copyout(&tmrval, maxval, 
				    sizeof(struct timestruc_t))) != 0) {
				ut->ut_error = EFAULT;
				return(-1);
			}
			break;

		default:
			/*  No such timer.  */
			ut->ut_error = EINVAL;
			return(-1);
		}
		
	return(0);
}


/*
 * NAME: resinc (POSIX 1003.4)
 *                                                                    
 * FUNCTION: Return the resolution of the requested timer.
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *	This routine may only be called under a process.
 *
 *	This routine may page fault.
 *                                                                   
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error.
 *	If successful, the values calculated as the resolution and the
 *	maximum value for the timer specified by the timer_type parameter
 *	are placed at the addresses specified by the res and max
 *	parameters, respectively.  Upon error, errno is set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EINVAL:  The type of timer passed as a parameter to settimer()
 *		 does not specify a known timer.
 */  
int
resinc(register timer_t timerid,         /*Timerid for which res wanted*/
       register struct timestruc_t *res, /*Minimum resolution of the timer*/
       register struct timestruc_t *max) /*Maximum value of the timer*/
{
	struct   timestruc_t	tmrval;
	struct   uthread	*ut;

	TRCHKLT_SYSC(RESINC, timerid); 

	ut = curthread->t_uthreadp;

	if((TIMERID_NOTVALID(timerid)) || (TIMERID_ISBSD(timerid))) {
		ut->ut_error=EINVAL;
		return(-1);
	}

	tmrval.tv_sec = 0;
        tmrval.tv_nsec = min_ns;

	/*
	 *  Only privileged users may submit fine granularity 
	 *  timers.  If the user is NOT privileged AND the 
	 *  timer resolution is LESS THAN 10 millisecond, then 
	 *  make the timer resolution 10 millisecond.
	 */
	if(privcheck(SET_PROC_RAC) == EPERM)  {
		if(tmrval.tv_nsec < (NS_PER_MSEC * 10))  {
			tmrval.tv_nsec = (NS_PER_MSEC * 10);
		}
	}

	/*  Give the minimum values to the user.  */
	if((copyout(&tmrval, res, 
		    sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}

	/*  Give the maximum values to the user.  */
	tmrval.tv_sec = MAXTODSEC;
	tmrval.tv_nsec = MAXTODNSEC;
	if((copyout(&tmrval, max, 
		    sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}
	
	return(0);
}


/*
 * NAME:  tsig
 *
 * FUNCTION:  Called from sig_deliver under a process in supervisor state.  
 *		This routine examines the state of the timers for the 
 *		current process.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called under a process from sig_deliver().
 *
 *      It may page fault.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:
 */
tsig()
{
}


/*
 * NAME:  reltimerid (POSIX 1003.4)
 *                                                                    
 * FUNCTION:  This service frees up a timer which has been initialized 
 *	via a call to gettimerid().
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called from a process.
 *
 *	This routine may page fault.
 *                                                                   
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error 
 *	with errno set as follows:
 *	EINVAL:  The type of timer passed as a parameter to reltimerid()
 *		 is not defined.
 */  
int
reltimerid(timerid)
register timer_t timerid;		/* id of timer to be freed	*/
{
	struct thread 		*th;
	struct proc 		*p;
	struct uthread 		*ut;
	int 			ipri;
        struct   trb            **timer_array;
        int                     index;
        boolean_t             timer_locked;


	ASSERT(csa->prev == NULL);

	th = curthread;
	p = th->t_procp;
	ut = th->t_uthreadp;
	
	/* Don't need to lock timers anchored by the uthread structure */
        if (timerid < NTIMERS) {
                if (timer_locked = (p->p_active > 1))
                        ipri = disable_lock(INTTIMER, &U.U_timer_lock);
                else
                        ipri = i_disable(INTTIMER);
        }

	/*  Validate input parameter.  */
	if(TIMERID_NOTVALID(timerid))  {
	        if (timerid < NTIMERS) {
        	        if (timer_locked)
	               	        unlock_enable(ipri, &U.U_timer_lock);
	       	        else
	                        i_enable(ipri);
	        }
		ut->ut_error = EINVAL;
		return(-1);
	}

        /* Determine base address of timers: ublock or uthread. */
        if ((timerid >= 0) && (timerid < NTIMERS)) {
                timer_array = &U.U_timer[0];
		index = timerid;
        }
        else {
                timer_array = &ut->ut_timer[0];
		index = timerid - TIMERID_REAL1;
        }

	if(!(TIMERID_ISBSD(timerid)))  {
		/*
		 *  For non-BSD timers, if the timer associated with this
		 *  timerid is active, then it needs to be deactivated (i.e.
		 *  taken off the system active list via a call to tstop()) 
		 *  before it can be freed.
		 */
		if((timer_array[index]->flags & T_ACTIVE))  {
			while (tstop(timer_array[index]));
		}
	}
	else  {
		/*
		 *  For BSD timers, if the timer associated with this timerid
		 *  is active, then it needs to be deactivated (i.e. just clear
		 *  the T_ACTIVE flag) before it can be freed.
		 */
		if((timer_array[index]->flags & T_ACTIVE))  {
			timer_array[index]->flags &= ~T_ACTIVE;
		}
	}

	tfree(timer_array[index]);

	timer_array[index] = NULL;

        if (timerid < NTIMERS) {
                if (timer_locked)
                        unlock_enable(ipri, &U.U_timer_lock);
                else
                        i_enable(ipri);
        }

	return(0);
}


/*
 * NAME:  texit
 *
 * FUNCTION:  
 *	Clean up all timer related structures and requests for an
 *	exiting process or thread.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called under a process only.
 *
 *	This routine may page fault.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION: None
 */
void
texit(register struct user *user, register struct uthread *ut)
{
	register int	index;			/* index into timer array */	

	/* Free process timers */
	if (user != NULL) {
		for(index = 0; index < NTIMERS; index++)  {
			if(user->U_timer[index] != NULL)  {
				/*  Free up this timer.  */
				(void) reltimerid(index);
			}
	 	}
	}

	/* Free thread timers */
	if (ut != NULL) {
        	for(index = 0; index < NTIMERS_THREAD; index++)  {
                	if(ut->ut_timer[index] != NULL)  {
                        	/*  Free up this timer.  */
                        	(void) reltimerid(index + TIMERID_REAL1);
                	}
        	}
	}
}

/*
 * NAME: nsleep (POSIX 1003.4)
 *
 * FUNCTION: High resolution process sleep
 *
 *           This routine exists to provide libpthreads with an intercept
 *           point for the call to nsleep.  Control flows immediately
 *           to nsleep, with no additional logic.
 *
 */
int
_nsleep(register struct timestruc_t *rqtp, /*Time interval to suspend exec*/
        register struct timestruc_t *rmtp) /*Time remaining on interval or 0*/
{
        return nsleep( rqtp, rmtp );
}

/*
 * NAME: nsleep (POSIX 1003.4)
 *                                                                    
 * FUNCTION: High resolution process sleep
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *	This routine may only be called under a process.
 *
 *	This routine may page fault.
 *                                                                   
 * RETURN VALUE DESCRIPTION:  0 if the requested sleep time had elapsed,
 *	-1 if the nsleep() returned due to the notification of a signal.
 *	In the latter case, the rmtp argument is updated to contain the 
 *	unslept amount (the requested time minus the time actually slept),
 *	and errno is set as follows:
 *	EINTR:	 A signal was caught by the calling process and control
 *		 has been returned from the signal-catching routine;
 *	EINVAL:  The rqtp argument specified a nanosecond value less than
 *		 zero or greater than or equal to one billion.
 *	EFAULT:  An argument address referenced invalid memory;
 */  
int
nsleep(register struct timestruc_t *rqtp, /*Time interval to suspend exec*/
       register struct timestruc_t *rmtp) /*Time remaining on interval or 0*/
{
	register int	ipri;		/* caller's interrupt priority	*/
	register int	rv;		/* return value from e_sleep	*/
	register struct	trb	*trb;	/* trb to submit timeout request*/
	struct	 timestruc_t	tcp;	/* copyin/out for parameters	*/
	struct   thread		*th;
	struct   uthread	*ut;

	ASSERT(csa->prev == NULL);

	th = curthread;
	ut = th->t_uthreadp;

	/*  Get the time parameter from user space.  */
	if((copyin(rqtp, &tcp, sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}

	/*  Validate input parameter.  */
	if((tcp.tv_nsec >= NS_PER_SEC) ||
	   (tcp.tv_nsec < 0))  {
		ut->ut_error = EINVAL;
		return(-1);
	}

	/*  Allocate the timer request block.  */
	trb = talloc();
	if (trb == (struct trb *)NULL) {
		ut->ut_error = EAGAIN;
		return(-1);
	}

	/*
	 *  Ensure that this is treated as an incremental timer, not an
	 *  absolute one.
	 */
	trb->flags		=  T_INCINTERVAL;
	trb->func		=  (void (*)())rtsleep_end;
	trb->eventlist		=  EVENT_NULL;
        trb->ipri               =  INTTIMER;
	trb->id			=  th->t_tid;
	trb->func_data		=  (uint)(&(trb->eventlist));
	trb->timeout.it_value 	=  tcp;

	e_assert_wait(&(trb->eventlist), TRUE);

	tstart(trb);

	rv = e_block_thread();
	switch (rv) {
		case THREAD_AWAKENED :
			rv = EVENT_SUCC;
			break;
		default :
			rv = EVENT_SIG;
			break;
	}	

	while (tstop(trb));

	if(rv == EVENT_SIG)  {

		curtime(&tcp);
		if(ntimercmp(tcp, trb->timeout.it_value, <))  {
			ntimersub(trb->timeout.it_value, tcp, tcp);
		}
		else  {
			/*
			 *  The process was signalled out of the sleep but
			 *  time has advanced to the point of the sleep any
			 *  way so set the "time unslept" parameter to 0.
			 */
			tcp.tv_sec = tcp.tv_nsec = 0;
		}
                ut->ut_error = EINTR;
		rv = -1;
	}
	else  {
		/*
		 *  Process sleep went to term so set the "time unslept"
		 *  parameter to 0.
		 */
		tcp.tv_sec = tcp.tv_nsec = 0;
		rv = 0;
	}
	tfree(trb);

	/*  Return the unslept time to the user.  */
	if((copyout(&tcp, rmtp, sizeof(struct timestruc_t))) != 0)  {
		ut->ut_error = EFAULT;
		return(-1);
	}

	return(rv);
}
