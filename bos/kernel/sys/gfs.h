/* @(#)00	1.12  src/bos/kernel/sys/gfs.h, syslfs, bos411, 9428A410j 2/24/94 09:29:04 */

#ifndef _H_GFS
#define _H_GFS

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/lock_def.h>

extern Simple_lock gfs_lock;		/* GFS Table lock */

struct gfs {
	struct vfsops	*gfs_ops;
	struct vnodeops	*gn_ops;
	int		gfs_type;	/* type of gfs (from <sys/vmount.h>) */
	char		gfs_name[16];	/* name of vfs (eg. "jfs","nfs", ...)*/
	int		(*gfs_init)();	/* ( gfsp ) - if ! NULL, */
					/*   called once to init gfs */
	int		gfs_flags;	/* flags for gfs capabilities */
	caddr_t		gfs_data;	/* ptr to gfs's private config data */
	int		(*gfs_rinit)();
	int             gfs_hold;       /* count of mounts of this gfs type  */
};

/*
* The new_root structure is passed through the sysconfig system
* call to announce a new root file system device and type.  This
* does not take effect until process 1 (ie. init) exits.
*/

struct new_root {
	dev_t		nr_dev;		/* device number of root gfs	*/
	int		nr_gfs_type;	/* new root's gfs type		*/
	char		nr_misc[32];	/* misc parameters, interpeted
					   by the new gfs's initialization
					   routine			*/
};

/* defines for gfs_flags */
#define GFS_SYS5DIR	0x00000001	/* directory entries are 16 bytes */
#define GFS_REMOTE	0x00000002	/* this is a remote file system */
#define	GFS_INIT	0x00000004	/* gfs has been initialized	*/
#define	GFS_FUMNT	0x00000008	/* gfs supports forced umount	*/
#define	GFS_VERSION4	0x00000010	/* gfs modified for AIX version 4 */

#endif /* _H_GFS */

