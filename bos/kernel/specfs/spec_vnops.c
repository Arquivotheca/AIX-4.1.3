static char sccsid[] = "@(#)02	1.24.1.7  src/bos/kernel/specfs/spec_vnops.c, sysspecfs, bos41J, 9521A_all 5/23/95 08:02:57";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: spec_badop, spec_noop, spec_open, spec_close, spec_rdwr
 *            spec_select, spec_ioctl, spec_strategy, spec_lookup
 *            spec_link, spec_getattr, spec_remove, spec_rename
 *
 * ORIGINS: 3, 24, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986 by Sun Microsystems, Inc.
 */

/*
 * NOTE: What you find here is as follows.  Any special requirements on
 *       behalf of devices from the native vnode operations are not
 *	 implemented in line.  They are forced through this "sanitizer"
 *	 in an attempt to isolate device specific function from non-device
 *	 specific function, especially mulitplexed devices and especially
 *	 in the area of security.  We belive the performance cost of this 
 * 	 implementation to be negligible.
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/uio.h"
#include "sys/file.h"
#include "sys/flock.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/device.h"
#include "sys/malloc.h"
#include "sys/poll.h"
#include "sys/syspest.h"
#include "sys/specnode.h"
#include "sys/lock_def.h"
#include "sys/fs_locks.h"

void spec_badop(void);
extern ulong ocr_event;		/* open/close/revoke event list */

/*
 * NAME:	spec_lookup(dvp, vpp, pname, vattrp, crp)
 *
 * FUNCTION:	Do a "lookup" on a device special file.  This only
 *		makes sense for multiplexed character special files.
 *		In this case, the channel of a multiplexed special file
 *		will be allocated given the name of the channel
 *		and the base of the multiplexed special file.
 *
 * PARAMETERS:	dvp	- spec vnode to lookup name for
 *		vpp	- returned vnode
 *		pname	- name to lookup
 *		vattrp	- attributes to be returned
 *		crp	- credentials pointer
 *
 *
 * RETURN:	returns from subroutines
 */

int
spec_lookup(
	struct vnode *	dvp,		/* spec vnode for lookup */
	struct vnode **	vpp,		/* spec vnode to return	*/
	caddr_t		pname,		/* name to lookup	*/
        int		flag,           /* unused               */
	struct vattr *	vattrp,		/* attrributes to be returned */
	struct ucred *	crp)		/* credentials          */
{
	int		rc = 0;		/* return code		*/

	switch (VTOGP(dvp)->gn_type)
	{
		case VBLK:
		case VCHR:
			spec_badop();
			/* does not return */

		case VMPC:
			rc = mpx_lookup(dvp, vpp, pname, crp);
			break;

		default:
			panic("unknown device type");
	}

	if (vattrp)
		vattrp->va_flags = VA_NOTAVAIL;

	return rc;
}

/*
 * NAME:	spec_link(vp, dvp, name)
 *
 * FUNCTION:	This function creates a link with the specified name
 *		in the specified directory to the specified device
 *		special file.  It is invalid to create a link to a
 *		channel of a multiplexed device.
 *
 * PARAMETERS:	vp	- vnode of special file to link to
 *		dvp	- vnode of directory to place link in
 *		name	- name to use for link in directory
 *
 * RETURN:	returns from subroutines
 */

int
spec_link(
	struct vnode *	vp,		/* source spec vnode	*/
	struct vnode *	dvp,		/* name's dir vnode	*/
	char         *	name,		/* name for link	*/
	struct ucred *	crp)		/* credentials          */
{
	int		rc;		/* return code		*/
	struct gnode *	sgp;		/* gnode of spec vnode	*/

	sgp = VTOGP(vp);

	switch (sgp->gn_type)
	{
		case VMPC:
			/* It is invalid to try to make a link
			 * to an mpx device channel.
			 */
			if (sgp->gn_chan != BASE_MPX)
			{
				rc = EINVAL;
				break;
			}

			/* fall through */

		case VBLK:
		case VCHR:
			if (!vp->v_pfsvnode)
				return EINVAL;

			/* let the PFS handle the link */
			rc = VNOP_LINK(vp->v_pfsvnode, dvp, name, crp);
			break;

		default:
			panic("unknown device type");
	}

	return rc;
}

/*
 * NAME:	spec_getattr(vp, vattrp)
 *
 * FUNCTION:	This function returns the attributes of a device
 *		special file in a filesystem independent format.
 *
 * PARAMETERS:	vp 	- vnode of special file to get attributes for
 *		vattrp	- return location for attributes
 *
 * RETURN :	non-zero on error from PFS getattr vnode operation
 */

int
spec_getattr(
	struct vnode *	vp,		/* spec vnode to stat		*/
	struct vattr *	vattrp,		/* returned attributes		*/
	struct ucred *	crp)		/* credentials          	*/
{
	struct gnode *	sgp;		/* spec gnode of spec vnode	*/
	struct specnode *snp;		/* specnode of device		*/
	int		rc = 0;		/* return code			*/

	/* Get the attributes from the PFS.  For channels of an mpx
	 * device, the attributes will mostly be those of the base.
	 */
	if (vp->v_pfsvnode)
	{
		if (rc = VNOP_GETATTR(vp->v_pfsvnode, vattrp, crp))
			return rc;
	}
	else
		bzero(vattrp, sizeof *vattrp);	/* XXX */

	sgp = VTOGP(vp);

	/* Most mpx channel attributes are kept in memory.  The
	 * attributes of a channel are those of the base mpx
	 * overlayed with the in-memory attributes of the channel.
	 */
	if (MPX_CHANNEL(sgp) || !vp->v_pfsvnode)
	{
		snp = SGTOSP(sgp);

		/* 
		 * Lock the specnode while assigning values to the
		 * attribute structure. The data may be stale once
		 * we give up the lock, but it will be consistently
		 * stale.
		 */
		SPECNODE_LOCK(snp);
		vattrp->va_type = sgp->gn_type;
		vattrp->va_rdev = sgp->gn_rdev;
		vattrp->va_chan = sgp->gn_chan;
		vattrp->va_size = sgp->gn_chan;		/* XXX */
		vattrp->va_mode = snp->sn_mode;
		vattrp->va_uid = snp->sn_uid;
		vattrp->va_gid = snp->sn_gid;
		vattrp->va_atime = snp->sn_atime;
		vattrp->va_mtime = snp->sn_mtime;
		vattrp->va_ctime = snp->sn_ctime;
		vattrp->va_nlink = 1;		/* always for channels */
		vattrp->va_gen = snp->sn_gen;
		SPECNODE_UNLOCK(snp);
	}

	if (MPX_CHANNEL(sgp)) {
		/* Temporary hack for those expecting sticky bit of mpx. */
		vattrp->va_mode |= S_ISVTX;
	}

	/* The multiplexed nature of a device is an in-memory attribute. */
	else if (MPX_BASE(sgp))
		vattrp->va_type = sgp->gn_type;

	return rc;
}

/*
 * NAME:	spec_remove(vp, dvp, pname)
 *
 * FUNCTION:	This function removes the specified device special file
 *		with the specified name in the specified directory.
 *
 * PARAMETERS:	vp 	- vnode that represents the special file to remove
 *		dvp	- parent directory to remove special file from
 *		pname 	- name of special file in directory
 *
 * RETURN :	errors from subroutines
 */

int
spec_remove(
	struct vnode *	vp,		/* spec vnode to remove		*/
	struct vnode *	dvp,		/* dir vnode of spec vnode	*/
	char 	     *	pname,		/* name of spec vnode to remove	*/
	struct ucred *	crp)		/* credentials          	*/
{
	int		rc = 0;		/* return code			*/
	struct gnode *	sgp;		/* gnode of spec vnode		*/

	sgp = VTOGP(vp);

	switch (sgp->gn_type)
	{
		case VMPC:
			/* It is invalid to try to remove mpx device
			 * channels.
			 */
			if (sgp->gn_chan != BASE_MPX)
			{
				rc = EINVAL;
				break;
			}

			/* fall through */

		case VBLK:
		case VCHR:
			if (!vp->v_pfsvnode)
				return EINVAL;

			/* let the PFS handle the remove */
			rc = VNOP_REMOVE(vp->v_pfsvnode, dvp, pname, crp);
			break;

		default:
			panic("unknown device type");
	}

	return rc;
}

/*
 * NAME:	spec_open(vp, flag, ext, vinfop)
 *
 * FUNCTION:	This function opens the device referenced by the
 *		specified device special file.
 *
 * PARAMETERS:	vp	- vnode of special file to open device for
 *		flag	- file open flags
 *		ext	- extended open information
 *		vinfop	- vfs specific information
 *
 * RETURN :	ENXIO	- device number invalid
 *		EBUSY	- if current open is for mount and there are writers
 *		EINTR	- interupted in device open routine
 */

int
spec_open(
	struct vnode *	vp,		/* spec vnode to open		*/
	int		flag,		/* file open flags		*/
	int		ext,		/* extended open info		*/
	caddr_t      *	vinfop,		/* vfs specific info, not used	*/
	struct ucred *	crp)		/* credentials          	*/
{
	int		rc = 0;		/* return code			*/
	dev_t		dev;		/* real device number		*/
	struct gnode *	dgp;		/* gnode for spec vnode		*/
	struct specnode *snp;		/* specnode for device to open	*/

	snp = VTOSP(vp);

	if (flag & FEXEC)
		return EACCES;

	SPECNODE_LOCK(snp);

	/* check the open permissions */
	if (rc = specaccess(vp, flag, crp))
	{
		SPECNODE_UNLOCK(snp);
		return rc;
	}

	SPECNODE_UNLOCK(snp);

	dgp = VTODGP(vp);

	switch (vp->v_type)
	{
		case VBLK:
			rc = bdev_open(dgp, flag, ext);
			break;

		case VCHR:
			rc = cdev_open(dgp, flag, ext);
			break;

		case VMPC:
			rc = mpx_open(dgp, flag, ext);
			break;

		default:
			panic("unknown device type");
	}

	return rc;
}

/*
 * NAME:	spec_close(vp, flag, vinfo)
 *
 * FUNCTION:	This function closes the device associated with the
 *		specified device special file.
 *
 * PARAMETERS:	vp	- spec vnode to close
 *		flag	- open flags
 *		vinfop	- vfs specific information
 *
 * RETURN :	returns from subroutines
 *
 */

spec_close (
	struct vnode *	vp,		/* spec vnode to close	*/
	int		flag,		/* file open flags	*/
	caddr_t		vinfo,		/* vfs specific info	*/
	struct ucred *	crp)		/* credentials          */
{
	int		rc = 0;		/* return code		*/
	dev_t		dev;		/* device number	*/
	struct gnode *	dgp;		/* dev gnode of vnode	*/
	struct devnode *dp;		/* devnode of vnode	*/

	dgp = VTODGP(vp);
	dp = VTODP(vp);
	dev = dgp->gn_rdev;

	/* flush block devices' block from the buffer cache */
	if (vp->v_type == VBLK)
		bflush(dev);

	/* 
	 * We should hold the devnode lock across the call to devcclose
	 */
	DEVNODE_LOCK(dp);
	rc = devcclose(dgp, flag);
	DEVNODE_UNLOCK(dp);

	/* invalidate block devices' blocks in the buffer cache */
	if (vp->v_type == VBLK)
		binval(dev);

	return  rc;
}

/*
 * NAME:	spec_rdwr(vp, rwmode, flags, uiop, ext, vinfo, vattrp, crp)
 *
 * FUNCTION:	This function reads from or writes to the device
 *		associated with the specified device special file.
 *
 * PARAMETERS:	vp	- spec vnode of device to read or write
 *		rwmode	- read or write type (UIO_READ or UIO_WRITE)
 *		flag	- file open flags
 *		uiop	- read or write location information
 *		ext	- extended open information
 *		vinfo	- vfs specific information
 *              vattrp  - attributes to return
 *              crp     - credentials
 *
 * RETURN :	returns from subroutines
 *		returns from bio interface
 *		EINVAL	- negative r/w offset
 */

spec_rdwr (
	struct vnode *	vp,		/* vnode to read or write	*/
	enum uio_rw 	mode,		/* UIO_READ or UIO_WRITE	*/
	int 		flags,		/* file open flags		*/
	struct uio *	uiop,		/* read or write location info	*/
	int 		ext,		/* extended open flag		*/
	caddr_t 	vinfo,		/* vfs specific info		*/
	struct vattr	*vattrp,	/* attributes to be returnes	*/
	struct ucred *	crp)		/* credentials         		*/
{
	struct vnode *	pvp;		/* underlying PFS vnode		*/
	struct gnode *	sgp;		/* device gnode			*/
	int		rc;		/* return code			*/

	sgp = VTOGP(vp);
	pvp = vp->v_pfsvnode;

	switch (sgp->gn_type)
	{
		case VBLK:
			rc = bdev_rdwr(sgp, pvp, mode, flags, uiop, ext, crp);
			break;

		case VCHR:
			rc = cdev_rdwr(sgp, pvp, mode, flags, uiop, ext, crp);
			break;

		case VMPC:
			rc = mpx_rdwr(sgp, pvp, mode, flags, uiop, ext, crp);
			break;

		default:
			panic("unknown device type");
	}
	
	if (vattrp)
		vattrp->va_flags = VA_NOTAVAIL;

	return rc;
}

/*
 * NAME:	spec_setattr(vp, cmd, arg1, arg2, arg3, crp)
 *
 * FUNCTION:	This function updates the owner, group, or times
 *		for the file based upon the specified command.
 *
 * PARAMETERS:	vp	- spec vnode to set attributes of
 *		cmd	- command indicating what portion of the attributes
 *				we wish to set and how the arguments
 *				should be interpretted
 *		arg1	-  various arguments whose meaning is determined
 *		arg2		by the specified command
 *		arg3
 *              crp     - credentials pointer
 *
 * RETURNS:	error returns from the PFS setattr vnode op
 */

spec_setattr (
	struct vnode *	vp,		/* spec vnode for PFS file	*/
	int		cmd,		/* command for attr's to set	*/
	int		arg1,		/* arguments for PFS vn_setattr	*/
	int		arg2,
	int		arg3,
	struct ucred *	crp)		/* credentials         		*/
{
	struct specnode	*snp;		/* specnode for spec vnode	*/
	struct gnode	*sgp;		/* spec gnode for spec vnode	*/
	int		rc = 0;		/* return code			*/

	snp = VTOSP(vp);
	sgp = STOSGP(snp);

	SPECNODE_LOCK(snp);

	/* The attributes for a multiplexed channel are changed in
	 * memory only.
	 */
	if (MPX_CHANNEL(sgp) || !vp->v_pfsvnode)
	{
		switch (cmd)
		{
			case V_MODE:
				rc = dochmod(snp->sn_uid,
					     snp->sn_gid,
					     arg1,		/* new mode */
					     &snp->sn_mode,
					     crp);
				break;
			case V_OWN:
				rc = dochown(arg1,		/* flags   */
					     arg2,		/* new uid */
					     arg3,		/* new gid */
					     &snp->sn_uid,
					     &snp->sn_gid,
					     &snp->sn_mode,
					     crp);
				break;
			case V_UTIME:
				/* do T_SETTIME checking here */
				if (arg1 & T_SETTIME &&
					     (crp->cr_uid != snp->sn_uid) &&
					     privcheck_cr(SET_OBJ_STAT, crp) &&
					     specaccess(vp, FWRITE, crp))
				{
					rc = EACCES;
					break;
				}

				rc = doutime(snp->sn_uid,
					     arg1,	/* flags	*/
					     arg2,	/* new atime	*/
					     arg3,	/* new mtime	*/
					     &snp->sn_atime,
					     &snp->sn_mtime,
					     crp);
				break;
			default:
				rc = EINVAL;
				break;
		}
		goto done;
	}

	/* let the PFS handle the setattr */
	rc = VNOP_SETATTR(vp->v_pfsvnode, cmd, arg1, arg2, arg3, crp);
done:
	SPECNODE_UNLOCK(snp);
	return  rc;
}

spec_strategy (
	struct vnode *	vp,	/* spec vnode to call strategy for	*/
	struct buf *	bp,	/* buf struct to call strategy on	*/
	struct ucred *	crp)	/* credentials   	      		*/
{
	int		rc;	/* return code				*/

	if (VTOGP(vp)->gn_type == VBLK)
		rc = devstrat(bp);
	else
		rc = EINVAL;

	return rc;
}

/*
 * NAME:	spec_select(vp, corl, e, re, notify, vinfo)
 *
 * FUNCTION:	This function does a select operation on the device
 *		associated with the specified device special file.
 *
 * PARAMETERS:	vp	- vnode to perform select upon
 *		corl	- select correlator
 *		event	- select event
 *		reventp	- event list
 *		notify  - function pointer of notify routine for nested polls
 *		vinfo	- vfs specific information (not used)
 *
 * RETURN :	subroutine returns
 */

spec_select (
	struct vnode *	vp,		/* spec vnode to select upon	*/
	int		corl,		/* select correlator		*/
	ushort		event,		/* select event			*/
	ushort *	reventp,	/* event list			*/		
	void		(* notify)(),	/* notify func for nested polls	*/
	caddr_t		vinfo,		/* vfs specific info		*/
	struct ucred *	crp)		/* credentials         		*/
{
	struct gnode *	sgp;		/* spec gnode for device	*/
	int		rc = 0;		/* return code			*/

	sgp = VTOGP(vp);

	switch (sgp->gn_type)
	{
		case VCHR:
		case VMPC:
			rc = rdevselect(sgp->gn_rdev,
					corl,
					event,
					reventp,
					notify,
					sgp->gn_chan);
			break;

		case VBLK:
			/* return 0 by default */
			rc = 0;
			break;

		default:
			panic("unknown device type");
	}

	return rc;
}

/*
 * NAME:	spec_lockctl(vp, offset, lockdat, cmd, retry_fcn, retry_id)
 *
 * FUNCTION:	This function gets, sets, and tests locks on the specified
 *		device special file.
 *
 * PARAMETERS:	vp		- spec vnode
 *		offset		- offset within file
 *		lckdat		- data describing lock
 *		cmd		- locking command
 *		retry_fcn	- locking retry function
 *		retry_id	- ptr to arg for retry function
 *
 * RETURNS:	errors from subroutines
 */

spec_lockctl(
	struct vnode *	vp,		/* spec vnode to lock		*/
	offset_t	offset,		/* offset within PFS file	*/
	struct eflock *	lckdat,		/* data describing lock		*/
	int		cmd,		/* locking command		*/
	int		(* retry_fcn)(), /* locking retry function	*/
	ulong        *	retry_id,	/* ptr to arg for retry func	*/
	struct ucred *	crp)		/* credentials         		*/
{
	/* Do all locking locally. */
	return common_reclock(VTOGP(vp),
			      0,	/* size */
			      offset,
			      lckdat,
			      cmd,
			      retry_fcn,
			      retry_id,
			      NULL,
			      NULL);
}

/*
 * NAME:	spec_ioctl(vp, cmd, arg, flag, ext)
 *
 * FUNCTION:	This function calls the ioctl entry point of the device
 *		associated with the specified device special file.
 *
 * PARAMETERS:	vp	- spec vnode of device to send ioctl to
 *		cmd	- ioctl command
 *		arg	- argument for ioctl command
 *		flag	- file open flags
 *		ext	- extended ioctl information
 *
 * RETURNS:	errors from subroutines
 */

spec_ioctl (
	struct vnode *	vp,		/* spec vnode to send ioctl to	*/
	int		cmd,		/* user specified ioctl command	*/
	int		arg,		/* argument for ioctl command	*/
	long		flag,		/* file open flags		*/
	caddr_t		ext,		/* extended ioctl information	*/
	struct ucred *	crp)		/* credentials         		*/
{
	struct gnode *sgp;		/* gnode for spec vnode		*/
	label_t       jumpbuf;		/* state buffer			*/
	int           rv;		/* return value			*/

	sgp = VTOGP(vp);
	rv = rdevioctl(sgp->gn_rdev, cmd, arg, flag, sgp->gn_chan, ext);
	return(rv);
}

/*
 * NAME:	spec_hold(vp)
 *
 * FUNCTION:	This function increments the hold count on a vnode.
 *
 * PARAMETERS:	vp	- vnode to increment the hold count of
 *
 * RETURNS:	errors from the PFS hold vnode operation
 */
spec_hold (
	struct vnode *	vp)		/* vnode to increment count for	*/
{
	struct specnode *snp;		/* specnode for this vnode */
	int		rc = 0;		/* return code for this routine */
	int		snlocked;       /* specnode locked on entry */

	snp = VTOSP(vp);
	if (!(snlocked = lock_mine(&snp->sn_lock)))
		SPECNODE_LOCK(snp);
	vp->v_count++;

	/* hold the PFS vnode to keep the counts the same. */
	if (vp->v_pfsvnode)
		rc = VNOP_HOLD(vp->v_pfsvnode);

	if (!snlocked)
		SPECNODE_UNLOCK(snp);
	return rc;
}

/* 
 * NAME:	spec_rele(vp)
 *
 * FUNCTION:	Decrement the count on a spec vnode and the underlying
 *		PFS vnode.  If it was the last reference to the vnode,
 *		release the memory associated with the vnode and put
 *		the specnode.  If this is the last reference to an
 *		mpx channel, the channel will be deallocated in snput().
 *		Mpx channels are allocated in mpx_lookup().
 *
 * PARAMETERS:	vp	- vnode to be released
 *
 * RETURNS:	Returns any errors from the PFS VNOP_RELE().
 */
spec_rele(
	struct vnode *	vp)	/* spec vnode to release	*/
{
	struct specnode *snp;	/* specnode of vnode		*/
	struct vnode *	pvp;	/* PFS vnode for spec vnode	*/
	int		vtype;	/* vnode type			*/
	int		count;	/* hold count of the vnode 	*/
	int 		rc=0;	/* return code from routine     */
	int		snlocked;   /* specnode locked on entry */

	snp = VTOSP(vp);
	if (!(snlocked = lock_mine(&snp->sn_lock)))
		SPECNODE_LOCK(snp);

	/* hang onto the PFS vnode in case we free the vnode */
	pvp = vp->v_pfsvnode;

	/* The counts on the device vnode should all be matched on the
	 * the PFS vnode.
	 */
	assert(!pvp || pvp->v_count >= vp->v_count);

	count = --vp->v_count;

	/* When the vnode count hits zero, free the vnode and put the
	 * specnode.
	 */
	if (count == 0)
	{
		vn_free(vp);
		/* This put will deallocate an mpx channel if we are
		 * the only holder of the specnode.
		 */
		snput(snp);
	}
	else
		if (!snlocked)
			SPECNODE_UNLOCK(snp);

	/* Keep the vnode counts consistent by releasing the
	 * PFS vnode.
	 */
	if (pvp)
		rc = VNOP_RELE(pvp);
	
	return rc;
}

/*
 * NAME:	spec_fid(vp, fidp)
 *
 * FUNCTION: 	This function modifies the file identifier pointed to by
 *		the fhp.  The physical file system does all the work.
 *
 * PARAMETERS:	vp	- spec vnode for device special file to get fid for
 *		fidp	- file handle to fill in
 *
 * RETURN:	Always zero
 *			
 */
int
spec_fid (
	struct vnode *	vp,		/* spec vnode to get fid for	*/
	struct fileid *	fidp,		/* file handle to fill in	*/
	struct ucred *	crp)		/* credentials         		*/
{
	/* We cannot create a file handle for an mpx channel as there
	 * is no underlying file system object.
	 */
	if (MPX_CHANNEL(VTOGP(vp)) || !vp->v_pfsvnode)
		return EINVAL;

	/* let the PFS get the file id */
	return VNOP_FID(vp->v_pfsvnode, fidp, crp);
}

/*
 * NAME:	spec_access(vp, mode, who, crp)
 *
 * FUNCTION:	This function checks for access permission to the specified
 *		device special file.
 *
 * PARAMETERS:	vp	- spec vnode to check access to
 *		mode	- access mode to check (W_ACC, R_ACC, and/or X_ACC)
 *		who	- whom to check access for, one of:
 *					ACC_SELF 
 *				 	ACC_OTHERS 
 *				 	ACC_ANY 
 *				 	ACC_ALL
 *
 * RETURNS:	0      - success
 *		EINVAL - invalid mode or who argument
 */

spec_access (
	struct vnode *	vp,		/* spec vnode to check access	*/
	int		mode,		/* access mode to check		*/
	int		who,		/* whom to check access for	*/
	struct ucred *	crp)		/* credentials         		*/
{

	struct specnode *snp;
	int rc = 0;

	snp = VTOSP(vp);

	/* The access information for mpx channels is in memory */
	if (MPX_CHANNEL(VTOGP(vp)) || !vp->v_pfsvnode)
	{
		SPECNODE_LOCK(snp);
		rc = ic_specaccess(snp, mode, who, crp);
		SPECNODE_UNLOCK(snp);
		return rc;
	}

	/* let the PFS check for access */
	return VNOP_ACCESS(vp->v_pfsvnode, mode, who, crp);
}

/*
 * NAME:	spec_revoke(vp, cmd, flag, vinfop)
 *
 * FUNCTION:	This function revokes access to the specified file.
 * 		This is performed in two steps (there are two forms 
 *		of the call).  The first form of the call checks that
 *		access revocation is supported on this object
 *		(it would be better to have this as a flag in the
 *		gnode, established by getattr).
 *		The second form of the call actually does the
 *		revocation.
 *
 * PARAMETERS:	vp	- spec vnode of device to revoke access to
 *		cmd	- command indicating form of call:
 *			0	- revoke() system call
 *			1	- frevoke() system call
 *		flag	- future enhancements flag (not used)
 *		vinfop	- file information (not used)
 *
 * RESULTS:	EINVAL	- device does not support revoke
 *		errors returned from device revoke entry point
 */

spec_revoke(
	struct vnode *	vp,		/* spec vnode of dev to revoke	*/
	int		cmd,		/* command for form of call	*/
	int		flag,		/* future use flag (not used)	*/
	caddr_t		vinfop,		/* file information (not used)	*/
	struct ucred *	crp)		/* credentials         		*/
{
	struct devnode * dp;		/* devnode of device		*/
	struct gnode *	 sgp;		/* spec gnode of device		*/
	dev_t		 dev;		/* device number of device	*/

	sgp = VTOGP(vp);
	dev = sgp->gn_rdev;

	/* check to see if device supports revoke */
	if (devsw[major(dev)].d_revoke == 0)
		return EINVAL;

	/* make sure close isn't happening and return if it is
	 */
	dp  = VTODP(vp);
	DEVNODE_LOCK(dp);
	if (dp->dv_mntcnt + dp->dv_rdcnt + dp->dv_wrcnt == 0)
	{
		DEVNODE_UNLOCK(dp);
		return 0;
	}
	dp->dv_flag |= DV_REVOKING;
	DEVNODE_UNLOCK(dp);

	/* call the device's revoke entry point */
	(*devsw[major(dev)].d_revoke)(dev, sgp->gn_chan, cmd);

	/*
	 * hack similar to openi() ... need to set ttyd where know <dev>.
	 *
	 * if the calling process is to become a process group leader,
	 * ttrevoke will set:
	 * 1)	u.u_ttyp to point to the terminal
	 * 2)	u.u_ttyd to 0 (NODEVICE might be a better choice)
	 * this assumes that the device (0,0) is not a terminal
	 */
	if (cmd && (u.u_ttyp) && (u.u_ttyd == NODEVICE))
	{
		u.u_ttyd = dev;
		u.u_ttympx = sgp->gn_chan;
	}

	/* wake up any pending closes
	 */
	DEVNODE_LOCK(dp);
	dp->dv_flag &= ~DV_REVOKING;
	if (dp->dv_flag & DV_CLOSEPENDING)
	{
		dp->dv_flag &= ~DV_CLOSEPENDING;
		e_wakeup(&ocr_event);
	}
	DEVNODE_UNLOCK(dp);

	return 0;
}

/*
 * NAME:	spec_getacl(vp, uiop)
 *
 * FUNCTION:	This function returns the access control list for
 *		the specified device special file.
 *
 * PARAMETERS:	vp 	- vnode for device special file to get ACL for
 *		uiop	- uio structure indicating where to put the ACL
 *
 * RETURN:	return value from PFS getacl vnode operation
 */

int
spec_getacl(
	struct vnode *	vp,
	struct uio *	uiop,
	struct ucred *	crp)
{
	struct specnode *	snp;		/* specnode of device	*/
	int rc = 0;

	/* let the PFS get the access control list */
	if (!MPX_CHANNEL(VTOGP(vp)) && vp->v_pfsvnode)
	{
		/* The getacl vnode op may be NULL if the PFS does not support ACLs.
		 * Returning ENOSYS lets the LFS know to handle things.
		 */
		if (vp->v_pfsvnode->v_gnode->gn_ops->vn_getacl == NULL)
			return ENOSYS;

		return VNOP_GETACL(vp->v_pfsvnode, uiop, crp);
	}

	/* The acl's for mpx channels are in memory */
	snp = VTOSP(vp);
	SPECNODE_LOCK(snp);
	rc = ic_getacl(snp->sn_acl, snp->sn_mode, uiop);
	SPECNODE_UNLOCK(snp);
	return rc;
}

/*
 * NAME:	spec_setacl(vp, uiop)
 *
 * FUNCTION:	This function stores the access control list for
 *		the specified device special file.
 *
 * PARAMETERS:	vp 	- vnode for device special file to set ACL for
 *		uiop	- uio struct indicating where to get the ACL from
 *
 * RETURN:	return value from PFS setacl vnode operation
 */

int
spec_setacl(
	struct vnode *	vp,
	struct uio *	uiop,
	struct ucred *	crp)
{
	struct specnode *snp;
	int rc = 0;

	/* let the PFS set the access control list */
	if (!MPX_CHANNEL(VTOGP(vp)) && vp->v_pfsvnode)
	{
		/* The setacl vnode op may be NULL if the PFS does not support ACLs.
		 * Returning ENOSYS lets the LFS know to handle things.
		 */
		if (vp->v_pfsvnode->v_gnode->gn_ops->vn_setacl == NULL)
			return ENOSYS;

		return VNOP_SETACL(vp->v_pfsvnode, uiop, crp);
	}

	/* The acl's for mpx channels only affect the mode. */
	
	snp = VTOSP(vp);
	if ((crp->cr_uid != snp->sn_uid) && privcheck_cr(SET_OBJ_DAC, crp))
		return EPERM;
	else
	{
		SPECNODE_LOCK(snp);
		rc = ic_setacl(&snp->sn_acl, &snp->sn_mode, snp->sn_gid,
				uiop, crp);
		SPECNODE_UNLOCK(snp);
		return rc;
	}
}

/*
 * NAME:	spec_getpcl(vp, uiop)
 *
 * FUNCTION:	This function returns the privelege control list for
 *		the specified device special file.
 *
 * PARAMETERS:	vp 	- vnode for device special file to return PCL for
 *		uiop	- uio structure in which to put the PCL
 *
 * RETURN:	return value from PFS getpcl vnode operation
 */

int
spec_getpcl(
	struct vnode *	vp,
	struct uio *	uiop,
	struct ucred *	crp)
{
	struct specnode *	snp;		/* specnode of device	*/
	struct pcl		pcl;		/* priv control list	*/
	int			pcllen;		/* length of pcl	*/
	int			rc;		/* return code		*/

	/* The getpcl vnode op may be NULL if the PFS does not support PCLs.
	 * Returning ENOSYS lets the LFS know to handle things.
	 */
	if (vp->v_pfsvnode->v_gnode->gn_ops->vn_getpcl == NULL)
		return ENOSYS;

	/* let the PFS get the privilege control list */
	if (!MPX_CHANNEL(VTOGP(vp)) && vp->v_pfsvnode)
		return VNOP_GETPCL(vp->v_pfsvnode, uiop, crp);

	/* The pcl's for mpx channels are in memory */

	snp = VTOSP(vp);

	/* set up size of the PCL structure to move */
	pcllen = PCL_SIZ;

	/* If the user's recieve buffer is smaller than the PCL, try	*/
	/* to inform the user of the necessary size and return ENOSPC.	*/
	if (uiosiz(uiop) < pcllen)
	{
		uiop->uio_offset = 0;
		uiop->uio_iov->iov_len = uiop->uio_resid = sizeof pcllen;
		if (rc = uiomove((uchar *)&pcllen, sizeof pcllen, UIO_READ,
				 uiop))
			return rc;
		else
			return ENOSPC;
	}

	/* fill in fields in privilege control list */
	pcl.pcl_len = PCL_SIZ;
	pcl.pcl_mode = snp->sn_mode;
	bzero(&pcl.pcl_default, sizeof pcl.pcl_default);	/* default pcl	*/

	/* copy privilege control list to caller's buffer */
	return uiomove((uchar *)&pcl, pcl.pcl_len, UIO_READ, uiop);
}

/*
 * NAME:	spec_setpcl(vp, uiop)
 *
 * FUNCTION:	This function stores the privilege control list for
 *		the specified device special file.
 *
 * PARAMETERS:	vp 	- vnode for device special file to set PCL for
 *		uiop	- uio struct indicating where to get the PCL from
 *
 * RETURN:	return value from PFS setpcl vnode operation
 */

int
spec_setpcl(
	struct vnode *	vp,
	struct uio *	uiop,
	struct ucred *	crp)
{
	struct pcl *	pcl;
	int		len;

	/* The setpcl vnode op may be NULL if the PFS does not support PCLs.
	 * Returning ENOSYS lets the LFS know to handle things.
	 */
	if (vp->v_pfsvnode->v_gnode->gn_ops->vn_setpcl == NULL)
		return ENOSYS;

	/* let the PFS set the privilege control list */
	if (!MPX_CHANNEL(VTOGP(vp)) && vp->v_pfsvnode)
		return VNOP_SETPCL(vp->v_pfsvnode, uiop, crp);

	/* The pcl's for mpx channels are ignored silently. */

	return 0;
}

void
spec_badop(void)
{
	panic("spec_badop");
}

int
spec_einval(void)
{
	return EINVAL;
}

int
spec_noop(void)
{
	return 0;
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
