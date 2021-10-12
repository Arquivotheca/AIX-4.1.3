static char sccsid[] = "@(#)80	1.3.1.7  src/bos/kernel/specfs/fifo_init.c, sysspecfs, bos411, 9428A410j 5/16/94 13:25:44";
/*
 * COMPONENT_NAME: (SYSSPECFS) Special File System
 *
 * FUNCTIONS: fifo_init
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

#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/specnode.h"
#include "sys/lock_alloc.h"
#include "sys/lockname.h"
#include "sys/gpai.h"
#include "sys/param.h"

extern int fifo_badop();
extern int fifo_einval();
extern int fifo_noop();

extern int fifo_link();
extern int fifo_mknod();
extern int fifo_remove();
extern int fifo_rename();
extern int fifo_fid();
extern int fifo_open();
extern int fifo_hold();
extern int fifo_rele();
extern int fifo_close();
extern int fifo_access();
extern int fifo_getattr();
extern int fifo_setattr();
extern int fifo_rdwr();
extern int fifo_rw();
extern int fifo_lockctl();
extern int fifo_select();
extern int fifo_getacl();
extern int fifo_setacl();
extern int fifo_getpcl();
extern int fifo_setpcl();
extern Simple_lock fifobuf_lock;
struct vfs pipevfs;		/* vfs for pipes (which don't have a PFS vfs) */

/* Construct a "sparse" operations array for special files.
 *
 * The logical file system is written to use parent directory vnodes
 * to cross into the vfs environment.  That is why things like link, remove etc
 * point to spec_badop().  While you can link to a special you can't
 * link "through" a special.
 */

struct vnodeops fifo_vnops = {
	fifo_link,	/* Link */
	fifo_einval,	/* Mkdir */
	fifo_badop,	/* Mknod */
	fifo_remove,	/* Remove */
	fifo_rename,	/* Rename */
	fifo_einval,	/* Rmdir */
	fifo_badop,	/* Lookup */
	fifo_fid,	/* Fid */
	fifo_open,	/* Open */
	fifo_einval,	/* Create */
	fifo_hold,	/* Hold */
	fifo_rele,	/* Rele */
	fifo_close,	/* Close */
	fifo_badop,	/* Map */
	fifo_badop,	/* Unmap */
	fifo_access,	/* Access */
	fifo_getattr,	/* Get attributes */
	fifo_setattr,	/* Set attributes. fifo_setattr? */
	fifo_badop,	/* XXX. should work? Fclear */
	fifo_badop,	/* XXX. should work? Fsync */
	fifo_noop,	/* Ftrunc */
	fifo_rdwr,	/* Read write */
	fifo_lockctl,	/* Lock control */
	fifo_badop,	/* Ioctl */
	fifo_einval,	/* Readlink */
	fifo_select,	/* Select */
	fifo_einval,	/* Symlink */
	fifo_einval,	/* Read dir */
	fifo_badop,	/* Strategy */
	fifo_badop,	/* Revoke */
	fifo_getacl,	/* Getacl */
	fifo_setacl,	/* Setacl */
	fifo_getpcl,	/* Getpcl */
	fifo_setpcl,	/* Setpcl */
};

struct galloc fifonode_pool = {    /* allocation struct for fifonodes */
	sizeof(struct fifonode),
	1024,
	0, 0, 0, 0 
};

void
fifo_init(void)
{
	extern struct galloc fifonode_pool;

	/* initialize pipe buffer size variables */
	(void)fifo_pool_init();

	/* initialize the pipe vfs */
	pipevfs.vfs_gfs = NULL;
	bzero(&pipevfs, sizeof pipevfs);	/* XXX */
	pipevfs.vfs_vnodes = NULL;

	/* allocate and initialize the pipe buffer lock */
	lock_alloc(&fifobuf_lock,LOCK_ALLOC_PAGED,FIFOBUF_LOCK_CLASS,-1);
	simple_lock_init(&fifobuf_lock);

	/* initialize the pipenode allocation pool */
	gpai_init(&fifonode_pool);

#ifdef FEFIFOFUM
	{
		static struct fileops fefifofum;
		extern int fifo_rw();
		/*
		 * first time thru here: setup our fifo fileops.
		 */
		fefifofum = vnodefops;		/* copy vnode FOPS	*/
		fefifofum.fo_rw = fifo_rw;	/* fast path thru gorp	*/
	}
#endif
}
