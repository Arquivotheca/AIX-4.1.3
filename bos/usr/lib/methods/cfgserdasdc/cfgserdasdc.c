static char sccsid[] = "@(#)19	1.15  src/bos/usr/lib/methods/cfgserdasdc/cfgserdasdc.c, cfgmethods, bos411, 9428A410j 2/16/94 14:54:27";
/*
 * COMPONENT_NAME: (CFGMETH) configuration method for serial dasd controller
 *
 * FUNCTIONS : 	build_dds, generate_minor, make_special_files,
 *		download_microcode, query_vpd, define_children.
 * 
 * ORIGINS : 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * Unpublished Work
 * All Rights Reserved
 *
 * RESTRICTED RIGHTS LEGEND
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <cf.h>
#include <sys/bootrecord.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/sd.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/errno.h>
#include <sys/cfgdb.h>
#include <sys/stat.h>
#include "cfgdebug.h"
#include "cfgserdasd.h"

int build_dds( char *, char **, long * );

#define NULLPVID "00000000000000000000000000000000"

int sd_phase2_dwld_ucode   = TRUE;     /* true if phase 2 microcode download */
				       /* needed in define_children routine  */
int num_switches = 0;                  /* Number of switch cards attached to */
                                       /* the controller                     */
#define	INQSIZE	255
static	uchar	inq_data[INQSIZE];	/* Storage area for VPD	*/


/*
 * NAME:	generate_minor
 *
 * FUNCTION:	This function generates minor device number for the 
 *	     	Serial DASD controller
 * 
 * ENVIRONMENT: This function operates as a device dependent subroutine 
 *		called by the generic configure method for all devices.
 *		It makes use of generic genminor() function.
 *
 * NOTES : 	Minor numbers for controllers are between 0x2000, and 0x3fff
 *
 * RETURNS:	0, or E_MINORNO
 *
 */

long generate_minor(char	*lname,	    /* Logical name of device        */
		    long	major_no,   /* Major number of device        */
		    long	*minor_dest)/* Address to store minor number */
					    /* generated                     */
{
	long	*minor_list;		/* address getminor stored list    */
	long	maximum_so_far = 0x1fff;/* maximum minor used in range     */
					/* 0x2000 is minimum minor number  */
					/* and 0x3fff is max minor number  */
					/* possible */
	int	how_many;		/* Number of minor numbers in list */

	DEBUG_0("cfgserdasdc: generate_minor\n")

	minor_list = getminor( major_no, &how_many, (char *)NULL );

	/* Find the highest minor number already assigned within the range */

	while( how_many-- ) {
		if(( *minor_list > maximum_so_far ) && ( *minor_list < 0x4000 ))
			maximum_so_far = *minor_list;
		minor_list++;
	}
	
	minor_list = genminor(lname,major_no,maximum_so_far+1,1,1,1);

	if(minor_list == (long *)NULL)
		return E_MINORNO;

	DEBUG_2("genminor(..%ld..)=%d\n", major_no, *minor_list )
	*minor_dest = *minor_list;
	return 0;
}
/*
 * NAME:	make_special_files
 * 
 * FUNCTION:	Creates serial DASD controller special file.
 * 
 * ENVIRONMENT:	This function operates as a device dependent subroutine
 *		called by the generic configure method for all devices.
 *		The routine mk_sp_file() from cfgtools.c performs most of the
 *		work.
 *
 * RETURNS:	Returns 0 if success else -1
 *
 */

int make_special_files(char  *lname,  /* Logical name of controller          */
		       dev_t	devno)/* Major & minor number for controller */
{
	/* Note: the permissions are  "crw-rw-rw-"  */

	DEBUG_0("cfgserdasdc: make_special_files\n")

	return(mk_sp_file(devno,lname,
		(S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)));
}
/*
 * NAME:	build_dds
 * 
 * FUNCTION:	Creates a dds structure passed to the driver via the config
 *		entry point
 * 
 * ENVIRONMENT:	This function operates as a device dependent subroutine
 *		called by the generic configure method for all devices.
 *
 * INPUTS:	lname, dds_addr_pointer (address to store pointer to dds),
 *		dds_len (address to store length of dds )
 *
 * RETURNS:	Returns 0 if success else Error number ( from cf.h )
 *
 */

int build_dds(char  *lname,	  /* Logical name for controller */
	      char **dds_addr_ptr,/*Address to store address of dds generated*/
	      long *dds_len)      /* Address to store length of dds generated*/
{
int			rc;		/* Return code                      */
int			how_many;	/* Used by getminor()               */
long			major_no,	/* Major number for controller      */
			minor_no;	/* Minor number for controller      */
struct Class		*cusatt,	/* CuAt class                       */
			*preatt;	/* PdAt class                       */
struct CuDv		cusobj,		/* CuDv object for controller       */
			parobj;		/* CuDv object for parent           */
struct CuAt             *CuAt;	        /* customized attribute          */
struct PdAt             PdAt;		/* prefined attribute		    */
struct PdDv             PdDv;           /* predefined device                */
struct sd_ctrl_dds	*dds;		/* Address of dds structure         */
char			sstr[SD_MAX_STRING_LEN];   /* Search string         */
uchar                   model_number=0;	/* controller's model number        */
uchar                   feature_code=0;	/* controller's feature code        */
int	                adapfd;		/* file descriptor for adapter	    */
int                     scsi_id, lun;	/* Extracted from connwhere	    */
int                     index;          /* Index into list of ctlr messages */
char                    *display_msg[SD_MAX_CTRL_MSG]; /* displayable       */
				        /* message  numbers for controllers */
        char	adapter_name[20];	/* Name of serial dasd adapter	    */

        DEBUG_0("cfgserdasdc: build_dds\n")

	if( NULL== ( dds = ( struct sd_ctrl_dds *)malloc(
		sizeof(struct sd_ctrl_dds))))
		return E_MALLOC;

	ODMOPEN( cusatt, CuAt_CLASS )
	ODMOPEN( preatt, PdAt_CLASS )
	sprintf( sstr, "name = '%s'", lname );
	GETOBJ( CuDv_CLASS, sstr, &cusobj, ODM_FIRST, E_NOCuDv )

	sprintf( sstr, "name = '%s'", cusobj.parent );
	GETOBJ( CuDv_CLASS, sstr, &parobj, ODM_FIRST, E_NOCuDvPARENT )

	dds->dev_type = (uchar)1; /* I.E. Controller */

	strncpy( dds->resource_name, lname, 16 );

	DEBUG_1("dds->resource_name=%s\n", dds->resource_name )

	/* Generate Parent's devno */

	major_no = genmajor( parobj.ddins );

	minor_no = *getminor( major_no, &how_many, parobj.name );	

	DEBUG_2("major_no = %ld, minor_no=%ld\n", major_no, minor_no )

	dds->adapter_devno = makedev( major_no, minor_no );

        DEBUG_1("cusobj.connwhere = '%s'\n", cusobj.connwhere )

	dds->target_id = (uchar) atoi( cusobj.connwhere );
	DEBUG_1("dds->target_id=%d\n", dds->target_id )

	/* 
	 * Get the "desc" attribute 
	 */
	 sprintf(sstr, "uniquetype = %s AND type = T",cusobj.PdDvLn_Lvalue);
         rc = (int)odm_get_first(PdAt_CLASS,sstr,&PdAt);
         if (rc==-1) {
	     /* ODM failure */
	     DEBUG_0("ODM failure getting description attribute \n")
	     return(E_ODMGET);
	 } else if (rc == 0) {
	     /* No description attribute present */
	     DEBUG_0("No description attribute\n")
	     return(E_NOATTR);
	 } else {
	     DEBUG_1("PdAt.values = %s \n",PdAt.values)
	     display_msg[0] = strtok(&PdAt.values,",");
	     /*
	      * Build up order dependent table of message numbers
	      */
	     for (index=1; index<SD_MAX_CTRL_MSG; index++) {
		 display_msg[index] = strtok((char *)NULL,",");
	     }
	     
	 }

	/*
	 * Find out what sort of controller it is
         */
	 scsi_id = atoi(cusobj.connwhere);
         lun = 0x10; /* i.e. controller */

	/* open adapter (i.e. parent) */

	sprintf(adapter_name,"/dev/%s",cusobj.parent);
	if((adapfd = open(adapter_name,O_RDWR)) < 0) {
		DEBUG_2("build_dds: failed to open %s, errno=%d\n",
			cusobj.parent,errno)
		return(E_DEVACCESS);
	}
	

	/* get inquiry data */
	if ((rc = scsi_inq(adapfd,inq_data,(scsi_id<<5)|lun))!=0) {
		DEBUG_3("build_dds: inquiry at sid %d lun %d on %s failed\n",
			scsi_id,lun,cusobj.parent)
#ifdef CFGDEBUG
		/* analyse the error for debugging purposes */
		switch (errno) {
		case EIO:
		case ETIMEDOUT:
			DEBUG_2("TIMEOUT / EIO on sid %d, lun %d\n",
				scsi_id,lun )
			break;
		case ENODEV:
			DEBUG_2("NO Device on sid %d, lun %d\n",
				scsi_id,lun)
			break;
		default:
			DEBUG_3("Error %d on sid %d, lun %d\n",
				errno,scsi_id,lun)
			break;
		}
#endif
		return(rc);
	}
   

#ifdef CFGDEBUG
	/* Dump out the inquiry data received */
        DEBUG_2("build_dds: data from sid %d lun %d:\n",scsi_id,lun);
	hexdump(inq_data,(long)(inq_data[4]+5));
#endif

	/* close the adapter */
	close(adapfd);
        /*
	 * Extract controller model number and 
	 * controller feature code from controller's
	 * inquiry data.
	 */
         strncpy(sstr,&inq_data[20],3);
         sstr[3] = '\0';

         model_number = atoi(sstr);

         dds->fence_enabled = FALSE;  /* assume no fencing */
         dds->conc_enabled = FALSE;   /* or concurrent ops */
         num_switches = 0;
         index = 0;                   /* old message           */
         if (model_number != 0x00) {      /* old controller        */
	     /*
	      * New controller can be one of four types
	      * depending upon its feature code and sid.
	      */
	     feature_code = (uchar)inq_data[23];
	     DEBUG_3("model number %x    feature code %x sid %x \n",model_number,feature_code,scsi_id)       			
		 
	     
	     switch (feature_code)
	     {
	       case 0:           
		 dds->fence_enabled = TRUE;
		 dds->conc_enabled = TRUE; 
		     index = 1;     /* two port controller   */
		 break;
	       case 1:             
		 dds->fence_enabled = TRUE;
		 dds->conc_enabled = TRUE; 
		 num_switches = 1;  /* one switch            */
		 index = 2;         /* four port ctlr        */
		 break;
	       case 2:           
		 dds->fence_enabled = TRUE;
		 dds->conc_enabled = TRUE; 
		 num_switches = 2;  /* two switches          */
		 index = 3;         /* eight port controller */
		 break;
	       case 4:
		 if (scsi_id>3)
		 {
		     index =4;      /* switched controller */
		     dds->fence_enabled = TRUE;
		     dds->conc_enabled = TRUE; 
		 } else 
		 {
		     return(E_WRONGDEVICE);
		 }
		 break;
	       default:
		 return(E_WRONGDEVICE);
	     }
	 }		
	       
         DEBUG_1("message index: %x \n", index)		
	 if( ( CuAt = getattr( lname, "desc", 0,&how_many )) 
	                             == (struct CuAt *)NULL ) 
	 {
	     DEBUG_0("ERROR: getattr() failed\n")
	     return(E_NOATTR);
	 }
         DEBUG_1("Current controller message number: %s\n",CuAt->value )

	 /* Only rewrite if actually changed */
	 if (strcmp(CuAt->value,display_msg[index])) {
	     DEBUG_1("updating CuAt with new message desc %s\n ",display_msg[index])
	     strcpy( CuAt->value, display_msg[index]);
	     if ( rc = putattr( CuAt ))
	     {
		 
		 DEBUG_1("ERROR: putattr() failed with rc %d\n",rc)
		     return(rc);
	     }
	 }

	ODMCLOSE( CuAt_CLASS )
	ODMCLOSE( PdAt_CLASS )

	*dds_addr_ptr = (char *)dds;
	*dds_len = sizeof( struct sd_ctrl_dds );

	DEBUG_0("dds complete...\n")

	return 0;
}
/*
 * NAME:	query_vpd
 *
 * FUNCTION:	Obtains Vital Product Data from the Controller,
 *		and places it in the customized database
 *
 * ENVIRONMENT: Dynamically loaded & run by generic config
 *
 * RETURNS:	0. if succeeded, else an error number from cf.h
 * 
 */


int query_vpd(struct CuDv *cusobjptr, /* Pointer to customized object rec   */
	      mid_t	kmid,	      /* Kernel module I.D. for Dev Driver  */
	      dev_t	devno,	      /* Concatenated Major & Minor No.s    */
	      char	*vpd_dest)    /* Area to place VPD	       	    */
{
        char	adapter_name[20];	/* Name of serial dasd adapter	    */
       	int     scsi_id, lun;		/* Extracted from connwhere	    */
 	int	how_many;	        /* Used by getminor()               */
	int	rc;			/* Return code			    */
	int	adapfd;			/* file descriptor for adapter	    */
	struct  CuAt *cusatobj; 	/* customized attribute             */
	char	tmp_str[100];

	DEBUG_0("cfgserdasdc: query_vpd()\n")

	scsi_id = atoi(cusobjptr->connwhere);
	lun = 0x10; /* i.e. controller */

	sprintf(adapter_name,"/dev/%s",cusobjptr->parent);

	/* open adapter (i.e. parent) */

	if((adapfd = open(adapter_name,O_RDWR)) < 0) {
		DEBUG_2("query_vpd: failed to open %s, errno=%d\n",
			adapter_name,errno)
		return(E_DEVACCESS);
	    }
	

	/* get inquiry data */
	if ((rc = scsi_inq(adapfd,inq_data,(scsi_id<<5)|lun))!=0) {
		DEBUG_3("query_vpd: inquiry at sid %d lun %d on %s failed\n",
			scsi_id,lun,adapter_name)
#ifdef CFGDEBUG
		/* analyse the error for debugging purposes */
		switch (errno) {
		case EIO:
		case ETIMEDOUT:
			DEBUG_2("TIMEOUT / EIO on sid %d, lun %d\n",
				scsi_id,lun )
			break;
		case ENODEV:
			DEBUG_2("NO Device on sid %d, lun %d\n",
				scsi_id,lun)
			break;
		default:
			DEBUG_3("Error %d on sid %d, lun %d\n",
				errno,scsi_id,lun)
			break;
		}
#endif
		return(rc);
	}
	

#ifdef CFGDEBUG
	/* Dump out the inquiry data received */
	DEBUG_2("query_vpd: data from sid %d lun %d:\n",scsi_id,lun)
	hexdump(inq_data,(long)(inq_data[4]+5));
#endif

	/* close the adapter */
	close(adapfd);

	/* Store the VPD in the database */
	*vpd_dest = '\0';

	/* Controller card part number */
	add_desc_fixed( "PN", vpd_dest, inq_data, 114, 12);

	/* Serial number */
	add_desc_fixed( "SN", vpd_dest, inq_data, 44, 8 );

	/* Product type & Model number  "XXXX-YYY" */
	strncpy( tmp_str, &inq_data[16], 4 );
	tmp_str[4] = '-';
	strncpy( &tmp_str[5], &inq_data[20], 3 );
	tmp_str[8] = '\0';
	add_descriptor( vpd_dest, "TM", tmp_str );

	/* Manufacturer */
	strncpy( tmp_str, &inq_data[8], 8 );
	tmp_str[8] = '-';
	strncpy( &tmp_str[9], &inq_data[98], 4 );
	tmp_str[13] = '\0';
	add_descriptor( vpd_dest, "MF", tmp_str );

	/* Revision level */
	add_desc_fixed( "RL", vpd_dest, inq_data, 32, 2 );


	/*
	 * Now deal with any switch cards that we may have.
	 * If the controller has switch card(s) in it, we store the
	 * VPD from them.
	 */

	
	if (num_switches != 0) 
	{
	    /*
	     *   Switch 0
	     */
	   
	    /* Part Number */
	    add_desc_fixed( "Z1", vpd_dest, inq_data, 160, 12 );
	    
	    /* EC level */
	    add_desc_fixed( "Z2", vpd_dest, inq_data, 172, 10 );
	    
	    /* Switch Card Serial Number */
	    add_desc_fixed( "Z3", vpd_dest, inq_data, 148, 8 );
	    
	    /* Switch Card FRU Number    */
	    add_desc_fixed( "Z4", vpd_dest, inq_data, 182, 12 );

	    /* Manufacturer */
	    strncpy( tmp_str, &inq_data[8], 8 );
	    tmp_str[8] = '-';
	    strncpy( &tmp_str[9], &inq_data[156], 4 );
	    tmp_str[13] = '\0';
	    add_descriptor( vpd_dest, "Z5", tmp_str );

	    /*
	     *   Switch 1
	     */

	    /* Part Number */
	    add_desc_fixed( "ZA", vpd_dest, inq_data, 216, 12 ); 
	    
	    /* EC level */
	    add_desc_fixed( "ZB", vpd_dest, inq_data, 228, 10 );
	    
	    /* Switch Card Serial Number */
	    add_desc_fixed( "ZC", vpd_dest, inq_data, 204, 8 );
	    
	    /* Switch Card FRU Number    */
	    add_desc_fixed( "ZD", vpd_dest, inq_data, 238, 12 );
	    
	    /* Manufacturer */
	    strncpy( tmp_str, &inq_data[8], 8 );
	    tmp_str[8] = '-';
	    strncpy( &tmp_str[9], &inq_data[212], 4 );
	    tmp_str[13] = '\0';
	    add_descriptor( vpd_dest, "ZE", tmp_str );
	}		
	
	DEBUG_0( "VPD stored\n" );
	
	return rc;
}


/*
 * NAME:	download_microcode
 * 
 * FUNCTION:	This function determines the proper level of microcode to
 *		download to the device, downloads it, and updates the CuAt
 *		object class to show the name of the file that was used.
 *
 * ENVIRONMENT:	This function operates as a device dependent subroutine
 *		called by the generic configure method for all devices.
 *
 * INPUT:	logical_name of device
 *
 * RETURNS:	Returns 0 if success else -1
 *
 * NOTE :	During download, the adapter is opened in diagnostics mode
 *		(..thus requiring exclusive access)
 *		Microcode file size can not be greater than 64K.
 */

int download_microcode(char *lname)
{
	char	sstr[SD_MAX_STRING_LEN];/* Search string              */
	struct	CuDv	cusobj;		/* CuDv ovject for controller */
	int	sid;			/* SCSI id for device         */
	int rc = 0;                     /* return code                */
	int	adapfd = -1;		/* File descriptor for device */

	DEBUG_0("cfgserdasdc: download_microcode\n")

	/* Read CuDv object for device */ 
	sprintf(sstr,"name = '%s'",lname);
	GETOBJ( CuDv_CLASS, sstr, &cusobj, ODM_FIRST, E_NOCuDv )


	sid = atoi( cusobj.connwhere );

	rc = download_microcode_ctrl(sid,cusobj.parent,0,adapfd);

	if (!rc)
		sd_phase2_dwld_ucode = 0;
	return(rc);
}
/*
 * NAME:	define_children
 *
 * FUNCTION:	This routine detects and defines disks attached to the
 *		controller
 *
 * ENVIRONMENT:	This function operates as a device dependent subroutine
 *		called by the generic configure method for all devices.
 *
 * INPUTS : 	logical_name, phase
 *
 * RETURNS: 	0 if success else -1
 *
 */

#ifndef INQSIZE
#define INQSIZE 255
#endif

#ifndef NULLPVID
#define NULLPVID "00000000000000000000000000000000"
#endif

int define_children(char  *lname,	     /* Logical name of controller   */
		    int	phase)		     /* Phase that is running        */
{
	char	filename[20];		/* Special filename of controller */
	char	*out_p;			/* stdout Output from odm_run_method */
	struct CuDv cusobj;		/* Controller CuDv object */
	struct CuDv dasdobj;		/* DASD CuDv Object */
	struct CuAt dasdatt;		/* DASD CuAt Object */
	struct PdDv PdDv;		/* DASD PdDv Object */
	char	sstr[100];		/* Search string */
	int	sid;			/* SCSI id of controller */
	int	lun;			/* lun of DASD */
	char	inq_data[INQSIZE];	/* Inquiry data from DASD */
	int	fd;			/* File descriptor for Controller */
	int	rc;			/* return code */
	unique_id_t pvid;		/* pvid read from disk (as a value) */
	char	pvidstr[33];		/* pvid expanded into a hex string */
	int	srchflg;		/* ODM_FIRST or ODM_NEXT */
	int	numdef;			/* Number of DASD same pvid elsewhere*/
	struct	sd_ioctl_parms reset_ctrl; /* command structure for download */
	char	name_to_configure[20];	/* Name of DASD to be configured */
	int	adapfd = -1;		/* File descriptor for device */
	IPL_REC	*iplrec;


	DEBUG_0("cfgserdasdc: define_children\n")




	sprintf( sstr, "name = '%s'", lname );
	GETOBJ( CuDv_CLASS, sstr, &cusobj, ODM_FIRST, E_NOCuDv )

	sid = atoi( cusobj.connwhere ) ;

	if ((phase == PHASE2) && (sd_phase2_dwld_ucode)){
		/*
		 * If this is a phase 2 controller configuration then
		 * verify controller microcode and if necessary perform 
		 * a download.  This is necessary because the controller
		 * could have been configured in phase1 when no microcode
		 * is available.  If this is the case then we
		 * will not hit the download microcode point this time
		 * so must check here to perform the initial phase 2 
		 * download if necessary.
		 */

		rc = download_microcode_ctrl(sid,cusobj.parent,0,adapfd);
	}

	sprintf( filename, "/dev/%s", lname );


	/* Open controller in normal mode */



	if(( fd = open(filename,O_RDWR)) < 0)
		return -1;

	for( lun=0; lun<16; lun++ ) {

		DEBUG_2("define_children(): sid=%d, lun=%d\n", sid, lun )


		if ((rc = scsi_inq(fd,inq_data,(sid<<5)|lun))!=0) {
			if( errno == ENODEV ) {
				DEBUG_2("ENODEV on sid %d, lun %d\n", sid,lun )
				continue;	/* No device at this locn */
			}
#ifdef CFGDEBUG
			
			switch (errno) {
			case EIO:
			case ETIMEDOUT:
				DEBUG_2("TIMEOUT / EIO on sid %d, lun %d\n",
					sid,lun )
				break;
			case ENODEV:
				DEBUG_2("NO Device on sid %d,lun %d\n",sid,lun)
				break;
			default:
				DEBUG_3("Error %d on sid %d, lun %d\n",
					errno,sid,lun)
				break;
			}
#endif
			continue;
		}

		if( inq_data[0] == (char)0x3f ) {
			DEBUG_1("No disk at lun %d\n", lun )
			continue;
		}

		if (inq_data[1] & 0x7f) {
			DEBUG_1("This type of DASD is not supported %d\n", 
				inq_data[1])
			continue;
		}	
	
		if( inq_data[0] == (char)0x7f ) {
			DEBUG_1("No disks on, or beyond lun %d\n", lun )
			break;
		}

		hexdump( inq_data, (long)INQSIZE );

		DEBUG_0("Evaluating uniquetype\n")

		if( get_unique_type( inq_data, PdDv.uniquetype ) != 0 )
		{
			DEBUG_0("get_uniquetype() failed!!\n")
			continue;
		}
		
		/*
		 * Don't redefine an available DASD listed at this location
		 * and do not output its name.
		 */
		sprintf( sstr,
			"parent = '%s' AND connwhere = '%d' AND status = '%d'",
			lname, lun, AVAILABLE );

		DEBUG_1("Calling odm_get_obj(..%s..)\n",sstr )

		rc = (int)odm_get_obj( CuDv_CLASS, sstr, &dasdobj, ODM_FIRST );

		if( rc == -1 ) return E_ODMGET;

		if( rc != 0 ) {
			DEBUG_3("%s already configured at sid %d lun %d\n",
				dasdobj.name, sid, lun )
			continue;
		}

		/* Read pvid from disk */
		DEBUG_1("Calling read_sd_pvid( fd, 0x%02x, &pvid)\n",
			(sid<<5)|lun)
		pvidstr[0] = '\0';

		/*
		 * To maintain compatibility with older microcode loads which
		 * do not support read with reserve, we must first attempt
		 * a normal extended read.  If that fails we will then try
		 * to use the read which works even if the DASD is reserved
		 */
		if( read_sd_pvid( fd, (sid<<5)|lun,(char) 0, pvidstr) == -1 ) {
			/*
			 * If a normal read failed, lets see if one which
			 * ignores host reservations can succeed. This type
			 * of read requires a special microcode load.  It 
			 * allow us to configure a shared disk on
			 * both hosts.
			 */

		        DEBUG_0("define_children: attempting SPECIAL read\n")
			if( read_sd_pvid( fd, (sid<<5)|lun,
					 (char) SD_READ_WITH_RESERVE, 
					 pvidstr)   == -1 ) {
				
				DEBUG_0("define_children: read_sd_pvid() failed!!\n" )
				continue;
			}


		}


		if (pvidstr[0] == '\0') {
			/*
			 * If disk has nullpvid and it has the
			 * same location as an already defined disk
			 * which also has the NULLPVID then
			 * consider these the same disks
			 * Output its name for cfgmgr
			 */
			
			sprintf( sstr,
				"parent = '%s' AND connwhere = '%d' AND status = '%d' AND PdDvLn = '%s'",
				lname, lun, DEFINED,PdDv.uniquetype );
	
			DEBUG_1("Calling odm_get_obj(..%s..)\n",sstr )
		
			rc = (int)odm_get_obj( CuDv_CLASS, sstr, 
					      &dasdobj, ODM_FIRST );
	
			if( rc == -1 ) return E_ODMGET;
	
			if( rc != 0 ) {


					DEBUG_3("%s already defined at sid %d lun %d\n",
					dasdobj.name, sid, lun )
					if (dasdobj.chgstatus == MISSING) {
						dasdobj.chgstatus = SAME;
						odm_change_obj( CuDv_CLASS, 
							       &dasdobj );
					}
					printf( "%s ", dasdobj.name );
					continue;

				}
			else {
				/*
				 * If we get here then we have NOT found a 
				 * defined disk at this location with  
				 * same uniquetype. 
				 * Therefore we will define
				 * a new disk instance.
				 */

				sprintf( sstr, "uniquetype = %s", 
					PdDv.uniquetype );
				GETOBJ( PdDv_CLASS, sstr, &PdDv, ODM_FIRST, 
				       E_NOPdDv)

				sprintf(sstr, "-c %s -s %s -t %s -p %s -w %d",
					PdDv.class, PdDv.subclass, PdDv.type, 
					lname, lun );
				DEBUG_2("cfgserdasdc running method :%s %s\n",
					PdDv.Define,sstr)
				if (rc=odm_run_method(PdDv.Define, sstr, 
						      &out_p, NULL)) {
					DEBUG_2("cfgserdasdc:can't run %s, rc=%d\n",
						PdDv.Define, rc)
				        close( fd );
				        return E_ODMRUNMETHOD;
				}

				fprintf(stdout, "%s ", out_p);

				continue;
			}		
	
		}

		sprintf( sstr, "attribute = 'pvid' AND value = '%s'",pvidstr );

		DEBUG_1("PVID comparison:%s\n", sstr )

		/*
		 * Search for all disks with same pvid
		 */

		for(
			*name_to_configure = '\0',
			numdef = 0,
			srchflg = ODM_FIRST;
			rc = (int)odm_get_obj( CuAt_CLASS, sstr, &dasdatt, 
					      srchflg),
			( rc != 0 ) && ( rc != -1 );
			srchflg = ODM_NEXT
		) {
			DEBUG_1("Comparison found: %s\n", dasdatt.name )

			sprintf( sstr, "name = '%s'", dasdatt.name );

			rc = (int) odm_get_obj( CuDv_CLASS, sstr,&dasdobj,
					       ODM_FIRST);

			if( ( rc == 0 ) || ( rc == -1 ) ) {
				DEBUG_2("Error reading CuDv where %s. rc=%d\n",
					sstr, rc )
				continue;
			}

			if(strcmp(dasdobj.PdDvLn_Lvalue,PdDv.uniquetype)!=0)
			{
				DEBUG_0("uniquetype doesn't match\n")
				continue;
			}

			if( ( strcmp( dasdobj.parent, lname ) == 0 ) &&
				( atoi(dasdobj.connwhere) == lun ) ) {
				DEBUG_1("same parent & lun: %s\n",dasdobj.name)
				strcpy( name_to_configure, dasdobj.name);
				numdef = 0;
				/* 
				 * Do not define but output name for
				 * cfgmgr
				 */
				break;
			}
#ifdef CFGDEBUG
			else {
				DEBUG_2("parent = '%s', connwhere = '%s'\n",
					dasdobj.parent, dasdobj.connwhere )
				DEBUG_2("lname = '%s', lun = %d\n", lname, lun)
			}
#endif
			if ( dasdobj.status == DEFINED ) 
				if( !numdef++ ) {
					/*
					 * If disk is defined with the same 
					 * Non null pvid, but different 
					 * location then change customized
					 * object
					 */
					DEBUG_1("DEFINED, & same pvid:%s\n",
						dasdobj.name )
					strcpy(name_to_configure,dasdobj.name);
				}
		}

		DEBUG_3(
		"Out of for loop: rc=%d, name_to_config = %s, numdef = %d\n",
			rc, name_to_configure, numdef )

		if( (*name_to_configure) && (numdef == 0) ) {
			DEBUG_1(
			"%s found DEFINED with same PVID at same locn\n",
				dasdobj.name )
			if (dasdobj.chgstatus == MISSING) {
				dasdobj.chgstatus = SAME;
				odm_change_obj( CuDv_CLASS, &dasdobj );
				}
			printf( "%s ", dasdobj.name );
			continue;
		}

		if ( (*name_to_configure) && (numdef < 2) ) {
			DEBUG_3("%s found (same PVID) at new sid-lun: %d-%d\n",
				dasdobj.name, sid, lun )
			strcpy( dasdobj.parent, cusobj.name );
			sprintf( dasdobj.location, "%s-%02d", cusobj.location,
				lun );
			sprintf( dasdobj.connwhere, "%02d", lun );
			if (dasdobj.chgstatus == MISSING) 
						dasdobj.chgstatus = SAME;
			odm_change_obj( CuDv_CLASS, &dasdobj );
			printf( "%s ", dasdobj.name );
			continue;
		}

		sprintf( sstr, "uniquetype = %s", PdDv.uniquetype );
		GETOBJ( PdDv_CLASS, sstr, &PdDv, ODM_FIRST, E_NOPdDv)

		sprintf(sstr, "-c %s -s %s -t %s -p %s -w %d",
			PdDv.class, PdDv.subclass, PdDv.type, lname, lun );
		DEBUG_2("cfgserdasdc running method :%s %s\n",PdDv.Define,sstr)
		if (rc=odm_run_method(PdDv.Define, sstr, &out_p, NULL)) {
			DEBUG_2("cfgserdasdc:can't run %s, rc=%d\n",
				PdDv.Define, rc)
			close( fd );
			return E_ODMRUNMETHOD;
		}

		fprintf(stdout, "%s ", out_p);

		if( strlen(out_p) > 0 )
			if( out_p[strlen(out_p)-1] == ' ' )
				out_p[strlen(out_p)-1] = '\0';
		if (*pvidstr)
			SETATT( out_p, "pvid", pvidstr );
	}

	close( fd );
	return 0;
}
