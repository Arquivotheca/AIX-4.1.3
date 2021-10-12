static char sccsid[] = "@(#) 97 1.2 src/bos/usr/lpp/bosinst/BosMenus/Sizes.c, bosinst, bos41J, 9518A_all 95/04/27 12:17:14";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: getNeededOverwrite, getNeededPreserve, getNeededShrink,
 *            getAvailOverwrite, getAvailPreserve
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>
#include <lvm.h>
#include <unistd.h>    /* Needed for access subroutine constants */
#include <stdio.h>
#include <math.h>      /* Needed for pow function */
#include "BosInst.h"
#include "ImageDat.h"

/* Converts a power of 2 to number of meg (eg. 2^^21 -> 2 Meg) */
#define conv_to_meg(ppsize)  (int)(pow (2, (ppsize - 20)))

/* Global variables */
extern struct BosInst BosData;                  /* bosinst.data file    */
extern struct imageDat ImageData;               /* image.data file      */
extern struct disk_info *head;			/* disk information head */

/*
 * NAME: getNeededOverwrite
 *
 * FUNCTION: returns the amount of disk space needed for overwrite
 *	install in MB
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Part of the BOS Install menu system.  Prerequisites: read_bosdata(),
 *	read_imagedata(), and read_targetVGS() must be called first to
 *	initialize BosData, ImageData, and disk information (head)
 *
 * NOTES: Adds all the space used by each logical volume in the image data
 *	file.
 *
 * RETURNS: size required in MB
 */

int getNeededOverwrite()
{
    struct lv_data *lvdp;	/* logical volume data traversal ptr */
    int NeededSize = 0;
   
    /* Get the nneded size by  adding all the LP values in the 
     * lv_data stanzas, then multiply by PP_SIZE and COPIES.  
     */
    for (lvdp = ImageData.lv_data; lvdp; lvdp = lvdp->next)
    {
	NeededSize += (atoi(lvdp->LPs) * atoi(lvdp->PP_SIZE) *
			atoi(lvdp->COPIES));
    }

    return NeededSize;

}


/*
 * NAME: getNeededPreserve
 *
 * FUNCTION: returt the amount of disk space needed for a  
 *	preservation install in MB
 *
 * EXECUTION ENVIRONMENT:
 *      Part of the BOS Install menu system.  Prerequisites: read_bosdata(),
 *	read_imagedata(), and read_targetVGS() must be called first to
 *	initialize BosData, ImageData, and disk information (head)
 *
 * RETURNS: required size in MB
 */

int getNeededPreserve()
{
    int NeededSize = 0;
    struct lv_data *lvdp;       /* logical volume data traversal ptr */

    /* For a preservation check, get the needed size by adding the LP values
     * from image.data (/+/usr+/var). Multiplu this total by PPSIZE.  
     */
    for (lvdp = ImageData.lv_data; lvdp; lvdp = lvdp->next)
    {
	if (strncmp (lvdp->LOGICAL_VOLUME, "hd4", 3) == 0 ||
	    strncmp (lvdp->LOGICAL_VOLUME, "hd2", 3) == 0 ||
	    strncmp (lvdp->LOGICAL_VOLUME, "hd9var", 6) == 0)
	   NeededSize += (atoi(lvdp->LPs) * atoi(lvdp->PP_SIZE)
			 * atoi( lvdp->COPIES));
    }
    return NeededSize;
}


/*
 * NAME: getNeededShrink
 *
 * FUNCTION: returns the amount of disk space needed for
 *	a mksysb shrink install in MB
 *
 * EXECUTION ENVIRONMENT:
 *      Part of the BOS Install menu system.  Prerequisites: read_bosdata(),
 *	read_imagedata(), and read_targetVGS() must be called first to
 *	initialize BosData, ImageData, and disk information (head)
 *
 * RETURNS: size needed in MB
 */

int getNeededShrink()
{
    int ppsize = 0;		/* partition size                    */
    int NeededSize = 0; 	/* Needed size                       */
    int copies = 0;		/* # mirror copies		     */
    int FSup;			/* FS_MIN_SIZE                       */
    struct lv_data *lvdp;       /* logical volume data traversal ptr */
    struct fs_data *fsdp;       /* lv_data ptr                       */

    /* For a shrink check, get the needed size by adding
     * all the LPs values from lv_data stanzas which do not have type=jfs,
     * then add to that all FS_MIN_SIZE values.  Remeber ppsize and copies
     * for use later.
     * Note that FS_MIN_SIZE values are in 512 byte blocks
     */
    for(lvdp = ImageData.lv_data; lvdp; lvdp = lvdp->next)
    {
	if (!strcmp(lvdp->TYPE, "jfs"))
	    continue;

	if (ppsize == 0)
	    ppsize = atoi(lvdp->PP_SIZE);

	if (copies == 0)
	    copies = atoi(lvdp->COPIES);

	NeededSize += (atoi(lvdp->LPs) * ppsize * copies);
	 
    }

    for (fsdp = ImageData.fs_data; fsdp; fsdp = fsdp->next)
    {
	FSup = atoi(fsdp->FS_MIN_SIZE);

	/* Convert FSup from 512 byte blocks to MB, round up to next highest */
	FSup = (((FSup % 2048) == 0) + FSup) / 2048;

        /* Round FSup to next partition size */
	FSup += ppsize - FSup % ppsize;

	NeededSize += (FSup * copies);
    }
    return NeededSize;
}


/*
 * NAME: getAvailOverwrite
 *
 * FUNCTION: get the available space for an overwrite install
 *
 * EXECUTION ENVIRONMENT:
 *      Part of the BOS Install menu system.  Prerequisites: read_bosdata(),
 *	read_imagedata(), and read_targetVGS() must be called first to
 *	initialize BosData, ImageData, and disk information (head)
 *
 * NOTES: 
 *	Add up all the available space in target drives
 *
 * RETURNS: available space in MB
 */

int getAvailOverwrite()
{
    int AvailableSize = 0;
    struct target_disk_data *tddp;

    for (tddp = BosData.targets; tddp; tddp = tddp->next)
    {
	AvailableSize += atoi(tddp->SIZE_MB);
    }
    return AvailableSize;
}


/*
 * NAME: getAvailPreserve
 *
 * FUNCTION: Gets available space from existing drives for preservation 
 *           install.  It does this by adding up the space used by /, 
 *           /usr, /var, and the freespace left in the volume group.
 *
 * EXECUTION ENVIRONMENT:
 *      Part of the BOS Install menu system.  Prerequisites: read_bosdata(),
 *	read_imagedata(), and read_targetVGS() must be called first to
 *	initialize BosData, ImageData, and disk information (head)
 *
 * RETURNS: Total available space in MB.
 */

int getAvailPreserve (char *pvname)
{
    int i;
    int AvailableSize = 0;
    int ppsize_in_meg;         /* ppsize in megabytes (as opposed to bytes) */
    struct querylv *querylv;   /* lvm record for querying lvm database */
    struct queryvg *q;         /* vg record for querying lvm database */
    struct unique_id vg_id;    /* vg_id returned from the lvm subroutine */
    struct lv_id lv_id;        /* lvm logical volume id record */

    /* Query the target disk for volume group information */
    if (lvm_queryvg (&vg_id, &q, pvname) != 0)
    {
	perror("lvm_queryvg");
	return -1;
    }

    /* Convert the ppsize to number of megabytes */
    ppsize_in_meg = conv_to_meg (q->ppsize);

    for (i = 0; i < q->num_lvs; ++i)
    {
	/* If its /, /usr or /var then query the lv to get its 
         * current size to add to AvailableSize
         */

	if (strcmp (q->lvs[i].lvname, "hd2") == 0 ||
	    strcmp (q->lvs[i].lvname, "hd4") == 0 ||
	    strcmp (q->lvs[i].lvname, "hd9var") == 0)
	{
	    if (lvm_querylv (&q->lvs[i].lv_id, &querylv, pvname) != 0)
	    {
		perror("lvm_querylv");
		return -1;
	    }

	    /* Add available size */
	    AvailableSize += querylv->currentsize * ppsize_in_meg;
	}
    }

    /* Add free space */
    AvailableSize +=  q->freespace * ppsize_in_meg;

    return AvailableSize;
}
