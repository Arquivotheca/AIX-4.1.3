static char sccsid[] = "@(#)14	1.10.1.8  src/bos/kernel/proc/POWER/m_berk.c, sysproc, bos41J, 9521A_all 5/19/95 18:09:36";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: adj_tick
 *		adjtime
 *		kgettickd
 *		ksettickd
 *
 *   ORIGINS: 26,27,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>		/* always needed			*/
#include <sys/adspace.h>        /* for the WRITE_CLOCK macro to work    */
#include <sys/time.h>		/* for the timeval structure		*/
#include <sys/param.h>		/* to define the HZ label		*/
#include <sys/mstsave.h>	/* mstsave area def. for asserting	*/
#include <sys/user.h>		/* the u structure to return errnos	*/
#include <sys/errno.h>		/* define the errno's to be returned	*/
#include <sys/syspest.h>	/* for the ASSERT and assert macros	*/
#include <sys/intr.h>		/* for the serialization stuff		*/
#include <sys/rtc.h>		/* for real time clock related defines	*/
#include <sys/low.h>		/* access the csa for asserts		*/
#include <sys/machine.h>	/* for machine model macros		*/
#include <sys/systemcfg.h>	/* for system config structure          */
#include <sys/sys_resource.h>	/* for system resource structure        */

int tickadj = (uS_PER_SEC / HZ) / 10; /* standard clock adjust, ms./tick */
long bigadj = 1000000;		/* use 5x skew if greater than bigadj us.*/
int uSdelta = 0;                /* current clock skew, us. per tick      */

extern Simple_lock tod_lock;
extern void set_time();

/*
 * True if time changed through system call : adjtime or settimer.
 */
int time_adjusted = 0;

struct ppda_adjtime {
	int	prev_delta;
	int 	delta;
} timedelta[MAXCPU];

/*
 * NAME:  adjtime (BSD)
 *                                                                    
 * FUNCTION:  Correct the time to allow synchronization of the system clock.
 *            Send the adjtime values to the other processors
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
 *		 privilege to set the system time.
 */  
int
adjtime(struct timeval *delta, struct timeval *olddelta)
{
	struct   timeval atv;		/* timeval to adjust by		*/
	struct   timeval oatv;		/* old timeval to adjust by	*/
	register long	 ndelta;
	int i;
	int ipri;
	int most_advanced;
	cpu_t my_cpu=CPUID;
	int ncpus=NCPUS();

	ASSERT(csa->prev == NULL);

	/*  Only a user with authority can set the time.  */
	if(privcheck(SYS_CONFIG) == EPERM)  {
		u.u_error = EPERM;
		return(-1);
	}

	if((copyin((caddr_t)delta, (caddr_t)&atv, 
		   sizeof(struct timestruc_t))) != 0)  {
		u.u_error = EFAULT;
		return(-1);
	}

	ndelta = atv.tv_sec * 1000000 + atv.tv_usec;

	ipri = disable_lock(INTMAX, &tod_lock);

	/*
	 * Calculate global tick adjustment
	 */
        if (ndelta > bigadj)
                uSdelta = 5 * tickadj;
        else
                uSdelta = tickadj;

	/* 
	 * 1 ms is the lower limit for clock adjustments.
	 * truncate request to 1ms increments.			
	 */
	if (ndelta % uSdelta)
		ndelta = (ndelta / uSdelta) * uSdelta;

	if (olddelta != NULL) {
		oatv.tv_sec = timedelta[my_cpu].delta / 1000000;
		oatv.tv_usec = timedelta[my_cpu].delta % 1000000;
	}

	/*
	 * If all processors have not made the same time adjustments
	 * then calculate the value needed on a per processor basis 
	 * to catch up.  Store this value in the processor's prev_delta
	 * field to be applied in a single increment.
	 */
	most_advanced = 0;
	for (i = 0; i < ncpus; i++) {
		if (timedelta[i].delta < 0)
			if ((most_advanced == 0) ||
			    (most_advanced < timedelta[i].delta))
				most_advanced = timedelta[i].delta;
		else if (timedelta[i].delta > 0)
			if ((most_advanced == 0) ||
			    (most_advanced > timedelta[i].delta))
				most_advanced = timedelta[i].delta;
	}
	for (i = 0; i < ncpus; i++) {
		timedelta[i].prev_delta += (timedelta[i].delta - most_advanced);
	 	timedelta[i].delta = ndelta;
	}

	/* Set if time adjusted through system call */
	time_adjusted = TRUE;

	unlock_enable(ipri, &tod_lock);

	if(olddelta != NULL)  {
		(void)copyout((caddr_t)&oatv, (caddr_t)olddelta,
			      sizeof (struct timeval));
	}
	return(0);
}


/*
 * NAME:  adj_tick
 *
 * FUNCTION:  Adjust the time each clock tick if an adjtime() was done.
 *
 * EXECUTION ENVIRONMENT:
 *	This routine is called at INTTIMER from sys_timer().
 *
 *      It does not page fault.
 *
 * EXTERNAL PROCEDURES CALLED: None
 */
void
adj_tick()
{
	struct timestruc_t nt;		/* current system time		*/
	struct timestruc_t dt;		/* delta to current time	*/
	cpu_t my_cpu=CPUID;

	ASSERT(csa->intpri == INTMAX);

#ifdef _POWER_MP
	simple_lock(&tod_lock);
#endif
	
	if ((timedelta[my_cpu].prev_delta == 0) &&
	    (timedelta[my_cpu].delta == 0)) {
#ifdef _POWER_MP
		simple_unlock(&tod_lock);
#endif
		return;
	}
	
	dt.tv_sec = 0;

	/* 
	 * Update the current time (ct) and the reference time
         * for the next clock tick (ref_time).                 
	 */
#ifdef _PEGASUS
        if (__pegasus())
                pgs_rtc_stop();
#endif
	curtime(&nt);

	if (timedelta[my_cpu].prev_delta < 0) {
	    	dt.tv_nsec = -(timedelta[my_cpu].prev_delta) * 
					(NS_PER_SEC / (uS_PER_SEC));
		ntimersub(nt, dt, nt);
		timedelta[my_cpu].prev_delta = 0;
	}
	else if (timedelta[my_cpu].prev_delta > 0) {
	    	dt.tv_nsec = timedelta[my_cpu].prev_delta * 
					(NS_PER_SEC / (uS_PER_SEC));
		ntimeradd(nt, dt, nt);
		timedelta[my_cpu].prev_delta = 0;
	}
	else { 
	        dt.tv_nsec = uSdelta * (NS_PER_SEC / (uS_PER_SEC));

		if (timedelta[my_cpu].delta < 0) {
			timedelta[my_cpu].delta += uSdelta;
			ntimersub(nt, dt, nt);
		}
		else  {
			timedelta[my_cpu].delta -= uSdelta;
			ntimeradd(nt, dt, nt);
		}
	}

	/* update hardware time and memory mapped time variables */
	set_time(nt);
#ifdef _PEGASUS
        if (__pegasus())
                pgs_rtc_start();
#endif /* _PEGASUS */

	/*  Restore caller's interrupt priority.  */
#ifdef _POWER_MP
	simple_unlock(&tod_lock);
#endif
}

/*
 * NAME:  kgettickd, ksettickd
 *
 * FUNCTION: These services allow kernel extensions to set/get the
 *  static variables timedelta and uSdelta, which are referenced
 *  during the clock interrupt by sys_timer(), and are used to
 *  account for the drift of the clock.  The clock is adjusted
 *  accordyingly.
 *
 * EXECUTION ENVIRONMENT:
 *  These services should not page fault.
 *  Caller is expected to disable interrupts.
 *
 */
kgettickd(int *timed, int *tickd, int *clock_set)
{
        int ipri;
        cpu_t my_cpu=CPUID;

        ipri = disable_lock(INTMAX, &tod_lock);

        *timed = timedelta[my_cpu].delta;
        *tickd = uSdelta;

	/* true if time adjusted through system call. */
        *clock_set = time_adjusted;

        unlock_enable(ipri, &tod_lock);
        return(0);
}

ksettickd(int *timed, int *tickd, int *clock_set)
{
        int i;
        int ipri;
	int most_advanced;
	int ncpus=NCPUS();

        ipri = disable_lock(INTMAX, &tod_lock);

        if (timed != NULL) {
	        /*
         	 * If all processors have not made the same time adjustments
        	 * then calculate the value needed on a per processor basis
       	  	 * to catch up.  Store this value in the processor's prev_delta
       	  	 * field to be applied in a single increment.
       	  	 */
        	most_advanced = 0;
        	for (i = 0; i < ncpus; i++) {
                	if (timedelta[i].delta < 0)
                        	if ((most_advanced == 0) ||
                            	    (most_advanced < timedelta[i].delta))
                                	most_advanced = timedelta[i].delta;
                	else if (timedelta[i].delta > 0)
                        	if ((most_advanced == 0) ||
                            	    (most_advanced > timedelta[i].delta))
                                	most_advanced = timedelta[i].delta;
        	}
        	for (i = 0; i < ncpus; i++) {
                	timedelta[i].prev_delta += 
					(timedelta[i].delta - most_advanced);
                	timedelta[i].delta = *timed;
        	}
	}

        if (clock_set != NULL)
                time_adjusted = (*clock_set ? TRUE : FALSE);
        else
                time_adjusted = FALSE;

        if (tickd != NULL)
                uSdelta = *tickd;

        unlock_enable(ipri, &tod_lock);

        return(0);
}
