static char sccsid[] = "@(#)55  1.38.1.21  src/bos/kernel/proc/resource.c, sysproc, bos411, 9428A410j 6/10/94 06:37:46";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: PGBNDUP
 *		get_pgrp_nice
 *		getpriority
 *		getrlimit
 *		getrusage
 *		set_pgrp_nice
 *		setpriority
 *		setrlimit
 *
 *   ORIGINS: 27, 3, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
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
#include <sys/cred.h>
#include <sys/lock.h>
#include <sys/pseg.h>
#include <sys/audit.h>
#include <sys/uio.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include "ld_data.h"

#define NANOTOMICRO 1000

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern int copyin();
extern int copyout();
extern timevaladd();
extern donice();

typedef int nice_t;
typedef char error_t;

#define	PGBNDUP(x)	( ((x)+PAGESIZE-1) & ~((long)(PAGESIZE-1)) )

#define OK 0

/*
 * NAME: getpriority()
 *
 * FUNCTION:
 *	The scheduling "nice" value of the process, process group, or
 *	user, as indicated by "which" and "who".  "Which" is one of
 *	PRIO_PROCESS, PRIO_PGRP, or PRIO_USER, and "who" is interpreted
 *	relative to "which" (a process indentifier, process group indentifier,
 *	and a user ID, respectively).  A zero value of "who" denotes the
 *	current process, process group, or user.
 *
 * GLOBAL DATA STRUCTURES USED:
 *	Process table. (proc[0] though proc[n])
 *	Process table lock (proc_lock)
 *
 * EXECUTION ENVIRONMENT:
 *	BSD System Call.
 *	Can page fault.
 *
 * RETURN VALUE DESCRIPTION:
 *	-1 - if an error was detected; in which case u_error
 *           has been set to EINVAL or ESRCH.
 *
 *                     - OR -
 *
 *           the lowest 'nice' value (i.e. most favored) of all the
 *           specified process. The nice value returned will be in
 *           range PRIO_MIN <= nice <= PRIO_MAX (i.e. berkley's
 *           range for 'nice').
 *
 * NOTES:
 *      Since Berkley's 'nice' value range is returned, a -1
 *      can be legitimately returned. If -1 does indicate an
 *      error, u_error has been set.
 */

getpriority( int which, int who )
/*
 *        which : identifies how the "who" parameter is handled.
 *        who   : Process, process group, or UID.
 */
{

	register struct proc *p;
	int ret_value = P_NICE_MAX + 1;
	
	TRCHKGT_SYSC(GETPRIORITY,which,who,NULL,NULL,NULL);

	simple_lock(&proc_tbl_lock);

	switch (which) {

	case PRIO_PROCESS:
		
		/*
		 * If the "who" parameter was given to us as a zero, then
		 * we must assume it is the currently running process that
		 * is to be used.  If the "who" parameter was non-zero, we
		 * need to try to make sure it is a real process.
		 */

		if (who == 0)
			p = curproc;
		else
			/*
			 * Be sure that the "who" parameter is a
			 * pid.
			 */

			if ( !(p = VALIDATE_PID(who)) || p->p_stat == SNONE
						      || p->p_stat == SIDL
						      || p->p_stat == SZOMB ) {
				break;
			}

		ret_value = EXTRACT_NICE(p);

                break;

	case PRIO_PGRP:

		/*
		 * If the "who" parameter was given to us as a zero, then
		 * we must assume it is the currently running process's
		 * group is to be used.
		 */

		if (who == 0) {
			p = curproc;
		}
		else
			/*
			 * Be sure that the "who" parameter is a
			 * pid.  And that the group anchor is valid.
			 * The group leader could have exited and in
			 * the zombie state, but as long as the group
			 * anchor is valid, there are other processes
			 * in the group.
			 */

			if ( !((p = VALIDATE_PID(who)) && (p = p->p_ganchor)
				&& (p->p_pgrp == who)))
				break;

		ret_value = get_pgrp_nice( p );

                break;

	case PRIO_USER:

		/*
		 * If the "who" parameter was given to us as a zero, then
		 * we must assume it is the currently running process's
		 * uid is to be used.
		 */

		if (who == 0)
			who = curproc -> p_uid;


		for (p = &proc[0]; p < max_proc; p++)  {
			/*
			 * need to skip over 'inactive' process slots
			 */
			if (  p->p_stat != SNONE  &&
			      p->p_stat != SIDL   &&
		              p->p_stat != SZOMB  &&
			      p->p_uid == who  )  {
				    ret_value = MIN(ret_value,EXTRACT_NICE(p));
                        }
                }


                break;

	default:
		u.u_error = EINVAL;
	}

	simple_unlock(&proc_tbl_lock);

	if (u.u_error != 0)
		return(-1);

	/*
	 * If low hasn't been changed from the time we came into
	 * this routine or only processes with fixed priorities
	 * were found, then ret_value is equal to P_NICE_MAX + 1.
	 * In either case the return value should be converted to -1
	 * and u_error should be set to ESRCH.
	 * Otherwise, we need to adjust the 'nice' value to
	 * Berkley's range.
	 */

	if (ret_value == P_NICE_MAX + 1) {
		u.u_error = ESRCH;
		return(-1);
	}


	/*
	 * Return what we did find, adjusting to Berkley's nice value
	 * range.
	 */

	return( ret_value - 20 );
}

/*
 * NAME: setpriority()
 *
 * FUNCTION:
 *	The scheduling "nice" value of the process, process group, or
 *	user, as indicated by "which" and "who".  "Which" is one of
 *	PRIO_PROCESS, PRIO_PGRP, or PRIO_USER, and "who" is interpreted
 *	relative to "which" (a process indentifier, process group indentifier,
 *	and a user ID, respectively).  A zero value of "who" denotes the
 *	current process, process group, or user.  The "nice" parameter is
 *	a value in the range -20 to 20. Lower priorities cause more favorable
 *      scheduling.
 *
 *	The setpriority() call sets the "nice" values of all the
 *	specified processes to the specified value.  If the specified value
 *	is less than -20, a value of -20 is used.  If it is greater than
 *	20, a value of 20 is used.  Only processes that have SET_PROC &
 *	US1.PRIORITY system privlege may lower "nice" values.
 *
 * GLOBAL DATA STRUCTURES USED:
 *
 * EXECUTION ENVIRONMENT:
 *	BSD System Call.
 *	Can Page Fault.
 *
 * RETURN VALUE DESCRIPTION:
 * 	0 - all the specified process have their nice value
 *          reset and their priorities recalculated.
 *     -1 - some/all of the specified processes nice values
 *          could not be reset (u_error has been set).
 */
int
setpriority( int which, int who, nice_t nice )
/*
 *  which  :  This identifies how the "who" param is handled.
 *  who    :  Process, process group, or UID.
 *  nice   :  New "nice" value request.
 */
{
	register struct proc *p;
        error_t save_u_error;
	int ret_value = 0;
        static int svcnum = 0;
	struct ucred *crp;

        if(audit_flag && audit_svcstart("PROC_SetPri", &svcnum, 1, nice)){
                audit_svcfinis();
        }
	
	TRCHKGT_SYSC(SETPRIORITY,which,who,nice,NULL,NULL);

	/*
	 * make any range adjustments and 'deBERKLEYize' the nice value
	 */
        if ( nice > PRIO_MAX )
	       nice = PRIO_MAX;

        if ( nice < PRIO_MIN )
	       nice = PRIO_MIN;

        nice = nice + 20;

	crp = crref();
	simple_lock(&proc_tbl_lock);

	switch (which) {

	case PRIO_PROCESS:

		/*
		 * If the "who" parameter was given to us as a zero, then
		 * we must assume it is the currently running process that
		 * is to be used.  If the "who" parameter was non-zero, we
		 * need to try to make sure it is a real process.
		 */

		if (who == 0)
			p = curproc;
		else
			/*
			 * Be sure that the "who" parameter is a
			 * pid.
			 */

			if ( !(p = VALIDATE_PID(who)) || p->p_stat == SNONE
						      || p->p_stat == SIDL
						      || p->p_stat == SZOMB ) {
				u.u_error = ESRCH;
				ret_value = -1;
				break;
			}


		ret_value =  donice(p, nice, crp);

                break;


	case PRIO_PGRP:

		/*
		 * If the "who" parameter was given to us as a zero, then
		 * we must assume it is the currently running process's
		 * process group that is to be used.  If the "who"
		 * parameter was non-zero, we need to try to make sure it
		 * is a real process.
		 */

		if (who == 0)
			p = curproc;
		else
			/*
			 * Be sure that the "who" parameter is a
			 * pid.  And that the group anchor is valid.
			 * The group leader could have exited and in
			 * the zombie state, but as long as the group
			 * anchor is valid, there are other processes
			 * in the group.
			 */

			if ( !((p = VALIDATE_PID(who)) && (p = p->p_ganchor)
				&& (p->p_pgrp == who))) {
				u.u_error = ESRCH;
				ret_value = -1;
				break;
			}


		ret_value =  set_pgrp_nice(p, nice, crp);

                break;

	case PRIO_USER:

		/*
		 * If the "who" parameter was given to us as a zero, then
		 * we must assume it is the currently running process's
		 * uid is to be used.
	         * NOTE: if all the processes with the same uid have fixed
		 *       priorities then return ERSCH. Since the function
		 *       'donice' sets u_error if a process is inactive or
		 *       has a fixed priority to ERSCH, we need to
		 *       save/restore u_error under this condition.
		 *       u_error should be set to ERSRCH only if
	         *       all the processes with the same uid have fixed
	         *       priorities or are inactive.
		 */

		if (who == 0)
			who = curproc -> p_uid;

                ret_value = 0;             /* assume we won't find a match  */
		for (p = &proc[0]; p < max_proc; p++)  {

			/*
			 * If the UID matches, then try to change
			 * the "nice" value to the desired value.
			 */

			if (p->p_uid == who) {
				 save_u_error = u.u_error;
				 (void) donice(p, nice, crp);
				 if (u.u_error == ESRCH)
				       u.u_error = save_u_error;
                                 else
				       ret_value++; /* indicate that a
					               process was found */
			}
                }

                if (ret_value == 0)      /* didn't find process  */
		      u.u_error = ESRCH;
                if (u.u_error != 0)
		      ret_value = -1;
		else
		      ret_value = 0;
		break;

	default:

		u.u_error = EINVAL;
		ret_value = -1;

	}

	simple_unlock(&proc_tbl_lock);
	crfree(crp);
			
	return(ret_value);

}

/*	does system call stuff then calls xsetrlimit above
 *	can page fault.
 */
setrlimit( int resource, struct rlimit *rlp )
/* int resource;		Desired resource to change. */
/* struct rlimit *rlp;		Address in user space to read from. */
{
	struct rlimit alim;
	static int svcnum = 0;
	int rc;

	if(audit_flag && audit_svcstart("PROC_Limits", &svcnum, 0)){
		audit_svcfinis();
	}

	TRCHKLT_SYSC(SETRLIMIT,resource);

	/*
	 * If the caller makes an unresonable request,
	 * slam the door in his face.
	 */

	if (resource >= RLIM_NLIMITS || resource < RLIMIT_CPU) {
		u.u_error = EINVAL;
		return(-1);
	}


	/*
	 * If we can't read what the caller has, let him know it.
	 */

	if (copyin((caddr_t)rlp,(caddr_t)&alim,sizeof(struct rlimit)) != OK){
		u.u_error = EFAULT;
		return(-1);
	}

	simple_lock(&U.U_handy_lock);

	rc = xsetrlimit(resource,&alim);

	simple_unlock(&U.U_handy_lock);

	return rc;
}

/*
 * NAME: getrlimit()
 *	
 * FUNCTION:
 *	Limits on the consumption of system resources by the current
 *	process and each process it creates may be retrieved with the
 *	getrlimit() call.
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
 *	can Page Fault.
 *
 * RETURN VALUE DESCRIPTION:
 *	A return value of zero indicates the call succeeded, changing or 
 *	returning the resource limit.  A return value of -1 indicates that 
 *	an error occurred, and an error code is stored in the global variable 
 *	"errno".
 */

getrlimit( int resource, struct rlimit *rlp )
/* int	resource;	The resource requested. */
/* struct rlimit *rlp;	Address of rlimit struct in user space. */
{
	struct rlimit tmplim;

	TRCHKLT_SYSC(GETRLIMIT,resource);

	/*
	 * If the resource parameter is greater than that specified
	 * within the system, return an error.
	 */

	if (resource >= RLIM_NLIMITS || resource < RLIMIT_CPU) {
		u.u_error = EINVAL;
		return(-1);
	}

	/*
	 * If the caller has a bogus address for rlp, give him
	 * an error return.  No locking is required since the
	 * the data is not referenced by the caller under a lock.
	 */

	if (copyout((caddr_t)&U.U_rlimit[ resource ], (caddr_t)rlp,
	    	sizeof (struct rlimit)) != OK) {
		u.u_error = EFAULT;
		return(-1);
	}

	return(0);
}

/*
 * NAME: getrusage()
 *
 * FUNCTION:
 *	The getrusage() call returns information describing the resources 
 *	utilized by the current process, or all its terminated child 
 *	processes.  The "who" parameter is one of RUSAGE_SELF or 
 *	RUSAGE_CHILDREN.  The buffer to which "rusage" points will be filled 
 * 	in as described in sys/resource.h.
 *
 * EXECUTION ENVIRONMENT:
 *	BSD System Call.
 *	Can Page Fault.
 *
 * RETURN VALUE DESCRIPTION:
 *	A zero return value indicates that the call succeeded.	A return 
 *	value of -1 indicates that an error occurred, and an error code is 
 *	stored in the global variable "errno".
 */

getrusage( int who, struct rusage *rusage)
/* who: Your process or it's children. */
/* rusage: Place in user space to write it. */
{
	struct rusage rup;
	long		ixrss;		/* shared memory size		*/
	long		idrss;		/* unshared data size		*/

	TRCHKLT_SYSC(GETRUSAGE,who);

	switch (who) {

	case RUSAGE_SELF:

            	U.U_ru.ru_isrss = 0;		
            	U.U_ru.ru_nswap = 0;	
            	U.U_ru.ru_minflt = curproc->p_minflt;
        	U.U_ru.ru_majflt = curproc->p_majflt;
		idrss = 4 * vms_rusage(U.U_adspace.srval[PRIVSEG]);
		idrss += 4 * bd_rusage(&U);
		ixrss = 4 * vms_rusage(U.U_adspace.srval[TEXTSEG]);
		if (U.U_adspace.alloc & ((unsigned)0x80000000 >> SHDATASEG))
			idrss += 4 * vms_rusage(U.U_adspace.srval[SHDATASEG]);
		if (OVFL_EXISTS((struct loader_anchor *)U.U_loader))
			idrss += 4 * vms_rusage(((struct loader_anchor *)
				(U.U_loader))->la_ovfl_srval);

 		/* Only update if no error from vms_rusage */
		if (idrss >= 0 && ixrss >= 0)
                        while ((idrss + ixrss) > U.U_ru.ru_maxrss &&
                                !compare_and_swap(&U.U_ru.ru_maxrss,
                                	&U.U_ru.ru_maxrss, idrss + ixrss));
		rup = U.U_ru;
		rup.ru_utime.tv_usec = rup.ru_utime.tv_usec / NANOTOMICRO;
		rup.ru_stime.tv_usec = rup.ru_stime.tv_usec / NANOTOMICRO;

		break;

	case RUSAGE_CHILDREN:

        	U.U_cru.ru_isrss = 0;			/* Not supported */
        	U.U_cru.ru_nswap = 0;			/* Not supported */
		rup = U.U_cru;
		rup.ru_utime.tv_usec = rup.ru_utime.tv_usec / NANOTOMICRO;
		rup.ru_stime.tv_usec = rup.ru_stime.tv_usec / NANOTOMICRO;

		break;

	default:

		u.u_error = EINVAL;
		return(-1);

	}

	/*
	 * If the caller has a bogus address for rusage, give him
	 * an error return.
	 */

	if (copyout((caddr_t)&rup, (caddr_t)rusage,
		sizeof (struct rusage)) != OK) {
		u.u_error = EFAULT;
		return(-1);
	}
	return(0);
}

/*
 * NAME: get_pgrp_nice()
 *
 * FUNCTION:
 *	This routine finds the lowest "nice" value within a process group.
 *
 * GLOBAL DATA STRUCTURES USED:
 *	Process table. (proc[0] though proc[n])
 *
 * EXECUTION ENVIRONMENT:
 *	Cannot page fault, since it disables interrupts.
 *
 * RETURN VALUE DESCRIPTION:
 *	Returns the lowest nice value found.
 *                  - OR -
 *      P_NICE_MAX + 1 if a process could not be found, or
 *      the process(es) had a fixed priority.
 *
 * NOTES:
 *      The nice value returned will be in the range
 *      P_NICE_MIN <= nice <= P_NICE_MAX.
 */
nice_t
get_pgrp_nice( struct proc *p	/* process structure */ )
{
	nice_t low;                   	/* lowest nice value so far     */
	int ipri;			/* saved interrupt priority 	*/

	ASSERT(lock_mine(&proc_tbl_lock));	/* read only reference */

	/*
	 * If we don't belong to a process group, then we are
	 * by ourselves and we return our own "nice" value.
	 */
	if (p->p_pgrp == 0)
	     return(EXTRACT_NICE(p));

	/*
	 * determine lowest nice value of process group by extracting
	 * nice value of processes in the process group list.
	 */
	p = PROCPTR(p->p_pgrp);
	p = p->p_ganchor;		/* begin with a process in the pgrp */
	low = EXTRACT_NICE(p);		/* extract nice value               */

	for (p = p->p_pgrpl; p; p = p->p_pgrpl)  {
	    low = MIN(low,EXTRACT_NICE(p));	 /* save minimum nice value */
	}

	return(low);
}

/*
 * NAME: set_pgrp_nice()
 *
 * FUNCTION:
 *	This routine sets the "nice" value for a given process group.
 *
 * GLOBAL DATA STRUCTURES USED:
 *	Process structure.
 *
 * EXECUTION ENVIRONMENT:
 *	Cannot page fault, interrupts are disabled.
 *
 * RETURN VALUE DESCRIPTION:
 *	-1 - some or all of the process's nice value within
 *           the group could not be reset. u_error has been
 *           set to the appropriate value.
 *       0 - all the process's nice value have been reset
 *           and their priorities have been recalculated.
 *
 */

set_pgrp_nice( struct proc *p, nice_t nice, struct ucred *crp)
/*
 *       p    : pointer to an 'active' process
 *       nice : new nice value
 */
{
	error_t	save_u_error;         	/* save u_error         	*/
	int	nice_value_set=FALSE;   /* niced a process		*/
	int	ipri;			/* save interrupt level		*/

	ASSERT(lock_mine(&proc_tbl_lock));	/* read only references */

	/*
	 * If we don't belong to a process group, then we are
	 * by ourselves and we just reset our own nice value
	 */
	if (p->p_pgrp == 0) {
		return (donice(p, nice, crp));
	}

	/*
	 * This loop attempts to set everyone within this process group
	 * to the new "nice" value.
	 * NOTE: if all the processes in the the group have fixed priorities
	 *       then return ERSCH. Since the function 'donice' sets
	 *       u_error if a process is inactive or has a fixed priority
	 *       to ERSCH, we need to save/restore u_error under this
	 *       condition. u_error should be set to ERSRCH only if
	 *       all the processes in the group are inactive or have
	 *       fixed priorities.
	 */
	p = PROCPTR(p->p_pgrp);
	p = p->p_ganchor;		/* begin with a process in the pgrp */

	/* loop on process in p->p_pgrp on linked p_pgrpl list */
	for (; p; p = p->p_pgrpl) {
	    save_u_error = u.u_error;
	    (void) donice(p, nice, crp);
	    if (u.u_error == ESRCH) {
		/* process either had a fixed priority or was inactive */
		u.u_error = save_u_error; /* restore prior error code  */
	    }
	    else
		nice_value_set = TRUE;	/* it was a valid process to nice */
	}

	if (nice_value_set == FALSE)	/* never found a valid process */
	    u.u_error = ESRCH;

	return (u.u_error ? -1 : 0);
}
