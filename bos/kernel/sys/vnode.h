/* @(#)05       1.36.1.8  src/bos/kernel/sys/vnode.h, syslfs, bos411, 9428A410j 7/25/94 15:01:53 */

#ifndef _H_VNODE
#define _H_VNODE

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 24
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef	_SUN
/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */
#endif	/* _SUN */

/*
 *
 * vnode, gnode and vnode operations structures and defines
 *
 * defines:
 *	struct vnode
 *	struct gnode
 *	struct vnodeops
 *	vnodeops calling macros
 *	struct exec_data
 *	struct open_data
 *	struct create_data
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/lockl.h>
#include <sys/uio.h>

struct ucred;
struct vattr;
struct buf;
struct eflock;
extern int audit_flag;

/*
 * vnode ("virtual inode") structure
 *	vnodes are used to keep the name and place of an object in the
 * filesystem straight, so there may be more than one vnode per object.
 * A "gnode" is used to abstract the object itself.
 * NOTE: there is always at least one vnode per gnode.
 */
struct vnode {
        ushort	v_flag;		/* see definitions below		*/
        ulong	v_count;	/* the use count of this vnode		*/
	int	v_vfsgen;	/* generation number for the vfs	*/
	Simple_lock v_lock;     /* lock on the structure		*/
        struct vfs *v_vfsp;	/* pointer to the vfs of this vnode	*/
        struct vfs *v_mvfsp;	/* pointer to vfs which was mounted over this */
				/* vnode; NULL if no mount has occurred */
	struct gnode *v_gnode;	/* ptr to implementation gnode		*/
	struct vnode *v_next;	/* ptr to other vnodes that share same gnode */
	struct vnode *v_vfsnext; /* ptr to next vnode on list off of vfs */
	struct vnode *v_vfsprev; /* ptr to prev vnode on list off of vfs */
	union v_data {
		void *		_v_socket;	 /* vnode associated data */
		struct vnode *	_v_pfsvnode;	 /* vnode in pfs for spec */
	} _v_data;
	char *	v_audit; 	 /* ptr to audit object			*/
};

#define v_socket	_v_data._v_socket
#define v_pfsvnode	_v_data._v_pfsvnode

/*
 * Definitions for v_flag field
 * of vnode structure.
 */
#define V_ROOT		0x01		/* vnode is the root of a vfs	*/
#ifdef _SUN
#define VROOT V_ROOT
#endif /* _SUN */

#define V_INTRANSIT	0x04		/* vnode is midway through	*/
					/*   vfs_vget processing	*/
#define V_DMA		0x08		/* buffer bypass		*/
#define V_TEXT		0x10		/* currently being executed	*/
#define V_RMDC		0x20		/* Usable by remote directory	*/
					/*   cache			*/
#define V_RENAME	0x40		/* Rename is in process         */
#define V_LOCK		0x80		/* Serialize exec's		*/
#define V_SPEC         0x100		/* vnode for a specfs object    */

/* this is only used as a template for per vfs data attached to gnodes */
struct gn_vfsdata {
	struct gn_vfsdata *gnv_next;	/* next in chain, NULL ends chain */
	struct gnode	*gnv_gnode;	/* pointer back to gnode */
	int	gn_gfstype;		/* gfs type of this vfs */
	/* vfs specific stuff here */
};

/*
 * gnode types
 * DO NOT rearrange/redefine these first 10!
 * VNON means no type.
 */
enum vtype { VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VBAD, VFIFO, VMPC };
#define VUNDEF	VNON		/* undefined is same as nothing */

/*
 * gnode structure - represents a real object in the file system;
 * i.e. there is a 1:1 correspondance between an gnode and a file on a disk.
 */
struct gnode {
	enum vtype gn_type;		/* type of object: VDIR,VREG,... */
	short	gn_flags;		/* attributes of object		*/
	ulong	gn_seg;			/* segment into which file is mapped */
	long	gn_mwrcnt;		/* count of map for write	*/
	long	gn_mrdcnt;		/* count of map for read	*/
	long	gn_rdcnt;		/* total opens for read		*/
	long	gn_wrcnt;		/* total opens for write	*/
	long	gn_excnt;		/* total opens for exec		*/
	long	gn_rshcnt;		/* total opens for read share	*/
	struct vnodeops *gn_ops;
	struct vnode *gn_vnode;	/* ptr to list of vnodes per this gnode */
	dev_t	gn_rdev;	/* for devices, their "dev_t" */
	chan_t	gn_chan;	/* for devices, their "chan", minor's minor */

	Simple_lock	gn_reclk_lock;  /* lock for filocks list        */
	int		gn_reclk_event; /* event list for file locking  */
	struct filock  *gn_filocks;     /* locked region list           */

	caddr_t	gn_data;	/* ptr to private data (usually contiguous) */
};

/*
 * definitions of gn_flags
 */
#define	GNF_TCB		0x0001	/* gnode corresponds to a file in the TCB */
#define	GNF_WMAP	0x0002	/* mapped for writing at some time	*/
#define GNF_NSHARE	0x0004	/* opened non share			*/

/*
 * some defines so old things keep working
 */
#define gn_segcnt	gn_mwrcnt
#define v_type		v_gnode->gn_type
#define v_vntype	v_gnode->gn_type
#define v_rdev		v_gnode->gn_rdev
#define v_data		v_gnode->gn_data

/*
 * track device opens for mount with gnode exec count.
 */
#define gn_mntcnt	gn_excnt

/*
 * Structure containing operations vector
 */
struct vnodeops {
	/* creation/naming/deletion */
	int	(*vn_link)(struct vnode *, struct vnode *, char *,
			struct ucred *);
	int	(*vn_mkdir)(struct vnode *, char *, int, struct ucred *);
	int	(*vn_mknod)(struct vnode *, caddr_t, int,
			dev_t, struct ucred *);
	int	(*vn_remove)(struct vnode *, struct vnode *, char *,
			struct ucred *);
	int	(*vn_rename)(struct vnode *, struct vnode *, caddr_t, 
			struct vnode *,struct vnode *,caddr_t,struct ucred *);
	int	(*vn_rmdir)(struct vnode *, struct vnode *, char *,
			struct ucred *);
	/* lookup,  file handle stuff */
	int	(*vn_lookup)(struct vnode *, struct vnode **, char *, int,
			struct vattr *, struct ucred *);
	int	(*vn_fid)(struct vnode *, struct fileid *, struct ucred *);
	/* access to files */
	int	(*vn_open)(struct vnode *, int, int, caddr_t *, struct ucred *);
	int	(*vn_create)(struct vnode *, struct vnode **, int, caddr_t,
			int, caddr_t *, struct ucred *);
	int	(*vn_hold)(struct vnode *);
	int	(*vn_rele)(struct vnode *);
	int	(*vn_close)(struct vnode *, int, caddr_t, struct ucred *);
	int	(*vn_map)(struct vnode *, caddr_t, uint, uint, uint,
			struct ucred *);
	int	(*vn_unmap)(struct vnode *, int, struct ucred *);
	/* manipulate attributes of files */
	int	(*vn_access)(struct vnode *, int, int, struct ucred *);
	int	(*vn_getattr)(struct vnode *, struct vattr *, struct ucred *);
	int	(*vn_setattr)(struct vnode *, int, int, int, int,
			struct ucred *);
	/* data update operations */
#ifdef _LONG_LONG
	int	(*vn_fclear)(struct vnode *, int, offset_t, offset_t, 
			caddr_t, struct ucred *);
#else
	int	(*vn_fclear)();
#endif
	int	(*vn_fsync)(struct vnode *, int, int, struct ucred *);

#ifdef _LONG_LONG
	int	(*vn_ftrunc)(struct vnode *, int, offset_t, caddr_t,
			struct ucred *);
#else
	int	(*vn_ftrunc)();
#endif
	int	(*vn_rdwr)(struct vnode *, enum uio_rw, int, struct uio *,
			int, caddr_t, struct vattr *, struct ucred *);
#ifdef _LONG_LONG
	int	(*vn_lockctl)(struct vnode *, offset_t, struct eflock *, int, 
			int (*)(), ulong *, struct ucred *);
#else
	int	(*vn_lockctl)();
#endif
	/* extensions */
	int	(*vn_ioctl)(struct vnode *, int, caddr_t, size_t, int,
			struct ucred *);
	int	(*vn_readlink)(struct vnode *, struct uio *, struct ucred *);
	int	(*vn_select)(struct vnode *, int, ushort, ushort *, void (*)(),
			caddr_t, struct ucred *);
	int	(*vn_symlink)(struct vnode *, char *, char *, struct ucred *);
	int	(*vn_readdir)(struct vnode *, struct uio *, struct ucred *);
	/* buffer ops */
	int	(*vn_strategy)(struct vnode *, struct buf *, struct ucred *);
	/* security things */
	int	(*vn_revoke)(struct vnode *, int, int, struct vattr *,
			struct ucred *);
	int	(*vn_getacl)(struct vnode *, struct uio *, struct ucred *);
	int	(*vn_setacl)(struct vnode *, struct uio *, struct ucred *);
	int	(*vn_getpcl)(struct vnode *, struct uio *, struct ucred *);
	int	(*vn_setpcl)(struct vnode *, struct uio *, struct ucred *);
};

#ifdef _KERNEL

/*
 * Macros for the vnode operations.
 */
#define VNOP_ACCESS(vp, mode, who, ucred) \
	vnop_access(vp, mode, who, ucred)
#define VNOP_CLOSE(vp, flags, vinfo, ucred) \
	vnop_close(vp, flags, vinfo, ucred)
#define VNOP_CREATE(dp, vpp, flags, name, mode, vinfo, ucred) \
	vnop_create(dp, vpp, flags, name, mode, vinfo, ucred)
#define VNOP_FCLEAR(vp, flags, offset, length, vinfo, ucred) \
	vnop_fclear(vp, flags, offset, length, vinfo, ucred)
#define VNOP_FID(vp, fid, ucred) \
	vnop_fid(vp, fid, ucred)
#define VNOP_FSYNC(vp, flags, vinfo, ucred) \
	vnop_fsync(vp, flags, vinfo, ucred)
#define VNOP_FTRUNC(vp, flags, length, vinfo, ucred) \
	vnop_ftrunc(vp, flags, length, vinfo, ucred)
#define VNOP_GETACL(vp, uiop, ucred) \
	vnop_getacl(vp, uiop, ucred)
#define VNOP_GETATTR(vp, vattrp, ucred) \
	vnop_getattr(vp, vattrp, ucred)
#define VNOP_GETPCL(vp, uiop, ucred) \
	vnop_getpcl(vp, uiop, ucred)
#define VNOP_HOLD(vp) \
	vnop_hold(vp)
#define VNOP_IOCTL(vp, cmd, arg, flags, ext, ucred) \
	vnop_ioctl(vp, cmd, arg, flags, ext, ucred)
#define VNOP_LINK(vp, dp, target, ucred) \
	vnop_link(vp, dp, target, ucred)
#define VNOP_LOCKCTL(vp, offset, lckdat, cmd, retry_fcn, retry_id, ucred) \
	vnop_lockctl(vp, offset, lckdat, cmd, retry_fcn, retry_id, ucred)
#define VNOP_LOOKUP(dp, vpp, nam, flags, vattrp, ucred) \
	vnop_lookup(dp, vpp, nam, flags, vattrp, ucred)
#define VNOP_MAP(vp, addr, length, offset, flags, ucred) \
	vnop_map(vp, addr, length, offset, flags, ucred)
#define VNOP_MKDIR(vp, name, mode, ucred) \
	vnop_mkdir(vp, name, mode, ucred)
#define VNOP_MKNOD(vp, name, mode, dev, ucred) \
	vnop_mknod(vp, name, mode, dev, ucred)
#define VNOP_OPEN(vp, flags, ext, vinfop, ucred) \
	vnop_open(vp, flags, ext, vinfop, ucred)
#define VNOP_SELECT(vp, correl, reqevents, rtnevents, notify, vinfo, ucred) \
	vnop_select(vp, correl, reqevents, rtnevents, notify, vinfo, ucred)
#define VNOP_RDWR(vp, op, flags, uiop, ext, vinfo, vattrp, ucred) \
	vnop_rdwr(vp, op, flags, uiop, ext, vinfo, vattrp, ucred)
#define VNOP_READDIR(vp, uiop, ucred) \
	vnop_readdir(vp, uiop, ucred)
#define VNOP_READLINK(vp, uiop, ucred) \
	vnop_readlink(vp, uiop, ucred)
#define VNOP_RELE(vp) \
	vnop_rele(vp)
#define VNOP_REMOVE(dp, vp, name, ucred) \
	vnop_remove(dp, vp, name, ucred)
#define VNOP_RENAME(vp, dp, name, tvp, tdp, tname, ucred) \
	vnop_rename(vp, dp, name, tvp, tdp, tname, ucred)
#define VNOP_REVOKE(vp, cmd, flags, vattrp, ucred) \
	vnop_revoke(vp, cmd, flags, vattrp, ucred)
#define VNOP_RMDIR(vp, dp, name, ucred) \
	vnop_rmdir(vp, dp, name, ucred)
#define VNOP_SETACL(vp, uiop, ucred) \
	vnop_setacl(vp, uiop, ucred)
#define VNOP_SETATTR(vp, op, arg1, arg2, arg3, ucred) \
	vnop_setattr(vp, op, arg1, arg2, arg3, ucred)
#define VNOP_SETPCL(vp, uiop, ucred) \
	vnop_setpcl(vp, uiop, ucred)
#define VNOP_STRATEGY(vp, bp, ucred) \
	vnop_strategy(vp, bp, ucred)
#define VNOP_SYMLINK(vp, name, target, ucred) \
	vnop_symlink(vp, name, target, ucred)
#define VNOP_UNMAP(vp, addr, ucred) \
	vnop_unmap(vp, addr, ucred)

/*
 * Function prototypes for vnode op wrapper functions.
 */
int
vnop_access(
	struct vnode *,
	int,
	int,
	struct ucred *);

int
vnop_close(
	struct vnode *,
	int,
	caddr_t,
	struct ucred *);

int
vnop_create(
	struct vnode *,
	struct vnode **,
	int,
	char *,
	int,
	caddr_t *,
	struct ucred *);

#ifdef	_LONG_LONG
int
vnop_fclear(
	struct vnode *,
	int,
	offset_t,
	offset_t,
	char *,
	struct ucred *);
#else
int
vnop_fclear();
#endif

int
vnop_fid(
	struct vnode *,
	struct fileid *,
	struct ucred *);

int
vnop_fsync(
	struct vnode *,
	int,
	int,
	struct ucred *);

#ifdef	_LONG_LONG
int
vnop_ftrunc(
	struct vnode *,
	int,
	offset_t,
	caddr_t,
	struct ucred *);
#else
int
vnop_ftrunc();
#endif

int
vnop_getacl(
	struct vnode *,
	struct uio *,
	struct ucred *);

int
vnop_getattr(
	struct vnode *,
	struct vattr *,
	struct ucred *);

int
vnop_getpcl(
	struct vnode *,
	struct uio *,
	struct ucred *);

int
vnop_hold(
	struct vnode *);

int
vnop_ioctl(
	struct vnode *,
	int, 
	caddr_t, 
	size_t,
	int,
	struct ucred *);

int
vnop_link(
	struct vnode *,
	struct vnode *,
	char *,
	struct ucred *);

#ifdef	_LONG_LONG
int
vnop_lockctl(
	struct vnode *,
	offset_t,
	struct eflock *,
	int,
	int (*)(),
	ulong *,
	struct ucred *);
#else
int
vnop_lockctl();
#endif

int
vnop_lookup(
	struct vnode *,
	struct vnode **,
	char *,
	int,
	struct vattr *,
	struct ucred *);

int
vnop_map(
	struct vnode *,
	caddr_t,
	uint,
	uint,
	uint,
	struct ucred *);

int
vnop_mkdir(
	struct vnode *,
	char *,
	int,
	struct ucred *);

int
vnop_mknod(
	struct vnode *,
	caddr_t,
	int,
	dev_t,
	struct ucred *);

int
vnop_open(
	struct vnode *,
	int,
	int,
	caddr_t *,
	struct ucred *);

int
vnop_select(
	struct vnode *,
	int,
	ushort,
	ushort *,
	void (*)(),
	caddr_t,
	struct ucred *);

int
vnop_rdwr(
	struct vnode *,
	enum uio_rw,
	int,
	struct uio *,
	int,
	caddr_t,
	struct vattr *,
	struct ucred *);

int
vnop_readdir(
	struct vnode *,
	struct uio *,
	struct ucred *);

int
vnop_readlink(
	struct vnode *,
	struct uio *,
	struct ucred *);

int
vnop_rele(
	struct vnode *);

int
vnop_remove(
	struct vnode *,
	struct vnode *,
	char *,
	struct ucred *);

int
vnop_rename(
	struct vnode *,
	struct vnode *,
	caddr_t,
	struct vnode *,
	struct vnode *,
	caddr_t,
	struct ucred *);

int
vnop_revoke(
	struct vnode *,
	int,
	int,
	struct vattr *,
	struct ucred *);

int
vnop_rmdir(
	struct vnode *,
	struct vnode *,
	char *,
	struct ucred *);

int
vnop_setacl(
	struct vnode *,
	struct uio *,
	struct ucred *);

int
vnop_setattr(
	struct vnode *,
	int,
	int,
	int,
	int,
	struct ucred *);

int
vnop_setpcl(
	struct vnode *,
	struct uio *,
	struct ucred *);

int
vnop_strategy(
	struct vnode *,
	struct buf *,
	struct ucred *);

int
vnop_symlink(
	struct vnode *,
	char *,
	char *,
	struct ucred *);

int
vnop_unmap(
	struct vnode *,
	int,
	struct ucred *);

#endif /*_KERNEL*/


#ifdef _SUN
/* for Sun vnode compatibility, they expect vattr to be in vnode.h */
#include <sys/vattr.h>
enum vcexcl	{ NONEXCL, EXCL};	/* (non)excl create (create) */
#endif /* _SUN */

/*
 * Convert inode formats to vnode types
 */
extern enum vtype iftovt_tab[];		/* these are located in stat.c */
extern int   vttoif_tab[];

#define IFTOVT(M)	(iftovt_tab[((M) & S_IFMT) >> 12])
#define VTTOIF(T)	(vttoif_tab[(int)(T)])

#define MAKEIMODE(T, M)	(VTTOIF(T) | (M))

#define VTOGP(x)	((struct gnode *)((x)->v_gnode))

#endif /* _H_VNODE */
