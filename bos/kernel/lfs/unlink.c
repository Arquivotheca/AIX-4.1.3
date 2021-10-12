static char sccsid[] = "@(#)69	1.21.1.4  src/bos/kernel/lfs/unlink.c, syslfs, bos411, 9428A410j 12/15/93 14:45:50";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: unlink
 *
 * ORIGINS: 3, 27
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

#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/user.h"
#include "sys/fs_locks.h"
#include "sys/errno.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/pathname.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/priv.h"

BUGVDEF(dbremove, 0);

/******************************************************************
 *
 * NAME:  unlink()       (system call entry point)
 *
 * FUNCTION:	removes an entry from a directory, if this is the
 *		last link and the file is not open the space for the
 *		file is released.
 *
 * PARAMETERS:	fname - pathname of file to unlink
 *
 * RETURNS:	if an error occurs then u.u_error is set to EBUSY if 
 *		the entry to be unlinked is the mount point for a
 *		mounted file system.
 *
 *****************************************************************/	

unlink(fname)
char    *fname;
{
	struct vnode *dvp, *vp;
	struct pathname pn;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	struct ucred *crp;
	static int svcnum = 0;
	static int tcbmod = 0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Get current user credentials */
	crp = crref();

	if ((audit_flag) && (audit_svcstart("FILE_Unlink", &svcnum, 0)))
	{
                if(fname){
                        char *ptr;
			int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                                rc = ENOMEM;
                                goto out;
                        }
                        else if(copyinstr(fname, ptr, MAXPATHLEN, &len)){
                                rc = EFAULT;
                                free(ptr);
                                goto out;
                        }
                        else
                        	audit_svcbcopy(ptr, len);
                        free(ptr);
                }
		audit_svcfinis();
	}

	BUGLPR(dbremove, BUGNTF, ("unlink(nm=%s)\n", fname));

	if (rc = pn_get(fname, USR, &pn)) {
		goto out;
	}

	rc = lookuppn(&pn, L_DEL | L_NOFOLLOW, &dvp, &vp, crp);


	if (rc) {
		pn_free(&pn);
		goto out;
	}

	if (vp->v_flag & V_ROOT){
		rc = EBUSY;
		pn_free(&pn);
		VNOP_RELE(vp);
		if (dvp)        /* dvp == NULL for unlink("/"); */
			VNOP_RELE(dvp);

		goto out;
	}


	if ((vp->v_vntype != VDIR) || priv_req(FS_CONFIG))
		rc = VNOP_REMOVE(vp, dvp, pn.pn_path, crp);
	else
		rc = EPERM;

	/* 
	 * Success? TCB_Mod has occurred 
	 */

	if ((rc == 0)
	    && (audit_flag)
	    && (vp->v_gnode->gn_flags & GNF_TCB))
	{
                if(audit_svcstart("TCB_Mod", &tcbmod, 0))
                        audit_svcfinis();
	}

	pn_free(&pn);
	VNOP_RELE(vp);
	VNOP_RELE(dvp);

out:
	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
