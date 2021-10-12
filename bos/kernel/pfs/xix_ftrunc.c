static char sccsid[] = "@(#)12	1.26  src/bos/kernel/pfs/xix_ftrunc.c, syspfs, bos411, 9428A410j 7/7/94 16:53:34";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_ftrunc
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
#include "sys/file.h"
#include "sys/vfs.h"

/*
 * NAME:	jfs_ftrunc (vp, flags, length, vinfo, crp)
 *
 * FUNCTION:	Truncate a file to the specified size, 
 *		any full blocks released are freed.
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the file
 *			  to be truncated.
 *		flags	- open flags
 *		length	- new length
 *		vinfo	- unused
 *		crp	- credential
 *
 * RETURN :	Zero is returned if itrunc completes sucessfully.
 *			
 */

jfs_ftrunc (vp, flags, length, vinfo, crp)
struct vnode	*vp;		/* Vnode for open file			*/
int		flags;		/* Open flags				*/
offset_t	length;		/* New length				*/
caddr_t		vinfo;		/* Gfs specific inofo			*/
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc = 0;
	struct inode *ip;
	struct eflock bf;
	extern int iwritelockx();
	extern int iwriteunlockx();

	ip = VTOIP(vp);                 /* get inode                    */
	IWRITE_LOCK(ip);

	if (ENF_LOCK(ip->i_mode))
	{                               /* enforced lock mode?          */
		bf.l_type = F_WRLCK;    /* Write lock                   */
		bf.l_whence = 0;
		bf.l_start = length;    /* start of region to lock */
		bf.l_len = 0;           /* lock to EOF             */
		bf.l_pid = U.U_procp->p_pid;
		bf.l_sysid = 0;
		bf.l_vfs = MNT_JFS;

		if (  (rc = common_reclock(ITOGP(ip),
					   ip->i_size,
					   length,
					   &bf,
		      ( (flags & (FNDELAY|FNONBLOCK)) ?
				 INOFLCK : SLPFLCK|INOFLCK ),
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
	
	/* if we're truncating to a non-zero length, we need to 
	 * make sure we are bound it to a vm segment
	 */
	if (length != 0 && ip->i_seg == 0 && (rc = ibindseg(ip)))
	 	goto out;

	rc = itrunc(ip, (off_t) length, crp);

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
