/* @(#)53	1.19.1.8  src/bos/kernel/sys/vfs.h, syslfs, bos411, 9428A410j 5/16/94 13:25:16 */

#ifndef _H_VFS
#define _H_VFS

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/gfs.h>
#include <sys/vnode.h>
#include <sys/vmount.h>

extern Simple_lock vfs_list_lock;	/* global vfs list lock */

/*
** where vfs data is stored (VFSfile)
** where the helpers reside (VFSdir) by default
** where to find filesystem data (from IN/FSdefs.h)
*/

#define VFSfile "/etc/vfs"
#define VFSdir  "/sbin/helpers/"
#define FSfile  FSYSname

/*
** struct vfs_ent discribes an entry in the VFSfile
**
** these are returned by various getvfsent(), etc. routines.
**
** many of the vfs_ prefixes are disguises for gfs objects
** so be careful about naming when using this header file
*/

struct vfs_ent
{
  char *vfsent_name;      /* name (ie. "aix3", "nfs", etc.) */
  int   vfsent_type;      /* in vmount.h */
  int   vfsent_flags;
  char *vfsent_mnt_hlpr;  /* mount helper pathname */ 
  char *vfsent_fs_hlpr;   /* filesystem helper pathname */
};

/*
** no helper string
*/  

#define NO_HELPER "none"

/*
** as keyed by the %default VFSfile control directive
*/

#define VFS_DFLT_LOCAL     0x1
#define VFS_DFLT_REMOTE    0x2

/*
 * the vfs structure represents a virtual file system.
 *	One of them is created for each mounted file-system/object.
 */
struct vfs {
	struct vfs	*vfs_next;	/* vfs's are a linked list */
	struct gfs	*vfs_gfs;	/* ptr to gfs of vfs */
	struct vnode	*vfs_mntd;	/* pointer to mounted vnode, */
					/*	the root of this vfs */
	struct vnode	*vfs_mntdover;	/* pointer to mounted-over */
					/*	vnode		 */
	struct vnode	*vfs_vnodes;	/* all vnodes in this vfs */
	int		vfs_count;	/* number of users of this vfs */
	caddr_t		vfs_data;	/* private data area pointer */
	unsigned int	vfs_number;	/* serial number to help */
					/*  distinguish between */
					/*  different mounts of the */
					/*  same object */
	int		vfs_bsize;	/* native block size */
#ifdef	_SUN
	short		vfs_exflags;	/* for SUN, exported fs flags */
	unsigned short	vfs_exroot;	/* for SUN, " fs uid 0 mapping */
#else
	short		vfs_rsvd1;	/* Reserved */
	unsigned short	vfs_rsvd2;	/* Reserved */
#endif	/* _SUN */
	struct vmount	*vfs_mdata;	/* record of mount arguments */
};

/* these defines are for backwards compatibility */
#define vfs_fsid	vfs_mdata->vmt_fsid 
#define vfs_date	vfs_mdata->vmt_time 
#define vfs_flag	vfs_mdata->vmt_flags
#define vfs_type	vfs_gfs->gfs_type 
#define vfs_ops		vfs_gfs->gfs_ops 

#ifdef	_SUN
/* the defines are for the SUN style names */
#define vfs_vnodecovered vfs_mntdover
#define vfs_pdata vfs_data
#endif	/* _SUN */

/* note no define for vfs_op, use VFS_func() macros! */

/*
 * Definitions for bits in the vfs_flag field.
 * 
 * This flags field is shared with the mount flags (referred to by
 * vmt_flags).  The mount flags should be contained in the low order
 * 16 bits, and the vfs flags in the high order 16 bits.  The flags
 * field is shared because some of the vfs flags are redefinitions of
 * the mount flags.  See sys/vmount.h for mount flag definitions.
 */
#define	VFS_READONLY	MNT_READONLY	/* file system mounted rdonly	*/
#define	VFS_REMOVABLE	MNT_REMOVABLE	/* removable (diskette) mount	*/
#define	VFS_DEVMOUNT	MNT_DEVICE	/* device mount			*/
#define	VFS_REMOTE	MNT_REMOTE	/* remote file system		*/
#define	VFS_UNMOUNTING	MNT_UNMOUNTING	/* originated by unmount()	*/
#define VFS_SYSV_MOUNT	MNT_SYSV_MOUNT	/* System V style mount		*/
#define	VFS_NOSUID	MNT_NOSUID	/* don't maintain suid-ness	*/
					/* across this mount		*/
#define	VFS_NODEV	MNT_NODEV	/* don't allow device access	*/
					/* across this mount		*/

#define	VFS_DISCONNECTED 0x00010000	/* file mount not in use        */
#define	VFS_SHUTDOWN	0x00020000	/* forced unmount for shutdown	*/
#define VFS_VMOUNTOK	0x00040000	/* dir/file mnt permission flag	*/
#define	VFS_SUSER	0x00080000	/* client-side suser perm. flag */
#define VFS_SOFT_MOUNT 	0x00100000	/* file-over-file or directory  */
					/* over directory "soft" mount  */
#define VFS_UNMOUNTED   0x00200000      /* unmount completed, stale     */
					/* vnodes are left in the vfs   */

/* 
 * Structure containing pointers to routines
 * which implement vfs operations
 */
struct vfsops {
	/* mount a file system */
	int (*vfs_mount)(struct vfs *, struct ucred *);
	/* unmount a file system */
	int (*vfs_unmount)(struct vfs *, int, struct ucred *);
	/* get the root vnode of a file system */
	int (*vfs_root)(struct vfs *, struct vnode **, struct ucred *);
	/* get file system information */
	int (*vfs_statfs)(struct vfs *, struct statfs *, struct ucred *);
	/* sync all file systems of this type */
	int (*vfs_sync)();
	/* get a vnode matching a file id */
	int (*vfs_vget)(struct vfs *, struct vnode **, struct fileid *,
			struct ucred *);
	/* do specified command to file system */
	int (*vfs_cntl)(struct vfs *, int, caddr_t, size_t, struct ucred *);
	/* manage file system quotas */
	int (*vfs_quotactl)(struct vfs *, int, uid_t, caddr_t, struct ucred *);
};

#ifdef	_KERNEL

/*
 * Macros for calls to the VFS switch routines.
 */
#define VFS_MOUNT(vfsp, crp) \
	vfs_mount(vfsp, crp)
#define VFS_UNMOUNT(vfsp, flags, crp) \
	vfs_unmount(vfsp, flags, crp)
#define VFS_ROOT(vfsp, vpp, crp) \
	vfs_root(vfsp, vpp, crp)
#define VFS_STATFS(vfsp, sfsp, crp) \
	vfs_statfs(vfsp, sfsp, crp)
#define VFS_SYNC(gfsp) \
	vfs_sync(gfsp)
#define VFS_VGET(vfsp, vpp, fidp, crp) \
	vfs_vget(vfsp, vpp, fidp, crp)
#define VFS_CNTL(vfsp, cmd, arg, argsize, crp) \
	vfs_cntl(vfsp, cmd, arg, argsize, crp)
#define VFS_QUOTACTL(vfsp, cmds, uid, arg, crp) \
	vfs_quotactl(vfsp, cmds, uid, arg, crp)

#endif /* _KERNEL */

#endif /* _H_VFS */
