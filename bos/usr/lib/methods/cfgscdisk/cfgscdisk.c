static char sccsid[] = "@(#)39  1.24.1.22  src/bos/usr/lib/methods/cfgscdisk/cfgscdisk.c, cfgmethods, bos41J, 9521A_all 5/23/95 17:47:17";
/*
 * COMPONENT_NAME: (CFGMETHODS) SCSI Disk config method
 *
 * FUNCTIONS: generate_minor, make_special_files,
 * FUNCTIONS: query_vpd, scan_vpd, build_dds
 * FUNCTIONS: spin_up_disk
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */
#include	<stdio.h>
#include        <fcntl.h>
#include        <malloc.h>
#include        <sys/errno.h>
#include        <sys/types.h>
#include        <sys/device.h>
#include        <sys/sysmacros.h>
#include        <sys/cfgdb.h>
#include        <sys/stat.h>
#include        <sys/scsi.h>
#include	<sys/scdiskdd.h>
#include        <sys/cfgodm.h>
#include        <sys/pm.h>
#include        <cf.h>
#include        "cfgdebug.h"
#include        "cfghscsi.h"
#include        "cfgtoolsx.h"

#include        <sys/bootrecord.h>
#include        <sys/utsname.h>
#include        <sys/mdio.h>
#include		<string.h>

#define NULLPVID "00000000000000000000000000000000"
#define CALL_FINDDISK   -1

char *pvidtoa();
void get_pvid();

#define INQSIZE 255
static  uchar   inq_data[INQSIZE];      /* Storage area for inquery data */
static  uchar   pvid_str[33];

/*
 * NAME: generate_minor
 *
 * FUNCTION: Generates a minor number for the scsi disk being configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 *      0 on success ( with minor number stored via the last parameter )
 *      errno on error
 */

long generate_minor( logical_name, major_no, minor_dest )
char *logical_name;     /* Name of device                       */
long major_no;          /* Major no. already generated          */
long *minor_dest;       /* Destination for minor number         */
{
        long    *genret;
        long *genminor();

        /*
         * Request a minor number. Only one number is required, and there is
         * no preferred number, nor is there a boundary
         */
        DEBUG_0( "generate_minor()\n" )

        if ((genret = genminor(logical_name, major_no,-1,1,1,1)) == NULL )
                return E_MINORNO;

        *minor_dest = *genret;

        return 0;

}

/*
 * NAME: make_special_files
 *
 * FUNCTION: Generates two special files for the scsi disk being configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *    This code is designed to be linked with cfgdisk.
 *    The files generated are /dev/<logical_name>, and /dev/r<logical_name>
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 *
 */

int make_special_files( logical_name, devno )
char    *logical_name;
dev_t   devno;
{
        char basename[NAMESIZE+1];
        int rc;

        DEBUG_0( "make_special_files()\n" )

        /* Generate block file */
        rc = mk_sp_file( devno, logical_name, (S_IRUSR | S_IWUSR | S_IFBLK));
        if (rc)
                return(rc);

        /* Create the character special file */
        sprintf( basename, "r%s", logical_name );
        rc = mk_sp_file( devno, basename, (S_IRUSR | S_IWUSR | S_IFCHR));
        return(rc);
}

/*
 * NAME: query_vpd
 *
 * FUNCTION: Obtains the VPD data, formats the data and
 *           the VPD to cfgdisk.
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 * 0. if succeeded
 * E_VPD if could not obtain VPD data
 *
 */

int
query_vpd( cusobj, kmid, devno, vpd_dest )
struct  CuDv    *cusobj;        /* Pointer to customized object rec     */
mid_t   kmid;                   /* kernel module I.D. for Dev Driver    */
dev_t   devno;                  /* Concatenated Major & Minor No.s      */
char    *vpd_dest;              /* Area to place VPD                    */
{
    char        fmt_str[] =
                "MF0808C,TM1010C,PN720cC,RL2004X,SN2408C,EC7e0aC,FN880cC,Z00008X,Z12c0cC,Z26204C,Z36605C,Z46c04C,Z57002C,Z6940aC" ;
    int         vpd_len ;
    int 	rc;

    /* BEGIN query_vpd */

    vpd_len = scvpd(cusobj, vpd_dest, fmt_str) ;
    if (vpd_len > 0)
    {
    /* vpd_len > 0 is good, got some/all VPD data */
#ifdef CFGDEBUG
        hexdump(vpd_dest, vpd_len) ;
#endif
        rc = 0 ;
    }
    else
    {
        DEBUG_0("query_vpd: No VPD received from scvpd\n") ;
        rc = E_VPD ;
    }
    return(rc);
} /* END query_vpd */

/*
 * NAME: build_dds
 *
 * FUNCTION: Device dependent dds build for SCSI-Disk
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:	0 if succeed, or errno if fail
 */

int build_dds( cusobj, parobj, dds_data_ptr, dds_len )
struct	CuDv	*cusobj;	/* Disk's CuDv object 			*/
struct	CuDv	*parobj;	/* Disk's parent's CuDv object 		*/
char	**dds_data_ptr;		/* Address to store pointer to dds	*/
int		*dds_len;	/* Address to store length of dds	*/

{
	struct	disk_ddi	*dds;	/* dds for SCSI-Disk from scdisk.h*/
	int	rc;			/* Return code			  */
	long	major_no;		/* Major number of adapter	  */
	long	minor_no;		/* Minor number of adapter	  */
	struct	attr_list	*alist;
	int	how_many;
	int	bc;
	uchar	sid,lun;		/* SCSI Id and LUN of disk        */
	int     ansi_type,cmdque_bit;
	long	*getminor();
	long	genmajor();
	int     max_dev_data_rate,dev_data_rate,adap_data_rate,data_rate;
	float   bus_rate, media_data_rate;
	char    data_value[30];
	struct   CuAt   *cuat;
	int	queue_depth;
	struct	disk_scsd_inqry_data	scsd_vpd;
	struct  disk_scsd_mode_data	scsd_mode_data, scsd_deflt_data;
	int	pg,scsd_flg=FALSE;
	char	led[6],diagstr[30],msg[3];
	char    tmpstr[20];
	int	reset_delay;
	int     size_in_mb;
	int	i;
	int     wbus16_flg=0;		/* Non-zero indicates the device is a */
					/* 16 bit device.		      */


	DEBUG_0( "build_dds()\n" )

	if ((dds = (struct disk_ddi *)malloc(sizeof(struct disk_ddi))) == NULL)
	{
		DEBUG_0("Failed to malloc space for struct disk_ddi\n")
		return(E_MALLOC);
	}

	/* Zero out dds structure */
	bzero(dds, sizeof(struct disk_ddi));

	/* put device name into DDS */
	strncpy(dds->resource_name, cusobj->name, sizeof(dds->resource_name));

	/* initialize some DDS fields */
	/* These are hard coded here rather than in the PdAt database */
	/* to save save disk space */
	dds->segment_cnt = 0;
	dds->byte_count = 0;
	dds->async_flag = 0;
	dds->segment_size = 1000000;
	dds->max_request = 0x40000;
	dds->prevent_eject = 0;
	dds->dev_type = DK_DISK;
	dds->fmt_timeout =  0;            /* not used for Disk */
	dds->pm_susp_bdr = 0;             /* Required to be 1 only for  */
					  /* one Teac CD-ROM. This is a */
					  /* for that buggy drive.	*/


	/* Set CD-ROM only attributes   */

	dds->play_audio = FALSE;
	dds->valid_cd_modes = 0;  	  /* CD-ROM data modes valid	*/
	dds->cd_mode2_form1_code = 0;
	dds->cd_mode2_form2_code = 0;
	dds->cd_da_code = 0;
	dds->multi_session = FALSE;
	dds->load_eject_alt = FALSE;	  /* Required to be TRUE only for */
					  /* Toshiba's early CDROM's	  */


	/* Read the attributes from the customized & predefined classes */
	alist = (struct	attr_list *) get_attr_list(cusobj->name,
						   cusobj->PdDvLn_Lvalue,
						   &how_many,10);
	if (alist == (struct	attr_list *) NULL) {
		if (how_many == 0)
			return(E_NOATTR);
		else
			return(how_many);
	}


	/* 
	 * Get inquiry data for the disk.  We need it here to do the
	 * following special check for 355mb and 670mb drives.  But we go
	 * ahead and get it here for any drive type, then save it in a
	 * static buffer so that the query_vpd() routine can make use of it.
	 */
	rc = get_inquiry_data(cusobj,(int)NO_PAGE,inq_data);
	if (rc == 0)
		return(E_DEVACCESS);
	
	/*
	 * Detemine if this device is a SCSI-2 16 bit device.
	 */

	wbus16_flg = inq_data[7] & SCSI_WBUS16_MASK;

        if ((rc=getatt(alist,"queue_depth",&queue_depth,'i',&bc))
            == E_NOATTR) {
                /*
                 * don't fail if this attribute is not there, just assume
                 * queue_depth = 1
                 */
                dds->queue_depth = 1;

        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }
        else {
                dds->queue_depth = queue_depth;
        }


	if (!strcmp(cusobj->PdDvLn_Lvalue, "disk/scsi/scsd")){
		pg=0xC7;
		rc = get_inquiry_data(cusobj,pg,&scsd_vpd);
		if (rc == 0)
			return(E_WRONGDEVICE);
		
		if(strncmp(scsd_vpd.scsd_id_field,"SCDD",4)==0){
			scsd_flg=TRUE;
		}
		else
			return(E_WRONGDEVICE);
	} /* end strcmp */


	if (scsd_flg){
		
		/* 
		 * If SCSD (Self Configuring SCSI Device) disk drive
		 */
		
		dds->safe_relocate=1;
		dds->extended_rw=1;
		dds->buffer_ratio = 0;
	

		sprintf(tmpstr,"%02x%02x%02x",scsd_vpd.reset_delay[0],
			scsd_vpd.reset_delay[1],scsd_vpd.reset_delay[2]);

		reset_delay = strtoul(tmpstr,NULL,16);
		if (reset_delay > SCSD_BDR_DELAY)
			dds->reset_delay=2;
                else
                        dds->reset_delay=0;
		


		/*
		 * Build up size_in_mb attribute for this device
		 */

		sprintf(tmpstr,"%02x%02x%02x%02x",scsd_vpd.storage_capacity[0],
			scsd_vpd.storage_capacity[1],
			scsd_vpd.storage_capacity[2],
			scsd_vpd.storage_capacity[3]);

		size_in_mb = strtoul(tmpstr,NULL,16);
		sprintf(tmpstr,"%d",size_in_mb);

		/*
		 * Update size_in_mb for this device.
		 */
		if ((cuat = getattr(cusobj->name,"size_in_mb",0,&bc))
		    == (struct CuAt *)NULL ) {
			DEBUG_0("ERROR: getattr() failed\n")
				return E_NOATTR;
		}
		
		/* Only rewrite if actually changed */
		if (strcmp(cuat->value,tmpstr)) {
			strcpy(cuat->value, tmpstr);
			putattr(cuat);
		}
		
		if (scsd_vpd.reassign_timeout != 0) {
			dds->reassign_timeout = scsd_vpd.reassign_timeout;
		} 
		else {
			dds->reassign_timeout=DK_REASSIGN_TIMEOUT;
		}
		if (scsd_vpd.rw_timeout != 0) {
			dds->rw_timeout=scsd_vpd.rw_timeout;
		}
		else {
			dds->rw_timeout= DK_TIMEOUT;
		}

		if (scsd_vpd.start_timeout != 0) {
			dds->start_timeout=scsd_vpd.start_timeout;
		}
		else {
			dds->start_timeout=DK_START_TIMEOUT;
		}
					


		
		if (!(dds->queue_depth < scsd_vpd.queue_depth))
			dds->queue_depth = scsd_vpd.queue_depth;
		/*
		 * NOTE: Driver will ensure that queue_depth can
		 * never be less then 1.
		 */
				

		if (dds->queue_depth > 1) {
			/*
			 * This device claims to support 
			 * Command Tag Queuing so enable
			 * all queuing functions from SCSD information
			 */

			if (scsd_vpd.q_flags & SCSD_QERR_FLG) {
				dds->q_err_value = TRUE;
			}
			else {
				dds->q_err_value = FALSE;
			}

			if (scsd_vpd.q_flags & SCSD_Q_TYPE_FLG) {
				dds->q_type = SC_ORDERED_Q;
			}
			else {
				dds->q_type = SC_SIMPLE_Q;
			}

			if (scsd_vpd.operation_flags & SCSD_FLUSH_Q_FLG ) {
				dds->clr_q_on_error = TRUE;
			}
			else {
				dds->clr_q_on_error = TRUE;
			}
		}
		else {
			/*
			 * This device does not support Command Tag
			 * Queuing so default all queuing settings
			 */

			dds->q_err_value = TRUE;
			dds->clr_q_on_error = TRUE;
			dds->q_type = SC_NO_Q;

		}
		
		if ((cuat = getattr(cusobj->name,"led",0,&bc)) 
		    == (struct CuAt *)NULL) {
			DEBUG_0("ERROR: getattr() failed\n")
				return E_NOATTR;
		}

		sprintf(tmpstr,"0x%02x%02x%02x",scsd_vpd.led_no[0],
			scsd_vpd.led_no[1],scsd_vpd.led_no[2]);
		

		if (!strcmp(tmpstr,"0x741000")) {
			/*
			 * The value of 0x741000 is
			 * the only valid value under the old
			 * SCSD implementation. In that implementation
			 * one drive type had an led of 741, which
			 * was represented in left justified binary
			 * nibbles. 
			 */
			strncpy(led,tmpstr,5);
			led[5] = '\0';
		} else {
			/*
			 * Since this value is not 0x741000, then
			 * this SCSD device must be using the 
			 * correct ASCII implementation of the led
			 * number.
			 */
			sprintf(led,"0x%c%c%c",
				scsd_vpd.led_no[0],
				scsd_vpd.led_no[1],
				scsd_vpd.led_no[2]);

		}


		/* Only rewrite if actually changed */
		if (strcmp(cuat->value,led)) {
			strcpy(cuat->value, led );
			putattr(cuat);
		}

		/* 
		 * Determine correct display message for this
		 * device
		 */


		switch (scsd_vpd.technology_code) {
		    case SCSD_DISK:
			if (wbus16_flg) {
				/*
				 * If this is a 16 bit device
				 * then set for 16 bit message
				 */
				if (scsd_vpd.interface_id == SCSD_SE) {
					strcpy(msg,"53");
				}
				else if (scsd_vpd.interface_id == SCSD_DIFF) {
					strcpy(msg,"54");
				}
				else {
					strcpy(msg,"53");
				}
			}
			else {
				/*
				 * This is not
				 * a 16 bit device
				 */
				if (scsd_vpd.interface_id == SCSD_SE) {
					strcpy(msg,"51");
				}
				else if (scsd_vpd.interface_id == SCSD_DIFF) {
					strcpy(msg,"52");
				}
				else {
					strcpy(msg,"51");
				}
			}
			break;

		    case SCSD_OEM_DISK:
		    default:
			strcpy(msg,"8");
			
		}
		
		/*
		 * Try to update ODM
		 */
		if ((cuat = getattr(cusobj->name,"message_no",0,&bc)) 
		    == (struct CuAt *)NULL ) {
			DEBUG_0("ERROR: getattr() failed\n")
				return E_NOATTR;
		}
		
		/* Only rewrite if actually changed */
		if (strcmp(cuat->value,msg)) {
			strcpy(cuat->value, msg);
			putattr(cuat);
		}
		

		/*
		 * Build diagnostic string
		 */
		sprintf(diagstr,"%02x%02x",scsd_vpd.operation_flags,
			scsd_vpd.diagdata1);
		
		for (i = 0; i < SCSD_LEN_DIAG2; i++) {
			sprintf(diagstr,"%s%02x",diagstr,
				scsd_vpd.diagdata2.diagbytes.bytes[i]);
		}



		if ((cuat =getattr(cusobj->name,"diag_scsd",0,&bc))
		    == (struct CuAt *)NULL ) {
			DEBUG_0("ERROR: getattr() failed\n")
				return E_NOATTR;
		}
		/* Only rewrite if actually changed */
		if (strcmp(cuat->value,diagstr)) {
			strcpy(cuat->value, diagstr );
			putattr(cuat);
		}


		dds->recovery_limit=scsd_vpd.recovery_limit;
				

		pg=0xC8;
		rc = get_inquiry_data(cusobj,pg,&scsd_mode_data);
		if (rc == 0) {
			dds->mode_data_length = 0;
			dds->mode_data[0] = '\0';
		}else {
			dds->mode_data_length = scsd_mode_data.page_length;
			bcopy(scsd_mode_data.mode_data,
			      dds->mode_data,
			      scsd_mode_data.page_length);
		}
	
		pg=0xC9;
		rc = get_inquiry_data(cusobj,pg,&scsd_deflt_data);
		if (rc == 0) {
			dds->mode_default_length = 0;
			dds->mode_default_data[0] = '\0';
		}else {
			dds->mode_default_length = scsd_deflt_data.page_length;
			bcopy(scsd_deflt_data.mode_data,
			      dds->mode_default_data,
			      scsd_deflt_data.page_length);
		}
		
	} /* end scsd_flg */


	else {
		
		/*
		 * This is for non-SCSD disk drives.
		 */
		
		if ((rc=getatt(alist,"reassign_to",&dds->reassign_timeout,'h',
			       &bc)) == E_NOATTR) {
			/*
			 * Do not fail if reassign timout is not in ODM, 
			 * just assume default.
			 */
			dds->reassign_timeout = DK_REASSIGN_TIMEOUT;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(alist,"start_timeout",&dds->start_timeout,'h',
			       &bc)) == E_NOATTR) {
			/*
			 * Do not fail if start unit timeout is not in ODM, just assume
			 * default.
			 */
			dds->start_timeout = DK_START_TIMEOUT;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(alist,"rw_timeout",&dds->rw_timeout,'h',
			       &bc)) == E_NOATTR) {
			/*
			 * Do not fail if read/write attribute is not in ODM, just 
			 * assume default.
			 */
			dds->rw_timeout = DK_TIMEOUT;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(alist,"safe_relocate",&dds->safe_relocate,'c',
			       &bc)) == E_NOATTR) {
			/*
			 * Do not fail if safe_relocate is not in ODM, just assume
			 * default.
			 */
			dds->safe_relocate = 1;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(alist,"extended_rw",&dds->extended_rw,'c',&bc)) == E_NOATTR) {
			/*
			 * Do not fail if extended_rw is not in ODM, just assume
			 * default (i.e. extended read/write is supported)
			 */
			dds->extended_rw = 1;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(alist,"reset_delay",&dds->reset_delay,'c',&bc)) 
		    == E_NOATTR) {
			/*
			 * Do not fail if reset_delay is not in ODM, just assume
			 * default (i.e. reset_delay is not needed)
			 */
			dds->reset_delay = 0;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		
		if ((rc=getatt(alist,"recovery_limit",&dds->recovery_limit,'i',&bc)) 
		    != 0) {
			return(rc);
		}
		
		
		if ((rc=getatt(alist,"q_type",data_value,'s',&bc)) == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just assume
			 * no queuing
			 */
			data_value[0] = '\0';
			
			
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((!strcmp(data_value,"ordered"))) {
			dds->q_type = SC_ORDERED_Q;
		}
		else if ((!strcmp(data_value,"simple"))) {
			dds->q_type = SC_SIMPLE_Q;
		}
		else {
			/*
			 * If invalid q_type or default q_type 
			 * then set it to no queuing
			 */
			dds->q_type = SC_NO_Q;
		}

		
		if ((rc=getatt(alist,"q_err",data_value,'s',&bc)) == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just assume
			 * default (i.e. set Qerr bit if supported otherwise
			 * device does not clear it queue on error).
			 */
			data_value[0] = '\0';
			
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		if ((!strcmp(data_value,"no"))) {
			dds->q_err_value = FALSE;
		}
		else {
			/*
			 * If invalid qerr_value or "yes" then set it to default (i.e.
			 * device driver should set Qerr bit if it is supported
			 * by the device.
			 */
			dds->q_err_value = TRUE;
		}
		
		
		if ((rc=getatt(alist,"clr_q",data_value,'s',&bc)) == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just assume
			 * default (i.e. device does not clear its queue on error).
			 */
			data_value[0] = '\0';
			
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		if ((!strcmp(data_value,"yes"))) {
			dds->clr_q_on_error = TRUE;
		}
		else {
			/*
			 * If invalid clr_q_on_error or "no" then set it to FALSE (i.e.
			 * the device does not clear its queue on error.
			 */
			dds->clr_q_on_error = FALSE;
		}
		
		if ((rc=getatt(alist,"max_data_rate",
			       &max_dev_data_rate,'i',&bc)) == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just set
			 * dds->buffer_rate = 0.
			 */
			max_dev_data_rate = 0;
			
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(alist,"media_rate",&dev_data_rate,'i',&bc)) 
		    == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just 
			 * set dds->buffer_rate = 0.
			 */
			dev_data_rate = 0;
			
		}
	else if (rc != 0) {
		/* 
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}
	/*
	 * Convert to float for calculation below
	 */
	media_data_rate = dev_data_rate;
	if ((max_dev_data_rate > 0) && (dev_data_rate > 0)) {
		
                if ((cuat=getattr(parobj->name,"max_data_rate",
						 0,&bc))
		    == (struct CuAt *)NULL) {
			/*
			 * don't fail if this attribute is not there, 
			 * just set dds->buffer_ration = 0.
			 */
			adap_data_rate = 0;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else
			adap_data_rate = atoi(cuat->value);
	}
		
		dds->buffer_ratio = 0;
		
		if ((max_dev_data_rate > 0) && 
		    (dev_data_rate > 0) && (adap_data_rate > 0)) {	
			/*
			 * All the essential attributes were found so compute the 
			 * buffer ratio and cause the SCSI disk device driver
			 * to override the mode_data attribute buffer ratio with
			 * this new one.
			 */
			
			/*
			 * The data rate we use in the calculation
			 * is the minimum of the adapter data rate and the
			 * maximum device data rate.
			 */
			
			if (max_dev_data_rate > adap_data_rate)
				data_rate = adap_data_rate;
			else
				data_rate = max_dev_data_rate;
			
			if (data_rate != 0) {
				/*
				 * Convert data rate to float 
				 */
				bus_rate = data_rate;
				data_rate = 256 * (bus_rate - media_data_rate)/bus_rate;
				
				/*
				 * Guarantee that dds->buffer_ratio is non-negative
				 */
				if (data_rate < 0)
					dds->buffer_ratio = -data_rate;
				else
					dds->buffer_ratio = data_rate;
			}
			
		}
		
		if ((rc=getatt(alist,"mode_data",dds->mode_data,'b',&bc)) != 0)
			return(rc);
		
		dds->mode_data_length = bc;
		
		if ((rc=getatt(alist,"mode_default",dds->mode_default_data,'b',&bc)) 
		    == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just assume
			 * there is no mode default data
			 */
			dds->mode_default_length = 0;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else
			dds->mode_default_length = bc;
		
		
		/* The following is used to correct for 355mb and 670mb mode select
		 * data for drives which have version numbers less than 0x30.
		 */
		if ((strcmp(cusobj->PdDvLn_Lvalue, "disk/scsi/670mb") == 0 ) ||
		    (strcmp(cusobj->PdDvLn_Lvalue, "disk/scsi/355mb") == 0)) {
			
			if ((strncmp( &inq_data[8], "IBM", 3) != 0) ||
			    (inq_data[32] < (char)(0x30))) {
				
				if ((rc=getatt(alist,"alt_mode_data",&dds->mode_data,
					       'b',&bc)) != 0)
					return(rc);
				dds->mode_data_length = bc;
				
			} else if (inq_data[32] < (char)(0x33)) {
				if ((rc=getatt(alist,"alt_mode_data2",&dds->mode_data,
					       'b',&bc)) != 0)
					return(rc);
				dds->mode_data_length = bc;
			}
		}
		
	}/* end else */	
	
	
	
	/*
	 * The following is code common to both SCSD and non-SCSD
	 * devices.
	 */	
        if ((rc=getatt(alist,"block_size",&dds->block_size,'i',&bc))
            == E_NOATTR) {
                /*
                 * Do not fail if block_size is not in ODM, just assume
                 * default.
                 */
                dds->block_size = DK_BLOCKSIZE;
        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }
	
        if ((rc=getatt(alist,"max_coalesce",&dds->max_coalesce,'i',&bc))
            == E_NOATTR) {
                /*
                 * Do not fail if max_coalesce is not in ODM, just assume
                 * default
                 */
                dds->max_coalesce = 0x10000;
        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }


       if ((rc=getatt(alist,"reserve_lock",
                       data_value,'s',&bc)) == E_NOATTR) {
                /*
                 * don't fail if this attribute is not there, just set
                 * it to TRUE.
                 */
                dds->reserve_lock = TRUE;

        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }
        else {
                if ((!strcmp(data_value,"yes")))
                        dds->reserve_lock = TRUE;
                else
                        dds->reserve_lock = FALSE;
        }
	

	/* device idle time */
	if ((rc=getatt(alist,"pm_dev_itime",&dds->pm_dev_itime,
			'i',&bc)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_itime = 0.
		 */
		dds->pm_dev_itime = 0;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}
	/* device standby time */
	if ((rc=getatt(alist,"pm_dev_stime",&dds->pm_dev_stime,
			'i',&bc)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_stime = 0.
		 */
		dds->pm_dev_stime = 0;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}
	/* device attribute */
	if ((rc=getatt(alist,"pm_dev_att",&dds->pm_dev_att,
			'i',&bc)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_dev_att = 0. (no attribute)
		 * If no attribute, it is handled as PM_OTHER_DEVICE.
		 */
		dds->pm_dev_att = PM_OTHER_DEVICE;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}

	if ((rc=getatt(alist,"pm_devid",&dds->pm_device_id,
			'i',&bc)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_devid = PMDEV_UNKNOWN_SCSI;
		 */
		dds->pm_device_id = PMDEV_UNKNOWN_SCSI;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}



	
	/* Get sid and lun and save in DDS */
	if ( get_sid_lun( cusobj->connwhere, &sid, &lun) )
                return( E_INVCONNECT );
	dds->scsi_id = sid;
	dds->lun_id = lun;
	
	dds->pm_device_id |= (sid << 6) | lun;
	
	/*
	 * Detemine if this disk is a SCSI-2 disk and if it supports command
	 * tag queuing
	 */
	
	
	/*
	 * First get ANSI type by masking of the last 3 bits of
	 * byte 2
	 */
	ansi_type = inq_data[2] & 0x7; 
	
	/*
	 * Get value of cmdque bit from inquiry
	 * data
	 */
	cmdque_bit = inq_data[7] & 0x2;

	if ((ansi_type >= 0x2) && (cmdque_bit == 0x2) &&
	    (dds->q_type != SC_NO_Q)) {
		/*
		 * If this is a SCSI-2 Disk and it supports command tag 
		 * queuing and the ODM database says the system should try to 
		 * queue to it, then tell the SCSI disk device driver
		 * that the disk supports command tag queuing.
		 */
		
		dds->cmd_tag_q = TRUE;
	}
	else {
		/*
		 * set it to no queuing
		 */
		dds->q_type = SC_NO_Q;
		dds->cmd_tag_q = FALSE;
	}
	
	if (ansi_type == 1) {
		/*
		 * This device is a SCSI-1 device.  So limit
		 * it to read(6)/write(6) SCSI commands. In other
		 * words don't allow it to use extended read/write 
		 * commands, because these are not supported by all
		 * SCSI-1 devices.
		 */
		dds->extended_rw = 0;
	}
	/* Generate Parent's devno */
	major_no = genmajor( parobj->ddins );
	minor_no = *getminor( major_no, &how_many, parobj->name );	
	dds->adapter_devno = makedev( major_no, minor_no );
	
	DEBUG_1( "dds->adapter_devno = %ld\n", dds->adapter_devno)
		
	*dds_data_ptr = (char *)dds;		/* Store address of struct*/
	*dds_len = sizeof( struct disk_ddi );	/* Store size of structure*/

#ifdef CFGDEBUG
	hexdump(*dds_data_ptr, (long)*dds_len);
#endif

	DEBUG_0( "build_dds() returning 0\n" )

	return(E_OK);
}



/*
 * NAME: disk_present
 *
 * FUNCTION: Device dependent routine that checks to see that the disk device
 *           is still at the same connection location on the same parent as
 *           is indicated in the disk's CuDv object.
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:     0 if succeed, or errno if fail
 */

int
disk_present(disk,preobj,parent,pvidattr)
struct  CuDv    *disk;          /* disk's CuDv object */
struct  PdDv    *preobj;        /* disk's PdDv object */
struct  CuDv    *parent;        /* disk's parents CuDv object */
char            *pvidattr;      /* disk's CuAt pvid attribute value */
{
        struct  CuAt dpvidattr, sys_cuattr;
        struct  CuDv    tmpcudv;
        int     dpvid,adap;
        int     rc;
        char    pvidstr[33];
        char    adapname[64];
        char    sstr[256];
        uchar   sid,lun;                /* SCSI Id and LUN of disk */
	int	error;

	rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
				 &sys_cuattr);
	if (rc == -1) {
		DEBUG_0("ODM error getting rds_facility attribute\n")
		return(E_ODMGET);
	} else if (rc != 0 && sys_cuattr.value[0]=='y') {
                rc = rds_disk_present(disk,preobj,parent,pvidattr);
                return (rc);
        }
        /* make sure parent is configured */
        if (parent->status!=AVAILABLE) {
                DEBUG_1("disk_present: parent status=%d\n",parent->status)
                return(CALL_FINDDISK);
        }

        /* Check to see if there is another disk already AVAILABLE at the */
        /* same parent/parent connection.  If there is, then will need to */
        /* call finddidsk() to see if this disk was moved. */
        sprintf(sstr,"parent = '%s' AND connwhere = '%s' AND status = '%d'",
                        disk->parent, disk->connwhere, AVAILABLE);
        rc = (int)odm_get_first(CuDv_CLASS,sstr,&tmpcudv);
        if (rc == -1) {
                DEBUG_0("disk_present: ODM error.\n")
                return(E_ODMGET);
        }
        else if (rc) {
                DEBUG_1("disk_present: %s already cfged at this location.\n",
                                                                tmpcudv.name)
                return(CALL_FINDDISK);
        }

        /* get scsi id and lun from connwhere field */
        if ( get_sid_lun( disk->connwhere, &sid, &lun) )
                return( E_INVCONNECT );

        /* get inquiry data from drive */
        sprintf(adapname,"/dev/%s",disk->parent);
        if ((adap = open(adapname,O_RDWR))<0) {
                /* error opening adapter */
                DEBUG_2("disk_present: error opening %s, errno=%d\n",adapname,errno)
                return(CALL_FINDDISK);
        }

        /* issue SCIOSTART for this id/lun */
        if (ioctl(adap,SCIOSTART,IDLUN((int)sid,(int)lun))< 0) {
                DEBUG_3("disk_present: Failed to SCIOSTART device at sid %d lun %d errno=%d\n",
                        sid,lun,errno)
                close(adap);
                return(CALL_FINDDISK);
        }

        /* get inquiry data to make sure this is a disk */
        DEBUG_3("disk_present: calling issue_scsi_inq adap=%s sid=%d lun=%d\n",
                adapname,sid,lun)
        if ((rc=issue_scsi_inquiry(adap,inq_data,sid,lun,(int)NO_PAGE,&error))!=0) {
                /* error getting inquiry data from drive */
                DEBUG_4("disk_present: SCIOINQU error rc=%d adap=%s sid=%d lun=%d\n",
                        rc,adapname,sid,lun)
                ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun));
                close(adap);
                return(CALL_FINDDISK);
        }

        /* make sure inquiry data is for a disk */
        if ((inq_data[0] & '\037') != '\000') {
                DEBUG_0("Device is of wrong class entirely\n")
                ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun));
                close(adap);
                return (CALL_FINDDISK); /* Device class is incorrect */
        }

        /* make sure disk is spun up */
        DEBUG_2("disk_present: calling spin_up_disk sid=%d lun=%d\n",sid,lun)
 	if (spin_up_disk(adap,sid,lun) != 0) {
           DEBUG_2("disk_present: Failed to spin up disks sid=%d lun=%d\n",sid,lun)
                ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun));
                close(adap);
                return(-1);
        }

        /* now get final inquiry data */
        DEBUG_3("disk_present: calling issue_scsi_inq adap=%s sid=%d lun=%d\n",
                adapname,sid,lun)
        if ((rc=issue_scsi_inquiry(adap,inq_data,sid,lun,(int)NO_PAGE,&error))!=0) {
                /* error getting inquiry data from drive */
                DEBUG_4("disk_present: SCIOINQU error rc=%d adap=%s sid=%d lun=%d\n",
                        rc,adapname,sid,lun)
                ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun));
                close(adap);
                return(CALL_FINDDISK);
        }

#ifdef CFGDEBUG
        /* look at inquiry data */
        hexdump(inq_data,(long)(inq_data[4]+5));
#endif

        /* get pvid from the disk */
        get_pvid(adap,sid,lun,pvidstr);

        /* issue SCIOSTOP to sid/lun */
        if (ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun))<0) {
                DEBUG_2("disk_present: Failed to SCIOSTOP device at scsi %d lun %d\n",
                        (int)sid, (int)lun)
                close(adap);
                return(CALL_FINDDISK);
        }

        close(adap);

        /* make sure drive is the right type */
        DEBUG_0("disk_present: calling chktype\n")
        rc = chktype(inq_data,disk->PdDvLn_Lvalue,SCSI_DISK,disk->parent,sid,lun);
        if (rc) {
                if (rc == E_ODMGET) {
                        /* error checking on drive types */
                        DEBUG_0("ODM error\n")
                        return(E_ODMGET);
                } else {
                        DEBUG_0("Wrong type of disk\n")
                        return(CALL_FINDDISK);
                }
        }

/* SHOULDNT WE CHECK FOR SAME DRIVE TYPE IN FOLLOWING TEST? */
        /* if the drive has a pvid then see if there is an object for it in
           the database */
        dpvid = 0;                      /* initialize to no pvid attr */
        if (*pvidstr) {
                sprintf(sstr,"attribute = 'pvid' AND value = '%s'",pvidstr);
                dpvid = (int)odm_get_first(CuAt_CLASS,sstr,&dpvidattr);
                if (dpvid == -1) {
                        return(E_ODMGET);
                }
        }

        /* If the CuDv disk object we want to configure has NO pvid attribute
         * but the disk has a pvid on it and there's a matching pvid
         * attribute in the database
         *
         * OR
         *
         * If the CuDv disk object we want to configure HAS a pvid attribute
         * and the disk has a pvid attribute on it but they don't match
         *
         * THEN
         *     the disk is the wrong device
         */
        DEBUG_1("disk pvid = %s\n",pvidstr)
        DEBUG_1("CuAt pvid = %s\n",pvidattr)
        if (((pvidattr[0]=='\0') && *pvidstr && dpvid) ||
            ((pvidattr[0]!='\0') && strcmp(pvidattr,pvidstr))) {
                rc = CALL_FINDDISK;
        } else {
                rc = E_OK;
        }
        DEBUG_1("disk_present: returning %d\n",rc)
        return(rc);
}

/*
 * NAME: get_pvidstr
 *
 * FUNCTION: Copies the pvid string into the input string.
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 * 0. if succeeded
 *
 */

int
get_pvidstr(disk, dest_pvid)
struct  CuDv *disk;
char    *dest_pvid;
{
        int     adap;
        int     rc;
        char    adapname[64];
        char    sstr[256];
        uchar   sid,lun;                /* SCSI Id and LUN of disk */

        /* BEGIN get_pvidstr */


        /* get scsi id and lun from connwhere field */
        if ( get_sid_lun( disk->connwhere, &sid, &lun) )
                return( E_INVCONNECT );

        /* get inquiry data from drive */
        sprintf(adapname,"/dev/%s",disk->parent);
        if ((adap = open(adapname,O_RDWR))<0) {
                /* error opening adapter */
                DEBUG_2("get_pvidstr: error opening %s, errno=%d\n",adapname,errno)
                return(-1);
        }

        /* issue SCIOSTART for this id/lun */
        if (ioctl(adap,SCIOSTART,IDLUN((int)sid,(int)lun))< 0) {
                DEBUG_3("get_pvidstr: Failed to SCIOSTART device at sid %d lun %d errno=%d\n",
                        sid,lun,errno)
                close(adap);
                return(-1);
        }
 
        /* make sure disk is spun up */

 	if (spin_up_disk(adap,sid,lun) != 0) {
           DEBUG_2("get_pvidstr: Failed to spin up disks sid=%d lun=%d\n",sid,lun)
                ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun));
                close(adap);
                return(-1);
        }


        /* get pvid from the disk */
        get_pvid(adap,sid,lun,pvid_str);
        strcpy(dest_pvid, pvid_str) ;

        /* issue SCIOSTOP to sid/lun */
        if (ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun))<0) {
                DEBUG_2("get_pvidstr: Failed to SCIOSTOP device at scsi %d lun %d\n",
                        (int)sid, (int)lun)
                close(adap);
                return(-1);
        }

        close(adap);
        return(0) ;
}



/*
 * NAME: bb_disk_notrdy
 *
 * FUNCTION: Used to see when the disk has been spun up.
 *
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called only by disk_present().
 *
 * RETURNS:     0 if succeed, or errno if fail
 */

int bb_disk_notrdy(adapter,id,lun,immed_flag)
int     adapter;
uchar   id,lun;
int     immed_flag;
{
uchar   *p;
struct  sc_ready  tr_pb;
int     rc,ntur,try_async,total_time;

        /* clear sc_ready structure */
        p = (uchar *)&tr_pb;
        ntur = sizeof(struct sc_ready);
        while(ntur--)*p++ = 0;

        tr_pb.flags = 0;
        tr_pb.scsi_id = id;
        tr_pb.lun_id = lun;

        total_time = 0;
        try_async = 0;

        for(ntur = 0;; ntur++){
            tr_pb.status_validity = 0;
            tr_pb.scsi_status = 0;
            rc = ioctl(adapter,SCIOTUR,&tr_pb);
            if(rc == 0) {
                DEBUG_2("bb_turdy: validity=%d status=0x%x\n",
                    tr_pb.status_validity,tr_pb.scsi_status)
                break;
            }                           /* else error condition */
            if(ntur == 0)continue;              /* retry once anyway */
            /* the following allows retry, but does not wait until
               device comes ready */
            if((ntur > 1) && (immed_flag))break;
            /* clear vendor unique and reserved bits from scsi status */
            tr_pb.scsi_status &= SCSI_STATUS_MASK;

            if(errno == ENOCONNECT){
                try_async = 1;
                break;
            }
            else {
                if((errno == EIO) &&
                   (tr_pb.status_validity == SC_SCSI_ERROR) &&
                   (tr_pb.scsi_status != SC_RESERVATION_CONFLICT)) {
                    DEBUG_2("bb_turdy: validity=%d status=0x%x\n",
                        tr_pb.status_validity,tr_pb.scsi_status)
                    if (total_time >= SUP_TIMEOUT) {
                        break;          /* timeout condition met */
                    }
                    else {
                        sleep(TUR_WAIT);
                        total_time += TUR_WAIT;
                        /* command is now retried */
                    }
                }
                else {
                    DEBUG_3("turdy:err=%d on id%d lun%d\n",errno,id,lun)
                    break;
                }
            }
        }       /* end for ntur loop */

        if(rc && (try_async == 1)){     /* on enoconnect try async once */
            tr_pb.flags = SC_ASYNC;
            rc = ioctl(adapter,SCIOTUR,&tr_pb);
        }
        return(rc);
}


/*
 * NAME: bb_disk_spinup
 *
 * FUNCTION: Used to spun up a disk.
 *
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called only by disk_present().
 *
 * RETURNS:     0 if succeed, or errno if fail
 */

int bb_disk_spinup(adapter,id,lun)
int     adapter;
uchar   id,lun;
{
uchar   *p;
struct  sc_startunit su_pb;
int     nsup,tried_async,rc;

        /* clear startunit structure */
        p = (uchar *)&su_pb;
        nsup = sizeof(struct sc_startunit);
        while(nsup--)*p++ = 0;

        tried_async = 0;
        su_pb.flags = 0;
        su_pb.scsi_id = id;
        su_pb.lun_id = lun;
        su_pb.start_flag = TRUE;        /* start the unit */
        su_pb.immed_flag = TRUE;        /* immediate return */
        su_pb.timeout_value = SUPI_TIMEOUT;
        for(nsup = 0; nsup < 3; nsup++){

            rc = ioctl(adapter,SCIOSTUNIT,&su_pb);
            if(rc == 0) break;  /* else error condition */
            if(nsup > 0){
                if((errno == ENOCONNECT) && (tried_async == 0)){
                    tried_async = 1;
                    su_pb.flags = SC_ASYNC;
                }

                else {
                    DEBUG_3("bb_spinup:err=%d on id%d lun%d.\n",errno,id,lun)
                    rc = -1;
                }


            }
        }       /* end for nsup loop */
        return(rc);
}


/*
 * NAME: get_pvid
 *
 * FUNCTION: Low level routine used to format pvid from disk's boot record.
 *
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called only by disk_present() and
 *    get_inquiry_data().
 *
 * RETURNS:     none
 */

void
get_pvid(adap,sid,lun,pvidstr)
int     adap;
uchar   sid,lun;                /* SCSI Id and LUN of disk */
char    *pvidstr;
{
        int     rc;
        uchar   pvidbuf[4096];
        IPL_REC *iplrec;

        /* get pvid from the disk */
        if (read_scsi_pvid(adap,sid,lun,pvidbuf)!=0) {
                /* error reading pvid, assume NO pvid */
                DEBUG_0("get_pvid: error reading pvid")
                pvidstr[0] = '\0';
        } else {
                iplrec = (IPL_REC *)pvidbuf;
                if (iplrec->IPL_record_id == (uint)IPLRECID) {
                        strcpy(pvidstr,pvidtoa(&(iplrec->pv_id)));
                        if (!strcmp(pvidstr,NULLPVID))
                                pvidstr[0] = '\0';
                } else
                        pvidstr[0] = '\0';
        }
        return;
}

/*
 * NAME: read_scsi_pvid
 *
 * FUNCTION: Low level routine used to read boot record from disk.
 *
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called only by get_pvid(),
 *
 * RETURNS:     0
 */

int read_scsi_pvid(adapter, id, lun, ptr)
int     adapter;
uchar   id, lun;
uchar    *ptr;
{
        struct sc_readblk pvid_data;
        int     blksiz;
        uchar   tried_async;
        char    devname[32];
        int     rc;
        int     tries;

        DEBUG_4("read_scsi_pvid: adapter_fd=%d, id=%d,lun=%d,ptr=%x\n",
                adapter,id,lun,ptr);

        pvid_data.scsi_id = id;
        pvid_data.lun_id  = lun;
        pvid_data.blkno   = 0;
        pvid_data.timeout_value = DK_TIMEOUT;
        pvid_data.data_ptr = ptr;
        pvid_data.rsv1  = 0;
        pvid_data.rsv2  = 0;
        pvid_data.rsv3  = 0;
        pvid_data.rsv4  = 0;
        pvid_data.flags = 0;

        for (blksiz = 512; blksiz <= 4096; blksiz <<= 1) {

                pvid_data.blklen  = blksiz;

                for (tries = 0; tries < 3; tries++) {
                        rc = ioctl(adapter, SCIOREAD, &pvid_data);
                        if (rc == 0) {
                                return(rc);
                        }
                        DEBUG_2("read_scsi_pvid: blksiz=%d,tries = %d\n",
                                blksiz,tries);
                        /* else error condition */
                        if (tries > 0) {

                                switch (errno) {
                                      case ENODEV:
                                        DEBUG_2("read_scsi_pvid: ENODEV on %d %d. try ASYNC\n",
                                                id, lun);
                                        tried_async=1;
                                        pvid_data.flags |= SC_ASYNC;
                                      case ENOCONNECT:
                                        tried_async=1;
                                        pvid_data.flags |= SC_ASYNC;
                                        DEBUG_2("read_scsi_pvid: ENOCONNECT @ %d %d. try ASYNC\n",
                                                id, lun);
                                        break;
                                      case ETIMEDOUT:
                                        pvid_data.timeout_value = DK_TIMEOUT;
                                        break;
                                      case EIO:
                                        DEBUG_2("read_scsi_pvid: EIO @ %d %d. do new blocksize\n",
                                                id, lun);
                                        break;
                                      default:
                                        DEBUG_3("read_scsi_pvid:err=%d on %d %d\n",
                                                errno, id, lun);
                                        rc = -1;
                                }
                        }
                }
        }
        return(rc);
}

/*
 * NAME: spin_up_disk
 *
 * FUNCTION: check if disk spun up, if not, spin it up and wait
 *	     for it to spin up.
 *
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called by get_pvidstr and disk_
 *    present.
 *
 * RETURNS:  0 if success, else -1
 */

int spin_up_disk(adap, sid, lun )
int     adap;
uchar   sid, lun;
{
					/* set immediate flag */
     if (bb_disk_notrdy(adap,sid,lun,1) !=0 ) {

        /* make sure disk is spun up */
        DEBUG_2("spin_up_disk: calling bb_disk_spinup sid=%d lun=%d\n",sid,lun)
        if (bb_disk_spinup(adap,sid,lun)!=0) {
                DEBUG_2("spin_up_disk: error spinning up %s, errno=%d\n",adap,errno)
                ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun));
                close(adap);
                return(-1);
        }

        /* wait for drive to spin up */
        DEBUG_2("spin_up_disk: calling bb_disk_notrdy sid=%d lun=%d\n",sid,lun)
        if (bb_disk_notrdy(adap,sid,lun,0)!=0) {
                DEBUG_2("spin_up_disk: error spinning up %s, errno=%d\n",
                        adap,errno)
                ioctl(adap,SCIOSTOP,IDLUN((int)sid,(int)lun));
                close(adap);
                return(-1);
        }
    } /* end if */
    return (0);

}

