static char sccsid[] = "@(#)65	1.12.1.6  src/bos/kernel/lfs/symlink.c, syslfs, bos411, 9428A410j 3/26/94 12:35:14";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: symlink, readlink
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
 */

#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/fs_locks.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "sys/audit.h"
#include "sys/malloc.h"

BUGVDEF(dbslink, 0);

/**********************************************************************
 *
 * NAME: symlink()     system call entry point
 *
 * FUNCTION:	creates a symbolic link in the file system. A symbolic
 *		link is a special inode type (IFLNK), the data blocks
 *		of the inode contain a path name. When a symbolic link
 *		is encountered during a lookup call, the path name is
 *		added to the current lookup path and the lookup continues
 *		This routine is similar to the link routine.
 *
 * PARAMETERS:	target   - the contents of the symbolic link
 *		linkname - the file name of the new symbolic link
 *
 * RETURNS:	returns with u.u_error set to EEXIST if the linkname
 *		already exists.
 *
 *********************************************************************/

symlink(target, linkname)
char	*target;
char	*linkname;
{
	struct vnode	*dvp;
	struct vnode	*vp;
	struct pathname	tpn;
	struct pathname	lpn;
	struct ucred    *crp;
	int		rc;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	static int svcnum = 0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* get the current credentials */
	crp = crref();

	if ((audit_flag) && (audit_svcstart("FILE_Link",&svcnum,0)))
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

	if (rc = pn_get(linkname, USR, &lpn))
		goto out;

	/**** lookup linkname , obtain the PARENT vnode ********/

	rc = lookuppn(&lpn, L_CREATE | L_NOFOLLOW, &dvp, &vp, crp);

	if (rc) {
		pn_free(&lpn);
		goto out;
	}

	/* Have to release both the parent and the linkname's
	 * vnode when the linkname already exists.
	 */
	if (vp != NULL) {
		VNOP_RELE(vp);
		VNOP_RELE(dvp);
		rc = EEXIST;
		pn_free(&lpn);
		goto out;
	}
	rc = pn_get(target, USR, &tpn);
	if (rc == 0) {
		rc = VNOP_SYMLINK(dvp, lpn.pn_path, tpn.pn_path, crp);
		pn_free(&tpn);
	}
	VNOP_RELE(dvp);
	pn_free(&lpn);
out:
	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}

/********************************************************************
 *
 * NAME:  readlink()     Read contents of symbolic link.
 *
 * FUNCTION:	reads and returns the contents of a symbolic link. The
 *		purpose of this is to provide a user level interface
 *		symbolic links.
 *
 * PARAMETERS:	name - pathname associated with the symbolic link.
 *		buf  - pointer to user space where the contents of
 *		       the symbolic link are to go.
 *		count- size of buf in bytes
 *
 * RETURNS:	If there was no error, then it sets return value (rc) to the
 *		size of the symbolic link. If there was an error then
 *		u.u_error will be set to the following error code:
 *		EFAULT if the pathname or buf extends outside
 *		the process's address space. 
 *
 ***************************************************************/

readlink(name, buf, count)
char	*name;
char	*buf;
size_t	count;
{
	struct vnode *vp;
	struct pathname pn;
	struct iovec	aiov;
	struct uio	auio;
	int		rc;
	int		rval;   /* return value (number of bytes read)  */
	struct ucred    *crp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* get the current credentials */
	crp = crref();

	if (rc = pn_get(name, USR, &pn))
		goto out;

	rc = lookuppn(&pn, L_SEARCH | L_NOFOLLOW, NULL, &vp, crp);

	if (rc) {
		pn_free(&pn);
		goto out;
	}

	if (vp->v_vntype != VLNK){
		rc = EINVAL;
		VNOP_RELE(vp);
		pn_free(&pn);
		goto out;
	}

	aiov.iov_base = buf;
	aiov.iov_len = count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_resid = count;

	rc = VNOP_READLINK(vp, &auio, crp);

	/* AES requires link name to be null terminated */
	if (rc == 0 && auio.uio_resid > 0)
	{
		rc = ureadc('\0', &auio);
		auio.uio_resid++;
	}

	rval = count - auio.uio_resid;

	pn_free(&pn);	
	VNOP_RELE(vp);
out:
	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : rval;
}
