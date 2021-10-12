static char sccsid[] = "@(#)38  1.31  src/bos/usr/lib/methods/cfgsccd/cfgsccd.c, cfgmethods, bos41J, 9521A_all 5/23/95 17:49:32";
/*
 * COMPONENT_NAME: (CFGMETHODS) SCSI CD-ROM and SCSI R/W optical drive
 *			     	Config method
 *
 * FUNCTIONS: generate_minor, make_special_files, download_microcode,
 * FUNCTIONS: query_vpd, define_children, build_dds
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
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/device.h>
#include	<sys/sysconfig.h>
#include	<sys/cfgdb.h>
#include	<sys/stat.h>
#include	<sys/scsi.h>
#include	<sys/sysmacros.h>
#include	<fcntl.h>
#include	<malloc.h>
#include	<sys/watchdog.h>
#include	<sys/scdiskdd.h>
#include	<sys/cfgodm.h>
#include	"cfgdebug.h"
#include	"cfghscsi.h"
#include	"cf.h"
#include 	<string.h>
#include 	<sys/pmdev.h>


#define	INQSIZE	255
static	uchar	inq_data[INQSIZE];	/* Storage area for inquery data */



/*
 * NAME: generate_minor
 *
 * FUNCTION: Generates a minor number for the SCSI CD-ROM or SCSI
 *	     R/W optical drive being configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 *
 * RETURNS:
 * The 0, or errno if an error occurs.
 * The minor number is passed back via the pointer in the last parameter
 * 
 */

long generate_minor( logical_name, major_no, minor_dest )
char *logical_name;		/* Name of device			*/
long major_no;			/* Major no. already generated		*/
long *minor_dest;		/* Address to store minor number	*/
{
	long	*genret;
	long *genminor();

	/* Request a minor number. Only one number is required, and	*/
	/* there is no preferred number, nor is there a boundary	*/

	DEBUG_0( "generate_minor()\n" )

	if(( genret=genminor( logical_name, major_no, -1, 1, 1, 1 )) == NULL )
		return E_MINORNO;

	*minor_dest = *genret;
	return 0;
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: Generates two special files for the SCSI CD-ROM or the SCSI
 *	     read/write optical drive being configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 * 2. The files generated are /dev/<logical_name>, and /dev/r<logical_name>
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */

#define MSP_MAXLEN	14	/* Maximum length of a file name	*/

int make_special_files( logical_name, devno )
char	*logical_name;
dev_t	devno;
{
	char basename[MSP_MAXLEN+1];
	char sstr[100];
	struct CuDv cusobj;
	int perm_mask;
	int rc;

	DEBUG_0( "make_special_files()\n" )

	/* Check that there is enough room in basename for r<logical_name> */

	if( strlen(logical_name) > (MSP_MAXLEN-1) )
		return E_MAKENAME;


	sprintf(sstr,"name = %s",logical_name);
	rc = (int) odm_get_obj(CuDv_CLASS,sstr,&cusobj,ODM_FIRST);
	if ((!rc) || (rc < 0)) {
		/*  Error finding device in CuDv so fail this routine*/
		return E_MAKENAME;
	}
	if (!strncmp(cusobj.PdDvLn_Lvalue,"cdrom",5)) {
		/* 
		 * This is a CD-ROM Device so make the special files 
		 * with the appropriate permissions(444).            
		 */
		perm_mask = S_IRUSR | S_IRGRP | S_IROTH;
	}
	else if(!strncmp(cusobj.PdDvLn_Lvalue,"rwoptical",9)) {
		/* 
		 * This is a R/W Optical Device so make the special files 
		 * with the appropriate permissions (666).                
		 */
		perm_mask = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | 
			S_IWGRP | S_IWOTH;
	}
	else {
		/* Unknown device class */
		return E_MAKENAME;
	}
	/* Create the character special file */

	sprintf( basename, "r%s", logical_name );

	if( rc = mk_sp_file( devno, basename,
			( perm_mask | S_IFCHR ) ) )
		return(rc);

	/* Generate block file */

	sprintf( basename, "%s", logical_name );

	return( mk_sp_file( devno, basename,
		( perm_mask | S_IFBLK ) ) );
}

/*
 * NAME: download_microcode
 *
 * FUNCTION: Downloads micro-code to device
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *	assumed.
 * 2. The SCSI CD-ROMs and SCSI read/write optical drives only use micro-code 
 *	for diagnostics (if at all), therefore the diagnostic programs will 
 *	download their own micro-code.
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */

int download_microcode( lname  )
char	*lname;
{
	DEBUG_0( "download_microcode()\n" );

	return 0;
}

/*
 * NAME: query_vpd
 *
 * FUNCTION: Obtains Vital Product Data From SCSI-CD-ROM Device or
 *	     SCSI read/write optical device, and places it
 *		in the customized database
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */


int query_vpd( cusobj, kmid, devno, vpd_dest )
struct  CuDv    *cusobj;        /* Pointer to customized object rec             
*/
mid_t   kmid;                   /* Kernel module I.D. for Dev Driver    */
dev_t   devno;                  /* Concatenated Major & Minor No.s      */
char    *vpd_dest;              /* Destination for formatted vpd        */
{
    struct CuAt sys_cuattr;
    char        *fmt_str;
    int         vpd_len ;
    int         rc;
    char	device_type;
    uchar       sid ;                   /* SCSI ID of device.           */
    uchar       lun ;                   /* LUN of device (usually zero) */
    int         len ;                   /* length of data returned.     */

        /* switch on the device */
	rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
				&sys_cuattr);
	if (rc == -1) {
		DEBUG_0("ODM error getting rds_facility attribute\n")
		return(E_ODMGET);
	} else if (rc != 0 && sys_cuattr.value[0]=='y') {
                rc = rds_update_location(cusobj);
                if ( rc != 0)
                        DEBUG_0("Can not update the location code\n");
        }

    len = strtoul(cusobj->connwhere, NULL, 16) ;
    sid = (len >> 4) & 0x0f ;
    lun = len & 0x0f ;

    sid = strtoul(cusobj->connwhere,NULL,10);
    lun = strtoul(strchr(cusobj->connwhere,',')+1,NULL,10);


    if (!strncmp(cusobj->PdDvLn_Lvalue,"rwoptical",9)) {
	    /*
	     * Use default VPD mapping for SCSI read/write optical drives
	     * and set device type
	     */
	    fmt_str =  
		    "MF0808C,TM1010C,PN720cC,RL2004C,SN2408C,EC7e0aC,Z00008X,Z12c0cC";
	    device_type = SCSI_RWOPT;
    }
    else {
	    /*
	     * Use default VPD mapping for SCSI CD_ROM drives and set device
	     * type
	     */
	    fmt_str = "MF0808C,TM1010C,RL2004C,Z00008X,PN880cC,EC7e0aC,FN720cC" ;
	    device_type = SCSICDROM;
    }

    /* 
     * make sure that the device we're looking at is the same type
     * as the database says it is 
     */
    if ((rc=chktype(inq_data,cusobj->PdDvLn_Lvalue,device_type,cusobj->parent,sid,lun))!=0) {
	    DEBUG_1("query_vpd: chktype failed, rc=%d\n",rc)
            return(rc);
    }

    vpd_len = scvpd(cusobj, vpd_dest, fmt_str) ;
    if (vpd_len > 0) {
    /* vpd_len > 0 is good, got some/all VPD data */
#ifdef CFGDEBUG
        hexdump(vpd_dest, vpd_len) ;
#endif
        rc = 0 ;
    }
    else {
        DEBUG_0("query_vpd: No VPD received from scvpd\n") ;
        rc = E_VPD ;
    }
    return(rc);
} /* END query_vpd */


/*
 * NAME: define_children
 *
 * FUNCTION: Scans for child devices
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *		assumed.
 * 2. The SCSI CD-ROMs and SCSI read/write optical drives do not have child 
 *	devices, so this routine always succeeds
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 */

int define_children( lname, cusdev, phase, children )
char	*lname;
long	cusdev;
int		phase;
char	*children;
{
	DEBUG_0( "define_children()\n" )
	return 0;
}

/*
 * NAME: build_dds
 *
 * FUNCTION: Device dependent dds build for SCSI-CD-ROM or SCSI read/write
 *	     optical drives
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded routine.
 *
 * RETURNS:	0 if succeed, or errno if fail
 */

#define		CUSTOM	0	/* Customized attribute current value	*/

#ifdef CLOSE_RET
#undef CLOSE_RET
#endif
#define	CLOSE_RET(retval) {	\
		odm_close_class(PdAt_CLASS);	\
		odm_close_class(CuAt_CLASS);	\
		return retval;			\
	}

int build_dds( lname, dds_data_ptr, dds_len )
char	*lname;			/* Logical name				*/
char	**dds_data_ptr;		/* Address to store pointer to dds	*/
int	*dds_len;		/* Address to store length of dds	*/
{
	struct	disk_ddi	*dds;	/* dds for SCSI-CD-ROM from scdisk.h  */
	char	search_str[128];	/* Search criterion 		     */
	struct	Class	*c_att_oc;	/* Customized Attribute Object Class */
	struct	Class	*p_att_oc;	/* Predefined Attribute Object Class */
	struct	CuDv	cusobj;		/* Customized Device Object 	     */
	int	rc;			/* Return code 			     */
	long	major_no,		/* Major number of adapter 	     */
		minor_no;		/* Major No. for adapter 	     */
	int	how_many;
	int	bc;
	uchar	sid,lun;		/* SCSI Id and LUN of device 	     */
	long	*getminor();
	long	genmajor();
	int     ansi_type,cmdque_bit;
	int     max_dev_data_rate,dev_data_rate,adap_data_rate,data_rate;
	float   bus_rate, media_data_rate;
	char    data_value[30];
	struct  CuAt   sys_cuattr;
	struct  CuAt   *cuat;
	int	queue_depth;
	uchar   prevent_dflt;
	uchar   reserve_dflt;
	int	rw_timeout;
	struct	cd_scsd_inqry_data	scsd_vpd;
	struct  cd_scsd_mode_data	scsd_mode_data, scsd_deflt_data;
        int     pg,scsd_flg=FALSE;
	char	led[6],diagstr[30],msg[3];
	char    tmpstr[20];
	int	reset_delay;
	int     size_in_mb;
	int	i;
	int     wbus16_flg=0;		/* Non-zero indicates the device is a */
					/* 16 bit device.		      */

	DEBUG_0( "build_dds()\n" )

	if((dds = (struct disk_ddi *)malloc(sizeof(struct disk_ddi)))
		== NULL )
	{
		DEBUG_0("Cannot malloc space for struct cdrom_dds_df\n")
		return E_MALLOC;
	}

	bzero(dds, sizeof(struct disk_ddi));

        dds->segment_cnt = 0;
        dds->byte_count = 0;
        dds->async_flag = 0;
        dds->segment_size = 1000000;
	dds->max_request = 0x40000;
	dds->load_eject_alt = FALSE;	


	/* Open required Object Classes */

	if( (int)( c_att_oc = odm_open_class( CuAt_CLASS ) ) == -1 )
	{
		DEBUG_0( "build_dds() for SCSI-CD-ROM: Error opening CuAt\n")
		return E_ODMOPEN;
	}

	if( (int)( p_att_oc = odm_open_class( PdAt_CLASS ) ) == -1 )
	{
		DEBUG_0( "build_dds() for SCSI-CD-ROM: Error opening PdAt\n")
		return E_ODMOPEN;
	}

	/* Read CuDv for SCSI device */

	sprintf( search_str, "name = '%s'", lname);

	DEBUG_1( "Performing odm_get_obj(CuDv,%s)\n", search_str )

	rc = (int)odm_get_obj(CuDv_CLASS,search_str,&cusobj,ODM_FIRST);

	if( rc == 0 )
	{
		DEBUG_1( "cfgsccd: Record not found in CuDv: %s\n",search_str)
		CLOSE_RET( E_NOCuDv )
	} else if ( rc == -1 )
	{
		DEBUG_0( "build_dds() for SCSI-CD-ROM: Error Reading CuDv\n")
		CLOSE_RET( E_ODMGET )
	}

	DEBUG_1( "odm_get_obj(CuDv,%s) succeeded\n", search_str )

        /* switch on the device */
	rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
				&sys_cuattr);
	if (rc == -1) {
		DEBUG_0("ODM error getting rds_facility attribute\n")
		return(E_ODMGET);
	} else if (rc != 0 && sys_cuattr.value[0]=='y') {
          	rc = rds_power_on_device(&cusobj);
		if (rc != 0)
            		DEBUG_0("Problem in rds_power_on_device\n");
        }


	rc = get_inquiry_data(&cusobj,(int)NO_PAGE,inq_data);

	if (rc == 0)
		return(E_DEVACCESS);


	/*
	 * Detemine if this device is a SCSI-2 16 bit device.
	 */

	wbus16_flg = inq_data[7] & SCSI_WBUS16_MASK;

        if ((!strcmp(cusobj.PdDvLn_Lvalue,"cdrom/scsi/scsd")) ||
	    (!strcmp(cusobj.PdDvLn_Lvalue,"rwoptical/scsi/scsd"))) {
		pg=0xC7;
		rc = get_inquiry_data(&cusobj,pg,&scsd_vpd);
		if (rc == 0)
			return(E_WRONGDEVICE);

		if(strncmp(scsd_vpd.scsd_id_field,"SCDD",4)==0){
			scsd_flg=TRUE;
		}
		else
			return(E_WRONGDEVICE);
	} /* end strcmp */
	
	
	if ((rc=getatt(&queue_depth,'i',c_att_oc, p_att_oc, 
		       lname,cusobj.PdDvLn_Lvalue,
		       "queue_depth",NULL)) == E_NOATTR) {
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
	
	if (scsd_flg) {
		
		/* 
		 * If SCSD (Self Configuring SCSI Device) 
		 */

		if (!strncmp(cusobj.PdDvLn_Lvalue,"cdrom",5)) {
			/* CD-ROM drive */
			dds->dev_type = DK_CDROM;
			dds->safe_relocate = 0;
			prevent_dflt = FALSE;
			reserve_dflt = FALSE;
			dds->reassign_timeout = 1;
			dds->fmt_timeout = 0;
			dds->buffer_ratio = 0;
			rw_timeout = DK_TIMEOUT;			
		
			/* 
			 * Determine correct display message for this
			 * device
			 */
			
			switch (scsd_vpd.technology_code) {
			    case SCSD_CD_ROM:
				
				if (wbus16_flg) {
					/*
					 * If this is a 16 bit device
					 * then set for 16 bit message
					 */

					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"16");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"18");
					}
					else {
						strcpy(msg,"16");
					}

				}
				else {
					/*
					 * This is not
					 * a 16 bit device
					 */
					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"12");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"14");
					}
					else {
						strcpy(msg,"12");
					}
				}
				break;
			    case SCSD_MM_CD_ROM:
				if (wbus16_flg) {
					/*
					 * If this is a 16 bit device
					 * then set for 16 bit message
					 */
					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"17");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"19");
					}
					else {
						strcpy(msg,"17");
					}
				}
				else {
					/*
					 * This is not
					 * a 16 bit device
					 */				
					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"13");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"15");
					}
					else {
						strcpy(msg,"13");
					}
				}
				break;
				
			    case SCSD_OEM_CD_ROM:
			    default:
				strcpy(msg,"10");

				
			}

			
		} /* end scsd cdrom */
		
		else if (!strncmp(cusobj.PdDvLn_Lvalue,"rwoptical",9)) {
			/* R/W optical drive */
			dds->dev_type = DK_RWOPTICAL;
			dds->safe_relocate = 1;
			prevent_dflt = TRUE;
			reserve_dflt = TRUE;
			dds->buffer_ratio = 0;
			rw_timeout = DK_RWOPT_TIME;

			if (scsd_vpd.reassign_timeout != 0) {
				dds->reassign_timeout = scsd_vpd.reassign_timeout;
			} 
			else {
				dds->reassign_timeout=DK_REASSIGN_TIMEOUT;
			}

			if (scsd_vpd.fmt_timeout != 0) {
				dds->fmt_timeout=scsd_vpd.fmt_timeout;
			}
			else {
				dds->fmt_timeout=DK_FMT_TIMEOUT;
			}

			/* 
			 * Determine correct display message for this
			 * device
			 */
			

			switch (scsd_vpd.technology_code) {
			    case SCSD_RWOPT:
				if (wbus16_flg) {
					/*
					 * If this is a 16 bit device
					 * then set for 16 bit message
					 */
					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"26");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"27");
					}
					else {
						strcpy(msg,"26");
					}					
				}
				else {
					/*
					 * This is not
					 * a 16 bit device
					 */
					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"22");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"23");
					}
					else {
						strcpy(msg,"22");
					}
				}
				

				break;
			    case SCSD_MM_RWOPT:
				if (wbus16_flg) {
					/*
					 * If this is a 16 bit device
					 * then set for 16 bit message
					 */
					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"28");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"29");
					}
					else {
						strcpy(msg,"28");
					}
				}
				else {
					/*
					 * This is not
					 * a 16 bit device
					 */
					if (scsd_vpd.interface_id == SCSD_SE) {
						strcpy(msg,"24");
					}
					else if (scsd_vpd.interface_id == SCSD_DIFF) {
						strcpy(msg,"25");
					}
					else {
						strcpy(msg,"24");
					}
				}
								
				
				break;
				
			    case SCSD_OEM_RWOPT:
			    default:
				strcpy(msg,"20");
				
			}

			


			/*
			 * Generate device PVID in CuAt if necessary.

			 */
			if ((rc = create_pvid_in_db(lname)) != 0)
				return (rc);
		} /* end SCSD rwoptical */
		else {
			/* Invalid device class */
			return (E_WRONGDEVICE);
		}
				
			
		/* 
		 * Common Code for SCSD CD-ROMs and SCSD R/W optical 
		 * drives.
		 */

		dds->extended_rw = 1;		
		

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
		if ((cuat = getattr(cusobj.name,"size_in_mb",0,&bc))
		    == (struct CuAt *)NULL ) {
			DEBUG_0("ERROR: getattr() failed\n")
				return E_NOATTR;
		}
		
		/* Only rewrite if actually changed */
		if (strcmp(cuat->value,tmpstr)) {
			strcpy(cuat->value, tmpstr);
			putattr(cuat);
		}
		
		if (scsd_vpd.rw_timeout != 0) {
			dds->rw_timeout=scsd_vpd.rw_timeout;
		}
		else {
			dds->rw_timeout=rw_timeout;
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

		dds->recovery_limit=scsd_vpd.recovery_limit;
		
		if (scsd_vpd.scsi_support_flags & SCSD_CD_PLAY_AUDIO_FLG ) {
			/*
			 * This device supports SCSI-2 Play Audio
			 * commands.
			 */
			dds->play_audio = TRUE;
		}
		else {
			dds->play_audio = FALSE;
		}

		if (scsd_vpd.scsi_support_flags & SCSD_CD_READ_DISC_FLG ) {
			/*
			 * This device supports the vendor unique READ
			 * DISC INFO command that we use to enable
			 * multi-session CD's.
			 */
			dds->multi_session = TRUE;
		}
		else {
			dds->multi_session = FALSE;
		}
		
		if (scsd_vpd.cd_mode_byte & SCSD_CD_M2F1 )
			dds->valid_cd_modes |= DK_M2F1_VALID;	
		
		if (scsd_vpd.cd_mode_byte & SCSD_CD_M2F2)
			dds->valid_cd_modes |= DK_M2F2_VALID;
		
		if (scsd_vpd.cd_mode_byte & SCSD_CD_DA )
			dds->valid_cd_modes |= DK_CDDA_VALID;
		
		dds->cd_mode2_form1_code=scsd_vpd.cd_m2f1;
		dds->cd_mode2_form2_code=scsd_vpd.cd_m2f2;
		dds->cd_da_code=scsd_vpd.cd_da;
		

		if ((cuat = getattr(cusobj.name,"led",0,&bc)) 
		    == (struct CuAt *)NULL) {
			DEBUG_0("ERROR: getattr() failed\n")
			return E_NOATTR;
		}

		sprintf(tmpstr,"0x%02x%02x%02x",scsd_vpd.led_no[0],
			scsd_vpd.led_no[1],scsd_vpd.led_no[2]);
		
		if (!strcmp(tmpstr,"0x734000")) {
			/*
			 * The value of 0x734000 is
			 * the only valid value under the old
			 * SCSD implementation. In that implementation
			 * one drive type had an led of 734, which
			 * was represented in left justified binary
			 * nibbles. 
			 */
			strncpy(led,tmpstr,5);
			led[5] = '\0';
		} else {
			/*
			 * Since this value is not 0x734000, then
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
		 * Update displayable message for
		 * this device.
		 */
		if ((cuat = getattr(cusobj.name,"message_no",0,&bc))
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



		if ((cuat = getattr(cusobj.name,"diag_scsd",0,&bc)) 
		    == (struct CuAt *)NULL ) {
			DEBUG_0("ERROR: getattr() failed\n")
				return E_NOATTR;
		}
		/* Only rewrite if actually changed */
		if (strcmp(cuat->value,diagstr)) {
			strcpy(cuat->value, diagstr );
			putattr(cuat);
		}


		pg=0xC8;
		rc = get_inquiry_data(&cusobj,pg,&scsd_mode_data);
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
		rc = get_inquiry_data(&cusobj,pg,&scsd_deflt_data);
		if (rc == 0) {
			dds->mode_default_length = 0;
			dds->mode_default_data[0] = '\0';
		}else {
			dds->mode_default_length = scsd_deflt_data.page_length;
			bcopy(scsd_deflt_data.mode_data,
			      dds->mode_default_data,
			      scsd_deflt_data.page_length);
		}



		
	} /* end if scsd */


	else {

		/*
		 * This is for non-SCSD CD-ROMs and R/W optical
		 * drives.
		 */
		
		if (!strncmp(cusobj.PdDvLn_Lvalue,"cdrom",5)) {
			
			/* CD-ROM drive */
			
			dds->dev_type = DK_CDROM;
			dds->safe_relocate = 0;
			prevent_dflt = FALSE;
			reserve_dflt = FALSE;
			rw_timeout = DK_TIMEOUT;
			dds->reassign_timeout = 1;   /* not used for CDROM.  This
						      * value is set to 1, since
						      * is possible for some process
						      * to call the device driver
						      * strategy routine asking for
						      * a reassign.  We do not
						      * want to hang on it forever
						      */
			dds->fmt_timeout = 0;        /* not used for CDROM */
			
			
			if (!strcmp(cusobj.PdDvLn_Lvalue,"cdrom/scsi/cdrom1")) {
				/* 
				 * The early Toshiba CDROM's did not support
				 * the SCSI-2 load eject command so we need to
				 * let the device driver know this, so it can still
				 * eject media via the Toshiba unique Eject Disc Caddy
				 * command.
				 */
				dds->load_eject_alt = TRUE;
			}
			
			
		}
		else if (!strncmp(cusobj.PdDvLn_Lvalue,"rwoptical",9)) {
			
			/* R/W optical drive */
			
			dds->dev_type = DK_RWOPTICAL;
			if ((rc=getatt(&dds->safe_relocate,'c',c_att_oc, p_att_oc, 
				       lname,cusobj.PdDvLn_Lvalue,
				       "safe_relocate",NULL)) == E_NOATTR) {
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
			if ((rc=getatt(&dds->reassign_timeout,'h',c_att_oc, p_att_oc, 
				       lname,cusobj.PdDvLn_Lvalue,
				       "reassign_to",NULL)) == E_NOATTR) {
				/*
				 * Do not fail if reassign timeout is not in ODM, 
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
			if ((rc=getatt(&dds->fmt_timeout,'h',c_att_oc, p_att_oc, 
				       lname,cusobj.PdDvLn_Lvalue,
				       "fmt_timeout",NULL)) == E_NOATTR) {
				/*
				 * Do not fail if format timeout is not in ODM, just 
				 * assume default.
				 */
				dds->fmt_timeout = DK_FMT_TIMEOUT;  
			}		
			else if (rc != 0) {
				/* 
				 * There may be a problem with the ODM data base
				 */
				return (rc);
			}
			prevent_dflt = TRUE;
			reserve_dflt = TRUE;
			rw_timeout = DK_RWOPT_TIME;
			
			/*
			 * Generate device PVID in CuAt if necessary.
			 */
			if ((rc = create_pvid_in_db(lname)) != 0)
				return (rc);
		}
		else {
			/* Invalid device class */
			return (E_WRONGDEVICE);
		}
	

		
		/*
		 * Common code for non-SCSD CD-ROMs and R/W optical drives.
		 */

		/* Read the attributes from the customized & predefined classes */

		if ((rc=getatt(&dds->start_timeout,'h',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "start_timeout",NULL)) == E_NOATTR) {
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
		if ((rc=getatt(&dds->rw_timeout,'h',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "rw_timeout",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if read/write time out attribute is not in ODM, 
			 * just assume default.
			 */
			dds->rw_timeout = rw_timeout;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(&dds->reset_delay,'c',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "reset_delay",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if reset_delay is not in ODM, just assume
			 * default (i.e. no reset delay is needed)
			 */
			dds->reset_delay = 0;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		if ((rc=getatt(&dds->recovery_limit,'i', c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "recovery_limit",NULL)) == E_NOATTR) {
		/*
		 * Do not fail if recovery_limit is not in ODM, just assume
		 * default.
		 */
			dds->recovery_limit = 0; 
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		

		if( ( dds->mode_data_length = - ( rc =
						 getatt(dds->mode_data, 'b', c_att_oc, p_att_oc, lname,
							cusobj.PdDvLn_Lvalue, "mode_data", NULL))) < 0 )
			CLOSE_RET(rc)
				
				if( ( dds->mode_default_length = - ( rc =
								    getatt(dds->mode_default_data, 'b', c_att_oc, p_att_oc, lname,
									   cusobj.PdDvLn_Lvalue, "mode_default", NULL))) < 0 )
					/*
					 * If mode default attribute doesn't exist, don't fail,
					 * just ignore
					 */
					dds->mode_default_length = 0;
		
		
		if ((rc=getatt(&max_dev_data_rate,'i',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "max_data_rate",NULL)) == E_NOATTR) {
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
		
		if ((rc=getatt(&dev_data_rate,'i',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "media_rate",NULL)) == E_NOATTR) {
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
			
			if ((cuat=(struct CuAt *)getattr(cusobj.parent,
							 "max_data_rate",0,&bc)) 
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
			 * buffer ratio and cause the SCSI device driver
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
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "audio_supported",NULL)) == E_NOATTR) {
			/*
			 * don't fail if this attribute is not there, just set
			 * it to FALSE.
			 */
			dds->play_audio = FALSE;
		}
		else if (rc != 0) {
			/* 
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			if ((!strcmp(data_value,"yes"))) 
				dds->play_audio = TRUE;
			else 
				dds->play_audio = FALSE;
		}
		
		if ((rc=getatt(&dds->cd_mode2_form1_code,'c',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "m2f1_code",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->cd_mode2_form1_code = 0;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			dds->valid_cd_modes |= DK_M2F1_VALID;
		}
		
		if ((rc=getatt(&dds->cd_mode2_form2_code,'c',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "m2f2_code",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->cd_mode2_form2_code = 0;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			dds->valid_cd_modes |= DK_M2F2_VALID;
		}
		
		if ((rc=getatt(&dds->cd_da_code,'c',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "cdda_code",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->cd_da_code = 0;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		else {
			dds->valid_cd_modes |= DK_CDDA_VALID;
		}
		
		if ((rc=getatt(&dds->multi_session,'c',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "multi_session",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if this attribute is not in ODM,
			 * just assume default.
			 */
			dds->multi_session = 0;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "clr_q",NULL)) == E_NOATTR) {
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
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "q_err",NULL)) == E_NOATTR) {
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
		
		
		
		if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc, 
			       lname,cusobj.PdDvLn_Lvalue,
			       "q_type",NULL)) == E_NOATTR) {
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
		
		if ((rc=getatt(&dds->extended_rw, 'c',c_att_oc, p_att_oc,
			       lname,cusobj.PdDvLn_Lvalue,
			       "extended_rw",NULL)) == E_NOATTR) {
			/*
			 * Do not fail if extended_rw is not in ODM, just assume
			 * default (i.e. extended read/write not supported)
			 */
			dds->extended_rw = 1;
		}
		else if (rc != 0) {
			/*
			 * There may be a problem with the ODM data base
			 */
			return (rc);
		}



		if (!strcmp(cusobj.PdDvLn_Lvalue,"cdrom/scsi/enhcdrom4")) {

			/*
			 * If this is the 1/4 high Teac CD-ROM
			 * drive for lap tops, then we have to work
			 * around a hardware problem. The drive
			 * can't resume from suspend. As a result
			 * we need to tell the device driver to
			 * issue a BDR (Bus Device Reset) to this
			 * device when it resumes it. This is done
			 * via the pm_susp_bdr flag.
			 */
			dds->pm_susp_bdr = 1;
		}


		
	}/* end scsd else */

	/*
	 * The following is code common to both SCSD and non-SCSD
	 * devices.
	 */
	
        if ((rc=getatt(&dds->block_size,'i',c_att_oc, p_att_oc,
                       lname,cusobj.PdDvLn_Lvalue,
                       "block_size",NULL)) == E_NOATTR) {
		
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
	
	
        if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc,
                       lname,cusobj.PdDvLn_Lvalue,
                       "reserve_lock",NULL)) == E_NOATTR) {
                /*
                 * don't fail if this attribute is not there, just set
                 * it to TRUE.
                 */
                dds->reserve_lock = reserve_dflt;
		
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


        if ((rc=getatt(data_value,'s',c_att_oc, p_att_oc,
                       lname,cusobj.PdDvLn_Lvalue,
                       "prevent_eject",NULL)) == E_NOATTR) {
                /*
                 * don't fail if this attribute is not there, just set
                 * it to FALSE.
                 */
                dds->prevent_eject = prevent_dflt;
        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }
        else {
                if ((!strcmp(data_value,"yes")))
                        dds->prevent_eject = TRUE;
                else
                        dds->prevent_eject = FALSE;
        }

       if ((rc=getatt(&dds->max_coalesce,'i',c_att_oc, p_att_oc,
                       lname,cusobj.PdDvLn_Lvalue,
                       "max_coalesce",NULL)) == E_NOATTR) {
                /*
                 * Do not fail if max_coalesce is not in ODM, just assume
                 * default.
                 */
                dds->max_coalesce = 0x10000;
        }
        else if (rc != 0) {
                /*
                 * There may be a problem with the ODM data base
                 */
                return (rc);
        }


	/* device idle time */
	if ((rc=getatt(&dds->pm_dev_itime, 'i',c_att_oc, p_att_oc,
			lname,cusobj.PdDvLn_Lvalue,
			"pm_dev_itime",NULL)) == E_NOATTR) {
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
	if ((rc=getatt(&dds->pm_dev_stime, 'i',c_att_oc, p_att_oc,
			lname,cusobj.PdDvLn_Lvalue,
			"pm_dev_stime",NULL)) == E_NOATTR) {
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
	if ((rc=getatt(&dds->pm_dev_att, 'i',c_att_oc, p_att_oc,
			lname,cusobj.PdDvLn_Lvalue,
			"pm_dev_att",NULL)) == E_NOATTR) {
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

	if ((rc=getatt(&dds->pm_device_id, 'i',c_att_oc, p_att_oc,
			lname,cusobj.PdDvLn_Lvalue,
			"pm_devid",NULL)) == E_NOATTR) {
		/*
		 * don't fail if this attribute is not there, just
		 * set dds->pm_devid = PMDEV_UNKNOWN_SCSI.
		 */
		dds->pm_device_id = PMDEV_UNKNOWN_SCSI;
	} else if (rc != 0) {
		/*
		 * There may be a problem with the ODM data base
		 */
		return (rc);
	}





	/* Get sid and lun and save in DDS */
	if ( get_sid_lun( cusobj.connwhere, &sid, &lun) )
		return ( E_INVCONNECT );

	dds->scsi_id = sid;
	dds->lun_id = lun;

	dds->pm_device_id |= (sid << 6) | lun;

	/*
	 * Detemine if this device is a SCSI-2 device and if it supports 
	 * command tag queuing.
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
		 * If this is a SCSI-2 Device and it supports command tag 
		 * queuing and the ODM database says the system should try to 
		 * queue to it, then tell the SCSI device driver
		 * that the device supports command tag queuing.
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

	strncpy( dds->resource_name, lname, sizeof(dds->resource_name) );


	/* Read CuDv for parent (SCSI-Adapter) */

	sprintf( search_str, "name = '%s'", cusobj.parent );

	DEBUG_1( "performing odm_get_obj(CuDv,%s)\n", search_str )

	if( ( rc=(int)odm_get_obj(CuDv_CLASS,search_str,&cusobj,TRUE)) == 0 )
	{
		DEBUG_1( "build_dds() Record not found in CuDv: %s\n",
			search_str)
		CLOSE_RET( E_NOCuDvPARENT)
	} else if ( rc == -1 )
	{
		DEBUG_0( "build_dds() for SCSI-CD-ROM: Error Reading CuDv\n")
		CLOSE_RET( E_ODMGET )
	}

	DEBUG_0( "odm_get_obj() succeeded\n" )

	/* Generate Parent's devno */

	major_no = genmajor( cusobj.ddins );
	minor_no = *getminor( major_no, &how_many, cusobj.name );

	dds->adapter_devno = makedev( major_no, minor_no );

	DEBUG_1( "dds->adapter_devno = %ld\n", dds->adapter_devno)

	*dds_data_ptr = (char *)dds;		/* Store address of struct */
	*dds_len = sizeof(struct disk_ddi);	/* Store size of structure */

#ifdef CFGDEBUG
	dump_dds( *dds_data_ptr, *dds_len );
#endif 

	DEBUG_0( "build_dds() returning 0\n" )

	CLOSE_RET(0)
}




/*
 * NAME: create_pvid_in_db
 *
 * FUNCTION: Generate unique PVID and place in CuAt.
 *
 * EXECUTION ENVIRONMENT:
 *    
 *
 * RETURNS:	0        - Successful completion
 *		E_ODMGET - odmget error
 */

int create_pvid_in_db(lname)
char 	*lname;
{
	struct unique_id  uid;              /* unique PVID 		      */
	char              pvidstr[33];
        char              sstring[256];    /* search criteria pointer 	      */
        struct CuAt       attrobj;         /* customized attribute object     */
	int		  rc=0;


	/*
	 * Remove pv attribute in CuAt for this device if it exists.
	 */
	sprintf(sstring,"name = '%s' AND attribute = 'pv'",
		lname);
	odm_rm_obj(CuAt_CLASS,sstring);

	/*
	 * See if device has a PVID attribute in the CuAt
	 */

        sprintf(sstring,"name = '%s' AND attribute = 'pvid'", lname);
        rc = (int)odm_get_first(CuAt_CLASS,sstring,&attrobj);
        if (rc == -1) {
                DEBUG_0("create_pvid_in_db: odmget err\n")
                return(E_ODMGET);
        } else if (rc == 0) {
                /* 
		 * It does not have a PVID so generate one 
		 */
		mkuuid(&uid);
		strcpy(pvidstr,pvidtoa(&uid));
		putpvidattr(lname,pvidstr);
		DEBUG_0("create_pvid_in_db: Adding PVID to CuAt")
	}

	return (0);

}

