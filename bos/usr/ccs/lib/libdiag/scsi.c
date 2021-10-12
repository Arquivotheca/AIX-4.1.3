static char sccsid[] = "@(#)87  1.2  src/bos/usr/ccs/lib/libdiag/scsi.c, dacdrom, bos41B, 9505A 1/9/95 15:32:44";
/*
 * COMPONENT_NAME: (LIBDIAG)  DIAGNOSTICS LIBRARY
 *
 * FUNCTIONS: diag_get_sid_lun
 *            diag_get_scsd_cuat
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <strings.h>
#include <limits.h>
#include <math.h>
#include <sys/cfgodm.h>
#include <diag/scsd.h>
#include <diag/dascsi.h>

/*
 * NAME: diag_get_sid_lun
 *
 * FUNCTION: Extracts the sid, and lun from a SCSI address (connwhere format)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *	 1. This code is designed to be loadable, therefore no global variables 
 * 	    are assumed.
 *	 2. The basic function is to convert the input string
 *	    from the form 'sss,lll' to uchar values (i.e. the sid, and the lun )
 *	    sss  = decimal characters for the sid
 *	    lll  = decimal characters for the lun
 *
 * RETURNS:
 *	 0 for success, -1 for failure.
 *
 */
int
diag_get_sid_lun( scsiaddr, sid_addr, lun_addr )
char    *scsiaddr;
uchar   *sid_addr;
uchar   *lun_addr;
{

        if (*scsiaddr == '\0') return -1;
        if ( strchr(scsiaddr,',') == NULL) return -1;

/* We utilize the behavior of strtoul which stops converting characters at
   the first non-base character Thus after the start position is set, the
   conversion stops either at the ',' or at the NULL end-of-string */

        *sid_addr = (uchar)strtoul(scsiaddr,NULL,10);
        *lun_addr = (uchar)strtoul(strchr(scsiaddr,',')+1,NULL,10);

        return 0;
}

/*
 * NAME: diag_get_scsd_cuat
 *
 * FUNCTION: Extracts SCSD information from the CuAt
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *       1. This code is designed to be loadable, therefore no global variables 
 *          are assumed.
 *       2. The basic function is to extract the diag_scsd attribute
 *	    from the CuAt and fill in the odm_info structure.
 *
 * RETURNS:
 *       0 for success, -1 for failure.
 *
 */
int
diag_get_scsd_cuat(cusobj, odm)
struct CuDv cusobj;
ODM_INFO *odm;
{
	int rc;				/* return code 		*/
	struct CuAt cuat, *cuat_ptr;
	char criteria[64];
	char scsd_vpd_buffer[sizeof(struct disk_diag_scsd_cuat)];
	char tmpstr[3];
	struct  disk_diag_scsd_cuat *disk_scsd_ptr;	
	double	multiplier, exponent, denominator, one = 1.0,ten = 10.0;
	int bc;
	int value_length, upper_limit,value_index,i; 
	int capacity;

	/* Extract the diag_scsd string from ODM		*/
	 
	sprintf(criteria,"name = %s and attribute = diag_scsd",cusobj.name);
	rc = (int) odm_get_first(CuAt_CLASS,criteria,&cuat);
	if (rc < 1)  {
		return (-1);
	}
        if ((!strcmp(cusobj.PdDvLn_Lvalue,"cdrom/scsi/scsd")) ||
            (!strcmp(cusobj.PdDvLn_Lvalue,"rwoptical/scsi/scsd")) ||
	    (!strcmp(cusobj.PdDvLn_Lvalue,"disk/scsi/scsd"))) {

		/*
		  SCSD SCSI CD-ROMs, Disks and R/W optical drives
		  use the same diag_scsd attribute
		*/

		/*
		  Determine the length of the cuat.value string
		*/

		value_length = strlen(cuat.value);

		/*
		  Determine the length of our structure for storing
		  the cuat SCSD information
		*/

		upper_limit = sizeof(struct disk_diag_scsd_cuat);

		bzero(scsd_vpd_buffer,upper_limit);

	        if (value_length % 2) {
			/*
			  Since we will be converting two ASCII characters
			  at at time to a single binary value, we must
			  ensure that the length of our string is divisible 
			  by two.
			*/
			value_length--;
		}

		if ((2 * upper_limit) > value_length) {
			/*
			  If the cuat.value string is
			  smaller then our formatted 
			  structure, then we only want
			  copy the relevant bytes.
			*/
			upper_limit = value_length/2;
		}

		value_index = 0;
		
		/*
		  Loop through the cuat.value string converting
		  each two characters into a binary value at a time.
		*/
		for (i = 0;i < upper_limit;i++) {
			strncpy(tmpstr,&(cuat.value[value_index]),2);
			tmpstr[2]='\0';
			scsd_vpd_buffer[i] = strtoul(tmpstr,NULL,16);
			value_index += 2;

		}

		disk_scsd_ptr = (struct disk_diag_scsd_cuat *)scsd_vpd_buffer;

		if (disk_scsd_ptr->operation_flags & SCSD_FRU_FLG) {

			odm->de_card_fru = 1;
		}else
			odm->de_card_fru = 0;

		denominator = disk_scsd_ptr->data_err_mult * pow(ten,
					     disk_scsd_ptr->data_err_base_ten);
		if (fabs(denominator - 0.0) > FLT_MIN) {
			
			odm->soft_data_error_ratio = one / denominator;
		}
		else {
			/* 
			  This denominator is so close to zero
			  that with our level of precision this    
			  this would be equivalent to division
			  by zero.  Since this in not valid,
			  lets just set this field to default.
			*/
			odm->soft_data_error_ratio = 0.0008;
		}

		denominator = disk_scsd_ptr->equip_chk_mult * pow(ten,
					 disk_scsd_ptr->equip_chk_base_ten);
		if (fabs(denominator - 0.0) > FLT_MIN) {
			
			odm->soft_equip_error_ratio = one / denominator;
		}
		else {
			/* 
			  This denominator is so close to zero
			  that with our level of precision this    
			  this would be equivalent to division
			  by zero.  Since this in not valid,
			  lets just set this field to default.
			*/
			odm->soft_equip_error_ratio = 0.00125;
		}


		odm->max_hard_data_error = 3;
		odm->max_hard_equip_error = 0;
		odm->max_soft_data_error = 10;
		odm->max_soft_equip_error = 125;

		/*
		  Extract capacity of device from ODM
		*/
                if ((cuat_ptr = (struct CuAt *)getattr(cusobj.name,"size_in_mb",0,&bc))
                    == (struct CuAt *)NULL ) {
                        return (-1);
                }

		capacity = strtoul(cuat_ptr->value,NULL,10);
		if (capacity > 0) {
			odm->certify_time = DIAG_SCSD_CERT_TIME * capacity + 1;
		}
		else {
			/*
			  If capacity is not greater then
			  zero, then default.
			*/
			odm->certify_time = 15;
		}
		odm->send_diag[0] = '\0';

		/*
		  The following flag is not
		  used in diagnostic
		*/

		odm->use_subsystem_diag = 0;
		return (0);
	}
	else {
		/*
		  Invalid dev_type
		*/
		return (-1);
	}
}


/*
 * NAME: diag_get_scsd_opflag
 *
 * FUNCTION: Extracts the operation flags from the CuAt
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *       1. This code is designed to be loadable, therefore no global variables 
 *          are assumed.
 *       2. The basic function is to extract the diag_scsd attribute
 *	    from the CuAt to fill in the operation flags.
 *
 * RETURNS:
 *       0 for success, -1 for failure.
 *
 */
int
diag_get_scsd_opflag(name, operation_flags)
char *name;
uchar *operation_flags;
{
        int rc;
	struct CuAt cuat;
	char criteria[64];
	char	tmpstr[3];

	/*
	  Get information from the SCSD diagnostic
	  CuAt attribute.
	 */
	 
	sprintf(criteria,"name = %s and attribute = diag_scsd",
		name);
	rc = (int) odm_get_first(CuAt_CLASS,criteria,&cuat);
	if (rc < 1)  {
		return (-1);
	}
	strncpy(tmpstr,cuat.value,2);
	tmpstr[2] ='\0';
	*operation_flags = strtoul(tmpstr,NULL,16);
	return (0);
}
