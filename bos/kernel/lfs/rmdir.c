static char sccsid[] = "@(#)61	1.12.2.4  src/bos/kernel/lfs/rmdir.c, syslfs, bos411, 9428A410j 6/3/94 16:07:22";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: rmdir
 *
 * ORIGINS: 26, 27
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
 *
 */


#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/fs_locks.h"
#include "sys/syspest.h"
#include "sys/pathname.h"
#include "sys/audit.h"
#include "sys/malloc.h"

BUGXDEF(dbdir);

/********************************************************************
 *
 * NAME:  rmdir system call
 *
 * FUNCTION:	removes a directory
 *
 * PARAMETERS:	path - name of directory to remove
 *
 * RETURNS:	returns zero upon successful removal, otherwise
 *		returns error code of EBUSY if the directory is
 *		in use as either the mount point for a file system
 *		or the current directory of the process that 
 *		issued the rmdir. returns error code ENOTDIR if
 *		the named path is not a directory.
 *
 ********************************************************************/

rmdir(path)
char    *path;
{
	struct vnode *vp, *dvp;
	struct pathname pn;
	static int svcnum=0;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	static int tcbmod = 0;
	struct ucred *crp;
	int rc;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Get the current user credentials */
	crp = crref();

        if ((audit_flag) && (audit_svcstart("FS_Rmdir", &svcnum, 0)))
        {
                if (path)
                {
                        char *ptr;
                        int len;

                        if ((ptr = malloc(MAXPATHLEN)) == NULL)
                        {
                                rc = ENOMEM;
                                goto out2;
                        }
                        if (copyinstr (path, ptr, MAXPATHLEN, &len))
                        {
                                rc = EFAULT;
                                free(ptr);
                                goto out2;
                        }

                        audit_svcbcopy(ptr, len);
                        free(ptr);
                }
                audit_svcfinis();
        }

	if (rc = pn_get(path, USR, &pn)) {
		goto out2;
	}

	if (rc = lookuppn(&pn, L_DEL | L_NOFOLLOW, &dvp, &vp, crp)) {
		pn_free(&pn);
		goto out2;
	}
	/*
	 *   Check if its a directory
	 */
	if( vp->v_vntype != VDIR ){
		rc = ENOTDIR;
		goto out;
	}

	/*
	* If the directory to be removed is a mount point and
	* the parent directory is in a read-only file system,
	* the svvs expects EROFS instead of EBUSY.  Lookup
	* fails to check for EROFS because it checks the mounted 
	* over directory, not the stub.
	*/
	if (dvp && dvp->v_vfsp->vfs_flag & VFS_READONLY) {
		rc = EROFS;
		goto out;
	}

	if( vp == U.U_cdir ){
		rc = EBUSY;
		goto out;
	}

	if(vp->v_flag & V_ROOT){
		rc = EBUSY;
		goto out;
	}

	rc = VNOP_RMDIR(vp, dvp, pn.pn_path, crp);

	if (rc == 0) {

		/* 
		 * Success TCB_Mod has occured 
		 */

		if ((audit_flag) && (vp->v_gnode->gn_flags & GNF_TCB))
		{
			if (audit_svcstart("TCB_Mod", &tcbmod, 0))
				audit_svcfinis();
		}
	}

out:
	VNOP_RELE(vp);
	if (dvp)	/* "path" could have been '/' */
		VNOP_RELE(dvp);
	pn_free(&pn);

out2:
	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
