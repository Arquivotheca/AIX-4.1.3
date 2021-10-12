static char sccsid[] = "@(#)07	1.14.1.18  src/bos/kernext/cfs/cdr_vfsops.c, sysxcfs, bos411, 9439C411d 9/30/94 17:08:21";
/*
 * COMPONENT_NAME: (SYSXCFS) CDROM File System
 *
 * FUNCTIONS: CDRFS vfs operations. 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* standard includes */
#include <sys/types.h>
#include <unistd.h>
#include <values.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/priv.h>
#include <sys/acl.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/vfs.h>
#include <sys/statfs.h>
#include <sys/vmuser.h>
#include <sys/vmount.h>
#include <sys/filsys.h>
#include <sys/dir.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/syspest.h>
#include <sys/lock_def.h>
#include <sys/gfs.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

/* CD-ROM file system includes */
#include "cdr_xcdr.h"
#include "cdr_rrg.h"
#include "cdr_xa.h"
#include "cdr_cdrfs.h"
#include "cdr_cdrnode.h"

/* function definitions for vfs operations */
int
cdr_init (	struct gfs *		gfsp);

int
cdr_rootinit (	void);

static
int
cdr_mount (	struct vfs *		vfsp,
		struct ucred *		crp);

static
int
cdr_umount (	struct vfs *		vfsp,
		int			flags,
		struct ucred *		crp);
static
int
cdr_root (	struct vfs *		vfsp,
		struct vnode **		vpp,
		struct ucred *		crp);
static
int
cdr_statfs (	struct vfs *		vfsp,
		struct statfs *		statfsp,
		struct ucred *		crp);
static
int
cdr_sync (	void);

static
int
cdr_vget (	struct vfs *		vfsp,
		struct vnode **		vpp,
		struct cdrfileid *	fidp,
		struct ucred *		crp);
static
int
cdr_cntl (	struct vfs *		vfsp,
		int			cmd,
		caddr_t			arg,
		unsigned long		argsize,
		struct ucred *		crp);

static
int
cdr_quotactl (	struct vfs *		vfsp,
		int			cmd,
		uid_t			id,
		caddr_t 		arg,
		struct ucred *		crp);

/* This is the vfs operations structure.  It is attached to the vfs,
 * and used by the LFS to invoke file system specific operations in
 * a file system independent manner.
 */
struct vfsops cdr_vfsops = {
	cdr_mount,
	cdr_umount,
	cdr_root,
	cdr_statfs,
	cdr_sync,
	cdr_vget,
	cdr_cntl,
	cdr_quotactl
};

/* cdr_strategy() is the strategy routine for handling CD-ROM
 * block requests.  cdr_strategy() is not allowed to page fault, as
 * it is called by the page fault handler.  Since cdr_strategy()
 * must touch unpinned cdrnode data to figure out how to map a file
 * block request to a disk block request, cdr_strategy() merely
 * queues request buffers for cdr_pager() to handle as a kernel
 * process (which can page fault) later.
 * cdrfs_iodone() intercepts and filters exceptions from device driver.
 */
extern int		cdr_strategy(struct buf *);
extern void		cdr_pager(void);
extern void		cdrfs_iodone(struct buf *);

/* local function declarations */
static
int
cdrpmount (
	struct vfs		*vfsp,
	struct file *		fp,
	dev_t			dev);

static
int
cdrpumount (
	dev_t		dev);

/* debugger level variable declaration to control BUGLPR() macro */
BUGVDEF(buglevel, 7)

/* vnode operations to attach to gfs structure */
extern struct vnodeops cdr_vops;

/* gfs structure describing the CD-ROM filesystem type */
struct gfs	cdrgfs =	{ &cdr_vfsops,
				&cdr_vops,
				MNT_CDROM,
				"CDROM",
				cdr_init,
				GFS_FUMNT | GFS_VERSION4,
				NULL,
				cdr_rootinit };

pid_t	pager_pid;	/* process id of cdr pager kproc */


Simple_lock	cdrfs_lock;

int		cdr_state = CDRFS_UNCONFIGURED;

/* function definitions */

/*
 * NAME:	cdr_config(cmd, uiop)
 *
 * FUNCTION:	This function is called to initialize the gfs
 *		at the kernel extension configuration time.
 *
 * PARAMETERS:	cmd	- configuration command (CFG_INIT or CFG_TERM)
 *		uiop	- ignored
 *
 * RETURN :	0	- success
 *		EINVAL	- invalid configuration command
 *		errors from subroutines
 * 
 * SERIALIZATION: At the call to gfsadd(), we are serialized with the
 *		  the GFS_LOCK. When the cdr_init() function is called
 *		  within gfsadd(), the cdrfs lock will be taken and held
 *		  throughout the duration of cdr_config. At termination,
 *		  a state flag will be set to indicate that we are 
 *		  terminating and thus serialize gfsdel() with the mounting
 *		  of another GFS which calls cdr_init again.
 */
int
cdr_config(
	int		cmd,		/* command to config routine	*/
	struct uio *	uiop)		/* ignored			*/
{
	int		rc;		/* return code			*/
	int		old_state;      /* cdr_state before swap        */
	extern struct proc *	sleep_list;	/* proc list for sleep	*/
	extern struct buf *	buf_list;	/* list of queued bufs	*/

	if (cmd == CFG_INIT)
	{
		BUGLPR(buglevel, 9, ("initializing CD-ROM kernel extension\n"));
		BUGLPR(buglevel, 1, ("&buglevel = 0x%x\n", &buglevel));
		BUGLPR(buglevel, 1, ("&cdr_config = 0x%x\n", *(int *)(void *)cdr_config));

		old_state = CDRFS_UNCONFIGURED;
		if (!compare_and_swap(&cdr_state,
				&old_state, CDRFS_INITIALIZING))
		{
			/* the old cdr_state indicates that the cdrfs  *
			 * is currently in use or configuring.         */
			return EBUSY;
		}

		/* Initialize the cdrfs lock and hold it */
		lock_alloc(&cdrfs_lock,LOCK_ALLOC_PAGED,CDRFS_LOCK_CLASS,-1);
		simple_lock_init(&cdrfs_lock);

		/* Add the CD-ROM filesystem gfs structure to the gfs list.
		 * This will result in cdr_init() being called.  If this
		 * is successful, pin the functions and data structures which
		 * are run or accessed with interrupts disabled. Also, define
		 * CD_ROM filesystem to the VMM via vm_mount().
		 */
		if ((rc = gfsadd(MNT_CDROM, &cdrgfs)) ||
		    (rc = pincode(cdr_strategy))      ||
		    (rc = vm_mount(D_REMOTE, cdr_strategy, 64)))
		{
			/* Back out what might have been done.  This routines
			 * can be called even though some or all of the above
			 * routines may not have been called.  Ignore the error
			 * returns so we can return the error found above.
			 */
			cdr_term();
			(void)gfsdel(MNT_CDROM);
			(void)unpincode(cdr_strategy);
			(void)vm_umount(D_REMOTE, cdr_strategy);
			old_state = CDRFS_INITIALIZING;
			while (!compare_and_swap(&cdr_state,
					&old_state, CDRFS_UNCONFIGURED));
			return rc;
		}
		old_state = CDRFS_INITIALIZING;
		while (!compare_and_swap(&cdr_state,
				&old_state, CDRFS_CONFIGURED));
		return 0;
	}
	else if (cmd == CFG_TERM)
	{
		/* Delete the CD-ROM filesystem gfs structure from the gfs
		 * list.  Return the error code from gfsdel if it fails.
		 * Otherwise unpin everything that is pinned by the init
		 * code above, ignoring error returns. Also, remove CD-ROM
		 * filesystem from VMM device table via vm_umount().
		 */
		old_state = CDRFS_CONFIGURED;
		if (!compare_and_swap(&cdr_state,
				&old_state, CDRFS_TERMINATING))
		{
			/* The old cdr_state indicates that the cdrfs  *
			 * is not currently in an operational state.   */
			return EBUSY;
		}

		BUGLPR(buglevel, 9, ("terminating CD-ROM kernel extension\n"));
		if (rc = gfsdel(MNT_CDROM))
		{
			/* put back the old state */
			old_state = CDRFS_TERMINATING;
			while(!compare_and_swap(&cdr_state,
					&old_state, CDRFS_CONFIGURED));
			return rc;
		}
		cdr_term();
		cdrnoterm();
		(void)unpincode(cdr_strategy);
		(void)vm_umount(D_REMOTE, cdr_strategy);
		old_state = CDRFS_TERMINATING;
		while(!compare_and_swap(&cdr_state,
				&old_state, CDRFS_UNCONFIGURED));
		return 0;
	}

	/* invalid command */
	return EINVAL;
}

/*
 * NAME:	cdr_init(gfsp)
 *
 * FUNCTION:	This function is called to initialize the CD-ROM file system.
 *		It pins functions and data structures which will be run or
 *		accessed with interrupts disabled.  It creates a kernel
 *		process to handle I/O request buffers from the strategy
 *		routine.  Finally, it initializes the cdrnode management
 *		system.
 *
 * PARAMETERS:	gfsp	- pointer to the CD-ROM filesystem gfs struct
 *
 * RETURN :	0	- success
 *		EAGAIN	- couldn't create kproc for paging
 *		errors from subroutines
 *
 * SERIALIZATION: The CDRFS_LOCK is held on entry to this routine.
 */
int
cdr_init (struct gfs *		gfsp)	/* gfs of CD-ROM filesystem	*/
{
	int			rc;	/* return code			*/

	BUGLPR(buglevel, 9, ("creating kernel paging process\n"));
	/* Create the kernel process running cdr_pager() to handle
	 * request buffers queued by cdr_strategy().
	 */
	if ((pager_pid = creatp()) == -1)
		return EAGAIN;	/* no more processes */

	if (rc = initp(pager_pid, cdr_pager, NULL, 0, "cdpg"))
		return rc;

	BUGLPR(buglevel, 9, ("initializing cdrnode management system\n"));
	/* initialize the cdrnode management system */
	return cdrnoinit();
}

/*
 * NAME:	cdr_rootinit(void)
 *
 * FUNCTION:	This function initializes a root for the root filesystem.
 *		Since, for the moment, a CD-ROM filesystem cannot be used
 *		as the root filesystem, cdr_rootinit() returns EINVAL.
 *
 * PARAMETERS:	none
 *
 * RETURN :	EINVAL
 */
int
cdr_rootinit(void)
{
	return EINVAL;
}

/*
 * NAME:	cdr_mount(vfsp, crp)
 *
 * FUNCTION:	CD-ROM file system VFS_MOUNT().
 *		This function mounts a CD-ROM device (containing a CD-ROM
 *		disk) or a directory or file from a CD-ROM filesystem
 *		over a directory (or file in the case of file mounts) in any
 *		file system.  A vfs which has been partially filled is taken
 *		as a parameter.  For "file over file" and "directory over
 *		directory" mounts, very little is done save attaching a vnode
 *		representing the mounted object to the vfs.  For "device over
 *		directory" mounts, the format of the CD-ROM in the drive is
 *		determined (ISO 9660:1988, High Sierra, Rock Ridge format),
 *		and the vnode for the root directory of the filesystem on the
 *		disk is attached to the vfs.  Only readonly mounts are allowed.
 *		The user must have FS_CONFIG privelege in order to enact
 *		"device over directory" mounts.
 *
 * PARAMETERS:	vfsp	- CD-ROM virtual file system to mount
 *
 * RETURN :	0	- success
 *		errors from subroutines
 *
 * SERIALIZATION: Since mount can be called either with the CDRFS LOCK
 *		  held from cdr_root or not held via the VFS switch,
 *		  we need to check if we have it or not.
 */
static
int
cdr_mount(
	struct vfs *		vfsp,	/* vfs to be mounted		*/
	struct ucred *		crp)	/* caller's credentials		*/
{
	struct vnode *		stubvp;	/* vnode of stub mounted over	*/
	struct vnode *		objectvp; /* vnode of object to mount 	*/
	struct gnode *		gp;	/* gnode for object to mount	*/
	struct file *		fp;	/* file ptr for object to mount	*/
	fsid_t			fsid;	/* file system id		*/
	int			rc;	/* return code			*/
	int			flag;	/* device open flags		*/
	dev_t			dev;	/* dev num of object to mount	*/
	struct cdrnode *	rootcdrp; /* cdrnode of filesystem root	*/
	struct vnode *		rootvp;	/* vnode of root of file system	*/
	struct cdrfsmount	*cdrfsp; /* mounted file system data	*/
	int			waslocked;	/* was the cdrfs lock taken? */	

	BUGLPR(buglevel, 1, ("&buglevel = 0x%x\n", &buglevel));
	BUGLPR(buglevel, 1, ("&cdr_config = 0x%x\n", *(int *)(void *)cdr_config));
	BUGLPR(buglevel, 9, ("vfsp = 0x%x\n", vfsp));


	if (!(waslocked = lock_mine(&cdrfs_lock)))
		CDRFS_LOCK();

	/* 
	 * If we are not fully configured, we should not allow
	 * mounts to happen.  The state is set in cdr_config().
	 */
	if (cdr_state != CDRFS_CONFIGURED)
	{
		if (!waslocked)
			CDRFS_UNLOCK();
		return ENOSYS;
	}

	/* check that mount is read-only */
	if (!(vfsp->vfs_flag & VFS_READONLY)) {
		if (!waslocked)
			CDRFS_UNLOCK();
		return EROFS;
	}

	/* get stub vnode */
	stubvp = vfsp->vfs_mntdover;
	BUGLPR(buglevel, 9, ("stubvp = 0x%x\n", stubvp));

	/* Resolve the object path to a vnode.  Note that this vnode is not
	 * necessarily in the CD-ROM filesystem.  For device mounts it will
	 * be in the native (currently jfs) filesystem.  For file and
	 * directory mounts, it should be in the CD-ROM filesystem, but we are
	 * called based upon the vfs type specified by the user.  The LFS does
	 * not currently check to see that the object vfs type is correct.
	 * We will do so below.  Mounting of remote objects is not allowed
	 * here, so we inhibit the search from crossing to remote nodes.
	 */
	objectvp = NULL;
	if (rc = lookupvp(vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT),
			L_SEARCHLOC, &objectvp, crp)) {
		if (!waslocked)
			CDRFS_UNLOCK();
		return rc;
	}

	BUGLPR(buglevel, 9, ("objectvp = 0x%x\n", objectvp));

	/* Mark vfs as mounting.  We won't use it until mntd is filled in.
	 */
	vfsp->vfs_mntd = NULL;

	switch (objectvp->v_vntype)
	{
	case VBLK:		/* mount block device */

		/* Verify that the stub for the device mount is a directory
		 * and is on the local system.
		 */
		if (stubvp->v_vntype != VDIR)
		{
			rc = ENOTDIR;
			goto out;
		}

		/* Verify that user has filesystem config privilege (required
		 * to perform device mounts.
		 */
		if (!priv_req(FS_CONFIG))
		{
			rc = EPERM;
			goto out;
		}

		/* get information about block device to be mounted */
		gp = VTOGP(objectvp);
		dev = brdev(gp->gn_rdev);
		BUGLPR(buglevel, 9, ("device to be mounted:  0x%x\n", dev));
		flag = FMOUNT | FREAD;

		/* Create the private (mounted file system specific)
		 * data structure to be attached to the vfs.
		 * This structure contains data which is applicable for
		 * everything in the file system, is needed at various
		 * times, and would be expensive to recalculate each time
		 * it is needed.
		 */
		if ((cdrfsp = (struct cdrfsmount *)malloc(sizeof (struct cdrfsmount))) == NULL) {
			rc = ENOMEM;
			goto out;
		}
		bzero(cdrfsp, sizeof (struct cdrfsmount));
		vfsp->vfs_data = (caddr_t) cdrfsp;
		cdrfsp->fs_vfs = vfsp;

		/* The CD-ROM filesystem doesn't need to keep a mount structure,
		 * as the file pointer contains all the information needed
		 * to keep things straight.  We register the mounted device
		 * directly through the device interfaces in order to simply
		 * check for open files during unmount.  (see cdractivity())
		 */
		if (rc = fp_opendev(dev, flag, NULL, 0, &fp)) {
			free(CDRVFSDATA(vfsp));
			vfsp->vfs_data = NULL;
			goto out;
		}
		BUGLPR(buglevel, 9, ("device:  fp = 0x%x, gp = 0x%x\n", fp, fp->f_vnode));
		cdrfsp->fs_fp = fp;

		/* complete the mounting of the device */
		if (rc = cdrpmount(vfsp, fp, dev))
		{
			fp_close(fp);
			free(CDRVFSDATA(vfsp));
			vfsp->vfs_data = NULL;
			goto out;
		}

		/* set the device mount flag in the vfs */
		vfsp->vfs_flag |= VFS_DEVMOUNT;

		/* Get the root cdrnode so we can mark it specially later.
		 * Verify that the root directory has a ".." entry that
		 * refers to himself.  Note that cdrptovp() will cdrput()
		 * the rootcdrp if it detects an error.
		 */
		rc = cdrget(dev, cdrfsp->fs_rootdirent, 0, &rootcdrp);
		if (!rc)
			rc = cdrptovp(vfsp, rootcdrp, &rootvp);
		if (!rc && rootcdrp->cn_pdirent != cdrfsp->fs_rootdirent)
			/* "." and ".." entries of root directory do	*/
			/* not match					*/
		{
			BUGLPR(buglevel, 7, ("root dirent = %d, root pdirent = %d\n", cdrfsp->fs_rootdirent, rootcdrp->cn_pdirent));
			cdrput(rootcdrp);
			rc = EFORMAT;
		}
		if (rc)
		{
			cdrpumount(dev);
			fp_close(fp);
			free(CDRVFSDATA(vfsp)->fs_pvd);
			free(CDRVFSDATA(vfsp));
			vfsp->vfs_data = NULL;
			goto out;
		}
		BUGLPR(buglevel, 9, ("root:  vp = 0x%x, rootcdrp = 0x%x\n", rootvp, VTOCDRP(rootvp)));

		break;

	case VREG:
	case VDIR:		/* mount file or device */

		/* Verify that the vfs type of the object to be mounted is
		 * correct.  We are called based upon the user's request for
		 * the CD-ROM filesystem type, not because the LFS determined
		 * that the object was of this vfs type.  The LFS currently
		 * doesn't check, so we must.
		 */
		if (objectvp->v_vfsp->vfs_gfs->gfs_type != MNT_CDROM)
		{
			BUGLPR(buglevel, 9, ("gfs type object to mount is %d\n", objectvp->v_vfsp->vfs_gfs->gfs_type));
			rc = EINVAL;
			goto out;
		}

		/* If we're not disconnected, we must verify that the vfs has
		 * either the mountok flag or the suser flag set.
		 */
		if (!(vfsp->vfs_flag & VFS_DISCONNECTED) &&
				!(vfsp->vfs_flag & (VFS_VMOUNTOK|VFS_SUSER)))
		{
			rc = EPERM;
			goto out;
		}

		/* Verify that the stub and the object are either both
		 * directories or are both regular files.
		 */
		if (objectvp->v_vntype != vfsp->vfs_mntdover->v_vntype)
		{
			rc = ENOTDIR;
			goto out;
		}

		/* Copy the private data pointer from the vfs of the object.
		 * The private data is specific to the volume that the object
		 * is really on, so we propogate it from the device mount
		 * to the non-device mounts.  This private data is freed
		 * only when the device mount is unmounted.
		 */
		cdrfsp = CDRVFSDATA(objectvp->v_vfsp);
		vfsp->vfs_data = (caddr_t) cdrfsp;

		/* If this is a regular file and this is the first
		 * mount of the file, go ahead and disconnect it until
		 * lookuppn attempts to	access it again.  When lookuppn()
		 * tries to access the object, cdr_root() will recognize
		 * the mount as disconnected and we'll complete the mount.
		 */
		if (objectvp->v_vntype == VREG &&
				!(vfsp->vfs_flag & VFS_DISCONNECTED))
		{
			stubvp->v_mvfsp = vfsp;
			vfsp->vfs_flag |= VFS_DISCONNECTED;
			goto out;
		}

		/* clear the device mount flag in the vfs */
		vfsp->vfs_flag &= ~VFS_DEVMOUNT;

		/* Create the vnode for the root of the new filesystem from
		 * the vnode in the other filesystem.  We do this by getting
		 * the cdrnode that the vnode references and using cdrptovp()
		 * to create a new vnode in the new vfs for the cdrnode.
		 * Note that we've already verified that the object to be
		 * mounted is from the CD-ROM filesystem, so we can use
		 * VTOCDRP().
		 */
		rootcdrp = VTOCDRP(objectvp);
		/* increment cn_count */
		rc = cdrget(brdev(rootcdrp->cn_dev),
			    rootcdrp->cn_dirent,
			    rootcdrp->cn_pdirent,
			    &rootcdrp);
		if (rc || (rc = cdrptovp(vfsp, rootcdrp, &rootvp)))
			goto out;	/* cdrptovp() cdrput's on error	*/

		break;			/* from switch */

	default:
		rc = ENOTBLK;		/* generic "bad object" return */
		goto out;
	}

	/* finish off device specific mount info */
	fsid.fsid_dev = rootcdrp->cn_dev;
	fsid.fsid_type = MNT_CDROM;
	vfsp->vfs_fsid = fsid;
	vfsp->vfs_bsize = LBLKSIZE(cdrfsp);
	BUGLPR(buglevel, 9, ("file system block size = %d\n", vfsp->vfs_bsize));

	/* Mark the root vnode as root and as part of a mounted vfs */
	rootvp->v_flag |= V_ROOT;

	/* Save the root vnode in the vfs for cdr_root() to return.
	 * Also indicate that this is not a disconnected mount.
	 */
	vfsp->vfs_mntd = rootvp;
	vfsp->vfs_flag &= ~VFS_DISCONNECTED;

out:
	/* perform cleanup */
	if (objectvp)
		VNOP_RELE(objectvp);

	if (!waslocked)
		CDRFS_UNLOCK();
	BUGLPR(buglevel, 9, ("finished mounting cdrom\n"));
	return rc;
}

/*
 * NAME:	cdr_umount(vfsp, flags, crp)
 *
 * FUNCTION:	CD-ROM file system VFS_UNMOUNT().
 *		This function unmounts a CD-ROM device (containing a CD-ROM
 *		disk) or a directory or file from a CD-ROM filesystem.
 *		For "file over file" and "directory over directory" mounts,
 *		the only work is to release the vnode of the stub which the
 *		file or directory was mounted over.  The LFS handles the
 *		checking and marks the mount as "unmounting".  The vfs
 *		will hang around until the last vnode is released.
 *		For "device over directory" mounts, the directory cache
 *		is purged and activity (held cdrnodes) is checked.  If the
 *		VFS_SHUTDOWN flag of the vfs is set or if this is a forced
 *		unmount, activity is ignored.  Otherwise, if any held cdrnodes
 *		are found, EBUSY is returned.  Finally, the data for the root
 *		vnode and the mount cdrnode is released, the strategy routine
 *		is unregistered with the VMM and the device is closed.
 *
 * PARAMETERS:	vfsp	- CD-ROM virtual file system to unmount
 *		flags	- unmount flags (UVMNT_FORCE)
 *
 * RETURN :	0	- success
 *		EBUSY	- activity found on unforced mount
 *		errors from subroutines
 *
 * SERIALIZATION: We should not be holding the cdrfs lock upon entry
 *
 */
static
int
cdr_umount(
	struct vfs *		vfsp,	/* vfs to be unmounted		*/
	int			flags,	/* unmount flags		*/
	struct ucred *		crp)	/* caller's credentials		*/
{
        dev_t			dev;	/* device number of vfs		*/
        struct cdrnode *	root;	/* root cdrnode of file system	*/
        struct file *		fp;	/* device open file pointer	*/
	int			forced;	/* forced unmount flag		*/
	int			rc;    	/* return code			*/

	/* Take the cdrfs lock for the duration of unmount */
	CDRFS_LOCK();

	/* If this is the undoing of a directory or file mount,
	 * nearly all the work is file system independent and
	 * is done up in the vnode layer by kunmount().
	 */
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT)) {
		/* If this file system is not disconnected, release the root
		 * of the file system.  For "directory over directory" and
		 * "file over file" mounts, the LFS handles the rest of the
		 * details, so we're done.
		 */
		struct vnode *rootv;
                /*
                 * Before releasing root vnode,  make sure that
                 * there are no other vnodes in vfs.
                 */
                rootv = vfsp->vfs_mntd;
                if (rootv->v_count > 1)
                        rc = EBUSY;
		else
		{
			vfsp->vfs_data = NULL;
			if (!(vfsp->vfs_flag & VFS_DISCONNECTED))
			{
				VNOP_RELE(rootv);
				vfsp->vfs_mntd = NULL;
			}
			rc = 0;
		}
		CDRFS_UNLOCK();
		return rc;	
	}

	/* get the device file pointer and number */
	fp = CDRVFSDATA(vfsp)->fs_fp;
	BUGLPR(buglevel, 9, ("device:  fp = 0x%x, vp = 0x%x\n", fp, fp->f_vnode));
        dev = ((struct gnode *)fp->f_vnode)->gn_rdev;
	BUGLPR(buglevel, 9, ("dev = 0x%x\n", dev));

	/* (XXX)  Purge the directory cache.  There needs to be more thought
	 * given to quieting the file system before this point.  I
	 * doubt if the bases are covered.  Note that, even if the unmount
	 * fails, the directory cache has been purged.
	 */
	
	dc_purge(dev);

	/* No files are allowed to be active in a device being unmounted
	 * unless this is a forced unmount or a shutdown unmount.  Check the
	 * activity of the device by looking for held cdrnodes.  If
	 * cdractivity() succeeds, it will have released the root cdrnode
	 * for us.  The mount cdrnode will still be held by us.  It is freed
	 * in cdrpumount().
	 */
	root = VTOCDRP(vfsp->vfs_mntd);
	BUGLPR(buglevel, 9, ("root:  vp = 0x%x, cdrnode = 0x%x\n", vfsp->vfs_mntd, root));
	forced = (flags & UVMNT_FORCE) || (vfsp->vfs_flag & VFS_SHUTDOWN);
	BUGLPR(buglevel, 9, ("forced = %d\n", forced));
	rc = cdractivity(vfsp, dev, root, forced);

	/* This is where unmounts fail EBUSY. */
	if (!forced && rc) {
		CDRFS_UNLOCK();
		return rc;
	}

	BUGLPR(buglevel, 9, ("proceeding with unmount after checking activity\n"));

	/* Throw away the vfs private data.  Note that this is only done for
	 * device unmounts.  If this is not a forced unmount, we know that no
	 * other vfs is mounted from this device, because cdractivity() would
	 * fail.  If this is a forced unmount, cdractivity() has already
	 * unmounted all file systems mounted from this device.
	 */
	free(CDRVFSDATA(vfsp)->fs_pvd);
	free(CDRVFSDATA(vfsp));
	vfsp->vfs_data = NULL;

	/* if the root vnode is not held by anyone else, free it;
	 * otherwise, release it.
	 */
	if (vfsp->vfs_mntd->v_count == 1)
		vn_free(vfsp->vfs_mntd);
	else
		VNOP_RELE(vfsp->vfs_mntd);
	vfsp->vfs_mntd = NULL;

	/* finish the unmount */
	(void) cdrpumount(dev);

	/* finish unmount and close device */
	(void) fp_close(fp);
	BUGLPR(buglevel, 9, ("finished unmounting cdrom\n"));

	CDRFS_UNLOCK();

	/* CDRFS cleaned up: force cleanup LFS */
	return(0);
}

/*
 * NAME:	cdr_root(vfsp, vpp, crp)
 *
 * FUNCTION:	CD-ROM file system VFS_ROOT().
 *		This function returns the root of the specified file system.
 *		If the vfs is remaining after a forced unmount (waiting
 *		for its last vnode to be freed), ESTALE is returned.
 *		If the vfs has not completed its mount, the mount is
 *		completed.  The vnode for the root of the filesystem is held
 *		and returned.
 *
 * PARAMETERS:	vfsp	- CD-ROM virtual file system return root for
 *		vpp	- return address of root vnode
 *
 * RETURN :	0	- success
 *		ESTALE	- vfs has been forced unmounted
 *		errors from subroutines
 * 
 * SERIALIZATION: Since this function can be called with the cdrfs lock
 *		  held as in the case of VNOP_MOUNT, or not held via the
 *		  VFS switch, we must check if we have the lock or not.
 *			
 */
static
int
cdr_root(
	struct vfs *		vfsp,	/* vfs to get root of		*/
	struct vnode **		vpp,	/* return address of root vnode	*/
	struct ucred *		crp)	/* caller's credentials		*/
{
	struct vnode *		vp;	/* vnode of root		*/
	int			rc;	/* return code			*/
	int			waslocked;/* was the lock taken		*/


	if (!(waslocked = lock_mine(&cdrfs_lock)))
		CDRFS_LOCK();

	/* If the vfs is remaining after a forced unmount (waiting
	 * for its last vnode to be freed), ESTALE is returned.
	 */
	if (vfsp->vfs_flag & (VFS_SHUTDOWN | VFS_UNMOUNTING)){
		if (!waslocked)
			CDRFS_UNLOCK();
		return ESTALE;
	}

	/* check for mount in progress for non-file-on-file mount */
	if ((vfsp->vfs_mntd == NULL) &&
	    !(vfsp->vfs_flag & VFS_DISCONNECTED)){
		if (!waslocked)
			CDRFS_UNLOCK();
		return EAGAIN;
	}
	
	/* If the vfs mount is disconnected, call cdr_mount() to complete
	 * the mount.  This will be the case for "file over file" mounts
	 * when nobody is holding the mounted file's vnode.
	 */
	if (vfsp->vfs_flag & VFS_DISCONNECTED) {
		/* Try to complete the mount.  VFS_DISCONNECTED tells
		 * cdr_mount() to connect it this time.
		 */
		if (rc = VFS_MOUNT(vfsp, crp)) {
			if (!waslocked)
				CDRFS_UNLOCK();
			return rc;
		}
	}

	/* Get and hold the vnode of the root of the filesystem. */
	vp = vfsp->vfs_mntd;

	if (rc = VNOP_HOLD(vp))
		*vpp = (struct vnode *)NULL;
	else
		*vpp = vp;


	if (!waslocked)
		CDRFS_UNLOCK();
	BUGLPR(buglevel, 9, ("root vnode returned:  0x%x (cdrnode 0x%x), count = %d\n", *vpp, VTOCDRP(*vpp), (*vpp)->v_count));
	return rc;
}

/*
 * NAME:	cdr_statfs(vfsp, statfsp, crp)
 *
 * FUNCTION:	CD-ROM file system VFS_STATFS().
 *		This function fills in a structure with information
 *		about the specified virtual file system.
 *
 * PARAMETERS:	vfsp	- CD-ROM virtual file system return root for
 *		statfsp	- statfs structure to fill in
 *
 * RETURN :	0	- success
 *
 * SERIALIZATION: We should not be holding the cdrfs lock upon entry
 *
 */
static
int
cdr_statfs(
	struct vfs *		vfsp,		/* vfs to get stat on	*/
	struct statfs *		statfsp,	/* statfs to fill in	*/
	struct ucred *		crp)		/* caller's credentials	*/
{
	struct cdrpvd	*pvd;	/* primary volume data from vfs	*/
	struct vmount	*vmountp;

	/* Take the cdrfs lock */
	CDRFS_LOCK();

	/* If the vfs is remaining after a forced unmount (waiting
	 * for its last vnode to be freed), ESTALE is returned.
	 */
	if (vfsp->vfs_flag & (VFS_SHUTDOWN | VFS_UNMOUNTING)){
		CDRFS_UNLOCK();
		return ESTALE;
	}

	/* check for mount in progress for non-file-on-file mount */
	if ((vfsp->vfs_mntd == NULL) &&
	    !(vfsp->vfs_flag & VFS_DISCONNECTED)) {
		CDRFS_UNLOCK();
		return EAGAIN;
	}
	
	/* get primary volume data from vfs */
	pvd = CDRVFSDATA(vfsp)->fs_pvd;

	/* Fill in the statfs structure.  Initialize the structure by clearing
	 * the fields to zero.  Clear the name arrays so that names are
	 * automatically null terminated.
	 */
	bzero(statfsp, sizeof *statfsp);

	/* These fields are implicitly filled in by bzero().  We consider
	 * the volume to have no space free because the user cannot write
	 * any data to CD-ROM.
	 * Note:  this is a comment, not code.
	 
	statfsp->f_version	= 0;
	statfsp->f_type		= 0;
	statfsp->f_bfree	= 0;
	statfsp->f_bavail	= 0;
	statfsp->f_ffree	= 0;
	statfsp->f_vfsoff	= 0;
	statfsp->f_vfslen	= 0;
	statfsp->f_vfsvers	= 0;
	*/

	/* Explicitly fill in the non-zero fields in the statfs structure.
	 * Leave the file system name clear and fill in file system pack name.
	 */
	if (CDRVFSDATA(vfsp)->fs_format & CDR_ISO9660)
	{
		statfsp->f_bsize	= pvd->pvd_lblksize;
		statfsp->f_fsize	= pvd->pvd_lblksize;
		statfsp->f_blocks	= pvd->pvd_volspcsize;
		bcopy(pvd->pvd_vol_id, statfsp->f_fpack, MIN(32, CDR_VOLIDLEN));
	}
	else
	{
		ASSERT(CDRVFSDATA(vfsp)->fs_format == CDR_HIGHSIERRA);
		statfsp->f_bsize	= HSPVD(pvd)->pvd_lblksize;
		statfsp->f_fsize	= HSPVD(pvd)->pvd_lblksize;
		statfsp->f_blocks	= HSPVD(pvd)->pvd_volspcsize;
		bcopy(HSPVD(pvd)->pvd_vol_id, statfsp->f_fpack, MIN(32, CDR_VOLIDLEN));
	}
	BUGLPR(buglevel, 9, ("stat:  blocks = %d, lblk size = %d\n", statfsp->f_blocks, statfsp->f_bsize));

	/* copy mount point into fname */
	vmountp = vfsp->vfs_mdata;
	bcopy(vmt2dataptr(vmountp, VMT_STUB), statfsp->f_fname, MIN(32, vmt2datasize(vmountp, VMT_STUB)));

	/* Fill in the remaining fields of the statfs structure.  The
	 * "files" fields is a kludge because the CD-ROM does not keep
	 * such information.  The "files" value chosen is the largest
	 * possible number of files on the volume (minimum of 1 block
	 * per file).
	 */
	statfsp->f_files	= statfsp->f_blocks;	/* must be > 0 */
	statfsp->f_fsid		= vfsp->vfs_mdata->vmt_fsid;
	statfsp->f_vfstype	= MNT_CDROM;
	statfsp->f_vfsnumber	= vfsp->vfs_number;
	if (CDRVFSDATA(vfsp)->fs_format == CDR_ROCKRIDGE)
		statfsp->f_name_max	= CDR_RRG_NAME_MAX;
	else
		statfsp->f_name_max	= CDR_ISO_NAME_MAX;

	CDRFS_UNLOCK();

	return 0;
}

/*
 * NAME:	cdr_sync()
 *
 * FUNCTION:	CD-ROM file system VFS_SYNC().
 *		This function is called to make sure that all files in
 *		all CD-ROM file systems are consistent with their images
 *		on the physical media.  For CD-ROM, this is always the
 *		case, so this function does nothing and returns 0 for
 *		success.
 *
 * PARAMETERS:	none
 *
 * RETURN :	0	- success
 */
static
int
cdr_sync(void)
{
	return 0;
}

/*
 * NAME:	cdr_vget(vfsp, vpp, fidp, crp)
 *
 * FUNCTION:	CD-ROM file system VFS_VGET().
 *		This function returns a vnode from the specified file
 *		system which is associated with the file indicated by the
 *		specified fileid structure.
 *
 * PARAMETERS:	vfsp	- vfs to get vnode for file from
 *		vpp	- address for vnode return
 *		fidp	- fileid structure identifying file
 *
 * RETURN :	0	- success
 *		EINVAL	- invalid fileid structure
 *		ESTALE	- filed refers to different volume
 *		errors from subroutines
 *
 * SERIALIZATION: We should not be holding the cdrfs lock upon entry
 *
 */
static
int
cdr_vget(
	struct vfs *		vfsp,	/* vfs to get vnode from	*/
	struct vnode **		vpp,	/* address for vnode return	*/
	struct cdrfileid *	fidp,	/* file struct specifying file	*/
	struct ucred *		crp)	/* caller's credentials		*/
{
	struct cdrnode *	cdrp;	/* cdrnode of file		*/
	int			rc;	/* return code			*/
	ushort_t		hash;	/* volume hash value		*/
	char *			volname; /* volume name			*/
	register char *		s;	/* fast pointer into vol name	*/
	register int		i;	/* fast counter for loop	*/


	BUGLPR(buglevel, 9, ("getting vnode for nfs\n"));
	BUGLPR(buglevel, 9, ("fid length is %d\n", fidp->fid_len));
	BUGLPR(buglevel, 9, ("fid dirent is 0x%x\n", fidp->fid_dirent));
	BUGLPR(buglevel, 9, ("fid parent dirent is 0x%x\n", fidp->fid_pdirent));
	BUGLPR(buglevel, 9, ("fid hash number is 0x%x\n", fidp->fid_hash));

	/* Take the cdrfs lock */
	CDRFS_LOCK();

	/* If the vfs is remaining after a forced unmount (waiting
	 * for its last vnode to be freed), ESTALE is returned.
	 */
	if (vfsp->vfs_flag & (VFS_SHUTDOWN | VFS_UNMOUNTING))
	{
		BUGLPR(buglevel, 7, ("vfs stale for NFS vnode get\n"));
		rc = ESTALE;
		goto exit;
	}

	/* check for mount in progress for non-file-on-file mount */
	if ((vfsp->vfs_mntd == NULL) &&
	    !(vfsp->vfs_flag & VFS_DISCONNECTED))
	{
		rc = EAGAIN;
		goto exit;
	}

	cdrp = NULL;

	/* Check the length of the fileid.  fileid structures on the CD-ROM
	 * filesystem must be at least big enough to hold two dirents and
	 * the hash value.
	 */
	BUGLPR(buglevel, 9, ("minimum fid length is %d\n", 2 * sizeof(daddr_t) + sizeof(ushort_t)));
	if (fidp->fid_len < 2 * sizeof(daddr_t) + sizeof(ushort_t))
	{
		CDRFS_UNLOCK();
		BUGLPR(buglevel, 7, ("fid length wrong for NFS vnode get\n"));
		return EINVAL;
	}

	/* Generate the hash value for the volume.  This hash value is used
	 * to determine with high probability whether the volume we are on
	 * is the same as the volume the file id was created for.  We compare
	 * the hash value we generate with the hash value saved in the file id.
	 */
	if (CDRVFSDATA(vfsp)->fs_format & CDR_ISO9660)
	{
		BUGLPR(buglevel, 9, ("volume sequence number is %d\n", CDRVFSDATA(vfsp)->pvd->pvd_volseqno));
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

	/* Get the cdrnode for the file indicated by the fields in the fileid.
	 * Use the low order bit of the pdirent to determine whether we are
	 * getting the cdrnode for a directory.  We need to know this to pass
	 * the appropriate pdirent to cdrget().
	 */
	if (rc = cdrget(vfsp->vfs_fsid.fsid_dev,
			fidp->fid_dirent,
			(fidp->fid_pdirent & 1)? 0 : fidp->fid_pdirent,
			&cdrp))
	{
		BUGLPR(buglevel, 7, ("couldn't get cdrnode for NFS vnode get, rc = %d\n", rc));
		BUGLPR(buglevel, 7, ("dev = 0x%x, dirent = 0x%x, pdirent = 0x%x\n", vfsp->vfs_fsid.fsid_dev, fidp->fid_dirent, fidp->fid_pdirent));
		if (rc == EFORMAT)
		{
			rc = ESTALE; /* probably different disk */
			goto exit;
		}
		else
			goto exit;
	}
	
	/* Add the file creation time to the hash, one short at a time.
	 */
	BUGLPR(buglevel, 9, ("volume creation time is 0x%x\n", cdrp->cn_ctime));
	hash ^= *(ushort_t *)&cdrp->cn_ctime;
	hash ^= *(((ushort_t *)&cdrp->cn_ctime) + 1);

	/* verify the volume hash */
	if (hash != fidp->fid_hash)
	{
		BUGLPR(buglevel, 7, ("file id hash value wrong for NFS vnode get, %d should be %d\n", fidp->fid_hash, hash));
		cdrput(cdrp);
		rc = ESTALE;
		goto exit;
	}

	/* Check the parent's directory entry locations against each other.
	 * If the cdrnode already existed, cdrget() does not check that the
	 * parent's directory entry matches.  We clear the directory bit of
	 * the pdirent to get a match.
	 */
	BUGLPR(buglevel, 9, ("fid parent dirent is 0x%x\n", fidp->fid_pdirent));
	if (cdrp->cn_pdirent != (fidp->fid_pdirent & ~0x1))
	{
		BUGLPR(buglevel, 7, ("parent dirent's don't match for NFS vnode get, 0x%x should be 0x%x\n", fidp->fid_pdirent, (cdrp->cn_pdirent & ~0x1)));
		rc = ESTALE;
		goto exit;
	}

	/* Get the vnode for the cdrnode.  cdrptovp() will hold the vnode. */
	if (rc = cdrptovp(vfsp, cdrp, vpp))
		cdrput(cdrp);

exit:
	CDRFS_UNLOCK();
	return rc;
}

/*
 * NAME:	cdr_cntl(vfsp, cmd, arg, argsize, crp)
 *
 * FUNCTION:	CD-ROM file system VFS_CNTL().
 *		This function handles control operations for the CD-ROM
 *		filesystem.  Since extending the filesystem is the only
 *		vfs operation at the current time, this always returns
 *		EINVAL.
 *
 * PARAMETERS:	vfsp	- vfs to perform control operation on
 *		cmd	- operation to perform
 *		arg	- argument to operation
 *		argsize	- size of argument struct
 *
 * RETURN :	EINVAL	- no valid control operations for the CD-ROM filesystem
 */
static
int
cdr_cntl(
	struct vfs *		vfsp,
	int			cmd,
	caddr_t			arg,
	unsigned long		argsize,
	struct ucred *		crp)
{
	return EINVAL;
}

/*
 * NAME:	cdr_quotactl(vfsp, cmd, id, arg, crp)
 *
 * FUNCTION:	CD-ROM file system VFS_QUOTACTL().
 *		This function is not supported at this time.
 *
 * PARAMETERS:	vfsp	- vfs to perform quota control operation on
 *		cmd	- operation to perform
 *		id	- id to set
 *		arg	- size of argument struct
 *
 * RETURN :	EOPNOTSUPP - function is currently not supported.
 */
static
int
cdr_quotactl(
	struct vfs *		vfsp,
	int			cmd,
	uid_t			id,
	caddr_t 		arg,
	struct ucred *		crp)
{
	return EOPNOTSUPP;
}

/*
 * NAME:	cdrpmount(vfsp, fp, dev)
 *
 * FUNCTION:	This function is used by cdr_mount() to perform the
 *		physical mount of a device.
 *
 * PARAMETERS:	vfs	- virtual file system for the mount
 *		fp	- file pointer for the device
 *		dev	- device to mount
 *
 * RETURN :	0	- success
 *		errors from subroutines
 *
 * SERIALIZATION: We are called by cdr_mount which is serialized by the
 *		  cdrfs lock.
 */
static
int
cdrpmount(
	struct vfs		*vfsp,	/* virtual file system to mount	*/
	struct file *		fp,	/* file pointer for device	*/
	dev_t			dev)	/* device to mount		*/
{
	int			rc = 0;	/* return code			*/
	struct cdrfsmount	*cdrfsp; /* mounted cdrfs */
	struct cdrnode		*mntcdrp = NULL; /* mounted cdrfs cdrnode */
	struct cdrvd		*vds = NULL;
	struct cdrvd		*vd = NULL;
	struct cdrpvd		*pvd = NULL;	
	struct cdrxa_label	*cdrxalabel;
	int			format;	/* format standard for volume	*/
	daddr_t			lsn;	/* Logical Sector number */
	int			n, i;

	cdrfsp = (struct cdrfsmount *) vfsp->vfs_data;

	/* Search through the Logical Sectors of the Volume Descriptor
	 * Set area (from Logical Sector 16) of the Data Area
	 * for the Primary Volume Descriptor (PVD).
	 * Volume Descriptor Set area is read by n consecutive sectors 
	 * at a time where n = 2, 4, 6, ... and last two sectors are 
	 * probed for PVD in each try (this is to support multi-session
	 * disk with single address remapping by the CD-ROM device driver,
	 * i.e.,  CDRDD remaps the Logical Sector 16 to the Logical Sector 
	 * of the last/only session of the disk).
	 */
	for (n = 2, i = 0; ; n +=2, i += 2) {

		/* allocate the volume descriptor set area buffer */
		if ((vds = (struct cdrvd *)malloc(CDR_LSCTRSIZE * n)) == NULL)
			return ENOMEM;

		/* read in volume descriptor set area */
		if (rc = cdrbread(fp, PBLKOFFSET(LSCTR2PBLK(CDR_VDSAREA), 0), 
			  	  CDR_LSCTRSIZE * n, vds))
			goto out;

		for (lsn = CDR_VDSAREA + i, vd = vds + i; lsn < CDR_VDSAREA + n; 
		     lsn++, vd++) {

			/* determine the base format of the disk from 
		 	 * the stdid field of the volume descriptor.
		 	 * . ISO 9660:1988 - CD001
		 	 * . HSG - CDROM 
		 	 */
			if (strncmp(vd->vd_std_id, 
				    CDR_STDID_ISO9660, CDR_STDIDLEN) == 0) {
				if (vd->vd_voldesvers != 1) {
					rc = EFORMAT;
					goto out;
				}
				if (lsn = CDR_VDSAREA) {
					BUGLPR(buglevel, 5, ("volume in ISO 9660 format\n"));
					cdrfsp->fs_format = CDR_ISO9660;
					format = CDR_ISO9660;
				} else if (format != CDR_ISO9660) {
					rc = EFORMAT;
					goto out;
				}
			} else if (strncmp(HSVD(vd)->vd_std_id, 
				   CDR_STDID_HIGHSIERRA, CDR_STDIDLEN) == 0) {
				if (lsn = CDR_VDSAREA) {
					BUGLPR(buglevel, 5, ("volume in high sierra format\n"));
					cdrfsp->fs_format = CDR_HIGHSIERRA;
					format = CDR_HIGHSIERRA;
				} else if (format != CDR_HIGHSIERRA) {
					rc = EFORMAT;
					goto out;
				}
			} else {
				/* The disk is not in either format.  
				 * This is where we catch other disks, 
				 * particulary music disks.
				 */
				BUGLPR(buglevel, 7, ("non-standard format\n"));
				rc = EFORMAT;
				goto out;
			}

			/* probe for PVD */
			switch (format == CDR_ISO9660 ? vd->vd_voldestype
					: HSVD(vd)->vd_voldestype) {
			/* 
			 * skip Boot Records, Supplementary Volume Descriptors,
			 * and Volume Partition Descriptors.
			 * If we find the Volume Descriptor Set Terminator,
			 * it means we haven't found the Primary Volume
			 * Vescriptor, and this disk is not valid.
			 */
			case CDR_BR:
			case CDR_SVD:
			case CDR_VPD:
				break;
	
			case CDR_VDST:
				BUGLPR(buglevel, 7, ("primary volume descriptor not found\n"));
				rc = EFORMAT;
				goto out;
				break;

			case CDR_PVD:
				/* Get a cdrnode to represent the mounted file 
				 * system with the "inode" number of 0 and
				 * the device number of the mounted filesystem. 
	 	 	 	 */
				if (rc = cdrget(dev, 0, 0, &mntcdrp))
					goto out;
				BUGLPR(buglevel, 9, ("mount cdrnode:  0x%x\n", mntcdrp));
				mntcdrp->cn_cdrfs = cdrfsp;

				/* Make a copy of the PVD and attach it 
				 * to the cdrfsmount.
			 	 */
				if ((pvd = (struct cdrpvd *)malloc(CDR_VDSIZE)) 
					 == NULL) {
					rc = ENOMEM;
					goto out;
				}
				bcopy(vd, pvd, CDR_VDSIZE);
				cdrfsp->fs_pvd = pvd;

				/* compute the bit shift for logical block size 
		 	 	 * in the PVD to be able to calculate logical 
				 * offsets (using mount file system structure 
				 * cdrfsmount) from now on.
			 	 * Save the location of the root's directory 
				 * entry in the private data for the vfs to be 
				 * used by cdr_mount() to get the cdrnode for 
				 * the root (when creating the root vnode).
			 	 */
				if (format == CDR_ISO9660) {
					/* 
				 	 * ISO 9660 and extensions format 
				 	 */
					cdrfsp->fs_lblksize = pvd->pvd_lblksize;
					cdrfsp->fs_lblkshift = 
						log2shift(pvd->pvd_lblksize);
					cdrfsp->fs_rootdirent = LOFFSET(cdrfsp,
						SHORT2INT(pvd->pvd_rootdir.d_locext) +
						pvd->pvd_rootdir.d_xar_len, 0);

				} else {
					/* 
				 	 * High Sierra Group format 
				 	 */
					cdrfsp->fs_lblksize = 
						HSPVD(pvd)->pvd_lblksize;
					cdrfsp->fs_lblkshift =
						log2shift(HSPVD(pvd)->pvd_lblksize);
					cdrfsp->fs_rootdirent = LOFFSET(cdrfsp,
						SHORT2INT(HSPVD(pvd)->pvd_rootdir.d_locext) +
						HSPVD(pvd)->pvd_rootdir.d_xar_len, 0);
					if (HSPVD(pvd)->pvd_lblknum !=
						LSCTR2LBLK(cdrfsp, lsn)) {
						BUGLPR(buglevel, 7, ("lsn = %d, logical block = %d, lblk shift = %d\n", lsn, 
							HSPVD(pvd)->pvd_lblknum, 
							cdrfsp->fs_lblkshift));
						rc = EFORMAT;
						goto out;
					}
				}

				/* granularity sanity check:  
			 	 * logical sector size (2**11) >= 
			 	 * logical block size (2**(n+9)) >= 
			 	 * physical block size (2**9).
			 	 */
				if (cdrfsp->fs_lblkshift > CDR_LSCTRSHIFT ||
			    	    cdrfsp->fs_lblkshift < CDR_PBLKSHIFT) {
					BUGLPR(buglevel, 7, ("logical block shift = %d\n", 
						cdrfsp->fs_lblkshift));
					rc = EFORMAT;
					goto out;
				}

				rc = 0;
				goto iso9660_extension;
				break;

			default:
				/* Some unknown volume descriptor was found before we
			 	* found the primary volume descriptor.
			 	*/
				BUGLPR(buglevel, 7, ("unknown volume descriptor type:  %d\n",
					format == CDR_ISO9660 ? vd->vd_voldestype
						     : HSVD(vd)->vd_voldestype));
				rc = EFORMAT;
				goto out;
			}
		}

		/* free the volume descriptor set buffer */
		free(vds);
	}

iso9660_extension:
	/*
	 * test for ISO 9660 extension format
	 */
	if (format == CDR_ISO9660) {
		/* 
		 * test for CD-ROM XA extension 
		 */
		cdrxalabel = (struct cdrxa_label *) ((char *)pvd + CDRXA_LABEL_OFFSET);
		if (strncmp(cdrxalabel->signature, CDRXA_SIGNATURE,
		    CDRXA_SIGNATURE_LEN) == 0)
			cdrfsp->fs_format = CDR_XA;
					
		/* 
		 * test for Rock Ridge SUSP/RRIP extension
		 */
		else
			cdr_rrg_extension(cdrfsp);
	}

out:
	if (rc) {
		/* We've found an error somewhere. 
		 * If we've allocated the mount cdrnode, cdrput() it here.  
		 * If we've allocated a primary volume descriptor copy,
		 * free it here.
		 */
		if (mntcdrp)
			cdrput(mntcdrp);
		if (pvd)
			free(pvd);
	}

	/* free the volume descriptor set buffer */
	free(vds);

	return rc;
}

/*
 * NAME:	cdrpumount(dev)
 *
 * FUNCTION:	This function is used by cdr_umount() to perform the
 *		physical unmount of a device.
 *
 * PARAMETERS:	dev	- device to unmount
 *
 * RETURN:	0	- success
 *		errors from subroutines
 *
 * SERIALIZATION: We are called by cdr_umount which is serialized by the
 *		  cdrfs lock.
 *
 */
static
int
cdrpumount(dev_t		dev)		/* device to unmount	*/
{
	int			rc = 0;		/* return code		*/
	struct cdrnode *	mntcdrp;	/* device mount cdrnode	*/

	/* clean up mount cdrnode */
	mntcdrp = cdrfind(dev, 0);
	cdruncache(mntcdrp);
	cdrunhash(mntcdrp);

	/* Invalidate all the buffers in the buffer cache for this device. */
	binval(dev);

	return rc;
}

/*
 * NAME:	cdrptovp(vfsp, cdrp, vpp)
 *
 * FUNCTION:	This function gets a vnode for a cdrnode in the specified
 *		virtual file system.  It looks for a vnode for the cdrnode
 *		in the specified vfs.  If it doesn't find one, it creates
 *		a vnode and initializes the gnode, vnode and vfs linkages.
 *		The cdrnode must be held before calling this function.
 *		The cdrnode is always cdrput(), even on an error.  The
 *		returned vnode will already be held.
 *
 * PARAMETERS:	vfsp	- virtual file system where we want to find this
 *			  vnode
 *		cdrp	- cdrnode to get the vnode for
 *		vpp	- address to return the vnode
 *
 * RETURN:	0	- success
 *		ENOMEM	- no space is available for a new vnode
 */
int
cdrptovp(
	struct vfs *		vfsp,	/* vfs to find vnode in		*/
	struct cdrnode *	cdrp,	/* cdrnode to get the vnode for	*/
	struct vnode **		vpp)	/* address to return the vnode	*/
{
	int			rc;	/* return code			*/
	struct vnode *		vp;	/* vnode connected with cdrnode	*/
	struct gnode *		gp;	/* gnode connected with cdrnode	*/

	/* get the gnode from the cdrnode */
	gp = CDRTOGP(cdrp);

	/* Search through the file system objects (vnodes) associated with
	 * cdrnode looking for the one in the desired vfs.
	 */
	for (vp = gp->gn_vnode; vp; vp = vp->v_next)
		if (vp->v_vfsp == vfsp)
		{
			/* Release the cdrnode, hold the vnode, and return.
			 */
			cdrput(cdrp);
			VNOP_HOLD(vp);
			*vpp = vp;
			BUGLPR(buglevel, 9, ("found vnode 0x%x for cdrnode 0x%x in vfs\n", vp, cdrp));
			return 0;
		}

	/* We didn't find a vnode for the gnode in the appropriate vfs,
	 * so create a new vnode and initialize its flag field.  We
	 * cdrput() the cdrnode even on an error.
	 */
	if (rc = vn_get(vfsp, CDRTOGP(cdrp), &vp))
		cdrput(cdrp);
	else
	{
		BUGLPR(buglevel, 9, ("created vnode 0x%x for cdrnode 0x%x in vfs\n", vp, cdrp));
		/* this operation on the vfs is serialized with the CDRFS_LOCK */
		vp->v_vfsnext = vfsp->vfs_vnodes;	/* link off vfs */
		vp->v_vfsprev = NULL;
		vfsp->vfs_vnodes = vp;
		if (vp->v_vfsnext != NULL)
			vp->v_vfsnext->v_vfsprev = vp;

		if (cdrp->cn_dirent == CDRVFSDATA(vfsp)->fs_rootdirent)
			vp->v_flag = V_ROOT;
		else
			vp->v_flag = 0;
		*vpp = vp;
	}

	return rc;
}

/*
 *	cdr_rrg_extension
 *
 * function: called by cdrpmount() 
 *	     to identify SUSP/RRIP extension format on ISO 9660:1988
 *
 * note: SUSP/RRIP extension is identified by the '.' entry of
 * the root directory
 */
static
int
cdr_rrg_extension (
	struct cdrfsmount 	*cdrfsp) /* cdrfs mount structure */
{
	int	rc = 0;	
	daddr_t	rootdirent;
	struct cdrdirent *de;	/* directory record cursor */
	uchar	*sua, *suabp1;
	int	sualen;
	ushort	signature;
	struct rrg_suf		*suf;
	struct susp_sp_suf	*sp_suf;
	struct susp_er_suf	*er_suf;
	struct susp_ce_suf	*ce_suf;
	int	sp_found = FALSE;
	char	*ce_found = NULL;
	caddr_t	lsbuffer = NULL; /* malloc()ed Logical Sector buffer */

	/* get the buffer to read the root directory */
	if ((lsbuffer = malloc(CDR_LSCTRSIZE)) == NULL)
		return ENOMEM;

	rootdirent = cdrfsp->fs_rootdirent;

	/* read in the root directory */
	if (rc = cdrbread(cdrfsp->fs_fp, rootdirent, CDR_LSCTRSIZE, lsbuffer))
		goto done;

	/* locate the "." directory entry to read 
	 */
	de = (struct cdrdirent *) lsbuffer;

	/* determine system use area */
	sualen = de->d_drec_len - de->d_fileid_len - 33;
	if ((de->d_fileid_len & 0x01) == 0)
		sualen -= 1;
	if (sualen > 0) 
		sua = suabp1 = (char *)de + de->d_drec_len - sualen;
	else 
		goto done;

ca_sua: /* loop for continuation area extension for system use area */
	ce_found = NULL;

	while (sualen > 0 && *sua != 0x00) { /* watch for pad for sua */	
		suf = (struct rrg_suf *) sua; 

		/* process SUSP/RRIP SUFs */
		signature = BYTE2SHORT(suf->signature);
		switch (signature) {
		/*
 		 * 	SUSP_SP: SUSP indicator
 		 *
 		 * note: SP start at bp1 in SUA of the first directory record ("." entry)
 		 * of the root directory of each directory structure with SUSP
 		 */
		case SUSP_SP:
			/* SP start at bp1 of SUA */
			if (sua != suabp1)
				goto done;

			sp_suf = (struct susp_sp_suf *) suf;
			/* SP length : 7 */
			/* SP version: 1 */
			if (sp_suf->ver != 1)
				return EFORMAT;

			/* check bytes: 0xbeef */
			if (BYTE2SHORT(sp_suf->check_bytes) != CDR_SUSP_CHECKBYTES)
				return EFORMAT;
			/* number of bytes skipped within SUA before recording SUFs */
			cdrfsp->fs_rrg_sufoffset = sp_suf->len_skp;
			sp_found = TRUE;
			break;

		/*
 		 * 	SUSP_ER: extension reference
 		 *
 		 * note: SUSP ER with RRIP extension identifier must appear 
		 * in SUA of the first directory record ("." entry)
 		 * of the root directory of each directory structure with RRIP
 		 */
		case SUSP_ER: 
			if (sp_found != TRUE)
				goto done;

			er_suf = (struct susp_er_suf *) suf;
			/* ER length: 8 + 10 + 84 + 135 = 237 */
	 		/* ER version: 1 */
			if (er_suf->ver != 1)
				return EFORMAT;

			/* identifier length: 10 */
			if (strncmp(er_suf->ext_id, CDR_RRIP_EXTID, 10) != 0)
				return EFORMAT;
			
			cdrfsp->fs_format = CDR_ROCKRIDGE;
			goto done;
			break;

		/* 
 		 *	SUSP_CE: continuation area (CA) 
		 *
		 * note: Each CA consist of a single Logical Sector.
		 * The CA specified by the current CE SUF should be processed 
		 * after the current SUA or CA is processed.
 		 */
		case SUSP_CE:
			ce_found = sua;
			break;


		default:
			break;
		} /* end switch(signature) */
		
		sua += suf->len;
		sualen -= suf->len;
	} /* end while */

	/*
	 * check for continuation area
	 */
	if (ce_found) {
		ce_suf = (struct susp_ce_suf *) ce_found;
		sua = lsbuffer + BYTE2INT(ce_suf->offset);
		sualen = BYTE2INT(ce_suf->len_cont);
		if (rc = cdrbread(cdrfsp->fs_fp, LOFFSET(cdrfsp, BYTE2INT(ce_suf->location), 0), CDR_LSCTRSIZE, lsbuffer))
			goto done;
		goto ca_sua;
	}

done:
	if (lsbuffer)
		free(lsbuffer);
	return rc;
}

/*
 * NAME:	cdrbread(fp, daddr, len, buf)
 *
 * FUNCTION:	This function reads from the CD-ROM and places
 *		the data in the specified buffer.  
 *
 * PARAMETERS:	fp	- file pointer for CD-ROM device
 *		offset  - start disk address in byte
 *		len	- length of bytes to read
 *		buf	- address of buffer to put sector data in
 *
 * RETURN:	0	- success
 *		EIO	- could not read len bytes of data
 *		errors from subroutines
 *
 * SERIALIZATION: We may or may not be holding the cdrfs lock. But
 *		  we should release in before calls out to the LFS.
 */
int
cdrbread(
	struct file *		fp,	/* file pointer for device	*/
	daddr_t			daddr,	/* start disk address to read */
	size_t			len,	/* length of bytes to read */
	void *			buf)	/* address of buffer to fill	*/
{
	int			rc = 0;	/* return code			*/
	int			count;  /* number of bytes read	*/
	int			locked; /* do we have the cdrfs lock?	*/

	/* 
	 * If we have the cdrfs lock release it around the calls out of 
	 * the CDROM to insure better paralelization.
	 */
	if (locked = lock_mine(&cdrfs_lock))
		CDRFS_UNLOCK();
	if (rc = fp_lseek(fp, daddr, SEEK_SET))
		goto exit;
	if (rc = fp_read(fp, buf, len, 0, UIO_SYSSPACE, &count))
		goto exit;
	if (count != len)
		rc = EIO;
exit:
	if (locked)
		CDRFS_LOCK();
	return rc;
}

/*
 * NAME:	log2shift(num)
 *
 * FUNCTION:	This function calculates the log base 2 of a number.
 *
 * PARAMETERS:	num	- number to find log base 2 of
 *
 * RETURN:	log base 2 of number
 *		-1	- number not positive or number not a power of two
 */
int
log2shift(register uint	num)
{
	register uint		shift = 0;

	while (num > 0)
	{
		/* if num is one, success */
		if (num == 1)
			return shift;

		/* if low bit set, num not power of 2 */
		if (num & 1)
			return -1;

		/* iterate */
		++shift;
		num >>= 1;
	}

	/* num not positive */
	return -1;
}
