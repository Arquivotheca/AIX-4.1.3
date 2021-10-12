static char sccsid[] = "@(#)39	1.37  src/bos/kernel/pfs/xix_umount.c, syspfs, bos41J, 9507C 2/14/95 13:45:24";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_umount
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/syspest.h"
#include "sys/sysinfo.h"
#include "sys/malloc.h"
#include  "vmm/vmsys.h"

extern 	struct inode *ifind();

/*
 * NAME:	jfs_umount (vfsp, flags, crp)
 *
 * FUNCTION:	Unmount vfs operation
 *
 * PARAMETERS:	vfsp	- virtual file system pointer
 *		flags	- unmount for shutdown
 *		crp	- credential
 *
 * RETURN :	EBUSY	- device has open files
 */

jfs_umount (vfsp, flags, crp)
register struct vfs	*vfsp;
int	 		flags;
struct ucred		*crp;		/* pointer to credential structure */
{
        dev_t dev;
        struct inode *root;
        struct gnode *gp;
	struct vnode *vp, *rootv;
	int forced, ronly, oflag, rc = 0;

	/*
	 * file-on-file or directory-on-directory mount:
	 *
	 * nearly all the work is file system independent and
	 * is done up in the vnode layer by kunmount().
	 */
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT)) {
		rootv = vfsp->vfs_mntd;
		if (rootv->v_count > 1)
			return (EBUSY);
		vfsp->vfs_mntd = NULL;
		jfs_rele(rootv);
		return (0);	
	}

	/* device mount
	 */
	gp = (struct gnode *)vfsp->vfs_data;
        dev = gp->gn_rdev;

	/* Purge directory name lookup cache. 
	 */
	dnlc_purge(dev);

	/* There can be no files active in a device
	 * being unmounted (unless this is a shutdown
	 * unmount, in which case we force the vfs to be
	 * unmounted anyway).
	 */
	forced = (vfsp->vfs_flag & VFS_SHUTDOWN);
	if ((rc = iactivity(dev, forced)) && !forced)
	{
		return(rc);
	}

	ICACHE_LOCK();

	/* point of no return for unmount: 
	 * ignore failure of further intermediate steps and
	 * return success to notify LFS to cleanup.
	 */

	rootv = vfsp->vfs_mntd;
	root = VTOIP(rootv);
	assert(root->i_count == 1);
	vfsp->vfs_mntd = NULL;

	/* disable quotas
	 */
	dqumount(root->i_ipmnt->i_jmpmnt);

	/* close root vode/inode:
	 * (bypass jfs_rele() which takes ICACHE_LOCK() to avoid deadlock)
	 */
	iuncache(root);
	iunhash(root);
	ICACHE_UNLOCK();

	/* close file system files
	 */
	ronly = (vfsp->vfs_flag & VFS_READONLY);
	(void) pumount(dev, ronly);

	/* close device
	 */
	oflag = FMOUNT | ((ronly)? FREAD: FREAD|FWRITE);
	(void) rdevclose(gp, oflag, 0);

	return 0;
}

/*
 * NAME:	pumount (dev, ronly)
 *
 * FUNCTION:	logical unmount of device mount file syste.
 *		file system special file inodes are unbound with 
 *		memory object and freed.
 *
 * PARAMETERS:	dev	- device to unmount
 *
 * RETURN :	Errors from subroutines.
 *			
 * SERIALIZATION: ICACHE_LOCK() is NOT held on entry/exit
 */

pumount (dev, ronly)
dev_t	dev;
int	ronly;
{
	int	rc = 0, logdev, sr12save, k, errors = 0;
	struct 	inode *ipmnt, *iplog, *ipind, *is[16], **ipp = is;
	struct superblock *sb;
	struct jfsmount *jmp;
	struct vmdlist *anchor;
	union xptentry *xpt;

	/* get the mount inode representing the mounted filesystem.
	 * (dev_t of the mounted filesystem, i_number of 0)
	 */
	ICACHE_LOCK();
	ipmnt = ifind(dev);
	ICACHE_UNLOCK();

	iplog = ipmnt->i_iplog;
	logdev = iplog->i_dev;
	jmp = ipmnt->i_jmpmnt;
	assert(jmp);

	/*
	 * Free the indirect blocks referenced by INDIR_I.
	 *
	 * The first blocks 2 are reserved. Another oddity
	 * is that v_findiblk() maintains indirect blocks
	 * 0 to NDADDR in the inode and blocks NDADDR to FIRSTIND
	 * in page 1 of .indirect.
	 */
	ipind = ipmnt->i_ipind;
	sr12save = chgsr(12, SRVAL(ipind->i_seg, 0, 0));

	anchor = NULL;

	xpt = (union xptentry *) (ipind->i_rdaddr);
	for (k = 2; k < NDADDR; k++)
	{
		if (xpt[k].word)
			dlistadd(&anchor, xpt[k].word);

	}

	xpt = (union xptentry *)(SR12ADDR + PAGESIZE);
	for (k = NDADDR; k < FIRSTIND; k++)
	{
		if (xpt[k].word)
			dlistadd(&anchor, xpt[k].word);
	}

	freedisk(ipind, anchor);
	mtsr(12, sr12save);	
	
	/*
	 * close file system files 
	 */
	*ipp++ = ipmnt->i_ipinodex;
	*ipp++ = ipmnt->i_ipinomap;
	*ipp++ = ipmnt->i_ipinode;
	*ipp++ = ipmnt->i_ipind;
	*ipp = ipmnt->i_ipdmap;		/* must be last dynamic special */

	for (k = 0; k <= (ipp - is); k++)	
	{
		if (rc = iflush(is[k]))
			errors++;
		isegdel(is[k]);
	}

	ICACHE_LOCK();
	for (k = 0; k <= (ipp - is); k++)	
		iunhash(is[k]);
	ICACHE_UNLOCK();

	/*
	 * close superblock: set state in superblock if mounted read-write
	 *
	 * mark it as unmounted-cleanly if mounted normally; 
	 * otherwise state is dirty.
	 */
	if (!ronly)
	{	
		label_t	jb;
		
		/* establish exception return point for io errors. 
		 * errors may occur as result of loads or stores 
		 * into memory mapped files.
		 */
		if ((rc = setjmpx(&jb)) == 0)
		{
			iptovaddr(ipmnt->i_ipsuper, 0, &sb);

			/* if mounted normally set state to clean.
			 */
			if (sb->s_fmod == FM_MOUNT)
				sb->s_fmod = FM_CLEAN;

			clrjmpx(&jb);		/* pop exception return */
		}

		/* if exception above, the rest of this will with
		 * out doubt fail.
		 */

		ipundo(sb);

		/* write it out and wait for i/o to finish.
		 */
		iflush(ipmnt->i_ipsuper);
		vms_iowait(ipmnt->i_ipsuper->i_seg);

		/* delete the segment. 
		 */
		isegdel(ipmnt->i_ipsuper);
	}
	else
		assert(iplog == NULL);

	ICACHE_LOCK();

	iunhash(ipmnt->i_ipsuper);

	/*
	 * close "mount" inode.
	 */
	iunhash(ipmnt);

	ICACHE_UNLOCK();

	/*
	 * close log: remove device from its active list.
	 */
	if (!ronly)
		rc = logclose(iplog, dev);

	/* free mount structure storage.
	 */
	 free(jmp);

	/* Return buf structs allocated by mount 
	 */
	rc = vm_umount(D_FILESYSTEM,dev);

	/* XXX. Temporary hack until charater devices for fs cmds
	 */
	binval(dev);

	return rc;
}
