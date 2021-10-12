static char sccsid[] = "@(#)55	1.16  src/bos/kernel/proc/sysproc_pn.c, sysproc, bos41J, 9524C_all 6/13/95 11:00:12";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: getpid
 *		orphan_child_pgrp
 *		orphan_pgrp
 *		resign_pgrp
 *		set_orphan_pgrp
 *		setpgid
 *		setsid
 *		setsid2
 *		thread_self
 *		update_proc_slot
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
#include <sys/atomic_op.h>

void   update_proc_slot( struct proc * );
void   orphan_pgrp();			/* may orphan process's pgrp	*/
void   orphan_child_pgrp();		/* may orphan child's pgrp   	*/

static void resign_pgrp( struct proc *p );
static void   set_orphan_pgrp( pid_t pgrp );	/* mark orphaned pgrp	*/


/*
 * NAME: getpid, thread_self
 *
 * FUNCTION:
 *	getpid() returns the process ID of the calling process
 *	thread_self() returns the thread ID of the calling thread
 *
 * EXECUTION ENVIRONMENT:
 *
 *	getpid() and thread_self() may be called from the interrupt 
 * 	environment and consequently must be in pinned memory.
 *
 * RETURNS:
 *	successful: returns process/thread ID requested
 *	failure:    not a process/thread
 */
pid_t
getpid(void)
{
	if(csa->prev != NULL)
		return(-1);
	return(U.U_procp->p_pid);
}

tid_t
thread_self(void)
{
        if(csa->prev != NULL)
                return(-1);
        return(curthread->t_tid);
}


/*
 * NAME: setpgid 
 *
 * FUNCTION: sets ths process group ID (supports job control)
 *
 *	This is the system call handler for all the variations of setpgid.
 *	The library routines for AT&T's setpgrp() and BSD's setpgrp() transform
 *	their arguments into the setpgid() form, and make this system call.
 *
 * EXECUTION ENVIRONMENT:
 *	may not page fault, disables interrupts.
 *
 * NOTES:
 *	setpgrp() by AT&T is a subset, it has no parameters and sets the
 *	process group ID of the calling process to its process ID and
 *	returns the new value.  Maps to setpgid(0, 0), returns pgrp.
 *
 *	setpgrp(pid, pgrp) by BSD has two parameters and allows the calling
 *	process to specify the pid.  Map to setpgid(pid, pgrp) purposely
 *	restricts access to unchanged children only.
 *
 * RETURNS:
 *	 0	upon success
 *	-1	if unsuccessful, errno indicates the reason
 */
int
setpgid (pid_t pid, pid_t pgrp)
/* pid_t		pid, pgrp;	process group ID to join */
{
	struct proc	*p, *g, *q;	/* current, group, temp pointers */
	int		ipri;		/* save interrupt priority */
	struct thread	*ct = curthread;/* current running thread */
	struct proc	*cp;		/* current running process */
	char		*errorp;	/* current u.u_error */

	cp = ct->t_procp;
	errorp = &ct->t_uthreadp->ut_error;

	TRCHKGT_SYSC(SETPGRP,pid,pgrp,NULL,NULL,NULL);

	if (pid == 0)	/* special cases for zero arguments */
		pid = cp->p_pid;
	if (pgrp == 0)
		pgrp = pid;

	*errorp = 0;

	/* 
	 * To change process groups, which can only happen at the
	 * process level, one needs both the proc_tbl_lock and
	 * the proc_int_lock.  To traverse the process group list 
	 * (without changing it) one needs the proc_int_lock or proc_tbl_lock.
	 */
	simple_lock(&proc_tbl_lock);
	p = VALIDATE_PID(pid);
	g = (pgrp == pid) ? p : VALIDATE_PID(pgrp);

	/* 
	 * check for errors:
	 * pid must be calling process or a child 
	 * pgrp < 0 or not a possible pid value 
	 */
	if ( !p || 
	     ((pid != cp->p_pid) && (p->p_ppid != cp->p_pid)) || 
	     p->p_stat == SNONE )
	{
		*errorp = ESRCH;  
	}
	else if ((pgrp < 0) || (pgrp > PIDMAX))
	{
		*errorp = EINVAL;	
	}

	/*
     	 * EPERM: pid is a session leader or
     	 *        caller and child are not in the same session or
     	 *        pgrp != pid and no process group in session matches pgrp
     	 */
	else if (p->p_sid == pid ||
	         p->p_sid != cp->p_sid ||
		 (pgrp != pid && (!g || !g->p_ganchor || g->p_sid != p->p_sid)))
	{
		*errorp = EPERM;
	}
	else if (pid != cp->p_pid && p->p_flag & SEXECED)
	{
		*errorp = EACCES;	/* child has sucessfully exec'd */
	}

	/* Passed error tests */
	else if (p->p_pgrp != pgrp) 
	{					/* pgrp not already set  */

		if (p->p_pgrp)			/* in a process group    */
			resign_pgrp(p);		/* leave current group   */

                ipri = disable_lock(INTMAX, &proc_base_lock);

		p->p_pgrp = pgrp;		/* set new process group */

		/* chain process to process group */
		if (pid != pgrp || p->p_ganchor) {
		   /* 
	            * Either 1) the process is joining a different pgrp or
	            *        2) the process was a process group leader 
	            *           that resigned from his process group,
	            *		 which means that a process can anchor 
	            *           one process group and be a member of another.
	            */
		   ASSERT(g->p_ganchor);	/* process group exists */

		   /*
		    * Must chain the process to the anchor of the process group
		    * which may differ from the slot given by the process group
		    * id.
		    */
		   p->p_pgrpl = g->p_ganchor->p_pgrpl;
		   g->p_ganchor->p_pgrpl = p;

		   /* New process may un-orphan the process group */
		   if (g->p_flag & SORPHANPGRP)
		   {
			/* 
			 * Parent is in a different pgrp but in the 
			 * same session.
			 */
			if (PROCPTR(p->p_ppid)->p_pgrp != p->p_pgrp &&
			    PROCPTR(p->p_ppid)->p_sid  == p->p_sid)
			{
				/* un-orphan the pgrp */
				for (q = g->p_ganchor; q; q = q->p_pgrpl)
					q->p_flag &= ~SORPHANPGRP;
			}
			else
			{
				/* joined an orphaned pgrp */
				p->p_flag |= SORPHANPGRP;
			}
		   } 
		   else
		   {
			p->p_flag &= ~SORPHANPGRP;
		   }
		} 
		else 
		{
		   p->p_pgrpl = NULL;		/* only member of pgrp	    */
		   p->p_ganchor = p;		/* anchor the pgrp	    */

		   if (p->p_sid) 
		   {				/* process is in a session  */
			/* 
			 * Insert new pgrp into ttyl chain of 
			 * pgrp's in the session 
			 */
			g = PROCPTR(p->p_sid);	/* session leader	    */
			p->p_ttyl = g->p_ttyl;
			g->p_ttyl = p;		/* insert new process group */

			/* 
			 * If parent is in the session, the pgrp is 
			 * not orphaned 
			 */
			q = PROCPTR(p->p_ppid);
			if (q->p_sid == p->p_sid)
				p->p_flag &= ~SORPHANPGRP;
			else
				p->p_flag |= SORPHANPGRP;
		   }
	    	}

                unlock_enable(ipri, &proc_base_lock);
	}

	simple_unlock(&proc_tbl_lock);

	return ( *errorp ? -1 : 0 );	/* return: 0 success, -1 error */

}


/*
 * NAME: orphan_pgrp()
 *
 * FUNCTION: 
 *	p is a member of a process group that may be orphaned when
 *      p exits.  If so, then mark the members of the process group
 *     	as belonging to an orphaned process group.  A process group
 *     	is orphaned when no process in the group has a parent in the
 *     	same session but in a different process group.
 *
 *     	Two possibilities for newly orphaned process groups:
 *      (1) the exiting process orphans the process group it belongs to,
 *      (2) the exiting process orphans children in another pgrp
 *
 * EXECUTION ENVIRONMENT: 
 *	This code is serialized by having the proc_lock. 
 *	Cannot page fault after disabling.
 *
 * NOTE: Caller checks that p->p_sid != 0
 *       Only called by kexit() and setsid().
 *
 * 	Both orphan_pgrp and orphan_child_pgrp do nothing if there is only
 * 	1 process group in the session.  An optimization for performance is
 * 	to check if there is only 1 process group in the session and return.
 * 	The optimization lengthens the worst case path which is a consideration
 * 	in real time support.
 *
 * 	There is only 1 process group in the session if (1) and (2) are met:
 *   	(1) p->p_ttyl == NULL, and
 *   	(2) i. p is the exiting session leader, or
 *	   ii. p is not the session leader
 *	       but the session leader has already exited and the session
 *	       leader's p_ttyl anchor points to p's process group.
 *
 *      In code,
 *	    i.	p->p_pid == p->p_sid
 *        		or,
 *         ii.	s = PROCPTR(p->p_sid);
 *	    	(s->p_flag & SEXIT) && s->p_ttyl = PROCPTR(p->p_pgrp)
 */

void 
orphan_pgrp(struct proc *p)
{
	register struct proc  *pp, *q;	/* pointers to proc table slots    */

	ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
	ASSERT(lock_mine(&proc_base_lock));
	ASSERT(lock_mine(&proc_int_lock));
#endif

	if (p->p_flag & SORPHANPGRP)
	{
		return;			/* pgrp is already orphaned	   */
	}
	/* 
	 * p can orphan its pgrp if its parent is in the same session 
	 * and in a different pgrp.  For real time, this optimization can 
	 * be deleted 
	 */
	q = (PROCPTR(p->p_ppid));
	if (q->p_pgrp == p->p_pgrp || q->p_sid != p->p_sid)
	{
		return;
	}

	/* 
	 * Orphan the exiting process's pgrp unless another member of the
         * pgrp has a parent in the same session but a different pgrp. 
	 */
	for (q = PROCPTR(p->p_pgrp)->p_ganchor; q; q = q->p_pgrpl) 
	{
		if ((pp = PROCPTR(q->p_ppid))->p_pgrp != p->p_pgrp)
		{
			/* 
			 * if parent is in the same sesson and its not 
			 * the exiting process, then process group is
			 * not orphaned.
			 */
			if ((pp->p_sid == p->p_sid) && (q != p))	
				break;	
		}
	}

	if (q == NULL)			/* orphaning process group   */
		set_orphan_pgrp( p->p_pgrp );

}


/*
 * NAME: orphan_child_pgrp()
 *
 * FUNCTION: 
 *	orphan child process's process groups as needed
 *
 * EXECUTION ENVIRONMENT: 
 *	Caller has the proc lock.
 *	Cannot page fault.
 *
 * NOTE: 
 *	Caller checks that p->p_sid != 0
 *      Only called by kexit() and setsid().
 */

void 
orphan_child_pgrp(struct proc *p)
{
	register struct proc  *g, *pp, *q;   /* pointers to proc table slots */

	ASSERT(csa->intpri == INTMAX);
#ifdef _POWER_MP
	ASSERT(lock_mine(&proc_base_lock));
	ASSERT(lock_mine(&proc_int_lock));
#endif

	/*
         * When a child is in a different pgrp and the pgrp is not orphaned
         * and the child is in the same session, check if the exiting process
         * orphans the child's pgrp.
         *
         * The child's pgrp is not orphaned if any process in its pgrp has a
         * parent in a different process group in the same session.
         *
         * p is exiting so it will not stop a process group from 
	 * becoming orphaned.
         */
	for (q = p->p_child; q ; q = q->p_siblings) 
	{
		if (  q->p_pgrp != p->p_pgrp   && 
		      q->p_pgrp                &&
		    !(q->p_flag & SORPHANPGRP) && 
		      q->p_sid  == p->p_sid) 
		{
		   for (g = PROCPTR(q->p_pgrp)->p_ganchor; g; g = g->p_pgrpl) 
		   {
			pp = PROCPTR(g->p_ppid);

			if (pp->p_pgrp != q->p_pgrp && /* different pgrp      */
			    pp->p_pid != p->p_pid   && /* not p's child       */
			    pp->p_sid == q->p_sid)     /* same session	      */
				break;		       /* pgrp is not orphaned*/
		   }

		   if (g == NULL)		/* exiting proc orphans pgrp  */
			set_orphan_pgrp( q->p_pgrp );
		}
	}
}


/*
 * NAME: set_orphan_pgrp()
 *
 * FUNCTION: mark process group as orphaned.
 *
 *	If any process in the pgrp is stopped, then signal the pgrp with 
 *	SIGHUP and SIGCONT.  POSIX 3.2.2 (Point 8): In an implementation 
 *	supporting job control, a SIGHUP signal followed by a SIGCONT signal 
 *	is sent to each process in a newly-orphaned process group if any 
 *	member of the newly-orphaned process group is stopped.
 *
 * EXECUTION ENVIRONMENT: 
 *	Caller has the proc lock.
 * 	Caller has also disabled interrupts to serialize access to p_pgrpl.
 *	Cannot page fault.
 */

static void 
set_orphan_pgrp( pid_t pgrp /* mark pgrp as orphaned */ )
{
	register struct proc	*g;

	for (g = PROCPTR(pgrp)->p_ganchor; g; g = g->p_pgrpl) 
	{
		g->p_flag |= SORPHANPGRP;	/* mark in orphaned pgrp*/
		/* a process is stopped or stopping */
		if (g->p_stat == SSTOP || g->p_int & SSUSP) {	
			while (g = g->p_pgrpl)	/* finish orphaning pgrp */
				g->p_flag |= SORPHANPGRP;
			pgsignal(pgrp, SIGHUP);	/* post hangup to pgrp	 */
			pgsignal(pgrp, SIGCONT);/* unstop, deliver HUP	 */
			break;		/* send signals to pgrp at most once */
		}
	}
}


/*
 * NAME: update_proc_slot()
 *
 * FUNCTION: 
 *	remove process from its process group and when appropriate, free
 *     	process slots whose p_ganchor and p_ttyl are no longer used.
 *
 * EXECUTION ENVIRONMENT: 
 *	The caller has the proc lock.
 *	This subroutine may not page fault.
 *
 * NOTE: 
 *  	Care is taken when deleting or inserting into the linked lists. The 
 *	linked lists are modified by a single step so that a new link is set 
 *	to the next link before setting the previuos link to point to the new 
 * 	link. update_proc_slot is called only by kwaitpid() and setsid().

 *	If p is a session leader or p is a process group leader and any
 *	active process is still in the process group, then the p_ttyl and
 *	p_ganchor chains are unchanged and serve as anchors to the linked
 *	lists.
 */

void 
update_proc_slot(struct proc *p)
{
	register struct proc	*g;	/* pointer to process group leader */
	register struct proc	*q;	/* pointer to process table slots  */
	register struct proc	*s;	/* pointer to session leader	   */
	int 	ipri;			/* interrupt priority		   */

	ASSERT(p->p_pgrp != 0);
	ASSERT(csa->intpri == INTBASE);
	ASSERT(lock_mine(&proc_tbl_lock));

        ipri = disable_lock(INTMAX, &proc_base_lock);

	/*
         * If an already exited pgrp leader or session leader slot was anchoring
         * the p_ganchor or p_ttyl lists, then free the proc table slot of the
         * already exited process group leader and/or session leader under these
         * conditions:
         *  1) The last process in the group has been waited for by its parent
	 *     (or init) or last process in the group has called setsid().
         *  2) If the process group leader was also a session leader,
         *     there must be no active process in the session.
         *  3) The already exited process is not a zombie (p_stat changed from
         *     SZOMB to SNONE in freeproc() called by kwaitpid() when a parent
         *     or init has waited for the process.)
         *
         * Note that "exited," as used here, signifies that the parent has
         * waited for the process so that it is no longer a zombie.  As a
         * result, freeproc has already been called.  The call to 
	 * freeprocslot(r) puts the already exited process' proc slot on 
	 * the free list of process slots to be reused.  This code depends 
	 * on freeproc setting p_stat == SNONE.
         *
         * Remove process from p_pgrpl chain.
         * May need to remove process group from old session's p_ttyl chain.
         * May need to free slot of exited session leader or process group
         * leader when no process is in the session or process group.
         * This code allows for unlikely cases like the last process
         * in a session becoming the session leader of a new session.
         */
	g = PROCPTR(p->p_pgrp);		/* process group leader anchors list */

	/* remove a process from a pgrp */
	if ((q = g->p_ganchor) == p) 
	{
		/* may be the process group leader   */
		g->p_ganchor = p->p_pgrpl;	
	} else 
	{
		for (; q && q->p_pgrpl != p; q = q->p_pgrpl);

		/* check to make sure we found the entry. It has to be there */
		assert(q);

		/* remove p from p_pgrpl chain */
		q->p_pgrpl = p->p_pgrpl;	

	}

	p->p_pgrpl = NULL;		/* !!! extraneous ? !!!		     */
	
        unlock_enable(ipri, &proc_base_lock);

	if (g->p_ganchor == NULL) 	/* process group is now empty	     */
	{	
		if (g->p_sid) 		/* member of a session		     */
		{		
			s = PROCPTR(g->p_sid);
			if (g != s) 	/* the pgrp ldr is not a session ldr */
			{	
				for (q = s; q->p_ttyl != g; q = q->p_ttyl);

				/* remove from list of process groups*/
				q->p_ttyl = g->p_ttyl;	

				/* remove from list of groups */
				g->p_ttyl = NULL;	
			}

			/* 
			 * freeproc(s) did not freeprocslot() because 
			 * of session members 
			 */
			if ( s->p_stat==SNONE && 
			     s->p_ganchor == NULL && 
			     s->p_ttyl == NULL)
			{
				/* no members in session, free slot  */
				freeprocslot(s);	
			}
		}

		/* 
		 * freeproc(g) did not freeprocslot() for leader 
		 * of an active pgrp 
		 */
		if (g->p_stat == SNONE && g->p_pid != g->p_sid)
			/* no members in pgrp, free proc slot*/
			freeprocslot(g);	
	}
}


/*
 * NAME: resign_pgrp 
 *
 * FUNCTION: 
 *	Process is revoking its membership in a process group.
 *     	Caller is changing the process' process group membership.
 *
 * EXECUTION ENVIRONMENT: 
 *	may not page fault. 
 *
 * RETURNS: none
 */

static void 
resign_pgrp(struct proc	*p)
{
	struct proc *g, *q;
	int 	intpri;

        intpri = disable_lock(INTMAX, &proc_base_lock);

	/* 
	 * Check if the process group that p is leaving might 
	 * become orphaned if p and p's parent are in different 
	 * process groups in the same session. 
	 */
	if ( (q = PROCPTR(p->p_ppid))->p_pgrp != p->p_pgrp &&
	      q->p_sid == p->p_sid 			   && 
	      p->p_sid )
	{
#ifdef _POWER_MP
		simple_lock(&proc_int_lock);
#endif
		orphan_pgrp(p);
#ifdef _POWER_MP
		simple_unlock(&proc_int_lock);
#endif
	}


	/* remove process from process group in which it is a member */
	g = PROCPTR(p->p_pgrp);
	if ((q = g->p_ganchor) == p)
	{
		g->p_ganchor = p->p_pgrpl;
	}
	else 
	{
		for (; q && q->p_pgrpl != p; q = q->p_pgrpl);

		/* Ensure we've found the entry.  It has to be there. */
		assert(q);

		q->p_pgrpl = p->p_pgrpl;
	}
	p->p_pgrpl = NULL;		/* !!! extraneous ? !!!		     */

        unlock_enable(intpri, &proc_base_lock);

	/*
         * if p was the last process in the process group,
         *   (1) if in a session, remove the process group from the p_ttyl list,
         *   (2) free the proc slot of the process group leader
         */
	if (g->p_ganchor == NULL) 
	{
		if (g->p_sid != g->p_pid) /* pgrp leader not a session leader */
		{	
			if (g->p_sid) 	 /* pgrp leader was part of a session */
			{		
				for (q = PROCPTR(g->p_sid); 
					q->p_ttyl != g; q = q->p_ttyl);

				/* remove pgrp from session	     */
				q->p_ttyl = g->p_ttyl;	

				/* remove from list of groups */
				g->p_ttyl = NULL;	
			}
			if (g->p_stat == SNONE)
				/* reuse proc slot, put on free list */
				freeprocslot(g);	
		} 
		/* 
		 * else do nothing to session leader because p is 
		 * in the session   
		 */
	}
}

/*
 * NAME: setsid 
 *
 * FUNCTION: sets the calling process to a session leader
 *
 *	If the the calling process is not a session leader, a new session is 
 *	created.  The calling process becomes the session leader of this new
 *	session with no controlling terminal.
 *
 * EXECUTION ENVIRONMENT:
 *	preemptable
 *	may page fault
 *
 * RETURNS:
 *	 pid_t	upon success, the new process group ID
 *	 -1	if unsuccessful, errno indicates the reason
 */

pid_t setsid(void)
{
	struct proc	*p = U.U_procp;		/* process table pointer */

	/* error if a pgrp leader. ie p->p_ganchor->p_pgrp == p->p_pid */
	if (p->p_ganchor)
		u.u_error = EPERM;		
	else
		setsid2();

	return(u.u_error ? -1 : p->p_pgrp);	/* return process group */
}

/*
 * NAME: setsid2
 *
 * FUNCTION: sets the calling process to a session leader
 *
 *	If the the calling process is not a session leader, a new session is 
 *	created.  The calling process becomes the session leader of this new
 *	session with no controlling terminal.
 *
 * EXECUTION ENVIRONMENT:
 *	preemptable
 *	may page fault
 *
 * RETURNS:
 *	 pid_t	upon success, the new process group ID
 *	 -1	if unsuccessful, errno indicates the reason
 */

pid_t 
setsid2()
{
	struct proc	*p = U.U_procp;		/* process table pointer   */
	int		ipri;			/* save interrupt priority */

        simple_lock(&proc_tbl_lock);

        ipri = disable_lock(INTMAX, &proc_base_lock);

	if (p->p_pgrp) {
		if (p->p_sid) {		/* p leaving the session may cause a */
#ifdef _POWER_MP
			simple_lock(&proc_int_lock);
#endif
			orphan_pgrp(p);	/* process group to become orphaned  */
			orphan_child_pgrp(p);
#ifdef _POWER_MP
			simple_unlock(&proc_int_lock);
#endif
		}
                unlock_enable(ipri, &proc_base_lock);
		update_proc_slot(p);	/* remove from pgrp's p_pgrpl chain  */
        	ipri = disable_lock(INTMAX, &proc_base_lock);
	}

	/* 
	 * new session leader is only process in its process group
         * and its process group is only pgrp in session 
	 */
	p->p_pgrpl = p->p_ttyl = NULL;

	/* process becomes a session leader and process group leader */
	p->p_sid = p->p_pgrp = p->p_pid;
	p->p_ganchor = p;	/* need anchor b/c leader can join a pgrp   */

	/*
         * By definition, the session leader is in an orphaned process
         * group.  No parent of processes in the session leader's process
         * group is in a different process group in this session.  (POSIX,
         * P1003.1, p. 201)
         */
	p->p_flag |= SORPHANPGRP;
	p->p_flag &= ~(SNOCNTLPROC|SPPNOCLDSTOP|SJOBSESS|SJOBOFF);

        unlock_enable(ipri, &proc_base_lock);

	/* initially, the session leader has no controlling terminal */
	U.U_ttysid = U.U_ttyp = (pid_t *) 0;
	U.U_ttyid = 0;
	U.U_ttyf = NULL;

        simple_unlock(&proc_tbl_lock);
	return(u.u_error ? -1 : p->p_pgrp);	/* return process group */
}
