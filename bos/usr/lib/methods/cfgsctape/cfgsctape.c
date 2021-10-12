static char sccsid[] = "@(#)40        1.11.2.9  src/bos/usr/lib/methods/cfgsctape/cfgsctape.c, cfgmethods, bos41J, 9513A_all 3/24/95 16:13:07";
/*
 * COMPONENT_NAME: (CFGMETHODS) SCSI Tape Config Method
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
#include        <stdio.h>
#include        <sys/errno.h>
#include        <sys/types.h>
#include        <sys/device.h>
#include        <sys/sysconfig.h>
#include        <sys/cfgdb.h>
#include        <sys/stat.h>
#include        <malloc.h>
#include        <fcntl.h>
#include        <sys/scsi.h>
#include        <sys/sysmacros.h>
#include        <sys/watchdog.h>
#include        <sys/tapedd.h> 
#include        <sys/pmdev.h>
#include        <sys/cfgodm.h>
#include        <cf.h>
#include        "cfgdebug.h"
#include        "cfghscsi.h"
#include 	<string.h>

/*
 * NAME: generate_minor
 *
 * FUNCTION: Generates a group of minor numbers for the scsi tape being
 *      configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables
 *      are assumed.
 *
 * RETURNS:
 *      0 For succeed, errno for fail.
 *      If successful, the first of eight consecutive minor numbers is
 *      stored via the pointer passed in as the last parameter
 *
 */

long generate_minor( logical_name, major_no, minor_dest )
char *logical_name;     /* Name of device                               */
long major_no;          /* Major no. already generated                  */
long *minor_dest;       /* Adrress to store minor number to             */
{
        long    *genret;
        long    *genminor();

        /* Request a list of eight minor numbers. These numbers must be */
        /* contiguous, starting on a multiple-of-eight boundary         */

        DEBUG_0( "generate_minor()\n" )

        if(( genret = genminor(logical_name, major_no, -1, 8, 1, 8 )) == NULL)
                return E_MINORNO;

        *minor_dest = *genret;
        return 0;
}

/*
 * NAME: make_special_files
 *
 * FUNCTION: Generates eight special files for the scsi tape being configured
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *              assumed.
 * 2. The files generated are /dev/<logical_name>,
 *              and /dev/<logical_name>.1 .. /dev/<logical_name>.7
 * 3. The minor number passed in is simply the first in a sequence of eight
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 *
 */

#define MSP_MAXLEN      14              /* Maximum length of a filename */

int make_special_files( logical_name, devno )
char *logical_name;                     /* Prefix, e.g. rmt0            */
dev_t   devno;                          /* Major/Minor no. of device    */
{
        int     suffix;                 /* counter from 0 to 7          */
        int     rc;                     /* Return codes                 */
        char basename[MSP_MAXLEN+1];    /* basename of special file     */

        DEBUG_0( "make_special_files()\n" )

/* Check that there is enough room in basename for <logical_name>.digit */
        if( strlen(logical_name) > (MSP_MAXLEN-2))
                return E_MAKENAME;

        /* Create the character special files */
        /* File modes are rw-rw-rw- */

        for( suffix=0; suffix<8; suffix++ )
        {
                if( suffix == 0 )
                        sprintf( basename, "%s", logical_name );
                else
                        sprintf( basename, "%s.%d", logical_name, suffix );

                if( rc = mk_sp_file( devno + suffix, basename,
                        ( S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
                                S_IWGRP | S_IROTH | S_IWOTH )) )
                        return rc;
        }
        return 0;
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
 *      assumed.
 * 2. The SCSI Tapes only use micro-code for diagnostics (if at all),
 *      therefore the diagnostic programs will download their own micro-code.
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 *
 */

int download_microcode( lname )
char *lname;
{

        DEBUG_0( "Downloading Microcode\n" )

        return 0;
}

/*
 * NAME: query_vpd
 *
 * FUNCTION: Obtains Vital Product Data From SCSI-Tape Device, and places it
 *                              in the customized database
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. This code is designed to be loadable, therefore no global variables are
 *      assumed.
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 *
 */

#define INQSIZE 255

int query_vpd( cusobj, kmid, devno, vpd_dest )
struct  CuDv    *cusobj;        /* Pointer to customized object rec     */
mid_t   kmid;                   /* Kernel module I.D. for Dev Driver    */
dev_t   devno;                  /* Concatenated Major & Minor No.s      */
char    *vpd_dest;              /* destination for vpd                  */
{
	struct  CuAt   sys_cuattr;
        uchar   inq_data[INQSIZE];      /* Storage area for inquiry data */
        char    fmt_str[] =
                "MF0808C,TM1010C,Z12004C,SN240CC,LI2c04X,PN3008C,FN720cC,EC7e0aC,Z00008X,Z36204C" ;
        int     vpd_len ;
        int     rc;       
        uchar   sid ;                   /* SCSI ID of device.           */
        uchar   lun ;                   /* LUN of device (usually zero) */
        int     len ;                   /* length of data returned.     */


        /* update the location of the device */
	rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
                                 &sys_cuattr);
        if (rc == -1) {
                DEBUG_0("ODM error getting rds_facility attribute\n")
                return(E_ODMGET);
        } else if (rc != 0 && sys_cuattr.value[0]=='y') {
                rc = rds_update_location(cusobj);
                if (rc != 0)
                        DEBUG_0("Can not update the location code\n");
        }

        DEBUG_0( "query_vpd()\n")


        if (get_inquiry_data(cusobj, (int)NO_PAGE, inq_data) == 0)
        {
            DEBUG_0("query_vpd: error getting inquiry data\n") ;
            return(E_DEVACCESS) ;
        }

        len = strtoul(cusobj->connwhere, NULL, 16) ;
        sid = (len >> 4) & 0x0f ;
        lun = len & 0x0f ;
        
        sid = strtoul(cusobj->connwhere,NULL,10);
        lun = strtoul(strchr(cusobj->connwhere,',')+1,NULL,10);

        /* make sure that the device we're looking at is the same type
           as the database says it is */
        if ((rc=chktype(inq_data,cusobj->PdDvLn_Lvalue,SCSI_TAPE,
			cusobj->parent,sid,lun))!=0) {
                DEBUG_1("query_vpd: chktype failed, rc=%d\n",rc)
                return(rc);
        }

        /* format the VPD for the database */

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
}




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
 *      assumed.
 * 2. The SCSI Tapes do not have child devices, so this routine always succeeds
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 *
 */

int define_children( lname, phase )
char    *lname;
int             phase;
{
        DEBUG_0( "define_children()\n" )
        return 0;
}

/*
 * NAME: build_dds
 *
 * FUNCTION: Device dependant dds build for SCSI-Tape
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded routine.
 *
 * RETURNS:     0 if succeed, or errno if fail
 */

/* Constants for use when calling getatt(): */
#define         CUSTOM  0       /* Customized attribute current value */

#ifdef CLOSE_RET
#undef CLOSE_RET
#endif
#define CLOSE_RET(retval) {             \
        odm_close_class(PdAt_CLASS);    \
        odm_close_class(CuAt_CLASS);    \
        return retval;                  \
        }

int build_dds( lname, dds_data_ptr, dds_len )
char    *lname;                 /* Logical name */
char    **dds_data_ptr;         /* Address to store pointer to dds */
int     *dds_len;               /* Address to store length of dds */
{
        struct  tape_ddi_df     *dds;   /* dds for SCSI-Tape */
        char    search_str[128];        /* Search criterion */
        struct  Class   *c_att_oc;      /* Customized Attribute Object Class */
        struct  Class   *p_att_oc;      /* Predefined Attribute Object Class */
        struct  CuDv    cusdesc;        /* Customized Device Object */
	struct  CuAt    sys_cuattr;
        int     rc;                     /* Return code */
        char    tmp_str[20];
        long    major_no;               /* Major number of adapter */
        long    minor_no;               /* Minor number of adapter */
        int     how_many;
        int     density;
        int     timeval;		/* Used for PM driver */
        int     attr;			/* Used for PM driver */
        long    *getminor();
        long    genmajor();

        DEBUG_0( "build_dds()\n" )

        if(( dds = (struct tape_ddi_df *)malloc(sizeof(struct tape_ddi_df)))
                == NULL )
        {
                DEBUG_0( "malloc for struct tape_ddi_df{} failed\n")
                return E_MALLOC;
        }


        /* Open required Object Classes */

        if((int)( c_att_oc = odm_open_class( CuAt_CLASS )) == -1 )
        {
                DEBUG_0( "build_dds() for SCSI-Tape: Error opening CuAt\n")
                return E_ODMOPEN;
        }

        if((int)( p_att_oc = odm_open_class( PdAt_CLASS )) == -1 )
        {
                DEBUG_0( "build_dds() for SCSI-Tape: Error opening PdAt\n")
                return E_ODMOPEN;
        }

        /* Read CuDv for SCSI-Tape */

        sprintf( search_str, "name = '%s'", lname);

        DEBUG_1( "Performing odm_get_obj(CuDv,%s)\n", search_str )

        if(( rc = (int)odm_get_first(CuDv_CLASS,search_str,&cusdesc)) == 0 )
        {
                DEBUG_1( "build_dds(): Record not found in CuDv: %s\n",
                        search_str)
                CLOSE_RET( E_NOCuDv )
        }
        else if ( rc == -1 )
        {
                DEBUG_0( "build_dds(): for SCSI-Tape: Error Reading CuDv\n")
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
                rc = rds_power_on_device(&cusdesc);
                if (rc != 0)
                        DEBUG_0("Problem in rds_power_on_device\n");
        }

        /* Read the attributes from the customized attribute class */

        if(rc=getatt( tmp_str, 's', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "mode", NULL)) CLOSE_RET( rc )
        dds->mode = ( tmp_str[0] == 'y' ) ? 2 : 1;
        if(rc=getatt(&dds->dev_type, 'c', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "dev_type", NULL)) CLOSE_RET( rc )
        if(rc=getatt( tmp_str, 's', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "ret", NULL)) CLOSE_RET( rc )
        dds->retention_flag = ( tmp_str[0] == 'y' ) ? 1 : 0;
        if(rc=getatt( tmp_str, 's', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "extfm", NULL)) CLOSE_RET( rc )
        dds->extend_filemarks = ( tmp_str[0] == 'y' ) ? 0 : 0x80;
        if(rc=getatt(&dds->blocksize, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "block_size", NULL)) CLOSE_RET( rc )
        if(rc=getatt(&dds->min_read_error, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "min_read_error", NULL)) CLOSE_RET(rc)
        if(rc=getatt(&dds->min_write_error, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "min_write_error", NULL)) CLOSE_RET(rc)
        if(rc=getatt(&dds->read_ratio, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "read_ratio", NULL)) CLOSE_RET( rc )
        if(rc=getatt(&dds->write_ratio, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "write_ratio", NULL)) CLOSE_RET( rc )
        if(( dds->mode_data_length = -( rc =
                getatt(dds->mode_select_data, 'b', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "mode_data", NULL))) < 0 )
                CLOSE_RET( rc )
        if(rc=getatt( tmp_str, 's', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "res_support", NULL)) CLOSE_RET( rc )
        dds->res_sup = ( tmp_str[0] == 'y' ) ? TRUE : FALSE;
        if(rc=getatt(&dds->var_blocksize, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "var_block_size", NULL)) CLOSE_RET( rc )
        if(rc=getatt(&density, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "density_set_1", NULL)) CLOSE_RET( rc )
        dds->density_set1 = (uchar) density;
        if(rc=getatt(&density, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "density_set_2", NULL)) CLOSE_RET( rc )
        dds->density_set2 = (uchar) density;
        dds->ecc_flag = 0;   /* reserved field */
        DEBUG_2("res_sup=%d, var_blocksize=%d\n",
                 dds->res_sup, dds->var_blocksize)
        DEBUG_2("high_density=%x, low_density=%x\n",
                 dds->density_set1,dds->density_set2)
        /* For 8mm5g get compression attribute */
        rc=getatt( tmp_str, 's', c_att_oc, p_att_oc, lname,
                    cusdesc.PdDvLn_Lvalue, "compress", NULL);
        if (rc == 0){
            /* if the compress attribute is found then check it */
            dds->compression_flag = ( tmp_str[0] == 'y' ) ? TRUE : FALSE;
        }
        else{
            /* the compress attribute was not found so set to 0 */
            /* which means compression is turned off            */
            dds->compression_flag = FALSE;
        }
	/* For 3490e get autoload attribute */
	rc=getatt( tmp_str, 's', c_att_oc, p_att_oc, lname,
	    cusdesc.PdDvLn_Lvalue, "autoload", NULL);
	if (rc == 0){
	    /* if the autoload attribute is found then check it */
	    dds->autoloader_flag = ( tmp_str[0] == 'y' ) ? TRUE : FALSE;
	}
	else{
	    /* the autoload attribute was not found so set to 0 */
	    /* which means the autoloader is turned off         */
	    dds->autoloader_flag = FALSE;
	}
	/* For ost get delay attribute, for other drives it won't */
	/* be present, so set to 0.				  	*/
	if(getatt(&dds->delay, 'i', c_att_oc, p_att_oc, lname,
	    cusdesc.PdDvLn_Lvalue, "delay", NULL)) dds->delay=0;
	/* For ost get read/write timeout, for other drives it won't */
	/* be present, so set to 0.				  	*/
	if(getatt(&dds->readwrite, 'i', c_att_oc, p_att_oc, lname,
	    cusdesc.PdDvLn_Lvalue, "rwtimeout", NULL)) dds->readwrite=0;
	/* Add the attributes for power management support, if present. */
	/* Get the device idle time. */
        if((rc=getatt(&timeval, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "pm_dev_itime", NULL))  == E_NOATTR) {
		/* Time will be otherwise set to zero */
		dds->tape_pm_ptr.pmh.device_idle_time = 0;
	}
	else if (rc) {
                CLOSE_RET( rc )
	}
	else {
		dds->tape_pm_ptr.pmh.device_idle_time = timeval;
	}
	/* Get the device standby time. */
        if((rc=getatt(&timeval, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "pm_dev_stime", NULL))  == E_NOATTR) {
		/* Time will be otherwise set to zero */
		dds->tape_pm_ptr.pmh.device_standby_time = 0;
	}
	else if (rc) {
                CLOSE_RET( rc )
	}
	else {
		dds->tape_pm_ptr.pmh.device_standby_time = timeval;
	}
	/* Get the device attribute type */
	dds->tape_pm_ptr.pmh.attribute = 0;
	/* Get the device ID */
        if((rc=getatt(&attr, 'i', c_att_oc, p_att_oc, lname,
                cusdesc.PdDvLn_Lvalue, "pm_device_id", NULL))  == E_NOATTR) {
		/* The device ID is currently set to unknown scsi device */
		/* if there is no value found for it in the database */
		dds->tape_pm_ptr.pm_device_id = PMDEV_UNKNOWN_SCSI;
	}
	else if (rc) {
                CLOSE_RET( rc )
	}
	else {
		dds->tape_pm_ptr.pm_device_id = attr;
	}

        strncpy( dds->resource_name, lname, sizeof(dds->resource_name));

        if( get_sid_lun( cusdesc.connwhere, &dds->scsi_id, &dds->lun_id ))
        {
                CLOSE_RET( E_INVCONNECT )
        }

        /* Read CuDv for parent (SCSI-Adapter) */

        sprintf( search_str, "name = '%s'", cusdesc.parent );

        DEBUG_1( "performing odm_get_obj(CuDv,%s)\n", search_str )

        if(( rc=(int)odm_get_first(CuDv_CLASS,search_str,&cusdesc)) == 0 )
        {
                DEBUG_1( "build_dds(): Record not found in CuDv: %s\n",
                        search_str )
                CLOSE_RET( E_NOCuDvPARENT )
        }
        else if ( rc == -1 )
        {
                DEBUG_0( "build_dds() for SCSI-Tape: Error Reading CuDv\n")
                CLOSE_RET( E_ODMGET )
        }

        DEBUG_0( "odm_get_obj() succeeded\n" )

        /* Generate Parent's devno */

        major_no = genmajor( cusdesc.ddins );
        minor_no = *getminor( major_no, &how_many, cusdesc.name  );

        dds->adapter_devno = makedev( major_no, minor_no );

        DEBUG_1( "dds->adapter_devno = %ld\n", dds->adapter_devno)

        *dds_data_ptr = (char *)dds;            /* Store address of struct */
        *dds_len = sizeof( struct tape_ddi_df );/* Store size of structure */

#ifdef CFGDEBUG
        dump_dds( *dds_data_ptr, *dds_len );
#endif

        DEBUG_0( "build_dds() returning 0\n" )

        CLOSE_RET( 0 )
}



