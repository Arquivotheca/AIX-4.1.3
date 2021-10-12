static char sccsid[] = "@(#)78	1.3.1.4  src/bos/kernel/specfs/spec_init.c, sysspecfs, bos411, 9428A410j 3/3/94 17:27:17";
/*
 * COMPONENT_NAME: (SYSSPECFS) Special File System
 *
 * FUNCTIONS: spec_init
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "sys/errno.h"
#include "sys/vnode.h"
#include "sys/specnode.h"
#include "sys/lock_alloc.h"
#include "sys/lockname.h"
#include "sys/gpai.h"
#include "sys/param.h"

extern int spec_lookup();
extern int spec_fid();
extern int spec_link();
extern int spec_setattr();
extern int spec_remove();
extern int spec_access();
extern int spec_getattr();
extern int spec_setattr();
extern int spec_open();
extern int spec_close();
extern int spec_rdwr();
extern int spec_select();
extern int spec_strategy();
extern int spec_lockctl();
extern int spec_ioctl();
extern int spec_hold();
extern int spec_rele();
extern int spec_revoke();
extern int spec_getacl();
extern int spec_setacl();
extern int spec_getpcl();
extern int spec_setpcl();

extern void spec_badop();
extern int spec_einval();
extern int spec_noop();

int spec_elist[] = {EINTR, 0};		/* ok from special's support */

/* Construct a "sparse" operations array.  Specfs common vnode ops are
 * named spec_* and can be found in spec_vnops.c
 */
struct vnodeops spec_vnops = {
	spec_link,	/* Link */
	spec_einval,	/* Mkdir */
	spec_einval,	/* Mknod */
	spec_remove,	/* Remove */
	spec_badop,	/* Rename */
	spec_einval,	/* Rmdir */
	spec_lookup,	/* Lookup */
	spec_fid,	/* Fid */
	spec_open,	/* Open */
	spec_einval,	/* Create */
	spec_hold,	/* Hold */
	spec_rele,	/* Rele */
	spec_close,	/* Close */
	spec_badop,	/* Map */
	spec_badop,	/* Unmap */
	spec_access,	/* Access */
	spec_getattr,	/* Get attributes */
	spec_setattr,	/* Set attributes */
	spec_badop,	/* Fclear */
	spec_badop,	/* Fsync */
	spec_noop,	/* Ftrunc */
	spec_rdwr,	/* Read write */
	spec_lockctl,	/* Lock control */
	spec_ioctl,	/* Ioctl */
	spec_einval,	/* Readlink */
	spec_select,	/* Select */
	spec_einval,	/* Symlink */
	spec_einval,	/* Read dir */
	spec_strategy,	/* Strategy */
	spec_revoke,	/* Revoke */
	spec_getacl,	/* Getacl */
	spec_setacl,	/* Setacl */
	spec_getpcl,	/* Getpcl */
	spec_setpcl,	/* Setpcl */
};

/*
 * Set up the specnode and devnode allocation pools.
 * The pool size for each type is one page.
 */
struct galloc specnode_pool = {   /* allocation struct for specnodes */
	sizeof(struct specnode),
	2048,
	(int)&((struct specnode *)0)->sn_lock,
	GPA_SIMPLE,
	SPECNODE_LOCK_CLASS,
	1
};
struct galloc devnode_pool = {    /* allocation struct for devnodes */
	sizeof(struct devnode),
	2048,
	(int)&((struct devnode *)0)->dv_lock,
	GPA_SIMPLE,
	DEVNODE_LOCK_CLASS,
	1
};


void
spec_init(void)
{
	struct hnode *hsnp;
	int i = 0;

	/* initialize the spec hash */
	for (hsnp = &hnodetable[0]; hsnp < &hnodetable[NHNODE]; hsnp++)
	{
		hsnp->hdv_forw = (struct devnode *)hsnp;
		hsnp->hdv_back = (struct devnode *)hsnp;
		lock_alloc(&hsnp->hn_lock,LOCK_ALLOC_PAGED,
				SPECHASH_LOCK_CLASS, ++i);
		simple_lock_init(&hsnp->hn_lock);
	}

	/* initialize the specnode and devnode allocation pools.
	 * the pool size for each type is one page.
	 */
	gpai_init(&specnode_pool);
	gpai_init(&devnode_pool);
}
