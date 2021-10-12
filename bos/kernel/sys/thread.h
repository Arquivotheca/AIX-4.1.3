/* @(#)55	1.31  src/bos/kernel/sys/thread.h, sysproc, bos41J, 9515B_all 4/10/95 13:52:46 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: MAYBE_TID
 *              THREADMASK
 *              THREADPTR
 *              VALIDATE_TID
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_THREAD
#define _H_THREAD

#include <sys/types.h>
#include <sys/mstsave.h>
#include <sys/processor.h>
#include <sys/lock_def.h>
#include <sys/var.h>
#include <sys/atomic_op.h>

/*
 *  One structure allocated per active thread.  Entries that are
 *  in use are pinned to avoid page faults in kernel critical sections.
 *
 *  This structure contains per-thread information that may be used by
 *  other threads in the process. Private per-thread data is kept in
 *  in the uthread structure, <sys/uthread.h>.
 *
 *  When changes are made here, be sure to update the assembler version
 *  in kernel/ml/thread.m4, too.
 */

#define MAXTHREADS	512		/* max number of threads per process, */
					/* IN ADDITION to the default thread. */
					/* MUST be a multiple of 128, assuming*/
					/* uthread is padded to 32 bytes and a*/
					/* page is 4096-byte long */

struct	thread {
	char		t_state;	/* thread state 		*/
	char		t_wtype;	/* type of thread wait		*/
	short		t_suspend;	/* suspend signal nesting level */
	unsigned long	t_flags;	/* thread flags (proc_int_lock)	*/
	unsigned long	t_atomic;	/* thread flags	(atomic op's)	*/
	char		*t_stackp;	/* saved user stack pointer	*/

	/* related data structures	*/
	struct proc	*t_procp;	/* owner process		*/
	struct {
	    struct uthread *uthreadp;	/* local data			*/
	    struct user	*userp;		/* owner process' ublock (const)*/
	}		t_uaddress;

	/* main thread link pointers	*/
	struct {
	    struct thread *prevthread;	/* previous thread		*/
	    struct thread *nextthread;	/* next thread			*/
	}		t_threadlist;	/* circular doubly linked list	*/

	/* sleep and lock fields */
	atomic_p	t_ulock;	/* wait identifier - user lock  */
	char		*t_wchan;	/* wait identifier - hashed	*/
	char		*t_wchan1;	/* wait identifier - real	*/
	int		t_wchan1sid;    /* SID of lock eaddr (wchan1)   */
	int		t_wchan1offset; /* OFFSET of lock eaddr (wchan1) */
	char		*t_wchan2;	/* VMM wait channel		*/
	char		*t_swchan;	/* simple/complex lock channel	*/
	struct thread	*t_eventlist;	/* subscribers to event list	*/
	int		t_result;	/* wait result			*/
	int		t_polevel;	/* page out wait level		*/
	unsigned long	t_pevent;	/* pending events		*/
	unsigned long	t_wevent;	/* awaited events		*/
	struct thread	*t_slist;	/* threads waiting for a slock	*/
	unsigned short	t_lockcount;	/* number of locks held		*/

	/* dispatcher fields */
	unsigned short	t_ticks;	/* # of ticks since dispatched	*/
	struct thread	*t_prior;	/* running list	             	*/
	struct thread	*t_next;	/* running/wait list		*/
	struct thread	*t_synch;	/* event list for threads waiting
					   for this thread suspension	*/
	unsigned long	t_dispct;	/* number of dispatches		*/
	unsigned long	t_fpuct;	/* number of FP unavail ints.	*/

	/* scheduler fields */
	cpu_t		t_cpuid;	/* processor on which I'm bound */
	cpu_t		t_scpuid;	/* saved last t_cpuid for funnel*/
	cpu_t		t_affinity;	/* processor on which I last ran*/
	char		t_pri;		/* current effective priority   */
	char		t_policy;	/* scheduling policy		*/
	u_short		t_cpu;		/* processor usage		*/
                                        /* NOTE: the bounds for t_cpu   */
                                        /* 0 <= t_cpu <= T_CPU_MAX      */
#define T_CPU_MAX       20+HZ           /* maximum value for t_cpu      */
        char            t_lockpri;      /* lock priority boost count 	*/
        char            t_wakepri;      /* wakeup priority for the thread */
                                        /* NOTE: this field is set when   */
                                        /* the thread goes to sleep, and  */
                                        /* is reset to PIDL by 'setrq'.   */ 
	unsigned char   t_time;  	/* resident time for scheduling */
	char		t_sav_pri;	/* Original, unboosted priority */
	char		t_pad;		/* padding 			*/

	/* signal information */
	char		t_cursig;	/* current/last signal taken    */
	sigset_t	t_sig;		/* pending signals		*/
	sigset_t	t_sigmask;	/* current signal mask		*/
	struct sigcontext *t_scp;	/* sigctx location in user space*/

	/* identifier fields */
	tid_t		t_tid;		/* unique thread identifier	*/

	/* miscellaneous fields */
	void		*t_graphics;    /* graphics user address state  */
        struct tstate   *t_cancel;      /* asynchronous cancelation     */

        struct thread   *t_lockowner;   /* thread to be boosted         */
        int             t_boosted;      /* boosted count                */

	tid_t		t_tsleep;	/* event list for thread_tsleep	*/
	int		t_userdata;	/* user-owned data		*/

	long		t_extra[4];	/* padding to 32 byte boundary  */
};

#define t_link		t_procp		/* any field not used when free */

#define t_uthreadp	t_uaddress.uthreadp
#define t_userp		t_uaddress.userp

#define t_prevthread	t_threadlist.prevthread
#define t_nextthread	t_threadlist.nextthread

/*
 * thread flags, t_flags (must be different from the t_atomic flags)
 *
 * This field can be updated only if the caller is disabled to INTMAX
 * and holds the proc_int_lock. 
 */
#define TTERM		0x00000001	/* thread should be terminated	    */
#define TSUSP		0x00000002	/* thread should be suspended	    */
#define TSIGAVAIL	0x00000004	/* signal available for delivery    */
#define TINTR		0x00000008	/* conditional suspension 	    */  
#define TLOCAL		0x00000010	/* does not account for SIGWAITING  */
#define TASSERTSIG	0x00000020	/* assert wait signal		    */
#define	TTRCSIG		0x00000040	/* thread identification for ptrace */
#define TOMASK		0x00000100	/* restore old mask after signal    */
#define	TWAKEONSIG	0x00000400	/* signal will abort the sleep	    */
#define SWAKEONSIG	0x00000800	/* does not really belong here, but */
					/* was used as a p_flag and in	    */
					/* xxx_sleep interfaces, so must    */
					/* still exist with the same value  */
					/* Must NOT be equal to PCATCH!     */
					/* Both PCATCH and SWAKEONSIG must  */
					/* be greater than PMASK            */
#define TKTHREAD	0x00001000	/* kernel thread		    */
#define TFUNNELLED	0x00002000	/* thread is funnelled		    */
#define TSETSTATE	0x00004000	/* a state change is in progress    */
#define TPRIOBOOST	0x00010000	/* priority was boosted (NOT USED)  */	
#define TSIGWAIT        0x00020000      /* tsleeping with unblocked signals */
#define TPMREQSTOP      0x00040000      /* thread is requested to pm stop   */
#define TPMSTOP         0x00080000      /* thread has been pm stopped       */
#define TBOOSTING	0x00100000	/* thread is pri boosting another   */	

/* 
 * flags triggering call to sig_slih:
 *
 * When changing these flags, one must also change thread.m4.  Moreover,
 * the bits must be consecutive to work with rlinm.
 */
#define TSIGSLIH	(TTERM|TSUSP|TSIGAVAIL)	
#define TSIGINTR	(TSIGSLIH|TINTR)	

/*
 * thread flags, t_atomic (must be different from the t_flags flags)
 *
 * This field should be updated only with an atomic primitive. 
 */
#define TSIGDELIVERY	0x00000080	/* setup of signal delivery underway*/
#define TSELECT		0x00008000	/* selecting: wakeup/waiting danger */

/*
 * thread states, t_state
 */
#define TSNONE		0		/* slot is available    */
#define TSIDL		1		/* being created	*/
#define TSRUN		2		/* runnable		*/
#define TSSLEEP		3		/* awaiting an event    */
#define TSSWAP		4		/* swapped		*/
#define TSSTOP		5		/* stopped		*/
#define TSZOMB		6		/* being deleted	*/

/*
 * types of thread waits, t_wtype 
 */
#define TNOWAIT		0
#define TWEVENT		1		/* waiting for event(s) 	    */
#define TWLOCK		2		/* waiting for serialization lock W */
#define TWTIMER		3		/* waiting for timer 		    */
#define TWCPU		4		/* waiting for CPU (in ready queue) */
#define TWPGIN		5		/* waiting for page in 		    */
#define TWPGOUT		6		/* waiting for page out level 	    */
#define TWPLOCK		7		/* waiting for physical lock 	    */
#define TWFREEF		8       	/* waiting for a free page frame    */
#define TWMEM		9		/* waiting for memory 		    */
#define TWLOCKREAD	10		/* waiting for serialization lock R */
#define TWUEXCEPT	11		/* waiting for user exception	    */

/*
 * Defines and macros for use with thread table entries
 */
#define TIDRESERVED	6		/* number of bits reserved in tid */
#define THREADSHIFT	18		/* number of bits in thread index */
#define TGENSHIFT	8		/* thread index offset		  */
#define TGENMASK	((1<<TGENSHIFT)-1)
#define NTHREAD		(1<<THREADSHIFT)
#define TIDMASK		((NTHREAD-1)<<TGENSHIFT)

/* tid are odd, but 1 is a pid too */
#define MAYBE_TID(tid)	(tid & 1)

/* the null tid */
#define NULL_TID	0

/* swapper's tid (may be any odd number less than 1<<TGENSHIFT) */
#define SWAPPER_TID	3		/* 1 may be confused with init's PID */

/* mask for thread index		*/
#define THREADMASK(tid)	(((tid)&TIDMASK)>>TGENSHIFT)

#ifdef _KERNEL
extern struct thread thread[NTHREAD];	/* the thread table, itself      */

/* convert tid to thread pointer	*/
#define THREADPTR(tid)	(&thread[THREADMASK(tid)])

/* validate whether a thread id is valid or not  */
#define	VALIDATE_TID(tid)						\
	(((THREADPTR(tid) >= (struct thread *)v.ve_thread) ||		\
	  (THREADPTR(tid)->t_tid != (tid))) ?				\
					NULL : THREADPTR(tid))
#endif

/*
 * Constants used by user locks
 */
#define UL_FREE		0x00000000
#define UL_BUSY		0x10000000
#define UL_WAIT		0x20000000

/*
 * constants and structures used by thread_setstate()
 */
#define TSTATE_LOCAL	0x00000001
#define TSTATE_INTR	0x00000002

struct tstate {
	struct mstsave	mst;
	int		**errnop_addr;
	sigset_t	sigmask;
	sigset_t	psig;
	int		policy;
	int		priority;
	int		flags;
	int		flagmask;
	int		userdata;
	int		pad[6];
};

struct func_desc {
	char *entry_point;
	char *toc_ptr;
	char *data_ptr;
};

extern int errno;
extern int *errnop;

/*
 * thread-related routines
 */
#ifdef _NO_PROTO
tid_t	thread_create();
int	thread_kill();
void	kthread_kill();
tid_t	thread_self();
int	thread_setsched();
int	thread_setstate();
int	kthread_start();
void	thread_terminate();
int	thread_terminate_ack();
int	thread_tsleep();
int	thread_twakeup();
int	thread_unlock();
int	thread_userdata();
int	thread_waitlock();
#else /* _NO_PROTO */
tid_t	thread_create();
int	thread_kill(tid_t, int);
void	kthread_kill(tid_t, int);
tid_t	thread_self();
int	thread_setsched(tid_t, int, int, int);
int	thread_setstate(tid_t, struct tstate *, struct tstate *);
int	kthread_start(tid_t, void (*)(void *), void *,size_t,void *,sigset_t *);
void	thread_terminate();
int	thread_terminate_ack(tid_t);
int     thread_tsleep(int, atomic_p, const sigset_t *);
int	thread_twakeup(tid_t, int);
int	thread_unlock(atomic_p);
int	thread_userdata();
int	thread_waitlock(atomic_p);
#endif /* _NO_PROTO */

#endif  /* _H_THREAD */

