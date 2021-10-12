static char sccsid[] = "@(#)37	1.19.1.5  src/bos/kernel/lfs/chownx.c, syslfs, bos411, 9428A410j 12/15/93 14:45:11";

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS: chownx, fchownx
 *
 * ORIGINS: 27, 3, 26
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

#include "sys/user.h"
#include "sys/fs_locks.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/errno.h"
#include "sys/lockl.h"
#include "sys/trchkid.h"
#include "sys/chownx.h"

/*
 * NAME: chownx()
 *
 * FUNCTION: Change the owner or group of the named file.  Control is allowed
 *           over how the translation of the user and group ID's are done.
 *
 * PARAMETERS: fname, uid, gid and tflag.  fname is a file name, uid and gid
 *             are the new user and group id's and tflag is an integer 
 *             indicating how to do the translation.
 *
 * RETURN VALUES: explicitly none, implicitly u.u_error gets set.
 */
chownx(fname, uid, gid, tflag)
char	*fname;
uid_t	uid;
gid_t	gid;
int	tflag;
{
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	static int svcnum = 0;
	uid_t	luid;
	gid_t	lgid;

	TRCHKL2T(HKWD_SYSC_LFS | hkwd_SYSC_CHOWNX, uid, gid);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	luid = (tflag & T_OWNER_AS_IS) ? -1 : uid;
	lgid = (tflag & T_GROUP_AS_IS) ? -1 : gid;

	if ((audit_flag)
	    && (audit_svcstart("FILE_Owner", &svcnum, 2, luid, lgid)))
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

	rc = setnameattr(fname, V_OWN, tflag, (int) uid, (int) gid, 0);

	/* Unlock the kernel lock unless nested locks occurred */
fail:
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}

/*
 * NAME: fchownx()
 *
 * FUNCTION: Change the owner and/or group of the file refered to by 
 *	 	descriptor.
 *
 * PARAMETERS: fd, uid, gid and tflag.  fd is a file descriptor, uid and gid
 *             are the new user and group id's and tflag is a flag word
 *             indicating how to do the translation.
 *
 * RETURN VALUES: explicitly none, implicitly u.u_error gets set.
 */

fchownx(fdes, uid, gid, tflag)
int	fdes;
uid_t	uid;
gid_t	gid;
int	tflag;
{
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	struct ucred *crp;
	int rc;
	static int svcnum=0;
	uid_t	luid;
	gid_t	lgid;

	TRCHKL2T(HKWD_SYSC_LFS | hkwd_SYSC_FCHOWNX, uid, gid);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	luid = (tflag & T_OWNER_AS_IS) ? -1 : uid;
	lgid = (tflag & T_GROUP_AS_IS) ? -1 : gid;

	if ((audit_flag)
	    && (audit_svcstart("FILE_Owner", &svcnum, 2, luid, lgid)))
		audit_svcfinis();

	if ((rc = getft(fdes, &fp, DTYPE_VNODE)) == 0)
	{
		/* get creds for the operation */
		crp = crref();

		rc = vsetattr(fp->f_vnode, V_OWN, tflag,
				(int) uid, (int) gid, crp);

		crfree(crp);
		ufdrele(fdes);
	}

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}
