static char sccsid[] = "@(#)14	1.28.1.5  src/bos/kernel/pfs/xix_init.c, syspfs, bos411, 9438B411a 9/20/94 11:56:51";
/*
 * COMPONENT_NAME: (SYSPFS) Physical File System
 *
 * FUNCTIONS: jfs_init
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
#include "jfs/fsvar.h"
#include "sys/vfs.h"
#include "sys/errno.h"

/*
 * JFS initialization parameter declarations
 */
struct fsvar fsv = { 
	10,		/* initial # inodes	*/
	NULL,
	0,		/* initial # mounts	*/
	NULL,
	0,		/* initial # file	*/
	NULL,
	0,		/* initial # flock	*/
	NULL,
	256,		/* initial # devinodes	*/
	NULL,
	10,		/* initial # dquots	*/
	NULL 
};

/* recognizable error conditions from setjmpx */
int reg_elist[] = {EIO, ENOSPC, EFBIG, ENOMEM, EDQUOT, 0};


/*
 * JFS virtual file system operations
 */

extern int jfs_mount();
extern int jfs_umount();
extern int jfs_root();
extern int jfs_statfs();
extern int jfs_sync();
extern int jfs_vget();
extern int jfs_cntl();
extern int jfs_quotactl();
extern int jfs_init();

struct vfsops jfs_vfsops = {
	jfs_mount,
	jfs_umount,
	jfs_root,
	jfs_statfs,
	jfs_sync,
	jfs_vget,
	jfs_cntl,
	jfs_quotactl
};

 
/*
 * JFS virtual file system vnode operations
 */
 
extern int jfs_access();
extern int jfs_chdirec();
extern int jfs_close();
extern int jfs_create();
extern int jfs_fclear();
extern int jfs_fid();
extern int jfs_fsync();
extern int jfs_ftrunc();
extern int jfs_getattr();
extern int jfs_hold();
extern int jfs_inactive();
extern int jfs_link();
extern int jfs_lockctl();
extern int jfs_lookup();
extern int jfs_mkdir();
extern int jfs_mknod();
extern int jfs_open();
extern int jfs_badop();
extern int jfs_rdwr();
extern int jfs_readdir();
extern int jfs_rele();
extern int jfs_remove();
extern int jfs_rename();
extern int jfs_rmdir();
extern int jfs_setattr();
extern int jfs_map();
extern int jfs_unmap();
extern int jfs_readlink();
extern int jfs_symlink();
extern int jfs_getacl();
extern int jfs_setacl();
extern int jfs_getpcl();
extern int jfs_setpcl();
extern int jfs_einval();
extern int jfs_noop();

struct vnodeops jfs_vops = {
	jfs_link,
	jfs_mkdir,
	jfs_mknod,
	jfs_remove,
	jfs_rename,
	jfs_rmdir,
	jfs_lookup,
	jfs_fid,
	jfs_open,
	jfs_create,
	jfs_hold,
	jfs_rele,
	jfs_close,
	jfs_map,
	jfs_unmap,
	jfs_access,
	jfs_getattr,
	jfs_setattr,
	jfs_fclear,
	jfs_fsync,
	jfs_ftrunc,
	jfs_rdwr,
	jfs_lockctl,
	jfs_einval,	/* vnop_ioctl */
	jfs_readlink,
	jfs_badop,	/* vnop_select */
	jfs_symlink,
	jfs_readdir,
	jfs_badop,	/* vnop_strategy */
	jfs_einval,	/* vnop_revoke */
	jfs_getacl,
	jfs_setacl,
	jfs_getpcl,
	jfs_setpcl
};

extern int vm_initlock(), inoinit(), dnlc_init();

extern Simple_lock jfs_sync_lock;
extern Simple_lock jfs_comp_lock;
extern Complex_lock jfs_quota_lock;


/*
 * NAME:	jfs_init (gfsp)
 *
 * FUNCTION:	initialize the file system
 *
 * PARAMETERS:	gfsp	- Pointer our gfs as set up at configuration time
 *
 * RETURNS:	Always zero
 */

jfs_init(gfsp)
struct gfs *gfsp;		/* gfs pointer		*/
{
	/* initialize the sync lock
	 */
	lock_alloc(&jfs_sync_lock,LOCK_ALLOC_PAGED,SYNC_LOCK_CLASS,-1);
	simple_lock_init(&jfs_sync_lock);

	/* initialize the compression lock
	 */
	lock_alloc(&jfs_comp_lock,LOCK_ALLOC_PAGED,COMP_LOCK_CLASS,-1);
	simple_lock_init(&jfs_comp_lock);

	/* initialize the quota lock
	 */
        lock_alloc(&jfs_quota_lock,LOCK_ALLOC_PAGED,QUOTA_LOCK_CLASS,-1);
        lock_init(&jfs_quota_lock,1);

	/* initialize jfs vmm services */
	vm_initlock();

	/* initialize JFS inode cache and directory name lookup cache
	 */
	inoinit();
	dnlc_init();
}


/*
 * NAME:	p2fsinit
 *
 * FUNCTION:	extend pfs data structures to run time levels
 *
 * PARAMETERS:	None
 *
 * RETURNS:	None
 */

void
p2fsinit()
{
	ICACHE_LOCK();
	/* grow inode cache */
	inogrow();
	ICACHE_UNLOCK();

	/* grow directory name lookup cache */
	dnlc_grow();
}


/*
 * NAME:	jfs_badop()
 *
 * FUNCTION:	Grossly invalid vnode or vfs operation
 *
 * PARAMETERS:	Ignored
 *
 * RETURNS:	Never
 */

jfs_badop()
{
	panic ("jfs_badop");
}


/*
 * NAME:	jfs_einval()
 *
 * FUNCTION:	Invalid vnode or vfs operation
 *
 * PARAMETERS:	Ignored
 *
 * RETURNS:	EINVAL
 */

jfs_einval()
{
	return EINVAL;
}


/*
 * NAME:	jfs_noop()
 *
 * FUNCTION:	Ignored vnode or vfs operation
 *
 * PARAMETERS:	Ignored
 *
 * RETURNS:	0
 */

jfs_noop()
{
	return 0;
}
