static char sccsid[] = "@(#) 01 1.2 src/bos/usr/lpp/bosinst/BosMenus/readTargetVGS.c, bosinst, bos411, 9428A410j 94/01/21 17:32:51";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: readTargetVGS
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
/* readTagetVGS
 *
 * Description: open and read TARGETVGS file
 *
 * Return value: pointer to the head  of the list 
 */
/*
 * NAME: read_targetVGS
 *
 * FUNCTION: Read targetvgs file into global list structure
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure is called by BosMenus and CheckSize to read the
 *      image.data fiel pointed to by the environment variable IMAGEDATA.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      linked list of target disks
 *
 * RETURNS: head of list
 */

#include <string.h>
#include <stdio.h>
#include "Menus.h"
#include "BosDisks.h"
#include "BosInst.h"
#include "ImageDat.h"

extern struct BosInst BosData;                 /* bosinst.data file    */
extern struct imageDat ImageData;

struct disk_info *readTargetVGS()
{
    FILE *fp;                        /* file pointer to targetvgs */
    char *targetvgs;                /* targetvgs path            */
    struct src_disk_data *sddp = 0;
    char line[80];                /* a line in targetvgs       */
    char bootable;
    struct disk_info *dip, *head, *last;        /* disk_info ptr             */

    /* if the list is empty, build a new list.  The disk list is pointed
     * to by the TARGETVGS environment variable
     */
    
    targetvgs = getenv("TARGETVGS");
    if (!targetvgs)
    {
        targetvgs = "targetvgs";
    }
    
    fp = fopen(targetvgs, "r");
    if (!fp)
    {
        perror(targetvgs);
        return 0;
    }

    head = last = 0;
    /* loop until the people come home */
    while (1)
    {
        if (!fgets(line, 80, fp))
            break;
        
        /* allocate a new node */
        dip = (struct disk_info *)malloc(sizeof (struct disk_info));

        if (!dip)
        {
            fprintf(stderr, "Malloc error in ChgDiskPre()");
            exit(2);
        }

	/* clear the structure */
	bzero(dip, sizeof(struct disk_info));

        /* insert the new node into the list */
        if (last)
        {
            last->next = dip;
        }
        else
        {
            head = dip;
        }
        last = dip;
        dip->next = 0;

        sscanf(line, "%s %d %s %s %s %s %c", dip->vgid, &dip->vgstat, 
            dip->level, dip->name, dip->location, dip->size, &bootable );
        if ((bootable == 'y') || (bootable == 'Y'))
            dip->bootable = 1;
        else
            dip->bootable = 0;
    }
    fclose(fp);

    return head;
}
