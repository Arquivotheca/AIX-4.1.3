static char sccsid[] = "@(#)15        1.16.1.3  src/bos/kernel/s/priv/chpriv.c, sysspriv, bos411, 9428A410j 2/1/94 17:03:00";

/*
 * COMPONENT_NAME:  TCBPRIV
 *
 * FUNCTIONS:  chpriv(), fchpriv()
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
#include "sys/priv.h"
#include "sys/malloc.h"
#include "sys/uio.h"
#include "sys/lockl.h"
#include "sys/vfs.h"
#include "sys/audit.h"

#define	DPRINTF(args)

/*
 * Definitions
 */

static int setpcl(struct vnode *vp, struct pcl *buf, int siz, struct ucred *crp);

static int chkpcl(struct pcl *pclp);

/*
 * NAME: fchpriv()        (system call entry point)
 *
 * FUNCTION: set the PCL for a file descriptor.
 *
 * PARAMETERS: fdes, pclbuf, pclsiz.  File descriptor, PCL buffer and PCL size.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
fchpriv(int		fdes,
	struct pcl	*pclbuf,
	int		pclsiz)
{
	struct file	*fp;
	struct vnode	*vp;
	static int svcnum = 0;


	if(audit_flag && audit_svcstart("FILE_Privilege",&svcnum,0)){
		audit_svcfinis();
	}

	/* 
	 * must have SET_OBJ_PRIV privilege 
	 * and be on the Trusted Path
	 */
	if (privcheck(SET_OBJ_PRIV) ||
	    (privcheck(TRUSTED_PATH) && privcheck(BYPASS_TPATH)))
	{
		u.u_error = EPERM;
		goto out;
	}

	/* get the file pointer from the file descriptor */
	if (u.u_error = getft(fdes, &fp, DTYPE_VNODE))
		goto out;

	vp = fp->f_vnode;

	/* since we are going to modify, this FS must be writable */
	if (vp->v_vfsp->vfs_flag & VFS_READONLY)
	{
		u.u_error = EROFS;
		goto outf;
	}

	/* fail immediately if file is opened for write by another */
	if (((fp->f_flag & FWRITE) == 0))
		u.u_error = EBUSY;
	else
	{
		struct ucred	*crp;

		crp = crref();
		u.u_error = setpcl(vp, pclbuf, pclsiz, crp);
		crfree(crp);
	}

outf:
	/* Decrement the file descriptor count */
	ufdrele(fdes);
out:
	return(u.u_error ? -1 : 0);
}


/*
 * NAME: chpriv()        (system call entry point)
 *
 * FUNCTION: set the PCL for a file pathname
 *
 * PARAMETERS: path, pclbuf, pclsiz.  Pathname, PCL buffer and PCL size.
 *
 * RETURN VALUES: 0 on success; -1 and u.u_error set on error
 */

int
chpriv(char		*path,
       struct pcl	*pclbuf,
       int		pclsiz)
{
	struct vnode	*vp;
	struct ucred	*crp;
	static int svcnum = 0;


	if(audit_flag && audit_svcstart("FILE_Privilege",&svcnum,0)){
		audit_svcfinis();
	}

	/* 
	 * must have SET_OBJ_PRIV privilege 
	 * and be on the Trusted Path
	 */
	if (privcheck(SET_OBJ_PRIV) ||
	    (privcheck(TRUSTED_PATH) && privcheck(BYPASS_TPATH)))
	{
		u.u_error = EPERM;
		goto out;
	}

	crp = crref();

	DPRINTF(("chpriv(): lookupname(%s, ... )\n", path));
	/* Convert the pathname into a vnode pointer */
	if (u.u_error = lookupname(path, USR, L_SEARCH|L_EROFS, NULL, &vp, crp))
		goto outr;

	/* Fail immediately if this file is open for write */
	DPRINTF(("setpcl(): check open for write\n"));

	/* set the PCL */
	u.u_error = setpcl(vp, pclbuf, pclsiz, crp);

	VNOP_RELE(vp);

outr:
	crfree(crp);
out:
	DPRINTF(("chpriv(): returning\n"));
	return(u.u_error ? -1 : 0);
}


int
setpcl(struct vnode	*vp,
       struct pcl	*buf,
       int		siz,
       struct ucred	*crp)
{
	struct uthread  *ut = curthread->t_uthreadp;
	struct pcl	*pclp;
	struct vattr	vattrbuf;
	struct inode	*ip;
	struct uio	uio;
	struct iovec	iov;
	int		rc;
	static int tcbmod = 0;

	DPRINTF(("setpcl(): starting\n"));

	/* Fail immediately if filesystem has no VNOP_SETPCL */
	DPRINTF(("setpcl(): check VNOP\n"));
	if (vp->v_gnode->gn_ops->vn_setpcl == NULL)
		return(EINVAL);

	/* Fail immediately if size is less that the smallest PCL */
	if (siz < sizeof(struct pcl) - sizeof(struct pcl_entry))
		return(EINVAL);

	if ((pclp = (struct pcl *)malloc(siz)) == NULL)
		return(ENOMEM);

	if (copyin(buf, pclp, siz))
	{
		free(pclp);
		return(EFAULT);
	}

	/* Fail if siz < PCL length */
	if (siz < pclp->pcl_len)
	{
		free(pclp);
		return(EINVAL);
	}
	
	DPRINTF(("setpcl(): checking PCL\n"));
	if (chkpcl(pclp))
	{
		free(pclp);
		return(EINVAL);
	}
	
	DPRINTF(("setpcl(): VNOP 0x%x\n", vp->v_gnode->gn_ops->vn_setpcl));
	/*
	 * build a uio structure to hold the PCL
	 */
	iov.iov_base = (char *)pclp;
	iov.iov_len = pclp->pcl_len;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = pclp->pcl_len;
	uio.uio_segflg = UIO_SYSSPACE;

	rc = VNOP_SETPCL(vp, &uio, crp);
	free(pclp);

        if((vp->v_gnode->gn_flags & GNF_TCB)){
		if (ut->ut_audsvc)
			ut->ut_audsvc->svcnum = 1;
        	if(audit_flag && audit_svcstart("TCB_Mod", &tcbmod, 0)){
        		audit_svcfinis();
        	}
        }

	if (rc == ENOSYS)
		rc = EINVAL;
	return(rc);
}

/*
 * chkpcl():  check the validity of the pcl (i.e. travel down the 
 * chain making sure the lengths are correct).
 */
int
chkpcl(struct pcl	*pclp)
{
	struct pcl_entry	*pceptr;
	struct pce_id		*pceidptr;
	int	pcltot;
	int	pcllen;
	int	pcetot;
	int	pcelen;

	/*
	 * If the S_ITP bit is on, so must the S_ITCB
	 */
	if ((pclp->pcl_mode & (S_ITP|S_ITCB)) == S_ITP)
		return(-1);
		
	/* pcllen is the total length of the PCL */
	pcllen = pclp->pcl_len;
	/* pcltot is a running total of the entry's and id's lengths */
	pcltot = sizeof(struct pcl) - sizeof(struct pcl_entry);
	pceptr = pclp->pcl_ext;
	
	/*
	 * go through the PCLs entries
	 */
	while (pcltot < pcllen)
	{
		pcelen = pceptr->pce_len;
		pcetot = sizeof(struct pcl_entry) - sizeof(struct pce_id);
		pceidptr = pceptr->pce_id;
		if (pcetot == pcelen)
			return(-1);	/* no IDs in this entry... */
		/*
		 * go through this entry's IDs
		 */
		while (pcetot < pcelen)
		{
			pcetot += pceidptr->id_len;
			pceidptr = pcl_id_nxt(pceidptr);
		}
		if (pcetot != pcelen)
			return(-1);	/* sum of ID lengths not correct... */
		pcltot += pcelen;
		pceptr = pcl_nxt(pceptr);
	}
	if (pcltot != pcllen)
		return(-1);	/* sum of entry lengths not correct... */

	return(0);
}
