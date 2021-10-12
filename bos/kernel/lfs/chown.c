static char sccsid[] = "@(#)02	1.12.1.5  src/bos/kernel/lfs/chown.c, syslfs, bos411, 9428A410j 12/15/93 14:48:02";

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS: _chown, chown, lchown, fchown
 *
 * ORIGINS: 27, 3, 26
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/fs_locks.h"
#include "sys/vnode.h"
#include "sys/chownx.h"
#include "sys/vattr.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/trchkid.h"

/*
 * NAME: _chown()    (Common routine)
 * 
 * FUNCTION: _chown is the common routine called by chown() and lchown()
 *	     system calls. This routine will change the owner or group of
 *	     the named file.
 *
 * CALLERS: chown()
 *	    lchown()
 *
 * PARAMETERS:	fname	file name to change the ownership of.
 *		uid	user id to se the ownership to.
 *		gid	group id to set the gid attribute to.
 *		lflag	lookup flag
 *
 * RETURN VALUES: explicitly none, implicitly sets u.u_error
 */
int
_chown(fname, uid, gid, lflag)
char	*fname;
uid_t	uid;
gid_t	gid;
unsigned int	lflag;
{
	register int flags;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	static int svcnum = 0;
	int rc;

	TRCHKL2T(HKWD_SYSC_LFS | hkwd_SYSC_CHOWN, uid, gid);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag)
	    && (audit_svcstart("FILE_Owner", &svcnum, 2, uid, gid)))
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

	/* take the parameters and call setnameattr */

	flags = 0;

	/* BSD compatibility */
	if (uid == -1)
		flags |= T_OWNER_AS_IS;
	if (gid == -1)
		flags |= T_GROUP_AS_IS;

	/* set the new usr and group id's */
	rc = setnameattr(fname, V_OWN, flags, (int) uid, (int) gid, lflag);

	/* Unlock the kernel lock unless nested locks occurred */
fail:
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : 0;
}

/*
 * NAME: chown()    (system call entry point)
 *
 * FUNCTION: chown changes the owner or group of the named file
 *
 * PARAMETERS: fname, uid, gid.  Filename to change and the new userid and
 *             group id.
 *
 * RETURN VALUES: explicitly none, implicitly sets u.u_error
 */
int
chown(fname, uid, gid)
char	*fname;
uid_t	uid;
gid_t	gid;
{
	return _chown(fname, uid, gid, 0);
}

/*
 * NAME: lchown()    (system call entry point)
 *
 * FUNCTION: lchown changes the owner or group of a named file, even if
 * 	     if that named file resolves to a symbolic link. This is
 *	     possible by passing the L_NOFOLLOW flag to chown_comm().
 *
 * PARAMETERS: fname, uid, gid.  Filename to change and the new userid and
 *             group id.
 *
 * RETURN VALUES: explicitly none, implicitly sets u.u_error
 */
int
lchown(fname, uid, gid)
char	*fname;
uid_t	uid;
gid_t	gid;
{
	return _chown(fname, uid, gid, L_NOFOLLOW);
}

/*
 * NAME: fchown()    (system call entry point)
 *
 * FUNCTION: change the owner and group on an open file associated with the 
 *           provided file descriptor.
 * 
 * PARAMETERS: fdes, uid, gid. fdes is the open file descriptor, uid and gid
 *             are new values for the userid and groupid of the file.
 *
 * RETURN VALUES: explicitly none, impicitly u.u_error is set
 */
int
fchown(fdes, uid, gid)
int	fdes;
uid_t	uid;
gid_t	gid;
{
	struct file *fp;
	register int flags;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	struct ucred *crp;
	static int svcnum = 0;
	int rc;

	TRCHKL2T(HKWD_SYSC_LFS | hkwd_SYSC_FCHOWN, uid, gid);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag)
	    && (audit_svcstart("FILE_Owner", &svcnum, 2, uid, gid)))
		audit_svcfinis();

	/* let's get the file pointer for the file descriptor */
	if ((rc = getft(fdes, &fp, DTYPE_VNODE)) == 0)
	{
		flags = 0;

		/* BSD compatibility */
		if (uid == -1)
			flags |= T_OWNER_AS_IS;
		if (gid == -1)
			flags |= T_GROUP_AS_IS;

		/* get creds for the operation */
		crp = crref();

		/* set the new usr and group id's */
		rc = vsetattr(fp->f_vnode, V_OWN, flags, 
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
