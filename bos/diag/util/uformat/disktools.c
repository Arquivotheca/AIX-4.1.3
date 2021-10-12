static char sccsid[] = "@(#)63  1.8  src/bos/diag/util/uformat/disktools.c, dsauformat, bos41B, 9505A 1/6/95 14:41:55";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: format_sense_data
 *		get_odm_info
 *		process_sense_data
 *		set_default_sense
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include	<stdio.h>
#include	<sys/cfgodm.h>
#include	<sys/types.h>
#include	<diag/dascsi.h>
#include 	<sys/scdisk.h>
#include 	<sys/scsi.h>
#include        <diag/scsi_atu.h>

ODM_INFO static_odm_info;

typedef struct  {
	char *attribute;
	char conv;
	void *value;
	} Param;

Param params[]={
	{"de_card_fru",		'h', &(static_odm_info.de_card_fru)},
	{"sde_threshold",	'f', &(static_odm_info.soft_data_error_ratio)},
	{"see_threshold",	'f', &(static_odm_info.soft_equip_error_ratio)},
	{"hard_data_errs",	'i', &(static_odm_info.max_hard_data_error)},
	{"hard_eqp_errs",	'i', &(static_odm_info.max_hard_equip_error)},
	{"soft_data_errs",	'i', &(static_odm_info.max_soft_data_error)},
	{"soft_eqp_errs",	'i', &(static_odm_info.max_soft_equip_error)},
	{"certify_time",	'i', &(static_odm_info.certify_time)},
	{"send_diag",		'b', &(static_odm_info.send_diag)},
	{"mode_data",		'b', &(static_odm_info.mode_data)},
	{"use_subsys_diag",	'h', &(static_odm_info.use_subsystem_diag)},
};
#define MAX_PARAMS	11	/* this value should be modified whenever
				   records are added/deleted from params[] */

extern	get_diag_att(char *, char *, char, int *, void *);
extern int  diag_get_scsd_cuat(struct CuDv,ODM_INFO *);

/*
 * NAME: get_odm_info
 *
 * FUNCTION: Given the logical device name, return information from
 *		the ODM data base.
 * DATA STRUCTURE: static_odm_info is used as a static temporary
 *		structure which is then copied to odm_info argument
 *		before returns
 * RETURN: 0 if everything is OK
 *	  -1 if get_CuDv_list fails
 *	  -2 if attributes are missing
 */

int
get_odm_info(devicename, failing_function_code, osdisk, odm_info)
char	*devicename;
int	*failing_function_code;
short	*osdisk;
ODM_INFO *odm_info;
{
        char	odm_search_crit[80];	/* odm search criteria */
        struct	CuDv	*cudv;		/* ODM Customized device struct */
	struct  CuAt    *cuat;
        struct	listinfo	obj_info;
	int	rc;
	int	byte_count;
	int	i, bc;
	
	/* Failing function code will be used to generate menu number
   	 * as well as SRN.
	 */
        sprintf( odm_search_crit, "name = %s", devicename);
        cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit, &obj_info, 1, 2 );

        if ( ( cudv == ( struct CuDv * ) -1 ) ||
             ( cudv == ( struct CuDv * ) NULL ) )
                return(-1);

        *failing_function_code = cudv->PdDvLn->led;
	if(*failing_function_code == 0) {
		/* No LED number in PdDv so check for type Z */
		/* attribute                                 */
		
		
		if ((cuat = (struct CuAt *)getattr(cudv->name,"led",0,&bc)) 
		    == (struct CuAt *)NULL) {
			
			/* Error from CuAt */
			return(-1);
		}	
		*failing_function_code = (int)strtoul(cuat->value,NULL,0);					
	} 
	*osdisk = (*failing_function_code == 0x721);

	/*
	 * Obtain the information from the ODM data base.
	 * Certain attributes, if not found will result in a -1 to be returned
	 * to the caller. This will allow the DA to generate a menu goal to warn
	 * the user of missing attributes.
	 */
	if (strcmp(cudv->PdDvLn->type,"scsd")) {
		
		/*
		 * This is not an SCSD device
		 */
		for (i=0; i<MAX_PARAMS; i++){
			if (*osdisk && (strcmp(params[i].attribute,"mode_data")==0))
				/* the mode_data attribute for osdisk is taken care of
				   by the SA/DA, it is ignored here */
				continue;
			
			rc=get_diag_att(cudv->PdDvLn->uniquetype, params[i].attribute,
					params[i].conv, &byte_count, params[i].value);
			
			if(rc == -1) {
				rc=get_diag_att("deflt_diag_att", params[i].attribute,
						params[i].conv, &byte_count, 
						params[i].value);
				if(rc == -1){
					odm_free_list(cudv, &obj_info);
					return(-2);
				}
			}
			/* special case for mode_data to get mode_data_length */
			if (strcmp(params[i].attribute, "mode_data") == 0)
				static_odm_info.mode_data_length=byte_count;
		}
	}
	else {
		/*
		 * This is an SCSD device
		 */
		rc = diag_get_scsd_cuat(*cudv,&static_odm_info);
		if (rc == -1) {
			return (-1);
		}

	}

	odm_free_list(cudv, &obj_info);
	memcpy(odm_info, &static_odm_info, sizeof(static_odm_info));
	return(0);
}

/*  */
/*
 * NAME: process_sense_data
 *
 * FUNCTION: Determine the error type base on sense key and sense code.
 *
 * NOTE: recovery_count is used to determined whether or not a reassign
 *	 command should be issue for the block that has the error.
 *
 * RETURNS: Next step to execute.
 */

int
process_sense_data(sense_key, sense_code, recovery_count,
	reassign_block, error_type)
int	sense_key;
int	sense_code;
short	recovery_count;
short	*reassign_block;
short	*error_type;
{
	int	return_code;


	switch(sense_key){
	case 1: /* Recovered Errors */
		switch(sense_code){
		case 0x0100:
		case 0x0200:
		case 0x0403:
		case 0x0800:
		case 0x0900:
		case 0x1f00:
		case 0x3e00:
		case 0x4400:
		case 0x5c02:
			*error_type=RECOVERED_EQUIP_ERROR;
			break;
		case 0x0c01:
		case 0x1406:
		case 0x1603:
		case 0x1706:
		case 0x1802:
		case 0x1807:
		case 0x1900:
		case 0x1c00:
		case 0x1c01:
		case 0x1c02:
			*error_type=RECOVERED_DATA_ERROR;
			break;
		case 0x1000:
		case 0x1103:
		case 0x1200:
		case 0x1401:
		case 0x1402:
		case 0x1403:
			if(recovery_count > 4){
				*error_type=RECOVERED_DATA_ERROR;
				*reassign_block=1;
			}
			break;
		case 0x0c03:
		case 0x1300:
		case 0x1600:
		case 0x1601:
		case 0x1602:
		case 0x1604:
		case 0x1700:
		case 0x1701:
		case 0x1705:
		case 0x1707:
		case 0x1708:
		case 0x1709:
		case 0x1800:
		case 0x1801:
		case 0x1805:
		case 0x1806:
		case 0x1880:
		case 0x1881:
		case 0x1882:
			*error_type=RECOVERED_DATA_ERROR;
			*reassign_block=1;
			break;
		case 0x5d00:
			*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		default:
			if( ((sense_code >= 0x0300) && (sense_code < 0x0403)) ||
			    ((sense_code >= 0x1500) && (sense_code < 0x1600)) )	
				*error_type=RECOVERED_EQUIP_ERROR;
			if ((sense_code >= 0x1400) && (sense_code < 0x1500))
				if(recovery_count > 4){
					*error_type=RECOVERED_DATA_ERROR;
					*reassign_block=1;
				}
			if ((sense_code >= 0x1600) && (sense_code < 0x1900))
				*error_type=RECOVERED_DATA_ERROR;
			break;
		} /* Switch of sense code for sense key of 1 */
		break;
	case 2: /* Not ready */
		switch(sense_code){
		case 0x0300:
		case 0x0403:
		case 0x0485:
		case 0x0500:
		case 0x0800:
		case 0x3101:
		case 0x3e00:
		case 0x4080:
		case 0x40b0:
		case 0x40c0:
		case 0x4400:
		case 0x4c00:
		case 0x5d00:
			*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		case 0x0400:
		case 0x0401:
		case 0x0402:
		case 0x2200:
		case 0x3100:
			*error_type=RECOVERED_EQUIP_ERROR;
			break;
		default:
			if ((sense_code >= 0x3100) && (sense_code < 0x3200))
				*error_type=RECOVERED_EQUIP_ERROR;
			 /* Ignore any other unknown sense data */
		} /* Switch of sense code for sense key of 2 */
		break;

	case 3: /* Medium error */
		switch(sense_code){
		case 0x0200:
		case 0x0300:
		case 0x1500:
		case 0x3000:
		case 0x3100:
		case 0x4400:
		case 0x5d00:
			*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		case 0x0c02:
		case 0x1104:
		case 0x1400:
		case 0x1c00:
			*error_type=UNRECOVERED_DATA_ERROR;
			break;
		case 0x0c03:
			*error_type=RECOVERED_EQUIP_ERROR;
			*reassign_block=1;
			break;
		case 0x1000:
		case 0x1100:
		case 0x110b:
		case 0x1200:
		case 0x1300:
		case 0x1401:
		case 0x1405:
		case 0x1600:
		case 0x1604:
		case 0x1d00:
			*error_type=UNRECOVERED_DATA_ERROR;
			*reassign_block=1;
			break;
		default:
			if((sense_code >= 0x1900) && (sense_code < 0x1a00)) 
				*error_type=UNRECOVERED_DATA_ERROR;
			if( (sense_code >= 0x3200) && (sense_code < 0x3300) )
				*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		}
		break;
  
	case 4: /* Hardware error */
		switch(sense_code){
		case 0x1000:
		case 0x1200:
		case 0x1400:
		case 0x1401:
		case 0x1600:
			*error_type=UNRECOVERED_DATA_ERROR;
			*reassign_block=1;
			break;
		case 0x0900:
		case 0x1b00:
		case 0x2200:
		case 0x2100:
		case 0x3100:
		case 0x4300:
		case 0x4400:
		case 0x4500:
		case 0x4700:
		case 0x5d00:
		case 0xb900:
			*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		default:
			if((sense_code <= 0x0800) ||
			   ((sense_code >= 0x3200) && (sense_code <= 0x4100)) 
		           || ((sense_code >= 0x1500) && (sense_code < 0x1600)))
				*error_type=UNRECOVERED_EQUIP_ERROR;
			if((sense_code >= 0x1900) && (sense_code < 0x1a00)) 
				*error_type=UNRECOVERED_DATA_ERROR;
			break;
		}
		break;

	case 5: /* Illegal Request */
		switch(sense_code){
		case 0x1900:
			*error_type=UNRECOVERED_DATA_ERROR;
			break;
		case 0x3900:
			*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		case 0x1a00:
		case 0x2000:
		case 0x2100:
		case 0x2200:
		case 0x2400:
		case 0x2500:
		case 0x2600:
		case 0x2601:
		case 0x49001:
			/* Device attachement errors, could be drive */
			/*   or cable. Ignore these. */
			break;
		default:
			if( (sense_code >= 0x3200) && (sense_code < 0x3300) )
				*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		}
		break;

	case 6: /* Unit Attention */
		switch(sense_code){
		case 0x4200:
			*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		default:
			break;
		}
		break;
			
	case 7: /* Data Protect */
		switch(sense_code){
		case 0x2700:
			*error_type=UNRECOVERED_EQUIP_ERROR;
			break;
		default:
			break;
		}
	case 0x0b: /* Aborted command */
		switch(sense_code){
		case 0x0000:
		case 0x1b00:
		case 0x2500:
		case 0x4300:
		case 0x4400:
		case 0x4500:
		case 0x4600:
		case 0x4700:
		case 0x4800:
		case 0x4900:
		case 0x4e00:
			/* Device attachement errors, could be drive */
			/*   or cable. Ignore these. */
			break;
		default:
			break;
		}
	case 0x0e:
		switch(sense_code){
		case 0x1d00:
			*error_type=UNRECOVERED_DATA_ERROR;
			break;
		default:
			break;
		}
		break;
	default:
		;
	}
}
/*  */
/*
 * NAME: format_sense_data
 *
 * FUNCTION: Given the mode data from the MODE SENSE command or from
 *           ODM, format the DISK_MODE_FORMAT struct to contain the
 *           mode data offset of each page.
 *
 * RETURNS: N/A (void)
 *
 * NOTE: mf->page_index[0] will contain the offset into the mode data for
 *       page zero and mf->page_index[1] will contain the offset for page
 *       one and so on.
 *
 */

void format_sense_data(
uchar *mode_data,
int  sense_length,
DISK_MODE_FORMAT *mf)
{
        short   page=0;
        int     i;
        short   bd_length,offset,p_length;

        for (i=0;i<MAX_MODE_PAGES;i++)
                /*
                 * initialize all page indices to -1
                 */
                mf->page_index[i] = -1;

        mf->sense_length = sense_length;
        /*
         * get length of block descriptor
         */
        bd_length = mode_data[BLK_DESC_LN_INDEX];
        if (bd_length == 8) {
                mf->block_length = ((mode_data[9] << 16) | (mode_data[10]<< 8) |
                                  mode_data[11]) & 0x00FFFFFF;
        } else {
                mf->block_length = 0;
        }
        /*
         * compute offset to first page (i.e. offset to block descriptor +
         * length of block descriptor area )
         */
        offset = BLK_DESC_LN_INDEX+bd_length+1;
        while (offset < mf->sense_length) {
                /*
                 * For the remainder of the sense data...
                 * Store the index into the data buffer for each page
                 */
                page = (mode_data[offset] & MAX_MODE_PAGES);
                mf->page_index[page] = offset;
                p_length = mode_data[++offset];
                for (i=0;i<=p_length;i++)
                        /*
                         * step over the data bytes for this page
                         */
                        ++offset;
        }
        return;
}
/*  */
/*
 * NAME: set_default_sense
 *
 * FUNCTION: Given the sense data from the MODE SENSE command, setup all
 *              the pages that can be set.
 *
 * RETURNS: changeable mode data to be used with the Mode Select command.
 *
 * NOTE: This routine could be move to the shared library libdiag.a if
 *       other SCSI devices DA need to use it.
 */

int
set_default_sense(
uchar   *desired_mode_data,
int     mode_data_length,
uchar   *current_mode_data,
uchar   *changeable_mode_data,
uchar   *buffer,
int     *sense_length
)
{
        DISK_MODE_FORMAT *dmf, *chmf;
        int     more_pages,resulting_length;
        int     i,ch_index,dd_index;
        short   ch_length,dd_length, page;

        /*
         * dmf is the desired formatted mode data from the ODM data base
         * and chmf is a bit mask of all changeable bits that can be set.
         */

        dmf=(DISK_MODE_FORMAT *)malloc(sizeof(DISK_MODE_FORMAT));
        chmf=(DISK_MODE_FORMAT *)malloc(sizeof(DISK_MODE_FORMAT));
        memset(dmf, 0x00, sizeof(DISK_MODE_FORMAT));
        memset(chmf, 0x00, sizeof(DISK_MODE_FORMAT));

        format_sense_data(desired_mode_data,mode_data_length,dmf);
        format_sense_data(changeable_mode_data,changeable_mode_data[0],chmf);

        /*
         * Copy the header and the block descriptor.
         */
        bcopy(desired_mode_data, buffer,
              HEADER_LENGTH + buffer[BLK_DESC_LN_INDEX]);

        resulting_length = (HEADER_LENGTH + buffer[BLK_DESC_LN_INDEX]);

        /*
         * Set page to 1 to begin with. When MAX_MODE_PAGES is reached,
         * set the page to 0 so page zero will be the last page.
         */
        more_pages = page = 1;
        while (more_pages) {
                if (page == MAX_MODE_PAGES) {
                        more_pages = page = 0; /* Last page. */
                }
                if (chmf->page_index[page] != (signed char) -1) {
                        /*
                         * if this page is supported
                         */
                        ch_index = (uchar)chmf->page_index[page];
                        ch_length = changeable_mode_data[ch_index+1];

                        buffer[resulting_length++] = page;
                        buffer[resulting_length++] = ch_length;

                        dd_index = dmf->page_index[page];
                        dd_length = desired_mode_data[dd_index+1];

                        for (i=2;i < ch_length+2; i++, dd_length--) {
                                if ((dd_length > 0) &&
                                    (desired_mode_data[dd_index+i] &
                                     changeable_mode_data[ch_index+i])) {
                                        /*
                                         * Use desired ODM mode data.
                                         * Use changeable data as a mask to
                                         * only change the changeable bits.
                                         */
                                        buffer[resulting_length++] =
                                             desired_mode_data[dd_index+i] &
                                             changeable_mode_data[ch_index+i];
                                } else {
                                        /*
                                         * No more desired data or no bits
                                         * that are changeable. Use current
                                         * mode data from the device.
                                         */
                                        buffer[resulting_length++] =
                                            current_mode_data[ch_index+i];
                                }
                        }
                } /* if this page supported */
                ++page;
        } /* while more pages */

        *sense_length = resulting_length;

        return(0);
}

