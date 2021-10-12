static char sccsid[] = "@(#)45	1.25.1.5  src/bos/kernel/lfs/ftruncate.c, syslfs, bos411, 9428A410j 12/21/93 15:26:35";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: ftruncate, truncate
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

#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/file.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/trchkid.h"
#include "sys/audit.h"
#include "sys/malloc.h"
#include "sys/access.h"

BUGVDEF(ftdebug, 0);          /* define ftrunc debug variable */

/***************************************************************
 *
 * NAME:  ftruncate()                     system call entry point
 *
 * FUNCTION:	removes all data beyond "length" bytes from the
 *		beginning of the specified file. Full blocks are
 *		returned to the file system so that they can be
 *		used again, and the file size is changed to the
 *		value of the length parameter.
 * 
 * PARAMETERS:	fildes - file descriptor of an open regular file.
 *		length - new size of file after truncation
 *
 * RETURNS:	error codes of EIO if an I/O error occurred; EBADF
 *		if the fildes is not a valid file descriptor open
 *		for writing; EINVAL if the file is a FIFO file,
 *		directory, or special file; EMFILE if the file is
 *		mapped "copy-on-write" by one or more processes;
 *		EAGAIN if an enforced lock exists on the portion
 *		of the file to be removed by the truncation.
 *
 ****************************************************************/

ftruncate(fd, length)
int		fd;
off_t		length;
{
	struct vnode *vp;
	struct file *fp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;
	static int svcnum = 0;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FILE_Write", &svcnum, 1, fd)))
		audit_svcfinis();

	if ((rc = getft(fd, &fp, DTYPE_VNODE)) == 0)
	{
		vp = (struct vnode *) fp->f_vnode;

		/* trace file descriptor, vnode pointer, and length */
		TRCHKL3T(HKWD_SYSC_LFS | hkwd_SYSC_FTRUNCATE, fd, vp, length);

		if ((fp->f_flag & FWRITE) == 0)
			rc = EBADF;
		else if (vp->v_vntype != VREG) 
			rc = EINVAL;
		else
			/* call filesystem dependent ftruncate routine */
			rc = VNOP_FTRUNC(vp, fp->f_flag, length,
					fp->f_vinfo, fp->f_cred);

		ufdrele(fd);
	}

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}


/***************************************************************
 *
 * NAME:  truncate()                      system call entry point
 *
 * FUNCTION:	removes all data beyond "length" bytes from the
 *		beginning of the specified file.
 * 
 * PARAMETERS:	path   - name of regular file
 *		length - new size of file after truncation
 *
 ****************************************************************/

truncate(path, length)
char		*path;
off_t		length;
{
	struct vnode *vp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	caddr_t vinfo;
	static int svcnum = 0;
	int rc;
	struct ucred *crp;

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	if ((audit_flag) && (audit_svcstart("FILE_Write", &svcnum, 0)))
	{
                if(path){
                        char *ptr;
			int len;

                        if((ptr = malloc(MAXPATHLEN)) == NULL){
                                rc = ENOMEM;
                                goto out;
                        }
                        else if(copyinstr(path, ptr, MAXPATHLEN, &len)){
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

	TRCHKL1T(HKWD_SYSC_LFS | hkwd_SYSC_TRUNCATE, length);

	crp = crref();

	rc = lookupname(path, USR, L_EROFS, (struct vnode **)0, &vp, crp);
	if (!rc)
	{
		if (vp->v_vntype != VREG)
			rc = EINVAL;
		else
			if ((rc = VNOP_ACCESS(vp, W_ACC, NULL, crp)) == 0)
				rc = VNOP_FTRUNC(vp,FWRITE,length,vinfo,crp);
		VNOP_RELE(vp);
	}

	crfree(crp);

	/* Unlock the kernel lock unless nested locks occurred */
out:
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : 0;
}
