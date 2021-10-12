static char sccsid[] = "@(#)66	1.18  src/bos/kernel/lfs/sync.c, syslfs, bos411, 9434A411a 8/22/94 13:30:31";
/*
 * COMPONENT_NAME: (SYSLFS) Logical File System
 *
 * FUNCTIONS: sync
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:	sync()		(system call entry point)
 *
 * FUNCTION:	Used to ensure that the filesystems are in an updated and
 *		consistent state.  It forces all active gfs's to flush any
 *		information out to disk or similar permanent storage.  
 *		NOTE: the sync system call only STARTS the update process
 *
 * PARAMETERS:	none
 *
 * RETURN VALUES: none, it is a "void" function
 */
#include <sys/gfs.h>
#include <sys/vfs.h>
#include "sys/fs_locks.h"

void
sync()
{
	extern int num_vfs;
	extern struct gfs *gfsindex[];
	struct gfs *gfsp;
	int i;

	GFS_LOCK();

	for (i=0; i < num_vfs; i++)
	{
		gfsp = gfsindex[i];
		if (gfsp && (gfsp->gfs_ops->vfs_sync))
		{
			/* Release GFS_LOCK across sync to avoid unmount
			 * deadlock if remote fs is dead.
			 */
			gfsp->gfs_hold++;
			GFS_UNLOCK();
			VFS_SYNC(gfsp);
			GFS_LOCK();
			gfsp->gfs_hold--;
		}
	}

	GFS_UNLOCK();
}
