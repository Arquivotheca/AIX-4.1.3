/* static char sccsid[] = "@(#)15	1.31  src/bos/usr/ccs/lib/libpthreads/pthread.h, libpth, bos41J, 9515A_all 3/31/95 08:34:57"; */
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
 * file: pthread.h
 */

/* -----------------------------------------------------------------------------
 * Definitions for the Pthreads package.
 */


#ifndef	_PTHREAD_H_
#define	_PTHREAD_H_

#include <standards.h>

#define _THREAD_SAFE      1

#include <unistd.h>			/* for SC_PAGE_SIZE */
#include <sys/types.h>
#include <sys/timers.h>
#include <errno.h>
#include <sys/sched.h>

#define	MUTEX_FAST_NP		2 	/* for DCE support */
#define	MUTEX_RECURSIVE_NP	1
#define	MUTEX_NONRECURSIVE_NP	0	/* Default */
#define PTHREAD_CANCELED	-1

                                                     /* Provisoire */
/* struct sched_param should be in <sched.h> */

struct sched_param {
	int sched_priority;
	int sched_policy;
	int sched_reserved[6];
};

int __page_size;
int __page_size_K;
int __page_sizeX16;
int __page_sizeX24;
int __page_sizeM1;

/*
 * 	POSIX CHOICES of LIBPTHREADS	
 *	(must be after <unistd.h>)
*/
#ifndef _POSIX_REENTRANT_FUNCTIONS
#define _POSIX_REENTRANT_FUNCTIONS
#endif
#ifndef _POSIX_THREADS
#define _POSIX_THREADS
#endif
#ifdef  _POSIX_THREAD_ATTR_STACKADDR
#undef	_POSIX_THREAD_ATTR_STACKADDR
#endif
#ifndef _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_ATTR_STACKSIZE
#endif
#ifdef  _POSIX_THREAD_FORKALL
#undef	_POSIX_THREAD_FORKALL
#endif
#ifndef _POSIX_THREAD_PRIORITY_SCHEDULING
#define	_POSIX_THREAD_PRIORITY_SCHEDULING
#endif
#ifdef  _POSIX_THREAD_PRIO_INHERIT
#undef  _POSIX_THREAD_PRIO_INHERIT
#endif
#ifdef  _POSIX_THREAD_PRIO_PROTECT
#undef  _POSIX_THREAD_PRIO_PROTECT
#endif
#ifdef  _POSIX_THREAD_PROCESS_SHARED
#undef  _POSIX_THREAD_PROCESS_SHARED
#endif

/* scheduling
*/
#define DEFAULT_SCHED			SCHED_OTHER

#define	PTHREAD_PRIO_MAX		127
#define PTHREAD_PRIO_MIN		1
#define DEFAULT_PRIO			PTHREAD_PRIO_MIN	

#define PTHREAD_INHERIT_SCHED		0
#define PTHREAD_EXPLICIT_SCHED		1
#define DEFAULT_INHERIT			PTHREAD_INHERIT_SCHED

#define PTHREAD_SCOPE_SYSTEM		0
#define PTHREAD_SCOPE_PROCESS		1
#define PTHREAD_SCOPE_GLOBAL		PTHREAD_SCOPE_SYSTEM
#define PTHREAD_SCOPE_LOCAL		PTHREAD_SCOPE_PROCESS
#define DEFAULT_SCOPE			PTHREAD_SCOPE_LOCAL

/* detach state
*/
#define PTHREAD_CREATE_DETACHED         1
#define PTHREAD_CREATE_UNDETACHED       0
#define DEFAULT_DETACHSTATE             PTHREAD_CREATE_DETACHED

/* cancelability 
*/
#define PTHREAD_CANCEL_DISABLE          0       /* general cancellation off */
#define PTHREAD_CANCEL_ENABLE           1       /* general cancellation on */
#define PTHREAD_CANCEL_DEFERRED         0       /* async cancellation off */
#define PTHREAD_CANCEL_ASYNCHRONOUS     1       /* async cancellation on */

/* specific-data
*/
#define	PTHREAD_SPECIFIC_DATA		__page_size	/* per-thread data */
#define	APTHREAD_DATAKEYS_MAX		(PTHREAD_SPECIFIC_DATA / sizeof(specific_data_t))
#ifndef PTHREAD_DATAKEYS_MAX
#define PTHREAD_DATAKEYS_MAX            (APTHREAD_DATAKEYS_MAX - 4)
#endif
/* some limits
*/
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN		(__page_sizeX24)
#endif

#ifdef PTHREAD_THREADS_MAX
#undef PTHREAD_THREADS_MAX
#endif
#ifndef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX		512	/* per-segment */
#endif

extern	int
sigthreadmask(int how, const sigset_t *set, sigset_t *oset);

/* -----------------------------------------------------------------------------
 * Machine Dependent definitions for the Pthreads Package.
 */

#ifndef	_MACHINE_PTHREAD_H_
#define	_MACHINE_PTHREAD_H_

typedef volatile int	__ptlock_type;

#endif	/*_MACHINE_PTHREAD_H_*/


#ifdef _NO_PROTO
#define __(args)        ()
#else /* _NO_PROTO */
#define __(args)        args
#endif /* _NO_PROTO */


/* -----------------------------------------------------------------------------
 * Pthread Attributes
 */

typedef struct __pt_attr	*pthread_attr_t;

extern pthread_attr_t	pthread_attr_default;

extern int
pthread_attr_init __((pthread_attr_t *));

extern int
pthread_attr_destroy __((pthread_attr_t *));

extern int
pthread_attr_setstacksize __((pthread_attr_t *, size_t));

extern int
pthread_attr_getstacksize __((const pthread_attr_t *, size_t *));

extern int
pthread_attr_setstackaddr __((pthread_attr_t *, void *));

extern int
pthread_attr_getstackaddr __((const pthread_attr_t *, void **));

extern int
pthread_attr_setschedpolicy __((pthread_attr_t *, int));

extern int
pthread_attr_getschedpolicy __((const pthread_attr_t *, int *));

extern int
pthread_attr_setschedparam __((pthread_attr_t *, const struct sched_param *));

extern int
pthread_attr_getschedparam __((const pthread_attr_t *, 
					struct sched_param *));

extern int
pthread_attr_setinheritsched __((pthread_attr_t *, int));

extern int
pthread_attr_getinheritsched __((const pthread_attr_t *, int *));

extern int
pthread_attr_setdetachstate __((pthread_attr_t *, int));

extern int
pthread_attr_getdetachstate __((const pthread_attr_t *, int *));

/* -----------------------------------------------------------------------------
 * Functions for fork()
 */

extern int
pthread_atfork(void (*)(void), void (*)(void), void (*)(void));

extern int
forkall(pid_t *);

/* -----------------------------------------------------------------------------
 * Pthreads
 */

typedef void	*pthread_t;

extern int
pthread_setschedparam __((pthread_t, int , const struct sched_param *));

extern int
pthread_getschedparam __((pthread_t, int *, struct sched_param *));

extern pthread_t
pthread_self __((void));

extern int
pthread_create __((pthread_t *, const pthread_attr_t *, void *(*)(void *), void *));

extern int
pthread_detach __((pthread_t));

extern int
pthread_join __((pthread_t, void **));

extern void
pthread_exit __((void *));

extern void
pthread_yield __((void));

extern void
pthread_cleanup_push __((void (*)(void *), void *));

extern void
pthread_cleanup_pop __((int));

extern int
pthread_cancel __((pthread_t));

extern int
pthread_attr_setscope __((pthread_attr_t *, int));

extern int
pthread_attr_getscope __((const pthread_attr_t *, int *));

extern int
pthread_equal __((pthread_t, pthread_t));

extern int
pthread_kill __((pthread_t, int));

#define	pthread_equal(t1, t2)	((t1) == (t2))


/* -----------------------------------------------------------------------------
 * Pthread Queue Structures
 */

typedef struct __ptq_queue {
	struct __ptq_queue	*__ptq_next;
	struct __ptq_queue	*__ptq_prev;
} __ptq_queue;

/* -----------------------------------------------------------------------------
 * Mutex Attributes
 */

typedef	struct __pt_attr		*pthread_mutexattr_t;

extern pthread_mutexattr_t		pthread_mutexattr_default;

extern int
pthread_mutexattr_init __((pthread_mutexattr_t *));

extern int
pthread_mutexattr_destroy __((pthread_mutexattr_t *));

extern int
pthread_mutexattr_setprotocol __((pthread_mutexattr_t *, int));

extern int
pthread_mutexattr_getprotocol __((const pthread_mutexattr_t *, int *));

extern int
pthread_mutexattr_setprioceiling __((pthread_mutexattr_t *, int));

extern int
pthread_mutexattr_getprioceiling __((const pthread_mutexattr_t *, int *));

extern int
pthread_mutexattr_getpshared __((const pthread_mutexattr_t *, int *));

extern int
pthread_mutexattr_setpshared __((const pthread_mutexattr_t *, int));

/* -----------------------------------------------------------------------------
 * Mutexes
 */

typedef struct {
	__ptq_queue	link;
	__ptlock_type	__ptmtx_lock;
	long		__ptmtx_flags;
#ifdef _POSIX_THREAD_PRIO_INHERIT
	int			protocol;
#ifdef _POSIX_THREAD_PRIO_PROTECT
	int			prioceiling;
#endif
#endif
	pthread_t	__ptmtx_owner;
	int		mtx_id;
	pthread_attr_t	attr; 		/* attributes pointer*/
	int		mtx_kind;	/* kind of the mutex */
	int		lock_cpt;	/* number of recursive locks */
	int		reserved[4];
} pthread_mutex_t;

extern int
pthread_mutex_init __((pthread_mutex_t *, pthread_mutexattr_t *));

extern int
pthread_mutex_destroy __((pthread_mutex_t *));

extern int
pthread_mutex_lock __((pthread_mutex_t *));

extern int
pthread_mutex_trylock __((pthread_mutex_t *));

extern int
pthread_mutex_unlock __((pthread_mutex_t *));

extern int
pthread_mutex_setprioceiling __((pthread_mutex_t *, int, int *));

extern int
pthread_mutex_getprioceiling __((pthread_mutex_t *, int *));

#define	pthread_mutex_getowner_np(mutex)	((mutex)->__ptmtx_owner)

#define MUTEX_INITSTATIC 2
#define PTHREAD_MUTEX_INITIALIZER {0, 0, 0, MUTEX_INITSTATIC, 0, 0, 0}

/* -----------------------------------------------------------------------------
 * Condition Variable Attributes
 */

typedef	struct __pt_attr		*pthread_condattr_t;

extern pthread_condattr_t		pthread_condattr_default;

extern int
pthread_condattr_init __((pthread_condattr_t *));

extern int
pthread_condattr_destroy __((pthread_condattr_t *));

extern int
pthread_condattr_getpshared __((const pthread_condattr_t *, int *));

extern int
pthread_condattr_setpshared __((const pthread_condattr_t *, int));

#define COND_INITSTATIC 2
#define PTHREAD_COND_INITIALIZER {0, 0, 0, COND_INITSTATIC, 0, 0, 0, 0}

/* -----------------------------------------------------------------------------
 * Condition Variables
 */

typedef struct {
	__ptq_queue	link;
	__ptlock_type	__ptcv_lock;
	long		__ptcv_flags;
	__ptq_queue	__ptcv_waiters;
	int 		cv_id;
	pthread_attr_t	attr; 		/* attributes pointer*/
	pthread_mutex_t	*mutex;		/* mutex attachs to condition variable*/
	int		cptwait;	/* number blocks on the condition */
	int		reserved;
	
} pthread_cond_t;

extern int
pthread_cond_init __((pthread_cond_t *, pthread_condattr_t *));

extern int
pthread_cond_destroy __((pthread_cond_t *));

extern int
pthread_cond_wait __((pthread_cond_t *, pthread_mutex_t *));

extern int
pthread_cond_timedwait __((pthread_cond_t *, pthread_mutex_t *,
			   const struct timespec *));

extern int
pthread_cond_signal __((pthread_cond_t *));

extern int
pthread_cond_broadcast __((pthread_cond_t *));


/* -----------------------------------------------------------------------------
 * Thread Specific Data
 */
struct specific_data {
	long	flags;
	const	void	*value;
};
typedef struct specific_data specific_data_t;

typedef unsigned int	pthread_key_t;

extern int
pthread_key_create __((pthread_key_t *, void (*)(void *)));

extern int
pthread_key_delete __((pthread_key_t ));

extern void *
pthread_getspecific __((pthread_key_t));

extern int
pthread_setspecific __((pthread_key_t, const void *));


/* -----------------------------------------------------------------------------
 * Cancellation
 */

extern void
pthread_testcancel __((void));

extern int
pthread_setcancelstate __((int, int *));

extern int
pthread_setcanceltype __((int, int *));

typedef struct __pthread_cleanup_handler_t {
	struct __pthread_cleanup_handler_t	*__next_handler;
	void	(*__handler_function)();
	void	*__handler_arg;
} __pthread_cleanup_handler_t;

extern pthread_key_t	__pthread_cleanup_handlerqueue;


/* -----------------------------------------------------------------------------
 * Pthread once
 */

typedef struct {
	__ptlock_type	__ptonce_lock;
	int		__ptonce_initialized;
	int		__ptonce_executing;
	int		__ptonce_completed;
	pthread_mutex_t	__ptonce_mutex;
	pthread_cond_t	__ptonce_executed;
} pthread_once_t;

#define	PTHREAD_ONCE_INIT	{UL_FREE, 0, 0, 0}

extern int
pthread_once __((pthread_once_t *, void (*)(void)));

/* -----------------------------------------------------------------------------
 * Non Posix interface
 */
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
extern  int             pthread_setconcurrency_np(int level);
extern  int             pthread_getconcurrency_np(int *level);
#endif

/* -----------------------------------------------------------------------------
 * DBX interface
 */
extern  void            __funcblock_np();

/* -----------------------------------------------------------------------------
 * DCE Compatibility
 */
#ifndef _AIX32_THREADS
extern int
pthread_getunique_np __((pthread_t *, int *));

extern int
pthread_mutexattr_getkind_np __((pthread_mutexattr_t *, int *));
#endif /* _AIX32_THREADS */

extern int
pthread_mutexattr_setkind_np __((pthread_mutexattr_t *, int ));

extern int
pthread_set_mutexattr_default_np __(( int ));

extern int
pthread_signal_to_cancel_np __((sigset_t *, pthread_t *));

extern int
pthread_delay_np __((struct timespec *));

extern int
pthread_get_expiration_np __((struct timespec *, struct timespec *));

extern void
pthread_lock_global_np __((void));

extern void
pthread_unlock_global_np __((void));

extern int
pthread_atfork_np __((void *, void (*)(void *), void (*)(void *), void (*)(void *)));

extern int
pthread_test_exit_np __((int *));

extern void
pthread_clear_exit_np __((pthread_t));

extern int
pthread_setcancelstate_np __((int, int *));

extern int
pthread_join_np __((pthread_t, void **));

pthread_mutex_t	_mutex_global_np; 	/* recursive mutex */


extern void
pthread_cleanup_push_np __((void (*)(void *), void *, pthread_t *));

extern void
pthread_cleanup_pop_np __((int, pthread_t));

#endif	/*_PTHREAD_H_*/
