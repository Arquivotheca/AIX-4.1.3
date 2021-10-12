static char sccsid[] = "@(#)36	1.18.1.9  src/bos/kernel/lfs/chdir.c, syslfs, bos411, 9428A410j 6/3/94 16:06:56";
/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS: chdir, chroot, chdirec, fchdir
 *
 * ORIGINS: 27, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/vnode.h"
#include "sys/access.h"
#include "sys/syspest.h"
#include "sys/priv.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/thread.h"

BUGVDEF (chrootbug,0);
/**************************************************************
 *
 * NAME:  chdir()        (system call entry point)
 *
 * FUNCTION:  changes the current working directory for a process
 *		It does this by setting the u_cdir value in the
 *		u structure. It has code in common with the
 *		"chroot" system call.
 *
 * PARAMETERS:	fname - the name of the new directory
 *
 * RETURNS:	Besides possible error values being returned
 *		in u.u_error, the U.U_cdir element will be set
 *		to a held vnode for the directory "fname" if
 *		there was not an error.
 *
 * SERIALIZATION: Take the U_FSO_LOCK when setting U.U_cdir.
 *
 **************************************************************/

chdir(fname)
char	*fname;
{
	struct vnode *vp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	static int svcnum = 0;
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FS_Chdir", &svcnum, 0)))
	{
		if(fname){
			char *ptr;
			int len;
			
			if((ptr = malloc(MAXPATHLEN)) == NULL){
				rc = ENOMEM;
				goto fail;
			}
			else if(copyinstr(fname, ptr, MAXPATHLEN, &len)){
				rc = EFAULT;
				free(ptr);
				goto fail;
			}
			else
				audit_svcbcopy(ptr, len);
			free(ptr);
		}
		audit_svcfinis();
	}

	rc = chdirec(fname, &vp);

	if (rc == 0) {
		struct vnode *oldvp;
		int tlock;	/* is multi-thread locking required? */

		if (tlock = (U.U_procp->p_active > 1))
			U_FSO_LOCK();
		oldvp = U.U_cdir;
		U.U_cdir = vp;
		if (tlock)
			U_FSO_UNLOCK();
		if (oldvp)	/* kprocs might not have any */
			VNOP_RELE(oldvp);
	}

fail:

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}


/**************************************************************
 *
 * NAME:  chroot()        (system call entry point)
 *
 * FUNCTION:  changes the root directory for a process.
 *		It does this by setting the u_rdir value in the
 *		u structure. It has code in common with the
 *		"chdir" system call.
 *
 * PARAMETERS:	fname - the name of the new directory
 *
 * RETURNS:	Besides possible error values being returned
 *		in u.u_error, the U.U_rdir element will be set
 *		to a held vnode for the directory "fname" if
 *		there was not an error.
 *
 * SERIALIZATION: Take the U_FSO_LOCK when setting U.U_rdir.
 *
**************************************************************/

chroot(fname)
char	*fname;
{
	struct vnode *vp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	static int svcnum = 0;
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FS_Chroot", &svcnum, 0)))
	{
		if(fname){
			char *ptr;
			int len;
			
			if((ptr = malloc(MAXPATHLEN)) == NULL){
				rc = ENOMEM;
				goto fail;
			}
			else if(copyinstr(fname, ptr, MAXPATHLEN, &len)){
				rc = EFAULT;
				free(ptr);
				goto fail;
			}
			else
				audit_svcbcopy(ptr, len);
			free(ptr);
		}
		audit_svcfinis();
	}

	if (priv_req(FS_CONFIG))
		rc = chdirec(fname, &vp);
	else
		rc = EPERM;

	if (rc == 0) {
		struct vnode *oldvp;
		int tlock;	/* is multi-thread locking required? */

		if (tlock = (U.U_procp->p_active > 1))
			U_FSO_LOCK();
		oldvp = U.U_rdir;
		U.U_rdir = vp;
		if (tlock)
			U_FSO_UNLOCK();
		if (oldvp)
			VNOP_RELE(oldvp);
	}

fail:
	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}


/**************************************************************
 *
 * NAME:  chdirec(pathname, vpp)
 *
 * FUNCTION: 	The chdirec routine is shared by the chdir and
 *		chroot system calls.  It checks the pathname and	
 *		returns a vnode pointing to it.	
 *	
 * PARAMETERS:	pathname - the name of the new directory
 *		**vpp    - return parameter vnode pointer
 *
 * RETURNS:	If there are no errors the vpp pointer will be set
 *		to the vnode corresponding to pathname and a zero is	
 *		returned indicating success. If an error occurs then
 *		the error is returned which will end up setting 
 *		u.u_error back in chdir or chroot. The only error
 *		this routine returns itself is ENOTDIR, other errors
 *		may be returned  and passed on through as a result
 *		of problems encountered in lookuppn
 *	
**************************************************************/

chdirec(pathname, vpp)
char *pathname;
register struct vnode **vpp;
{
	register int rc;
	struct ucred *crp = crref();

	if ((rc = lookupname(pathname, USR, L_SEARCH, NULL, vpp, crp)) == 0)
	{
		if ((*vpp)->v_vntype == VDIR)
			rc = VNOP_ACCESS(*vpp, X_ACC, NULL, crp);
		else
			rc = ENOTDIR;
		if (rc)
			VNOP_RELE(*vpp);

	}
	crfree(crp);
	return(rc);
}

/*
 *
 * NAME:  fchdir()        (system call entry point)
 *
 * FUNCTION:  changes the current working directory for a process
 *		It does this by setting the u_cdir value in the
 *		u structure.
 *
 * PARAMETERS:	fd - open file descriptor of new directory
 *
 * RETURNS:	Besides possible error values being returned
 *		in u.u_error, the U.U_cdir element will be set
 *		to a held vnode for the directory "fd" if
 *		there was not an error.
 *
 * SERIALIZATION: Take the U_FSO_LOCK when setting U.U_cdir.
 *
 */
fchdir(int fd)
{
	struct vnode *vp;
	struct file  *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	struct ucred *crp;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Get the current user credentials */
	crp = crref();

        if (!(rc = getft(fd, &fp, DTYPE_VNODE)))
        {
        	vp = (struct vnode *)fp->f_vnode;

		if (vp->v_vntype != VDIR) 
			rc = ENOTDIR;
		else 
			rc = VNOP_ACCESS(vp, X_ACC, NULL, crp);

		if(rc == 0) 
		{
			struct vnode *oldvp;
			int tlock;	/* is multi-thread locking required? */

			if (tlock = (U.U_procp->p_active > 1))
				U_FSO_LOCK();
			oldvp = U.U_cdir;
			U.U_cdir = vp;
			VNOP_HOLD(U.U_cdir);  /* Hold vnode in case fp closes */
			if (tlock)
				U_FSO_UNLOCK();
			if (oldvp)
				VNOP_RELE(oldvp);
		}

		ufdrele(fd);
	}
	
	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	
	return rc ? -1 : 0;
}
