static char sccsid[] = "@(#)54	1.18  src/bos/kernel/lfs/mntctl.c, syslfs, bos41B, 412_41B_sync 12/6/94 16:47:13";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: mntctl, get_mntinfo
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/user.h>
#include "sys/fs_locks.h"
#include <sys/vfs.h>
#include <sys/errno.h>
#include <sys/syspest.h>

BUGVDEF(rdebug,0);			/* for debugging purposes only  */

/************************************************************************
 *
 * NAME:	mntctl system call
 *
 * FUNCTION:	Provides user information about currently mounted
 *		filesystems.
 *
 * PARAMETERS:	cmd	- only MCTL_QUERY allowed.
 *		size	- size of user's buffer in bytes.
 *		buf	- pointer to user's buffer.
 *
 * RETURNS:	number of currently mounted filesystems or -1, if
 *		an error occurs. If user's buffer is too small to
 *		hold the full information, but is big enough for
 *		an integer, the first integer in user's buffer is
 *		set to the required size and a zero is returned
 *		(but no error is set).
 *
 ************************************************************************/
mntctl(cmd, size, buf)
int		cmd;	/* user-specified command code	*/
unsigned int	size;	/* size of user buffer in bytes */
char		*buf;	/* user buffer (word aligned)	*/
{

	int 	rval = 0;	/* return value */
	extern kernel_lock;	/* global kernel lock			*/
	int lockt;		/* previous state of kernel lock	*/
	int rc;			/* u_error value */

	/* Grab the global kernel lock */
	lockt = FS_KLOCK(&kernel_lock, LOCK_SHORT);

	BUGLPR(rdebug,BUGACT,("Entered mntctl system call\n"));

	/* now validate the input parameters				*/
	if ((!size) || (!buf))
	{
		BUGLPR(rdebug,BUGACT,("Size or buffer is 0\n"));
		rc = EINVAL;
		goto out;
	}

	BUGLPR(rdebug,BUGNTX,("Input parms are cmd %d size %d &buf 0x%x\n",
		cmd,size,buf));

	/* take the appropriate action according to the specified command*/
	switch (cmd) {
	case MCTL_QUERY :	/* get mount status NEW style */
		rc = get_mntinfo(size, buf, &rval);
		break;
	default:
		BUGLPR(rdebug,BUGACT,("bad command %d\n",cmd));
		rc = EINVAL;
	}

out:
	BUGLPR(rdebug,BUGGID,("exiting mntctl, ret=%d, err=%d\n", rval, rc));

	/* Unlock the kernel lock unless nested locks occurred */
	if( lockt != LOCK_NEST )
		FS_KUNLOCK(&kernel_lock);

        if (rc)
                u.u_error = rc;
        return rc ? -1 : rval;
}

int
get_mntinfo(size, buf, mcp)
int size;		/* size of user buffer in bytes	*/
char *buf;		/* user buffer address		*/
int  *mcp;		/* ^ to mount count		*/
{
	struct vfs	*v;
	unsigned int	newsize = 0,	/* required buffer size		*/
			mountcount = 0,	/* number of mounts performed	*/
			i;		/* index			*/
	int		rc = 0;		/* return code			*/
	extern struct vfs *rootvfs;	/* pointer to first vfs		*/

	/* calculate the buffer size needed to contain all mount info   */
	VFS_LIST_LOCK();
	for (v=rootvfs; v; v=v->vfs_next) {
		if (v->vfs_flag & VFS_UNMOUNTING)
			continue;
		newsize += v->vfs_mdata->vmt_length;
		mountcount++;
	}
	BUGLPR(rdebug,BUGNTF,("get_mntinfo: size=%d, newsize=%d, mnts=%d\n",
		size, newsize, mountcount));

	ASSERT(mountcount);

	*mcp = 0;

	if (size < newsize) {
		/*
		 * must have supplied at least one int of space
		 */
		if (size < sizeof (newsize))
		{
			VFS_LIST_UNLOCK();
			rc = EINVAL;
		}

		/*
		 * tell the user what buffer size is required to succeed.
		 *  see above ASSERT().
		 */
		else  
		{
			VFS_LIST_UNLOCK();
			if (copyout((caddr_t) &newsize, (caddr_t) buf
			    , sizeof (newsize)))
				rc = EFAULT;

		}
		return rc;
	}

	/*
	 * the user's buffer space is large enough; fill it
	 * copy contents of vmount structures into the buffer
	 * NOTE: guard against new mounts after the above check
	 * only copy mountcount vmount structures
	 */
	for (v=rootvfs, i = 0; v && i < mountcount; v=v->vfs_next) {
		BUGLPR(rdebug,BUGGID,("vfsflag = %x\n",v->vfs_flag))
		if (v->vfs_flag & VFS_UNMOUNTING)
			continue;

		if (copyout((caddr_t) v->vfs_mdata, buf, 
				v->vfs_mdata->vmt_length)) {
			rc = EFAULT;
			goto err;
		}

		buf += v->vfs_mdata->vmt_length;
		i++;
	}

	*mcp = mountcount;	/* pass back # of mounts	*/
	rc = 0;

    err:
	VFS_LIST_UNLOCK();
	return rc;
}
