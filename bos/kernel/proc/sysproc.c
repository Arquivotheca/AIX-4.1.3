static char sccsid[] = "@(#)36	1.31.1.13  src/bos/kernel/proc/sysproc.c, sysproc, bos41J, 9512A_all 3/20/95 19:25:40";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: child_adj
 *              getexcept
 *		getpgrp
 *		getppid
 *		getppidx
 *		getuerror
 *		kgetpgrp
 *		new_uidl
 *		setpgrp
 *		setuerror
 *
 *   ORIGINS: 3, 26, 27, 83
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
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/syspest.h>
#include <sys/lockl.h>
#include <sys/intr.h>
#include <sys/trchkid.h>
#include <sys/mstsave.h>
#include <sys/sleep.h>
#include <sys/except.h>
#include <sys/low.h>
#include <sys/var.h>
#include <sys/sysconfig.h>
#include <sys/id.h>

struct cfgncb cb;


/*
 * NAME: getppid, getppidx, getpgrp, kgetpgrp
 *
 * FUNCTION:
 *	getppid() returns the parent process ID of the calling process
 *	getppidx() returns the parent process ID of the specified process
 *	getpgrp() returns the process group ID of the calling process
 *	kgetpgrp() returns the process group ID of the specified process
 *
 * EXECUTION ENVIRONMENT:
 *
 *	These routines are preemptable.
 *
 * NOTE:
 *	kgetpgrp is a non-documented kernel service.
 *
 * RETURNS:
 *	successful: returns process ID requested
 *	failure:    not a process
 */
pid_t
getppid(void)
{
	return(curproc->p_ppid);
}

pid_t
getppidx(pid_t pid)
{
	struct proc *p;
	int error=FALSE;

	if(pid == 0)
		return(curproc->p_ppid);

	p = VALIDATE_PID(pid);
	if (!p || p->p_stat == SNONE || p->p_stat == SIDL || p->p_stat == SZOMB)
		error = TRUE;

	return(error ? (pid_t) -1 : p->p_ppid );
}

pid_t
getpgrp(void)
{
        return(curproc->p_pgrp);
}

pid_t
kgetpgrp(pid_t pid)
{
        struct proc *p;
        int pgrp;

        if(pid == 0)
                return(curproc->p_pgrp);

        if ( !(p = VALIDATE_PID(pid)) || p->p_stat == SNONE ||
            p->p_stat == SZOMB){
                pgrp = -1;
        }
        else {
                pgrp = p->p_pgrp;
        }

        return(pgrp);
}


/*
 * NAME: setpgrp()
 *
 * FUNCTION:
 *      Become leader of a new process group, if not already leader
 *
 * RETURN VALUES:
 *      Process group id of caller (always successful)
 *
 * NOTE: The BSD version of setpgrp is in libbsd with arguments pid and pgrp.
 */

pid_t
setpgrp()
{
        struct proc *p = curproc;
        TRCHKT_SYSC(SETPGRP);

        if(p->p_pid != p->p_pgrp)       /* If not already group leader */
                return(setsid());     /* set pgrp and release controlling tty */

        return(p->p_pgrp);              /* If already group leader */
}


/*
 * NAME: getuerror()
 *
 * FUNCTION: return current value of u.u_error
 *
 * RETURN VALUES:
 *	return current value of u.u_error
 *
 */
getuerror()
{
	return(u.u_error);
}

/*
 * NAME: setuerror()
 *
 * FUNCTION: sets current value of u.u_error
 *
 */
setuerror(int errno)
{
	return(u.u_error = errno);
}

/*
 * NAME: getexcept()
 *
 * FUNCTION: returns exception structure
 *
 */
void
getexcept(struct except *exceptp)
{
        bcopy(csa->except, exceptp, sizeof(struct except));
}


/*
 * NAME: child_adj 
 *
 * FUNCTION: 
 *	This is a process management config notification routine.  
 *	It validates proposed changes to the v structure by
 *      making sure that the maximum processes per uid is not
 *	set lower than _POSIX_CHILD_MAX.
 *
 * EXECUTION ENVIRONMENT:
 * 	preemptable
 *	may page fault
 *	This routine is registered on the cfgncb list by strtdisp(), and is
 *	executed by sysconfig when the SYS_SETPARMS argument is used. 
 *
 * RETURNS:
 *	0  -  The change to the v structure was validated
 *	byte offset of v_maxup field - The proposed change is too small
 *
 * NOTE: this routine is called only by sysconfig 
 */
int
child_adj( int cmd, struct var *cur, struct var *new)
{

	/* Serialization is contained in sysconfig */

	switch (cmd)
	{
	case CFGV_PREPARE:
	
		if ( (new->v_maxup < _POSIX_CHILD_MAX) ||
		     (new->v_maxup > NPROC) ) 
		{
			return((int)(&(new->v_maxup)) - (int)(new));
		}
		else
		{
			return(0);
		}
	case CFGV_COMMIT:
		return(0);

	default:
		panic("child_adj: invalid command.");
	}
}

/*
 *
 * NOTE: this routine is called only by setuidx 
 */
int
new_uidl( struct proc *pr, uid_t newuid, int mask)
{
	register struct proc *p; 	/* place holder in new uid list */
	register struct proc *q;	/* used to search uid lists */
	register int uid_cnt;		/* number of processes in new uid list*/

	/* grab process table lock */
	simple_lock(&proc_tbl_lock);

	/* find a process in the list we want to add to */
	p = proc;
	while ((p < max_proc) && (p->p_uid != newuid ||
				  p->p_stat == SNONE || p->p_stat == SZOMB))
		p++;

	/* if a list exists for the new uid, make sure there is room for us */
	if (p < max_proc) {
		for (q = p->p_uidl, uid_cnt = 1;
		    p != q;
		    q = q->p_uidl, uid_cnt++);

		/* if there isn't room and we don't have special privilege, 
		   return an error */
		if ((uid_cnt > (v.v_maxup - 1)) && _privcheck(SET_PROC_RAC)) {
			u.u_error = EAGAIN;
			simple_unlock(&proc_tbl_lock);
			return(-1);
		}
	}

	/* remove our process from his old list */
	for ( q = pr; 
	    pr != q->p_uidl;
	    q = q->p_uidl);
	q->p_uidl = pr->p_uidl;

	/* if we found a list for the new uid, add our process to it */
	if (p < max_proc) {
		q = p->p_uidl;
		pr->p_uidl = q;
		p->p_uidl = pr;
	}

	/* otherwise, start our own list */
	else 
		pr->p_uidl = pr;

	if (mask & ID_REAL)
	{
		pr->p_uid = newuid;
	}
	if (mask & ID_SAVED)
	{
		pr->p_suid = newuid;
	}

	/* let go of process table lock and exit */
	simple_unlock(&proc_tbl_lock);
	return(0);
}
