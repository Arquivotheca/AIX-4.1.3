/* @(#)16	1.2  src/bos/kernel/net/net_unixlock.h, sysnet, bos411, 9428A410j 9/15/93 18:11:25 */
/*
 *   COMPONENT_NAME: SYSNET
 *
 *   FUNCTIONS: lock_clear_recursive
 *		lock_done
 *		lock_init2
 *		lock_islocked
 *		lock_read
 *		lock_set_recursive
 *		lock_write
 *		lock_write_to_read
 *		simple_lock
 *		simple_lock_init
 *		simple_unlock
 *		
 *
 *   ORIGINS: 27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * 
 * (c) Copyright 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 */
/*
 * OSF/1 1.2
 */
/*
 * Lock debugging aids for UNIX.
 *
 * Because of splnet/splimp and single threading, these locks
 * should always succeed. Assertions provided for debugging.
 */

#ifdef	HMMMMMM


#ifndef _KERN_LOCK_H_
#define _KERN_LOCK_H_

typedef struct slock {
	unsigned long sm_lock;
#define S_LCK	(unsigned long)(0x87654321)
#define S_ULCK	(unsigned long)(0x12345678)
} *simple_lock_t, simple_lock_data_t;

typedef struct lock {
	unsigned long rw_lock;
#define A_LCK	(unsigned long)(0x9abcdef0)
#define R_LCK	(A_LCK|1)
#define W_LCK	(A_LCK|2)
#define L_ULCK	(unsigned long)(0x0fedcba9)
	int	count;
} *lock_t, lock_data_t;

extern char _net_lock_format_[];
extern char _net_simple_lock_[],_net_simple_unlock_[];
extern char _net_lock_write_[],_net_lock_read_[],_net_lock_recursive_[];
extern char _net_lock_write_to_read_[],_net_lock_done_[];

#define simple_lock_init(slp)	  \
	((slp)->sm_lock = S_ULCK)

#define simple_lock(slp)	{ \
	LOCK_ASSERTL_DECL	\
	LOCK_ASSERTL(_net_simple_lock_, ((slp)->sm_lock == S_ULCK)); \
	(slp)->sm_lock = S_LCK; \
}
#define simple_unlock(slp)	{ \
	LOCK_ASSERTL_DECL	\
	LOCK_ASSERTL(_net_simple_unlock_, ((slp)->sm_lock == S_LCK)); \
	(slp)->sm_lock = S_ULCK; \
}

#define lock_init2(lp,a,c) {	  \
	(lp)->rw_lock = L_ULCK; \
	(lp)->count = 0; \
}
#define lock_islocked(lp)	  \
	(((lp)->rw_lock & ~0x3) == A_LCK)
#define lock_write(lp)		{ \
	LOCK_ASSERTL_DECL	\
	if ((lp)->count == 0) { \
		LOCK_ASSERTL(_net_lock_write_, ((lp)->rw_lock == L_ULCK)); \
		(lp)->rw_lock = W_LCK; \
	} else { \
		LOCK_ASSERTL(_net_lock_write_, ((lp)->rw_lock == W_LCK)); \
		(lp)->count++; \
	} \
}
#define lock_read(lp)		{ \
	LOCK_ASSERTL_DECL	\
	if ((lp)->count == 0) { \
		LOCK_ASSERTL(_net_lock_read_, ((lp)->rw_lock == L_ULCK)); \
		(lp)->rw_lock = R_LCK; \
	} else { \
		LOCK_ASSERTL(_net_lock_read_, ((lp)->rw_lock == W_LCK)); \
		(lp)->count++; \
	} \
}
#define lock_write_to_read(lp)	{ \
	LOCK_ASSERTL_DECL	\
	LOCK_ASSERTL(_net_lock_write_to_read_, ((lp)->rw_lock == W_LCK && (lp)->count == 0)); \
	(lp)->rw_lock = R_LCK; \
}
#define lock_done(lp)		{ \
	LOCK_ASSERTL_DECL	\
	LOCK_ASSERTL(_net_lock_done_, lock_islocked(lp)); \
	if ((lp)->count > 1) (lp)->count--; \
	else { (lp)->rw_lock = L_ULCK; (lp)->count = 0; } \
}
#define lock_set_recursive(lp)	{ \
	LOCK_ASSERTL_DECL	\
	LOCK_ASSERTL(_net_lock_recursive_, ((lp)->rw_lock == W_LCK)); \
	if ((lp)->count == 0) (lp)->count = 1; \
}
#define lock_clear_recursive(lp){ \
	LOCK_ASSERTL_DECL	\
	LOCK_ASSERTL(_net_lock_recursive_, ((lp)->rw_lock == W_LCK && (lp)->count)); \
	if ((lp)->count == 1) (lp)->count = 0; \
}
#endif

#endif	/* HMMMMM */
