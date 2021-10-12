static char sccsid[] = "@(#)65	1.8  src/bos/kernel/lfs/vnops.c, syslfs, bos411, 9428A410j 7/7/94 16:55:34";
/*
 *   COMPONENT_NAME: SYSLFS
 *
 *   FUNCTIONS: vnop_access
 *		vnop_close
 *		vnop_create
 *		vnop_fclear
 *		vnop_fid
 *		vnop_fsync
 *		vnop_ftrunc
 *		vnop_getacl
 *		vnop_getattr
 *		vnop_getpcl
 *		vnop_hold
 *		vnop_ioctl
 *		vnop_link
 *		vnop_lockctl
 *		vnop_lookup
 *		vnop_map
 *		vnop_mkdir
 *		vnop_mknod
 *		vnop_open
 *		vnop_rdwr
 *		vnop_readdir
 *		vnop_readlink
 *		vnop_rele
 *		vnop_remove
 *		vnop_rename
 *		vnop_revoke
 *		vnop_rmdir
 *		vnop_select
 *		vnop_setacl
 *		vnop_setattr
 *		vnop_setpcl
 *		vnop_strategy
 *		vnop_symlink
 *		vnop_unmap
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "sys/types.h"
#include "sys/gfs.h"
#include "sys/vfs.h"
#include "sys/vnode.h"

#ifdef	_FSDEBUG
/*
 * This structure contains counts of the number of times
 * each vnode operation is called.
 */
struct {
	int access;	/*  0 */
	int close;	/*  1 */
	int create;	/*  2 */
	int fclear;	/*  3 */
	int fid;	/*  4 */
	int fsync;	/*  5 */
	int ftrunc;	/*  6 */
	int getacl;	/*  7 */
	int getattr;	/*  8 */
	int getpcl;	/*  9 */
	int hold;	/* 10 */
	int ioctl;	/* 11 */
	int link;	/* 12 */
	int lockctl;	/* 13 */
	int lookup;	/* 14 */
	int map;	/* 15 */
	int mkdir;	/* 16 */
	int mknod;	/* 17 */
	int open;	/* 18 */
	int rdwr;	/* 19 */
	int readdir;	/* 20 */
	int readlink;	/* 21 */
	int rele;	/* 22 */
	int remove;	/* 23 */
	int rename;	/* 24 */
	int revoke;	/* 25 */
	int rmdir;	/* 26 */
	int select;	/* 27 */
	int setacl;	/* 28 */
	int setattr;	/* 29 */
	int setpcl;	/* 30 */
	int strategy;	/* 31 */
	int symlink;	/* 32 */
	int unmap;	/* 33 */
} vnop_cnt;

#define VNOP_CNT(OP)	fetch_and_add(&vnop_cnt.OP, 1)
#else	/* _FSDEBUG */
#define VNOP_CNT(OP)
#endif	/* _FSDEBUG */

/*
 * NAME:	vnop_access
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_access(struct vnode *vp,                  	/* Source vnode */
	    int          mode,                 	/* Mode(s) to check */
	    int          who,
	    struct ucred *crp)
{
	VNOP_CNT(access);
	return (*vp->v_gnode->gn_ops->vn_access)(vp, mode, who, crp);
}

/*
 * NAME:	vnop_close
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_close(struct vnode *vp,            /* vnode pointer                */
	   int          flags,          /* flags from the file pointer  */
	   caddr_t      vinfo,          /* if remote                    */
	   struct ucred *crp)		/* credentials pointer          */
{
	VNOP_CNT(close);
	return (*vp->v_gnode->gn_ops->vn_close)(vp, flags, vinfo, crp);
}

/*
 * NAME:	vnop_create
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_create(struct vnode *dp,	/* parent directory vnode pointer       */
	    struct vnode **vpp, /* new vnode                            */
	    int flags,          /* flag from open file pointer          */
	    caddr_t name,       /* contains the name of the new file    */
	    int mode,           /* contains the permission modes for the file */
	    caddr_t *vinfo,     /* the vinfo from file pointer          */
	    struct ucred *crp)	/* credentials pointer			*/
{
	int rc;

	VNOP_CNT(create);
 	rc = (*(dp)->v_gnode->gn_ops->vn_create)
			(dp,vpp,flags,name,mode,vinfo,crp);
 	if (audit_flag)
		aud_vn_create(dp,vpp,flags,name,mode,vinfo,crp);
	return rc;
}

/*
 * NAME:	vnop_fclear
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_fclear(struct vnode *vp,   /* Vnode of open file           */
	    int flags,          /* Open flags                   */
	    offset_t offset,    /* Offset to begin              */
	    offset_t len,       /* # of bytes to clear          */
	    caddr_t vinfo,      /* Something if remote          */
	    struct ucred *crp)	/* credentials pointer		*/
{
	VNOP_CNT(fclear);
	return (*vp->v_gnode->gn_ops->vn_fclear)(vp,flags,offset,len,vinfo,crp);
}

/*
 * NAME:	vnop_fid
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_fid(struct vnode *vp,
	 struct fileid *fid,
	 struct ucred *crp)
{
	VNOP_CNT(fid);
	return (*vp->v_gnode->gn_ops->vn_fid)(vp,fid,crp);
}

/*
 * NAME:	vnop_fsync
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_fsync(struct vnode *vp,               /* vnode pointer                */
	   int flags,                      /* open flags                   */
	   int vinfo,                      /* fd used for mapped files     */
	   struct ucred *crp)		   /* credentials pointer          */
{
	VNOP_CNT(fsync);
	return (*vp->v_gnode->gn_ops->vn_fsync)(vp,flags,vinfo,crp);
}

/*
 * NAME:	vnop_ftrunc
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_ftrunc(struct vnode *vp, /* Vnode for open file                  */
	    int flags,        /* Open flags                           */
	    offset_t length,  /* New length                           */
	    caddr_t vinfo,    /* Gfs specific inofo                   */
	    struct ucred *crp)	/* credentials pointer		      */
{
	VNOP_CNT(ftrunc);
	return (*vp->v_gnode->gn_ops->vn_ftrunc)(vp,flags,length,vinfo,crp);
}

/*
 * NAME:	vnop_getacl
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_getacl(struct vnode    *vp,
	    struct uio      *uiop,
	    struct ucred    *crp)
{
	VNOP_CNT(getacl);
	return (*vp->v_gnode->gn_ops->vn_getacl)(vp,uiop,crp);
}

/*
 * NAME:	vnop_getattr
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_getattr(struct vnode *vp,
	     struct vattr *vattr,
	     struct ucred *crp)
{
	VNOP_CNT(getattr);
	return (*vp->v_gnode->gn_ops->vn_getattr)(vp,vattr,crp);
}

/*
 * NAME:	vnop_getpcl
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_getpcl(struct vnode *vp,
	    struct uio   *uiop,
	    struct ucred *crp)
{
	VNOP_CNT(getpcl);
	return (*vp->v_gnode->gn_ops->vn_getpcl)(vp,uiop,crp);
}

/*
 * NAME:	vnop_hold
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_hold(struct vnode *vp)
{
	VNOP_CNT(hold);
	return (*vp->v_gnode->gn_ops->vn_hold)(vp);
}

/*
 * NAME:	vnop_ioctl
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_ioctl(struct vnode *vp,
	   int cmd,
	   caddr_t arg,
	   size_t flags,
	   int	ext,
	   struct ucred *crp)
{
	VNOP_CNT(ioctl);
	return (*vp->v_gnode->gn_ops->vn_ioctl)(vp,cmd,arg,flags,ext,crp);
}

/*
 * NAME:	vnop_link
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_link(struct vnode *vp,
	  struct vnode *dp,
	  char *tnm,
	  struct ucred *crp)
{
	int rc;

	VNOP_CNT(link);
	rc = (*vp->v_gnode->gn_ops->vn_link)(vp,dp,tnm,crp);
	if (audit_flag)
		aud_vn_link (vp, dp, tnm, crp);
	return rc;
}

/*
 * NAME:	vnop_lockctl
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_lockctl(struct vnode *vp,
	     offset_t   offset,
	     struct  eflock *lckdat,
	     int     cmd,
	     int     (* retry_fcn)(),
	     ulong   *retry_id,
	     struct ucred *crp)
{
	VNOP_CNT(lockctl);
	return (*vp->v_gnode->gn_ops->vn_lockctl)(vp, offset, lckdat, cmd,
						  retry_fcn, retry_id, crp);
}

/*
 * NAME:	vnop_lookup
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_lookup(struct vnode	*dp,
	    struct vnode	**vpp,
	    char		*nam,
	    int			flag,
	    struct vattr	*vattrp,
	    struct ucred	*crp)
{
	VNOP_CNT(lookup);
	return (*(dp)->v_gnode->gn_ops->vn_lookup)(dp,vpp,nam,flag,vattrp,crp);
}

/*
 * NAME:	vnop_map
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_map(struct vnode *vp,
	 caddr_t addr,
	 uint length,
	 uint offset,
	 uint flag,
	 struct ucred *crp)
{
	VNOP_CNT(map);
	return (*vp->v_gnode->gn_ops->vn_map)(vp,addr,length,offset,flag,crp);
}

/*
 * NAME:	vnop_mkdir
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_mkdir(struct vnode  *vp,
	   char *nam,
	   int  mode,
	   struct ucred *crp)
{
	int rc;

	VNOP_CNT(mkdir);
	rc = (*vp->v_gnode->gn_ops->vn_mkdir)(vp,nam,mode,crp);
	if (audit_flag)
		aud_vn_mkdir (vp, nam, mode, crp);
	return rc;
}

/*
 * NAME:	vnop_mknod
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_mknod(struct vnode *vp,
	   caddr_t nm,
	   int mode,
	   dev_t dev,
	   struct ucred *crp)
{
	int rc;

	VNOP_CNT(mknod);
	rc = (*vp->v_gnode->gn_ops->vn_mknod)(vp,nm,mode,dev,crp);
	if (audit_flag)
		aud_vn_mknod (vp, nm, mode, dev, crp);
	return rc;
}

/*
 * NAME:	vnop_open
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_open(struct vnode *vp,               /* Vnode of file to open        */
	  int     flags,                  /* Open(2) flags                */
	  int     ext,                    /* Extended info for devs       */
	  caddr_t *vinfop,                /* gfs specific pointer         */
	  struct ucred *crp)
{
	VNOP_CNT(open);
	return (*vp->v_gnode->gn_ops->vn_open)(vp,flags,ext,vinfop,crp);
}

/*
 * NAME:	vnop_select
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_select(struct vnode *vp,
	    int corl,
	    ushort reqevents,
	    ushort *rtneventsp,
	    void (*notify)(),
	    caddr_t vinfo,
	    struct ucred *crp)
{
	VNOP_CNT(select);
	return (*vp->v_gnode->gn_ops->vn_select)
		(vp, corl, reqevents, rtneventsp, notify, vinfo, crp);
}

/*
 * NAME:	vnop_rdwr
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_rdwr(struct vnode    *vp,            /* Vnode for open file          */
	  enum uio_rw     op,             /* Read or write                */
	  int             flags,          /* Open flags                   */
	  struct uio      *uiop,          /* Uio info                     */
	  int             ext,            /* Extended open ?              */
	  caddr_t         vinfo,          /* if remote                    */
	  struct vattr	  *vattrp,	  /* attributes to be returned	  */
	  struct ucred    *crp)		  /* credentials pointer	  */
{
	VNOP_CNT(rdwr);
	if (audit_flag)
		aud_vn_rdwr(vp,op,flags,uiop,ext,vinfo,crp);
	return (*vp->v_gnode->gn_ops->vn_rdwr)
			(vp, op, flags, uiop, ext, vinfo, vattrp, crp);
}

/*
 * NAME:	vnop_readdir
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_readdir(struct vnode *vp,
	     struct uio *uiop,
	     struct ucred *crp)
{
	int rc;

	VNOP_CNT(readdir);
	rc = (*vp->v_gnode->gn_ops->vn_readdir)(vp,uiop,crp);
	if (audit_flag)
		aud_vn_readdir (vp, uiop, crp);
	return rc;
}

/*
 * NAME:	vnop_readlink
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_readlink(struct vnode *vp,		/* VLNK vnode                   */
	      struct uio *uiop,		/* Uio iformation               */
	      struct ucred *crp)	/* credentials pointer		*/
{
	VNOP_CNT(readlink);
	return (*vp->v_gnode->gn_ops->vn_readlink)(vp,uiop,crp);
}

/*
 * NAME:	vnop_rele
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_rele(struct vnode *vp)
{
	VNOP_CNT(rele);
	return (*vp->v_gnode->gn_ops->vn_rele)(vp);
}

/*
 * NAME:	vnop_remove
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_remove(struct vnode *dp,		/* To be removed        */
	    struct vnode *vp,		/* Its parent           */
	    char *nm,			/* Its name             */
	    struct ucred *crp)		/* credentials pointer	*/
{
	int rc;

	VNOP_CNT(remove);
	rc = (*(dp)->v_gnode->gn_ops->vn_remove)(dp,vp,nm,crp);
	if (audit_flag)
		aud_vn_remove (dp,vp,nm,crp);
	return rc;
}

/*
 * NAME:	vnop_rename
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_rename(struct vnode *vp,
	    struct vnode *dp,
	    caddr_t nm,
	    struct vnode *tp,
	    struct vnode *tdp,
	    caddr_t tnm,
	    struct ucred *crp)
{
	int rc;

	VNOP_CNT(rename);
	rc = (*vp->v_gnode->gn_ops->vn_rename)(vp,dp,nm,tp,tdp,tnm,crp);
	if (audit_flag)
		aud_vn_rename (vp, dp, nm, tp, tdp, tnm, crp);
	return rc;
}

/*
 * NAME:	vnop_revoke
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_revoke(struct vnode  *vp,
	    int     cmd,
	    int     flags,
	    struct vattr *vattrp,
	    struct ucred *crp)
{
	VNOP_CNT(revoke);
	return (*vp->v_gnode->gn_ops->vn_revoke)(vp,cmd,flags,vattrp,crp);
}

/*
 * NAME:	vnop_rmdir
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_rmdir(struct vnode  *vp,
	   struct vnode  *dp,
	   char *nm,
	   struct ucred *crp)
{
	int rc;

	VNOP_CNT(rmdir);
	rc = (*vp->v_gnode->gn_ops->vn_rmdir)(vp,dp,nm,crp);
	if (audit_flag)
		aud_vn_rmdir (vp, dp, nm, crp);
	return rc;
}

/*
 * NAME:	vnop_setacl
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_setacl(struct vnode    *vp,
	    struct uio      *uiop,
	    struct ucred    *crp)
{
	VNOP_CNT(setacl);
	return (*vp->v_gnode->gn_ops->vn_setacl)(vp,uiop,crp);
}

/*
 * NAME:	vnop_setattr
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_setattr(struct vnode *vp,
	     int op,
	     int arg1, 
	     int arg2, 
	     int arg3,
	     struct ucred *crp)
{
	VNOP_CNT(setattr);
	return (*vp->v_gnode->gn_ops->vn_setattr)(vp,op,arg1,arg2,arg3,crp);
}

/*
 * NAME:	vnop_setpcl
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_setpcl(struct vnode *vp,
	    struct uio   *uiop,
	    struct ucred *crp)
{
	VNOP_CNT(setpcl);
	return (*vp->v_gnode->gn_ops->vn_setpcl)(vp,uiop,crp);
}

/*
 * NAME:	vnop_strategy
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_strategy(struct vnode *vp,
	      struct buf *bp,
	      struct ucred *crp)
{
	VNOP_CNT(strategy);
	return (*vp->v_gnode->gn_ops->vn_strategy)(vp,bp,crp);
}

/*
 * NAME:	vnop_symlink
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_symlink(struct vnode    *vp,
	     char            *lnm,
	     char            *tnm,
	     struct ucred    *crp)
{
	int rc;

	VNOP_CNT(symlink);
	rc = (*vp->v_gnode->gn_ops->vn_symlink)(vp,lnm,tnm,crp);
	if (audit_flag)
		aud_vn_symlink (vp,lnm,tnm,crp);
	return rc;
}

/*
 * NAME:	vnop_unmap
 *
 * RETURNS: 	Returns the underlying vnode operation return code.
 */
int
vnop_unmap(struct vnode *vp,    /* vnode to unmap               */
	   int addr,            /* flags to unmap call          */
	   struct ucred *crp)	/* credentials pointer		*/
{
	VNOP_CNT(unmap);
	return (*vp->v_gnode->gn_ops->vn_unmap)(vp,addr,crp);
}
