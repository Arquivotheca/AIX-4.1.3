static char sccsid[] = "@(#)52	1.10.1.4  src/bos/kernel/lfs/mkdir.c, syslfs, bos411, 9428A410j 6/3/94 16:07:34";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: mkdir
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
#include "sys/fs_locks.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/pathname.h"
#include "sys/audit.h"
#include "sys/malloc.h"

BUGVDEF(dbdir, 0);

/*
 *
 * NAME:  mkdir()         (system call entry point)
 *
 * FUNCTION:	creates a new directory with the given name.
 *
 * PARAMETERS:	path - is the name of the new directory.
 *		mode - is the desired mode.
 *
 * RETURNS:	returns with u.u_error set to EEXIST if the named
 *		file already exists.
 *
 */

mkdir(path, mode)
char	*path;
int	mode;
{
        static int svcnum = 0;
	struct vnode *vp = (struct vnode *)NULL;
	struct vnode *dvp;
	struct pathname pn;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	struct ucred *crp;
	
	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

        /* Get the current user credentials */
        crp = crref();

        if ((audit_flag) && (audit_svcstart("FS_Mkdir", &svcnum, 0)))
        {
                if (path)
                {
                        char *ptr;
                        int len;

                        if ((ptr = malloc(MAXPATHLEN)) == NULL)
                        {
                                rc = ENOMEM;
                                goto out;
                        }
                        if (copyinstr (path, ptr, MAXPATHLEN, &len))
                        {
                                rc = EFAULT;
                                free(ptr);
                                goto out;
                        }

                        audit_svcbcopy(ptr, len);
                        free(ptr);
		}
                audit_svcfinis();
	}

	BUGLPR(dbdir, BUGNTF, ("mkdir:(path=%s,mode=%o)\n",
		   path,mode))

	if (rc = pn_get(path, USR, &pn))
		goto out;

	if (rc = lookuppn(&pn, L_CREATE | L_NOFOLLOW, &dvp, &vp, crp)) {
		pn_free(&pn);
		goto out;
	}

	/* If we actually got a vnode, then the directory already
	   existed so we return an error */

	if ( vp != NULL ) {
		rc = EEXIST;
		VNOP_RELE(vp);
	} 
	else {
		/* apply the umask to the mode */	
		mode &= ~U.U_cmask;
		rc = VNOP_MKDIR(dvp, pn.pn_path, mode & 0777, crp);
	}

	VNOP_RELE(dvp);
	pn_free(&pn);

out:
	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}


