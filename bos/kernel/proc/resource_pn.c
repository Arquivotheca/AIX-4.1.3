static char sccsid[] = "@(#)96  1.2  src/bos/kernel/proc/resource_pn.c, sysproc, bos411, 9428A410j 2/18/94 13:20:25";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: ruadd
 *		xsetrlimit
 *
 *   ORIGINS: 27, 3, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/prio_calc.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/intr.h>
#include <sys/resource.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/lock.h>
#include <sys/pseg.h>
#include <sys/audit.h>
#include <sys/uio.h>
#include <sys/systm.h>
#include <sys/syspest.h>

#define PGBNDUP(x)      ( ((x)+PAGESIZE-1) & ~((long)(PAGESIZE-1)) )

/*
 * NAME: xsetrlimit()
 *	
 * FUNCTION:
 *	Limits on the consumption of system resources by the current
 *	process and each process it creates may be set with the setrlimit()
 *	call.
 *
 * INPUT PARAMETER DESCRIPTION:
 *	The "resource" may be one of the following:
 *
 *	RLIMIT_CPU	The maximum amount of CPU time (in seconds) to be used
 *			by each process.
 *
 *	RLIMIT_FSIZE	The largest size, in bytes, of any single file that can
 *			be created.
 *
 *	RLIMIT_DATA	The maximum size, in bytes, of the data segment for a
 *			process; this defines how far a program may extend its
 *			"break" value with the sbrk() system call.
 *
 *	RLIMIT_STACK	The maximum size, in bytes, of the stack segment for
 *			a process.  This defines how far a program's stack
 *			segment may be extended.  Stack expansion is performed
 *			automatically by the system.
 *
 *	RLIMIT_CORE	The largest size, in bytes, of a "core" file that may
 *			be created.
 *
 *	RLIMIT_RSS	The maximum size, in bytes, to which a process's
 *			resident set size may grow.  This imposes a limit on
 *			the amount of physical memory to be given to a process.
 *			If memory is tight, the system will prefer to take
 *			memory from processes that are exceeding their declared
 *			resident set size.
 *
 * EXECUTION ENVIRONMENT:
 *	BSD System Call.
 *	Can Page Fault.
 *
 * RETURN VALUE DESCRIPTION:
 *	A return value of zero indicates the call succeeded, changing or 
 *	returning the resource limit.  A return value of -1 indicates that 
 *	an error occurred, and an error code is stored in the global 
 *	variable "errno".
 *
 *	internal form used by both setrlimit and ulimit - must do it this 
 *	way because setrlimit does copyin and ulimit can't call it.
 */

xsetrlimit( int resource, struct rlimit *rlp )
/* int resource;		Desired resource to change. */
/* struct rlimit *rlp;		Address in user space to read from. */
{
	struct rlimit alim;
	register struct rlimit *alimp;
	long delta;
	int smax, dsize, olddownlim, newdownlim;
	int ipri;

	ASSERT(lock_mine(&U.U_handy_lock));

	/*
	 * BSD now uses quad_t (unsigned) as the
	 * data type for rlim_cur/rlim_max.
	 */
	alim = *rlp;
	alimp = &U.U_rlimit[ resource ];

	switch (resource) {
		case RLIMIT_FSIZE:
		case RLIMIT_DATA:
		case RLIMIT_STACK:
			/*
			 * If the request for the current size is greater
			 * than the system maximum, set it to the level
			 * of the system maximum.
			 */
			if ((unsigned long)alim.rlim_cur > RLIM_INFINITY)
				alim.rlim_cur = RLIM_INFINITY;
			if ((unsigned long)alim.rlim_max > RLIM_INFINITY)
				alim.rlim_max = RLIM_INFINITY;
			break;
	}

	/*
	 * If the request is to exceed the maximum values set up by the
	 * the system, the caller had better have the privileges to
	 * change it.
	 */

	if ((unsigned long)alim.rlim_cur > (unsigned long)alimp->rlim_max ||
	    (unsigned long)alim.rlim_max > (unsigned long)alimp->rlim_max)

		/*
		 * If the caller doesn't have the necessary privileges,
		 * let him know about it.
		 */

		if ( privcheck(SET_PROC_RAC) == EPERM) {
			u.u_error = EPERM;
			return(-1);
		}


	if ((unsigned long)alim.rlim_cur > (unsigned long)alim.rlim_max)
		alim.rlim_cur = alim.rlim_max;

	switch (resource) {

	case RLIMIT_DATA:
		/* N.B. rlim_cur is same as u.u_dmax!*/

		if (alim.rlim_cur < U.U_dsize && alim.rlim_cur != RLIM_INFINITY)
		{
			u.u_error = EFAULT;
			return (-1);
		}

		break;

	case RLIMIT_STACK:
		/* N.B. rlim_cur is same as u.u_smax!*/

		/*
		 * Don't allow the soft stack limit to be set beyond the
		 * current end of the data.
		 */
	   	dsize = U.U_dsize < SEGSIZE ? U.U_dsize : U.U_sdsize;
		smax = SEGSIZE - K_REGION_SIZE - dsize;

		/* calculate down limits */
		if ((unsigned)alimp->rlim_cur > smax)
			olddownlim = smax;
		else
			olddownlim = alimp->rlim_cur;

		if ((unsigned)alim.rlim_cur > smax)
			newdownlim = smax; 
		else
			newdownlim = alim.rlim_cur; 

		/* It would be nice to check here that the new
		 * stack wasn't smaller than what the process
		 * is using right now - but we don't have that value.
		 * So we do nothing - if he moves the stack down
		 * over himself his process will fault.
		 * We could add a vmm interface to get the current
		 * value.
		 */

		if (U.U_lock & (PROCLOCK | DATLOCK)) /*	check for pinned */
		{
			/* find difference between old and new stack max */
			delta = PGBNDUP(newdownlim)-PGBNDUP(olddownlim);

			/* if stack is shrinking, unpin the difference */
			if (delta < 0){
				 u.u_error =
					unpinu((caddr_t)PRIVORG+U_REGION_SIZE-
					(long)(PGBNDUP(olddownlim)),
					-delta, (short)UIO_USERSPACE);
				if (u.u_error) {
					return(-1);
				}
			}
		}
		(void) vms_limits(SRTOSID(u.u_procp->p_adspace), dsize,
			   newdownlim + K_REGION_SIZE);

		if (U.U_lock & (PROCLOCK | DATLOCK)) /*	check for pinned */
		{
			/* if stack is growing, pin the difference */
			if (delta > 0) {

				u.u_error = pin(PRIVORG + U_REGION_SIZE -
						PGBNDUP(newdownlim), delta);
				if(u.u_error)
				{
					vms_limits(
						SRTOSID(u.u_procp->p_adspace),
						dsize,
			   		 	olddownlim + K_REGION_SIZE
						);
					return(-1);
				}
			}
		}
		break;

	case RLIMIT_FSIZE:
	case RLIMIT_RSS:
		break;
	}

	ipri = disable_lock(INTTIMER, &U.U_timer_lock);

	*alimp = alim;

	unlock_enable(ipri, &U.U_timer_lock);

	return(0);
}

/*
 * NAME: ruadd()
 *
 * FUNCTION:
 *      Add to resource usage structures, ru1 = ru1 + ru2;
 *
 * EXECUTION ENVIRONMENT:
 *      This routine may only be called by a kernel process.
 *      This routine may page fault.
 *
 * NOTE:
 *      This routine is DEPENDENT on the RUSAGE structure
 *
 * RETURN VALUE DESCRIPTION:
 *      There is NO return value from this routine.
 */

void
ruadd( struct rusage *ru, struct rusage *ru2)
/* struct rusage *ru;           Rusage structure no. 1 */
/* struct rusage *ru2;          Rusage structure no. 2 */
{
        register long *ip, *ip2;
        register int i;

        /*
         * Atomic primtives are not required because this routine
         * is called when the process is exiting and there is only
         * one thread active.
         */
        timevaladd(&ru->ru_utime, &ru2->ru_utime);
        timevaladd(&ru->ru_stime, &ru2->ru_stime);
        if (ru->ru_maxrss < ru2->ru_maxrss)
                ru->ru_maxrss = ru2->ru_maxrss;
        ip = &ru->ru_first; ip2 = &ru2->ru_first;
        for (i = &ru->ru_last - &ru->ru_first; i >= 0; i--)
                *ip++ += *ip2++;
}


