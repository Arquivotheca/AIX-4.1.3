/* static char sccsid[] = "@(#)09	1.35  src/bos/usr/ccs/lib/libpthreads/internal.h, libpth, bos41J, 9515A_all 4/4/95 11:12:12"; */
/*
 * COMPONENT_NAME: libpth
 * 
 * FUNCTIONS:
 * 
 * ORIGINS:  71, 83
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.2
 */

/*
 * file: internal.h
 */

/* -----------------------------------------------------------------------------
 * Internal definitions for the pthreads package.
 */


/* -----------------------------------------------------------------------------
 * Name space conversion
 */
#define	__ptlock_type			spinlock_t

#define	__pt_attr			pthread_attr

#define	__ptq_queue			pthread_queue
#define __ptq_next			next
#define __ptq_prev			prev

#define	__ptmtx_lock			lock
#define	__ptmtx_flags			flags
#define	__ptmtx_owner			owner

#define	__ptcv_lock			lock
#define	__ptcv_flags			flags
#define	__ptcv_waiters			waiters

#define	__pthread_cleanup_handler_t	pthread_cleanup_handler_t

#define	__ptonce_lock			lock
#define	__ptonce_initialized		initialized
#define	__ptonce_executing		executing
#define	__ptonce_completed		completed
#define	__ptonce_mutex			mutex
#define	__ptonce_executed		executed

#include "pthread.h"

#include <setjmp.h>
#include <sys/trchkid.h>

#ifndef _TRACE
#undef TRCHKGT
#define TRCHKGT(hw,a,b,c,d,e)
#endif

pid_t pid;
typedef struct __dbx_mutex {
	struct __dbx_mutex	*next;
	struct __dbx_mutex	*prev;
	pthread_mutex_t		*pt_mutex;
} __dbx_mutex;
typedef struct __dbx_cond {
	struct __dbx_cond	*next;
	struct __dbx_cond	*prev;
	pthread_cond_t		*pt_cond;
} __dbx_cond;

/* -----------------------------------------------------------------------------
 * General Macros
 */

#ifndef NULL
#define	NULL		(void *)0
#endif

typedef void	*(*pthread_func_t)(void *);
typedef void	(*pthread_push_t)(void *);

#define	NIL(t)		((t)0)
#define	NILFUNC(t)	NIL(t(*)())


/* -----------------------------------------------------------------------------
 * Queue Macros
 *
 * Queues are doubly linked lists.
 */
#define queue_init(queue)	((queue)->next = (queue)->prev = (queue))
#define queue_head(queue)	((queue)->next)
#define queue_tail(queue)	((queue)->prev)
#define queue_end(queue)	(queue)
#define queue_next(queue)	((queue)->next)
#define queue_prev(queue)	((queue)->prev)
#define queue_empty(queue)	((queue)->next == (queue))

/* Make element first in queue.
 */
#define	queue_insert(queue, element) \
		(element)->next = (queue)->next; \
		(element)->prev = (queue); \
		(queue)->next->prev = (element); \
		(queue)->next = (element); \

/* Make element last in queue.
 */
#define queue_append(queue, element) \
		(element)->next = (queue); \
		(element)->prev = (queue)->prev; \
		(queue)->prev->next = (element); \
		(queue)->prev = (element); \

/* Remove element from queue.
 */
#define	queue_remove(element) \
		(element)->prev->next = (element)->next; \
		(element)->next->prev = (element)->prev; \

/* Replace toQueue with fromQueue.
 */
#define queue_move(toQueue, fromQueue) \
 		queue_append(fromQueue, toQueue); \
 		queue_remove(fromQueue); \
 		queue_init(fromQueue); \

/* Append fromQueue content to toQueue.
 */
#define	queue_merge(toQueue, fromQueue) \
		(fromQueue)->prev->next = (toQueue); \
		(fromQueue)->next->prev = (toQueue)->prev; \
		(toQueue)->prev->next = (fromQueue)->next; \
		(toQueue)->prev = (fromQueue)->prev; \
		queue_init(fromQueue); \

/* Truncate fromQueue at atElement and move atElement to toQueue.
 */
#define	queue_split(fromQueue, toQueue, atElement) \
		(toQueue)->prev = (fromQueue)->prev; \
		(toQueue)->next = (atElement); \
		(fromQueue)->prev = (atElement)->prev; \
		(fromQueue)->prev->next = (fromQueue); \
		(atElement)->prev = (toQueue); \
		(toQueue)->prev->next = (toQueue); \


/* -----------------------------------------------------------------------------
 * Pthread internal definitions
 */

/* -----------------------------------------------------------------------------
 * Cancellation
 */

struct pthread_intr_state {
	unsigned int	state:1,	/* general cancel mode */
			mode:1,		/* async cancel mode */
			pending:1;	/* pending cancel */
};

typedef struct pthread_intr_state	pthread_intr_t;


/* -----------------------------------------------------------------------------
 * Pthreads
 */

/* mutex flags and handles
 */
#define	MUTEX_VALID		0x01
#define	MUTEXATTR_VALID		0x01
#define	NO_MUTEX		((pthread_mutex_t *)0)
#define	NO_MUTEX_ATTRIBUTE	((pthread_mutexattr_t)0)

/* condition variable flags and handles
 */
#define	COND_VALID		0x01
#define	CONDATTR_VALID		0x01
#define	NO_COND			((pthread_cond_t *)0)
#define	NO_COND_ATTRIBUTE	((pthread_condattr_t)0)

/* pthread flags and handles
 */
#define	PTHREAD_INITIAL_THREAD	0x01
#define	ATTRIBUTE_VALID		0x01
#define BAD_PTHREAD_ID		((pthread_t)0)
#define NO_PTHREAD		((pthread_d)0)
#define	NO_ATTRIBUTE		((pthread_attr_t)0)

/* pthread state
 */
enum pthread_state {
	PTHREAD_DETACHED =	0x01,	/* detached */
	PTHREAD_RETURNED =	0x02,	/* terminated */
	PTHREAD_INACTIVE =	0x04,	/* deactivated */
	PTHREAD_ABOUT_TO_WAIT =	0x08,	/* deactivated but not yet waiting */
	PTHREAD_AWOKEN =	0x10,	/* event arrived before wait */
	PTHREAD_WAITING =	0x20,	/* waiting for an event */
	PTHREAD_EVENT =		0x40,	/* event has been sent */
	PTHREAD_EXITED =	0x80 	/* exited */
};

struct atfork {
	pthread_queue	link;
	void    (*prepare)();
	void    (*parent)();
	void    (*child)();
	int	flag;
	void*	userstate;
};

struct pthread_attr {
	pthread_queue	link;
	long		flags;
	size_t		stacksize;
	size_t		cancel_stacksize;
	int 		type;
	int		attr_id;
	int		detachstate;
	int 		process_shared;
	int 		contentionscope;
        struct  sched_param     schedule;
	int 		inherit;
	int		protocol;
	int 		prio_ceiling;
	int 		mutex_kind;
};


struct pthread {
	pthread_queue		link;		/* current q */
	pthread_queue           DBXlink;
	struct pthread		*all_thread_link;
        pthread_queue           sigwaiter_link; /* List of sigwaiters */
	unsigned int		flags;
	unsigned int		state;
	struct vp		*vp;		/* bound vp */
	spinlock_t		lock;		/* guard this struct */
	unsigned int		join_count;	/* no of joiners */
	pthread_cond_t		done;		/* q of joiners */
	pthread_func_t		func;		/* start func */
	void			*arg;		/* arg to start func */
	void			*returned;	/* exit value of pthread */
	pthread_attr_t		attr;		/* attributes */
	pthread_intr_t		intr;		/* cancel status */
	int			event;		/* current event */
        struct {
                sigset_t        sigwait_set;    /* sigwait signals */
                sigset_t        pkill_set;      /* pthread kill posted to thd */
                sigset_t        siggot;
        } sig_data;

	struct specific_data	*specific_data;	/* base of per-thread data */
	pthread_cleanup_handler_t *cleanup_queue; /* top of handler stack */
	pthread_cleanup_handler_t *handler_pool; /* free handler  */
						/* for DBX kernel informations*/
	int			th_id;		/* identification number */
	tid_t			ti_tid;		/* thread_tid */
	unsigned long		ti_policy;	/* scheduling policy */
	unsigned long		ti_pri;		/* priority */
	unsigned long		ti_stat;	/* thread_state */
	unsigned long		ti_flags;	/* thread mode */
	unsigned long		ti_scount;	/* suspend count */
	unsigned long		ti_cursig;	/* signal management */
        unsigned long           ti_wtype;       /* type of thread wait */
        unsigned long           ti_wchan;       /*  wait channel       */
	unsigned long		ti_hold;
	int             	*thread_errno;
	pthread_cond_t		*cond_cancel;	/* used in 		*/
	pthread_mutex_t		*mutex_cancel;	/* pthread_cleanup_unwind */
	int			join_cancel;
};

typedef struct pthread	*pthread_d;

#define	pthread_id_lookup(p)	((pthread_d)(p))
#define	pthread_id_find(p)	((pthread_t)(p))
#define	pthread_id_create(p)	((pthread_t)(p))

extern void
_pthread_deactivate(pthread_t, pthread_queue *);

extern void
_pthread_activate(pthread_t);

extern void
_pthread_event_wait(pthread_t, int *, int, const struct timespec *);

extern int
_pthread_event_notify(pthread_t, int);

#define	INTERNAL_ERROR(func) \
	_pthread_internal_error(__FILE__, (func), __LINE__)

#define	LAST_INTERNAL_ERROR(func) \
	_last_internal_error(__FILE__, (func), __LINE__)

extern void
_pthread_internal_error(char *, char *, int);

extern void
_last_internal_error(char *, char *, int);

extern void *
_pmalloc(register size_t);

extern void
_pfree(void *);

/* -----------------------------------------------------------------------------
 * Stack database
 */

struct stk {
	pthread_queue	link;
	unsigned long	base;	/* lowest addressable word */
	unsigned long	size;	/* bytes in stack */
	unsigned long	limit;	/* highest addressable word */
	struct vp	*vp;	/* bound vp */
};

typedef struct stk	*stk_t;

/* -----------------------------------------------------------------------------
 * Definitions for the Virtual Processor (vp) layer
 */

/* vp flags and handles
 */
#define	VP_INITIAL_STACK	0x01
#define VP_STARTED		0x02
#define	NO_VP			((vp_t)0)


struct	vp {
	pthread_queue	link;		/* list of vps */
	unsigned int	flags;
	pthread_d	pthread;	/* bound pthread */
	tid_t		id;		/* kthread id */
	pthread_func_t	async_func;	/* func to call following async intr */
	void		*async_arg;	/* arg to async func */
	struct stk	stack;		/* bound stack */
	jmp_buf		exit_jmp;	/* jump back from pthread exit */
	sigset_t	mask;		/* signal mask of the creator */
	size_t		cancel_stack_size; /* size of the cancel stack */
	unsigned long	specific_data_address;	/* address of specific data */
};

typedef struct vp	*vp_t;

extern vp_t
_vp_create(const pthread_attr_t *);

extern void
_vp_bind(vp_t, pthread_d);

extern void
_vp_suspend(vp_t);

extern void
_vp_resume(vp_t);

extern void
_vp_setup(vp_t);

extern void
_vp_call(vp_t, pthread_func_t, void *);

extern void
_vp_call_setup(vp_t);

extern void
_vp_prune(void);


/* -----------------------------------------------------------------------------
 * vp events
 */

extern void
_event_wait(spinlock_t *, int *, const struct timespec *);

extern void
_event_waitabs(spinlock_t *, int *, const struct timespec *);

#define	NO_TIMEOUT	((struct timespec *)0)

enum event_id {
	EVT_NONE =	0x00,
	EVT_TIMEOUT =	0x5001,	/* wait timed out */
	EVT_RESUME =	0x5002,	/* restart cached vp */
	EVT_SIGNAL =	0x5003,	/* condition variable signal */
	EVT_CANCEL =	0x5004,	/* cancel request */
	EVT_SIGWAIT =	0x5005,	/* sigwait caller waiting */
	EVT_SIGRESUME =	0x5006,	/* sigwait caller resumed */
	EVT_SIGPOST =	0x6000	/* UNIX signal delivered */
};


/* -----------------------------------------------------------------------------
 * Stack information
 */

extern int
_alloc_stack(vp_t, size_t);

extern void
_dealloc_stack(vp_t);

extern caddr_t
_get_stack_pointer(void);


/* -----------------------------------------------------------------------------
 * Spin locks
 */

extern void
_spin_lock(spinlock_t *);

extern int
_spin_trylock(spinlock_t *);

extern void
_spin_unlock(spinlock_t *);

extern void
_spinlock_create(spinlock_t *);

extern void
_spinlock_delete(spinlock_t *);

#undef _spin_lock
#undef _spin_unlock
#undef _spinlock_create
#undef _spinlock_delete

#define _spinlock_create(lock)	_clear_lock((atomic_p)lock, UL_FREE)
#define _spinlock_delete(lock)	_clear_lock((atomic_p)lock, UL_FREE)

#define _get_lock(lock) \
	(!_check_lock((atomic_p)lock, UL_FREE, UL_BUSY) || \
	 !_check_lock((atomic_p)lock, UL_WAIT, UL_WAIT|UL_BUSY))

#define _put_lock(lock) \
	(NBCPU > 1 ? __iospace_sync() : (void)0, \
	 !cs((atomic_p)lock, UL_BUSY, UL_FREE))

#ifndef _TRACE

#define _spin_lock(lock)     {if(!_get_lock(lock))_spin_lock(lock);}
#define _spin_unlock(lock)   {int dummy;if(!_put_lock(lock))_spin_unlock(lock);}

#else /* _TRACE */

extern char *get_stkp(void);
#pragma mc_func get_stkp	{ "7C230B78" }  /* mr r3,r1 */
#pragma reg_killed_by get_stkp	gr3

struct stack_frame {
	struct stack_frame *next;
	int		   unused;
	int		   link_register;
};

#define CALLER (((struct stack_frame *)get_stkp())->next->link_register)

#define hkwd_LOCK_TAKEN		1
#define hkwd_LOCK_MISS		2
#define LOCK_SWRITE_TRACE	1

#define _spin_lock(lock) \
	{if (!_get_lock(lock)) \
		_spin_lock(lock); \
	else \
		TRCHKGT(HKWD_KERN_LOCK|hkwd_LOCK_TAKEN, \
				lock, *lock, LOCK_SWRITE_TRACE, CALLER, NULL);}

extern void _spin_unlock_trace(spinlock_t *);
#define _spin_unlock(lock) \
	{int dummy; \
	TRCHKGT(HKWD_KERN_UNLOCK, lock, *lock, CALLER, NULL, 0); \
	if (!_put_lock(lock)) \
		_spin_unlock_trace(lock);}

#undef thread_tsleep
#define thread_tsleep(time,lock,mask) \
	(lock ? \
	TRCHKGT(HKWD_KERN_UNLOCK, lock, *(int *)lock, CALLER, NULL, 0):(void)0,\
	thread_tsleep(time,lock,mask))

#endif /* _TRACE */

/* -----------------------------------------------------------------------------
 * Initialization Functions
 */

extern vp_t
_pthread_vp_startup(void);

extern void
_pthread_mutex_startup(void);

extern int
_initialize_mutex(pthread_mutex_t *, pthread_mutexattr_t);

extern void
_pthread_cond_startup(void);

extern int
_initialize_condition(pthread_cond_t *, pthread_condattr_t);

extern void
_pthread_attr_startup(void);

extern void
_pthread_stack_startup(vp_t);

extern void
_pthread_stackdb_startup(void);

extern void
_specific_data_startup(void);

extern void
_pthread_sigwait_startup(void);

extern void
_pthread_fork_startup(void);

extern int
_pthread_specific_startup(void);

extern void
_pthread_libs_init(pthread_d);


/* -----------------------------------------------------------------------------
 * Definitions for stack
 */

extern size_t	_pthread_default_stack_size;


/* -----------------------------------------------------------------------------
 * Thread specific data
 */

struct specific_key {
	long	flags;
	void	(*destructor)(void *);
};

typedef struct specific_key	specific_key_t;

#define	KEY_FREE	0x00
#define	KEY_ALLOCATED	0x01

#define	SPECIFIC_DATA_SET	0x01

extern void
_specific_data_setup(pthread_d);

extern void
_specific_data_setup_initial(pthread_d);

extern void
_specific_data_cleanup(pthread_d);

extern int
_pthread_getspecific_addr(pthread_key_t, void **);

/* -----------------------------------------------------------------------------
 * Tuning
 *
 * These values are declared here for visibility.
 */

/* Stacks
 *
 * Stacks contain per-thread data and a protected page.
 */
#define	FIRST_STACK_BASE	0		/* allocate from here up */
#define	PTHREAD_DATA		__page_size	/* per-thread data */
#define	RED_ZONE_SIZE		__page_size	/* protected */
#define	K_RED_ZONE_SIZE		__page_size_K	/* same value as above in k */

/* Thread Specific Data
 */
#define	KEYTABLE_SIZE		(sizeof(specific_key_t) * APTHREAD_DATAKEYS_MAX)

/* Locks
 *
 * Locks spin in a retry loop before falling back to a yield-retry loop.
 */
int	NBCPU;
#define YIELDTIME	40
int	YIELDLOOPTIME;		/* UP =0 ; MP =40 upate in pthread_init() */
int	TICKLOOPTIME;
				

/* VPs
 *
 * vp hash table is ordered by stack size.
 */
#define	VP_HASH_MAX		64			/* buckets in table */
#define	VP_HASH_RANGE		(__page_sizeX24)	/* size of buckets */

/* vps are cached up to a high water and then freed down to a low water.
 */
#define	VP_CACHE_LOW(max)	(max / 4)
#define	VP_CACHE_HIGH(max)	((2 * max) / 3)

#define pthread_self() \
        pthread_id_find(((vp_t)thread_userdata())->pthread)

/* -----------------------------------------------------------------------------
 * Debugging
 */

#ifdef	DEBUG_PTH

#ifndef private
#define	private
#endif

extern int	pthread_trace;

extern void
logPrintf(const char *, ...);

extern void
dbgPrintf(const char *, ...);

extern void
dump_pthread(void);

extern void
dump_vp(void);

extern void
dump_mutex(pthread_mutex_t *);

extern void
dump_cond(pthread_cond_t *);

#define PT_LOG(msg)	if (pthread_trace) logPrintf msg 
#define PT_DBG(msg)	dbgPrintf msg 

#else	/* DEBUG_PTH */

#ifndef private
#define	private	static
#endif

#define PT_LOG(msg)
#define PT_DBG(msg)

#endif	/* DEBUG_PTH */

