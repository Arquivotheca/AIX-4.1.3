static char sccsid[] = "@(#)34	1.24  src/bos/kernel/pfs/xix_statfs.c, syspfs, bos411, 9428A410j 7/7/94 16:54:54";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_statfs
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "jfs/jfslock.h"
#include "jfs/filsys.h"
#include "jfs/commit.h"
#include "sys/vfs.h"
#include "sys/statfs.h"
#include "sys/vmdisk.h"
#include "sys/dir.h"

/*
 * NAME:	jfs_statfs (vfsp, sfsp, crp)
 *
 * FUNCTION:	Return fs independent ustat.
 *
 * PARAMETERS:	vfsp	- virtual file system to return info for
 *		sfsp	- where to return info
 *		crp	- credential
 *
 * RETURN :	Always zero on success, non-zero on failure
 *			
 * SERIALIZATION: statfs() and extendfs() serializes by inode lock
 *		  of the inode map for both inode map and fragment
 *		  allocation map. 
 *		  All other access to fragment allocation map is 
 *		  serialized under VMM locking.
 */

int
jfs_statfs(vfsp, sfsp, crp)
register struct vfs	*vfsp;
register struct statfs	*sfsp;
struct ucred		*crp;		/* pointer to credential structure */
{
	register int rc;		/* return code		*/
	struct vnode *rootv;		/* root vnode		*/
	register struct inode *mounti;	/* mount inode		*/
	struct inode *ipim, *ipv;
	struct vmdmap *vmd = NULL;	/* virtual memory map	*/
	struct superblock *volatile sb = NULL;	/* superblock struct	*/
	label_t   jb;      		/* setjmpx/longjmpx buffer 	*/

	/*
	 * get the root vnode of the vfs
	 */
	if (rc = jfs_root(vfsp, &rootv, crp))
		return (rc);

	/*
	 * get the mount inode from the root vnode
	 */
	mounti = (VTOIP(rootv))->i_ipmnt;

 	/* statfs()/extendfs() serialized by iread lock of the inode map 
	 * for both inode map and fragment allocation map. 
	 */
	ipim = mounti->i_ipinomap;
	IREAD_LOCK(ipim);

	/* establish exception return point for permanent io errors 
	 * which may occur from loads or stores into memory mapped files.
	 */
	if (rc = setjmpx(&jb))
	{	if (vmd)		/* either map */
			ipundo(vmd);
		if (sb)
			ipundo(sb);
		rc = pfs_exception((rc|PFS_EXCEPTION), reg_elist);
		goto out;
	}

	/*
	 * map diskmap 
	 */
	ipv = mounti->i_ipdmap;
	if (rc = iptovaddr(ipv, 0, (caddr_t *) &vmd))
		goto clrexp;

	/*
	 * get the block stats from the diskmap
	 */
	sfsp->f_blocks = vmd->mapsize / mounti->i_fperpage;
	sfsp->f_bfree = sfsp->f_bavail = vmd->freecnt / mounti->i_fperpage;

	ipundo(vmd);		/* release diskmap vm	*/
	vmd = NULL;

	/*
	 * map inodemap
	 */
	if (rc = iptovaddr(ipim, 0, (caddr_t *) &vmd))
		goto clrexp;

	/*
	 * get the file stats from the inodemap
	 */
	sfsp->f_files = vmd->mapsize;
	sfsp->f_ffree = vmd->freecnt;

	ipundo(vmd);		/* release inodemap */
	vmd = NULL;

	/*
	 * map superblock
	 */
	ipv = mounti->i_ipsuper;
	if (rc = iptovaddr(ipv, 0, (caddr_t *) &sb))
		goto clrexp;

	/*
	 * get stuff we need from superblock
	 */
	bcopy(sb->s_fname, sfsp->f_fname, sizeof(sb->s_fname));
	bcopy(sb->s_fpack, sfsp->f_fpack, sizeof(sb->s_fpack));

	ipundo(sb);		/* release superblock vm	*/

	/*
	 * now, fill in easy stuff
	 */
	sfsp->f_bsize = PAGESIZE;
	sfsp->f_fsize = PAGESIZE / mounti->i_fperpage;
	sfsp->f_vfstype = MNT_JFS;
	sfsp->f_name_max = MAXNAMLEN;
	sfsp->f_vfsnumber = vfsp->vfs_number;
	sfsp->f_fsid = vfsp->vfs_fsid;

	/*
	 * fields in the statfs structure that we don't fill in ...
	 */
/*	long f_version;		version/type of statfs, 0 for now */
/*	long f_type;		type of info, zero for now */
/*	long f_vfsoff;		reserved, for vfs specific data offset */
/*	long f_vfslen;		reserved, for len of vfs specific data */
/*	long f_vfsvers;		reserved, for vers of vfs specific data */

clrexp:
	clrjmpx(&jb);		/* pop exception return */

out:
	IREAD_UNLOCK(ipim);

	/* release root vnode reference acquired in jfs_root()
	 */
	jfs_rele(rootv);

	return (rc);
}
