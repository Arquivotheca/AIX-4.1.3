static char sccsid[] = "@(#) 00 1.6 src/bos/usr/lpp/bosinst/BosMenus/readImage.c, bosinst, bos41J, 9523B_all 95/06/07 09:07:38";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: read_imagedata, write_imagedata
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
 * NAME: read_imagedata
 *
 * FUNCTION: Read image.data file into global structure
 *
 * EXECUTION ENVIRONMENT:
 *	This procedure is called by BosMenus and CheckSize to read the
 *	image.data fiel pointed to by the environment variable IMAGEDATA.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *	ImageData filled in
 *
 * RETURNS: none
 */

#include <string.h>
#include <stdio.h>
#include "ImageDat.h"
extern struct imageDat ImageData;              /* image.data file      */

struct comment				/* list of comments in the file */
{
    struct comment *next;
    char *text;				/* text of comment		*/
};
static struct comment *first_comment, *last_comment;

read_imagedata()
{
    char *imageFileName;		/* image.data file name		*/
    FILE *fp;				/* file ptr to image.data	*/
    char wholeline[160];			/* line of text from image.data	*/
    char *line;				/* ptr within a line of image.data */
    char *ptr, *eptr;			/* more ptrs within line	*/
    struct src_disk_data *sddp = 0;	/* source disk data ptr		*/
    struct lv_data *lvdp = 0;		/* logical volume data ptr	*/
    struct fs_data *fsdp = 0;		/* filesystem data ptr		*/
    struct comment *cmt;		/* comment structure */

    imageFileName = getenv("IMAGEDATA");
    if (!imageFileName)
	imageFileName = "image.data";
    
    fp = fopen(imageFileName, "r");
    if (!fp) return;


    /* loop until the end of file */
    while (1)
    {
	line = wholeline;
	if(!fgets(line, 160, fp)) break;

 	/* save comment line */
	if (*line == '#')
	{
	    cmt = malloc(sizeof (struct comment));
	    cmt->text = malloc(strlen(line));
	    if (first_comment)
	    {
		last_comment->next = cmt;
	    }
	    else
		first_comment = cmt;
	    
	    last_comment = cmt;
	    strncpy(cmt->text, line, 160);
	    continue;
	}

        /* remove newline */
        eptr = strchr(line, '\n');
	/* If no new line, set eptr to end of buffer */
	if(!eptr)
	{
	    eptr = line + 159;
	}
	ptr = strchr(line, '=');
        if (ptr)
	{
	    if((unsigned)(eptr-ptr) < 2)
		/* this field has no value, so continue */
		continue;
            else
	    {
		*eptr-- = '\0';
		/* remove all trailing blanks */
		while (*eptr == ' ')
		    *eptr-- = '\0';
	    }
	}
	/* parse the line */
	if (!strncmp(line, "source_disk_data:", 17))
	{

	    /* malloc a new node for target disk stucture */

	    sddp= (struct src_disk_data *)malloc (sizeof(struct src_disk_data));
	    if (!sddp)
		exit(2);

	    if (ImageData.src_disk_data)
		ImageData.last_sddp->next = sddp;
	    else
		ImageData.src_disk_data = sddp;

	    ImageData.last_sddp = sddp;
	    sddp->next = (struct src_disk_data *)0;
	    continue;
	}

	if (!strncmp(line, "lv_data:", 8))
	{
	    /* malloc a new node for pv data stucture */

	    lvdp= (struct lv_data *)malloc (sizeof(struct lv_data));
	    if (!lvdp)
		exit(2);

	    if (ImageData.lv_data)
		ImageData.last_lvdp->next = lvdp;
	    else
		ImageData.lv_data = lvdp;

	    bzero(lvdp, sizeof(struct lv_data));
	    ImageData.last_lvdp = lvdp;
	    continue;
	}

	if (!strncmp(line, "fs_data:",8))
	{
	    /* malloc a new node for fs data stucture */

	    fsdp= (struct fs_data *)malloc (sizeof(struct fs_data));
	    if (!fsdp)
		exit(2);

	    if (ImageData.fs_data)
		ImageData.last_fsdp->next = fsdp;
	    else
		ImageData.fs_data = fsdp;

	    bzero(fsdp, sizeof(struct fs_data));
	    ImageData.last_fsdp = fsdp;
	    continue;
	}

	/* That takes care of all of the stanza lines.  For the
	 * fields to be correctly identified, line needs to be 
	 * bumped past any whitespace
	 */
	while (isspace(*line))
		line++;

	/* Point ptr to the data of the field */
	ptr = strchr(line, '=');
	if (!ptr++) continue;
	while (isspace(*ptr))
		ptr++;

	if (!strncmp(line, "IMAGE_TYPE", 10))
	{
	    strcpy(ImageData.image_data.IMAGE_TYPE, ptr);
	    continue;
	}
	if (!strncmp(line, "DATE_TIME", 9))
	{
	    strcpy(ImageData.image_data.DATE_TIME, ptr);
	    continue;
	}
	if (!strncmp(line, "UNAME_INFO", 10))
	{
	    strcpy(ImageData.image_data.UNAME_INFO, ptr);
	    continue;
	}
	if (!strncmp(line, "LICENSE_INFO", 12))
	{
	    strcpy(ImageData.image_data.LICENSE_INFO, ptr);
	    continue;
	}
	if (!strncmp(line, "PRODUCT_TAPE", 12))
	{
	    strcpy(ImageData.image_data.PRODUCT_TAPE, ptr);
	    continue;
	}
/* USERVG_LIST is unused field
	if (!strncmp(line, "USERVG_LIST", 11))
	{
	    strcpy(ImageData.image_data.USERVG_LIST, ptr);
	    continue;
	}
*/
	if (!strncmp(line, "LANG",4))
	{
	    strcpy(ImageData.ils_data.LANG, ptr);
	    continue;
	}

	if (!strncmp(line, "SHRINK",6))
	{
	    strcpy(ImageData.lv_policy.SHRINK, ptr);
	    continue;
	}

	if (!strncmp(line, "EXACT_FIT",9))
	{
	    strcpy(ImageData.lv_policy.EXACT_FIT, ptr);
	    continue;
	}


	if (!strncmp(line, "HDISKNAME",9))
	{
	    if (sddp)
		strcpy(sddp->HDISKNAME, ptr);
	    continue;
	}

	if (!strncmp(line, "LOCATION",8))
	{
	    if (sddp)
		strcpy(sddp->LOCATION, ptr);
	    continue;
	}

	if (!strncmp(line, "SIZE_MB", 7))
	{
	    if (sddp)
		strcpy(sddp->SIZE, ptr);
	    continue;
	}


	if (!strncmp(line, "VGNAME", 6))
	{
	    strcpy(ImageData.vg_data.VGNAME, ptr);
	    continue;
	}

	if (!strncmp(line, "PPSIZE", 6))
	{
	    strcpy(ImageData.vg_data.PPSIZE, ptr);
	    continue;
	}

	if (!strncmp(line, "VARYON", 6))
	{
	    strcpy(ImageData.vg_data.VARYON, ptr);
	    continue;
	}

	if (!strncmp(line, "VG_SOURCE_DISK_LIST", 19))
	{
	    strcpy(ImageData.vg_data.VG_SOURCE_DISK_LIST, ptr);
	    continue;
	}

	if (!strncmp(line, "VOLUME_GROUP", 12))
	{
	    if (lvdp)
		strcpy(lvdp->VOLUME_GROUP, ptr);
	    continue;
	}

	if (!strncmp(line, "LV_SOURCE_DISK_LIST", 19))
	{
	    if (lvdp)
		strcpy(lvdp->LV_SOURCE_DISK_LIST, ptr);
	    continue;
	}

	if (!strncmp(line, "LOGICAL_VOLUME", 14))
	{
	    if (lvdp)
		strcpy(lvdp->LOGICAL_VOLUME, ptr);
	    continue;
	}

	if (!strncmp(line, "TYPE", 4))
	{
	    if (lvdp)
		strcpy(lvdp->TYPE, ptr);
	    continue;
	}

	if (!strncmp(line, "MAX_LPS", 7))
	{
	    if (lvdp)
		strcpy(lvdp->MAX_LPS, ptr);
	    continue;
	}

	if (!strncmp(line, "COPIES", 6))
	{
	    if (lvdp)
		strcpy(lvdp->COPIES, ptr);
	    continue;
	}

	if (!strncmp(line, "LPs", 3))
	{
	    if (lvdp)
		strcpy(lvdp->LPs, ptr);
	    continue;
	}

	if (!strncmp(line, "BB_POLICY", 9))
	{
	    if (lvdp)
		strcpy(lvdp->BB_POLICY, ptr);
	    continue;
	}

	if (!strncmp(line, "INTER_POLICY", 12))
	{
	    if (lvdp)
		strcpy(lvdp->INTER_POLICY, ptr);
	    continue;
	}

	if (!strncmp(line, "INTRA_POLICY", 12))
	{
	    if (lvdp)
		strcpy(lvdp->INTRA_POLICY, ptr);
	    continue;
	}

	if (!strncmp(line, "MIRROR_WRITE_CONSISTENCY", 24))
	{
	    if (lvdp)
		strcpy(lvdp->MIRROR_WRITE_CONSISTENCY, ptr);
	    continue;
	}

	if (!strncmp(line, "WRITE_VERIFY", 12))
	{
	    if (lvdp)
		strcpy(lvdp->WRITE_VERIFY, ptr);
	    continue;
	}

	if (!strncmp(line, "UPPER_BOUND", 11))
	{
	    if (lvdp)
		strcpy(lvdp->UPPER_BOUND, ptr);
	    continue;
	}

	if (!strncmp(line, "SCHED_POLICY", 12))
	{
	    if (lvdp)
		strcpy(lvdp->SCHED_POLICY, ptr);
	    continue;
	}

	if (!strncmp(line, "RELOCATABLE", 11))
	{
	    if (lvdp)
		strcpy(lvdp->RELOCATABLE, ptr);
	    continue;
	}

	if (!strncmp(line, "LV_SEPARATE_PV", 14))
	{
	    if (lvdp)
		strcpy(lvdp->LV_SEPARATE_PV, ptr);
	    continue;
	}

	if (!strncmp(line, "LABEL", 5))
	{
	    if (lvdp)
		strcpy(lvdp->LABEL, ptr);
	    continue;
	}

	if (!strncmp(line, "MAPFILE", 7))
	{
	    if (lvdp)
		strcpy(lvdp->MAPFILE, ptr);
	    continue;
	}

	if (!strncmp(line, "PP_SIZE", 7))
	{
	    if (lvdp)
		strcpy(lvdp->PP_SIZE, ptr);
	    continue;
	}

	if (!strncmp(line, "FS_NAME", 7))
	{
	    if (fsdp)
		strcpy(fsdp->FS_NAME, ptr);
	    continue;
	}

	if (!strncmp(line, "FS_SIZE", 7))
	{
	    if (fsdp)
		strcpy(fsdp->FS_SIZE, ptr);
	    continue;
	}

	if (!strncmp(line, "FS_MIN_SIZE", 11))
	{
	    if (fsdp)
		strcpy(fsdp->FS_MIN_SIZE, ptr);
	    continue;
	}

	if (!strncmp(line, "FS_LV", 5))
	{
	    if (fsdp)
		strcpy(fsdp->FS_LV, ptr);
	    continue;
	}

	if (!strncmp(line, "FS_FS", 5))
	{
	    if (fsdp)
		strcpy(fsdp->FS_FS, ptr);
	    continue;
	}

	if (!strncmp(line, "FS_COMPRESS", 7))
	{
	    if (fsdp)
		strcpy(fsdp->FS_COMPRESS, ptr);
	    continue;
	}

	if (!strncmp(line, "FS_NBPI", 7))
	{
	    if (fsdp)
		strcpy(fsdp->FS_NBPI, ptr);
	    continue;
	}

	if (!strncmp(line, "BOSINST_FILE", 7))
	{
	    strcpy(ImageData.post_install_data.BOSINST_FILE, ptr);
	    continue;
	}

    }
    fclose(fp);
}

/*
 * NAME: write_imagedata
 *
 * FUNCTION: Write image.data file from global structure
 *
 * EXECUTION ENVIRONMENT:
 *      This procedure is called by BosMenus to write the
 *      image.data file pointed to by the environment variable IMAGEDATA.
 *
 * NOTES:
 *	Most of the fields are not of much interest to BosMenus, only
 *	PRODUCT_TAPE, SHRINK, and EXACT_FIT are modified.
 *
 * DATA STRUCTURES:
 *      ImageData 
 *
 * RETURNS: none
 */

write_imagedata()
{
    int i;				/* loop index			*/
    char *imageFileName;		/* image datafile name		*/
    FILE *fp, *rfp;			/* write and read fiel ptrs	*/
    char line[80];			/* a lineof the file		*/
    char *ptr = line;			/* ptr within line		*/
    struct src_disk_data *sddp = 0;	/* source disk data ptr		*/
    struct lv_data *lvdp = 0;		/* logical volume data ptr	*/
    struct fs_data *fsdp = 0;		/* filesystem data ptr		*/
    struct comment *cmt;		/* comment structure */

    imageFileName = getenv("IMAGEDATA");
    if (!imageFileName)
	imageFileName = "image.data";
    
    fp = fopen(imageFileName, "w");

    if (!fp) return;

    /* write out the comment lines */
    for (cmt=first_comment; cmt; cmt = cmt->next)
    {
	fprintf(fp, "%s", cmt->text);
    }

    
    /* write out image_data stanza */
    fprintf(fp, "\nimage_data:\n");
    fprintf(fp, "    IMAGE_TYPE = %s\n", ImageData.image_data.IMAGE_TYPE);
    fprintf(fp, "    DATE_TIME = %s\n", ImageData.image_data.DATE_TIME);
    fprintf(fp, "    UNAME_INFO = %s\n", ImageData.image_data.UNAME_INFO);
    fprintf(fp, "    LICENSE_INFO = %s\n", ImageData.image_data.LICENSE_INFO);
    fprintf(fp, "    PRODUCT_TAPE = %s\n", ImageData.image_data.PRODUCT_TAPE);
/* USERVG_LIST is unused field
    fprintf(fp, "    USERVG_LIST = %s\n\n", ImageData.image_data.USERVG_LIST);
*/
    /* print logical volume stanza */
    fprintf(fp, "logical_volume_policy:\n");
    fprintf(fp, "    SHRINK = %s\n", ImageData.lv_policy.SHRINK);
    fprintf(fp, "    EXACT_FIT = %s\n\n", ImageData.lv_policy.EXACT_FIT);

    /* print ils data */
    fprintf(fp, "ils_data:\n");
    fprintf(fp, "    LANG=%s\n\n", ImageData.ils_data.LANG);

    /* print src disk data */
    for (sddp=ImageData.src_disk_data; sddp; sddp = sddp->next)
    {
	fprintf(fp, "source_disk_data:\n");
	fprintf(fp, "    LOCATION = %s\n", sddp->LOCATION);
	fprintf(fp, "    SIZE_MB = %s\n", sddp->SIZE);
	fprintf(fp, "    HDISKNAME = %s\n\n", sddp->HDISKNAME);
    }

    /* print volume group data */
    fprintf(fp, "vg_data:\n");
    fprintf(fp, "    VGNAME = %s\n", ImageData.vg_data.VGNAME);
    fprintf(fp, "    PPSIZE = %s\n", ImageData.vg_data.PPSIZE);
    fprintf(fp, "    VARYON = %s\n", ImageData.vg_data.VARYON);
    fprintf(fp, "    VG_SOURCE_DISK_LIST = %s\n\n", ImageData.vg_data.VG_SOURCE_DISK_LIST);

    /* print logical volume data */
    for (lvdp=ImageData.lv_data; lvdp; lvdp=lvdp->next)
    {
	fprintf(fp, "lv_data:\n");
	fprintf(fp, "    VOLUME_GROUP = %s\n", lvdp->VOLUME_GROUP);
	fprintf(fp, "    LV_SOURCE_DISK_LIST = %s\n", lvdp->LV_SOURCE_DISK_LIST);
	fprintf(fp, "    LOGICAL_VOLUME = %s\n", lvdp->LOGICAL_VOLUME);
	fprintf(fp, "    TYPE = %s\n", lvdp->TYPE);
	fprintf(fp, "    MAX_LPS = %s\n", lvdp->MAX_LPS);
	fprintf(fp, "    COPIES = %s\n", lvdp->COPIES);
	fprintf(fp, "    LPs = %s\n", lvdp->LPs);
	fprintf(fp, "    BB_POLICY = %s\n", lvdp->BB_POLICY);
	fprintf(fp, "    INTER_POLICY = %s\n", lvdp->INTER_POLICY);
	fprintf(fp, "    INTRA_POLICY = %s\n", lvdp->INTRA_POLICY);
	fprintf(fp, "    WRITE_VERIFY = %s\n", lvdp->WRITE_VERIFY);
	fprintf(fp, "    UPPER_BOUND = %s\n", lvdp->UPPER_BOUND);
	fprintf(fp, "    SCHED_POLICY = %s\n", lvdp->SCHED_POLICY);
	fprintf(fp, "    RELOCATABLE = %s\n", lvdp->RELOCATABLE);
	fprintf(fp, "    LABEL = %s\n", lvdp->LABEL);
	fprintf(fp, "    MIRROR_WRITE_CONSISTENCY = %s\n", lvdp->MIRROR_WRITE_CONSISTENCY);
	fprintf(fp, "    LV_SEPARATE_PV = %s\n", lvdp->LV_SEPARATE_PV);
	fprintf(fp, "    MAPFILE = %s\n", lvdp->MAPFILE);
	fprintf(fp, "    PP_SIZE = %s\n\n", lvdp->PP_SIZE);
    }

    /* print file system data */
    for (fsdp=ImageData.fs_data; fsdp; fsdp = fsdp->next)
    {
	fprintf(fp, "fs_data:\n");
	fprintf(fp, "    FS_NAME = %s\n", fsdp->FS_NAME);
	fprintf(fp, "    FS_SIZE = %s\n", fsdp->FS_SIZE);
	fprintf(fp, "    FS_MIN_SIZE = %s\n", fsdp->FS_MIN_SIZE);
	fprintf(fp, "    FS_LV = %s\n", fsdp->FS_LV);
	fprintf(fp, "    FS_FS = %s\n", fsdp->FS_FS);
	fprintf(fp, "    FS_COMPRESS = %s\n", fsdp->FS_COMPRESS);
	fprintf(fp, "    FS_NBPI = %s\n\n", fsdp->FS_NBPI);
    }

    /* post install data */
    fprintf(fp, "post_install_data:\n");
    fprintf(fp, "    BOSINST_FILE = %s\n\n", ImageData.post_install_data.BOSINST_FILE);
    fclose(fp);
 
}

