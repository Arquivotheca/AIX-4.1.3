/* @(#)94	1.5.1.12  src/bos/kernel/sys/lock_def.h, sysproc, bos411, 9438A411a 9/15/94 16:04:01 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#ifndef _H_LOCK_DEFINE
#define _H_LOCK_DEFINE

#include <sys/types.h>

/* type definition for simple_lock control structure */
/*
*        ----------------------------
*        |             |              |
*        |  I W X X S  |  owner_id    |
*        |             |              |
*        ----------------------------
*	
*	I		Interlock bit
*	W		Waiting bit
*       S               Secondary structure allocated
*			(DON'T CHANGE THIS BIT POSITION - instrumentation
*			 address dependent)
*	owner_id	thread_id
*/

typedef	int 	simple_lock_data;


/* type definition for complex_lock control structure */
/*
*        ---------------------------
*        |             |  thread id  |
*        | I W WW RD S |-------------|
*        |             |  read count |
*        ---------------------------
*        |             |  recursion  |
*        |    flags    |             |
*        |             |    depth    |
*        ---------------------------
*
*       I               Interlock bit
*       W               Waiting bit
*       WW              Want write bit
*       RD              Read mode bit
*       S               Secondary structure allocated
*			(DON'T CHANGE THIS BIT POSITION - instrumentation
*			 address dependent)
*       thread id       owner's thread id
*       read count      readers count
*       flags           hold recursive bit
*       recursion depth counter of recursive acquisitions
*
*/

struct complex_lock_data {
                int     status;
                short   flags;
                short   recursion_depth;
        };


/* 
 * Type definition for lock secondary structure.  Holds the lock control 
 * structure and the instrumentation fields.  Allocated by lock_alloc and
 * freed by lock_free.
 */
struct lock_data_instrumented {

		union {
			simple_lock_data		s_lock;
			struct complex_lock_data 	c_lock;
			struct lock_data_instrumented	*lock_next;
		} lock_control_word;
		unsigned int	acquisitions;		/* counter of hits */
		unsigned int	misses;			/* counter of misses */
		unsigned int	sleeps;		/* counter of sleeping misses */
		union	{				/* lock identifier */
			int 	name;
			struct  {
				short	_id;
				short	occurrence;
			} _lock_id;
		} _lockname;
		int	reserved[2];		/* cache line padding */
#ifdef DEBUG
		int	lock_lr;	/* link register of lock */
		int 	lock_caller; 	/* caller of lock */
		int 	lock_cpuid;	/* cpu id of lock */
		int	unlock_lr;	/* link register of unlock */
		int 	unlock_caller;  /* caller of unlock */
		int	dbg_zero;	/* this word must be zero */
		int 	unlock_cpuid;	/* cpu id of unlock */
		int	dbg_flags;	/* debug flags */
#define LOCK_IS_ALLOCATED 0x80000000    /* this entry is allocated */
#endif
	};

/*
*                                       Instrumentation Structure
*
*                                       |                        |
*        LOCK STRUCTURE                 |                        |
*        ---------------------          |------------------------|
*        |                   |--------->| CONTROL STRUCTURE      |
*        ---------------------          |         .              |
*                                       |         .              |
*                                       | Instrumentation word 1 |
*                                       | Instrumentation word n |
*                                       |------------------------|
*                                       |                        |
*                                       |                        |
*
*        LOCK_STRUCTURE 
*        --------------------- 
*        | CONTROL STRUCTURE |
*        --------------------- 
*/

/* type definition for simple_lock */
union _simple_lock{
	simple_lock_data		_slock;
	struct lock_data_instrumented	*_slockp;
};	

/* type definition for complex_lock */
union _complex_lock{
	struct complex_lock_data	_clock;
	struct lock_data_instrumented	*_clockp;
};	

typedef	union _simple_lock	Simple_lock;
typedef union _complex_lock	Complex_lock;

/* type definition for lock pointers */
typedef  Simple_lock	*simple_lock_t;
typedef  Complex_lock	*complex_lock_t;

/* v3 lockl definition */
typedef int			lock_t;

/* Initial available definitions */
#define SIMPLE_LOCK_AVAIL		0	
#define COMPLEX_LOCK_AVAIL		SIMPLE_LOCK_AVAIL 
#define LOCK_AVAIL  			((lock_t) -1)    /* lockl locks */

/* This macro is no longer compatible with AIXv3 */
#define LOCKL_OWNER_MASK	0x3fffffff 	
#ifdef _KERNSYS
#define IS_LOCKED(x)    ((tid_t)(*(x) & LOCKL_OWNER_MASK) == curthread->t_tid)
#else
#define IS_LOCKED(x)    \
  ((*(x) != LOCK_AVAIL) && ((tid_t)(*(x) & LOCKL_OWNER_MASK) == thread_self()))
#endif

/* lockl flags values: */
#define LOCK_SHORT      (0)             /* short wait, inhibit signals */
#define LOCK_NDELAY     (1)             /* do not wait, if unavailable */
#define LOCK_SIGWAKE    (2)             /* wake on signal */
#define LOCK_SIGRET     (4)             /* return on signal */

/* lockl return codes: */
#define LOCK_SUCC       (0)             /* success */
#define LOCK_NEST       (1)             /* already locked by this process */
#define LOCK_FAIL       (-1)            /* lock not available */
#define LOCK_SIG        (-2)            /* process signalled */

#ifdef _NO_PROTO

/* simple lock routines */
void simple_lock();
void simple_unlock();
boolean_t simple_lock_try();

/* synchronization for interrupt/interrupt and
 * thread/interrupt critical section
 */
int disable_lock();
void unlock_enable();

/* complex lock basic routines */
void lock_init();
void lock_write();
void lock_read();
void lock_done();

/* complex lock upgrade/downgrade routines */
boolean_t lock_read_to_write();
void lock_write_to_read();

/* complex lock non blocking routines */
boolean_t lock_try_write();
boolean_t lock_try_read();
boolean_t lock_try_read_to_write();

/* complex lock routines for recursion management */
void lock_set_recursive();
void lock_clear_recursive();
int lock_islocked();

/* lockl routines */
int lockl();
void unlockl();                 
boolean_t lockl_mine();	

#else /* _NO_PROTO */

/* simple lock routines */
void simple_lock(simple_lock_t);
void simple_unlock(simple_lock_t);
boolean_t simple_lock_try(simple_lock_t);

/* synchronization for interrupt/interrupt and thread/interrupt critical section
 */
int disable_lock(int,simple_lock_t);
void unlock_enable(int,simple_lock_t);

/* complex lock basic routines */
void lock_init(complex_lock_t , boolean_t);
void lock_write(complex_lock_t);
void lock_read(complex_lock_t);
void lock_done(complex_lock_t);

/* complex lock upgrade/downgrade routines */
boolean_t lock_read_to_write(complex_lock_t);
void lock_write_to_read(complex_lock_t);

/* complex lock non blocking routines */
boolean_t lock_try_write(complex_lock_t);
boolean_t lock_try_read(complex_lock_t);
boolean_t lock_try_read_to_write(complex_lock_t);

/* complex lock routines for recursion management */
void lock_set_recursive(complex_lock_t);
void lock_clear_recursive(complex_lock_t);
int lock_islocked(complex_lock_t);

/* lockl routines */
int lockl(lock_t *,int);
void unlockl(lock_t *);                
boolean_t lockl_mine(lock_t *);	

#endif /* _NO_PROTO */

#if defined(_POWER_MP) || !defined(_KERNSYS)

#ifdef _NO_PROTO
void simple_lock_init();
#else /* _NO_PROTO */
void simple_lock_init(simple_lock_t);
#endif /* _NO_PROTO */
#else /* _POWER_MP || !_KERNSYS */

/* if the function simple_lock_init changes, so must this MACRO */
#define simple_lock_init(l) *((simple_lock_data *)l) = SIMPLE_LOCK_AVAIL

#endif /* _POWER_MP || !_KERNSYS */

#ifdef _INSTRUMENTATION
#define	lo_next	lock_control_word.lock_next
#define lockname	_lockname.name
#define lock_id		_lockname._lock_id._id
#define _occurrence	_lockname._lock_id.occurrence
#endif

#ifdef _KERNSYS

/* Sub-definitions of complex_lock */
#define	_status			_clock.status
#define _flags			_clock.flags
#define _recursion_depth	_clock.recursion_depth

extern unsigned int maxspin;
#define MAXSPIN_MP			0xffffffff
#define MAXSPIN_UP			0x1

#define SIMPLE_LOCK_AVAIL_WAITERS	WAITING  /* lock available with threads
						  * waiting
						  */
/* bit field defines */
#define OWNER_MASK			0x07ffffff 	/* mask all status bit */
#define	INTERLOCK			0x80000000	/* INTERLOCK BIT */
#define	WAITING				0x40000000	/* WAITING BIT */
#define	WANT_WRITE			0x20000000	/* WANT_WRITE BIT */
#define	LOCKBIT				0x20000000	/* LOCK BIT */
#define RECURSION 			0x10000000	/* vmm RECURSION BIT */
#define	READ_MODE			0x10000000	/* READ_MODE BIT */
#define	INSTR_ON			0x08000000	/* RECURSIVE BIT */
#define READ_COUNT_MASK			OWNER_MASK	/* extract READ COUNT */
#define ONE_READER			0x10000001	/* set one reader	*/
#define THREAD_BIT			0x00000001	/* set if owner is a thread */

/* recursive flag define */
#define	RECURSIVE	    1	

/* trace sub-hooks */
#define hkwd_LOCK_TAKEN     1
#define hkwd_LOCK_MISS      2
#define hkwd_LOCK_RECURSIVE 3
#define hkwd_LOCK_BUSY      4
#define hkwd_LOCK_DISABLED  8

#define hkwd_SETRECURSIVE   1
#define hkwd_CLEARRECURSIVE 2

/* other defines: lock operation for traces */
#define LOCK_SWRITE_TRACE		1
#define LOCK_CWRITE_TRACE		2
#define LOCK_READ_TRACE			3
#define LOCK_UPGRADE_TRACE		4
#define LOCK_DOWNGRADE_TRACE		5

/* gets caller's return address from the stack (if trace on) */
#define GET_RETURN_ADDRESS(x)				\
if (TRC_ISON(0))					\
{							\
	extern int *get_stkp();				\
	struct stack_frame {				\
		struct stack_frame *next;		\
		int                 unused;		\
		int                 link_register;	\
	} *sf;						\
	sf = (struct stack_frame *)get_stkp();		\
	sf = sf->next;					\
	x = sf->link_register;				\
}

/* define for lock wait queue management */
extern int      lockhsque[];
extern int	locklhsque[];

#define LOCKHSQ          127             /* should be "prime like" */
#define HASH_VAL(X)     ((SRTOSID(mfsri((uint)(X))))+(SEGOFFSET((uint)(X))))
#define lockq_hash(X)   (&lockhsque[ ((uint)(HASH_VAL(X))&0x7fffffff) % LOCKHSQ ])
#define LOCKLHSQ	 13		 /* should be "prime like" */
#define locklq_hash(X)  (&locklhsque[ ((uint)(HASH_VAL(X))&0x7fffffff) % LOCKLHSQ ])

/* define for lockl instrumentation management */
extern struct lock_data_instrumented    lockl_hstat[];
#define LOCKLHS		 255
#define lockl_hash(X)   (&lockl_hstat[ ((uint)(X)&0x7fffffff) % LOCKLHS ])

#endif	/* _KERNSYS */

#endif /* _H_LOCK_DEFINE */

