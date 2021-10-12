static char sccsid[] = "@(#) 82 1.10 src/bos/usr/lpp/bosinst/BosMenus/datadaemon.c, bosinst, bos41J, 9523B_all 95/06/07 09:07:09";
/*
 * COMPONENT_NAME: (BOSINST) Base Operating System Install
 *
 * FUNCTIONS: datadaemon
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
 * NAME: DataDaemon.c
 *
 * FUNCTION: read and maintain bosinst.data and image.data files
 *
 * ENVIRONMENT:
 *	BOSINSTDATA - bosinst.data path
 *	IMAGEDATA - image.data path
 * NOTES:
 *
 * RETURNS:
 *	Data response codes:
 *	0 - OK; data in response buffer
 *	1 - error
 *	2 - OK, no data in response buffer
 *
 *	exit value:
 *	0 - always
 */

#include <stdio.h>
#include <fcntl.h>
#include "BosInst.h"
#include "ImageDat.h"
#include "bidata.h"

struct BosInst BosData;			/* bosinst.data file    */
struct imageDat ImageData;		/* image.data file      */
struct disk_info *head =  0;		/* disk info ptr        */

int datacheck = 0;			/* data integrety flag  */

main(
int argc,				/* arg count		*/
char *argv[])				/* arg vectors		*/
{

    int rfd, wfd;			/* read and write fds */
    struct bidata command;
    struct ddresponse response;
    int rc;
    static int image_status;


    /* make the pipes */
    rc=mkfifo(BIDATA_COMMAND, 0777);
    if (rc < 0)
	perror("mkfifo command");
    rc = mkfifo(BIDATA_RESPONSE, 0777);
    if (rc < 0)
	perror("mkfifo response");
    
    /* read the image and data files */
    read_bosinstdata();
    read_imagedata();

    rfd = open(BIDATA_COMMAND, O_RDWR);
    wfd = open(BIDATA_RESPONSE, O_RDWR);

    image_status = verify_imagedata();

    while (1)
    {
	/* read the command request and clear the response buffer */
	read(rfd, &command, sizeof(struct bidata));
	bzero(&response, sizeof(struct ddresponse));

#if DEBUG
	fprintf(stderr,"dd: command=%d  s=<%s> f=<%s> v=<%s> af=<%s> av=<%s>\n",
	command.type, command.stanza, command.field, command.value, command.auxfield, command.auxvalue);
#endif
	switch(command.type)
	{
	case GET_BOSINST_FIELD:

	    get_bidata(&command, &response);
	    break;

	case GET_IMAGEDATA_FIELD:
	    get_imdata(&command, &response);
	    break;

	case CHANGE_BOSINST_FIELD:
	    change_bidata(&command, &response);
	    break;

	case CHANGE_IMAGEDATA_FIELD:
	    change_imdata(&command, &response);
	    break;

	case DELETE_BOSINST_DISK:
	    del_bidisk(&command, &response);
	    break;

	case ADD_BOSINST_DISK:
	    add_bidisk(&command, &response);
	    break;

	case GET_ALL_DISKS:
	    get_all(&command, &response);
	    break;

	case DEL_ALL_DISKS:
	    del_all(&command, &response);
	    break;

	case GET_STATUS:
	    sprintf(response.data, "%d", datacheck);
	    break;

	default:
	    if ((command.type & WRITE_FILES) == WRITE_FILES)
	    {
		response.rc = 2;
		write_bosinstdata();
		write_imagedata();
	    }
	    if ((command.type & EXIT) == EXIT)
	    {
		response.rc = 2;
		write(wfd, &response, sizeof(struct ddresponse));
		unlink(BIDATA_COMMAND);
		unlink(BIDATA_RESPONSE);
		exit(0);
	    }

	    
	}
	write(wfd, &response, sizeof(struct ddresponse));
    }


    exit(0);
}


/*
 * NAME: get_bidata
 *
 * FUNCTION: extract bosinst.data field
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *	Search for the requested field
 *
 * RETURNS:
 *	0 - couldn't find data
 *	1 - OK
 */

int get_bidata(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    

    /* optimistically set return code */
    rsp->rc = 0;

    /* DO they want controil flow? */
    if (!strncmp(cmd->stanza, "control_flow", 12))
    {
	if (!strncmp(cmd->field, "CONSOLE", 7))
	{
	    strcpy(rsp->data, BosData.control_flow.CONSOLE);
	    return 1;
	}
	if (!strncmp(cmd->field, "INSTALL_METHOD", 14))
	{
	    strcpy(rsp->data, BosData.control_flow.INSTALL_METHOD);
	    return 1;
	}

	if (!strncmp(cmd->field, "PROMPT", 6))
	{
	    strcpy(rsp->data, BosData.control_flow.PROMPT);
	    return 1;
	}
	if (!strncmp(cmd->field, "EXISTING_SYSTEM_OVERWRITE", 25))
	{
	    strcpy(rsp->data, BosData.control_flow.EXISTING_SYSTEM_OVERWRITE);
	    return 1;
	}
	if (!strncmp(cmd->field, "INSTALL_X_IF_ADAPTER", 20))
	{
	    strcpy(rsp->data, BosData.control_flow.INSTALL_X_IF_ADAPTER);
	    return 1;
	}
	if (!strncmp(cmd->field, "RUN_STARTUP", 11))
	{
	    strcpy(rsp->data, BosData.control_flow.RUN_STARTUP);
	    return 1;
	}
	if (!strncmp(cmd->field, "RM_INST_ROOTS", 13))
	{
	    strcpy(rsp->data, BosData.control_flow.RM_INST_ROOTS);
	    return 1;
	}
	if (!strncmp(cmd->field, "ERROR_EXIT", 10))
	{
	    strcpy(rsp->data, BosData.control_flow.ERROR_EXIT);
	    return 1;
	}
	if (!strncmp(cmd->field, "CUSTOMIZATION_FILE", 18))
	{
	    strcpy(rsp->data, BosData.control_flow.CUSTOMIZATION);
	    return 1;
	}
	if (!strncmp(cmd->field, "TCB", 3))
	{
	    strcpy(rsp->data, BosData.control_flow.TCB);
	    return 1;
	}
	if (!strncmp(cmd->field, "INSTALL_TYPE", 12))
	{
	    strcpy(rsp->data, BosData.control_flow.INSTALL_TYPE);
	    return 1;
	}
	if (!strncmp(cmd->field, "BUNDLES", 7))
	{
	    strcpy(rsp->data, BosData.control_flow.BUNDLES);
	    return 1;
	}
    }

    /* Do they want target disk data? */
    if (!strncmp(cmd->stanza, "target_disk_data", 16))
    {
	struct target_disk_data *tddp;

	/* Get all the tdd stanzas.  If there is a criteria,
	 * get only the one which matches
	 */
	for (tddp = BosData.targets; tddp; tddp=tddp->next)
	{
	    /* do criteria */
	    if (cmd->auxfield[0])
	    {
		if (!strcmp(cmd->auxfield, "LOCATION"))
		{
		    /* skip if the location doesn't match */
		    if (strcmp(cmd->auxvalue, tddp->LOCATION))
			continue;
		}
		if (!strcmp(cmd->auxfield, "SIZE_MB"))
		{
		    /* skip if the size doesn't match */
		    if (strcmp(cmd->auxvalue, tddp->SIZE_MB))
			continue;
		}
		if (!strcmp(cmd->auxfield, "HDISKNAME"))
		{
		    /* skip if the hdiskname doesn't match */
		    if (strcmp(cmd->auxvalue, tddp->HDISKNAME))
			continue;
		}
	    }

	    /* cat the requested field into the reply buffer */
	    if (!strcmp(cmd->field, "LOCATION"))
	    {
		strcat(rsp->data, tddp->LOCATION);
		strcat(rsp->data, " ");
		    continue;
	    }
	    if (!strcmp(cmd->field, "SIZE_MB"))
	    {
		strcat(rsp->data, tddp->SIZE_MB);
		strcat(rsp->data, " ");
		    continue;
	    }
	    if (!strcmp(cmd->field, "HDISKNAME"))
	    {
		strcat(rsp->data, tddp->HDISKNAME);
		strcat(rsp->data, " ");
		    continue;
	    }
	}
	/* remove last " " if it's there */
	if (*(rsp->data + strlen(rsp->data)-1) == ' ')
	    *(rsp->data + strlen(rsp->data)-1) = '\0';

	return 1;
    }

    /* do they want the locale stanza? */
    if (!strncmp(cmd->stanza, "locale", 6))
    {
	if (!strncmp(cmd->field, "BOSINST_LANG", 12))
	{
	    strcpy(rsp->data, BosData.locale.BOSINST_LANG);
	}
	if (!strncmp(cmd->field, "CULTURAL_CONVENTION", 19))
	{
	    strcpy(rsp->data, BosData.locale.CULTURAL_CONVENTION);
	}
	if (!strncmp(cmd->field, "MESSAGES", 8))
	{
	    strcpy(rsp->data, BosData.locale.MESSAGES);
	}
	if (!strncmp(cmd->field, "KEYBOARD", 8))
	{
	    strcpy(rsp->data, BosData.locale.KEYBOARD);
	}
	return 1;
    }
    return 0;
}


/*
 * NAME: change_bidata
 *
 * FUNCTION: chang bosinst.data field
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *	Search for the requested field
 *
 * RETURNS:
 *	0 - field not found
 *	1 - data changed
 */

int change_bidata(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    
    /* optimistically set return code */
    rsp->rc = 2;

    /* DO they want controil flow? */
    if (!strncmp(cmd->stanza, "control_flow", 12))
    {
	if (!strncmp(cmd->field, "CONSOLE", 7))
	{
	    strcpy(BosData.control_flow.CONSOLE, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "INSTALL_METHOD", 14))
	{
	    strcpy( BosData.control_flow.INSTALL_METHOD, cmd->value);
	    return 1;
	}

	if (!strncmp(cmd->field, "PROMPT", 6))
	{
	    strcpy( BosData.control_flow.PROMPT, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "EXISTING_SYSTEM_OVERWRITE", 25))
	{
	    strncpy( BosData.control_flow.EXISTING_SYSTEM_OVERWRITE, cmd->value, 4);
	    return 1;
	}
	if (!strncmp(cmd->field, "INSTALL_X_IF_ADAPTER", 20))
	{
	    strncpy( BosData.control_flow.INSTALL_X_IF_ADAPTER, cmd->value, 4);
	    return 1;
	}
	if (!strncmp(cmd->field, "RUN_STARTUP", 11))
	{
	    strcpy( BosData.control_flow.RUN_STARTUP, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "RM_INST_ROOTS", 13))
	{
	    strncpy( BosData.control_flow.RM_INST_ROOTS, cmd->value, 4);
	    return 1;
	}
	if (!strncmp(cmd->field, "ERROR_EXIT", 10))
	{
	    strcpy( BosData.control_flow.ERROR_EXIT, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "CUSTOMIZATION_FILE", 18))
	{
	    strcpy( BosData.control_flow.CUSTOMIZATION, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "TCB", 3))
	{
	    strcpy( BosData.control_flow.TCB, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "INSTALL_TYPE", 12))
	{
	    strcpy( BosData.control_flow.INSTALL_TYPE, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "BUNDLES", 7))
	{
	    strncpy( BosData.control_flow.BUNDLES, cmd->value, 79);
	    return 1;
	}
    }

    /* Do they want target disk data? */
    if (!strncmp(cmd->stanza, "target_disk_data", 16))
    {
	struct target_disk_data *tddp;

	/* Get all the tdd stanzas.  If there is a criteria,
	 * get only the one which matches
	 */
	for (tddp = BosData.targets; tddp; tddp=tddp->next)
	{
	    /* do criteria */
	    if (cmd->auxfield[0])
	    {
		if (!strcmp(cmd->auxfield, "LOCATION"))
		{
		    /* skip if the location doesn't match */
		    if (strcmp(cmd->auxvalue, tddp->LOCATION))
			continue;
		}
		if (!strcmp(cmd->auxfield, "SIZE_MB"))
		{
		    /* skip if the size doesn't match */
		    if (strcmp(cmd->auxvalue, tddp->SIZE_MB))
			continue;
		}
		if (!strcmp(cmd->auxfield, "HDISKNAME"))
		{
		    /* skip if the hdiskname doesn't match */
		    if (strcmp(cmd->auxvalue, tddp->HDISKNAME))
			continue;
		}
	    }

	    /* cat the requested field into the reply buffer */
	    if (!strcmp(cmd->field, "LOCATION"))
	    {
		strcpy( tddp->LOCATION, cmd->value);
		    continue;
	    }
	    if (!strcmp(cmd->field, "SIZE_MB"))
	    {
		strcpy( tddp->SIZE_MB, cmd->value);
		    continue;
	    }
	    if (!strcmp(cmd->field, "HDISKNAME"))
	    {
		strcpy( tddp->HDISKNAME, cmd->value);
		    continue;
	    }
	}
	return 1;
    }

    /* do they want the locale stanza? */
    if (!strncmp(cmd->stanza, "locale", 6))
    {
	if (!strncmp(cmd->field, "BOSINST_LANG", 12))
	{
	    strcpy( BosData.locale.BOSINST_LANG, cmd->value);
	}
	else if (!strncmp(cmd->field, "CULTURAL_CONVENTION", 19))
	{
	    strcpy( BosData.locale.CULTURAL_CONVENTION, cmd->value);
	}
	else if (!strncmp(cmd->field, "MESSAGES", 8))
	{
	    strcpy( BosData.locale.MESSAGES, cmd->value);
	}
	else if (!strncmp(cmd->field, "KEYBOARD", 8))
	{
	    strcpy( BosData.locale.KEYBOARD, cmd->value);
	}
	return 1;
    }
    return 0;
}


/*
 * NAME: get_imdata
 *
 * FUNCTION: extract bosinst.data field
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *	Search for the requested field
 *
 * RETURNS:
 *	0 - data not found
 *	1 - data found
 */

int get_imdata(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    
    /* optimistically set return code */
    rsp->rc = 0;

    /* DO they want image data? */
    if (!strncmp(cmd->stanza, "image_data", 10))
    {
	/* What field? */
	if (!strncmp(cmd->field, "IMAGE_TYPE", 10))
	{
	    strcpy(rsp->data, ImageData.image_data.IMAGE_TYPE);
	    return 1;
	}
	if (!strncmp(cmd->field, "DATE_TIME", 9))
	{
	    strcpy(rsp->data, ImageData.image_data.DATE_TIME);
	    return 1;
	}
	if (!strncmp(cmd->field, "UNAME_INFO", 10))
	{
	    strcpy(rsp->data, ImageData.image_data.UNAME_INFO);
	    return 1;
	}
	if (!strncmp(cmd->field, "LICENSE_INFO", 12))
	{
	    strcpy(rsp->data, ImageData.image_data.LICENSE_INFO);
	    return 1;
	}
	if (!strncmp(cmd->field, "PRODUCT_TAPE", 12))
	{
	    strcpy(rsp->data, ImageData.image_data.PRODUCT_TAPE);
	    return 1;
	}
/* USERVG_LIST is unused field
	if (!strncmp(cmd->field, "USERVG_LIST", 11))
	{
	    strcpy(rsp->data, ImageData.image_data.USERVG_LIST);
	    return 1;
	}
*/
    }

    /* Do they want logical_volume_policy? */
    if (!strncmp(cmd->stanza, "logical_volume_policy", 21))
    {
	/* What field? */
	if (!strncmp(cmd->field, "SHRINK", 6))
	{
	    strcpy(rsp->data, ImageData.lv_policy.SHRINK);
	    return 1;
	}
	if (!strncmp(cmd->field, "EXACT_FIT", 9))
	{
	    strcpy(rsp->data, ImageData.lv_policy.EXACT_FIT);
	    return 1;
	}
    }

    /* Do they want ils_data? */
    if (!strncmp(cmd->stanza, "ils_data", 8))
    {
	/* What field? */
	if (!strncmp(cmd->field, "LANG", 4))
	{
	    strcpy(rsp->data, ImageData.ils_data.LANG);
	    return 1;
	}

    }
    /* Do they want source disk data? */
    if (!strncmp(cmd->stanza, "source_disk_data", 16))
    {
	struct src_disk_data *sddp;
	for (sddp=ImageData.src_disk_data; sddp; sddp = sddp->next)
	{

	    /* do the cirteria first */
	    if (cmd->auxfield[0])
	    {
		/* what field is criteria? */
		if (!strncmp(cmd->auxfield,  "HDISKNAME",9))
		{
		    if (strcmp(cmd->auxvalue, sddp->HDISKNAME))
			continue;
		}

		if (!strncmp(cmd->auxfield, "LOCATION", 8))
		{
		    if (strcmp(cmd->auxvalue, sddp->LOCATION))
			continue;
		}
		if (!strncmp(cmd->auxfield, "SIZE_MB", 9))
		{
		    if (strcmp(cmd->auxvalue, sddp->SIZE))
			continue;
		}
	    }

	    /* passed the criteria, load up the requested field */
	    if (!strncmp(cmd->field,  "HDISKNAME",9))
	    {
		strcat(rsp->data, sddp->HDISKNAME);

		/* add a space for multiple field requests */
		strcat(rsp->data, " ");		
		continue;
	    }

	    if (!strncmp(cmd->field,  "LOCATION",8))
	    {
		strcat(rsp->data, sddp->LOCATION);

		/* add a space for multiple field requests */
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field,  "SIZE_MB",9))
	    {
		strcat(rsp->data, sddp->SIZE);

		/* add a space for multiple field requests */
		strcat(rsp->data, " ");
		continue;
	    }

	}
	/* remove last " " if it's there */
	if (*(rsp->data + strlen(rsp->data)-1) == ' ')
	    *(rsp->data + strlen(rsp->data)-1) = '\0';
	return 1;
    }

    /* Do they want vg_data? */
    if (!strncmp(cmd->stanza, "vg_data", 7))
    {
	/* What field? */
	if (!strncmp(cmd->field, "VGNAME", 6))
	{
	    strcpy(rsp->data, ImageData.vg_data.VGNAME);
	    return 1;
	}
	if (!strncmp(cmd->field, "PPSIZE", 6))
	{
	    strcpy(rsp->data, ImageData.vg_data.PPSIZE);
	    return 1;
	}

	if (!strncmp(cmd->field, "VARYON", 6))
	{
	    strcpy(rsp->data, ImageData.vg_data.VARYON);
	    return 1;
	}

	if (!strncmp(cmd->field, "VG_SOURCE_DISK_LIST", 19))
	{
	    strcpy(rsp->data, ImageData.vg_data.VG_SOURCE_DISK_LIST);
	    return 1;
	}
    }

    /* Do they want lv_data? */
    if (!strcmp(cmd->stanza, "lv_data", 7))
    {
	struct lv_data *lvdp;
	for(lvdp=ImageData.lv_data; lvdp; lvdp=lvdp->next)
	{
	    /* do criteria first */
	    if (cmd->auxfield[0])
	    {
		if (!strncmp(cmd->auxfield, "VOLUME_GROUP", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->VOLUME_GROUP))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LV_SOURCE_DISK_LIST", 19))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LV_SOURCE_DISK_LIST))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LOGICAL_VOLUME", 14))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LOGICAL_VOLUME))
			continue;
		}
		if (!strncmp(cmd->auxfield, "TYPE", 4))
		{
		    if (strcmp(cmd->auxvalue, lvdp->TYPE))
			continue;
		}
		if (!strncmp(cmd->auxfield, "MAX_LPS", 7))
		{
		    if (strcmp(cmd->auxvalue, lvdp->MAX_LPS))
			continue;
		}
		if (!strncmp(cmd->auxfield, "COPIES", 6))
		{
		    if (strcmp(cmd->auxvalue, lvdp->COPIES))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LPs", 3))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LPs))
			continue;
		}
		if (!strncmp(cmd->auxfield, "BB_POLICY", 9))
		{
		    if (strcmp(cmd->auxvalue, lvdp->BB_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "INTER_POLICY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->INTER_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "INTRA_POLICY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->INTRA_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "WRITE_VERIFY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->WRITE_VERIFY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "UPPER_BOUND", 11))
		{
		    if (strcmp(cmd->auxvalue, lvdp->UPPER_BOUND))
			continue;
		}
		if (!strncmp(cmd->auxfield, "SCHED_POLICY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->SCHED_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "RELOCATABLE", 11))
		{
		    if (strcmp(cmd->auxvalue, lvdp->RELOCATABLE))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LABEL", 5))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LABEL))
			continue;
		}
		if (!strncmp(cmd->auxfield, "MIRROR_WRITE_CONSISTENCY", 24))
		{
		    if (strcmp(cmd->auxvalue, lvdp->MIRROR_WRITE_CONSISTENCY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LV_SEPARATE_PV", 14))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LV_SEPARATE_PV))
			continue;
		}
		if (!strncmp(cmd->auxfield, "MAPFILE", 7))
		{
		    if (strcmp(cmd->auxvalue, lvdp->MAPFILE))
			continue;
		}
		if (!strncmp(cmd->auxfield, "PP_SIZE", 7))
		{
		    if (strcmp(cmd->auxvalue, lvdp->PP_SIZE))
			continue;
		}
	    }

	    /* made it past the criteria, so copy the request field over */
	    if (!strncmp(cmd->field, "VOLUME_GROUP", 12))
	    {
		strcat(rsp->data, lvdp->VOLUME_GROUP);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "LV_SOURCE_DISK_LIST", 19))
	    {
		strcat(rsp->data, lvdp->LV_SOURCE_DISK_LIST);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "LOGICAL_VOLUME", 14))
	    {
		strcat(rsp->data, lvdp->LOGICAL_VOLUME);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "TYPE", 4))
	    {
		strcat(rsp->data, lvdp->TYPE);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "MAX_LPS", 7))
	    {
		strcat(rsp->data, lvdp->MAX_LPS);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "COPIES", 6))
	    {
		strcat(rsp->data, lvdp->COPIES);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "LPs", 3))
	    {
		strcat(rsp->data, lvdp->LPs);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "BB_POLICY", 9))
	    {
		strcat(rsp->data, lvdp->BB_POLICY);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "INTER_POLICY", 12))
	    {
		strcat(rsp->data, lvdp->INTER_POLICY);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "INTRA_POLICY", 12))
	    {
		strcat(rsp->data, lvdp->INTRA_POLICY);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "WRITE_VERIFY", 12))
	    {
		strcat(rsp->data, lvdp->WRITE_VERIFY);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "UPPER_BOUND", 11))
	    {
		strcat(rsp->data, lvdp->UPPER_BOUND);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "SCHED_POLICY", 12))
	    {
		strcat(rsp->data, lvdp->SCHED_POLICY);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "RELOCATABLE", 11))
	    {
		strcat(rsp->data, lvdp->RELOCATABLE);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "LABEL", 5))
	    {
		strcat(rsp->data, lvdp->LABEL);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "MIRROR_WRITE_CONSISTENCY", 24))
	    {
		strcat(rsp->data, lvdp->MIRROR_WRITE_CONSISTENCY);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "LV_SEPARATE_PV", 14))
	    {
		strcat(rsp->data, lvdp->LV_SEPARATE_PV);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "MAPFILE", 7))
	    {
		strcat(rsp->data, lvdp->MAPFILE);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "PP_SIZE", 7))
	    {
		strcat(rsp->data, lvdp->PP_SIZE);
		strcat(rsp->data, " ");
		continue;
	    }
	}
	/* remove last " " if it's there */
	if (*(rsp->data + strlen(rsp->data)-1) == ' ')
	    *(rsp->data + strlen(rsp->data)-1) = '\0';
	return 1;
    }

    /* Do they want fs_data? */
    if (!strncmp(cmd->stanza, "fs_data", 7))
    {
	struct fs_data *fsdp;

	for (fsdp=ImageData.fs_data; fsdp; fsdp=fsdp->next)
	{
	    /* do the criteria first */
	    if (cmd->auxfield[0])
	    {
		if (!strncmp(cmd->auxfield, "FS_NAME", 7))
		    if (strcmp(cmd->auxvalue, fsdp->FS_NAME))
			continue;
		if (!strncmp(cmd->auxfield, "FS_SIZE", 7))
		    if (strcmp(cmd->auxvalue, fsdp->FS_SIZE))
			continue;
		if (!strncmp(cmd->auxfield, "FS_MIN_SIZE", 11))
		    if (strcmp(cmd->auxvalue, fsdp->FS_MIN_SIZE))
			continue;
		if (!strncmp(cmd->auxfield, "FS_LV", 5))
		    if (strcmp(cmd->auxvalue, fsdp->FS_LV))
			continue;
		if (!strncmp(cmd->auxfield, "FS_FS", 5))
		    if (strcmp(cmd->auxvalue, fsdp->FS_FS))
			continue;
		if (!strncmp(cmd->auxfield, "FS_NBPI", 7))
		    if (strcmp(cmd->auxvalue, fsdp->FS_NBPI))
			continue;
		if (!strncmp(cmd->auxfield, "FS_COMPRESS", 11))
		    if (strcmp(cmd->auxvalue, fsdp->FS_COMPRESS))
			continue;
	    }

	    /* past the criteria, copy then over */
	    if (!strncmp(cmd->field, "FS_NAME", 7))
	    {
		strcat(rsp->data, fsdp->FS_NAME);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_SIZE", 7))
	    {
		strcat(rsp->data, fsdp->FS_SIZE);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_MIN_SIZE", 7))
	    {
		strcat(rsp->data, fsdp->FS_MIN_SIZE);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_LV", 7))
	    {
		strcat(rsp->data, fsdp->FS_LV);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_FS", 7))
	    {
		strcat(rsp->data, fsdp->FS_FS);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_NBPI", 7))
	    {
		strcat(rsp->data, fsdp->FS_NBPI);
		strcat(rsp->data, " ");
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_COMPRESS", 11))
	    {
		strcat(rsp->data, fsdp->FS_COMPRESS);
		strcat(rsp->data, " ");
		continue;
	    }
	}
	/* remove last " " if it's there */
	if (*(rsp->data + strlen(rsp->data)-1) == ' ')
	    *(rsp->data + strlen(rsp->data)-1) = '\0';
	return 1;
    }
    /* do they want post install data? */
    if (!strncmp(cmd->stanza, "post_install_data", 17))
    {
	if (!strncmp(cmd->field, "BOSINST_FILE", 12))
	{
	    strcpy(rsp->data, ImageData.post_install_data.BOSINST_FILE);
	}
	return 1;
    }
    return 0;
}

/*
 * NAME: change_imdata
 *
 * FUNCTION: change image.data field
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *	Search for the requested field and change it.  Stanzas which are
 *	not unique will change the field for all stanzas unless a criteria
 *	is used.
 *
 * RETURNS:
 *	1 - always
 */

int change_imdata(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    
    /* optimistically set return code */
    rsp->rc = 2;

    /* DO they want image data? */
    if (!strncmp(cmd->stanza, "image_data", 10))
    {
	/* What field? */
	if (!strncmp(cmd->field, "IMAGE_TYPE", 10))
	{
	    strcpy( ImageData.image_data.IMAGE_TYPE, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "DATE_TIME", 9))
	{
	    strcpy( ImageData.image_data.DATE_TIME, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "UNAME_INFO", 10))
	{
	    strcpy( ImageData.image_data.UNAME_INFO, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "LICENSE_INFO", 12))
	{
	    strcpy( ImageData.image_data.LICENSE_INFO, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "PRODUCT_TAPE", 12))
	{
	    strcpy( ImageData.image_data.PRODUCT_TAPE, cmd->value);
	    return 1;
	}
/* USERVG_LIST is unused field
	if (!strncmp(cmd->field, "USERVG_LIST", 11))
	{
	    strcpy( ImageData.image_data.USERVG_LIST, cmd->value);
	    return 1;
	}
*/
    }

    /* Do they want logical_volume_policy? */
    if (!strncmp(cmd->stanza, "logical_volume_policy", 21))
    {
	/* What field? */
	if (!strncmp(cmd->field, "SHRINK", 6))
	{
	    strcpy( ImageData.lv_policy.SHRINK, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "EXACT_FIT", 9))
	{
	    strcpy( ImageData.lv_policy.EXACT_FIT, cmd->value);
	    return 1;
	}
    }

    /* Do they want ils_data? */
    if (!strncmp(cmd->stanza, "ils_data", 8))
    {
	/* What field? */
	if (!strncmp(cmd->field, "LANG", 4))
	{
	    strcpy( ImageData.ils_data.LANG, cmd->value);
	    return 1;
	}

    }
    /* Do they want source disk data? */
    if (!strncmp(cmd->stanza, "source_disk_data", 16))
    {
	struct src_disk_data *sddp;
	for (sddp=ImageData.src_disk_data; sddp; sddp = sddp->next)
	{

	    /* do the cirteria first */
	    if (cmd->auxfield[0])
	    {
		/* what field is criteria? */
		if (!strncmp(cmd->auxfield,  "HDISKNAME",9))
		{
		    if (strcmp(cmd->auxvalue, sddp->HDISKNAME))
			continue;
		}

		if (!strncmp(cmd->auxfield, "LOCATION", 8))
		{
		    if (strcmp(cmd->auxvalue, sddp->LOCATION))
			continue;
		}
		if (!strncmp(cmd->auxfield, "SIZE_MB", 7))
		{
		    if (strcmp(cmd->auxvalue, sddp->SIZE))
			continue;
		}
	    }

	    /* passed the criteria, load up the requested field */
	    if (!strncmp(cmd->field,  "HDISKNAME",9))
	    {
		strcpy( sddp->HDISKNAME, cmd->value);
		continue;
	    }

	    if (!strncmp(cmd->field,  "LOCATION",8))
	    {
		strcpy( sddp->LOCATION, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field,  "SIZE_MB",7))
	    {
		strcpy( sddp->SIZE, cmd->value);
		continue;
	    }

	}
	return 1;
    }

    /* Do they want vg_data? */
    if (!strncmp(cmd->stanza, "vg_data", 7))
    {
	/* What field? */
	if (!strncmp(cmd->field, "VGNAME", 6))
	{
	    strcpy( ImageData.vg_data.VGNAME, cmd->value);
	    return 1;
	}
	if (!strncmp(cmd->field, "PPSIZE", 6))
	{
	    strcpy( ImageData.vg_data.PPSIZE, cmd->value);
	    return 1;
	}

	if (!strncmp(cmd->field, "VARYON", 6))
	{
	    strcpy( ImageData.vg_data.VARYON, cmd->value);
	    return 1;
	}

	if (!strncmp(cmd->field, "VG_SOURCE_DISK_LIST", 19))
	{
	    strcpy( ImageData.vg_data.VG_SOURCE_DISK_LIST, cmd->value);
	    return 1;
	}
    }

    /* Do they want lv_data? */
    if (!strcmp(cmd->stanza, "lv_data", 7))
    {
	struct lv_data *lvdp;
	for(lvdp=ImageData.lv_data; lvdp; lvdp=lvdp->next)
	{
	    /* do criteria first */
	    if (cmd->auxfield[0])
	    {
		if (!strncmp(cmd->auxfield, "VOLUME_GROUP", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->VOLUME_GROUP))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LV_SOURCE_DISK_LIST", 19))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LV_SOURCE_DISK_LIST))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LOGICAL_VOLUME", 14))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LOGICAL_VOLUME))
			continue;
		}
		if (!strncmp(cmd->auxfield, "TYPE", 4))
		{
		    if (strcmp(cmd->auxvalue, lvdp->TYPE))
			continue;
		}
		if (!strncmp(cmd->auxfield, "MAX_LPS", 7))
		{
		    if (strcmp(cmd->auxvalue, lvdp->MAX_LPS))
			continue;
		}
		if (!strncmp(cmd->auxfield, "COPIES", 6))
		{
		    if (strcmp(cmd->auxvalue, lvdp->COPIES))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LPs", 3))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LPs))
			continue;
		}
		if (!strncmp(cmd->auxfield, "BB_POLICY", 9))
		{
		    if (strcmp(cmd->auxvalue, lvdp->BB_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "INTER_POLICY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->INTER_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "INTRA_POLICY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->INTRA_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "WRITE_VERIFY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->WRITE_VERIFY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "UPPER_BOUND", 11))
		{
		    if (strcmp(cmd->auxvalue, lvdp->UPPER_BOUND))
			continue;
		}
		if (!strncmp(cmd->auxfield, "SCHED_POLICY", 12))
		{
		    if (strcmp(cmd->auxvalue, lvdp->SCHED_POLICY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "RELOCATABLE", 11))
		{
		    if (strcmp(cmd->auxvalue, lvdp->RELOCATABLE))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LABEL", 5))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LABEL))
			continue;
		}
		if (!strncmp(cmd->auxfield, "MIRROR_WRITE_CONSISTENCY", 24))
		{
		    if (strcmp(cmd->auxvalue, lvdp->MIRROR_WRITE_CONSISTENCY))
			continue;
		}
		if (!strncmp(cmd->auxfield, "LV_SEPARATE_PV", 14))
		{
		    if (strcmp(cmd->auxvalue, lvdp->LV_SEPARATE_PV))
			continue;
		}
		if (!strncmp(cmd->auxfield, "MAPFILE", 7))
		{
		    if (strcmp(cmd->auxvalue, lvdp->MAPFILE))
			continue;
		}
		if (!strncmp(cmd->auxfield, "PP_SIZE", 7))
		{
		    if (strcmp(cmd->auxvalue, lvdp->PP_SIZE))
			continue;
		}
	    }

	    /* made it past the criteria, so copy the request field over */
	    if (!strncmp(cmd->field, "VOLUME_GROUP", 12))
	    {
		strcpy( lvdp->VOLUME_GROUP, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "LV_SOURCE_DISK_LIST", 19))
	    {
		strcpy( lvdp->LV_SOURCE_DISK_LIST, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "LOGICAL_VOLUME", 14))
	    {
		strcpy( lvdp->LOGICAL_VOLUME, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "TYPE", 4))
	    {
		strcpy( lvdp->TYPE, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "MAX_LPS", 7))
	    {
		strcpy( lvdp->MAX_LPS, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "COPIES", 6))
	    {
		strcpy( lvdp->COPIES, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "LPs", 3))
	    {
		strcpy( lvdp->LPs, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "BB_POLICY", 9))
	    {
		strcpy( lvdp->BB_POLICY, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "INTER_POLICY", 12))
	    {
		strcpy( lvdp->INTER_POLICY, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "INTRA_POLICY", 12))
	    {
		strcpy( lvdp->INTRA_POLICY, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "WRITE_VERIFY", 12))
	    {
		strcpy( lvdp->WRITE_VERIFY, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "UPPER_BOUND", 11))
	    {
		strcpy( lvdp->UPPER_BOUND, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "SCHED_POLICY", 12))
	    {
		strcpy( lvdp->SCHED_POLICY, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "RELOCATABLE", 11))
	    {
		strcpy( lvdp->RELOCATABLE, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "LABEL", 5))
	    {
		strcpy( lvdp->LABEL, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "MIRROR_WRITE_CONSISTENCY", 24))
	    {
		strcpy( lvdp->MIRROR_WRITE_CONSISTENCY, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "LV_SEPARATE_PV", 14))
	    {
		strcpy( lvdp->LV_SEPARATE_PV, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "MAPFILE", 7))
	    {
		strcpy( lvdp->MAPFILE, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "PP_SIZE", 7))
	    {
		strcpy( lvdp->PP_SIZE, cmd->value);
		continue;
	    }
	}
	return 1;
    }

    /* Do they want fs_data? */
    if (!strncmp(cmd->stanza, "fs_data", 7))
    {
	struct fs_data *fsdp;

	for (fsdp=ImageData.fs_data; fsdp; fsdp=fsdp->next)
	{
	    /* do the criteria first */
	    if (cmd->auxfield[0])
	    {
		if (!strncmp(cmd->auxfield, "FS_NAME", 7))
		    if (strcmp(cmd->auxvalue, fsdp->FS_NAME))
			continue;
		if (!strncmp(cmd->auxfield, "FS_SIZE", 7))
		    if (strcmp(cmd->auxvalue, fsdp->FS_SIZE))
			continue;
		if (!strncmp(cmd->auxfield, "FS_MIN_SIZE", 11))
		    if (strcmp(cmd->auxvalue, fsdp->FS_MIN_SIZE))
			continue;
		if (!strncmp(cmd->auxfield, "FS_LV", 5))
		    if (strcmp(cmd->auxvalue, fsdp->FS_LV))
			continue;
		if (!strncmp(cmd->auxfield, "FS_FS", 5))
		    if (strcmp(cmd->auxvalue, fsdp->FS_FS))
			continue;
		if (!strncmp(cmd->auxfield, "FS_NBPI", 7))
		    if (strcmp(cmd->auxvalue, fsdp->FS_NBPI))
			continue;
		if (!strncmp(cmd->auxfield, "FS_COMPRESS", 11))
		    if (strcmp(cmd->auxvalue, fsdp->FS_COMPRESS))
			continue;
	    }

	    /* past the criteria, copy then over */
	    if (!strncmp(cmd->field, "FS_NAME", 7))
	    {
		strcpy( fsdp->FS_NAME, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_SIZE", 7))
	    {
		strcpy( fsdp->FS_SIZE, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_MIN_SIZE", 11))
	    {
		strcpy( fsdp->FS_MIN_SIZE, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_LV", 5))
	    {
		strcpy( fsdp->FS_LV, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_FS", 5))
	    {
		strcpy( fsdp->FS_FS, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_NBPI", 7))
	    {
		strcpy( fsdp->FS_NBPI, cmd->value);
		continue;
	    }
	    if (!strncmp(cmd->field, "FS_COMPRESS", 11))
	    {
		strcpy( fsdp->FS_COMPRESS, cmd->value);
		continue;
	    }
	}
	return 1;
    }

    /* do they want post install data? */
    if (!strncmp(cmd->stanza, "post_install_data", 17))
    {
	if (!strncmp(cmd->field, "BOSINST_FILE", 12))
	{
	    strcpy(ImageData.post_install_data.BOSINST_FILE, cmd->value);
	}
	return 1;
    }
    return 0;

}

/*
 * NAME: del_bidata
 *
 * FUNCTION: delete bosinst.data target disk data stanza
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *	1 - always
 */

int del_bidisk(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    
    struct target_disk_data *tddp;
    /* optimistically set return code */
    rsp->rc = 2;


    /* Get all the tdd stanzas.  If there is a criteria,
     * get only the one which matches
     */
    for (tddp = BosData.targets; tddp; tddp=tddp->next)
    {
	/* do criteria */
	if (cmd->auxfield[0])
	{
	    if (!strcmp(cmd->auxfield, "LOCATION"))
	    {
		/* skip if the location doesn't match */
		if (strcmp(cmd->auxvalue, tddp->LOCATION))
		    continue;
	    }
	    if (!strcmp(cmd->auxfield, "SIZE_MB"))
	    {
		/* skip if the size doesn't match */
		if (strcmp(cmd->auxvalue, tddp->SIZE_MB))
		    continue;
	    }
	    if (!strcmp(cmd->auxfield, "HDISKNAME"))
	    {
		/* skip if the hdiskname doesn't match */
		if (strcmp(cmd->auxvalue, tddp->HDISKNAME))
		    continue;
	    }
	}
	/* MUST have criteria for delete */
	else
	   break;

	/* if we got here, one of the criteria must have succeded,
	 * so break
	 */
	break;

    }

    /* Why did the loop break? */
    if (tddp)
    {
	/* delete this node */
	struct target_disk_data *td;


	/* is it the first in the list ? */
	if (tddp == BosData.targets)
	{
	    BosData.targets = BosData.targets->next;

	    /* is there only one in the list? */
	    if (tddp == BosData.last)
		BosData.last = NULL;
	}
	else
	{
	    /* it's in the middle */
	    for (td=BosData.targets; td ;td=td->next)
	    {
		if (td->next == tddp)
		{
		    td->next = tddp->next;
		    free(tddp);
		    break;
		}
	    }

	    /* check the last */
	    if (tddp == BosData.last)
		BosData.last = td;
	}
    }
    return 1;
}

/*
 * NAME: add_bidata
 *
 * FUNCTION: add bosinst.data target disk data stanza
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *	1 - always
 */

int add_bidisk(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    
    struct target_disk_data *tddp;		/* target disk data ptr */

    /* set response code OK */
    rsp->rc = 2;

    /* Add a new node to the end of the list */

    tddp = (struct target_disk_data *) malloc(sizeof(struct target_disk_data));
    if (!tddp)
	exit(2);

    /* clear the stanza */
    bzero(tddp, sizeof(struct target_disk_data));

    if (BosData.last)
	BosData.last->next = tddp;
    else
	BosData.targets = tddp;

    BosData.last = tddp;


    if (cmd->value[0])
	strcpy(tddp->LOCATION, cmd->value);
    if (cmd->field[0])
	strcpy(tddp->HDISKNAME, cmd->field);
    if (cmd->auxfield[0])
	strcpy(tddp->SIZE_MB, cmd->auxfield);
}

/*
 * NAME: verify_imagedata
 *
 * FUNCTION: validate image data
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *	This function performs these checks:
 *	1.  verify lv_data is unique with regard to LOGICAL_VOLUME
 *	2.  sort fs_data by FS_NAME (this isn't an error, but this
 *	    a good place to do it)
 *	3.  verify fs_data is unique with regard to FS_NAME
 *	4.  verify fs_data is unique with regard to FS_LV
 *	5.  remove null source disk data stanzas (not an error)
 *	6.  remove null target_disk_data stanzas (not an error)
 *	7.  verify target_disk_data stanzas are unique with regard to 
 *		name and location
 *
 *	Loops in this function are fairly quick, since the length of
 *	the list is expected to be pretty short (< 15) most of the time.
 *
 * RETURNS:
 *	0 - everything is OK 
 *	-1 - lv_data is not unique
 *	-2 - fs_data is not unique
 */
int verify_imagedata()
{
    int swap;					/* swap entries sort flag */
    int rc;					/* strcmp retrun code	  */
    struct lv_data *lvdp, *ulvdp, *tlv;		/* lv_data pointers       */
    struct fs_data *fsdp, *tfs, *last, *tfsd; /* ds_data pointers       */
    struct src_disk_data *sddp;			/* source disk data ptr   */
    struct target_disk_data *tddp;		/* target disk data ptr   */

    /* search for unique lvs */
    for (ulvdp=ImageData.lv_data; ulvdp; ulvdp=ulvdp->next)
    {
	for (lvdp=ImageData.lv_data; lvdp; lvdp=lvdp->next)
	{

	    if (ulvdp == lvdp)
		continue;		/* we just saw ourself */
	    
	    if (!strcmp(ulvdp->LOGICAL_VOLUME, lvdp->LOGICAL_VOLUME))
	    {
		/* This is a duplicate. 
	 	 * Find the previous node,  Delete this lvdp 
		 */
		for (tlv=ImageData.lv_data;tlv; tlv=tlv->next)
		{
		    if (tlv->next == lvdp)
			break;
		}

		/* this node is guatanteed to exist, since we had to
		 * pass it to get to lvdp.
		 */
		tlv->next = lvdp->next;
		if (lvdp == ImageData.last_lvdp)
		    ImageData.last_lvdp = tlv;
		free(lvdp);
		lvdp = tlv;	/* set up for next iteration */

		datacheck = LV_DUP;

	    }
	}
    }

    /* Sort/unique-ify fs_data.  This is done by a bubble sort.
     */
    last =0;

    /* loop until no swaps have occured */
    while (1)
    {
	swap=0;
	/* Scan the new list to see if it's already there, insert it
	 * at the correct place
	 */
	for (fsdp=ImageData.fs_data; fsdp && fsdp->next; fsdp=fsdp->next)
	{
	    rc = strcmp(fsdp->FS_NAME, fsdp->next->FS_NAME);
	    if (rc == 0)
	    {
		/* they are the same; delete the 'next' one */
		tfsd=fsdp->next;
		fsdp->next = fsdp->next->next;
		free(tfsd);
		
		/* remember that we deleted an entry */
		datacheck |= FS_DUP;
	    }
	    if (rc > 0)
	    {
		/* Is this the top of the list? */
		if (fsdp == ImageData.fs_data)
		{
		    ImageData.fs_data = fsdp->next;
		}
		else
		{
		    /* find the previous item in the list */
		    for(last=ImageData.fs_data; last->next!=fsdp; 
			last=last->next)
		    {
			;
		    }
		    last->next = fsdp->next;
		}
		/* need to swap positions */
		tfsd = fsdp->next->next;
		fsdp->next->next = fsdp;
		fsdp->next = tfsd;

		swap=1;
	    }
        }

	/* reset the last pointer */
	ImageData.last_fsdp = last;

	if (swap == 0)
	    break;
    }

    /* look for any duplicate FS_LV fields */
    for (fsdp=ImageData.fs_data; fsdp; last=fsdp, fsdp=fsdp->next)
    {
	for (tfsd=ImageData.fs_data; tfsd; tfsd=tfsd->next)
	{
	    if ((fsdp != tfsd) && !strcmp(fsdp->FS_LV, tfsd->FS_LV))
	    {
		/* its a dup, delete the second one */
		for (tfs=ImageData.fs_data;tfs; tfs=tfs->next)
		{
		    if (tfs->next == tfsd)
			break;
		}

		/* this node is guatanteed to exist, since we had to
		 * pass it to get to lvdp.
		 */
		tfs->next = tfsd->next;
		if (tfsd == ImageData.last_fsdp)
		    ImageData.last_fsdp = tfs;
		free(tfsd);
		tfsd = tfs;	/* set up for next iteration */

		datacheck = FS_DEV;
	    }
	}
    }
    for (sddp=ImageData.src_disk_data; sddp; )
    {
	struct src_disk_data *tsdp;

	/* check LOCATION, SIZE (_MB) and HDISKNAME */
	if (!(sddp->LOCATION[0] || sddp->SIZE[0] || sddp->HDISKNAME[0]))
	{

	    /* they're all blank, so delete it */
	    if (sddp == ImageData.src_disk_data)
	    {
		/* it's the first in the list */
		ImageData.src_disk_data = sddp->next;

		/* is it also the last in the list? */
		if (sddp == ImageData.last_sddp)
		    ImageData.last_sddp = NULL;

		free(sddp);
		sddp = ImageData.src_disk_data;
	    }
	    else
	    {
		/* find the previous one in the list */
		for (tsdp=ImageData.src_disk_data; tsdp; tsdp->next)
		{
		    if (tsdp->next == sddp)
		    {
			/* delete this one from the chain */
			tsdp->next = sddp->next;

			if (sddp == ImageData.last_sddp)
			    ImageData.last_sddp = tsdp;
			free(sddp);
			sddp = tsdp;
			break;
		    }
		}

	    }
	}
	else
	    sddp=sddp->next;
    }

    /* remove extra target disk data */
    for (tddp=BosData.targets; tddp; )
    {
	struct target_disk_data *tsdp, *dup;

	/* Scan the list for any duplicates of location or name.
	 * if the loop breaks with dup non-zero, there is a duplicate.
	 * Aribtrarily delete the first one.
	 */
	for (dup=BosData.targets; dup; dup = dup->next)
	{
	    if (dup == tddp)
		continue;
	    if ((dup->LOCATION[0] && !strcmp(dup->LOCATION, tddp->LOCATION)) ||
		(dup->HDISKNAME[0] && !strcmp(dup->HDISKNAME, tddp->HDISKNAME)))
	    {
		datacheck = TDD_DUP;
		break;
	    }
	}

	/* check LOCATION, SIZE (_MB) and HDISKNAME */
	if (!(tddp->LOCATION[0] || tddp->SIZE_MB[0] || tddp->HDISKNAME[0]) || dup)
	{

	    /* they're all blank, so delete it */
	    if (tddp == BosData.targets)
	    {
		/* it's the first in the list */
		BosData.targets = tddp->next;

		/* is it also the last in the list? */
		if (tddp == BosData.last)
		    BosData.last= NULL;

		free(tddp);

		/* next ... */
		tddp = BosData.targets;
	    }
	    else
	    {
		/* find the previous one in the list */
		for (tsdp=BosData.targets; tsdp; tsdp->next)
		{
		    if (tsdp->next == tddp)
		    {
			/* delete this one from the chain */
			tsdp->next = tddp->next;

			if (tddp == BosData.last)
			    BosData.last= tsdp;
			free(tddp);

			/* next ... */
			tddp = tsdp;
			break;
		    }
		}

	    }
	}
	else
	    /* next... */
	    tddp=tddp->next;
    }
}

/*
 * NAME: get_all
 *
 * FUNCTION: retieve all target disks in a colon separated format
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *	1 - always
 */

int get_all(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    
    struct target_disk_data *tddp;		/* target disk data ptr */
    char *ptr;					/* a char pointer       */

    /* set response code OK */
    if (BosData.targets)
	rsp->rc = 0;
    else 
	rsp->rc = 2;

    for (tddp=BosData.targets; tddp; tddp=tddp->next)
    {
	char buf[80];
	if (tddp->next)
	    sprintf(buf, "%s:%s:%s\n", tddp->LOCATION, tddp->SIZE_MB, tddp->HDISKNAME);
	else
	    sprintf(buf, "%s:%s:%s", tddp->LOCATION, tddp->SIZE_MB, tddp->HDISKNAME);

	strcat(rsp->data, buf);
    }

}


/*
 * NAME: del_all
 *
 * FUNCTION: retieve all target disks in a colon separated format
 *
 * ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *	1 - always
 */

int del_all(
struct bidata *cmd,		/* cmd request			*/
struct ddresponse *rsp)		/* replay			*/
{
    
    struct target_disk_data *tddp;		/* target disk data ptr */
    struct target_disk_data *ntddp;		/* target disk data ptr */

    /* set response code OK */
    rsp->rc = 2;

    for (tddp=BosData.targets; tddp; )
    {
	ntddp = tddp->next;
	free(tddp);
	tddp=ntddp;
    }
    BosData.targets = BosData.last = NULL;
}
