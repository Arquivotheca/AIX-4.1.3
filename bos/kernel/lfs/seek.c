static char sccsid[] = "@(#)62	1.26  src/bos/kernel/lfs/seek.c, syslfs, bos411, 9428A410j 2/14/94 17:58:41";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: lseek, fp_lseek, _lseek,
 *
 * ORIGINS: 3, 26, 27
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
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/fs_locks.h"
#include "sys/vnode.h"
#include "sys/vattr.h"
#include "sys/syspest.h"
#include "sys/fp_io.h"
#include "unistd.h"
#include "sys/trchkid.h"
#include "sys/limits.h"


/********************************************************************
 *
 * NAME:  lseek system call
 *
 * FUNCTION:	sets the file table offset for an open file
 *
 * PARAMETERS:	fdes - the file descriptor
 *		off  - the new offset
 *		sbase- one of the following modes:
 *			SEEK_SET: sets the file pointer to the value
 *				  of the "off" parameter.
 *			SEEK_CUR: sets the file pointer to its current
 *				  location plus the value of the "off"
 *				  parameter.
 *			SEEK_END: sets the file pointer to the size of 
 *				  the file plus the value of the "off"
 *				  parameter.
 *
 * RETURNS:	If successful the file offset is changed and the new 
 *		offset is returned. If not, the offset is left unchanged,
 *		-1 is returned and u.u_error is set to indicate error
 *		condition.
 * NOTES:	lseek is now a front end to the generalized 64 bit seek
 *		interface.
 *
 *****************************************************************/

off_t
lseek(
int	fdes,
off_t	off,
int	sbase)
{
	return lseek_common(fdes, (offset_t) off, sbase, 0);
}

/********************************************************************
 *
 * NAME:  _lseek system call
 *
 * FUNCTION:	64 bit version of the lseek system call.
 *
 * PARAMETERS:	fdes	file descriptor
 *		off	new file offset value
 *		sbase	seek mode, one of SEEK_SET, SEEK_CUR, or SEEK_END
 *		new_offp
 *			reference parameter used to return the resulting
 *			64 bit offset.  The normal return value linkage
 *			cannot be used because the system call mechanism
 *			is 32 bit limited.  llseek() is implemented in libc.
 *
 * RETURNS:	0 if successful, -1 w/ u.u_error set upon failure.
 *
 * NOTES:	_lseek is a front end to the generalized 64 bit seek
 *		routine.
 *
 *****************************************************************/

_lseek(
int		fdes,
offset_t	off,
int		sbase,
offset_t *	new_offp)
{
	return lseek_common(fdes, off, sbase, new_offp);
}

/********************************************************************
 *
 * NAME:	lseek_common
 *
 * FUNCTION:	common code for lseek and llseek.
 *
 * PARAMETERS:	fdes	file descriptor
 *		off	new file offset value
 *		sbase	seek mode, one of SEEK_SET, SEEK_CUR, or SEEK_END
 *		offp	returned offset pointer or NULL
 *
 * RETURNS:	0 or resulting offset if successful, -1 w/ u.u_error set
 *		on failure.
 *
 * NOTES:	When lseek calls lseek_common, offp is NULL and the
 *		resulting offset is returned via the return value.  When
 *		_lseek calls lseek_common, offp is the reference
 *		parameter passed to _lseek, the resulting offset is copied
 *		back to user space, and the return value is 0.
 *
 *****************************************************************/

lseek_common(
int		fdes,
offset_t	off,
int		sbase,
offset_t *	offp) 
{
	struct file *fp = NULL;
	struct vnode *vp;
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int offset_locked = 0;  /* f_offset field locking               */
	int rc;
	offset_t offmax;

	TRCHKL3T(HKWD_SYSC_LSEEK, fdes, off, sbase);

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	/* Fetch the file pointer */
	if (rc = getf(fdes, &fp))
		goto out;

	/* Don't allow seeks on sockets or other weird things */
	if (fp->f_type != DTYPE_VNODE) 
	{
		if (fp->f_type == DTYPE_SOCKET)
			rc = ESPIPE;
		else
			rc = EINVAL;
		goto out;
	}

	vp = fp->f_vnode;

	/* Don't allow seek on pipes */
	if (vp->v_vntype == VFIFO)
	{
		rc = ESPIPE;
		goto out;
	}

	if ((U.U_procp->p_threadcount != 1) || (fp->f_count != 1))
	{
		offset_locked = 1;
		FP_OFFSET_LOCK(fp);
	}

	switch (sbase) 
	{
		case SEEK_SET:
        	break;
		
       		case SEEK_CUR:
		off += fp->f_offset; 
		break;

		case SEEK_END:
		{
			struct vattr vattr;
			rc = VNOP_GETATTR(vp, &vattr, fp->f_cred);
			if (rc == 0)
				off = vattr.va_size + off;
			else 
				goto ulock;
		}
		break;
	
		default:
		/*
		 * The old System V behaviour of sending
		 * SIGSYS is no longer supported.
	 	 pidsig(U.U_procp->p_pid, SIGSYS);
		 */
		rc = EINVAL;
		goto ulock;
		break;
	}

	if (offp)	/* caller was llseek */
	{
		offmax = (vp->v_vntype == VCHR || vp->v_vntype == VBLK) ?
				 (offset_t) DEV_OFF_MAX : (offset_t) OFF_MAX;

		if (off < 0 || off > offmax )
			rc = EINVAL;
		else if ((rc = copyout(&off,offp,sizeof(off))) == 0)
		{
			fp->f_offset = off;
			fp->f_dir_off = 0;

			/* set off to 0 so ultimate return value will be 0 */
			off = 0;
		}
	}
	else		/* caller was normal lseek */
	{
		/* we do not allow negative offsets */
		if (off < 0 || off > OFF_MAX) 
			rc = EINVAL;
		else /* set the offset, finally */
		{
			fp->f_offset = off;
			fp->f_dir_off = 0;
		}
	}

ulock:
	if (offset_locked)
		FP_OFFSET_UNLOCK(fp);

out:
	if (fp)
		ufdrele(fdes);

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

	if (rc)
		u.u_error = rc;
	return rc ? -1 : (off_t) off;
}

/*
 * NAME:	fp_lseek
 *
 * FUNCTION:	Kernel service to set the file table offset
 *
 * PARAMETERS:
 *	struct	file	*fp;			file to change seek in
 *	off_t		offset;			offset to seek to
 *	int		whence;			relative base of offset
 *
 * RETURN:
 * 	non-zero for errors
 */

int
fp_lseek (struct file 	*fp, 
	  off_t 	offset, 
	  int 		whence)
{
	return fp_llseek (fp, (offset_t) offset, whence);
}

/*
 * NAME:	fp_llseek
 *
 * FUNCTION:	This kernel service changes the offset value in a file 
 *		table entry to the value specified by the offset parameter, 
 *		relative to the base indicated by whence.  The resulting 
 *		offset must be less than DEV_OFF_MAX. Negative offsets should
 *		be prohibited, however; they were allowed in earlier releases,
 *		so they continue to be allowed. 
 *
 * PARAMETERS:
 *	struct	file	*fp;			file to change seek in
 *	offset_t	offset;			offset to seek to
 *	int		whence;			relative base of offset
 *
 * RETURN:
 * 	non-zero for errors
 */

int
fp_llseek (struct file	*fp, 
	   offset_t 	offset, 
	   int 		whence)
{
	struct vattr	vattr;		/* attributes from getattr()	*/
	int 	klock;	        	/* save kernel_lock state       */
	int	rc = 0;			/* error code to return		*/
	int     offset_locked = 0;	/* f_offset field locked        */
	offset_t	newoff;		/* new offset			*/

	ASSERT(fp != NULL);
	ASSERT(fp->f_count > 0);

	if ((klock = IS_LOCKED(&kernel_lock)) != 0)
		unlockl(&kernel_lock);

	if ((U.U_procp->p_threadcount != 1) || (fp->f_count != 1))
	{
		offset_locked = 1;
		FP_OFFSET_LOCK(fp);
	}
	
	switch (whence) 
	{
		case SEEK_SET:
			newoff = offset;
			break;

	    	case SEEK_CUR:
			newoff = fp->f_offset + offset;
			break;

		case SEEK_END:
			if (fp->f_type != DTYPE_VNODE)
				rc = EINVAL;
			else
			{
				rc = VNOP_GETATTR(fp->f_vnode, &vattr,
					 fp->f_cred);

				if (!rc)
					newoff = vattr.va_size + offset;
			}
			break;

		default:
			rc = EINVAL;
	}

	if (newoff > DEV_OFF_MAX)
		rc = EINVAL;

	if (!rc)
		fp->f_offset = newoff;

	if (offset_locked)
		FP_OFFSET_UNLOCK(fp);

	if (klock)
		lockl(&kernel_lock, LOCK_SHORT);

	return rc;
}
