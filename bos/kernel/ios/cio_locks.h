/* @(#)08  1.3  src/bos/kernel/ios/cio_locks.h, sysios, bos411, 9428A410j 3/28/94 01:59:18 */

#ifndef _h_CIO_LOCKS
#define _h_CIO_LOCKS

/*
 * COMPONENT_NAME: (SYSIOS) Character I/O header file
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
 * $Log: cio_locks.h,v $
 * $EndLog$
 */

/*
 *	Locking order:
 *
 *	CLIST_LOCK		--	acquired first
 *	CIO_MUTEX_LOCK		--	acquired last
 *
 *	and:
 *
 *	C_MAINT_PINCF_LOCK	--	acquired first
 *	C_MAINT_LOCK
 *	CIO_MUTEX_LOCK		--	acquired last
 */

#include <sys/sleep.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>


/*
 * Locking primitives for the CIO module
 */

#if defined(_POWER_MP)

#define	CIO_MUTEX_LOCK_DECLARE		/* Declare the CIO module MUTEX lock */\
		Simple_lock cio_lock

#define	CIO_MUTEX_LOCK_ALLOC()		/* Allocate the lock */\
		lock_alloc(&cio_lock, LOCK_ALLOC_PIN, CIO_LOCK_FAMILY, -1)

#define	CIO_MUTEX_LOCK_INIT()		/* Initialize the lock */\
		simple_lock_init(&cio_lock)

#define	CIO_MUTEX_LOCK()		/* Acquire the lock */\
		disable_lock(INTCLIST, &cio_lock)

#define	CIO_MUTEX_UNLOCK(o_p)		/* Release the lock */\
		unlock_enable(o_p, &cio_lock)

extern Simple_lock cio_lock;

#else	/* UP version */

#define	CIO_MUTEX_LOCK_DECLARE		/* Declare the CIO module MUTEX lock */
#define	CIO_MUTEX_LOCK_ALLOC()		/* Allocate the lock */
#define	CIO_MUTEX_LOCK_INIT()		/* Initialize the lock */

#define	CIO_MUTEX_LOCK()		/* Acquire the lock */\
		i_disable(INTCLIST)

#define	CIO_MUTEX_UNLOCK(o_p)		/* Release the lock */\
		i_enable(o_p)

#endif	/* #if defined(_POWER_MP) */


#define	C_MAINT_LOCK_ALLOC()		/* Allocate the maintenance lock */\
		lock_alloc(&c_maint.lock, LOCK_ALLOC_PIN, CMAINT_LOCK_FAMILY, -1)

#define	C_MAINT_LOCK_INIT()		/* Initialize the maintenance lock */\
		simple_lock_init(&c_maint.lock)

#define	C_MAINT_LOCK()			/* Acquire the maintenance lock */\
		simple_lock(&c_maint.lock)

#define	C_MAINT_UNLOCK()		/* Release the maintenance lock */\
		simple_unlock(&c_maint.lock)


#define	C_MAINT_PINCF_LOCK_ALLOC()	/* Allocate the pincf() lock */\
		lock_alloc(&c_maint.pincf_lock,\
				LOCK_ALLOC_PIN, PINCF_LOCK_FAMILY, -1)

#define	C_MAINT_PINCF_LOCK_INIT()	/* Initialize the pincf() lock */\
		simple_lock_init(&c_maint.pincf_lock)

#define	C_MAINT_PINCF_LOCK()		/* Acquire the pincf() lock */\
		simple_lock(&c_maint.pincf_lock)

#define	C_MAINT_PINCF_UNLOCK()		/* Release the pincf() lock */\
		simple_unlock(&c_maint.pincf_lock)


#if defined(_POWER_MP)

#define	CLIST_LOCK_DECLARE		/* Declare the clist lock */\
		Simple_lock clist_lock

#define	CLIST_LOCK_ALLOC()		/* Allocate clist lock */\
		lock_alloc(&clist_lock, LOCK_ALLOC_PIN, CLIST_LOCK_FAMILY, -1)

#define	CLIST_LOCK_INIT()		/* Initialize clist lock */\
		simple_lock_init(&clist_lock)

#define	CLIST_LOCK()			/* Acquire clist lock */\
		disable_lock(INTCLIST, &clist_lock)

#define	CLIST_UNLOCK(o_p)		/* Release clist lock */\
		unlock_enable(o_p, &clist_lock)

extern Simple_lock clist_lock;

#else	/* UP version */

#define	CLIST_LOCK_DECLARE		/* Declare the clist lock */
#define	CLIST_LOCK_ALLOC()		/* Allocate clist lock */
#define	CLIST_LOCK_INIT()		/* Initialize clist lock */

#define	CLIST_LOCK()			/* Acquire clist lock */\
		i_disable(INTCLIST)

#define	CLIST_UNLOCK(o_p)		/* Release clist lock */\
		i_enable(o_p)

#endif	/* #if defined(_POWER_MP) */


/*
 * Sleep/wackup services
 */

#if defined(_POWER_MP)

#define	CIO_RELEASE_SLEEP(e)		/* Release cio_lock and go to sleep */\
		e_sleep_thread(e, &cio_lock, LOCK_HANDLER | INTERRUPTIBLE)

#else   /* UP version */

#define	CIO_RELEASE_SLEEP(e)		/* Just go to sleep */\
		e_sleep_thread(e, NULL, LOCK_HANDLER | INTERRUPTIBLE)

#endif	/* #if defined(_POWER_MP) */

#define	WAKEUP(e)			/* Wake up threads waiting for "e" */\
		e_wakeup(e)


#endif /* _h_CIO_LOCKS */
