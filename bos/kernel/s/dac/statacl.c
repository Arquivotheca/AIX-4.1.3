static char sccsid[] = "@(#)89        1.9.1.3  src/bos/kernel/s/dac/statacl.c, syssdac, bos411, 9428A410j 12/15/93 09:05:26";

/*
 * COMPONENT_NAME:  TCBACL
 *
 * FUNCTIONS:  statacl(), fstatacl()
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
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
#include "sys/acl.h"
#include "sys/mode.h"
#include "sys/uio.h"
#include "sys/lockl.h"

#define	DPRINTF(args)

/*
 * Definitions
 */

static int get_mode_bits(char *buf,int len,struct vnode *vp,struct ucred *crp);

/*
 * NAME: fstatacl()        (system call entry point)
 *
 * FUNCTION: get the ACL for a file descriptor.
 *
 * PARAMETERS: fdes, cmd, buf, len. File descriptor, cmd word, ACL struct
 *             and length.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
fstatacl(fdes, cmd, buf, len)
int		fdes;
int		cmd;
struct	acl	*buf;
int		len;
{
	struct file	*fp;
	struct ucred	*crp;


	if (cmd != 0)
	{
		u.u_error = EINVAL;
		goto out;
	}

	/* get the file pointer from the file descriptor */
	if (u.u_error = getft(fdes, &fp, DTYPE_VNODE))
		goto out;

	crp = crref();
	u.u_error = getacl(fp->f_vnode, buf, len, crp);
	crfree(crp);

	/* Decrement the file descriptor count */
	ufdrele(fdes);

out:
	return(u.u_error ? -1 : 0);
}


/*
 * NAME: statacl()        (system call entry point)
 *
 * FUNCTION: get the ACL for a file pathname
 *
 * PARAMETERS: path, cmd, buf, len. Pathname, cmd word, ACL struct
 *             and length.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
statacl(path, cmd, buf, len)
char		*path;
int		cmd;
struct	acl	*buf;
int		len;
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

	/* get the ACL */
	u.u_error = getacl(vp, buf, len, crp);

	VNOP_RELE(vp);

out:
	crfree(crp);

	return(u.u_error ? -1 : 0);
}

getacl(vp, buf, len, crp)
struct vnode	*vp;
struct acl	*buf;
int		len;
struct ucred	*crp;
{
	struct uio	uio;
	struct iovec	iov;
	int	rc;

	/*
	 * if this file system doesn't support a VNOP for ACLs 
	 * then craft one from the mode bits
	 */
	if (vp->v_gnode->gn_ops->vn_getacl == NULL)
		return(get_mode_bits((char *)buf, len, vp, crp));

DPRINTF(("getacl(): #2 buf=0x%x len=0x%x\n", buf, len));
	/*
	 * create a uio structure to hold the ACL
	 */
	iov.iov_base = (char *)buf;
	iov.iov_len = len;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = len;
	uio.uio_segflg = UIO_USERSPACE;

	if (rc = VNOP_GETACL(vp, &uio, crp))
	{
		if (rc != ENOSYS)
			return(rc);
		return(get_mode_bits((char *)buf, len, vp, crp));
	}

	return(0);
}


int
get_mode_bits(char		*buf,
	      int		len,
	      struct vnode	*vp,
	      struct ucred	*crp)
{
	struct acl	aclbuf;
	struct vattr	vattrbuf;
	int	rc;

	/* the minimum len is the sizeof a base ACL */
	if (len < ACL_SIZ)
	{
		aclbuf.acl_len = ACL_SIZ;

		if (copyout(&aclbuf.acl_len, buf, 
					sizeof(aclbuf.acl_len)))
			return(EFAULT);
		return(ENOSPC);
	}

	/* no ACL - use mode bits */
	if (rc = VNOP_GETATTR(vp, &vattrbuf, crp))
		return(rc);

	aclbuf.acl_len = ACL_SIZ;
	aclbuf.acl_mode = vattrbuf.va_mode & 07000;
	aclbuf.o_access = vattrbuf.va_mode & 7;
	 vattrbuf.va_mode >>= 3;
	aclbuf.g_access = vattrbuf.va_mode & 7;
	 vattrbuf.va_mode >>= 3;
	aclbuf.u_access = vattrbuf.va_mode & 7;
	
	if (copyout(&aclbuf, buf, ACL_SIZ))
		return(EFAULT);
	return(0);
}
