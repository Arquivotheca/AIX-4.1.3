static char sccsid[] = "@(#)16        1.8.1.3  src/bos/kernel/s/priv/statpriv.c, sysspriv, bos411, 9428A410j 12/15/93 09:05:52";

/*
 * COMPONENT_NAME:  TCBPRIV
 *
 * FUNCTIONS:  statpriv(), fstatpriv()
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include "sys/errno.h"
#include "sys/systm.h"
#include "sys/inode.h"
#include "sys/stat.h"
#include "sys/file.h"
#include "sys/user.h"
#include "sys/vattr.h"
#include "sys/vnode.h"
#include "sys/priv.h"
#include "sys/mode.h"
#include "sys/uio.h"
#include "sys/lockl.h"

#define	DPRINTF(args)	

/*
 * Definitions
 */

static int getpcl(struct vnode *vp,struct pcl *buf,int len,struct ucred *crp);
static int dummy_pcl(struct vnode *vp,struct pcl *buf,int len,struct ucred *crp);

/*
 * NAME: fstatpriv()        (system call entry point)
 *
 * FUNCTION: get the PCL for a file descriptor.
 *
 * PARAMETERS: fdes, cmd, buf, len. File descriptor, cmd word, PCL struct
 *             and length.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
fstatpriv(int		fdes,
	  int		cmd,
	  struct pcl	*buf,
	  int		len)
{
	struct file	*fp;

	/* get the file pointer from the file descriptor */
	if (u.u_error = getft(fdes, &fp, DTYPE_VNODE))
		goto out;

	if (cmd != 0)
	{
		u.u_error = EINVAL;
	}
	else
	{
		struct ucred	*crp;

		crp = crref();
		u.u_error = getpcl(fp->f_vnode, buf, len, crp);
		crfree(crp);
	}

	/* Decrement the file descriptor count */
	ufdrele(fdes);

out:
	return(u.u_error ? -1 : 0);
}


/*
 * NAME: statpriv()        (system call entry point)
 *
 * FUNCTION: get the PCL for a file pathname
 *
 * PARAMETERS: path, cmd, buf, len. Pathname, cmd word, PCL struct
 *             and length.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
statpriv(char		*path,
	 int		cmd,
	 struct pcl	*buf,
	 int		len)
{
	struct vnode	*vp;
	struct ucred	*crp;
	int	flags;

	if (cmd & ~(STX_LINK|STX_MOUNT|STX_HIDDEN)) 
	{
		u.u_error = EINVAL;	/* No weird bits allowed in cmd */
		return(-1);
	}

	/*
	 * Set up lookup flags based upon the supplied cmd.
	 */
	flags = L_SEARCH;
	if (cmd & STX_LINK)
		flags |= L_NOFOLLOW;
	if (cmd & STX_MOUNT)
		flags |= L_NOXMOUNT;
	if (cmd & STX_HIDDEN)
		flags |= L_NOXHIDDEN;

	crp = crref();

	/* Convert the pathname into a vnode pointer */
	if (u.u_error = lookupname(path, USR, flags, NULL, &vp, crp))
		goto out;

	/* get the PCL */
	u.u_error = getpcl(vp, buf, len, crp);

	VNOP_RELE(vp);

out:
	crfree(crp);

	return(u.u_error ? -1 : 0);
}


int
getpcl(struct vnode	*vp,
       struct pcl	*buf,
       int		len,
       struct ucred	*crp)
{
	struct pcl	pclbuf;
	struct vattr	vattrbuf;
	struct uio	uio;
	struct iovec	iov;
	int		rc;

	DPRINTF(("getpcl(): #1 buf=0x%x len=0x%x\n", buf, len));

	/*
	 * If this file system doesn't support VNOP_GETPCL 
	 * then dummy up one...
	 */
	DPRINTF(("getpcl():  vp->v_gnode->gn_ops = %x\n", vp->v_gnode->gn_ops));
	if (vp->v_gnode->gn_ops->vn_getpcl == NULL)
		return(dummy_pcl(vp, buf, len, crp));
	
	DPRINTF(("getpcl(): #2 buf=0x%x len=0x%x\n", buf, len));
	/*
	 * create a uio structure to hold the PCL
	 */
	iov.iov_base = (char *)buf;
	iov.iov_len = len;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = len;
	uio.uio_segflg = UIO_USERSPACE;

	DPRINTF(("getpcl(): #3 buf=0x%x len=0x%x\n", buf, len));
	rc = VNOP_GETPCL(vp, &uio, crp);
	if (rc == ENOSYS)
		return(dummy_pcl(vp, buf, len, crp));

	return(rc);
}


/*
 * dummy_pcl()
 *
 * if filesystem doesn't support PCLs then dummy one up.
 */
int
dummy_pcl(struct vnode	*vp,
	  struct pcl	*buf,
	  int		len,
	  struct ucred	*crp)
{
	struct vattr	vabuf;
	struct pcl	pclbuf;
	int		rc;

	if (len < PCL_SIZ)
	{
		len = PCL_SIZ;
		if (copyout(&len, buf, sizeof(len)))
			return(EFAULT);
		return(ENOSPC);
	}

	if (rc = VNOP_GETATTR(vp, &vabuf, crp))
		return(rc);

	pclbuf.pcl_len = PCL_SIZ;
	pclbuf.pcl_mode = vabuf.va_mode;
	pclbuf.pcl_default.pv_priv[0] = 0;
	pclbuf.pcl_default.pv_priv[1] = 0;
	if (copyout(&pclbuf, buf, PCL_SIZ))
		return(EFAULT);

	return(0);
}
		
 
