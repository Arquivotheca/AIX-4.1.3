static char sccsid[] = "@(#)87	1.4  src/bos/kernel/lfs/gfs.c, syslfs, bos411, 9428A410j 8/27/93 16:21:06";

/* 
 * COMPONENT_NAME: SYSLFS - Logical File System
 * 
 * FUNCTIONS: gfsadd, gfsdel, gfsqry
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/user.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <sys/gfs.h>
#include <sys/errno.h>
#include <sys/device.h>
#include "sys/fs_locks.h"

int
gfsadd(gfsno, gfsp)
int		gfsno;
struct gfs	*gfsp;
{
	int			i;		/* index into gfs table	*/
	int			rc;		/* return code		*/
	int			lockt;		/* state of kernel lock	*/

	extern struct gfs	gfs[];		/* table of gfs structs	*/
	extern struct gfs	*gfsindex[];	/* mapping table of gfs	*/
	extern int		max_gfstype;	/* gfsindex table size	*/
	extern int		num_vfs;	/* gfs table size	*/

	/* validate range of gfsno */
	if (gfsno < 0 || gfsno > max_gfstype)
		return EINVAL;

	GFS_LOCK();

	/* cannot replace another gfs */
	if (gfsindex[gfsno])
		rc = EBUSY;
	
	/* gfs must indicate version 4 conformance */
	else if ((gfsp->gfs_flags & GFS_VERSION4) == 0)
		rc = EINVAL;

	else
	{
		/* look through the gfs table for an empty slot */
		for (i = 0; i < num_vfs; i++) 
			if (gfs[i].gfs_ops == (struct vfsops *)NULL)
				break;
		/* fill in the gfs and gfsindex tables and call */
		/* the initialization routine for the gfs       */
		if (i < num_vfs)
		{
			gfs[i] = *gfsp;
			gfsindex[gfsno] = &gfs[i];
			if (gfsp->gfs_init)
				rc = ((*gfsp->gfs_init)(gfsp));
			else
				rc = 0;
		}
		else
			rc = ENOMEM;
	}

	GFS_UNLOCK();
	return (rc);
}

int
gfsdel(gfsno)
int	gfsno;
{
	struct gfs              *gfsp;          /* ptr to gfs structure */
	int			rc;		/* return code		*/

	extern struct gfs	gfs[];		/* table of gfs structs	*/
	extern struct gfs	*gfsindex[];	/* mapping table of gfs	*/
	extern int		max_gfstype;	/* gfsindex table size	*/

	/* validate range of gfsno */
	if (gfsno < 0 || gfsno > max_gfstype)
		return EINVAL;

	GFS_LOCK();

	/* set pointer to the gfs entry */
	gfsp = gfsindex[gfsno];

	/* check for gfs entry existence */
	if (gfsp == NULL)
		rc = ENOENT;

	/* make sure it is not in use */
	else if (gfsp->gfs_hold)
		rc = EBUSY;

	else
	{
		/* remove gfs entry */
		bzero(gfsp, sizeof(struct gfs));
		gfsindex[gfsno] = NULL;
		rc = 0;
	}

	GFS_UNLOCK();
	return rc;
}

int
gfsqry(gfsno, gfspp)
int		gfsno;
struct gfs **	gfspp;
{
	extern struct gfs	*gfsindex[];	/* mapping table of gfs */
	extern int		max_gfstype;	/* gfsindex table size	*/

	/* validate range of gfsno */
	if (gfsno < 0 || gfsno > max_gfstype)
		return EINVAL;

	*gfspp = gfsindex[gfsno];
	return 0;
}

