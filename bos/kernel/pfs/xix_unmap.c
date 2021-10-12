static char sccsid[] = "@(#)40	1.11.1.5  src/bos/kernel/pfs/xix_unmap.c, syspfs, bos411, 9428A410j 7/7/94 16:55:14";
/*
 * COMPONENT_NAME: (SYSPFS) unmap vnode op
 *
 * FUNCTIONS: jfs_unmap
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
#include "sys/shm.h"
#include "sys/syspest.h"

BUGVDEF(xunmap,0);

/* 
 * NAME:	jfs_unmap (vp, flag, crp)
 *
 * FUNCTION: decrements the usecount of map-writers if
 *	the vmobj represents a writer (NOT a general interface).
 *
 * EXECUTION ENVIRONMENT:
 *	called from fp_shmdt().
 *
 * RETURNS: 0
 *
 */

int
jfs_unmap (vp, flag, crp)
struct vnode	*vp;		/* vnode to unmap		*/
int		flag;		/* flags used to call jfs_map 	*/
struct ucred	*crp;		/* pointer to credential structure */
{
	struct inode *ip;
	struct gnode *gp;

	BUGLPR(xunmap, BUGNTF, ("jfs_unmap: vp=%X fl=%X\n", vp, flag));

	ip = VTOIP(vp);
	gp = VTOGP(vp);
	IWRITE_LOCK(ip);

	if (flag & SHM_RDONLY) {
		imark(ip, IACC);
		ASSERT(gp->gn_mrdcnt > 0);
		gp->gn_mrdcnt -= 1;

	} else {
		clearprivbits(ip, crp);
		imark(ip, (ip->i_flag & IDEFER) ? IACC : IUPD);
		ASSERT(gp->gn_mwrcnt > 0);
		gp->gn_mwrcnt -= 1;
	}

	/* if the file was opened deferred update backout changes
	 * to file if there are no more accessors.  this has to be done
	 * here to handle the open(), mmap(), close() sequence.
	 */
	if (ip->i_flag & IDEFER)
	{
		if ((gp->gn_rdcnt + gp->gn_wrcnt + gp->gn_excnt +
		     gp->gn_mrdcnt + gp->gn_mwrcnt) == 0)
		{
			ifreeseg(ip);
			ip->i_flag &= ~(IDEFER | IACC | ICHG | IFSYNC);
			ip->i_cflag = 0;
		}
	}

	IWRITE_UNLOCK(ip);

	/* release reference of mapper
	 */
	jfs_rele(vp);

	return 0;
}
