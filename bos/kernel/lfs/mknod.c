static char sccsid[] = "@(#)53	1.19.1.6  src/bos/kernel/lfs/mknod.c, syslfs, bos411, 9428A410j 4/25/94 13:50:44";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: mknod
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
#include "sys/errno.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/pathname.h"
#include "sys/mode.h"
#include "sys/lockl.h"
#include "sys/trchkid.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/fs_locks.h"

BUGVDEF(dbmknod, 0);

/*
 *
 * NAME:  mknod()		(system call entry point)
 *
 * FUNCTION:	creates regular or special files and directories. If
 *		a directory is requested it does what mkdir would do.
 *
 * PARAMETERS:	fname - the name of the file
 *		mode  - the new mode of the file, and the high order
 *			bits contain the type of the file.
 *		dev   - contains the device number if this is a special
 *			file.
 * 
 * RETURNS:	returns EPERM if the process's effective user ID is
 *		not super-user. returns EEXIST if the named file exists,
 *		otherwise it returns zero indicating success.
 *
 */
mknod(fname, fmode, dev)
char	*fname;
int	fmode;
dev_t	dev;
{
	struct vnode *vp, *dvp;
	struct pathname pn;
	int rc;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int svcnum = 0;
	struct ucred *crp = crref();

	TRCHKL1T(HKWD_SYSC_LFS | hkwd_SYSC_MKNOD, fmode);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag)
	    && (audit_svcstart("DEV_Create", &svcnum, 2, fmode, dev)))
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

	BUGLPR(dbmknod, BUGNTF, ("mknod(name=%s, mode=%x, dev=%x)\n",
	    fname, fmode, dev))

	if ((fmode & S_IFMT) != S_IFIFO)
		if (rc = privcheck_cr(DEV_CONFIG, crp))
			goto out;

	if (rc = pn_get(fname, USR, &pn))
		goto out;

	if ((rc = lookuppn(&pn, L_CREATE | L_NOFOLLOW, &dvp, &vp, crp)) == 0)
	{
		if ( vp != NULL )
		{
			VNOP_RELE(vp);
			rc = EEXIST;
		}
		else
		{
			/* apply the umask to the mode */
			fmode &= ~U.U_cmask;
			rc = VNOP_MKNOD(dvp, pn.pn_path, fmode, dev, crp);
		}

		VNOP_RELE(dvp);
	}

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
