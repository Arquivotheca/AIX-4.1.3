static char sccsid[] = "@(#)21	1.32.1.25  src/bos/kernel/proc/newproc.c, sysproc, bos41J, 9516B_all 4/14/95 13:07:18";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: PCDT_SIZE
 *		alloc_pcdt
 *		bzero
 *		freeproc
 *		freeprocslot
 *		newproc
 *		proc_dump_init
 *		proccdtf
 *
 *   ORIGINS: 27, 3, 26, 83
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
#include <sys/sched.h>
#include <sys/sleep.h>
#include <sys/prio_calc.h>
#include <sys/var.h>
#include <sys/systm.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/dump.h>
#include <sys/malloc.h>
#include <sys/pseg.h>
#include <sys/user.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

extern struct thread *newthread(char *error);
extern void freethread(struct thread *t);
extern ulong g_kxsrval;			/* value of segment with proc table  */
void freeprocslot( struct proc *p);	/* put proc slot on free list   */

struct pm_heap proc_cb;
static void *proccdt;			/* dump table for proc and u areas   */

/* reserve dump table slots for process table entry and kernel region   */
#define PCDT_INCR	128		/* xmalloc cdt_entry's		*/
#define PCDT_SIZE(n)	(sizeof(struct cdt_head) + \
			 (n * sizeof(struct cdt_entry)))
#define PCDT_INCREASE	(PCDT_INCR * sizeof(struct cdt_entry))
#define PCDT_DECREASE	(2 * PCDT_INCR * sizeof(struct cdt_entry))

/*
 * NAME: newproc
 *
 * FUNCTION: allocate a new process table entry
 *
 * NOTES:
 *
 *	This creates a process that eventually will be turned into
 *	either a user process or a kernel process.
 *
 *	The SIDL state is needed for exit() to realize that this
 *	is not yet a full process.  It is hooked onto the parent's
 *	chain only for purposes of exit() remembering to throw
 *	away the child pid if the parent gets killed before it finishes
 *	initializing the child.  Processes in the SIDL state cannot
 *	be sent signals.
 *
 * Process State Transitions:
 *	SNONE ==> SIDL
 *
 * EXECUTION ENVIRONMENT:
 *	Must be called under a process.
 *	Caller is expected already to have the process management lock.
 *	Cannot page fault since it disables interrupts.
 *	When called from strtdisp() during system initialization,
 *		there is no u-block available.  That is why the address
 *		to return error codes is passed in place of u.u_error.
 *
 * RETURNS:	proc struct of newly created process, if successful;
 *		NULL, if unsuccessful (*error has the reason).
 *
 *	Possible error values are:
 *		EAGAIN, if unable to create a new process or if the
 *			caller's user ID cannot create more
 *		ENOMEM, if unable to pin more proc structures
 */

struct proc *
newproc(register char *error, ulong ptype)
/* error		 address to return errno value */
{
	register struct proc *p;		/* new proc pointer */
	register struct proc *cp=curproc;	/* current process */
	register struct thread *t;		/* new thread pointer */
	register struct proc *q;		/* temp pointer */
	int ipri;
	int privcheck_rc;

	if (!(p = (struct proc *)pm_alloc(&proc_cb, error)))
		return(NULL);

	/* p_pid is allocated by pm_alloc */

	if (!(t = newthread(error))) {
		pm_free(&proc_cb, (char *)p);
		return(NULL);
	}

	if (error == &u.u_error)
		privcheck_rc = privcheck(SET_PROC_RAC);

	/* 
	 * Initialize those fields that do not require locking.  They
	 * are only referenced by other threads in the same process and 
	 * in this case there aren't any.
	 */
	t->t_procp = p;
	t->t_prevthread = t->t_nextthread = t;

	p->p_threadlist = t;
	p->p_threadcount = 1;
	p->p_sched_pri = PIDLE;			   /* null value for sched_pri*/
	p->p_synch = (struct thread *)EVENT_NULL;  /* null value for synch */ 

	/* may page fault */
	lock_alloc(&p->p_lock, LOCK_ALLOC_PIN, 
			PROC_LOCK_CLASS, p-(struct proc *)&proc);
	simple_lock_init(&p->p_lock);

	p->p_adspace = NULLSEGVAL;		   /* null value for adspace */

	/*
	 * The following fields are initialized from the parent.  The locking
	 * model only needs to be strictly followed when adding the new process
	 * to an existing list.  This particular order has been chosen because 
	 * it minimizes the cache footprint.
	 */
	simple_lock(&proc_tbl_lock);

        /* inherit some of the parents process attributes */
        p->p_flag  = cp->p_flag & (SKPROC | SFIXPRI     |
			      SORPHANPGRP | SNOCNTLPROC |
			         SJOBSESS | SJOBOFF     |
			   	    SLOAD | SPSEARLYALLOC |
			       SSIGNOCHLD | SSIGSET);
        /*
         * if the process will be running with a "fixed priority" then
         * set the 'no swap" bit. This is being done to reduce the latency
         * period when a process changes states - from "non-runnable" to
         * "runnable"
         */
        if (p->p_flag & SFIXPRI)
            	p->p_flag |= SNOSWAP;

	/*
	 * If this is a real fork (not one of the initial few
	 * processes) and we do not have SET_PROC_RAC access, then we
	 * link this process onto the list for the current uid,
	 * otherwise, we don't.
	 * 
	 * This makes processes with SET_PROC_RAC access not be
	 * counted in the total of processes for a given uid.
	 */
	if (error == &u.u_error && privcheck_rc) {
	    	p->p_uidl = cp->p_uidl; 	/* add child to user ID list */
	    	cp->p_uidl = p;
	} else
	    	p->p_uidl = p;

        p->p_nice      = cp->p_nice;
	p->p_uid       = cp->p_uid;
	p->p_suid      = cp->p_suid;
	p->p_ppid      = cp->p_pid;
	p->p_sid       = cp->p_sid;
	p->p_sigcatch  = cp->p_sigcatch;
	p->p_sigignore = cp->p_sigignore;
	p->p_auditmask = cp->p_auditmask;

	/* set process type flags */
	if (ptype == SKPROC) {
		p->p_flag |= SKPROC;
		t->t_flags |= TKTHREAD;
		t->t_userdata = 0;

        	/* reset sig fields which are not inherited for kprocs */
        	SIGINITSET(p->p_sig);
        	SIGINITSET(p->p_sigignore);
        	SIGINITSET(p->p_sigcatch);
        	SIGINITSET(t->t_sig);
        	SIGINITSET(t->t_sigmask);
	}

	ipri = disable_lock(INTMAX, &proc_base_lock);

	p->p_stat = SIDL;

	/* child linkage */
	p->p_siblings = cp->p_child;
	cp->p_child = p;

	/* if pgrp is not 0, chain child to parent's process group */
	if (p->p_pgrp = cp->p_pgrp) {		/* assign pgrp */
	    	p->p_pgrpl = cp->p_pgrpl;	
	    	cp->p_pgrpl = p;
	} 

	t->t_state = TSIDL;

	unlock_enable(ipri, &proc_base_lock);
	simple_unlock(&proc_tbl_lock);

	return(p);
}

/*
 * NAME: freeproc
 *
 * FUNCTION: return a process slot to the free list.
 *
 * NOTE:
 *	This is the inverse operation of newproc(), except that the
 *	caller has already removed the process from its parent's child
 *	list (because the caller knows where it is in that chain) and
 *	fork() or update_proc_slot() has removed the process from the
 *	p_pgrpl list.
 *
 * EXECUTION ENVIRONMENT:
 *	Caller has the proc_lock.
 *	Caller has disabled interrupts to serialize acess to p_pgrpl.
 *	Cannot page fault.
 */

void
freeproc( struct proc *p	/* proc table entry to free */ )
{
    	register struct proc *q;	/* for unhooking things		   */
    
    	/* caller MUST own the process management lock */
    	ASSERT(lock_mine(&proc_tbl_lock));

    	/* delete from the UID list */
    	q = p;
    	while (q->p_uidl != p)
		q = q->p_uidl;
    	q->p_uidl = p->p_uidl;

    	/* free the thread slot */
    	freethread(p->p_threadlist);

    	/* slot available or used only as an anchor for p_pgrpl or p_ttyl */
    	p->p_stat = SNONE;

    	/*
     	 * p_ttyl is set in setpgid() to link new process groups in
     	 * a session (p_sid != 0).  update_proc_slot() removes a pgrp
     	 * from the p_ttyl chain when no processes are in the pgrp.
     	 *
     	 * fork(), newproc(), setpgid() and setsid() set p_pgrpl.
     	 * update_proc_slot() removes processes from p_pgrpl.
     	 *
     	 * If a process group leader exits while processes are still members
     	 * of the process group, then the process group leaders proc slot
     	 * is used to anchor the process group list with p_ganchor.
     	 *
     	 * Likewise, the p_ganchor and p_ttyl pointers of the slot of an exited
     	 * session leader anchor the lists of active processes in its pgrp and
     	 * process groups in its session.
     	 *
     	 * WARNING: The session leader and process group leader proc slots
     	 * may be reused once the last process in the session or pgrp has 
	 * exited.  But the call to freeproc for the zombies may occurr much 
	 * later.  For this reason, freeproc at this point examines only the 
	 * process slot of the zombie that it is freeing.
     	 *
     	 * Free slot if,
     	 * 	(1) not a process group leader ( See Note below )
     	 * 	    OR,
     	 *  (2) a session leader with no process left in the session
     	 *      OR,
     	 *  (3) a pgrp ldr that is not a session leader with no process 
	 *      in its pgrp
     	 *
     	 * NOTE: A process can be an anchor for one group and a 
	 *       member of another.
     	 *
     	 */
    	if ((p->p_pid != p->p_pgrp && p->p_ganchor == NULL) ||
	    (p->p_ganchor == NULL && (p->p_sid != p->p_pid||p->p_ttyl == NULL)))
		freeprocslot(p);	/* put proc slot on free list      */
}

/*
 * NAME: freeprocslot
 *
 * FUNCTION: put a proc slot on the free list, adjust space in dump table
 *
 * EXECUTION ENVIRONMENT: 
 *	Caller has proc_lock.
 *
 * RETURNS:	
 * 	None.
 *
 */

void
freeprocslot( struct proc *p )
{    
    	ASSERT(lock_mine(&proc_tbl_lock));
    	ASSERT(!(p->p_int & SGETOUT));

    	/* free PROC_LOCK. Let's hope nobody is using it !!! */
    	lock_free(&p->p_lock);

    	/* return proc structure to the free list */
    	pm_free(&proc_cb, (char *)p);
}

/*
 * NAME: alloc_pcdt
 *
 * FUNCTION: allocate memory for proccdt, proc component dump table
 *
 * NOTES:
 *	The proccdt must be pinned in memory so that
 *	when a dump is done, the table can be filled in with
 *	pointers to proc table slots and kernel regions.  Therefore,
 *	2 slots in the dump table are needed for each process.
 *
 * EXECUTION ENVIRONMENT:
 *	Calls xmalloc.   A realloc to extend or shrink requested memory
 *	would be better.
 *
 * RETURNS:	None.
 *
 */
void
alloc_pcdt( int nproc )
{
    	register int size;		/* size of memory to allocate      */
    	register struct cdt_head *ch;	/* component dump table header ptr */
    	register void *new_pcdt;	/* new proc component dump table   */
    	register void *x;
    
    	size = PCDT_SIZE( nproc );
    	ch = (struct cdt_head *) proccdt;

    	/* increase pcdt size by PCDT_INCREASE or decrease by PCDT_DECREASE */
    	if (!ch || size >= ch->_cdt_len)
		size += PCDT_INCREASE;
    	else if (size > (ch->_cdt_len - PCDT_DECREASE))
		return;

    	/* allocate pinned memory for proc component dump table */
    	if ((new_pcdt = (void *) 
			xmalloc((uint) size, (uint) 0, pinned_heap)) != NULL) 
	{

		bzero(new_pcdt, size);
		x = proccdt;				/* save old table    */
		proccdt = new_pcdt;			/* set new table     */
		if (x)
	    		xmfree(x, pinned_heap);		/* free old table    */
		ch = (struct cdt_head *) proccdt;
		strcpy(ch->_cdt_name,"proc");		/* component is proc */
		ch->_cdt_len = size;			/* pcdt length	     */
		ch->_cdt_magic = DMP_MAGIC;		/* dump magic number */
    	}
}    

/*
 * NAME:     proccdtf
 *
 * FUNCTION: proc component dump table function initializes proccdt
 *	     with pointers to processes' proc table slots and to their
 *	     kernel region (kernel stack, load anchors, u area, more anchors).
 *
 * RETURNS:	Called by dmp_do at dump time, proccdtf returns 
 * 		a pointer to proccdt, the process component dump table.
 */
static struct cdt_head *
proccdtf( int status )
{
    	register struct proc *p;
    	register struct cdt_entry *c, *ce; /* component dump table entry ptr  */
    	struct cdt_head *ch;               /* ptr to component dump table hdr */

    	if (status == 2 || proccdt == NULL) {
        	return( (struct cdt_head *) NULL);
    	}

    	ch = (struct cdt_head  *) proccdt;               /* set pcdt header   */
    	c  = (struct cdt_entry *) (ch + 1);                  /* first entry   */
    	ce = (struct cdt_entry *) ((char *) ch + ch->_cdt_len); /* end of tbl */

    	/* dump valid proc entries & kernel regions (user struct, anchors) */
    	for (p = proc; c < ce && p < max_proc; p++) {

        	if ((p->p_stat != SNONE) || 
		    (p->p_pid && (p->p_pgrp == p->p_pid) && p->p_ganchor))
		{
            		/* dump proc entry */
            		sprintf(c->d_name, "%dp",p - proc); /* name = "slot#p"*/
            		c->d_len    = sizeof(struct proc);  /* length of data */
            		c->d_ptr    = (char *) p;           /* offset to data */
            		c->d_segval = g_kxsrval;            /* segment value  */
            		c++;                                /* next entry     */

            		/* dump user area, load anchors */
            		if (p->p_adspace != NULLSEGVAL && c < ce) {
                		sprintf( c->d_name, "%dU", p - proc ); 
                		c->d_ptr    = (char *)&__ublock;
                		c->d_len    = SEGSIZE - U_BLOCK;
                		c->d_segval = p->p_adspace;
                		c++;
            		}
        	}
    	}
    	ch->_cdt_len = (char *) c - (char *) ch;    /* actual length of pcdt  */

    	return( (struct cdt_head *) ch );
}

/*
 * NAME:     proc_dump_init
 *
 * FUNCTION: add proc component dump table to components to be dumped
 *
 * ENVIRONMENT: called once by strtdisp().
 *
 * RETURNS:  none
 *
 */
void
proc_dump_init()
{
    	dmp_add(proccdtf);	  /* put proc component in dump device table */
}
