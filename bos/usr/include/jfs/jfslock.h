/* @(#)68	1.3  src/bos/usr/include/jfs/jfslock.h, syspfs, bos411, 9438B411a 9/20/94 11:56:38 */
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfslock.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_JFS_LOCK
#define _H_JFS_LOCK

#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

/*
 * JFS lock definition
 */

/* inode cache lock
 */
extern Simple_lock	jfs_icache_lock;

/* directory name cache lock
 */
extern Simple_lock	jfs_ncache_lock;

#define ICACHE_LOCK()		simple_lock(&jfs_icache_lock)
#define ICACHE_UNLOCK()		simple_unlock(&jfs_icache_lock)

/* directory name lookup cache lock
 */
#define NCACHE_LOCK()		simple_lock(&jfs_ncache_lock)
#define NCACHE_UNLOCK()		simple_unlock(&jfs_ncache_lock)

/* sync() lock
 */
#define SYNC_LOCK()		simple_lock(&jfs_sync_lock)
#define SYNC_UNLOCK()		simple_unlock(&jfs_sync_lock)

/* log read/write lock
 */ 
#define LOG_LOCK(ip)		simple_lock(&((ip)->i_loglock))
#define LOG_UNLOCK(ip)		simple_unlock(&((ip)->i_loglock))

/* compression lock
 */
#define JFS_COMP_LOCK()		simple_lock(&jfs_comp_lock)
#define JFS_COMP_UNLOCK()	simple_unlock(&jfs_comp_lock)

/* inode read/write lock
 */
#ifdef _I_MULT_RDRS

#define IREAD_LOCK(ip)		lock_read(&((ip)->i_rdwrlock))
#define IREAD_UNLOCK(ip)	lock_done(&((ip)->i_rdwrlock))
#define IWRITE_LOCK(ip)		lock_write(&((ip)->i_rdwrlock))
#define IWRITE_LOCK_TRY(ip)	lock_try_write(&((ip)->i_rdwrlock))
#define IWRITE_UNLOCK(ip)	lock_done(&((ip)->i_rdwrlock))
#define INODE_LOCK(ip)		simple_lock(&((ip)->i_nodelock))
#define INODE_UNLOCK(ip)	simple_unlock(&((ip)->i_nodelock))
#define IREAD_TO_WRITE(ip)	lock_read_to_write(&((ip)->i_rdwrlock))
#define IWRITE_TO_READ(ip)	lock_write_to_read(&((ip)->i_rdwrlock))

#else /* simple lock */

#define IREAD_LOCK(ip)		simple_lock(&((ip)->i_rdwrlock))
#define IREAD_UNLOCK(ip)	simple_unlock(&((ip)->i_rdwrlock))
#define IWRITE_LOCK(ip)		simple_lock(&((ip)->i_rdwrlock))
#define IWRITE_LOCK_TRY(ip)	simple_lock_try(&((ip)->i_rdwrlock))
#define IWRITE_UNLOCK(ip)	simple_unlock(&((ip)->i_rdwrlock))
#define INODE_LOCK(ip)
#define INODE_UNLOCK(ip)
#define IREAD_TO_WRITE(ip)	0
#define IWRITE_TO_READ(ip)	0

#endif

/* disk quota lock
 */
#define DQUOT_LOCK(dp)		vcs_dqlock(dp) 
#define DQUOT_UNLOCK(dp)	vcs_unlockdq(dp)
#define QUOTA_READ_LOCK()	lock_read(&jfs_quota_lock)
#define QUOTA_WRITE_LOCK()	lock_write(&jfs_quota_lock)
#define QUOTA_LOCK_RECURSIVE()	lock_set_recursive(&jfs_quota_lock)
#define QUOTA_LOCK_UNRECURSIVE() lock_clear_recursive(&jfs_quota_lock)
#define QUOTA_UNLOCK()		lock_done(&jfs_quota_lock)

/* jfs/vmm serialization lock
 */
#define	COMBIT_LOCK(ip)				\
	if (ip->i_seg)				\
		vcs_lockseg(ip->i_seg)

#define	COMBIT_UNLOCK(ip)			\
	if (ip->i_seg)				\
		vcs_unlockseg(ip->i_seg,FALSE)

#define	COMBIT_UNLOCKD(ip)			\
	if (ip->i_seg)				\
		vcs_unlockseg(ip->i_seg,TRUE)

#endif /* _H_JFS_LOCK */
