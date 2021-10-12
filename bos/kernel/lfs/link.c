static char sccsid[] = "@(#)48	1.23.1.4  src/bos/kernel/lfs/link.c, syslfs, bos411, 9428A410j 12/15/93 14:48:07";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: link
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
 *
 */

#include "sys/systm.h"
#include "sys/user.h"
#include "sys/fs_locks.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/pathname.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/priv.h"

BUGVDEF(dblink, 0);

/***********************************************************************
 *
 * NAME:  link()           (system call entry point)
 *
 * FUNCTION:	link creates a new entry for an existing file in a
 *		directory.
 *
 * PARAMETERS:	target - The name of the file this will be a link of.
 *		linkname - The new name the link will have.
 *
 * RETURNS:	EEXIST	- if the link already exists.
 *		EPERM	- if the file named by the target is a directory
 *			  and the effective user ID is not super-user.
 *		EXDEV	- if the linkname and the target name are on
 *			  different file systems.
 *
 ************************************************************************/

link(target, linkname)
char	*target;
char	*linkname;
{
	struct vnode *tvp, *plvp;
	struct vnode *lvp = (struct vnode *)NULL;
	struct pathname lpn;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	static int svcnum = 0;
	static int tcbmod = 0;
	int rc;
	struct ucred *crp = crref();

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FILE_Link", &svcnum, 0)))
	{

                if(target){
                        char *ptr;
			int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                                rc = ENOMEM;
                                goto out;
                        }
                        else if(copyinstr(target, ptr, MAXPATHLEN, &len)){
                                rc = EFAULT;
                                free(ptr);
                                goto out;
                        }
                        else
                        	audit_svcbcopy(ptr, len);
                        free(ptr);
                }

                if(linkname){
                        char *ptr;
			int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                                rc = ENOMEM;
                                goto out;
                        }
                        else if(copyinstr(linkname, ptr, MAXPATHLEN, &len)){
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

	if (rc = lookupname(target, USR, L_SEARCH, NULL, &tvp, crp))
		goto out;

	if (tvp->v_vntype == VDIR && !priv_req(FS_CONFIG)) {
		VNOP_RELE(tvp);
		rc = EPERM;
		goto out;
	}

	if (rc = pn_get(linkname, USR, &lpn)) {
		VNOP_RELE(tvp);
		goto out;
	}

	rc = lookuppn(&lpn, L_CREATE|L_NOFOLLOW, &plvp, &lvp, crp);

	if (lvp != NULL) {
		pn_free(&lpn);
		VNOP_RELE(lvp);
		VNOP_RELE(plvp);
		VNOP_RELE(tvp);
		rc = EEXIST;
		goto out;
	}

	if (rc) {
		VNOP_RELE(tvp);
		pn_free(&lpn);
		goto out;
	}

	(void)specchk_link(&tvp);

	if (tvp->v_vfsp != plvp->v_vfsp) {
		VNOP_RELE(plvp);
		VNOP_RELE(tvp);
		pn_free(&lpn);
		rc = EXDEV;
		goto out;
	}

	rc = VNOP_LINK(tvp, plvp, lpn.pn_path, crp);

	/* 
	 * If no error then a TCB_Mod has occured
 	 */

	if ((rc == 0)
	    && (audit_flag)
	    && (tvp->v_gnode->gn_flags & GNF_TCB))
	{
		if (audit_svcstart("TCB_Mod", &tcbmod, 0))
               		audit_svcfinis();
	}

	VNOP_RELE(tvp);
	VNOP_RELE(plvp);
	pn_free(&lpn);

out:
	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	crfree(crp);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
