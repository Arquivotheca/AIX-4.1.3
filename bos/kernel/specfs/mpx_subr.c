static char sccsid[] = "@(#)01	1.30.1.5  src/bos/kernel/specfs/mpx_subr.c, sysspecfs, bos411, 9428A410j 4/19/94 16:16:46";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: mpx_lookup, mpx_close, mpx_open, mpx_rdwr
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/user.h"
#include "sys/vfs.h"
#include "sys/uio.h"
#include "sys/buf.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/time.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/device.h"
#include "sys/dev.h"
#include "sys/malloc.h"
#include "sys/specnode.h"
#include "sys/fs_locks.h"

/* Operations on mpx channels should never call any vnode operations (other than
 * hold and release) on the PFS vnode as this vnode is that of the base of the
 * mpx device.
 */

/*
 * NAME:	mpx_lookup(dvp, vpp, cname, crp)
 *
 * FUNCTION:	This function performs an action that resembles "lookup"
 *		in a directory, but is actually a channel allocation based
 *		upon a multiplexed character special file.  The channel
 *		allocated has no representation in the underlying physical
 *		file system.  The channel is allocated by calling the
 *		mpx entry point of the device driver indicated by the
 *		major number of the mpx file.  The device driver returns
 *		a channel number based upon the channel name looked up.
 *		The channel allocated here is deallocated when the last
 *		vnode referencing the mpx file is released.
 *		The channel will only be left allocated if no error is
 *		discovered.
 *
 * PARAMETERS:	dvp	- spec vnode to lookup channel for
 *		vpp	- returned vnode
 *		cname	- channel name
 *
 * RETURN :	returns from subroutines
 */

mpx_lookup(
	struct vnode *	dvp,		/* vnode for base mpx		*/
	struct vnode **	vpp,		/* vnode for channel mpx return	*/
	caddr_t		cname,		/* channel name to lookup	*/
	struct ucred *  crp)		/* credentials			*/
{
	struct specnode *snp;		/* specnode for mpx channel	*/
	dev_t		rdev;		/* device number		*/
	chan_t		chan;		/* channel number		*/
	uint		devstat;	/* result of devswqry()		*/
	struct vnode *	vp;		/* vnode to return		*/
	struct vattr	pfsvattr;	/* attributes of base mpx	*/
	int		rc;		/* return code			*/

	*vpp = NULL;

	rdev = VTOGP(dvp)->gn_rdev;

	/* validate rdev before calling d_mpx */
	if (rc = devswqry(rdev, &devstat, NULL))
		return rc;
	if (devstat == DSW_UNDEFINED || !(devstat & DSW_MPX))
		return ENXIO;

	/* Call the driver to allocate the channel.  The channel is
	 * deallocated when the last reference to the channel is released.
	 */
	DD_ENT(rc = ,
	       (*devsw[major(rdev)].d_mpx)(rdev, &chan, (cname)? cname : ""),
	       IPRI_NOPREMPT,major(rdev));
	if (rc)
		return rc;

	/* dev_vp will return a specfs vnode for the channel given the PFS vnode
	 * of the base and a channel number.
	 */
	vp = dvp->v_pfsvnode;
	if (rc = dev_vp(&vp, VMPC, chan))
	{
		/* Call the device driver to deallocate the channel. */
		DD_ENT((void),
		       (*devsw[major(rdev)].d_mpx)(rdev, &chan, NULL),
		       IPRI_NOPREMPT,major(rdev));
		return rc;
	}

	/* Hold the PFS vnode for each channel spec vnode referencing it. */
	VNOP_HOLD(vp->v_pfsvnode);

	snp = VTOSP(vp);

	 /* If this a new channel, we need to initialize the specnode
	  * in-core attributes.
	  */
	if (snp->sn_attr == NULL)
	{
		/* The attributes for the specnode should only be set up once.
		 * Hold everybody else here until the attributes are filled in.
		 * The getattr and getacl vnode ops can release the kernel lock.
		 */
		SPECNODE_LOCK(snp);

		/* Make sure some other process hasn't already allocated
		 * the in-core attributes while we slept.
		 */
		if (snp->sn_attr == NULL)
		{
			/* initialize in-memory attributes from base mpx */
			if ((rc = VNOP_GETATTR(dvp->v_pfsvnode, &pfsvattr, crp)) == 0)
			{
				/* allocate the in-core attributes structure */
				snp->sn_attr = (struct icattr *)
						malloc(sizeof *snp->sn_attr);
				if (snp->sn_attr == NULL)
					rc = ENOMEM;
				else
				{
					snp->sn_mode = pfsvattr.va_mode;
					snp->sn_uid = pfsvattr.va_uid;
					snp->sn_gid = pfsvattr.va_gid;

					snp->sn_atime = pfsvattr.va_atime;
					snp->sn_mtime = pfsvattr.va_mtime;
					snp->sn_ctime = pfsvattr.va_ctime;
					snp->sn_acl = NULL;

					/* copy any ACL from the base */
					get_pfs_acl(vp, crp);
				}
			}
		}
		SPECNODE_UNLOCK(snp);
	}

	if (rc)
		/* This will deallocate the channel appropriately. */
		VNOP_RELE(vp);
	else
		*vpp = vp;

	return rc;
}

/*
 * NAME:	mpx_open(dgp, flag, ext, vinfop)
 *
 * FUNCTION:	This function opens the multiplexed character device
 *		referenced by the specified multiplexed character device
 *		special file.  It establishes the controlling terminal,
 *		if appropriate.
 *
 * PARAMETERS:	dgp	- mpx device gnode to open
 *		flag	- open flags
 *		ext	- extended open info
 *
 * RETURN :	returns from device interface
 *		ENXIO	- device number invalid
 */

mpx_open (
	struct gnode *	dgp,		/* dev gnode to open	*/
	int		flag,		/* open mode		*/
	int		ext)		/* extended open info	*/
{
	int		rc = 0;		/* return code		*/
	int		noctl;		/* controlling tty?	*/
	dev_t		dev;		/* real device number	*/

	dev = dgp->gn_rdev;

	/* We pass only the allowed open flags and whether the caller
	 * was in kernel mode.
	 */
	flag &= (FMASK|FKERNEL);

	/* Hang onto the state of controlling terminal to see if
	 * one is established by the device open.
	 */
	noctl = (u.u_ttyp == NULL);

	/* register device open */
	rc = rdevopen(dev, flag, dgp->gn_chan, ext, &dgp);

	/* Save the controlling terminal device number and channel
	 * if the device open established the controlling terminal.
	 */
	if (rc == 0 && noctl && u.u_ttyp)
	{
		u.u_ttyd = dev;
		u.u_ttympx = dgp->gn_chan;
	}

	return rc;
}

/*
 * NAME:	mpx_rdwr (sgp, rwmode, flags, uiop, ext, crp)
 *
 * FUNCTION:	Read or write mpx device
 *
 * PARAMETERS:	sgp	- spec gnode to r or w
 *		rwmode	- r or w
 *		flag	- open flags
 *		uiop	- locality info
 *		ext	- extended open info
 *
 * RETURN :	returns from devsw[] interface
 *		EINVAL	- negative r/w offset
 */

/* ARGSUSED */
int
mpx_rdwr (
	struct gnode *	sgp,		/* spec gnode of device		*/
	struct vnode *	pvp,		/* PFS vnode of device		*/
	enum uio_rw 	rwmode,		/* UIO_READ or UIO_WRITE	*/
	int 		flags,		/* file open flags		*/
	struct uio *	uiop,		/* read or write location info	*/
	int 		ext,		/* extended open flag		*/
	struct ucred *  crp)		/* credentials			*/
{
	struct specnode *snp;		/* specnode of device		*/
	dev_t		dev;		/* device number of device	*/
	int		rc;		/* return code			*/
	struct timestruc_t t;

	snp = SGTOSP(sgp);
	dev = sgp->gn_rdev;

	curtime(&t);
	if (rwmode == UIO_READ)
	{
		if (pvp && sgp->gn_chan == BASE_MPX)
			(void)VNOP_SETATTR(pvp, V_STIME, &t, 0, 0, crp);
		else
			/* update access time to channel in memory */
			snp->sn_atime = t;

		/* read from device */
		rc = rdevread(dev, uiop, sgp->gn_chan, ext);
	}
	else	/* UIO_WRITE */
	{
		if (pvp && sgp->gn_chan == BASE_MPX)
			(void)VNOP_SETATTR(pvp, V_STIME, 0, &t, &t, crp);
		else
			/* update modification and change times of
			 * channel in memory
			 */
			snp->sn_mtime = snp->sn_ctime = t;

		/* write to device */
		rc = rdevwrite(dev, uiop, sgp->gn_chan, ext);
	}

	return rc;
}

int
get_pfs_acl(struct vnode *vp,
	    struct ucred *crp)
{
	struct specnode	*snp;
	struct acl	acl;
	struct acl	*aclp = &acl;
	struct iovec	iov;
	struct uio	uio;
	long		acllen = sizeof acl;
	int		rc;
			
	/* The getacl vnode op may be NULL if the PFS does not support ACLs */
	if (vp->v_pfsvnode->v_gnode->gn_ops->vn_getacl == NULL)
		return ENOSYS;

	/* Get the ACL from the PFS, assuming a small ACL. */

	iov.iov_base = (char *)aclp;
	iov.iov_len = acllen;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = acllen;
	uio.uio_segflg = UIO_SYSSPACE;
	

	rc = VNOP_GETACL(vp->v_pfsvnode, &uio, crp);

	if (rc == ENOSPC)
	{
		/* The ACL does not fit in a struct acl.  The necessary
		 * length is returned in the ACL.
		 */

		acllen = aclp->acl_len;
		
		/* enforce some limit on ACL size */
		if (acllen > PAGESIZE)
			return ENOMEM;		/* XXX */

		/* Create an ACL of sufficient size and have the PFS
		 * fill it in.
		 */
		  
		if ((aclp = (struct acl *)malloc(acllen)) == NULL)
			return ENOMEM;

		iov.iov_base = (char *)aclp;
		iov.iov_len = acllen;
		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = 0;
		uio.uio_resid = acllen;
		uio.uio_segflg = UIO_SYSSPACE;

		rc = VNOP_GETACL(vp->v_pfsvnode, &uio, crp);
		if (rc)
		{
			free(aclp);
			return rc;
		}
	}
	else if (rc)
		return rc;

	/* write the in-memory ACL */

	iov.iov_base = (char *)aclp;
	iov.iov_len = aclp->acl_len;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = aclp->acl_len;
	uio.uio_segflg = UIO_SYSSPACE;

	snp = VTOSP(vp);
	/* SPECNODE_LOCK is already held */
	rc = ic_setacl(&snp->sn_acl, &snp->sn_mode, snp->sn_gid, &uio, crp);
	return rc;
}
