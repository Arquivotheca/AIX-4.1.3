static char sccsid[] = "@(#)58	1.44  src/bos/kernel/specfs/devsubs.c, sysspecfs, bos41J, 9521A_all 5/23/95 08:02:22";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: devopen, devclose, devioctl,
 *            devmpx,  devread,  devwrite
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

#include "sys/user.h"
#include "sys/types.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/uio.h"
#include "sys/syspest.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/device.h"
#include "sys/lockl.h"
#include "sys/dev.h"
#include "sys/specnode.h"
#include "sys/fs_locks.h"
#include "sys/sleep.h"

/* Declarations */
static devcopen();

int ocr_event = EVENT_NULL;		/* open/close/revoke wait list */

/*
 * NAME:	fp_opendev(devno, flags, cname, ext, fpp)
 *
 * FUNCTION:	This function opens the specified device using the
 *		specified flags and returns a file struct pointer
 *		referencing the open device.
 *		This function examines character special devices to
 *		determine whether they are actually multiplexed devices.
 *
 * PARAMETERS:	devno	- device number of dev to open
 * 		flags	- device open flags (DREAD and/or DWRITE)
 *		cname	- channel name
 *		ext	- extension parameter
 *		fpp	- pointer for returned file struct pointer
 *
 * RETURN :	ENXIO	- Invalid device
 *		ENFILE	- file table overflow
 *		ENXIO	- bad channel allocation
 *			
 */

int
fp_opendev(
	dev_t		devno,		/* device number of dev to open	*/
	int		flags,		/* file open flags		*/
	caddr_t		cname,		/* channel name			*/
	int		ext,		/* extension parameter		*/
	struct file **	fpp)		/* file struct pointer return	*/
{
	int		rc;		/* return code			*/
	uint		status;		/* status of dev switch entry	*/
	chan_t		chan = 0;	/* channel number		*/
	struct gnode *	gp = NULL;	/* spec gnode			*/
	extern struct fileops gno_fops;	/* file ops for gnode type fps	*/
	int		waslocked;	/* kernel lock status		*/
	caddr_t		cnamex;		/* rationalized channel name	*/


	/* check file open flags */
	if ((flags & (DREAD|DWRITE)) == 0)
		return EINVAL;

	/* Lose the kernel lock to avoid hierarchy problems.  */
	if (waslocked = IS_LOCKED(&kernel_lock))
		unlockl(&kernel_lock);

	/* get and check the status of the dev switch entry */
 	if (devswqry(devno, &status, NULL) || status == DSW_UNDEFINED)
	{
		rc = ENXIO;
		goto out;
	}

	/* If this is an mpx device, allocate the mpx channel. */
	if (status & DSW_MPX)
	{
		cnamex = (cname)? cname : "";

		DD_ENT(rc = ,
		       (*devsw[major(devno)].d_mpx)(devno, &chan, cnamex),
		       IPRI_NOPREMPT,major(devno));
		if (rc)
		{
			rc = ENXIO;
			goto out;
		}
	}							

	/* Pass a NULL gnode to devcopen() because we don't have
	 * the spec gnode.
	 */
	gp = NULL;
	rc = devcopen(devno, DKERNEL|flags, chan, ext, &gp);
	if (rc == 0)
	{
		/* allocate the file struct pointer */
		if (rc = fpalloc(gp, flags, DTYPE_GNODE, &gno_fops, fpp))
			rdevclose(gp, flags);
	}

out:
	if (waslocked)
		lockl(&kernel_lock, LOCK_SHORT);
	return rc;
}
		

/*
 * NAME:	rdevopen(devno, flags, chan, ext, gpp)
 *
 * FUNCTION:	This function is a common device open routine, shared
 *		between the specfs and the rest of the kernel.  This
 *		routine serves two purposes.  First, it performs
 *		intra-kernel device opens. Second, it is used by the
 *		file system to "register" device opens processed through
 *		the file system.
 *
 * PARAMETERS:	devno	- device number of device to open
 * 		flags	- device open flags (DREAD, DWRITE, and/or DMOUNT)
 *		chan	- device channel number
 *		ext	- open extension parameter
 *		fpp	- pointer for returned file struct pointer
 *
 * RETURN :	ENXIO	- Invalid device
 *		ENFILE	- File table overflow
 *			
 */

int
rdevopen (
	dev_t		devno,		/* device number		*/
	int		flags,		/* DREAD, DWRITE, and/or DMOUNT	*/
	chan_t		chan,		/* device channel number	*/
	int		ext,		/* open extension parameter	*/
	struct gnode **	gpp)		/* spec gnode return		*/
{
	uint		status;		/* status of dev switch entry	*/
	int		waslocked;	/* kernel lock status		*/
	int		rc;

	/* Lose the kernel lock to avoid hierarchy problems.  */
	if (waslocked = IS_LOCKED(&kernel_lock))
		unlockl(&kernel_lock);

	/* Check for device number out of range or device entry
	 * being removed.
	 */
 	if (devswqry(devno, &status, NULL) || status == DSW_UNDEFINED)
		rc = ENXIO;
	else
		rc = devcopen(devno, flags, chan, ext, gpp);

	if (waslocked)
		lockl(&kernel_lock, LOCK_SHORT);
	return rc;
}

/*
 * NAME:	devcopen(devno, flags, chan, ext, gpp)
 *
 * FUNCTION:	This function is a common device open routine, shared
 *		between rdevopen() and fp_opendev().  It performs device
 *		opens for the kernel and for kernel extensions.
 *
 * PARAMETERS:	devno	- device number of dev to open
 * 		flags	- device open flags (DREAD, DWRITE, and/or DMOUNT)
 *		chan	- device channel number
 *		ext	- open extension parameter
 *		fpp	- pointer for returned file struct pointer
 *
 * RETURN :	ENXIO	- Invalid device
 *		ENFILE	- File table overflow
 *
 * SERIALIZATION:  This routine must be called with no locks held.
 */

static
int
devcopen (
	dev_t		devno,		/* dev number of device to open	*/
	int		flags,		/* DREAD, DWRITE, and/or DMOUNT	*/
	chan_t		chan,		/* device channel number	*/
	int		ext,		/* open extension parameter	*/
	struct gnode **	gpp)		/* spec gnode return		*/
{
	int		rc;		/* return codes			*/
	struct devnode *dp;		/* devnode for device		*/
	struct gnode *	gp;		/* dev gnode of device		*/
	int		majorno;        /* device major number          */
	int		mpsafe;         /* 1 = safe, 0 = funneled       */
	label_t		jb;		/* setjmpx/longjmpx buffer      */

	/* get the devnode for the device if it wasn't passed in */
	if (*gpp == NULL)
	{
		uint status = 0;	/* device switch status		*/
		int devtype;		/* type of device		*/

		/* get and check the status of the dev switch entry */
 		if (devswqry(devno, &status, NULL) || status == DSW_UNDEFINED)
			return ENXIO;

		/* set the device type for devget */
		devtype = (status & DSW_BLOCK) ? VBLK
			  : (status & DSW_MPX) ? VMPC : VCHR;

		/*
		 * If we succesfully return from devget(), the devnode,
		 * and hence the gnode, is locked.
		 */
		if (rc = devget(devno, chan, devtype, &dp))
			return rc;
		gp = DTODGP(dp);
	}
	else
	{
		/* Lock the devnode so the counts are protected. */
		gp = *gpp;
		dp = DGTODP(gp);
		DEVNODE_LOCK(dp);
	}

	/* Check for device closing.  If it is then wait till the
	 * close completes.
	 */
	while (dp->dv_flag & DV_CLOSING)
	{
		dp->dv_flag |= DV_OPENPENDING;
		rc = e_sleep_thread(&ocr_event, &dp->dv_lock,
				    INTERRUPTIBLE|LOCK_SIMPLE);
		if (rc == THREAD_INTERRUPTED)
		{
			if (*gpp == NULL)
				devput(dp);
			else
				DEVNODE_UNLOCK(dp);
			return EINTR;
		}
	}

	/* Set flags before calling open.  If open sleeps() I want to
	 * err on the conservative side in the case of mount.
	 * Opens for read are ok.
	 */
	if (flags & DMOUNT)
	{
		if (gp->gn_wrcnt || gp->gn_mntcnt)
		{
			if (*gpp == NULL)
				devput(dp);
			else
				DEVNODE_UNLOCK(dp);
			return EBUSY;
		}
	}

	/* adjust open counts */
	if (flags & DREAD)
		gp->gn_rdcnt += 1;
	if (flags & DWRITE)
		gp->gn_wrcnt += 1;
	if (flags & DMOUNT)
		gp->gn_mntcnt += 1;
	
	majorno = major(devno);
	mpsafe = devsw[majorno].d_opts & DEV_MPSAFE;

	/* We should never hold any private locks across the device open.
	 * For funneled device drivers, we are required to hold the
	 * kernel lock across the open call.
	 */
	DEVNODE_UNLOCK(dp);

	ASSERT(!IS_LOCKED(&kernel_lock));
	if (!mpsafe)
		lockl(&kernel_lock, LOCK_SHORT);

        /*
         * Establish exception return point.  Drivers can call
         * preemtible services that longjmp.  (eg. lockl)
         */
        if ((rc = setjmpx(&jb)) == 0)
	{
		DD_ENT(rc = , 
		       (*devsw[majorno].d_open)(devno,
		 		     	        flags & DMASK,
				     	        chan,
				     	        ext),
	       	       IPRI_NOPREMPT,majorno);

		clrjmpx(&jb);
	}

	/* Restore state of locks prior to call to device. */
	if (!mpsafe)
		unlockl(&kernel_lock);

	if (rc)
	{
		/* Lock the devnode to adjust counts */
		DEVNODE_LOCK(dp);

		/* If mpx, then decrement count and determine if devput
		   should be called, otherwise for all other drivers
	  	   always call close on error.
		 */
		if ((gp->gn_type == VMPC))
		{
			/* adjust open counts */
			if (flags & DREAD)
				gp->gn_rdcnt -= 1;
			if (flags & DWRITE)
				gp->gn_wrcnt -= 1;
			if (flags & DMOUNT)
				gp->gn_mntcnt -= 1;

			if (*gpp == NULL &&
			    (gp->gn_rdcnt + gp->gn_wrcnt + gp->gn_mntcnt) == 0)
				devput(dp);
			else
				DEVNODE_UNLOCK(dp);
		}
		else 
		{
			(void)devcclose(gp, flags);
			if (*gpp == NULL)
				devput(dp);
			else
				DEVNODE_UNLOCK(dp);
		}
	}
	else
	{
		*gpp = gp;
	}

	return rc;

}

/*
 * NAME:	rdevclose(gp, flags)
 *
 * FUNCTION:	registered device close for those who want the gnode cleaned up
 *
 * PARAMETERS:	gp 	- pointer to the gnode structure that 
 *			  represents the device
 *		flags	- device open flags (DREAD, DWRITE, and/or DMOUNT)
 *
 * RETURN :	EINVAL - Invalid file pointer
 */

int
rdevclose (
	struct gnode *	gp,		/* gnode of device to close	*/
	int		flags)		/* device open flags		*/
{
	int		rc;		/* return code			*/
	struct devnode	* dp;
	int		waslocked;	/* kernel lock status		*/

	/* Lose the kernel lock to avoid hierarchy problems.  */
	if (waslocked = IS_LOCKED(&kernel_lock))
		unlockl(&kernel_lock);

	dp = DGTODP(gp);
	DEVNODE_LOCK(dp);
	rc = devcclose(gp, flags);
	(void)devput(dp);

	/* Restore kernel lock if needed. */
	if (waslocked)
		lockl(&kernel_lock, LOCK_SHORT);
	return rc;
}

/*
 * NAME:	devcclose(gp, flags)
 *
 * FUNCTION:	Common device close, shared between specfs and devclose.
 *
 * PARAMETERS:	gp 	- pointer to the gnode structure that 
 *			  represents the device
 *		flags	- device open flags (DREAD, DWRITE, and/or DMOUNT)
 *
 * RETURN :	EINVAL - Invalid file pointer
 *
 * SERIALIZATION:  This routine must be called with the DEVNODE_LOCK held.
 */

int
devcclose (
	struct gnode *	gp,		/* gnode of device to close	*/
	int		flags)		/* device open flags		*/
{
	int		cnt;		/* sum of open counts		*/
	int		rc = 0;		/* return code			*/
	label_t		jb;		/* setjmpx/longjmpx buffer	*/

	/* adjust the open counts */
	if (flags & DREAD)
		gp->gn_rdcnt -= 1;
	if (flags & DWRITE)
		gp->gn_wrcnt -= 1;
	if (flags & DMOUNT)
		gp->gn_mntcnt -= 1;

	/* We always call the device close entry point for mpx devices.
	 * If the total of the open counts is zero, then after this close
	 * there will be no remaining open references to the device.
	 * For non-mpx devices, this is the only case in which we want
	 * to call the close entry point of the device.
	 * In all cases, this instance of the open is considered closed.
	 */
	cnt = gp->gn_rdcnt + gp->gn_wrcnt + gp->gn_mntcnt;
	if (cnt == 0 || gp->gn_type == VMPC)
	{	
		struct devnode *dp;     /* devnode for the gnode        */
		int	majorno;        /* device major number          */
		int	mpsafe;         /* 1 = safe, 0 = funneled       */

		dp = DGTODP(gp);
		majorno = major(gp->gn_rdev);
		mpsafe = devsw[majorno].d_opts & DEV_MPSAFE;

		/* Mark the device as closing to prevent opens */
		dp->dv_flag |= DV_CLOSING;
		
		/* sleep uninterruptibly waiting for revoke to finish
		 */
		while (dp->dv_flag & DV_REVOKING)
		{
			dp->dv_flag |= DV_CLOSEPENDING;
			e_sleep_thread(&ocr_event, &dp->dv_lock, LOCK_SIMPLE);
		}		

		/* Release private locks before calling the driver.
		 * For funneled device drivers, we are required to hold
		 * the kernel lock across the close call.
		 */
		DEVNODE_UNLOCK(dp);
		ASSERT(!IS_LOCKED(&kernel_lock));
		if (!mpsafe)
			lockl(&kernel_lock, LOCK_SHORT);

		/*
		 * Establish exception return point.  Drivers can call
		 * preemtible services that longjmp.  (eg. lockl)
		 */
		if ((rc = setjmpx(&jb)) == 0)
		{
			DD_ENT(rc =,
		       	       (*devsw[majorno].d_close)(gp->gn_rdev, 
							 gp->gn_chan, 0),
		       	       IPRI_NOPREMPT,majorno);

			clrjmpx(&jb);
		}

		/* Restore locks to previous state. */
		if (!mpsafe)
			unlockl(&kernel_lock);
		DEVNODE_LOCK(dp);

		/* Wake up any pending opens. */
		dp->dv_flag &= ~DV_CLOSING;
		if (dp->dv_flag & DV_OPENPENDING)
		{
			dp->dv_flag &=  ~DV_OPENPENDING;
			e_wakeup(&ocr_event);
		}
	}

	return rc;
}

/*
 * NAME:	rdevread(rdev, uiop, chan, ext)
 *
 * FUNCTION:	This function reads from the specified device.  This
 *		function is called from both the specfs and the fp services
 *		(fp_read()) for direct device opens (via fp_opendev()).
 *
 * PARAMETERS:	rdev	- device number of device to read from
 *		uiop	- read location information
 *		chan	- device channel number
 * 		ext	- read extension parameter
 *
 * RETURN :	errors from the device read entry point
 *			
 */

rdevread (
	dev_t		rdev,		/* dev number of device to read	*/
	struct uio *	uiop,		/* read location information	*/
	chan_t		chan,		/* device channel number	*/
	int		ext)		/* read extension parameter	*/
{
	int		rc;		/* return code			*/

	DD_ENT(rc =,
	       (*devsw[major(rdev)].d_read)(rdev, uiop, chan, ext),
	       IPRI_BASE,major(rdev));

	return rc;
}

/*
 * NAME:	rdevwrite(rdev, uiop, chan, ext)
 *
 * FUNCTION:	This function writes to the specified device.  This
 *		function is called from both the specfs and the fp services
 *		(fp_write()) for direct device opens (via fp_opendev()).
 *
 * PARAMETERS:	rdev	- device number of device to write to
 *		uiop	- write location information
 *		chan	- device channel number
 * 		ext	- write extension parameter
 *
 * RETURN :	errors from the device write entry point
 */

rdevwrite (
	dev_t		rdev,		/* dev num of device to write	*/
	struct uio *	uiop,		/* write location information	*/
	chan_t		chan,		/* device channel number	*/
	int		ext)		/* write extension parameter	*/
{
	int		rc;		/* return code			*/

	DD_ENT(rc = , (*devsw[major(rdev)].d_write) 
			(rdev, uiop, chan, ext),IPRI_BASE,major(rdev));

	return rc;
}

/*
 * NAME:	rdevselect(rdev, corl, event, reventp, notify, chan)
 *
 * FUNCTION:	perform select operation to character or mpx device
 *		from the file system
 *
 * PARAMETERS:	rdev 	- device number of device to select on
 *		corl	- select correlator
 *		event	- select event
 *		reventp	- event list
 *		notify  - function pointer of notify routine for nested polls
 *		chan	- device channel number
 *
 * RETURN :	Errors from d_select()
 */

rdevselect (
	dev_t		rdev,		/* device number of device	*/
	int		corl,		/* select correlator		*/
	ushort		event,		/* select event			*/
	ushort *	reventp,	/* event list			*/		
	void		(* notify)(),	/* notify func for nested polls	*/
	chan_t		chan)		/* device channel number	*/
{
	int		rc;		/* return code			*/

	rc = selreg(corl, rdev, chan, event, notify);
	if (rc == 0)
		DD_ENT(rc =,
		       (*devsw[major(rdev)].d_select)(rdev,
						      event,
						      reventp,
						      chan),
		       IPRI_BASE,major(rdev));
	return rc;
}

/*
 * NAME:	rdevioctl(rdev, cmd, arg, flag, chan, ext)
 *
 * FUNCTION:	Common device ioctl, shared between specfs and devclose.
 *
 * PARAMETERS:	rdev 	- device number of device to issue ioctl for
 *		cmd	- ioctl command
 *		arg	- address of parameter block
 *		flag	- file open flags
 *		chan	- device channel number
 * 		ext	- extended ioctl information
 *
 * RETURN :	Errors from d_ioctl
 */

rdevioctl (
	dev_t		rdev,		/* dev num of device to ioctl	*/
	int		cmd,		/* user specified ioctl command	*/
	int		arg,		/* argument for ioctl command	*/
	long		flag,		/* file open flags		*/
	chan_t		chan,		/* device channel number	*/
	caddr_t		ext)		/* extended ioctl information	*/
{
	int		rc;		/* return code			*/

	DD_ENT(rc =, (*devsw[major(rdev)].d_ioctl)(rdev,
						    cmd,
						    arg,
						    flag,
						    chan,
						    ext,
						    &u.u_ioctlrv),
			IPRI_BASE,major(rdev));

	return rc;
}
