static char sccsid[] = "@(#)09	1.19.1.5  src/bos/kernel/pfs/xix_fclear.c, syspfs, bos411, 9428A410j 7/7/94 16:53:26";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_fclear
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#include "sys/syspest.h"

/*
 * NAME:	jfs_fclear(vp, flags, offset, nbytes, vinfo, crp)
 *
 * FUNCTION:	this function clears bytes in a file, returning whole
 *		free blocks to the underlying file system.
 *
 * PARAMETERS:	vp	-  vnode that represents the file
 *			   to be cleared.
 *		flags	- open file flags.
 *		offset 	- offset in the open file structure.
 *		nbytes	- length of the area to be cleared
 *		vinfo 	- vfs specific data from the open file structure.
 *
 * RETURN :	Zero is returned if iclear() completes sucessfully.
 *		if an error occurrs an error code from errno.h is
 *		returned.
 *			
 */

jfs_fclear(vp, flags, offset, nbytes, vinfo, crp)
struct vnode	*vp;		/* Vnode of open file		*/
int		flags;		/* Open flags			*/
offset_t	offset;		/* Offset to begin		*/
offset_t	nbytes;		/* # of bytes to clear		*/
caddr_t		vinfo;		/* Something if remote		*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc = 0;			/* Return code			*/
	struct inode *ip;		/* Inode for vp			*/
	struct eflock bf;		/* Record locking buf		*/
	extern int iwritelockx();
	extern int iwriteunlockx();

	/*
	 * must check that we are not trying to clear past the ulimit.
	 * this would cause problems if it faulted since the VMM checks
	 * ulimit on page faults
	 */
	if (offset + nbytes > U.U_limit)
		return(EFBIG);

	ip = VTOIP(vp);
	IWRITE_LOCK(ip);

	ASSERT(ip->i_seg);

	if (ENF_LOCK(ip->i_mode))
	{                               /* enforced lock mode?  */
		bf.l_type = F_WRLCK;    /* Write lock                   */
		bf.l_whence = 0;
		bf.l_start = offset;	/* start of region to lock */
		bf.l_len = nbytes;      /* number of bytes to lock      */
		bf.l_pid = U.U_procp->p_pid;
		bf.l_sysid = 0;
		bf.l_vfs = MNT_JFS;

		if (   (rc = common_reclock(ITOGP(ip),
					    ip->i_size,
					    offset,
					    &bf,
		  ((flags & (FNDELAY|FNONBLOCK)) ? INOFLCK : SLPFLCK|INOFLCK),
					    0,
					    0,
					    iwritelockx,
					    iwriteunlockx))
		    || bf.l_type != F_UNLCK)
		{
			
			rc = (rc) ? rc : EAGAIN;
			goto out;
		}
	}

	rc = iclear(ip, (off_t) offset, (off_t) nbytes, crp);

	switch(rc)
        {
                case EMFILE:
                case EFBIG:
                case EINVAL:
                case E2BIG:
                        break;
                default:
                        clearprivbits(ip, crp);
	}

out:
	IWRITE_UNLOCK(ip);
	return rc;
}
