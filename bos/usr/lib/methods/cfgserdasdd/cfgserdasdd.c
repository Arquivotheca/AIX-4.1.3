static char sccsid[] = "@(#)59	1.12  src/bos/usr/lib/methods/cfgserdasdd/cfgserdasdd.c, cfgmethods, bos411, 9428A410j 5/28/93 05:22:24";
/*
 * COMPONENT_NAME: (CFGMETH) Serial DASD Disk config method
 *
 * FUNCTIONS:	generate_minor, make_special_files, 
 *	      query_vpd, build_dds
 *
 * ORIGINS:	27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<stdio.h>
#include	<fcntl.h>
#include	<malloc.h>
#include	<cf.h>
#include	<sys/sysmacros.h>
#include	<sys/sd.h>
#include	<sys/cfgodm.h>
#include	<sys/errno.h>
#include	<sys/cfgdb.h>
#include	<sys/stat.h>
#include	"cfgserdasd.h"
#include	"cfgdebug.h"
#include	<sys/bootrecord.h>

int build_dds( struct CuDv *,struct CuDv * ,char **,int *);
#define NULLPVID "00000000000000000000000000000000"

/*
 * NAME:	generate_minor
 *
 * FUNCTION:	Generates a minor number for the serial DASD disk being
 *		configured
 *
 * ENVIRONMENT:	Dynamically loaded & run by generic config
 *
 * RETURNS:	
 *		0 on success, or an error from cf.h
 */

long generate_minor(char	*lname,	    /* Logical name of device        */
		    long	major_no,   /* Major number of device        */
		    long	*minor_dest)/* Address to store minor number */
					    /* generated                     */
{
	long	*minorno;
	long	maximum_so_far = 0x3fff;
	int	how_many;

	DEBUG_0("cfgserdasdc: generate_minor\n") 

	minorno = getminor( major_no, &how_many, (char *)NULL );

	while( how_many-- )
	{
		if( *minorno > maximum_so_far )
			maximum_so_far = *minorno;
		minorno++;
	}
	
	minorno = genminor(lname,major_no,maximum_so_far+1,1,1,1);

	if(minorno == (long *)NULL)
		return E_MINORNO;

	DEBUG_2("genminor(..%ld..)=%d\n", major_no, *minorno ) 
	*minor_dest = *minorno;
	return 0;
}

/*
 * NAME:	make_special_files
 *
 * FUNCTION:	Generates two special files for the serial DASD disk being
 *		configured
 *
 * ENVIRONMENT:	Dynamically loaded & run by generic config
 *
 * NOTES:	The files generated are /dev/<logical_name>, and
 *		/dev/r<logical_name>
 *
 * RETURNS:	0 on success, or an eeror from cf.h
 */

#define MSP_MAXLEN	14	/* Maximum length of a file name	*/

int make_special_files(char  *logical_name,/* Logical name of controller     */
		       dev_t	devno)    /* Major & minor number for        */
					  /* controller			     */
{
	char basename[MSP_MAXLEN+1];
	int rc;

	DEBUG_0( "make_special_files()\n" ) 

	/* Check that there is enough room in basename for r<logical_name> */

	if( strlen(logical_name) > (MSP_MAXLEN-1) )
		return E_MAKENAME;

	/* Create the character special file */

	sprintf( basename, "r%s", logical_name );

	if( rc = mk_sp_file( devno, basename, (S_IRUSR | S_IWUSR | S_IFCHR)))
		return rc;

	/* Generate block file */

	sprintf( basename, "%s", logical_name );

	return mk_sp_file( devno, basename, ( S_IRUSR | S_IWUSR | S_IFBLK ));
}

/*
 * NAME:	query_vpd
 *
 * FUNCTION:	Obtains Vital Product Data From Disk, and places it in the
 *		customized database
 *
 * RETURNS:	0 on success, or an errno
 */
 
#define	INQSIZE	255
static	uchar	inq_data[INQSIZE];	/* Storage area for VPD	*/

int query_vpd(struct CuDv *cusobj, /* Pointer to customized object rec     */
	      mid_t kmid,	   /* Kernel module I.D. for Dev Driver	   */
	      dev_t  devno,	   /* Concatenated Major & Minor No.s	   */
	      char  *vpd_dest)     /* Area to place VPD			   */
{
	char	controller_name[20];	/* Name of serial DASD controller*/
	int	scsi_id, lun;		/* Extracted from connwhere	*/
	int	rc;			/* Return code			*/
	int	ctrlfd;			/* file descriptor for controller*/
	struct	CuDv  parobj;		/* CuDv object of parent	*/
	char	tmp_str[100];

	DEBUG_0( "query_vpd()\n") 

	/* Read CuDv for parent (controller ) */

	sprintf( tmp_str, "name = '%s'", cusobj->parent );
	GETOBJ( CuDv_CLASS, tmp_str, &parobj, ODM_FIRST, E_NOCuDvPARENT )

	scsi_id = atoi( parobj.connwhere );
	lun = atoi( cusobj->connwhere );

	DEBUG_0("Opening controller\n") 

	/* open controller (i.e. parent's parent) */
	sprintf(controller_name,"/dev/%s",parobj.name);

	if((ctrlfd = open(controller_name,O_RDWR)) < 0) {
		DEBUG_2("query_vpd: failed to open %s, errno=%d\n",
			controller_name,errno) 
		return E_DEVACCESS;
	}

	DEBUG_0("Calling scsi_inq()\n") 

	/* get inquiry data */
	if ((rc = scsi_inq(ctrlfd,inq_data,(scsi_id<<5)|lun))!=0) {
		DEBUG_3("query_vpd: inquiry at sid %d lun %d on %s failed\n",
			scsi_id,lun,controller_name) 
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
		return rc;
	}

#ifdef CFGDEBUG
	/* Dump out the inquiry data received */
	DEBUG_2("query_vpd: data from sid %d lun %d:\n",scsi_id,lun) 
	hexdump(inq_data,(long)(inq_data[4]+5));
#endif

	/* close the controller */
	close(ctrlfd);

	/* make sure that the disk we're looking at is the same type
	   as the database says it is */
	if ((rc=chktype(inq_data,cusobj->PdDvLn_Lvalue))!=0) {
		DEBUG_1("query_vpd: chktype failed, rc=%d\n",rc) 
		return rc;
	}

	/* Store the VPD in the database */
	*vpd_dest = '\0';

	/* Part number */
	add_desc_fixed( "PN", vpd_dest, inq_data, 114, 12 );

	/* EC Level */
	add_desc_fixed( "EC", vpd_dest, inq_data, 126, 10 );

	/* Serial number */
	add_desc_fixed( "SN", vpd_dest, inq_data,36,8);

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

	/*
	 * The RL field is built differently for allicat than for
	 * other devices.
	 * Also, of the Z fields, only Z2 is used for allicat.
	 */
	
	if (strncmp( cusobj->PdDvLn_Lvalue,"disk/serdasdc/2000mb") == 0){
	    tmp_str[0] = inq_data[32];
	    tmp_str[1] = '-';
	    tmp_str[2] = inq_data[34];
	    tmp_str[3] = '\0';
	    add_descriptor( vpd_dest, "RL", tmp_str );   
 
	    /* Ram load part number */
	    add_desc_fixed( "Z2", vpd_dest, inq_data, 44, 12 );
	}
	else {
	    /* ROM Code Level */
	    add_desc_fixed( "RL", vpd_dest, inq_data, 32, 2 );
	    
	    /* Product Ship Serial Number */
	    add_desc_fixed( "Z1", vpd_dest, inq_data, 23, 9 );
	    
	    /* Logic Card Serial Number */
	    add_desc_fixed( "Z2", vpd_dest, inq_data, 44, 8 );
	    
	}
	/* Logic Card FRU Part Number */
	add_desc_fixed( "Z3", vpd_dest, inq_data, 136, 12 );
	
	/* Date of manufacture */
	add_desc_fixed( "Z4", vpd_dest, inq_data, 102, 5 );
	
	DEBUG_0( "VPD stored\n" ) 
	return rc;
}

int chktype(uchar	*inqdata,
	    char	*utype)
{
	char	unique_type[50];	/* uniquetype from get_unique_type */

	if( get_unique_type( inqdata, unique_type ) != 0 )
		return -1;

	if( strcmp( utype, unique_type ) != 0 )
		/* unique types didn't match */
		return -1;

	return 0;	/* types appear to match */
}
/*
 * NAME:	build_dds
 *
 * FUNCTION:	Device dependant dds build for serial DASD disk
 *
 * RETURNS:	0 if succeed, or errno if fail
 */
int build_dds(struct CuDv *cusdesc,/* disk   */
	      struct CuDv  *parobj,/* controller of disk  */
	      char **dds_data_ptr,/*Address to store address of dds generated*/
	      int  *dds_len)      /*Address to store length of dds generated */

{
	struct	sd_dasd_dds *dds;	/* dds for Disk from serdasd.h	*/
	char	search_str[128];	/* Search criterion		*/
	struct	Class	*c_att_oc;	/* Customized Attribute Object Class */
	struct	Class	*p_att_oc;	/* Predefined Attribute Object Class */
	int	rc;			/* Return code			*/
	long	major_no;		/* Major number of adapter	*/
	long	minor_no;		/* Minor number of adapter	*/
	int	how_many;
	char	controller_name[20];	/* Name of serial DASD controller*/
	long	genmajor();
	int	ctrlfd;			/* file descriptor for controller*/
	int	scsi_id, lun;		/* Extracted from connwhere	*/
	char	inq_data[INQSIZE];	/* Inquiry data from DASD */

	DEBUG_0( "build_dds()\n" ) 

	if ((dds = (struct sd_dasd_dds *)malloc(sizeof(struct sd_dasd_dds)))
		== NULL) {
		DEBUG_0("Failed to malloc space for struct disk_ddi\n") 
		return E_MALLOC;
	}

	/* Zero dds structure */
	bzero(dds, sizeof(struct sd_dasd_dds));

	ODMOPEN( c_att_oc, CuAt_CLASS )
	ODMOPEN( p_att_oc, PdAt_CLASS )
	

	/* Read the attributes from the customized & predefined classes */

	dds->safe_relocate = 1;
	dds->extended_rw = 1;
	dds->segment_size = 1000000;
	dds->segment_cnt = 0;
	dds->byte_count = 0;
	dds->max_coalesce = 0x10000;
        GETATT( &dds->queue_depth, 'i', (*cusdesc), "queue_depth", NULL )


	if( ( dds->mode_data_length = -( rc = getatt(dds->mode_data,
	    'b', c_att_oc, p_att_oc, cusdesc->name, cusdesc->PdDvLn_Lvalue,
	    "mode_data", NULL ))) < 0 )
		return rc;

	if( ( dds->mode_default_length = -( rc = getatt(dds->mode_default_data,
	    'b', c_att_oc, p_att_oc, cusdesc->name, cusdesc->PdDvLn_Lvalue,
	    "mode_default", NULL ))) < 0 )
		/*
		 * If mode default attribute doesn't exist, don't fail,
		 * just ignore
		 */
		dds->mode_default_length = 0;

	dds->dev_type = 2; /* DASD */
	strncpy( dds->resource_name, cusdesc->name, sizeof(dds->resource_name) );
	dds->lun_id = atoi( cusdesc->connwhere );


	major_no = genmajor( parobj->ddins );
	minor_no = *getminor( major_no, &how_many, parobj->name );	
	dds->controller_devno = makedev( major_no, minor_no );

	*dds_data_ptr = (char *)dds;		/* Store address of struct*/
	*dds_len = sizeof(struct sd_dasd_dds );/* Store size of structure*/

#ifdef CFGDEBUG
	dump_dds( *dds_data_ptr, *dds_len );
#endif
	/* Read CuDv for parent (controller ) */

	scsi_id = atoi(parobj->connwhere );

	DEBUG_0("Opening controller\n") 
	

	/* open controller (i.e. parent's parent) */
	sprintf(controller_name,"/dev/%s",parobj->name);

	if((ctrlfd = open(controller_name,O_RDWR)) < 0) {
		DEBUG_2("build_dds: failed to open %s, errno=%d\n",
			controller_name,errno) 
		return E_DEVACCESS;
	}

	DEBUG_0("Calling scsi_inq()\n") 

	/* get inquiry data */
	if ((rc = scsi_inq(ctrlfd,inq_data,(scsi_id<<5)|dds->lun_id))!=0) {
		DEBUG_3("query_vpd: inquiry at sid %d lun %d on %s failed\n",
			scsi_id,dds->lun_id,controller_name) 
#ifdef CFGDEBUG
		/* analyse the error for debugging purposes */
		switch (errno) {
		case EIO:
		case ETIMEDOUT:
			DEBUG_2("TIMEOUT / EIO on sid %d, lun %d\n",
				scsi_id,dds->lun_id) 
			break;
		case ENODEV:
			DEBUG_2("NO Device on sid %d, lun %d\n",
				scsi_id,dds->lun_id) 
			break;
		default:
			DEBUG_3("Error %d on sid %d, lun %d\n",
				errno,scsi_id,dds->lun_id) 
			break;
		}
#endif
		return rc;
	}

	if (inq_data[1] & 0x7f) {
		DEBUG_1("This type of DASD is not supported %d\n", 
			inq_data[1])
		return E_DEVACCESS;;
	}	
	
	DEBUG_0( "build_dds() returning 0\n" ) 

	return 0;
}


int get_pvidstr(struct	CuDv *diskcudv,
		char	*pvidstr)
{
	unique_id_t	pvid;		/* Numerical form of pvid */
	int		tarlun;		/* sid<<5|lun for the disk */
	int		ctrlfd;		/* File descriptor for controller */
	char		dev[64];	/* Pathname for controller */
	IPL_REC		*iplrec;
	
	sprintf(dev,"/dev/%s",diskcudv->parent);

	if ((ctrlfd=open(dev,O_RDWR))<0) {
		DEBUG_2("get_pvidstr: open %s failed, errno=%d\n", dev, errno)
		return(-1);
	}

	tarlun = (( atoi(&diskcudv->location[6])) << 5 ) |
		( atoi(&diskcudv->location[9]) );

	DEBUG_1("get_pvidstr( tarlun = %d )\n", tarlun )
	pvidstr[0] = '\0';
	if ( read_sd_pvid( ctrlfd, tarlun, (char) 0,pvidstr) == -1 )
	{
		/*
		 * If a normal read failed, lets see if one which
		 * ignores host reservations can succeed.This type
		 * of read requires a special microcode load.  It 
		 * allow us to configure a shared disk on
		 * both hosts.  
		 */
		if( read_sd_pvid( ctrlfd, tarlun,
				 (char) SD_READ_WITH_RESERVE, 
				 pvidstr)  == -1 ) {
			DEBUG_0("read_sd_pvid() failed\n")
		        return -1;			
		}

	}




	
	DEBUG_1("get_pvidstr() yields pvidstr='%s'\n", pvidstr )

	return 0;
}


int disk_present(struct	CuDv	*disk,    /* Customized for this disk */
		 struct PdDv    *preobj,  /* predefined for this disk */
		 struct CuDv	*parent,  /* parent of this disk      */
		 char	        *pvidattr)/* pvid of this disk from CuAt */
{
	char	adapname[64];		/* path for adapter special file */
	int	adapfd;			/* File descriptor for adapter */
	uchar	inqdata[255];		/* Inquiry data */
	int	tarlun;			/* sid<<5 | lun for disk */
	char	pvidstr[33];		/* string form of pvid */
	int	chk;			/* Return from chktype() */
	char	sstr[200];		/* search string */
	struct	CuAt dpvidattr;		/* Dummy pvid attribute */
	struct	CuAt *dpvid;		/* Pointer to Dummy pvid attribute */


	tarlun = (( atoi(&disk->location[6])) << 5 ) |
		(atoi(&disk->location[9]) );

	DEBUG_2("disk_present( name = %s, tarlun = %d )\n",disk->name,tarlun)

	/* make sure parent is configured */
	if (parent->status!=AVAILABLE) {
		DEBUG_1("disk_present: parent status=%d\n",parent->status)
		return(E_PARENTSTATE);
	}

	/* open the adapter */
	sprintf(adapname,"/dev/%s",parent->parent);

	if ((adapfd = open(adapname,O_RDWR))<0) {
		/* error opening adapter */
		DEBUG_2("disk_present: error opening %s, errno=%d\n",
			adapname,errno)
		return(E_DEVACCESS);
	}

	/* Read the inquiry data from the disk */
	if( scsi_inq( adapfd, inqdata, tarlun) == -1 )
	{
		DEBUG_1("scsi_inq(tarlun = 0x%02x) failed \n", tarlun )
		return -1;
	}

	if( inqdata[0] != '\0' )
	{
		DEBUG_1("inqdata[0]=0x%02x (disk not there)\n",
			(int)inqdata[0] )
		return -1;
	}

	/* make sure drive is the right type */
	DEBUG_0("disk_present: calling chktype\n")
	chk=chktype(inqdata,disk->PdDvLn_Lvalue);

	if( chk==E_ODMGET ) {
		/* error checking on drive types */
		DEBUG_0("disk_present: ODMGET error checking on drive types\n")
		close(adapfd);
		return(chk);
	}

	if (chk==E_WRONGDEVICE) {
		/* not the same type */
		DEBUG_0("disk_present: Wrong type of disk!\n")
		close(adapfd);
		return(chk);
	}

	/* get pvid from the disk */
	if (get_pvidstr(disk,pvidstr)) {
		DEBUG_2("disk_present: failed to get pvid from %s, errno=%d\n",
			disk->name,errno)
		return(E_DEVACCESS);
	}

	/* If the pvid on the disk doesn't match the one in the database it */
	/* isn't the right device: 					    */
	if (strcmp(pvidattr,pvidstr) && (*pvidattr) && (*pvidstr)) {
		DEBUG_0("pvid comparision failed\n")
		return E_WRONGDEVICE;
	}

	/* if the drive has a pvid then see if there is an
	   object for it in the database */
	if (*pvidstr) {  

		sprintf(sstr,"attribute = 'pvid' AND value = '%s'",pvidstr);

		dpvid = odm_get_first(CuAt_CLASS,sstr,&dpvidattr);

		if( (int)dpvid == -1 )
			return(E_ODMGET);

		
	}

	/* Can't prove it isn't the same disk */

	return 0;
}
