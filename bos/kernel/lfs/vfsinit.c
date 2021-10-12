static char sccsid[] = "@(#)72	1.15  src/bos/kernel/lfs/vfsinit.c, syslfs, bos411, 9428A410j 5/16/94 13:25:21";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: vfsinit
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/user.h"
#include "sys/gpai.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/syspest.h"
#include "sys/errno.h"
#include "sys/fs_locks.h"
#include "sys/lock_alloc.h"
#include "sys/lockname.h"


/*
 * Declarations
 */

extern dev_t		rootdev;
extern int		root_vfstype;
extern struct vfs	*rootvfs;
extern struct vnode	*rootdir;
extern struct gfs	gfs[];
extern time_t 		time;
extern int		num_vfs;

struct galloc gpa_vfs = {		/* vfs gpai allocation struct */
	sizeof(struct vfs),
	1024,
	0
};
struct galloc gpa_vnode = {		/* vnode gpai allocation struct */
	sizeof(struct vnode),
	PAGESIZE,
	(int)&((struct vnode *)0)->v_lock,
	GPA_SIMPLE,
	VNODE_LOCK_CLASS,
	1
};


/*
 *	Vfsinit() is called once by main to initialize
 *	virtual file systems.
 *
 *
 * Abstract:
 *
 *	*. Call gfs_init() subroutine for all gfs_types
 *	*. Initialize the rootvfs, the root virtual file system
 *	*. Initialize "special" file system types
 *	*. Initialize services to the physical file systems
 *	*. Initialize any miscellaneous data
 *
 */

vfsinit()
{
        register struct gfs	*gfsp;		/* Varying gfs pointer  */
	int			(*rootinit)() = NULL;

	/* Initialize allocation pools for vfs structs and vnodes */
	gpai_init(&gpa_vfs);
	gpai_init(&gpa_vnode);

	/* Initialize the gfs list lock */
	lock_alloc(&gfs_lock,LOCK_ALLOC_PAGED,GFS_LOCK_CLASS,-1);
	simple_lock_init(&gfs_lock);

	/* Initialize the vfs list lock */
	lock_alloc(&vfs_list_lock,LOCK_ALLOC_PAGED,VFS_LIST_LOCK_CLASS,-1);
	simple_lock_init(&vfs_list_lock);

	/* Run the initialization routines for defined file systems */
        for (gfsp = gfs; gfsp < &gfs[num_vfs]; gfsp++)
	{
		if (gfsp->gfs_init != NULL && !(gfsp->gfs_flags & GFS_INIT) )
		{
			(*gfsp->gfs_init) (gfsp);
			gfsp->gfs_flags |= GFS_INIT;
		}
		if (gfsp->gfs_ops && gfsp->gn_ops 
		&&  gfsp->gfs_type == root_vfstype)
			rootinit = gfsp->gfs_rinit;
	}

	if ( rootinit == NULL )
		panic("vfsinit: bad root vfstype");

	/* Initialize the specfs, so rootinit can use it. */
	spec_init();

	/* Initialize the root vfs */

	if ((*rootinit)())
		panic("vfsinit: rootinit failed");

	assert(rootvfs != NULL);
	assert(rootdir != NULL);

	/* initialize "special" physical file systems */
	fifo_init();

	/* Initialize miscellaneous data */
	U.U_cdir = rootdir;
	VNOP_HOLD(rootdir);	/* bump the ref count for proc[0]'s u_cdir */
	U.U_rdir = NULL;
	U.U_start = time;
}
