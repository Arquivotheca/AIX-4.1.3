static char sccsid[] = "@(#)57	1.7  src/bos/kernel/proc/encap.c, sysproc, bos411, 9428A410j 6/23/94 15:15:10";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: getctty
 *		kgetsid
 *		is_blocked
 *		is_caught
 *		is_ignored
 *		is_orphan
 *		is_pgrp
 *		is_pgrpl
 *		is_sessl
 *		rusage_incr
 *		setctty
 *
 *   ORIGINS: 27, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/lock_def.h>
#include <sys/encap.h>

/*
 * NAME: is_blocked
 *
 * FUNCTION:
 *	Check for blocked signal in the current process.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	If 'signo' is blocked in the current process,
 *	return TRUE; else return FALSE.
 */
boolean_t
is_blocked( int signo )
{
	register boolean_t rc;

	/* Don't need serialization, since data is not referenced 
	 * under lock by the caller.
         */
	/*
	 * if signo is valid and is blocked 
	 * by the current process, return TRUE.
	 */
	if ((signo > 0) && (signo <= SIGMAX) &&
				SIGISMEMBER(curthread->t_sigmask, signo))
		rc = TRUE;
	else
		rc = FALSE;

	return(rc);
}

/*
 * NAME: is_caught
 *
 * FUNCTION:
 *      Check for caught signal in the current process.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *      If 'signo' is blocked in the current process,
 *      return TRUE; else return FALSE.
 */
boolean_t
is_caught( int signo )
{
        register boolean_t rc;

	/* Don't need serialization, since data is not referenced 
         * under lock by the caller.
         */
        /*
         * if signo is valid and is blocked
         * by the current process, return TRUE.
         */
        if ((signo > 0) && (signo <= SIGMAX) &&
        			SIGISMEMBER(curproc->p_sigcatch, signo))
                rc = TRUE;
        else
                rc = FALSE;

        return(rc);
}

/*
 * NAME: is_ignored
 *
 * FUNCTION:
 *	Check for ignored signal in the current process.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	If 'signo' is ignored in the current process,
 *	return TRUE; else return FALSE.
 */
boolean_t
is_ignored( int signo )
{
	register boolean_t rc;

	/* Don't need serialization, since data is not referenced 
         * under lock by the caller. 
         */ 
	/*
	 * if signo is valid and is being ignored
	 * by the current process, return TRUE.
	 */
	if ((signo > 0) && (signo <= SIGMAX) &&
				SIGISMEMBER(curproc->p_sigignore, signo))
		rc = TRUE;
	else
		rc = FALSE;

	return(rc);
}

/*
 * NAME: is_pgrp
 *
 * FUNCTION:
 *	Check to see if 'pgrp' is a valid process group.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	If 'pgrp' is a valid process group 
 *	return TRUE; else return FALSE.
 */
boolean_t
is_pgrp( pid_t pgrp )
{
        register struct proc *p;
        register int rc;

	/* Don't need serialization, since data is not referenced 
         * under lock by the caller.
         */

	p = PROCPTR(pgrp);
	if (!p ||
	     p->p_pid != pgrp ||
	     !p->p_ganchor)
	   return FALSE;

	return TRUE;
}

/*
 * NAME: is_pgrpl
 *
 * FUNCTION:
 *	Check to see if 'pid' is a process group leader.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	If 'pid' is a valid process and a process group 
 *	leader, return TRUE; else return FALSE.
 */
boolean_t
is_pgrpl( pid_t pid )
{
        register struct proc *p;
        register int rc;

	/* Don't need serialization, since data is not referenced 
         * under lock by the caller. */

        if (pid == 0) {
		p = curproc;
	}
        else {
                p = VALIDATE_PID(pid);
	}

        if (!p || p->p_stat == SNONE || p->p_stat == SIDL || p->p_stat == SZOMB)
                rc = FALSE;
        else
		rc = (p->p_pid == p->p_pgrp) ? TRUE : FALSE;

        return (rc);
}


/*
 * NAME: is_sessl
 *
 * FUNCTION:
 *	Check to see if 'pid' is a session leader.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	If 'pid' is a valid process and a session
 *	leader, return TRUE; else return FALSE.
 */
boolean_t
is_sessl( pid_t pid )
{
        register struct proc *p;
        register int rc;

	/* Don't need serialization, since data is not referenced 
         * under lock by the caller. */

        if (pid == 0) {
                p = curproc;
        }
        else {
                p = VALIDATE_PID(pid);
        }

        if (!p || p->p_stat == SNONE || p->p_stat == SIDL || p->p_stat == SZOMB)
                rc = FALSE;
        else 
                rc = (p->p_pid == p->p_sid) ? TRUE : FALSE;

        return (rc);
}


/*
 * NAME: is_orphan
 *
 * FUNCTION:
 *	Check to see if 'pid' is a member of an
 *	orphaned process group.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	If 'pid' is not a valid process or 'pid' is not
 *	a member of an orphaned prgp, return FALSE.
 *	Else, return TRUE.
 */
int
is_orphan( pid_t pid )
{
        register struct proc *p;
        register int rc;

	/* Don't need serialization, since data is not referenced 
         * under lock by the caller.
         */

        if (pid == 0) {
                p = curproc;
        }
        else {
                p = VALIDATE_PID(pid);
        }

        if (!p || p->p_stat == SNONE || p->p_stat == SIDL || p->p_stat == SZOMB)
                rc = FALSE;
        else
		rc = (p->p_flag & SORPHANPGRP) ? TRUE : FALSE;

        return (rc);
}


/*
 * NAME: kgetsid
 *
 * FUNCTION:
 *	Return session ID.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * RETURNS:
 *	If 'pid' is 0, then return session ID of current
 *	process; else return session ID of 'pid'.
 *
 *	If 'pid' is not valid, return -1.
 */
pid_t
kgetsid( pid_t pid )
{
	register struct proc *p;
	int error=FALSE;	

	/* Don't need serialization, since data is not referenced 
         * under lock by the caller.
         */

	if (pid == 0)
		p = curproc;
	else {
		p = VALIDATE_PID(pid);

		if (!p) {
			error = TRUE;
		}
	}
		
	if (error)
		return((pid_t) -1);

	return (p->p_sid);
}

/*
 * NAME: getctty
 *
 * FUNCTION:
 *	Returns fields in the ublock
 *	relating to TTY.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * NOTE:
 *
 * RETURNS:
 *	Fields in ublock relating to the TTY
 *	are copied into buf.
 */
void
getctty( struct ttyinfo *buf )
{
	/* Don't need serialization, since data is not referenced 
         * under lock by the caller.
         */

	buf->ti_ttysid = U.U_ttysid;
	buf->ti_ttyp   = U.U_ttyp;
	buf->ti_ttyd   = U.U_ttyd;
	buf->ti_ttympx = U.U_ttympx;
	buf->ti_ttys   = U.U_ttys;
	buf->ti_ttyid  = U.U_ttyid;
	buf->ti_ttyf   = U.U_ttyf;
}


/*
 * NAME: setctty
 *
 * FUNCTION:
 *	Sets fields in the ublock
 *	relating to TTY.
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * NOTE:
 *
 * RETURNS:
 *	None.
 */
void
setctty( struct ttyinfo *buf )
{
	/* Don't need serialization, since data is not referenced 
         * under lock by the caller
         */

	U.U_ttysid = buf->ti_ttysid;
	U.U_ttyp   = buf->ti_ttyp;
	U.U_ttyd   = buf->ti_ttyd;
	U.U_ttympx = buf->ti_ttympx;
	U.U_ttys   = buf->ti_ttys;
	U.U_ttyid  = buf->ti_ttyid;
	U.U_ttyf   = buf->ti_ttyf;
}

/*
 * NAME: rusage_incr
 *
 * FUNCTION:
 *	This function increments the 'field' in the current
 *	process' rusage structure by 'amount'.  The
 *	parameter 'field' should be set to one of:
 *		RUSAGE_MSGSENT
 *		RUSAGE_MSGRCV
 *		RUSAGE_INBLOCK
 *		RUSAGE_OUTBLOCK
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure can only be called by a process.
 *
 * NOTE:
 *
 * RETURNS:
 *	None.
 */
void
rusage_incr( int field, int amount )
{
	switch ( field )
	{
		case RUSAGE_MSGSENT:
			fetch_and_add(&(U.U_ru.ru_msgsnd), amount);
			break;
		case RUSAGE_MSGRCV:
			fetch_and_add(&(U.U_ru.ru_msgrcv), amount);
			break;
		case RUSAGE_INBLOCK:
			fetch_and_add(&(U.U_ior), amount);
			break;
		case RUSAGE_OUTBLOCK:
			fetch_and_add(&(U.U_iow), amount);
			break;
	}
}
