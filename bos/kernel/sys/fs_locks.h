/* @(#)69	1.5  src/bos/kernel/sys/fs_locks.h, syslfs, bos411, 9428A410j 5/16/94 13:25:14 */
/*
 *   COMPONENT_NAME: SYSLFS
 *
 *   FUNCTIONS: DEVNODE_LOCK
 *		DEVNODE_UNLOCK
 *		FFREE_LOCK
 *		FFREE_UNLOCK
 *		FIFOBUF_LOCK
 *		FIFOBUF_UNLOCK
 *		FILOCK_LOCK
 *		FILOCK_UNLOCK
 *		FP_LOCK
 *		FP_OFFSET_LOCK
 *		FP_OFFSET_UNLOCK
 *		FP_UNLOCK
 *		FS_COMPLEX_LOCK
 *		FS_COMPLEX_UNLOCK
 *		FS_KLOCK
 *		FS_KUNLOCK
 *		FS_SIMPLE_UNLOCK
 *		GFS_LOCK
 *		GFS_UNLOCK
 *		GN_RECLK_LOCK
 *		GN_RECLK_UNLOCK
 *		GPA_LOCK
 *		GPA_UNLOCK
 *		PN_LOCK
 *		PN_UNLOCK
 *		SPECHASH_LOCK
 *		SPECHASH_UNLOCK
 *		SPECNODE_LOCK
 *		SPECNODE_UNLOCK
 *		U_FD_LOCK
 *		U_FD_UNLOCK
 *		U_FSO_LOCK
 *		U_FSO_UNLOCK
 *		VFS_LIST_LOCK
 *		VFS_LIST_UNLOCK
 *		VN_LOCK
 *		VN_UNLOCK
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/lockl.h>
#include <sys/low.h>

/*
 * The FS_KLOCK and FS_KUNLOCK macros are used to hide the version 3
 * locking, which is not used in this implementation.
 */
#define FS_KLOCK(x,y)		LOCK_NEST
#define FS_KUNLOCK(x)

/*
 * type definitions for use as array indices in fs_lock_stat.
 * see fs_locks.c
 */
#define U_FD       0            /* file descriptor */
#define U_FSO      1            /* other ublock fields */
#define GPA        2		/* gpai allocation */
#define FFREE      3		/* file struct freelist */
#define FPTR       4		/* file struct */
#define FOFF       5		/* offset field in file struct */
#define GFS        6		/* gfs list */
#define PATH       7		/* pathname struct */
#define FILOCK     8		/* record lock lists */
#define RECLK      9		/* gnode record lock list */
#define VNODE     10		/* vnode struct */
#define VFS_LIST  11		/* vfs list */
#define VFS       12		/* vfs struct */
#define DEVNODE   13            /* devnode struct */
#define SPECNODE  14            /* specnode struct */
#define SPECHASH  15            /* specnode hash lists */
#define FIFOBUF   16            /* pipe buffers */
#define LTCOUNT   17		/* count of types */

/*
 * File System Ublock Locking
 * Before taking the file descriptor locks, the calling thread should
 * check to see if it is the only thread running in the process.
 * If so, no ublock locking is required.  Note that the caller must
 * remember whether or not the lock was taken and unlock accordingly.
 */
#ifdef _FSDEBUG

#define U_FD_LOCK()		fs_simple_lock(&U.U_fd_lock, U_FD)
#define U_FD_UNLOCK()		fs_simple_unlock(&U.U_fd_lock, U_FD)

#define U_FSO_LOCK()		fs_simple_lock(&U.U_fso_lock, U_FSO)
#define U_FSO_UNLOCK()		fs_simple_unlock(&U.U_fso_lock, U_FSO)

#else  /* _FSDEBUG */

#define U_FD_LOCK()		simple_lock(&U.U_fd_lock)
#define U_FD_UNLOCK()		simple_unlock(&U.U_fd_lock)

#define U_FSO_LOCK()		simple_lock(&U.U_fso_lock)
#define U_FSO_UNLOCK()		simple_unlock(&U.U_fso_lock)

#endif /* _FSDEBUG */

/*
 * File System Structure Locking
 */

/* macros for the file system locks */
#ifdef _FSDEBUG

#define DEVNODE_LOCK(dp)	fs_simple_lock(&(dp)->dv_lock, DEVNODE)
#define DEVNODE_UNLOCK(dp)	fs_simple_unlock(&(dp)->dv_lock, DEVNODE)
#define FFREE_LOCK()		fs_simple_lock(&ffree_lock, FFREE)
#define FFREE_UNLOCK()		fs_simple_unlock(&ffree_lock, FFREE)
#define FIFOBUF_LOCK()		fs_simple_lock(&fifobuf_lock, FIFOBUF)
#define FIFOBUF_UNLOCK()	fs_simple_unlock(&fifobuf_lock, FIFOBUF)
#define FILOCK_LOCK()		fs_simple_lock(&filock_lock, FILOCK)
#define FILOCK_UNLOCK()		fs_simple_unlock(&filock_lock, FILOCK)
#define FP_LOCK(fp)		fs_simple_lock(&(fp)->f_lock, FPTR)
#define FP_UNLOCK(fp)		fs_simple_unlock(&(fp)->f_lock, FPTR)
#define FP_OFFSET_LOCK(fp)	fs_simple_lock(&(fp)->f_offset_lock, FOFF)
#define FP_OFFSET_UNLOCK(fp)	fs_simple_unlock(&(fp)->f_offset_lock, FOFF)
#define GFS_LOCK() 		fs_simple_lock(&gfs_lock, GFS)
#define GFS_UNLOCK()		fs_simple_unlock(&gfs_lock, GFS)
#define GN_RECLK_LOCK(gp)	fs_simple_lock(&(gp)->gn_reclk_lock, RECLK)
#define GN_RECLK_UNLOCK(gp)	fs_simple_unlock(&(gp)->gn_reclk_lock, RECLK)
#define GPA_LOCK(ap)		fs_simple_lock(&(ap)->a_lock, GPA)
#define GPA_UNLOCK(ap)		fs_simple_unlock(&(ap)->a_lock, GPA)
#define PN_LOCK()		fs_simple_lock(&pn_lock, PATH)
#define PN_UNLOCK()		fs_simple_unlock(&pn_lock, PATH)
#define SPECHASH_LOCK(hp)	fs_simple_lock(&(hp)->hn_lock, SPECHASH)
#define SPECHASH_UNLOCK(hp)	fs_simple_unlock(&(hp)->hn_lock, SPECHASH)
#define SPECNODE_LOCK(sp)	fs_simple_lock(&(sp)->sn_lock, SPECNODE)
#define SPECNODE_UNLOCK(sp)	fs_simple_unlock(&(sp)->sn_lock, SPECNODE)
#define VFS_LIST_LOCK()		fs_simple_lock(&vfs_list_lock, VFS_LIST)
#define VFS_LIST_UNLOCK()	fs_simple_unlock(&vfs_list_lock, VFS_LIST)
#define VN_LOCK(vp)		fs_simple_lock(&(vp)->v_lock, VNODE)
#define VN_UNLOCK(vp)		fs_simple_unlock(&(vp)->v_lock, VNODE)

#else  /* _FSDEBUG */

#define DEVNODE_LOCK(dp)	simple_lock(&(dp)->dv_lock)
#define DEVNODE_UNLOCK(dp)	simple_unlock(&(dp)->dv_lock)
#define FFREE_LOCK()		simple_lock(&ffree_lock)
#define FFREE_UNLOCK()		simple_unlock(&ffree_lock)
#define FIFOBUF_LOCK()		simple_lock(&fifobuf_lock)
#define FIFOBUF_UNLOCK()	simple_unlock(&fifobuf_lock)
#define FILOCK_LOCK()		simple_lock(&filock_lock)
#define FILOCK_UNLOCK()		simple_unlock(&filock_lock)
#define FP_LOCK(fp)		simple_lock(&(fp)->f_lock)
#define FP_UNLOCK(fp)		simple_unlock(&(fp)->f_lock)
#define FP_OFFSET_LOCK(fp)	simple_lock(&(fp)->f_offset_lock)
#define FP_OFFSET_UNLOCK(fp)	simple_unlock(&(fp)->f_offset_lock)
#define GFS_LOCK()		simple_lock(&gfs_lock)
#define GFS_UNLOCK()		simple_unlock(&gfs_lock)
#define GN_RECLK_LOCK(gp)	simple_lock(&(gp)->gn_reclk_lock)
#define GN_RECLK_UNLOCK(gp)	simple_unlock(&(gp)->gn_reclk_lock)
#define GPA_LOCK(ap)		simple_lock(&(ap)->a_lock)
#define GPA_UNLOCK(ap)		simple_unlock(&(ap)->a_lock)
#define PN_LOCK()		simple_lock(&pn_lock)
#define PN_UNLOCK()		simple_unlock(&pn_lock)
#define SPECHASH_LOCK(hp)	simple_lock(&(hp)->hn_lock)
#define SPECHASH_UNLOCK(hp)	simple_unlock(&(hp)->hn_lock)
#define SPECNODE_LOCK(sp)	simple_lock(&(sp)->sn_lock)
#define SPECNODE_UNLOCK(sp)	simple_unlock(&(sp)->sn_lock)
#define VFS_LIST_LOCK()		simple_lock(&vfs_list_lock)
#define VFS_LIST_UNLOCK()	simple_unlock(&vfs_list_lock)
#define VN_LOCK(vp)		simple_lock(&(vp)->v_lock)
#define VN_UNLOCK(vp)		simple_unlock(&(vp)->v_lock)

#endif /* _FSDEBUG */

