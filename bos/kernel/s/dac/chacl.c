static char sccsid[] = "@(#)90        1.11.1.4  src/bos/kernel/s/dac/chacl.c, syssdac, bos411, 9428A410j 2/1/94 17:02:13";

/*
 * COMPONENT_NAME:  TCBACL
 *
 * FUNCTIONS:  chacl(), fchacl()
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
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
#include "sys/uio.h"
#include "sys/lockl.h"
#include "sys/vfs.h"
#include "sys/audit.h"

#define	DPRINTF(args)

/*
 * Definitions
 */

static int set_mode_bits(char *buf,int len,struct vnode *vp,struct ucred *crp);
static int setacl(struct vnode *vp,struct acl *buf,int siz,struct ucred *crp);

/*
 * NAME: fchacl()        (system call entry point)
 *
 * FUNCTION: set the ACL for a file descriptor.
 *
 * PARAMETERS: fdes, aclbuf, aclsiz.  File descriptor, ACL buffer and ACL size.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
fchacl(int		fdes,
       struct acl	*aclbuf,
       int		aclsiz)
{
	struct file	*fp;
	struct vnode	*vp;
	static int svcnum=0;

	if(audit_flag && audit_svcstart("FILE_Acl",&svcnum,0)){
		audit_svcfinis();
	}

	/* get the file pointer from the file descriptor */
	if (u.u_error = getft(fdes, &fp, DTYPE_VNODE))
		goto out;

	vp = fp->f_vnode;
	if (vp->v_vfsp->vfs_flag & VFS_READONLY)
	{
		u.u_error = EROFS;
	}
	else
	{
		struct ucred *crp;

		crp = crref();
		u.u_error = setacl(vp, aclbuf, aclsiz, crp);
		crfree(crp);
	}
	
	/* Decrement the file descriptor count */
	ufdrele(fdes);

out:
	return(u.u_error ? -1 : 0);
}

/*
 * NAME: chacl()        (system call entry point)
 *
 * FUNCTION: set the ACL for a file pathname
 *
 * PARAMETERS: path, aclbuf, aclsiz.  Pathname, ACL buffer and ACL size.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
chacl(char		*path,
      struct acl	*aclbuf,
      int		aclsiz)
{
	struct vnode	*vp;
	struct ucred	*crp;
	static int svcnum = 0;

	if(audit_flag && audit_svcstart("FILE_Acl",&svcnum,0)){
		audit_svcfinis();
	}

	crp = crref();

	DPRINTF(("chacl(): lookupname(%s, ... )\n", path));
	/* Convert the pathname into a vnode pointer */
	if (u.u_error = lookupname(path, USR, L_SEARCH|L_EROFS, NULL, &vp, crp))
		goto out;

	/* get the ACL */
	u.u_error = setacl(vp, aclbuf, aclsiz, crp);

	VNOP_RELE(vp);

out:
	crfree(crp);

	DPRINTF(("chacl(): returning\n"));
	return(u.u_error ? -1 : 0);
}

int
setacl(struct vnode	*vp,
       struct acl	*buf,
       int		siz,
       struct ucred	*crp)
{
	struct vattr	vattrbuf;
	struct inode	*ip;
	struct uio	uio;
	struct iovec	iov;
	int	len;
	int	rc;
	static int tcbmod = 0;

	/* 
	 * get the length of the ACL (the length is the first word
	 * in the structure -- always...)
	 */
	if (copyin(buf, &len, sizeof(len)))
		return(EFAULT);

	/*
	 * if the ACL length is greater than the given size 
	 * of the buffer, be concerned...
	 */
	if (len > siz)
		return(EINVAL);

	/*
	 * build a uio structure to hold the ACL
	 */
	iov.iov_base = (char *)buf;
	iov.iov_len = len;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = len;
	uio.uio_segflg = UIO_USERSPACE;

	/*
	 * The check for GNF_TCB will be done by the underlying
	 * file system
	 */

	/* if filesystem has no VNOP for ACLs set mode bits */
	if (vp->v_gnode->gn_ops->vn_setacl == NULL)
		return(set_mode_bits((char *) buf, len, vp, crp));

	if (rc = VNOP_SETACL(vp, &uio, crp))
	{
		if (rc != ENOSYS)
			return(rc);
		return(set_mode_bits((char *) buf, len, vp, crp));
	}

	return(0);
}


/*
 * no support for ACLs - use mode bits...
 */
int
set_mode_bits(char		*buf,
	      int		len,
	      struct vnode	*vp,
	      struct ucred	*crp)
{
	struct	acl	acl;
	long	newmode;
	int	rc;

	if (len < ACL_SIZ)
		return(EINVAL);
	if (rc = copyin(buf, &acl, ACL_SIZ))
		return(rc);
	newmode = (acl.acl_mode & 07000)	|
		  (acl.o_access & 7)		| 
		  ((acl.g_access & 7) << 3)	|
		  ((acl.u_access & 7) << 6);
	return(VNOP_SETATTR(vp, V_MODE, newmode, 0, 0, crp));
}
