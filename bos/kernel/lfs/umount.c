static char sccsid[] = "@(#)68	1.26.1.16  src/bos/kernel/lfs/umount.c, syslfs, bos41B, 412_41B_sync 12/6/94 16:47:24";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: umount, uvmount, kunmount, vfsrele, f_umount,
 *	newroot, memobj_rel
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/fs_locks.h"
#include "sys/vattr.h"
#include "sys/access.h"
#include "sys/syspest.h"
#include "sys/gpai.h"
#include "sys/malloc.h"
#include "sys/priv.h"
#include "sys/ldr.h"
#include "sys/audit.h"
#include "sys/sleep.h"

BUGVDEF(udebug, 0);
BUGVDEF(unoise, 0);
BUGVDEF(undebug, 0);

/* initobj externals */
extern int init_obj_end, pg_obj_end, pg_com_end, endcomm;

extern struct galloc gpa_vfs;	/* header used to allocate vfs's (or free) */

extern struct vfs *rootvfs;

extern int umount_elist;	/* global umount event list */

/*
 * umount  --   Front-end to umount system call code for
 *		traditional AIX umount (i.e., unmounts of local devices).
 */
/************************************************************************
 *
 * NAME:	umount system call
 *
 * FUNCTION:	The traditional AIX umount for unmounting of local
 *		devices over local directories.
 *
 * PARAMETERS:	device	- name of device to be unmounted.
 *
 * RETURNS:	zero on success, -1 on error. Also the device device
 *		is unmounted.
 *
 ************************************************************************/
umount(device)
char	*device;
{
	extern        kernel_lock;  /* global kernel lock  */
	static int    svcnum = 0;
	struct vnode  *vp;
	struct vfs    *vfsp;
	int           lockt,  /* previous state of kernel lock */
	              svcrc = 0,
	              audit_name = 1;
	int	      error = 0;
	struct ucred  *crp;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	BUGLPR(udebug, BUGNFO, ("Umounting %s\n", device));
	if (audit_flag)
		svcrc = audit_svcstart("FS_Umount", &svcnum, 0 );

	/* Get creds for this operation. */
	crp = crref();

	/*
	 * Look up the thing to be unmounted.
	 */
	error = lookupname(device, USR, L_SEARCH, NULL, &vp, crp);
	if (error) {
		BUGLPR(udebug, BUGNFO, ("Lookupname failed\n"));
		goto fastout;
	}
	/*
	 * Umount(2) is invoked with the name
	 * of the mounted block device; the device
	 * number is used to find the appropriate
	 * file system in the vfs list.
	 */
	if (vp->v_vntype != VBLK) {
		BUGLPR(udebug, BUGACT, ("%s is not a block device\n",
			device));
		error = ENOTBLK;
		goto out;
	}

	/*
	 * The loader keeps shared libraries open even when they
 	 * are not currently being used. L_PURGE causes the loader
	 * to close all inactive shared libraries.
	 */
	unloadx (L_PURGE);

	VFS_LIST_LOCK();
	for (vfsp = rootvfs; vfsp != NULL; vfsp = vfsp->vfs_next) {
		/*
		 * Examine all local device mounts.
		 */
		if ((vfsp->vfs_flag & VFS_DEVMOUNT) == 0)
			continue;
		if ((vfsp->vfs_flag & VFS_UNMOUNTING) != 0)
			continue;
		/* One must use the brdev macro here. */
		if (brdev(vfsp->vfs_fsid.fsid_dev) == brdev(vp->v_rdev) )
		{
			BUGLPR(udebug, BUGNFO, ("Match @ 0x%x\n", vfsp));
			/* must check for busy while holding vfs_list_lock */
			if (((error = mount_busy(vfsp)) != 0)
			    || (vfsp->vfs_count != 1))
			{
				vfsp = NULL;
				error = EBUSY;
			}
			else
				vfsp->vfs_flag |= VFS_UNMOUNTING;
			break;
		}
	}
	VFS_LIST_UNLOCK();

	if (vfsp == NULL)
	{
		/* error is set to EBUSY if mount_busy failed above;
		 * if that is not the case, then we did not find the vfs
		 * so set the error accordingly.
		 */
		if (!error)
			error = EINVAL;
		goto out;
	}

	if(svcrc) {
		if (vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT))
			audit_svcbcopy(vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT),
			  strlen(vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT)) + 1);
		if (vmt2dataptr(vfsp->vfs_mdata, VMT_STUB))
			audit_svcbcopy(vmt2dataptr(vfsp->vfs_mdata, VMT_STUB),
			  strlen(vmt2dataptr(vfsp->vfs_mdata, VMT_STUB)) + 1);
		audit_name = 0;
	}
	error = kunmount(vfsp, 0, crp);

	if (error)
	{
		VFS_LIST_LOCK();
		vfsp->vfs_flag &= ~VFS_UNMOUNTING;
		VFS_LIST_UNLOCK();
	}
	else
	{
		/* decrement the hold on the gfs */
		GFS_LOCK();
		vfsp->vfs_gfs->gfs_hold--;
		GFS_UNLOCK();

		if (vfsp->vfs_count == 0)
			vfsrele(vfsp);
	}

out:
	VNOP_RELE(vp);
fastout:

	if(svcrc && audit_name && device){
		char *ptr;
		int len;
	
		if((ptr = malloc(MAXPATHLEN)) == NULL)
			error = ENOMEM;
		else {
			if(copyinstr(device, ptr, MAXPATHLEN, &len))
				error = EFAULT;
			else
				audit_svcbcopy(ptr, len);
			free(ptr);
		}
	}

	crfree(crp);

	/* wakeup lookups sleeping on umount event list */
	e_wakeup(&umount_elist);

	if(svcrc)
		audit_svcfinis();

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (error)
		u.u_error = error;
	return error ? -1 : 0;
}

/*
 * uvmount --   Front-end to uvmount system call code for (possibly) remote
 *              unmounts of files, directories, and local devices.
 */
/************************************************************************
 *
 * NAME:	uvmount system call
 *
 * FUNCTION:	The new AIX umount for unmounting of local devices or
 *		local or remote directories over local directories.
 *		Also unmounts local or remote files over local files.
 *
 * PARAMETERS:	vfs_no	- The unique number given to every filesystem
 *				upon mounting. Find it using mntctl.
 *		flag	- only UVMNT_FORCE defined, but flag is passed
 *				on unchecked to the filesystem umount.
 *
 * RETURNS:	zero on success, -1 on error. Also the requested
 *		unmount operation is performed.
 *
 ************************************************************************/
uvmount(vfs_no, flag)
int	vfs_no;
int	flag;
{
	struct vfs  *vfsp;
	extern      kernel_lock; /* global kernel lock */
	static int  svcnum = 0;
	int         lockt,       /* previous state of kernel lock */
	            svcrc = 0;
	int	    error = 0;
	int	    maxvcount = 0; /* number of active references on vfs */
	struct ucred *crp;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	BUGLPR(udebug, BUGNFO, ("Uvmounting %d: flag = %d\n", vfs_no, flag));
	if (audit_flag)
		svcrc = audit_svcstart("FS_Umount", &svcnum, 0);

	/*
	 * The loader keeps shared libraries open even when they
 	 * are not currently being used. L_PURGE causes the loader
	 * to close all inactive shared libraries.
	 */
	unloadx (L_PURGE);

	/*
	 * Look up the thing to be unmounted.
	 */
	VFS_LIST_LOCK();
	for (vfsp = rootvfs; vfsp; vfsp = vfsp->vfs_next) {
		if (vfsp->vfs_number == vfs_no)
		{
			/*
			 * for device mount vfs the vfs_count is 1 for
			 * inactive references on the vfs. for soft
			 * mount vfs(s) the vfs_count reflects the number
			 * of active vnodes on the vfs including the root
			 * vnode. in the case of an iactive soft mount
			 * vfs the count is 2.
			 */
			maxvcount = (vfsp->vfs_flag & VFS_SOFT_MOUNT) ? 2 : 1;

			if ((vfsp->vfs_flag & VFS_UNMOUNTING) ||
			     ((error = mount_busy(vfsp)) != 0) ||
			     ((vfsp->vfs_count > maxvcount) &&
			     (!(flag & UVMNT_FORCE))))
			{
				vfsp = NULL;
				error = EBUSY;
			}
			else
				vfsp->vfs_flag |= VFS_UNMOUNTING;
			break;
		}
	}
	VFS_LIST_UNLOCK();

	if (vfsp == NULL)
	{
		/* error is set to EBUSY if mount_busy failed above;
		 * if that is not the case, then we did not find the vfs
		 * so set the error accordingly.
		 */
		if (!error)
			error = EINVAL;
		goto out;
	}

	if(svcrc) {
		if (vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT))
			audit_svcbcopy(vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT),
			  strlen(vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT)) + 1);
		if (vmt2dataptr(vfsp->vfs_mdata, VMT_STUB))
			audit_svcbcopy(vmt2dataptr(vfsp->vfs_mdata, VMT_STUB),
			  strlen(vmt2dataptr(vfsp->vfs_mdata, VMT_STUB)) + 1);
	}

	crp = crref();

	error = kunmount(vfsp, flag, crp);
	
	crfree(crp);

	if (error)
	{
		VFS_LIST_LOCK();
		vfsp->vfs_flag &= ~VFS_UNMOUNTING;
		VFS_LIST_UNLOCK();
	}
	else
	{
		/* decrement the hold on the gfs */
		GFS_LOCK();
		vfsp->vfs_gfs->gfs_hold--;
		GFS_UNLOCK();

		/*
		 * If there are no active files in this vfs, release it.
		 */
		if (vfsp->vfs_count == 0)
			vfsrele(vfsp);
	}

out:
	/* wakeup lookups sleeping on umount event list */
	e_wakeup(&umount_elist);

	if (svcrc)
		audit_svcfinis();

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (error)
		u.u_error = error;
	return error ? -1 : 0;
}

/*
 * kunmount --   Common code for all varieties of umount system call.
 *
 * NOTE: The vfs must be write_locked on entry to this function.
 */
int
kunmount(vfsp, flag, crp)
struct vfs	*vfsp;
int		flag;
struct ucred    *crp;
{
	int		   error = 0;
	extern struct vfs *rootvfs;
	struct vnode      *stubvp;

	if (vfsp->vfs_flag & VFS_DEVMOUNT) {
		/*
		 * A device is to be unmounted;
		 * the user must be running as superuser.
		 */
		if (!priv_req(FS_CONFIG)) {
			BUGLPR(udebug, BUGACT,
				("Device uvmount rejected\n"));
			return(EPERM);
		}
	} else {
		/*
		 * A directory or file is to be unmounted;
		 * the user must either be superuser or
		 * a writer in the stub's parent directory
		 */
		if (!priv_req(FS_CONFIG) && vfsp->vfs_mntdover &&
		    VNOP_ACCESS(vfsp->vfs_mntdover, W_ACC, NULL, crp)) {
			BUGLPR(udebug, BUGACT, ("Virtual unmount rejected\n"));
			return(EPERM);
		}
		/* When the last vnode is released it will drop the vfs */
		vfsp->vfs_flag |= VFS_UNMOUNTING; 
	}

	/* We could be the second process attempting an umount
	 * of this vfsp. Below we set the v_mvfsp to NULL, this
	 * check should blow the second guy out.
	 */
	if ((stubvp = vfsp->vfs_mntdover) != NULL)
	{
		VN_LOCK(stubvp);
		if (vfsp->vfs_mntdover->v_mvfsp != vfsp) {
			BUGLPR(udebug, BUGACT,
				("vfs_mntdover->v_mvfsp != vfsp (0x%x)!\n",
				vfsp->vfs_mntdover));
			return(EINVAL);
		}
		vfsp->vfs_mntdover->v_mvfsp = NULL;
	}

	/*
	 * Switch to file system dependent code
	 * to perform low-level unmount functions.
	 */
	if (error = VFS_UNMOUNT(vfsp, flag, crp)) {
		/*
		 * The file system cannot be unmounted,
		 * so scram.
		 */
		BUGLPR(udebug, BUGACT, ("vfs_umount failed, errno %d\n",
			error));
		if (stubvp)
			vfsp->vfs_mntdover->v_mvfsp = vfsp;
		vfsp->vfs_flag &= ~VFS_UNMOUNTING;
	}
	else
	{
		/* indicate successful unmount has happened;
		 * in some file system types the vfs may hang
		 * around for a while with stale vnodes 
		 */
		vfsp->vfs_flag |= VFS_UNMOUNTED;

		/* must decrement the initial hold count when 
 		 * the vfs was first mounted.
		 */
		vfs_unhold(vfsp);
	}

	if (stubvp)
		VN_UNLOCK(stubvp);

	return(error);
}

static int
mount_busy(struct vfs *vfsp)
{
	struct vfs *tmpvfsp;

	if (!(vfsp->vfs_gfs->gfs_flags & GFS_FUMNT))
	{
		/*
		 * This vfs can't be unmounted if it contains
		 * mount points (stubs) for other file systems.
		 * If the stub contains mount points their vfs
		 * should be between vfsp->vfs->next and the end
		 * of the list, becouse the mount code puts the
		 * new vfs at the end of the list.
		 */
		for (tmpvfsp = vfsp->vfs_next;
		     tmpvfsp;
		     tmpvfsp = tmpvfsp->vfs_next)
			if (tmpvfsp->vfs_mntdover
			    && tmpvfsp->vfs_mntdover->v_vfsp == vfsp
			    && !(tmpvfsp->vfs_flag & VFS_SOFT_MOUNT))
			{
				BUGLPR(udebug, BUGACT,
					("Vfs contains mount points\n"));
				return(EBUSY);
			}

		/*
		 * We want the root vnode of a device-mounted file system
		 * to be deallocated upon VNOP_RELEase, otherwise subsequent
		 * mounts and unmounts of the device will be inhibited by the
		 * root inode's use count (1 per vnode, with old root vnodes
		 * stacked up to the ceiling).  Therefore, the unmounting
		 * of a device will be disallowed if its root vnode is not
		 * dormant.
		 */
		if ((vfsp->vfs_flag & VFS_DEVMOUNT)
		    && (vfsp->vfs_mntd->v_count > 1))
		{ 
			BUGLPR(udebug, BUGACT, ("Root vnode busy (count %d)\n",
				vfsp->vfs_mntd->v_count));
			return(EBUSY);
		}
	}
	return(0);
}

/*
 * vfsrele --	Code to deallocate a vfs
 *
 * This is normally called from umount or uvmount.  In the case
 * of a disconnected mount it will be called from the file system 
 * VNOP_RELE function.
 */
vfsrele(vfsp)
register struct vfs *vfsp;
{
	register struct vfs *tvfsp, *lvfsp;

	BUGLPR(udebug, BUGACT, ("Trashing an old vfs @ 0x%x\n", vfsp));
	/*
	 * Unlink the dead vfs from the vfs list.
	 */
	VFS_LIST_LOCK();

	for (tvfsp = rootvfs, lvfsp = NULL;
	     tvfsp != NULL; 
	     lvfsp = tvfsp, tvfsp = tvfsp->vfs_next )
		if (tvfsp == vfsp)
			break;

	if ( lvfsp == NULL )
		rootvfs = tvfsp->vfs_next;
	else
		lvfsp->vfs_next = tvfsp->vfs_next;

	VFS_LIST_UNLOCK();

	/*
	 * Release the file system stub (mount point).
	 */
	BUGLPR(udebug, BUGACT, ("Releasing stub vp 0x%x (vfsp 0x%x)...\n",
		vfsp->vfs_mntdover ? vfsp->vfs_mntdover : 0,
		vfsp->vfs_mntdover ? vfsp->vfs_mntdover->v_vfsp : 0));

	if ( vfsp->vfs_mntdover )	/* should be NULL only for rootvfs */
		VNOP_RELE(vfsp->vfs_mntdover);

	/*
	 * Get rid of the dead vfs' rotting parts.
	 */
	free(vfsp->vfs_mdata);
	gpai_free(&gpa_vfs, vfsp);
}

extern dev_t rootdev;
extern int root_vfstype;		/* do we still need this? */

/* Forceful unmount. Called from rexit() when init dies.  Unmount all devices.
 * set up new root if we find something else mounted over root.
 */
void
f_umount()
{
	ulong flags;
	struct vfs *vfsp;	/* Varying vfsp */
	struct vfs *ldvfsp;	/* Last vfsp in vfs chain */
	struct vfs *newrootvfs;	/* new root vfs */
#ifdef DEBUG
	struct vnode *vp;
#endif /* DEBUG */
	extern struct vfs *rootvfs;
	struct ucred *crp = crref();

	sync();		  	/* Sync all file system types */

	/*
	 * try to find the 'new root' vfs.
	 */
	newrootvfs = NULL;
	VFS_LIST_LOCK();
	for(vfsp = rootvfs; vfsp->vfs_next != NULL; vfsp = vfsp->vfs_next)
		/*
		 * are you a root?
		 */
		if (vmt2datasize(vfsp->vfs_next->vfs_mdata, VMT_STUB) > 0 &&
		    strcmp(vmt2dataptr(vfsp->vfs_next->vfs_mdata, VMT_STUB),
			   "/") == 0) {
			newrootvfs = vfsp->vfs_next;
			/*
			 * terminate the list before the new root vfs so that
			 * the unmounting won't get this far!
			 */
			vfsp->vfs_next = NULL;
			break;
		}
	VFS_LIST_UNLOCK();

	/*
	 * found one that looks like a root?  if not, skip the forced
	 * unmount...  i.e. retain some root...
	 */
	if (newrootvfs != NULL) {
		/*
		 * release the mounted over vnode
		 */
		assert(newrootvfs->vfs_mntdover != NULL);
		VNOP_RELE(newrootvfs->vfs_mntdover);
		newrootvfs->vfs_mntdover = NULL;

		VNOP_RELE(U.U_cdir); /* release proc[0]'s current directory */

#ifdef DEBUG
		BUGLPR(undebug, BUGNFO, ("found a new root!\nold mounts:\n"));
		/* ASSERT(vfs_list_lock != U.U_procp->p_pid); */
		VFS_LIST_LOCK();
		for(vfsp = rootvfs; vfsp; vfsp = vfsp->vfs_next)
			BUGLPR(undebug, BUGNFO, ("%s over %s\n",
			       vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT),
			       vmt2dataptr(vfsp->vfs_mdata, VMT_STUB)));
		BUGLPR(undebug, BUGNFO, ("\nnew mounts:\n"));
		for(vfsp = newrootvfs; vfsp; vfsp = vfsp->vfs_next)
			BUGLPR(undebug, BUGNFO, ("%s over %s\n",
			       vmt2dataptr(vfsp->vfs_mdata, VMT_OBJECT),
			       vmt2dataptr(vfsp->vfs_mdata, VMT_STUB)));
		VFS_LIST_UNLOCK();
#endif /* DEBUG */

		flags = VFS_UNMOUNTING;
		/* Unmount all devices */
		for(;;)
		{
			/* Find the last vfs in the list.
			 * This list is guaranteed to be in the order the
			 * mounts were performed.  This allows us to 
			 * unwind them in a sane order.
			 */
			ldvfsp = NULL;
			VFS_LIST_LOCK();
			for(vfsp = rootvfs; vfsp; vfsp = vfsp->vfs_next)
				if (!(vfsp->vfs_flag & VFS_UNMOUNTING))
					ldvfsp = vfsp;
			VFS_LIST_UNLOCK();

			if (ldvfsp == NULL)
				break;		/* no more mounts */

			/* Force unmount
			 */
			BUGLPR(undebug, BUGNFO, ("Last mount: %s over %s\n",
				vmt2dataptr(ldvfsp->vfs_mdata, VMT_OBJECT),
				vmt2dataptr(ldvfsp->vfs_mdata, VMT_STUB)));

			/* I may need a lock here */
			ldvfsp->vfs_flag |= flags;

			if (VFS_UNMOUNT(ldvfsp, 0, crp))
			{	BUGLPR(undebug, BUGACT, 
					("VFS_UMOUNT failed!\n"));

				ldvfsp->vfs_flag &= ~flags;

				/*
				 * the unmount failed for some reason.  so
				 * we'll yank this guy off of the list to be
				 * unmounted, and stick him on the new root
				 * list so we can look at him later
				 */
				if (ldvfsp == rootvfs)
					rootvfs = rootvfs->vfs_next;
				else {
					/* find him */
					for(vfsp = rootvfs; vfsp->vfs_next &&
					    vfsp->vfs_next != ldvfsp;
					    vfsp = vfsp->vfs_next)
						;
					/* unlink him from the list
					 */
					if (vfsp->vfs_next != NULL)
						vfsp->vfs_next = ldvfsp->vfs_next;
					}
				/* link him behind newroot
				 */
				ldvfsp->vfs_next = newrootvfs->vfs_next;
				newrootvfs->vfs_next = ldvfsp;
				continue;
			}
			else
				/*
				 * remove the vfs from the list
				 */	
				vfsrele(ldvfsp);

		}

		/*
		 * take care of the new root vfs...
		 */
		rootvfs = newrootvfs;
		rootdir = rootvfs->vfs_mntd;
		U.U_cdir = rootdir;
		VNOP_HOLD(U.U_cdir);

		rootdev = (VTOGP(rootdir))->gn_rdev;
		root_vfstype = rootvfs->vfs_gfs->gfs_type;
	}
	crfree(crp);
}

newroot()
{
	int lockt;
	dev_t Orootdev;
	extern int kernel_lock;

	unloadx (L_PURGE);

	lockt = FS_KLOCK(&kernel_lock,LOCK_SHORT);

	Orootdev = rootdev;	/* save old root device number */
	f_umount();		/* force unmount all file systems */
	devswdel(Orootdev);	/* delete old root filesystem device */

	/* Release the initobj objects */
	memobj_rel(&pg_obj_end, (int)&init_obj_end - (int)&pg_obj_end - 1);
	memobj_rel(&pg_com_end, (int)&endcomm - (int)&pg_com_end - 1);

	proc1restart();		/* restart /etc/init */

	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);
}

/*
 * NAME:  memobj_rel
 *
 * FUNCTION:
 *      This routine will vm_release() whole pages of a given region.
 *
 * RETURN VALUE DESCRIPTION: None
 *
 * EXTERNAL PROCEDURES CALLED:
 *	vm_release
 */

memobj_rel(reladdr, relsize)
caddr_t	reladdr;		/* Release starting address */
int	relsize;		/* Release size */
{
	int	x;		/* temp space */
	
#ifdef DEBUG
	bzero( reladdr, relsize);
#endif /* DEBUG */

	/* Round to next page address and decrease release size if
	 * not already page aligned.
	 *
	 * If relsize < PAGESIZE then nothing to release.
	 */
	if( (int)reladdr & (PAGESIZE-1) ) {
		x = PAGESIZE - ((int)reladdr & (PAGESIZE-1));
		reladdr = (int)reladdr + x;
		relsize -= x;
	}

	if( relsize < PAGESIZE )
		return;
	
	/* Something to release so round to whole number of pages */

	relsize &= ~(PAGESIZE-1);

	vm_release( reladdr, relsize);

	return;
}
