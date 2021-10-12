static char sccsid[] = "@(#)51	1.10  src/bos/kernel/specfs/gno_fops.c, sysspecfs, bos411, 9428A410j 4/29/91 12:00:17";
/*
 * COMPONENT_NAME: (SYSSPECFS) Special File System
 *
 * FUNCTIONS: gno_rw, gno_close, gno_ioctl, gno_select, gno_badop
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/types.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/uio.h"
#include "sys/syspest.h"
#include "sys/errno.h"
#include "sys/conf.h"
#include "sys/device.h"
#include "sys/poll.h"
#include "sys/dev.h"

/* Declarations */
int	gno_rw(), gno_ioctl(), gno_select(), gno_close(), gno_badop();

/* Definitions */

#define	f_gnode	f_vnode

# define 	FPINVAL(fp) 	((fp)->f_gnode == NULL || (fp)->f_count != 1 \
			|| ((fp)->f_flag & (FREAD|FWRITE)) == 0)

struct fileops gno_fops = 
{
	gno_rw, gno_ioctl, gno_select, gno_close, gno_badop
};

/*
 * NAME:	gno_rw
 *
 * FUNCTION:	perform read/write operation to character or mpx device
 *		from the file system.
 *
 * RETURN :	Errors from devread() / devwrite()
 */

int
gno_rw(fp, rw, uiop, ext)
struct file	*fp;
enum uio_rw	rw;
struct uio	*uiop;
int		ext;
{
	int rc;
	struct gnode *gp;
	
	ASSERT( (rw == UIO_READ) || (rw == UIO_WRITE));

	if (FPINVAL(fp)) {
		rc = EINVAL;
	} else {
		gp = (struct gnode *) fp->f_gnode;
		if ( rw == UIO_READ )
			rc = rdevread(gp->gn_rdev, uiop, gp->gn_chan, ext);
		else
			rc = rdevwrite(gp->gn_rdev, uiop, gp->gn_chan, ext);
	}

	return(rc);
}

/*
 * NAME:	gno_ioctl
 *
 * FUNCTION:	perform ioctl operation on character or mpx device.
 *
 * RETURN :	Errors from devioctl()
 */

int
gno_ioctl(fp, cmd, arg, ext)
struct file	*fp;
int		cmd;
caddr_t		arg;
int		ext;
{
	int rc;
	struct gnode *gp;

	if (FPINVAL (fp))
		rc =  EINVAL;
	else
	{	gp = (struct gnode *) fp->f_gnode;
		rc = rdevioctl (gp->gn_rdev, cmd, arg, 
				DKERNEL|fp->f_flag, gp->gn_chan, ext);
	}

	return rc;

}

/*
 * NAME:	gno_close
 *
 * FUNCTION:	perform close operation on character or mpx device.
 *
 * RETURN :	Errors from devclose()
 */

int
gno_close(fp)
struct file	*fp;
{
	int rc;

	if ((fp->f_gnode == NULL) || !(fp->f_flag & (FREAD|FWRITE)))
		rc =  EINVAL;
	else
		rc = rdevclose (fp->f_gnode, fp->f_flag);
	return rc;

}

/*
 * NAME:	gno_select (fp, corl, e, re, notify)
 *
 * FUNCTION:	perform select operation to character or mpx device
 *		from the file system
 *
 * PARAMETERS:	fp 	- file pointer
 *		corl	- correlator
 *		event	- select event
 *		reventp	- event list
 *		notify	- function pointer of routine for nested poll
 *
 * RETURN :	Errors from rdevselect()
 */

int
gno_select (fp, corl, event, reventp, notify)
struct file	*fp;
int		corl;
ushort		event;
ushort		*reventp;
void (*notify)();
{
	int rc;
	struct gnode *gp;

	gp = (struct gnode *)fp->f_gnode;

	/* char and mpx. other devices, blk is no-op	 */
	switch (gp->gn_type)
	{	case VCHR:
		case VMPC:
			rc = rdevselect (gp->gn_rdev, corl, event, reventp, 
					 notify, gp->gn_chan);
			break;
		case VBLK:
			/* Return 0 by default */
			rc = 0;
			break;
		default:
			BUGLPR (1, 1, ("Unsupported device type\n"));
			rc = EINVAL;
			break;
	}

	return rc;
}

int
gno_badop()
{
	return EINVAL;
}
