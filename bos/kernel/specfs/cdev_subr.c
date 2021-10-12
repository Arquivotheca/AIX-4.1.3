static char sccsid[] = "@(#)94	1.28.1.3  src/bos/kernel/specfs/cdev_subr.c, sysspecfs, bos411, 9428A410j 4/19/94 16:16:37";
/*
 * COMPONENT_NAME: (SYSSPECFS) Special File Filesystem
 *
 * FUNCTIONS: cdev_close, cdev_open, cdev_setattr, cdevchmod,
 *            cdev_rdwr, cdev_strategy, cdevmpx
 * ORIGINS: 27
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
#include "sys/priv.h"
#include "sys/dev.h"
#include "sys/id.h"
#include "sys/specnode.h"
#include "sys/limits.h"

/* Definitions */

/* Declarations */
static cdevchmod();

/*
 * NAME:	cdev_open (dgp, flag, ext)
 *
 * FUNCTION:	This function opens the character device indicated by the
 *		specified character device special file.  It establishes
 *		the controlling terminal, if appropriate.
 *
 * PARAMETERS:	dgp	- character device gnode to open
 *		flag	- open flags
 *		ext	- extended open info
 *
 * RETURN :	returns from the device open routine
 *		ENXIO	- device number invalid
 *		EINTR	- interrupted
 */

int
cdev_open (
	struct gnode *	dgp,		/* dev gnode to open	*/
	int		flag,		/* open mode		*/
	int		ext)		/* extended open info	*/
{
	int		rc = 0;		/* return code		*/
	dev_t		dev;		/* real device number	*/
	int		noctl;		/* no controlling tty	*/

	dev = dgp->gn_rdev;

	/* We pass only the allowed open flags and whether the caller
	 * was in kernel mode.
	 */
	flag &= (FMASK|FKERNEL);

	/* Hang onto the state of controlling terminal to see if
	 * it is established by the device open.
	 */
	noctl = (u.u_ttyp == NULL);

	/* register device open */
	rc = rdevopen(dev, flag, NULL, ext, &dgp);

	/* Save the controlling terminal device number if the device
	 * open established the controlling terminal.
	 */
	if (rc == 0 && noctl && u.u_ttyp)
		u.u_ttyd = dev;

	return rc;
}

/*
 * NAME:	cdev_rdwr(sgp, pvp, rwmode, flags, uiop, ext, crp)
 *
 * FUNCTION:	This function reads or writes to the character device
 *		indicated by the specified character device special file.
 *		The file times are updated appropriately.
 *
 * PARAMETERS:	sgp	- spec gnode of device to read or write
 *		pvp	- vnode of PFS character device special file
 *		rwmode	- read or write type (UIO_READ or UIO_WRITE)
 *		flag	- file open flags
 *		uiop	- read or write location information
 *		ext	- extended read or write information
 *
 * RETURN :	returns from devsw[] interface
 *		EINVAL	- negative r/w offset
 */

int
cdev_rdwr (
	struct gnode *	sgp,		/* spec gnode of device		*/
	struct vnode *	pvp,		/* PFS vnode of device		*/
	enum uio_rw 	rwmode,		/* UIO_READ or UIO_WRITE	*/
	int 		flags,		/* file open flags		*/
	struct uio *	uiop,		/* read or write location info	*/
	int 		ext,		/* extended read or write info	*/
	struct ucred *  crp)		/* credentials			*/
{
	dev_t		dev;		/* device number		*/
	int		rc;		/* return code			*/
	struct timestruc_t t;

	/* 
	 * negative offsets and offsets that cannot fit in a buf header 
	 * b_blkno are not allowed for read or write 
	 */
	if (uiop->uio_offset < 0 || uiop->uio_offset > DEV_OFF_MAX)
		return EINVAL;

	dev = sgp->gn_rdev;

	if (rwmode == UIO_READ)
	{
		/* update the access time on the PFS file */
		if (pvp) {
			curtime(&t);
			(void)VNOP_SETATTR(pvp, V_STIME, &t, 0, 0, crp);
		}

		rc = rdevread(dev, uiop, sgp->gn_chan, ext);
	}
	else	/* UIO_WRITE */
	{
		/* update the update and change times on the PFS file */
		if (pvp) {
			curtime(&t);
			(void)VNOP_SETATTR(pvp, V_STIME, 0, &t, &t, crp);
		}

		rc = rdevwrite(dev, uiop, sgp->gn_chan, ext);
	}

	return rc;
}

int
mpx_dev(dev_t	dev)
{
	int	devstat;
	int	rc;

	rc = devswqry(dev, &devstat, NULL);

	return (rc == 0 && devstat & DSW_DEFINED && devstat & DSW_MPX);
}
