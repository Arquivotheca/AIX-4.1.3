static char sccsid[] = "@(#)19	1.3  src/bos/usr/bin/dosdir/reclaim.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:59:51";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _DFreclaimspace use_cluster release_cluster cluster_inuse 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "pcdos.h"
#include <doserrno.h>

/*
 *      reclaim.c            PC-DOS disk space manager
 *              reclaims all allocated clusters in chain
 *              starting at input parameter
 *              returns 0
 */
_DFreclaimspace(disk,x)
register DCB *disk;
register int x;
{
register int y;

	if (disk->magic != DCBMAGIC)
		return(0);
	if ((x <= 1) || (x > disk->ccount))
		return(0);

	/* Don't delete a cluster chain that someone is
	 * currently using (i.e., an open file).
	 */
	if (cluster_inuse(disk, x))
	{	doserrno = DE_ACCES;
		return(-1);
	}

	while (x < PC_EOF && x > 0)
	{       y =  disk->fat_ptr[x].cluster;
		disk->fat_ptr[x].cluster = 0;
		disk->fat_ptr[x].usecount = 0;
		if (y > disk->ccount)
			break;
		x = y;
	}
	disk->changed++;
	return(0);
}

/*
 * Mark a cluster chain as currently in use.
 * This routine is called when a DOS file is opened
 * to mark the first cluster in the file as being used.
 * By convention, "_DFreclaimspace" will not delete
 * a cluster chain marked as being used.
 *    This scheme prevents one process from deleting a file
 * that other processes have open.
 */
use_cluster(disk, cl)
DCB *disk;
int cl;	/* first cluster in cluster chain */
{
    if (cl <= 0)
	return;

    /* Increment use count.
     */
    disk->fat_ptr[cl].usecount++;

    TRACE(("Using cluster #%d, %s (count=%d)\n", cl, disk->dev_name,
      disk->fat_ptr[cl].usecount));
}

/*
 * Decrement use count on a cluster chain.
 */
release_cluster(disk, cl)
DCB *disk;
int cl;	/* first cluster in cluster chain */
{
    if (cl <= 0)
	return;

    /* Increment use count.
     */
    if (disk->fat_ptr[cl].usecount  >  0)
	disk->fat_ptr[cl].usecount--;

    TRACE(("Releasing cluster #%d, %s (count=%d)\n", cl, disk->dev_name,
      disk->fat_ptr[cl].usecount));
}

/*
 * Is a cluster chain in use?
 */
cluster_inuse(disk, cl)
DCB *disk;
int cl;	/* first cluster in cluster chain */
{
    if (cl <= 0)
	return(FALSE);

    TRACE(("Checking cluster #%d, %s: %s\n", cl, disk->dev_name,
      disk->fat_ptr[cl].usecount ? "in use" : "NOT in use"));

    return( disk->fat_ptr[cl].usecount  >  0);
}
