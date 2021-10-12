static char sccsid[] = "@(#)24        1.9.1.5  src/bos/kernel/s/dac/accessx.c, syssdac, bos411, 9428A410j 3/28/94 02:59:17";

/*
 * COMPONENT_NAME:  TCBACL
 *
 * FUNCTIONS:  accessx(), faccessx(), fp_accessx()
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

#include	<sys/access.h>
#include	<sys/errno.h>
#include	<sys/priv.h>
#include	<sys/user.h>
#include	<sys/vattr.h>
#include	<sys/vnode.h>
#include	<sys/file.h>
#include	<sys/fullstat.h>
#include	<sys/systm.h>
#include	<sys/lockl.h>
#include	"../auth/crlock.h"

extern  Simple_lock cred_lock;

/*
 * NAME: accessx()    (system call entry point)
 * 
 * FUNCTION: Determines access rights to a file.
 *
 * RETURN VALUES: Upon successful completion, the accessx() system call
 *	returns 0.  If the accessx() system call fails, a value of -1 is 
 *	returned and errno is set to indicate the error.
 */
int
accessx(path, mode, who)
char	*path;
int	mode;
int	who;
{
	struct vnode	*vp;
	struct ucred	*crp;
	int	rc;

	if ((mode & ~(R_ACC | W_ACC | X_ACC | E_ACC))	||
	    ((who != ACC_SELF)		&&
	     (who != ACC_INVOKER)	&&
	     (who != ACC_OTHERS)	&&
	     (who != ACC_ANY)		&&
	     (who != ACC_ALL)))
	{
		u.u_error = EINVAL;
		return(-1);
	}

	if (who == ACC_INVOKER)
	{
		CRED_LOCK();
		crp = (struct ucred *)_crdup(U.U_cred);	
		CRED_UNLOCK();
		crp->cr_uid = crp->cr_ruid;
		crp->cr_gid = crp->cr_rgid;
		crp->cr_epriv = crp->cr_ipriv;

		who = ACC_SELF;
	} else

		/*
		 * Increment ref. count so that no other thread frees
		 * cred during call to VNOP_ACCESS. Also copy U.U_cred
		 * pointer into crp. The whole operation is done atomically
		 */
		crp = crref();

        if (u.u_error = lookupname(path, USR,
     	 	                   (mode & W_ACC) ? L_SEARCH|L_EROFS : L_SEARCH,
                                   NULL, &vp, crp))
        	goto out;

        /* we know it exists... */
        if (mode == E_ACC)
        {
	         VNOP_RELE(vp);
       		 goto out;
	}

	u.u_error = VNOP_ACCESS(vp, mode, who, crp);
	VNOP_RELE(vp);

out:
        crfree(crp);

	return(u.u_error ? -1 : 0);
}


/*
 * NAME: faccessx()    (system call entry point)
 * 
 * FUNCTION:	Determines access rights to a file given a file descriptor.
 *
 * RETURN VALUES: Upon successful completion, the faccessx() system call
 *	returns 0.  If the faccessx() system call fails, a value of -1 is 
 *	returned and errno is set to indicate the error.
 */
int
faccessx(fildes, mode, who)
int	fildes;
int	mode;
int	who;
{
	struct vnode	*vp;
	struct	file	*fp;	/* file pointer from fildes */
	struct	ucred	*crp;

	if ((mode & ~(R_ACC | W_ACC | X_ACC | E_ACC))	||
	    ((who != ACC_SELF)		&&
	     (who != ACC_INVOKER)	&&
	     (who != ACC_OTHERS)	&&
	     (who != ACC_ANY)		&&
	     (who != ACC_ALL)))
	{
		u.u_error = EINVAL;
		return(-1);
	}

	/* turn the file descriptor into a file pointer */
	if (u.u_error = getft(fildes, &fp, DTYPE_VNODE))
		goto out;

	/* we know it exists... */
	if (mode == E_ACC)
		goto out1;

	/* turn the file pointer into a vnode pointer */
	vp = fp->f_vnode;

	if (who == ACC_INVOKER)
	{
		CRED_LOCK();
		crp = (struct ucred *)_crdup(U.U_cred);	
		CRED_UNLOCK();
		crp->cr_uid = crp->cr_ruid;
		crp->cr_gid = crp->cr_rgid;
		crp->cr_epriv = crp->cr_ipriv;
		who = ACC_SELF;
	} else
		/*
		 * Increment ref. count so that no other thread frees
		 * cred during call to VNOP_ACCESS. Also copy U.U_cred
		 * pointer into crp. The whole operation is done atomically
		 */
		crp = crref();

	u.u_error = VNOP_ACCESS(vp, mode, who, crp);

	crfree(crp);

out1:
	/* Decrement the file descriptor count */
	ufdrele(fildes);
out:
	return(u.u_error ? -1 : 0);
}


/*
 * NAME: fp_accessx()    (kernel service entry point)
 * 
 * FUNCTION:	Determines access rights to a file given a file pointer.
 *
 * RETURN VALUES: Upon successful completion, the fp_accessx() kernel service
 *	returns 0.  If the fp_accessx() kernel service fails, the error number
 *	is returned.
 */
int
fp_accessx(fp, mode, who)
struct file	*fp;
int	mode;
int	who;
{
	int	rc;
	struct vnode	*vp;
	struct ucred	*crp;

	if ((mode & ~(R_ACC | W_ACC | X_ACC | E_ACC))	||
	    ((who != ACC_SELF)		&&
	     (who != ACC_INVOKER)	&&
	     (who != ACC_OTHERS)	&&
	     (who != ACC_ANY)		&&
	     (who != ACC_ALL)))
	{
		return(EINVAL);
	}

	/* we know it exists... */
	if (mode == E_ACC)
		return(0);

	/* turn the file pointer into a vnode pointer */
	vp = fp->f_vnode;

	if (who == ACC_INVOKER)
	{
		CRED_LOCK();
		crp = (struct ucred *)_crdup(U.U_cred);	
		CRED_UNLOCK();
		crp->cr_uid = crp->cr_ruid;
		crp->cr_gid = crp->cr_rgid;
		crp->cr_epriv = crp->cr_ipriv;
		who = ACC_SELF;
	} else
		/*
		 * Increment ref. count so that no other thread frees
		 * cred during call to VNOP_ACCESS. Also copy U.U_cred
		 * pointer into crp. The whole operation is done atomically
		 */
		crp = crref();
		
	rc = VNOP_ACCESS(vp, mode, who, crp);

        crfree(crp);

out:
	return(rc);
}
