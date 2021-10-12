static char sccsid[] = "@(#)09	1.10.1.18  src/bos/kernext/cfs/cdr_vnops.c, sysxcfs, bos41J, 9508A 2/16/95 15:08:10";
/*
 * COMPONENT_NAME: (SYSXCFS) CDROM File System
 *
 * FUNCTIONS:  CDRFS vnode opeartions
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Define the CDROM file system so that the standard header files
 * will include the CDROM file system specific headers.
*/
#define _CDRFS

/* standard includes */
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/buf.h>
#include <sys/vfs.h>
#include <sys/statfs.h>
#include <sys/vnode.h>
#include <sys/vattr.h>
#include <sys/shm.h>
#include <sys/acl.h>
#include <sys/intr.h>
#include <sys/device.h>
#include <sys/utsname.h>
#include "fcntl.h"
#include <sys/flock.h>
#include <sys/dir.h>
#include <sys/inode.h>		/* should be <jfs/inode.h> */
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/trchkid.h>

/* CD-ROM filesystem includes */
#include "cdr_xcdr.h"
#include "cdr_rrg.h"
#include "cdr_xa.h"
#include "cdr_cdrfs.h"
#include "cdr_cdrnode.h"

/* Declare cdr_inval() and cdr_stale() here without a prototype so that
 * they are not checked against vnode function prototypes to which it is
 * assigned.
 */
static int cdr_inval();
static int cdr_stale();

/* declarations for internal functions */
static
int
cdr_access (
	struct vnode *		vp,
	int			mode,
	int			who,
	struct ucred *		crp);
static
int
cdr_close (
	struct vnode *		vp,
	int			flag,
	caddr_t			vinfo,
	struct ucred *		crp);
static
int
cdr_fid (
	struct vnode *		vp,
	struct cdrfileid *	fidp,
	struct ucred *		crp);
static
int
cdr_fsync (
	struct vnode *		vp,
	int			flags,
	int			fd,
	struct ucred *		crp);
static
int
cdr_getacl (
	struct vnode *		vp,
	struct uio *		uiop,
	struct ucred *		crp);

static
int
cdr_getattr (
	struct vnode *		vp,
	struct vattr *		vattrp,
	struct ucred *		crp);
static
int
cdr_getpcl (
	struct vnode *		vp,
	struct uio *		uiop,
	struct ucred *		crp);
static
int
cdr_hold (
	struct vnode *		vp);
static
int
cdr_lockctl (
	struct vnode *		vp,
	offset_t		offset,
	struct eflock *		lckdat,
	int			cmd,
	int 			(*retry_fcn)(),
	caddr_t			retry_id,
	struct ucred *		crp);
static
int
cdr_lookup (
	struct vnode *		dvp,
	struct vnode **		vpp,
	char *			pname,
	int			flag,
	struct vattr *		vattrp,
	struct ucred *		crp);
static
int
cdr_map (
	struct vnode *		vp,
	caddr_t			addr,
	off_t			length,
	off_t			offset,
	unsigned int		request,
	struct ucred *		crp);
static
int
cdr_open (
	struct vnode *		vp,
	int			flag,
	int			ext,
	caddr_t *		vinfop,
	struct ucred *		crp);

static
int
cdr_rdwr (
	struct vnode *		vp,
	enum uio_rw		rw,
	int			flags,
	struct uio *		uiop,
	int			ext,
	caddr_t			vinfo,
	struct vattr *		vattrp,
	struct ucred *		crp);
static
int
cdr_readdir (
	struct vnode *		vp,
	struct uio *		uiop,
	struct ucred *		crp);
static
int
cdr_readlink (
	struct vnode *		vp,
	struct uio *		uiop,
	struct ucred *		crp);
static
int
cdr_rele (
	struct vnode *		vp);

/* Despite its appearance, the strategy routine is not a vnode op */
int
cdr_strategy (
	struct buf *		bp);
static
int
cdr_unmap (
	struct vnode *		vp,
	int			flag,
	struct ucred *		crp);

/* cdrnode vnode operations */
struct vnodeops cdr_vops = {
	cdr_inval,	/* cdr_link */
	cdr_inval,	/* cdr_mkdir */
	cdr_inval,	/* cdr_mknod */
	cdr_inval,	/* cdr_remove */
	cdr_inval,	/* cdr_rename */
	cdr_inval,	/* cdr_rmdir */
	cdr_lookup,
	cdr_fid,
	cdr_open,
	cdr_inval,	/* cdr_create */
	cdr_hold,
	cdr_rele,
	cdr_close,
	cdr_map,
	cdr_unmap,
	cdr_access,
	cdr_getattr,
	cdr_inval,	/* cdr_setattr */
	cdr_inval,	/* cdr_fclear */
	cdr_fsync,
	cdr_inval,	/* cdr_ftrunc */
	cdr_rdwr,
	cdr_lockctl,
	cdr_inval,	/* cdr_ioctl */
	cdr_readlink,
	cdr_inval,	/* cdr_select */
	cdr_inval,	/* cdr_symlink */
	cdr_readdir,
	cdr_strategy,
	cdr_inval,	/* cdr_hangup */
	cdr_getacl,
	cdr_inval,	/* cdr_setacl */
	cdr_getpcl,
	cdr_inval	/* cdr_setpcl */
};
/* cdrnode vnode operations allowed for stale vnodes */
struct vnodeops stale_cdr_vops = {
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_hold,
	cdr_rele,
	cdr_close,
	cdr_stale,
	cdr_unmap,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_fsync,
	cdr_stale,
	cdr_stale,
	cdr_lockctl,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
	cdr_stale,
};

/* debugger level variable declaration to control BUGLPR() macro */
BUGXDEF(buglevel);

/* stale cdrnode for unmounted vnodes	*/
extern struct cdrnode stale_cdrnode;

/* stale vnode determination macro */
#define stale_vnode(vp)	((vp)->v_gnode == &stale_cdrnode.cn_gnode)

/*
 * NAME:	cdrstale(void)
 *
 * FUNCTION:	This function returns ESTALE.  It is used to occupy
 *		spaces in the vnode operation table which are invalid
 *		for vnodes from a forced unmounted filesystem.
 *
 * PARAMETERS:	none
 *
 * RETURN :	ESTALE	- vnode belongs to forced unmounted filesystem
 */
static
int
cdr_stale (void)
{
	return ESTALE;
}

/*
 * NAME:	cdrinval(void)
 *
 * FUNCTION:	This function returns EINVAL.  It is used to occupy
 *		spaces in the vnode operation table which are invalid
 *		for CD-ROM filesytem vnodes.
 *
 * PARAMETERS:	none
 *
 * RETURN :	EINVAL	- invalid vnode operation for CD-ROM filesystem
 */
static
int
cdr_inval (void)
{
	BUGLPR(buglevel, 9, ("Warning! Invalid cdrom operation attempted!\n"));
	return EINVAL;
}

/*
 * NAME:	cdr_access(vp, mode, who, crp)
 *
 * FUNCTION:	This function checks permissions.
 *
 * PARAMETERS:	vp	- vnode to check permissions on
 *		mode	- permissions to check for in OTH format.
 *		who	- who to check permissions for
 *			  one of: ACC_SELF 
 *				  ACC_OTHERS 
 * 				  ACC_ANY 
 *				  ACC_ALL
 *		crp	- credential
 *
 * RETURNS:	0	- success
 *		errors from subroutines
 *
 * SERIALIZATION: Not needed since we are not modifying cdrnode fields.
 *
 */
static
int
cdr_access (
	struct vnode *		vp,
	int			mode,
	int			who,
	struct ucred		*crp)
{
	int			rc = 0;
	struct cdrnode *	cdrp;

	/* multiple access types for ACC_SELF access only */
	if (who != ACC_SELF)
		switch (mode) {
		case R_ACC:
		case W_ACC:
		case X_ACC:
			break;
		default:
			return EINVAL;
		}

	/* check for write on a read-only file system */
	if (mode & W_ACC)
		return EROFS;

	/* get and lock cdrnode */
	cdrp = VTOCDRP(vp);

	/* check access by type */
	switch (who)
	{
	case ACC_SELF:
		/* regular access check */
		rc = cdraccess(cdrp, mode << USRMODESHIFT, crp);
		break;
	case ACC_OTHERS:
	case ACC_ANY:
		/* instant success if mode bits for "others" allows access */
		if (cdrp->cn_mode & mode)
			break;

		/* If ACC_ANY access is desired, check owner mode for access.
		 */
		if (who == ACC_ANY &&
		    ((cdrp->cn_mode >> OTHMODESHIFT) & mode) == mode)
			break;

		/* check group mode for access */
		if (((cdrp->cn_mode >> GRPMODESHIFT) & mode) == mode)
			break;
		
		rc = EACCES;
		break;
	case ACC_ALL:
		/* Check for any of owner, group, or others being
		 * denied access.
		 */
		if ((!((cdrp->cn_mode >> OTHMODESHIFT) & mode)) ||
		    (!((cdrp->cn_mode >> GRPMODESHIFT) & mode)) ||
		    (!(cdrp->cn_mode & mode)))
			rc = EACCES;
		break;
	default:
		rc = EINVAL;
		break;
	}
	return rc;
}

/*
 * NAME:	cdr_close(vp, flag, ext, vinfo, crp)
 *
 * FUNCTION:	This function performs cleanup required on a vnode close.
 *		We don't need to do anything, so we just return success.
 *
 * PARAMETERS:	ignored
 *
 * RETURN VALUE: 0	- success
 */
static
int
cdr_close (
	struct vnode *		vp,
	int			flag,
	caddr_t			vinfo,
	struct ucred *		crp)
{
	return 0;
}

/*
 * NAME:	cdr_fid(vp, fidp, crp)
 *
 * FUNCTION: 	This function creates a unique identifier for the
 *		CD-ROM filesystem.  The dirent number, parent directory's
 *		dirent number, recording time of the volume, sequence
 *		number of the volume, and the volume identifier are
 *		used to construct the file identifer.
 *
 * PARAMETERS:	vp	- pointer to the vnode that represents the object
 *			  for which a file handle is to be constructed
 *		fidp	- returned file handle
 *		crp	- credential
 *
 * RETURN:	0	- success
 *
 * SERIALIZATION: Not needed since we are not modifying cdrnode fields.
 *
 */
static
int
cdr_fid (
	struct vnode *		vp,	/* vnode for object		*/
	struct cdrfileid *	fidp,	/* file id to fill in		*/
	struct ucred *		crp)	/* cred pointer			*/
{
	struct cdrnode		*cdrp;	/* cdrnode for vp		*/
	struct cdrpvd 		*pvd;	/* primary vol desc		*/
	struct vfs *		vfsp;	/* vnode's virtual file system	*/
	ushort_t		hash;	/* volume hash value		*/
	char *			volname; /* volume name			*/
	register char *		s;	/* fast pointer into vol name	*/
	register int		i;	/* fast counter for loop	*/

	/* Get the cdrnode of the vnode and the primary volume descriptor
	 * associated with the vnode's vfs.
	 */
	BUGLPR(buglevel, 9, ("making fid for NFS\n"));
	cdrp = VTOCDRP(vp);
	vfsp = vp->v_vfsp;
	pvd = CDRVFSDATA(vfsp)->fs_pvd;
	
	/* Fill in the fields of the fileid.  We mark the pdirent of
	 * directories so we will know this when we get the file id back.
	 * cdrget() expects the pdirent to be zero for directories.
	 */
	bzero(fidp, sizeof *fidp);
	fidp->fid_len = MAXFIDSZ;
	fidp->fid_dirent = cdrp->cn_dirent;
	fidp->fid_pdirent = ((cdrp->cn_mode & IFMT) == IFDIR) ?
				cdrp->cn_pdirent | 0x1:
				cdrp->cn_pdirent;

	/* Generate the hash value for the volume.  This hash value is used
	 * to determine with high probability whether the volume we are on
	 * is the same as the volume we are on when the file id is handed
	 * back to us.  The hash value will be saved in the file id and
	 * calculated by the same method when we check it in cdr_vget().
	 */
	if (CDRVFSDATA(vfsp)->fs_format & CDR_ISO9660)
	{
		BUGLPR(buglevel, 9, ("volume sequence number is %d\n", CDRVFSDATA(vfsp)->fs_pvd->pvd_volseqno));
		/* start with the volume sequence number */
		hash = CDRVFSDATA(vfsp)->fs_pvd->pvd_volseqno;

		BUGLPR(buglevel, 9, ("volume name is %s\n", CDRVFSDATA(vfsp)->fs_pvd->pvd_vol_id));
		/* get the volume name */
		volname = CDRVFSDATA(vfsp)->fs_pvd->pvd_vol_id;
	}
	else
	{
		BUGLPR(buglevel, 9, ("volume sequence number is %d\n", HSPVD(CDRVFSDATA(vfsp)->fs_pvd)->pvd_volseqno));
		/* start with the volume sequence number */
		hash = HSPVD(CDRVFSDATA(vfsp)->fs_pvd)->pvd_volseqno;

		BUGLPR(buglevel, 9, ("volume name is %s\n", HSPVD(CDRVFSDATA(vfsp)->fs_pvd)->pvd_vol_id));
		/* get the volume name */
		volname = HSPVD(CDRVFSDATA(vfsp)->fs_pvd)->pvd_vol_id;
	}

	/* Add the volume name to the hash.  Note that this depends upon
	 * the fact that the volume name is on a short integer alignment.
	 * This is guaranteed by malloc() and by the offset within the
	 * volume descriptor.
	 */
	for (		s = volname, i = CDR_VOLIDLEN / sizeof(ushort_t);
			i;
			s += sizeof(ushort_t), --i)
		hash ^= *(ushort_t *)s;

	/* Add the file creation time to the hash, one short at a time.
	 */
	BUGLPR(buglevel, 9, ("volume creation time is 0x%x\n", cdrp->cn_ctime));
	hash ^= *(ushort_t *)&cdrp->cn_ctime;
	hash ^= *(((ushort_t *)&cdrp->cn_ctime) + 1);

	/* save hash value in fileid */
	fidp->fid_hash = hash;

	BUGLPR(buglevel, 9, ("fid length is %d\n", fidp->fid_len));
	BUGLPR(buglevel, 9, ("fid dirent is 0x%x\n", fidp->fid_dirent));
	BUGLPR(buglevel, 9, ("fid parent dirent is 0x%x\n", fidp->fid_pdirent));
	BUGLPR(buglevel, 9, ("fid hash number is 0x%x\n", fidp->fid_hash));
	return 0;
}

/*
 * NAME:	cdr_fsync(vp, flags, fd, crp)
 *
 * FUNCTION:	This function is used to ensure that a file's information
 *		is correctly recorded on disk.  This is always true for
 *		the CD-ROM filesystem (or at least we couldn't do much
 *		about it if it was not true), so we don't do anything and
 *		return success.
 *
 * PARAMETERS:	vp	- pointer to the vnode that represents the file
 *			  to be synced
 *		flags	- file open flags
 *		fd	- file descriptor
 *		crp	- credential
 *
 *
 * RETURN:	0	- success
 */
static
int
cdr_fsync (
	struct vnode *		vp,
	int			flags,
	int			fd,
	struct ucred *		crp)
{
	return 0;
}

/*
 * NAME:	cdr_getacl(vp, uiop, crp)
 *
 * FUNCTION:	This function returns the access control list for a file.
 *		The CD-ROM filesystem does not have access control lists,
 *		so we make one from the file mode.
 *
 * PARAMETERS:	vp 	- vnode that represents the file to get make the
 *				access control list for
 *		uiop	- uio structure describing where to put the
 *				access control list
 *		crp	- credential
 *
 * RETURN :	0	- success
 *		return from subroutines
 *
 * SERIALIZATION: Not needed since we are not modifying cdrnode fields.
 *
 */
static
int
cdr_getacl (
	struct vnode *		vp,		/* vnode for file	*/
	struct uio *		uiop,		/* return loc for acl	*/
	struct ucred *		crp)		/* cred pointer		*/
{
	struct cdrnode *	cdrp;		/* cdrnode of file	*/
	struct acl		acl;		/* access list buffer	*/
	ushort			mode;		/* file permissions	*/
	int			acllen;		/* length of acc list	*/
	int			rc;		/* return code		*/

	/* set up size of the ACL structure to move */
	acllen = ACL_SIZ;

	/* If the user's recieve buffer is smaller than the ACL, try	*/
	/* to inform the user of the necessary size and return ENOSPC.	*/
	if (uiosiz(uiop) < acllen)
	{
		uiop->uio_offset = 0;
		uiop->uio_iov->iov_len = uiop->uio_resid = sizeof acllen;
		if (rc = uiomove(&acllen, sizeof acllen, UIO_READ, uiop))
			return rc;
		else
			return ENOSPC;
	}

	/* get cdrp of file */
	cdrp = VTOCDRP(vp);

	/* fill in fields in access list structure */
	acl.acl_len = acllen;
	mode = cdrp->cn_mode;
	acl.acl_mode = mode;
	acl.u_access = (mode >> 6) & 0x7;
	acl.g_access = (mode >> 3) & 0x7;
	acl.o_access = mode & 0x7;

	/* copy access control list to caller's buffer */
	rc = uiomove(&acl, acllen, UIO_READ, uiop);
	return rc;
}

/*
 * NAME:	cdr_getattr(vp, vattrp, crp)
 *
 * FUNCTION:	This function returns the attributes for an object in a
 *		filesystem independent format.
 *
 * PARAMETERS:	vp 	- vnode that represents the file to stat
 *		vattrp	- attribute information to be filled in
 *		crp	- credential
 *
 * RETURN :	0	- success
 *
 * SERIALIZATION: Not needed since we are not modifying cdrnode fields.
 *
 */
static
int
cdr_getattr (
	struct vnode *		vp,	/* vnode to get attributes for	*/
	struct vattr *		vattrp,	/* attribute struct to fill in	*/
	struct ucred *		crp)	/* cred pointer			*/
{
	struct cdrnode	*cdrp;	/* cdrnode for vnode		*/
	struct cdrfsmount	*cdrfsp;


	/* get and lock cdrnode */
	cdrp = VTOCDRP(vp);

	/* get mounted file system for volume logical block size */
	cdrfsp = cdrp->cn_cdrfs;

	/* fill in time fields */
	vattrp->va_atim = cdrp->cn_atime;
	vattrp->va_mtim = cdrp->cn_mtime;
	vattrp->va_ctim = cdrp->cn_ctime;

	/* fill in volume fields */
	vattrp->va_dev = brdev(cdrp->cn_dev);
	vattrp->va_serialno = cdrp->cn_number;
	vattrp->va_size = cdrp->cn_size;
	vattrp->va_type = IFTOVT(cdrp->cn_mode);
	/* number of blocks in st_blocks unit */
	vattrp->va_blocks = cdrp->cn_nblocks * (cdrfsp->fs_lblksize / DEV_BSIZE);
	vattrp->va_blocksize = cdrfsp->fs_lblksize;

	/* fill in file fields */
	vattrp->va_mode = cdrp->cn_mode;
	vattrp->va_nlink = cdrp->cn_nlink;
	vattrp->va_rdev = cdrp->cn_rdev;
	vattrp->va_nid = xutsname.nid;
	vattrp->va_uid = cdrp->cn_uid;
	vattrp->va_gid = cdrp->cn_gid;
	vattrp->va_chan = 0;

	return 0;
}

/*
 * NAME:	cdr_getpcl(vp, uiop, crp)
 *
 * FUNCTION:	This function returns the privilege control list for a file.
 *		The CD-ROM filesystem does not have privilege control lists,
 *		so we make one from the file mode.
 *
 * PARAMETERS:	vp 	- vnode that represents the file to get make the
 *				privilege control list for
 *		uiop	- uio structure describing where to put the
 *				privilege control list
 * 		crp	- credential
 *
 * RETURN :	0	- success
 *		return from subroutines
 *
 * SERIALIZATION: Not needed since we are not modifying cdrnode fields.
 *
 */
static
int
cdr_getpcl (
	struct vnode *		vp,		/* vnode for file	*/
	struct uio *		uiop,		/* return loc for pcl	*/
	struct ucred *		crp)		/* cred pointer		*/
{
	struct cdrnode *	cdrp;		/* cdrnode of file	*/
	struct pcl		pcl;		/* priv control list	*/
	int			pcllen;		/* length of pcl	*/
	int			rc;		/* return code		*/
	
	/* set up size of the PCL structure to move */
	pcllen = PCL_SIZ;

	/* If the user's recieve buffer is smaller than the PCL, try	*/
	/* to inform the user of the necessary size and return ENOSPC.	*/
	if (uiosiz(uiop) < pcllen)
	{
		uiop->uio_offset = 0;
		uiop->uio_iov->iov_len = uiop->uio_resid = sizeof pcllen;
		if (rc = uiomove(&pcllen, sizeof pcllen, UIO_READ, uiop))
			return rc;
		else
			return ENOSPC;
	}

	/* get cdrp of file */
	cdrp = VTOCDRP(vp);

	/* fill in fields in privilege control list */
	pcllen = pcl.pcl_len = PCL_SIZ;
	pcl.pcl_mode = cdrp->cn_mode;
	bzero(&pcl.pcl_default, sizeof pcl.pcl_default);	/* default pcl	*/

	/* copy privilege control list to caller's buffer */
	rc = uiomove(&pcl, pcl.pcl_len, UIO_READ, uiop);
	return rc;
}

/*
 * NAME:	cdr_hold(vp)
 *
 * FUNCTION:	This function increments the hold count on a vnode.
 *
 * PARAMETERS:	vp	- vnode to hold
 *
 * RETURNS:	0	- success
 *
 * SERIALIZATION: This function can be called with the cdrfs lock held
 *		  as in cdr_mount, or called directly through the VNOP
 *		  switch. We must be holding the cdrfs lock when incrementing
 *		  the count on the vnode.
 */
static
int
cdr_hold (struct vnode *	vp)
{
	int	waslocked;		/* caller has the cdrfs_lock held */

	BUGLPR(buglevel, 9, ("vnode held:  0x%x, count = %d\n", vp, vp->v_count));

	if (!(waslocked = lock_mine(&cdrfs_lock)))
		CDRFS_LOCK();
	vp->v_count++;
	if (!waslocked)
		CDRFS_UNLOCK();
	return 0;
}

/*
 * NAME:	cdr_lockctl(vp, offset, lckdat, cmd, retry_fcn, retry_id, crp)
 *
 * FUNCTION:	This function implements file locks for the CD-ROM filesystem.
 *		Because exclusive locks are only really useful for write
 *		operations (except as synchronization flags), we do not
 *		support their semantics.  We verify that the lock command
 *		is reasonable, return success for setting read (shared)
 *		locks and unlocking, return failure for setting write
 *		(exclusive) locks and unknown lock types, and return a lock
 *		status of unlocked for all get lock requests.
 *
 * PARMETERS:	vp	- vnode for the file to be locked
 *		offset	- where the lock should start
 *		lckdat	- locking details
 *		cmd	- command for locking
 *		retry_fcn	- function that gets called for a sleep locks
 *				list entry when the corresponding blocking
 *				lock is released (for NFS use)
 *		retry_id	-  used to return the identifier that will
 *				be passed as an argument to the retry_fcn
 *				(This return value can be used by the
 * 				caller to correlate this VNOP_LOCKCTL() call
 *				with a later (* retry_fcn)() call.)
 *		crp	- credential
 *
 * RETURNS:	0	- success
 *		EINVAL	- invalid parameters
 *
 * SERIALIZATION: Not needed since we are not modifying cdrnode fields.
 *
 */
static
int
cdr_lockctl (
	struct vnode *		vp,	/* vnode for file to be locked	*/
	offset_t		offset,	/* where the lock should start	*/
	struct eflock *		lckdat,	/* locking details		*/
	int			cmd,	/* command for locking		*/
	int 			(*retry_fcn)(),	/* sleep retry function	*/
	caddr_t			retry_id,	/* arg to retry func	*/
	struct ucred *		crp)	/* cred pointer			*/
{

	if (stale_vnode(vp))
	{
		/* For stale vnodes, only unlocking is allowed. */
		if ((cmd == SETFLCK || cmd == (SETFLCK | SLPFLCK)) &&
				lckdat->l_type == F_UNLCK)
			return 0;
		else
			return ESTALE;
	}

	if (cmd == 0)
	{
		/* Return a status of unlocked for all get lock requests.
		 * This is reasonable because we should not get write
		 * lock requests and we always succeed on read lock
		 * requests.
		 */
		lckdat->l_type = F_UNLCK;
		return 0;
	}
	/* verify a reasonable lock command */
	else if (cmd != SETFLCK && cmd != (SETFLCK | SLPFLCK))
		return EINVAL;
	switch (lckdat->l_type)
	{
		/* Return success on attempts to set read locks or
		 * unlock.  This is reasonable because we do not
		 * allow write locks which would cause these to block.
		 */
		case F_RDLCK:
		case F_UNLCK:
			return 0;
		/* Return error on attempts to set write locks or
		 * bad lock types.  Write locks should never make it to
		 * here because the LFS should catch write lock requests
		 * on read-only opens (the only kind we allow).
		 */
		case F_WRLCK:
		default:
			return EINVAL;
	}
}

/*
 * NAME:	cdr_lookup(dvp, vpp, pname, flag, vattrp, crp)
 *
 * FUNCTION:	This function looks up a name in a directory.
 *
 * PARAMETERS:	dvp 	- vnode that represents the directory where
 *				the name is to be found
 *		vpp	- address to return vnode if name is found
 *		pname	- name to look for in directory
 *		flag	- flag
 *		vattrp  - attributes to be returned
 *		crp	- credential
 *
 * RETURN :	0	- success
 *		errors from subroutines
 */
static
int
cdr_lookup (
	struct vnode *		dvp,	/* directory to search for name	*/
	struct vnode **		vpp,	/* returned vnode for name	*/
	char *			pname,	/* name to look for in dir	*/
	int			flag,	/* flag				*/
	struct vattr *		vattrp, /* attributes to be returned	*/
	struct ucred *		crp)	/* cred pointer			*/
{
	int			rc;	/* return code			*/
	struct cdrfsmount	*cdrfsp; /* mounted file system data */
	struct cdrnode *	dcdrp;	/* cdrnode for directory	*/
	struct cdrnode *	cdrp;	/* cdrnode for entry in dir	*/
	daddr_t			dirent;	/* dir entry number for name	*/
	daddr_t			pdirent; /* parent's dir entry number	*/
	dname_t			nmp;	/* name to look for in dir	*/
	int			waslocked; /* caller has the lock held     */


	if (!(waslocked = lock_mine(&cdrfs_lock)))
		CDRFS_LOCK();

	/* get cdrnode for directory */
	dcdrp = VTOCDRP(dvp);

	/* Check the access rights to the directory.
	 * The cdrnode need not be locked in order to use cdraccess().
	 */
	if (rc = cdraccess(dcdrp, IEXEC, crp))
	{
		if (!waslocked)
			CDRFS_UNLOCK();
		return rc;
	}

	/* create a name structure for the directory */
	nmp.nmlen = strlen(pname);
	nmp.nm = pname;

	/* Look for directory entry of file in directory cache. */
	dirent = dc_lookup(dcdrp->cn_dev, dcdrp->cn_dirent, &nmp);

	/* If the directory entry of the file was not in the directory cache,
	 * search the directory file.
	 */
	if (dirent == 0)
	{
		BUGLPR(buglevel, 9, ("did not find dirent in directory cache:  name = %s\n", pname));
		cdrfsp = dcdrp->cn_cdrfs;
		/* search the directory for the name */
		if (cdrfsp->fs_format == CDR_ROCKRIDGE)
			rc = cdrlookup_rrg(dcdrp, &nmp, &cdrp);
		else
			rc = cdrlookup_iso(dcdrp, &nmp, &cdrp);
		if (rc) {
			if (!waslocked)
				CDRFS_UNLOCK();
			RETURNX(rc, cdr_elist);
		}

		/* since we gave up the CDRFS_LOCK while reading the 
		 * parent directory, we must search the name cache again
		 * to make sure it wasn't added during the time the lock
	  	 * was released. If it wasn't added then call dc_enter
		 * else simply continue.
		 */
		dirent = dc_lookup(dcdrp->cn_dev, dcdrp->cn_dirent, &nmp);
		if (dirent == 0)
		{
			/* Add an entry for the name and parent directory 
			 * to the directory cache.  We set the low order bit
			 * of the dirent for directories so that can identify
			 * them as directories later.
			 */
			if ((cdrp->cn_mode & IFMT) == IFDIR)
				dc_enter(dcdrp->cn_dev,
					 dcdrp->cn_dirent,
					 &nmp,
					 cdrp->cn_dirent | 1);
			else
				dc_enter(dcdrp->cn_dev,
					 dcdrp->cn_dirent,
					 &nmp,
					 cdrp->cn_dirent);
		}
	}
	/* If we are looking for the "." entry, don't do cdrget().
	 * If the dirents match (ignoring the low order directory bit),
	 * then we were looking for the "." entry and the cdrnode we want
	 * is the directory cdrnode, which is currently locked by us.
	 * cdrget() would sleep waiting for it, causing a deadlock.
	 * Just set cdrp to the directory cdrnode and we'll hold it below.
	 */
	else if ((dirent & ~1) == dcdrp->cn_dirent)
		cdrp = dcdrp;		/* "." entry */
	else
	{
		BUGLPR(buglevel, 9, ("found dirent in directory cache:  dirent = 0x%x\n", dirent));
		/* If the dirent is for a directory (low order bit is set
		 * in dirent from directory cache), call cdrget() with
		 * a zero dirent for the parent.  This lets cdrget() know
		 * that the cdrnode to get is for a directory.  This is
		 * why we need to set the low order bit of the dirent for
		 * directories in the directory cache.
		 */
		if (dirent & 1)
		{
			rc = cdrget(dcdrp->cn_dev, dirent & ~1, 0, &cdrp);
			if (!rc && strcmp(pname, "..") &&
					cdrp->cn_pdirent != dcdrp->cn_dirent)
				rc = EFORMAT;

			/* non-RRG zero timestamp fixup 
			 * for cdrnode in name cache but not in cdrnode cache
			 * (i.e., read '.' entry of the directory from disk):
			 * inherit timestamp from the directory entry 
			 * in the parent directory by forcing parent directory
			 * lookup.
			 */
			if (!rc && cdrp->cn_ctime == 0) {
				if (cdrp->cn_cdrfs->fs_format != CDR_ROCKRIDGE) {
					rc = cdrlookup_iso(dcdrp, &nmp, &cdrp);
					if (!rc)
						cdrput(cdrp);
				}
			}
		}
		else
			rc = cdrget(dcdrp->cn_dev,
				    dirent,
				    dcdrp->cn_dirent,
				    &cdrp);
		if (rc)
		{
			if (!waslocked)
				CDRFS_UNLOCK();
			return rc;
		}
	}

	/* If the new cdrnode is the same as the directory cdrnode, we need
	 * to increment its hold count, as the caller releases both the
	 * parent and child vnodes.
	 */
	if (cdrp == dcdrp)
	{
		VNOP_HOLD(dvp);
		*vpp = dvp;
	}
	else
		/* Get a vnode in the same vfs as the parent directory, to
		 * return from lookup.  cdrptovp() will adjust the counts
		 * on the vnode and gnode appropriately.
		 */
		rc = cdrptovp(dvp->v_vfsp, cdrp, vpp);
	
	if (vattrp != NULL && rc == 0)
		cdr_getattr(*vpp, vattrp, crp);


	BUGLPR(buglevel, 9, ("lookup returned vnode 0x%x (cdrnode 0x%x), count = %d\n", *vpp, VTOCDRP(*vpp), (*vpp)->v_count));
	if (!waslocked)
		CDRFS_UNLOCK();
	return rc;
}

/*
 * NAME:	cdr_map(vp, addr, length, offset, request, crp)
 *
 * FUNCTION:	This function determines the validity of a map request and
 *		increments the gnode map counts appropriately.  Only
 *		SHM_RDONLY map requests are allowed.
 *
 * PARAMETERS:	vp	- vnode of object to map
 *		addr	- address to map to
 *		length	- length of object to map
 *		offset	- offset within object to map
 *		request	- type of mapping desired
 *		crp	- credential
 *
 * RETURNS:	0	- success
 *		EINVAL	- bad request
 *
 * SERIALIZATION: Takes the cdrfs lock to serial the count of mappers.
 *
 */
static
int
cdr_map (
	struct vnode *		vp,	/* vnode of object to map	*/
	caddr_t			addr,	/* address to map to		*/
	off_t			length,	/* length of object to map	*/
	off_t			offset,	/* offset within object to map	*/
	unsigned int		request, /* type of mapping desired	*/
	struct ucred *		crp)	/* cred pointer			*/
{
	struct cdrnode *	cdrp;	/* cdrnode for file		*/

	/* we only allow read mapping */
	if (request & SHM_RDONLY) {

		CDRFS_LOCK();
		/* check the file format */
		cdrp = VTOCDRP(vp);
		if (cdrp->cn_format & (CD_ROMXA_M2F2 | CD_NTRLVD | CD_DA)){
			CDRFS_UNLOCK();
			return EFORMAT;
		}
		/*
		 * place a hold count on the vnode so that closing
		 * of the file does not interfere with mappers.
		 */
		vp->v_count++;

		++CDRTOGP(VTOCDRP(vp))->gn_mrdcnt;
		CDRFS_UNLOCK();
		return 0;
	}
	return EINVAL;
}

/*
 * NAME:	cdr_open(vp, flag, ext, vinfop, crp)
 *
 * FUNCTION:	This function verifies access to a file and creates
 *		a segment for the file.
 *
 * PARAMETERS:	vp	- vnode of file to open
 *		flag	- file open flags from the file pointer
 *		ext	- external data used by the device driver
 *		vinfop	- pointer to the vinfo field for the open file
 * 		crp	- pointer to the cred struct
 *
 * RETURNS:	0	- success
 *		EINVAL	- invalid file open flags
 *		errors from subroutines
 *
 * SERIALIZATION: Take the cdrfs lock to serialize lookups through
 *		  the CDROM file system.
 *
 */
static
int
cdr_open (
	struct vnode *		vp,	/* vnode of file to open	*/
	int			flag,	/* file open flags		*/
	int			ext,	/* ext data used by dev driver	*/
	caddr_t *		vinfop,	/* vinfo field for open file	*/
	struct ucred *		crp)	/* cred pointer			*/
{
	struct cdrnode *	cdrp;	/* cdrnode for file		*/
	int			type;	/* file type flags		*/
	int			rc;	/* return code			*/

	CDRFS_LOCK();
	/* get cdrnode and check the file type */
	cdrp = VTOCDRP(vp);
	type = cdrp->cn_mode & IFMT;
	if (!(type == IFREG || type == IFDIR)) {
		CDRFS_UNLOCK();
		return EINVAL;
	}

	/* check the open flags */
	if (flag & (FTRUNC | FWRITE)) {
		CDRFS_UNLOCK();
		return EINVAL;
	}

	/* check access to cdrnode */
	if (rc = cdrp_access(cdrp, flag, crp)) {
		CDRFS_UNLOCK();
		return rc;
	}

	/* get a segment for the cdrnode */
	if (!cdrp->cn_seg)
		rc = cdrbindseg(cdrp);

	CDRFS_UNLOCK();
	return rc;
}

/*
 * NAME:	cdr_rdwr(vp, rwmode, flags, uiop, ext, vinfop, vattrp, crp)
 *
 * FUNCTION:	This function handles all read and write calls for the
 *		CD-ROM filesystem.   Only read calls are allowed.
 *
 * PARAMETERS:	vp 	- vnode that represents the object we want
 *				to read from
 *		rwmode	- read or write mode (UIO_READ or UIO_WRITE)
 *		flags	- file open flags
 *		uiop	- amount to read or write and where it goes
 *		ext	- external data used by the device driver
 *		vinfop	- pointer to the vinfo field for the open file
 *		vattrp	- attributes to be returned
 *		crp	- credential
 *
 * RETURN :	0	- success
 *		EINVAL	- attempt to write
 *		errors from subroutines
 *
 * SERIALIZATION: Take the cdrfs lock to serialize reads through 
 *		  the CDROM file system. But in the call to readcdr()
 *		  we release the lock around the actual vm_move operation.
 *
 */
static
int
cdr_rdwr (
	struct vnode *		vp,	/* vnode of file to read from	*/
	enum uio_rw		rw,	/* read or write mode		*/
	int			flags,	/* file open flags		*/
	struct uio *		uiop,	/* uio struct describing read	*/
	int			ext,	/* ext data for device driver	*/
	caddr_t			vinfop,	/* ptr to vinfo field of file	*/
	struct vattr *		vattrp,/* attributes to be returned	*/
	struct ucred *		crp)	/* cred pointer			*/
{
	int			type;	/* file type flags		*/
	struct cdrnode *	cdrp;	/* cdrnode for file		*/
	int			rc;	/* return code			*/

	/* reject writes */
	if (rw == UIO_WRITE)
		return EINVAL;

	CDRFS_LOCK();
	/* get cdrnode and type */
	cdrp = VTOCDRP(vp);
	type = cdrp->cn_mode & IFMT;
	ASSERT(type == IFREG || type == IFDIR);

	/* check the file format */
	if (cdrp->cn_format & (CD_ROMXA_M2F2 | CD_NTRLVD | CD_DA)) {
		CDRFS_UNLOCK();
		return EFORMAT;
	}

	/* trace hooks */
	TRCHKL2T(HKWD_KERN_PFS|hkwd_PFS_RDWR, vp, cdrp);

	rc = readcdr(cdrp, flags, ext, uiop);

	if (vattrp != NULL && rc == 0)
		cdr_getattr(vp, vattrp, crp);

	CDRFS_UNLOCK();
	return rc;
}

/*
 * NAME:	cdr_readdir(vp, uiop, crp)
 *
 * FUNCTION:	This function reads the data in a directory,
 *		translates it to a filesystem independent format,
 *		and copies the results to user space.
 *
 * PARAMETERS:	vp 	- vnode that represents the directory
 *				that we want to read
 *		uiop	- amount to read and where it goes
 *		crp	- credential
 *
 * RETURN :	0	- success
 *		EINVAL	- vp not a directory or more than one
 *				I/O vector in uio
 *		errors from subroutines
 *
 * SERIALIZATION: Take the lock before we call to the specific disk format
 *		  routines(ISO 9660 or Rock Ridge)
 *
 */
static
int
cdr_readdir (
	struct vnode *		vp,	/* vnode for directory to read	*/
	struct uio *		uiop,	/* uio struct describing output	*/
	struct ucred *		crp)	/* cred pointer 		*/
{
	int			rc = 0;	/* return code			*/
	struct cdrfsmount	*cdrfsp; /* mounted file system data */
	struct cdrnode	*cdrp;

	/* We only allow one I/O vector (we use uiomove).  Verify that
	 * the vnode represents a directory.
	 */
	if (uiop->uio_iovcnt != 1 || vp->v_vntype != VDIR)
		return EINVAL;


	CDRFS_LOCK();
	/* read from directory */
	cdrp = VTOCDRP(vp);
	cdrfsp = cdrp->cn_cdrfs;
	if (cdrfsp->fs_format == CDR_ROCKRIDGE)
		rc = cdrreaddir_rrg(cdrp, uiop);
	else
		rc = cdrreaddir_iso(cdrp, uiop);

	if (rc) {
		CDRFS_UNLOCK();
		RETURNX(rc, cdr_elist);
	}
	CDRFS_UNLOCK();
	return 0;
}

/*
 * NAME:	cdr_readlink (vp, uiop, crp)
 *
 * FUNCTION:	Read a symbolic link
 *
 * PARAMETERS:	vp 	- pointer to the vnode that represents the 
 *			  symlink we want to read
 *		uiop	- How much to read and where it goes
 *		crp	- credential
 *
 * RETURN :	EINVAL	- if not a symbolic link
 *		errors from subroutines
 *
 * SERIALIZATION: Not needed since we aren't changing fields in the cdrnode.
 *
 *			
 */

cdr_readlink(vp, uiop, crp)
struct vnode	*vp;			/* VLNK vnode			*/
struct uio	*uiop;			/* Uio iformation		*/
struct ucred 	*crp;			/* cred pointer 		*/
{
	int	rc;			/* Return code			*/
	struct cdrnode *cdrp;		/* cdrnode for vp		*/

	if (vp->v_vntype != VLNK)
		return EINVAL;

	cdrp = VTOCDRP(vp);

	/* AES requires ERANGE if the link name won't fit */
	if (cdrp->cn_size > uiop->uio_resid)
		return ERANGE;

	/* copy the symlink from cached area
	 */
	if (cdrp->cn_size <= CN_PRIVATE)
		rc = uiomove(cdrp->cn_symlink, cdrp->cn_size, UIO_READ, uiop);
	else
		rc = uiomove(cdrp->cn_symfile, cdrp->cn_size, UIO_READ, uiop);

	return rc;
}

/* 
 * NAME:	cdr_rele(vp)
 *
 * FUNCTION:	This function decrements the count on a vnode.  If it was
 *		the last reference to the vnode, release the memory
 *		associated with the vnode and cdrput the associated cdrnode.
 *
 * PARAMETERS:	vp	- vnode to release
 *
 * RETURNS:	0	- success
 *
 * SERIALIZATION: Take the cdrfs lock to serialize releasing cdrnode 
 *		  with mounts, unmounts, and lookups.
 *
 */
static
int
cdr_rele (
	struct vnode *		vp)	/* vnode to release		*/
{
	struct cdrnode *	cdrp;	/* cdrnode for vnode		*/
	struct vfs *		vfsp;	/* vfs containing vnode		*/
	struct vnode *		tvp;	/* temp vnode pointer		*/
	int			waslocked; /* caller holds the cdrfs_lock  */

	/* get vfs for vnode */
	vfsp = vp->v_vfsp;
	BUGLPR(buglevel, 9, ("releasing vnode 0x%x (cdrnode 0x%x), count = %d\n", vp, VTOCDRP(vp), vp->v_count));

	/* check and decrement count */
	assert(vp->v_count > 0);
	if (!(waslocked = lock_mine(&cdrfs_lock)))
		CDRFS_LOCK();
	vp->v_count--;

	/* If this is the last access to a file over file mount, then go
	 * ahead and release it.
	 */
	if (		vp->v_count == 1 &&
			!(vfsp->vfs_flag & VFS_UNMOUNTING) &&
			(vp->v_flag & V_ROOT) &&
			vp->v_type == VREG)
		vp->v_count--;

	/* If the count is greater than zero, there is no work to do. */
	if (vp->v_count > 0)
	{
		if (!waslocked)
			CDRFS_UNLOCK();
		return 0;
	}

	BUGLPR(buglevel, 9, ("vnode count is zero, freeing vnode\n"));
	/* If this is a file over file mount point, set the
	 * disconnected flag in the vfs.  This will let us know
	 * to get a new vnode in vfs_root() vfs operation.
	 */
	if ((vp->v_flag & V_ROOT) && vp->v_type == VREG)
	{
		vfsp->vfs_mntd = NULL;
		vfsp->vfs_flag |= VFS_DISCONNECTED;
	}

	/* get cdrnode for the vnode */
	cdrp = VTOCDRP(vp);

	if (!stale_vnode(vp))
	{
		BUGLPR(buglevel, 9, ("vnode not stale\n"));
		/* If this vnode is not from a previously force
		 * unmounted filesystem, just free the vnode and
		 * cdrput the cdrnode.
		 */
		vn_free(vp);
		cdrput(cdrp);
	}
	else
	{
		BUGLPR(buglevel, 9, ("vnode stale\n"));
		/* The vnode is left over from a forced unmount.
		 * Free the vnode.  If the cdrnode already has a
		 * count of zero, the cdrnode is the special
		 * stale cdrnode.
		 */
		vn_free(vp);
		if (cdrp->cn_count > 0)
		{
			/* This is not the special stale cdrnode, so
			 * keep track of its count.
			 */
			if (--cdrp->cn_count > 0)
			{
				/* must be another stale vnode */
				ASSERT(vfsp->vfs_vnodes != NULL);
				ASSERT(cdrp->cn_gnode.gn_vnode != NULL);
				ASSERT(cdrp->cn_count >= cdrp->cn_mapcnt);
				if (!waslocked)
					CDRFS_UNLOCK();
				return 0;
			}

			/* This is the last use of a regular stale
			 * cdrnode.  Free the cdrnode's segments and
			 * clear the gnode's vnode list.
			 */
			ASSERT(cdrp->cn_mapcnt == 0);
			cdrclear(cdrp);
			cdrp->cn_gnode.gn_vnode = NULL;
		}
	}

	tvp = vfsp->vfs_vnodes;		/* Delink vp vfs list */
	if (tvp == vp)
		vfsp->vfs_vnodes = vp->v_vfsnext;
	if (vp->v_vfsnext != NULL)
		vp->v_vfsnext->v_vfsprev = vp->v_vfsprev;
	if (vp->v_vfsprev != NULL)
		vp->v_vfsprev->v_vfsnext = vp->v_vfsnext;

	BUGLPR(buglevel, 9, ("vnode list = 0x%x\n", vfsp->vfs_vnodes));
	/* If this filesystem has unmounting in progress, 
	 * we are just waiting to free the last active vnode,
	 * release the vfs here.
	 */
	if ((vfsp->vfs_vnodes == NULL) &&
	    (vfsp->vfs_flag & VFS_UNMOUNTED))
	{
		BUGLPR(buglevel, 7, ("released last vnode, releasing vfs\n"));
		vfsrele(vfsp);
	}

	if (!waslocked)
		CDRFS_UNLOCK();

	return 0;
}

/* 
 * NAME:	cdr_unmap(vp, flag, crp)
 *
 * FUNCTION:	This function decrements the map count for a mapped file
 *
 * PARAMETERS:	vp	- vnode of file to unmap
 *		flag	- type of mapping to undo
 *		crp	- credential
 *
 * RETURNS:	0	- success
 *		EINVAL	- unmap of a write mapping
 *
 * SERIALIZATION: Takes the cdrfs lock to serial the count of mappers.
 *
 *
 */
static
int
cdr_unmap (
	struct vnode *		vp,	/* vnode of file to unmap	*/
	int			flag,	/* type of mapping to undo	*/
	struct ucred *		crp)	/* cred pointer 		*/
{
	if (flag & SHM_RDONLY)
	{
		CDRFS_LOCK();
		/* Decrement the read map count, assuring that it
		 * is positive.
		 */
		ASSERT(CDRTOGP(VTOCDRP(vp))->gn_mrdcnt > 0);
		--CDRTOGP(VTOCDRP(vp))->gn_mrdcnt;

		/*
		 * decrement the hold count on the vnode
		 * since we incremented it on call to mmap().
		 */
		cdr_rele(vp);

		CDRFS_UNLOCK();
		return 0;
	}

	/* only read maps allowed */
	return EINVAL;
}

/*
 * NAME:	uiosiz(uiop)
 *
 * FUNCTION:	This function returns the total length of the buffers
 *		in a uio structure.
 *
 * PARAMETERS:	uiop	- uio structure to get the buffer length for
 *
 * RETURN :	length of the uio structure
 */
static
int
uiosiz (struct uio *	uiop)	/* uio structure to get buf length for	*/
{
	struct iovec *	iov;	/* pointer to iovec array		*/
	int		i;	/* index into iovec array		*/
	int		length;	/* cumulative length of iovec buffers	*/

	/* Traverse the I/O vectors accumulating the length. */
	length = 0;
	iov = uiop->uio_iov;
	for (i = 0; i < uiop->uio_iovcnt; ++i)
		length += iov[i].iov_len;
	return length;
}
