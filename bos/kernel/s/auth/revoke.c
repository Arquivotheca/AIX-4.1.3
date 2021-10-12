static char sccsid[] = "@(#)06        1.16.1.6  src/bos/kernel/s/auth/revoke.c, syssauth, bos41J, 9519B_all 5/5/95 10:09:12";

/*
 * COMPONENT_NAME:  SYSSAUD
 *
 * FUNCTIONS:  revoke(), frevoke()
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

#include "sys/user.h"
#include "sys/systm.h"
#include "sys/file.h"
#include "sys/vattr.h"
#include "sys/vnode.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/device.h"
#include "sys/sysinfo.h"
#include "sys/lockl.h"
#include "sys/id.h"
#include "sys/fs_locks.h"
#include "sys/sleep.h"
#include "crlock.h"

#define DPRINTF(args)

extern  Simple_lock cred_lock;

static int invalidate(struct file *rfp, struct vnode *rvp, struct vattr *rva);
static int dup(struct file *fp, struct file **nfpp, struct ucred *crp);
static int check(struct vnode *vp, struct vattr *va, struct ucred *crp);
int invalidate_fp(struct file *fp, struct inval_args *inval_check);

struct inval_args {
	struct file  	*rfp; /* for frevoke() this file table entry
                               * should not be invalidated
                               */
        struct vattr 	*rva; /* This contains the device number and channel
                               * to be revoked
                               */
	int		st;   /* Initially 0. On return from ftsearch
                               it contains number of file table entries
                               invalidated
                            */
};


/*
 * Revoke R/W access to a tty for all processes.  If a process is blocked
 * doing I/O then it will be killed.
 */

revoke(char *path)
{
	struct vnode	*vp = NULL;
	struct ucred	*crp;
	struct vattr	va;
	int	rc;

	DPRINTF(("revoke(): starting...\n"));

	crp = crref();

	DPRINTF(("revoke(): lookupname...\n"));
	/* get the underlying object */
	if (u.u_error = lookupname(path, USR, L_SEARCH, NULL, &vp, crp))
		goto out;

	DPRINTF(("revoke(): check...\n"));
	/* verify that it supports revocation */
	if (u.u_error = check(vp, &va, crp))
		goto out;

	DPRINTF(("revoke(): invalidate...\n"));
	/* invalidate ALL file table entries which refer to this object */
	rc = invalidate((struct file *)0, vp, &va);

	DPRINTF(("revoke(): VNOP_REVOKE...\n"));
	/* terminate operations in progress (if there are any) */
	if (rc)
		u.u_error = VNOP_REVOKE(vp, 0, 0, 0, crp);

out:	if (vp)
		VNOP_RELE(vp);

	crfree(crp);

	DPRINTF(("revoke(): ending...  u.u_error=%d\n", u.u_error));
	return(u.u_error ? -1 : 0);
}


/*
 * Revoke R/W access to a tty for all processes except this one.  
 * If a process is blocked doing I/O then it will be killed.
 */

frevoke(int fd)
{
	struct	ucred	*crp;
	struct	file	*fp;
	struct	file	*nfp;
	struct	vnode	*vp;
	struct	vattr	va;
	int	flag = 0x01;
	int	rc;

	U_FD_LOCK();
	if ( (fd < 0 || fd >= U.U_maxofile)  ||
	     ((fp = U.U_ufd[fd].fp) == NULL) ||
	     ((fp->f_flag & FREVOKED) && 1)  ||
	     (U.U_ufd[fd].flags & UF_CLOSING) )
	{
		u.u_error = EBADF;
		U_FD_UNLOCK();
		goto out;
	}

	if (fp->f_type != DTYPE_VNODE)
	{
		u.u_error = EINVAL;
		U_FD_UNLOCK();
		goto out;
	}

        U.U_ufd[fd].flags |= UF_CLOSING;

        while (U.U_ufd[fd].count != 0)
        {
                assert(curproc->p_threadcount > 1);
                (void) e_sleep_thread(&U.U_fdevent, &U.U_fd_lock, LOCK_SIMPLE);
        }

	U.U_ufd[fd].count++;
	U_FD_UNLOCK();

	vp = fp->f_vnode;

	crp = crref();

	/* verify that the object supports revocation */
	if (u.u_error = check(vp, &va, crp))
		goto outf;

	FP_LOCK(fp);
	/* if this open file table entry is shared, get private entry */
	if (fp->f_count > 1)
	{
		FP_UNLOCK(fp);
		if (u.u_error = dup(fp, &nfp, crp))
			goto outf;
		U_FD_LOCK();
		U.U_ufd[fd].fp = nfp;
		U_FD_UNLOCK();
	}
	else
	{
		FP_UNLOCK(fp);
		nfp = fp;
	}

	/* invalidate OTHER file table entries which refer to this object */
	rc = invalidate(nfp, vp, &va);

	/* terminate operations in progress */
	if (rc)
		u.u_error = VNOP_REVOKE(vp,flag,nfp->f_flag,(struct vattr*)&(nfp->f_vinfo),crp);

outf:
	U_FD_LOCK();
	U.U_ufd[fd].flags &= ~UF_CLOSING;
	U_FD_UNLOCK();

	/* Decrement the file descriptor count */
	ufdrele(fd);

	crfree(crp);

out:
	return(u.u_error ? -1 : 0);
}


/*
 * verify that the calling process can revoke access to this object.
 * 1)	the caller must be the owner of the object (or superuser)
 * 2)	the object must be a character (or mpx) device
 * 3)	the device must have a d_revoke() device routine
 * returns
 *	!0	failure, return value is errno
 *	0	success
 */

static
check(struct vnode *vp, struct vattr *va, struct ucred *crp)
{
	int rc, status;

	/*
	 * do a stat on this vnode to get:
	 *	owner ID
	 *	type (want IFCHR) - could just check v_type for VCHR/VMPC
	 *	dev_t
	 *	chan
	 */
	if (rc = VNOP_GETATTR(vp, va, crp))
		return rc;

	/* Is the current process the owner or has BYPASS_DAC_KILL privilege? */
	if ((va->va_uid != getuidx(ID_EFFECTIVE)) && 
	    (privcheck(BYPASS_DAC_KILL)))
		return EPERM;

	/*
	 * At present, only character devices are supported.
	 * In future, will need a more general way to determine
	 * whether object supports revocation ...
	 */
	if ((vp->v_type != VCHR) && (vp->v_type != VMPC))
		return EINVAL;

	/* Is this a revokable device? */

	if ( rc = devswqry(va->va_rdev, (uint *)&status, NULL))
		return rc;

	if (!(status & DSW_TCPATH))
		return EINVAL;

	return 0;
}


/*
 * NAME: dup
 *
 * FUNCTION: allocate a new open file table entry to REPLACE
 *           the specified open file table entry;
 *           the reference count of the old entry is decremented
 *
 * RETURNS:	new fp on success
 *		NULL on failure
 */
static
dup(struct file	*fp, struct file **nfpp, struct ucred *crp)
{
	struct vnode	*vp;
	int	rc;
	long	flag;

	FP_LOCK(fp);
	flag = fp->f_flag;
	FP_UNLOCK(fp);

	vp = fp->f_vnode;

	/* allocate a new file struct and attach the same vnode */

	VNOP_HOLD(vp);
	if (rc = fpalloc(vp, flag, fp->f_type, fp->f_ops, nfpp))
	{
		VNOP_RELE(vp);
		return rc;
	}

	FP_LOCK(fp);
	
	if ( fp->f_count == 1 )
	{
		FP_UNLOCK(fp);
		/*
		 * The count of the old file table entry has fallen to 
		 * 1, meaning that the file table entry is no longer
		 * shared
		 * Free the new entry that has been allocated 
		 */
		fpfree(*nfpp);
		VNOP_RELE(vp);
		*nfpp = fp;
		return(0);		

	}
	CRED_LOCK();
	fp->f_cred->cr_ref++;
	CRED_UNLOCK();
	(*nfpp)->f_cred = fp->f_cred;

	/*
	* In order to maintain the reference counts properly, we must
	* decrement the count on the old fp, set the count on the new
	* fp to 1 (done by fpalloc), and re-open the file.  It's tempting
	* to say that just calling VNOP_HOLD would be sufficient here,
	* however, we have created a new open instance rather than just
	* held the vnode.
	*/
	if (rc = VNOP_OPEN(vp,fp->f_flag,0,&(*nfpp)->f_vinfo,crp))
	{
		VNOP_RELE(vp);
		(*nfpp)->f_count = 0;
		FP_UNLOCK(fp);

		crfree((*nfpp)->f_cred);

		fpfree(*nfpp);
		return rc;
	}
	fp->f_count--;
	FP_UNLOCK(fp);
	return 0;
}


/*
 * invalidate all open file table entries which refer to the specified object.
 * if the object is a device, <dev> and <chan> are the device;
 * otherwise (in the future), a gnode match will be sought.
 * if <rfp> is not NULL, this is an FREVOKE (and <rfp> is the open file table
 * entry of the caller.
 *
 * RETURNS:  number of open file structs associated with this vnode
 */

static
invalidate(struct file	*rfp,
	   struct vnode	*rvp,
	   struct vattr	*rva)
{
	struct file	*fp;
	struct vnode	*vp;
	struct vattr	va;
	int		rc = 0;
	struct inval_args inval_check; 

	/*
	 * Put the checking parameters in this structure and send them over
	 * to ftsearch, ultimately invalidate_fp needs them
	 */
	inval_check.rfp = rfp;
	inval_check.rva = rva;

	inval_check.st = 0; /* Initially status is 0 */

	ftsearch ( invalidate_fp,  DTYPE_VNODE, &inval_check);
	
	/*
	 * Return number of objects revoked
	 */
	return(inval_check.st);
}


/*
 * This function is called by ftsearch with the file struct locked.
 *
 * RETURNS:  Always returns 0 so that the file table is searched
 *           completely.
 */

int invalidate_fp(struct file *fp, struct inval_args *inval_check)
{
        struct vnode    *vp;
        struct vattr    va;
	struct ucred	*crp;
	int	rc;


	/* Is this the file entry we want to protect (frevoke only)? */
        if ((inval_check->rfp) && ((inval_check->rfp) == fp))
       		 return(0);

	/*
	 * Revocation is supported only on character devices
	 */
        vp = fp->f_vnode;
        if ((vp->v_type != VCHR) && (vp->v_type != VMPC))
		return(0);

	/* Ignore things that are broken */
	crp = crref();
        rc = VNOP_GETATTR(vp, &va, crp);
	crfree(crp);
        if (rc)
        	return(0);

	/* Is it the same device number ? */
        if ((inval_check->rva->va_rdev == va.va_rdev) &&
        /* and if it is a multiplexed device, the same channel? */
             /* if chan == -1 then disregard the new channel */
             ((inval_check->rva->va_chan == -1) ||
             (inval_check->rva->va_chan == va.va_chan)))
                /* then do the revocation */
        {
		inval_check->st ++;
        	fp->f_flag |= FREVOKED;
        }
	return(0);
}


/*
 * fp_revoke()
 *
 * this is needed for the console driver redirection
 * 
 * INPUT:    fp      file pointer
 *           flag    flag argument to VNOP_REVOKE()
 *
 * RETURNS:  return from VNOP_REVOKE()
 */

fp_revoke(struct file *fp, int flag)
{
	struct ucred *crp;

	crp = crref();
	u.u_error = VNOP_REVOKE(fp->f_vnode, flag, 
			        fp->f_flag, (struct vattr*)&(fp->f_vinfo), crp);
	crfree(crp);
	return(u.u_error);
}
