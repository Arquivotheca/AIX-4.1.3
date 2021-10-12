
static char sccsid[] = "@(#) 91 1.1 src/bos/usr/lpp/bosinst/BosMenus/CheckSize.c, bosinst, bos411, 9428A410j 93/07/01 12:09:53";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: CheckSize
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
/*
 * NAME: CheckSize.c
 *
 * FUNCTION: perform size checks on selected disks
 *
 * ENVIRONMENT:
 *    these environment variable are used:
 *	BOSINSTDATA - bosinst.data path
 *	IMAGEDATA - image.data path
 * NOTES:
 *   Usage: CheckSize [-s | -p]
 * 	-s:  mksysb install with shrink check on
 *	-p:  preservation install check
 *
 *	Default values are read from bosinst.data and image.data.  
 *
 * RETURNS:
 *	0 - size is ok
 *	1 - available space on selected disk is insufficient
 *	2 - error occured
 *
 */

#include <lvm.h>
#include <unistd.h>    /*Needed for access subroutine constants*/
#include <stdio.h>
#include "BosInst.h"
#include "ImageDat.h"

struct BosInst BosData;			/* bosinst.data file    */
struct imageDat ImageData;		/* image.data file      */
struct disk_info *head =  0;		/* disk info ptr        */

main(argc, argv)
int argc;
char *argv[];
{
    int c;			/* getopt option		*/
    int shrink = 0;		/* shrink check flag		*/
    int error = 0; 		/* error flag			*/
    int preserve = 0;		/* preserve check flag		*/
    int NeedSize = 0;		/* Disk space needed		*/
    int AvailableSize = 0;	/* Disk space available		*/
    
    while ((c = getopt(argc, argv, "sp")) != EOF)
    {
	switch (c)
	{
	    case 's':
		if (shrink)
		    error++;
		else shrink++;
		break;

	    case 'p':
		if (preserve)
		    error++;
		else preserve++;
		break;
	}
    }

    /* Check the command line arguments */
    if (error)
    {
	printf("Usage: CheckSize [-s | -p]\n");
	exit(2);
    }

    /* read the bosinst.data and image.data files */
    read_bosinstdata();
    read_imagedata();


    if (preserve)
    {
	NeedSize = getNeededPreserve();
	AvailableSize = getAvailPreserve(BosData.targets->HDISKNAME);

    }
    else if (shrink)
    {
	NeedSize = getNeededShrink();
	AvailableSize = getAvailOverwrite();
    }
    else
    {
	NeedSize = getNeededOverwrite();
	AvailableSize = getAvailOverwrite();
    }
    
    /* Do the check */
    if ((NeedSize < 0) || (AvailableSize < 0))
	exit(2);

    if (NeedSize > AvailableSize)
	exit(1);

    exit(0);
}
