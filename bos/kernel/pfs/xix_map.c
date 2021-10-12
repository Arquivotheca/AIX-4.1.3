static char sccsid[] = "@(#)19	1.21.1.6  src/bos/kernel/pfs/xix_map.c, syspfs, bos41J, 9514A_all 3/31/95 12:52:36";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_map
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
#include "sys/errno.h"
#include "sys/user.h"
#include "sys/vmount.h"
#include "sys/vfs.h"
#include "sys/shm.h"
#include "sys/syspest.h"

BUGVDEF(xmapdbg,0)

/*
 * NAME:	jfs_map (vp, addr, length, offset, flags, crp)
 *
 * FUNCTION: map file, increments use counts
 *
 * RETURNS:
 *	0	successful
 *	nonzero error code if not successful
 */

int
jfs_map (vp, addr, length, offset, flags, crp)
struct vnode	*vp;
caddr_t		addr;
uint		length;
uint		offset;
uint		flags;
struct ucred	*crp;		/* pointer to credential structure */
{
	int rc;
	struct inode *ip;
	struct gnode *gp;
	struct eflock lckdat;
	struct vfs	*vfsp = vp->v_vfsp;
	extern int iwritelockx();
	extern int iwriteunlockx();

	BUGLPR(xmapdbg, BUGNTF, ("jfs_map: vp=%X fl=%X\n", vp, flags));

	/*
	 * It is an error if a request has been made
	 * to map a file that has an enforced lock.
	 * So, attempt to create an exclusive write
	 * lock ("There can be only one.") and check
	 * for success.
	 */

	gp = VTOGP(vp);
	ip = GTOIP(gp);

	IWRITE_LOCK(ip);

	/* only allow regular files to be mapped
	 */
	if (gp->gn_type != VREG) {
		rc = EINVAL;
		goto map_exit;
	}

	if (ENF_LOCK(ip->i_mode)) {
		lckdat.l_type	= (flags & SHM_RDONLY) ? F_RDLCK : F_WRLCK;
		lckdat.l_whence	= 0;
		lckdat.l_len	= 0;
		lckdat.l_start	= 0;
		lckdat.l_pid	= U.U_procp->p_pid;
		lckdat.l_sysid	= 0;
		lckdat.l_vfs	= MNT_JFS;
		BUGLPR(xmapdbg, BUGNTF,	
			("jfs_map: calling common_reclock\n"));
		rc = common_reclock (gp,
				     ip->i_size,
				     0,
				     &lckdat,
				     SETFLCK|INOFLCK,
				     0,
				     0,
				     iwritelockx,
				     iwriteunlockx);
		/*
		 * common_reclock sets lckdat.l_type to
		 * F_UNLCK upon success.  So, check both
		 * possible error indications.
		 */
		if (rc || (lckdat.l_type != F_UNLCK)) {
			BUGLPR(xmapdbg, BUGNTF,
					("jfs_map:failed rc=EAGAIN\n"));
			rc = EAGAIN;
			goto map_exit;
		}

	}

	/* commit at transition from non-deferred update to deferred-update
	 */
	if ((flags & SHM_COPY) && !(ip->i_flag & IDEFER)) {
		ip->i_flag |= IFSYNC;
		if (rc = commit(1, ip))
			goto map_exit;
		vcs_defer(gp->gn_seg);
		ip->i_flag |= IDEFER;
	}

	/* increment segment mapped count
	 */
	if (flags & SHM_RDONLY) {
		gp->gn_mrdcnt += 1;
	} else {

		/* set "sticky" mapped bit.
		 */
		gp->gn_mwrcnt += 1;
		gp->gn_flags |= GNF_WMAP;

		/* set last page to read-only, if this is first map
		 */
		if (gp->gn_mwrcnt == 1)
			isetsize(ip, ip->i_size);
	}

	/* acquire reference of mapper
	 */
	if (vfsp->vfs_flag & VFS_DEVMOUNT)
		fetch_and_add(&vp->v_count, 1);
	else
		vp->v_count++;

	rc = 0;

map_exit:
	IWRITE_UNLOCK(ip);
	return rc;
}
