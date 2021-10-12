static char sccsid[] = "@(#)37  1.20  src/bos/kernel/proc/newthread.c, sysproc, bos41J, 9513A_all 3/24/95 15:30:34";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: TCDT_SIZE
 *		alloc_tcdt
 *		freethread
 *		newthread
 *		thread_dump_init
 *		threadcdtf
 *
 *   ORIGINS: 27, 3, 26, 83
 *
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
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

extern ulong g_kxsrval;              	/* value of segment with thread table */

struct pm_heap thread_cb;		/* thread tables control block        */

static void *threadcdt;             	/* dump table for threads	      */

/* reserve dump table slots for thread table entry and kernel region   	*/
#define TCDT_INCR	128             /* xmalloc cdt_entry's                */
#define TCDT_SIZE(n)    (sizeof(struct cdt_head) + (n*sizeof(struct cdt_entry)))
#define TCDT_INCREASE   (TCDT_INCR * sizeof(struct cdt_entry))
#define TCDT_DECREASE   (2 * TCDT_INCR * sizeof(struct cdt_entry))

/*
 * NAME: newthread
 *
 * FUNCTION: allocate a new thread table entry
 *
 * NOTES:
 *
 *	This creates a thread that eventually will be turned into
 *	either a user thread or a kernel thread.
 *
 *	The TSIDL state is needed because the user needs to explicitly
 *	start the thread. Threads in the TSIDL state cannot be sent signals.
 *
 * 	Thread state transition :
 *		TSNONE ==> TSIDL
 *
 *	[In fact, the state is not changed to TSIDL because it must be done
 *	 atomically with the process state transition to SIDL and the linkage
 *	 (t_procp & p_threadlist).]
 *
 * EXECUTION ENVIRONMENT:
 *	Must be called under a process.
 *	When called from strtdisp() during system initialization,
 *		there is no u-block available.  That is why the address
 *		to return error codes is passed in place of u.u_error.
 *
 * RETURNS:	thread struct of newly created thread, if successful;
 *		NULL, if unsuccessful (*error has the reason).
 *
 *	Possible error values are:
 *		EAGAIN, if unable to create a new thread
 *		ENOMEM, if unable to pin more thread structures
 */

struct thread *
newthread(register char *error)
/* error		 address to return errno value */
{
	register struct thread *t;		/* new thread pointer */
	register struct thread *ct = curthread;	/* current thread     */

	if (!(t = (struct thread *)pm_alloc(&thread_cb, error)))
		return(NULL);

	/* Minimize cache misses by initializing in order */ 

	t->t_suspend = 1;			/* kernel mode: prevent sig */
	t->t_flags = ct->t_flags & TKTHREAD;
	t->t_stackp = ct->t_stackp;
	t->t_uthreadp = &uthr0;			/* fixed address (default) */
	t->t_userp = &U;			/* fixed address */
	t->t_synch = (struct thread *)EVENT_NULL; /* null value for sync */
	t->t_tsleep = EVENT_NULL;
	t->t_userdata = ct->t_userdata;
#ifdef _POWER_MP
	t->t_cpuid = t->t_scpuid = ct->t_scpuid;
	t->t_affinity = ct->t_affinity;
#endif
	t->t_pri = ct->t_pri;
	t->t_policy = ct->t_policy;
	t->t_wakepri = PIDLE;			/* null value for wakepri  */
	t->t_sav_pri = ct->t_sav_pri;
	t->t_sigmask = ct->t_sigmask;

	/* t_tid is allocated by pm_alloc */

	return(t);
}

/*
 * NAME: freethread
 *
 * FUNCTION: return a thread slot to the free list.
 *
 * NOTE:
 *	This is the inverse operation of newthread().
 *
 *	Thread state transition :
 *		xxx ==> TSNONE
 *
 * EXECUTION ENVIRONMENT:
 *	Must own proc_tbl_lock.
 */

void
freethread(struct thread *t	/* thread table entry to free */ )
{
    	ASSERT((t->t_state == TSNONE) || lock_mine(&proc_tbl_lock));

    	/* mark slot available */
    	t->t_state = TSNONE;

    	/* return thread structure to the free list */
    	pm_free(&thread_cb, (char *)t);
}

/*
 * NAME: alloc_tcdt
 *
 * FUNCTION: allocate memory for threadcdt, thread component dump table
 *
 * NOTES:
 *      The threadcdt must be pinned in memory so that
 *      when a dump is done, the table can be filled in with
 *      pointers to thread table slots and kernel regions.  Therefore,
 *      1(2) slot(s) in the dump table are needed for each process.
 *	[kernel stack and uthread should be dumped with thread instead of
 *	process]
 *
 * EXECUTION ENVIRONMENT:
 *      Calls xmalloc.   A realloc to extend or shrink requested memory
 *      would be better.
 *
 * RETURNS:     None.
 *
 */
void
alloc_tcdt( int nthread )
{
    	register int        size;          /* size of memory to allocate      */
    	register struct cdt_head    *ch;   /* component dump table header ptr */
    	register void       *new_tcdt;     /* new thread component dump table */
    	register void       *x;

    	size = TCDT_SIZE( nthread );
    	ch = (struct cdt_head *) threadcdt;

    	/* increase tcdt size by TCDT_INCREASE or decrease by TCDT_DECREASE */
    	if (!ch || size >= ch->_cdt_len)
        	size += TCDT_INCREASE;
    	else if (size > (ch->_cdt_len - TCDT_DECREASE))
        	return;

    	/* allocate pinned memory for thread component dump table */
    	if ((new_tcdt = (void *) 
			xmalloc((uint) size, (uint) 0, pinned_heap)) != NULL) 
	{

        	bzero(new_tcdt, size);
        	x = threadcdt;                         /* save old table      */
		threadcdt = new_tcdt;                  /* set new table       */
        	if (x)
            		xmfree(x, pinned_heap);        /* free old table      */
        	ch = (struct cdt_head *) threadcdt;
        	strcpy(ch->_cdt_name,"thrd");          /* component is thread */
        	ch->_cdt_len = size;                   /* tcdt length         */
        	ch->_cdt_magic = DMP_MAGIC;            /* dump magic number   */
    	}
}

/*
 * NAME:     threadcdtf
 *
 * FUNCTION: thread component dump table function initializes threadcdt
 *           with pointers to threads' thread table slots and to their
 *           kernel region ()
 *
 * RETURNS:     Called by dmp_do at dump time, threadcdtf returns
 *              a pointer to threadcdt, the thread component dump table.
 */
static struct cdt_head *
threadcdtf( int status )
{
        register struct thread *t;
        register struct uthread *ut;
        register struct cdt_entry *c, *ce;
        struct cdt_head *ch;

        if (status == 2 || threadcdt == NULL)
                return( (struct cdt_head *) NULL);

        ch = (struct cdt_head  *) threadcdt;
        c  = (struct cdt_entry *) (ch + 1);
        ce = (struct cdt_entry *) ((char *) ch + ch->_cdt_len);

        /* dump thread data */
        for (t = thread; c < ce && t < (struct thread *)v.ve_thread; t++) {

                if (t->t_state == TSNONE)
                        continue;

                /* dump thread entry */
                sprintf(c->d_name, "%dt",t - thread);   /* name is "slot#t" */
                c->d_len    = sizeof(struct thread);
                c->d_ptr    = (char *) t;
                c->d_segval = g_kxsrval;
                c++;

                /* dump user thread structure */
                sprintf(c->d_name, "%du",t - thread);   /*         "slot#u" */
                c->d_len    = sizeof(struct uthread);
                c->d_ptr    = (char *) t->t_uthreadp;
                c->d_segval = t->t_procp->p_adspace;
                c++;

		/* 
		 * Only dump the kernel stack if we're in kernel mode, 
		 * since a program cannot crash the system in user mode.
		 * Note: there is a small window where the thread is
		 * still SZOMB but has already lost its process private
		 * segment and thus has no stack anymore.
		 */ 
                if (t->t_suspend && t->t_procp->p_adspace != NULLSEGVAL) {
                        sprintf(c->d_name, "%ds",t - thread);   /* "slot#s" */

			/* 
			 * Calculate kernel stack size. 
			 *	normal stack.
			 * 	special fork stack.
			 * 	stack has been moved - grab a page. 
			 */
			ut = (struct uthread *)
				vm_att(t->t_procp->p_adspace, t->t_uthreadp);

			c->d_ptr = (char *)(ut->ut_save.gpr[1] & ~(PAGESIZE-1));
                        c->d_len = (ulong)ut->ut_kstack - (ulong)c->d_ptr;
			if ((c->d_len < 0) || (c->d_len > KSTACKSIZE))
			   	c->d_len = (ulong)&__ublock - KSTACKSIZE -
							(ulong)c->d_ptr;
			if ((c->d_len < 0) || (c->d_len > KSTACKSIZE))
			   	c->d_len = PAGESIZE;
			c->d_segval = as_getsrval(&ut->ut_save.as, c->d_ptr);

			vm_det(ut);
                        c++;
                }
        }
        ch->_cdt_len = (char *) c - (char *) ch;    /* actual length of tcdt  */

        return( (struct cdt_head *) ch );
}

/*
 * NAME:     thread_dump_init
 *
 * FUNCTION: add thread component dump table to components to be dumped
 *
 * ENVIRONMENT: called once by strtdisp().
 *
 * RETURNS:  none
 *
 */
void
thread_dump_init()
{
    	dmp_add(threadcdtf);    /* put thread component in dump device table */
}
