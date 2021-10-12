static char sccsid[] = "@(#)45	1.16  src/bos/kernel/lfs/vfsdata.c, syslfs, bos411, 9428A410j 7/7/94 16:55:28";

/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS:
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

#include "sys/param.h"
#include "sys/types.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/vmount.h"
#include "sys/gfs.h"
#include "sys/sleep.h"

/*
* The boot time rootdev is always 0,0.  This selects the "ram disk" device
* driver.  The real root file system is determined and mounted once the
* root volume group has been brought on-line.
*/

dev_t	rootdev = makedev(0,0);

int	root_vfstype = MNT_JFS;

struct vfs *	rootvfs;
struct vnode *	rootdir;

int	gn_reclk_count = 1;	/* occurrence number of gn_reclck_lock */
int	umount_elist = EVENT_NULL; /* global umount event list */

extern int jfs_init(), jfs_rootinit();
extern struct vfsops    jfs_vfsops;
extern struct vnodeops  jfs_vops;
Simple_lock	gfs_lock;	/* GFS Table Lock */
Simple_lock	vfs_list_lock;	/* VFS Table Lock */
Simple_lock	ffree_lock;	/* File Table Free List Lock */
Simple_lock	filock_lock;    /* File Lock Table Lock */

extern nodev();

struct gfs gfs[] =
{
	{ &jfs_vfsops, &jfs_vops, MNT_JFS, "jfs",  jfs_init,
		GFS_VERSION4, NULL, jfs_rootinit },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
};

int num_vfs = sizeof(gfs)/sizeof(struct gfs);

struct gfs *gfsindex[] =
{
	NULL,		/* MNT_AIX - old AIX file system		*/
	NULL,		/* Reserved					*/
	NULL,		/* MNT_NFS - Network File System		*/
	&gfs[0],	/* MNT_JFS - AIX V3 file system		*/
	NULL,		/* Reserved					*/
	NULL,		/* MNT_CDROM - CDROM File System		*/
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

int max_gfstype = sizeof(gfsindex)/sizeof(struct gfs *) - 1;

#include "sys/file.h"

#define	NFILE		200000
#define	MINFILE		200

int nfile_max = NFILE;
int nfile_min = MINFILE;

struct file file[NFILE];

#include "sys/flock.h"

#define	NFLOCKS		200000
#define	MINFLOCKS	200

struct filock flox[NFLOCKS];

int nflox_max = NFLOCKS;
int nflox_min = MINFLOCKS;
