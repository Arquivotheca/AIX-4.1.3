#if ! defined(_FUNNELED)
#define _FUNNELED
#endif

/* @(#)05  1.5  src/bos/kernel/ios/bio_locks.h, sysios, bos411, 9428A410j 5/16/94 13:41:27 */

#ifndef _h_BIO_LOCKS
#define _h_BIO_LOCKS

/*
 * COMPONENT_NAME: (SYSIOS) Block I/O header file
 * 
 * FUNCTIONS: provide implementation independent locking/unlocking
 *	primitives for the BIO module
 * 
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Level 1, 5 years, Bull confidental information
 */
/*
 * @BULL_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: bio_locks.h,v $
 * $EndLog$
 */


#include <sys/sleep.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/atomic_op.h>


/*
 * Locking primitives for the BIO module
 */

#if defined(_POWER_MP)

#define	BIO_MUTEX_LOCK_DECLARE		/* Declare the BIO module MUTEX lock */\
	Simple_lock bio_lock

#define	BIO_MUTEX_LOCK_ALLOC()		/* Allocate the lock */\
	lock_alloc((void *)&bio_lock, LOCK_ALLOC_PIN, BIO_LOCK_FAMILY, -1)

#define	BIO_MUTEX_LOCK_INIT()		/* Initialize the lock */\
	simple_lock_init(&bio_lock)

#define	BIO_MUTEX_LOCK()		/* Acquire the lock */\
	disable_lock(INTIODONE, &bio_lock)

#define	BIO_MUTEX_UNLOCK(o_p)		/* Release the lock */\
	unlock_enable(o_p, &bio_lock)

extern Simple_lock bio_lock;

#else /* for UP: */

#define	BIO_MUTEX_LOCK_DECLARE		/* Declare the BIO module MUTEX lock */
#define	BIO_MUTEX_LOCK_ALLOC()		/* Allocate the lock */
#define	BIO_MUTEX_LOCK_INIT()		/* Initialize the lock */

#define	BIO_MUTEX_LOCK()		/* Acquire the lock */\
	i_disable(INTIODONE)

#define	BIO_MUTEX_UNLOCK(o_p)		/* Release the lock */\
	i_enable(o_p)

#endif /* #if defined(_POWER_MP) */


/*
 * Global 'iodonelist' locking for funnelization
 */

#if defined(_POWER_MP) && defined(_FUNNELED)

#define	IODONELIST_LOCK_DECLARE		/* Declare the IODONE-list lock */\
	Simple_lock dil_lock

#define	IODONELIST_LOCK_ALLOC()		/* Allocate the IODONE-list lock */\
	lock_alloc((void *)&dil_lock, LOCK_ALLOC_PIN, DIL_LOCK_FAMILY, -1)

#define	IODONELIST_INIT()		/* Initialize the IODONE-list lock */\
	simple_lock_init(&dil_lock)

#define	IODONELIST_LOCK()		/* Acquire the IODONE-list lock */\
	disable_lock(INTMAX, &dil_lock)

#define	IODONELIST_UNLOCK(o_p)		/* Release the IODONE-list lock */\
	unlock_enable(o_p, &dil_lock)

extern Simple_lock dil_lock;


/*
 * strategy-list lock
 */

#define	STRATLIST_LOCK_DECLARE		/* Declare the strategy-list lock */\
	Simple_lock dsl_lock

#define	STRATLIST_LOCK_ALLOC()		/* Allocate the strategy-list lock */\
	lock_alloc((void *)&dsl_lock, LOCK_ALLOC_PIN, DSL_LOCK_FAMILY, -1)

#define	STRATLIST_INIT()		/* Initialize the strategy-list lock */\
	simple_lock_init(&dsl_lock)

#define	STRATLIST_LOCK()		/* Acquire the strategy-list lock */\
	disable_lock(INTMAX, &dsl_lock)

#define	STRATLIST_UNLOCK(o_p)		/* Release the strategy-list lock */\
	unlock_enable(o_p, &dsl_lock)

extern Simple_lock dsl_lock;

#endif


/*
 * Sleep/wait services
 */

#if defined(_POWER_MP)

#define	BIO_SLEEP(e)		/* Release bio_lock and go to sleep */\
	(void) e_sleep_thread(e, &bio_lock, LOCK_HANDLER)

#else /* for UP: */

#define	BIO_SLEEP(e)		/* Just go to sleep */\
	(void) e_sleep_thread(e, NULL, LOCK_HANDLER)

#endif /* #if defined(_POWER_MP) */

#define	WAKEUP(e)		/* Wake up all threads waiting for "e" */\
	e_wakeup(e)


/*
 * Miscellaneous definitions
 */

#if defined(_POWER_MP)
#define	INCREMENT(x)		fetch_and_add(&(x), 1)    /* Atomic increment */
#define	DECREMENT(x)		fetch_and_add(&(x), -1)   /* Atomic decrement */
#define	INCREMENT_h(x)		fetch_and_add_h(&(x), 1)  /* Atomic increment */
#define	DECREMENT_h(x)		fetch_and_add_h(&(x), -1) /* Atomic decrement */
#else /* for UP: */
#define	INCREMENT(x)		((x)++)
#define	DECREMENT(x)		((x)--)
#define	INCREMENT_h(x)		((x)++)
#define	DECREMENT_h(x)		((x)--)
#endif /* #if defined(_POWER_MP) */

#if defined(_DEBUG)
#define	AASSSSEERRTT(x)	ASSERT(x)
#else
#define	AASSSSEERRTT(x)
#endif

#endif  /* _h_BIO_LOCKS */
