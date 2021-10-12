static char sccsid[] = "@(#)18	1.20  src/bos/kernel/pfs/xix_lookup.c, syspfs, bos41J, 9514A_all 3/31/95 12:52:29";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_lookup
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/inode.h"
#include "jfs/commit.h"
#include "sys/vfs.h"
#include "sys/dir.h"
#include "sys/sysinfo.h"

/*
 * NAME:	jfs_lookup (vp, vpp, pname, flag, vattrp, crp)
 *
 * FUNCTION:	Lookup pname in the directory given by vp.  Allocate vnode
 *		when necessary and return in vpp.
 *		If vattrp is not null then fill in the vattr struct.
 *
 * PARAMETERS:	vp 	_ is the pointer to the vnode that represents the 
 *			  directory where name is to be
 *		vpp	- returned vnode if name found
 *		pname	- sought name
 *		flag
 *		vattrp  - attributes to be returned
 *		crp	- credential
 *
 * RETURN:	vnode with a reference, or
 *		errors from subroutines.
 *			
 */

jfs_lookup(dvp, vpp, pname, flag, vattrp, crp)
struct vnode	*dvp;
struct vnode	**vpp;
char		*pname;
int		flag;
struct vattr	*vattrp;
struct ucred	*crp;		/* pointer to credential structure */
{
	int 	rc;		/* Return code				*/
	struct hinode *hip;	/* hash list inode resides on		*/
	struct inode *dip;	/* Directory inode			*/
	struct inode *ip;	/* Returned inode from iget		*/
	dname_t nmp;		/* Name argument			*/
	ino_t	ino;		/* i_number from dnlc or directory	*/
	char	nmbuf[MAXNAMLEN];
	struct vfs	*vfsp;

	*vpp = NULL;

	vfsp = dvp->v_vfsp;
	if (!(vfsp->vfs_flag & VFS_DEVMOUNT))
		vfsp = NULL;

	dip = VTOIP(dvp);

	IREAD_LOCK(dip);

	if (rc = iaccess(dip, IEXEC, crp))
		goto out;

	/* search dnlc
	 */
	nmp.nmlen = strlen(pname);
	nmp.nm = pname;

	ino = dnlc_lookup(dip->i_dev, dip->i_number, &nmp);

	if (ino == 0)
	{
		/*
		 * dnlc miss
		 */
		/* search directory */
		if (rc = dir_lookup(dip, &nmp, &ino, crp))
			goto out;

		/* insert name to dnlc */
		dnlc_enter(dip->i_dev, dip->i_number, &nmp, ino);
	}

	/* get the inode/vnode from i_number
	 */
	if (ino != dip->i_number)
	{
		IHASH(dip->i_dev, ino, hip);
		ICACHE_LOCK();
		rc = _iget(dip->i_dev, ino, hip, &ip, 1, vfsp);
		ICACHE_UNLOCK();
		sysinfo.iget++;
		cpuinfo[CPUID].iget++;
		IREAD_UNLOCK(dip);
		if (rc == 0)
		{
			if (vfsp)
				/* v_count (and i_count on first reference
				 * by the base vnode) acquired by _iget(). 
				 */
				*vpp = ip->i_gnode.gn_vnode;
			else
				/* i_count acquired by _iget().
				 * iptovp() release i_count (thus ip) as needed.
				 */
				rc = iptovp(dvp->v_vfsp, ip, vpp);
		}

		if (rc == 0 && vattrp != NULL)
			jfs_getattr(*vpp, vattrp, crp);
		
		return rc;
	}
	else
	{
		/* looking up "."
		 */
		if (vfsp)
		{
			fetch_and_add(&dvp->v_count, 1);
		}
		else
		{
#ifdef _I_MULT_RDRS
			if (IREAD_TO_WRITE(dip))
				IWRITE_LOCK(dip);
#endif
			/* acquire v_count under IREAD_LOCK */
			dvp->v_count++;
		}

		*vpp = dvp;
		if (vattrp != NULL)
			get_vattr(dip, vattrp);
	}

out:
	IREAD_UNLOCK(dip);
	RETURNX(rc, reg_elist);
}
