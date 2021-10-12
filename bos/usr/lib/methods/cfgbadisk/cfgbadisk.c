static char sccsid[] = "@(#)42	1.18  src/bos/usr/lib/methods/cfgbadisk/cfgbadisk.c, cfgmethods, bos411, 9428A410j 2/24/93 08:49:57";
/*
 * COMPONENT_NAME: (CFGMETHODS) Direct attached disk configuration method
 *
 * FUNCTIONS : 	generate_minor, make_special_files,disk_present
 *		build_dds,download_microcode, query_vpd, get_pvidstr
 * 
 * ORIGINS : 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 *
 * RESTRICTED RIGHTS LEGEND
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysmacros.h>
/*#include <sys/intr.h>*/
#include <sys/badisk.h>
/*#include <sys/errno.h>*/
/*#include <sys/mdio.h>*/
#include <sys/bootrecord.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "cfgdebug.h"

#define	BADPERMS	S_IRUSR|S_IWUSR
#define	BAD_BUS_ID	0x800c0060
#define	NULLPVID	"00000000000000000000000000000000"

/*	extern declarations	*/
extern	long	*genminor();
char	*pvidtoa();


/*
 * NAME: generate_minor()
 *
 * FUNCTION: Generate the minor numbers needed to configure a device
 *
 * EXECUTION ENVIRONMENT:
 *	This function is part of the config method for the Bus attached
 *	disk. It executes whenever a Bus attached disk is being configured
 *	into the system. This can be at ipl or run time. This func-
 *	makes use of the ODM library to look up attributes used to
 *	build the dds.
 *
 * NOTES:
 *	This function uses the genminor() function to reserve the
 *	minor number used for this Bus attached disk. The reservation is
 *	logged in the configuration database by genminor().
 *
 *	generate_minor() is passed the logical name of the device
 *	that is being configured, and the device's major number.
 *	These values are used to log the minor numbers in the data-
 *	base.
 *
 * RECOVERY INFORMATION:
 *	Recovery for this function (assuming a failure that does not
 *	kill the process), is left up to the caller. Since the minor
 *	number cannot be generated, then the device configuration will
 *	probably fail.
 *
 * RETURNS:
 *	0 on success with minor number stored via last parameter.
 *	errno on failure
 */

int generate_minor(lname,majorno,minordest)
char	*lname;
long	majorno;
long	*minordest;
{
	long	*minorno;

	minorno = genminor(lname,majorno,-1,1,1,1);
	if(minorno == (long *)NULL)
		return E_MINORNO;
	*minordest = *minorno;
	return 0;
}

/*
 * NAME: make_special_files()
 *
 * FUNCTION: Creates the special files used to talk to a device.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is part of the config method for the Bus attached
 *	disk. It executes whenever a Bus attached disk is being configured
 *	into the system. This can be at ipl or run time.
 *
 * NOTES:
 *	This function creates both the raw and block device special
 *	files used to communicate with the Bus attached disk driver. It
 *	calls the function mk_sp_file() to actually create the special
 *	files.
 *
 *	This function gets passed the device's logical name (which
 *	is the basis for the special file name), and the devno of
 *	the device.
 *
 * RECOVERY INFORMATION:
 *	Recovery for this function (assuming a failure that does not
 *	kill the process), is left up to the caller. Since the spe-
 *	cial files can't be made, this means that the device config-
 *	uration will probably fail.
 *
 * RETURNS:
 *	0	If the special files are made succesfully.
 *	errno	If the special files can't be made.
 */

int make_special_files(lname,devno)
char	*lname;
dev_t	devno;
{
char	filename[32];
int	rc;

	if((rc=mk_sp_file(devno,lname,(S_IFBLK|BADPERMS))) > 0)
		return rc;
	sprintf(filename,"r%s",lname);
	if((rc=mk_sp_file(devno,filename,(S_IFCHR|BADPERMS))) > 0)
		return rc;
	return 0;
}


/*
 * NAME: build_dds()
 *
 * FUNCTION: Build the DDS to initialize the BAdisk driver
 *
 * EXECUTION ENVIRONMENT:
 *	This function is part of the config method for the Bus attached
 *	disk. It executes whenever a Bus attached disk is  configured
 *	into the system. This can be at ipl or run time. This func-
 *	makes heavy use of the ODM library to look up attributes
 *	used to build the dds.
 *
 * NOTES:
 *	The basic strategy for this function is to look up all of the
 *	attributes for the disk, and its parents, needed for it to
 *	complete the DDS. If the function can't get any of the attri-
 *	butes, then it should fail.
 *
 *	The function gets the logical name of the disk that it is
 *	configuring that it uses to base all of its data base quer-
 *	ies from. It also gets a pointer to the calling function's
 *	dds pointer (that it will set to point to the finished dds),
 *	and a pointer to an integer that holds the length of the
 *	finished dds (which build_dds also fills in).
 *
 * RECOVERY INFORMATION:
 *	Recovery for this function (assuming a failure that does not
 *	kill the process), is left up to the caller. Since the DDS
 *	cannot be built in a failure, then this will probably mean
 *	that the device configuration will fail.
 *
 * DATA STRUCTURES:
 *	This function creates a valid (complete) DDS needed to con-
 *	figure a Bus attached disk driver.
 *
 * RETURNS:
 *	0	if the dds is built succesfully, the ddsptr is set
 *		to point at the completed dds, and the ddslen is
 *		set to the length in bytes of the dds.
 *	errno	if fail
 *
 */

int build_dds(cusobj,parobj,dds_data_ptr,dds_len)
struct	CuDv	*cusobj;	/* device's CuDv object */
struct	CuDv	*parobj;	/* device's parent's CuDv object */
char	**dds_data_ptr;		/* returned DDS pointer */
long	*dds_len;		/* returned DDS size */
{
int	rc;			/* used for return codes */
char	bb[4];			/* used to recieve battery backed attr val. */
char	*lname;			/* pointer to device name */
char	*pname;			/* pointer to parent name */
char	*ut;			/* pointer to device's uniquetype */
char	*pt;			/* pointer to parent's uniquetype */
static  struct  ba_dds *dds;	/* pointer to DDS */
struct	Class	*cusatt,*preatt;

	dds = (struct ba_dds *)malloc(sizeof(struct ba_dds));
	if(dds == (struct ba_dds *)NULL){
	    DEBUG_0("bld_bbdds: malloc failed\n")
	    return E_MALLOC;
	}

	/* save some strings for short hand */
	lname = cusobj->name;
	ut = cusobj->PdDvLn_Lvalue;
	pname = cusobj->parent;
	pt = parobj->PdDvLn_Lvalue;

	/* Initialize some fields in the DDS */
	/* These are hard coded here instead of being attributes in */
	/* PdAt to save disk space and because they are not changable */
	dds->bucket_size = 1000000;
	dds->bucket_count = 0;
	dds->byte_count = 0;


	DEBUG_2("build_dds: pname = %s pt = %s\n",pname,pt)
	if((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open CuAt\n")
	    return E_ODMOPEN;
	}
	if((int)(preatt = odm_open_class(PdAt_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open PdAt\n")
	    return E_ODMOPEN;
	}

	/* get bus attributes */
	if((rc=getatt(&dds->bus_id,'l',cusatt,preatt,pname,pt,
	    "bus_id",NULL))>0)return rc;
	dds->bus_id |= BAD_BUS_ID;
	if((rc=getatt(&dds->bus_type,'h',cusatt,preatt,pname,pt,
	    "bus_type",NULL))>0)return rc;

	/* get device attributes */
	if((rc=getatt(&dds->base_address,'i',cusatt,preatt,lname,ut,
	    "bus_io_addr",NULL))>0)return rc;
	if((rc=getatt(&dds->intr_level,'i',cusatt,preatt,lname,ut,
	    "bus_intr_lvl",NULL))>0)return rc;
	if((rc=getatt(&dds->intr_priority,'i',cusatt,preatt,lname,ut,
	    "intr_priority",NULL))>0)return rc;
	if((rc=getatt(&dds->dma_level,'c',cusatt,preatt,lname,ut,
	    "dma_lvl",NULL))>0)return rc;
	if((rc=getatt(bb,'s',cusatt,preatt,lname,ut,"bb",NULL))>0)return rc;
	dds->battery_backed = (strcmp(bb,"yes") == 0) ? 1 : 0;
	if((rc=getatt(&dds->max_coalesce,'i',cusatt,preatt,lname,ut,
	    "max_coalesce",NULL))>0)return rc;

	dds->alt_address = (dds->base_address == 0x3510) ? (0) : (1);
	strcpy(dds->resource_name,lname);
 	dds->slot = atoi(cusobj->connwhere)-1;

	*dds_data_ptr = (char *)dds;
	*dds_len = sizeof(struct ba_dds);

	DEBUG_1("BAdisk DDS length: %d\n",*dds_len)
#ifdef	CFGDEBUG
	hexdump(*dds_data_ptr,(long)*dds_len);
#endif	CFGDEBUG
	odm_close_class(cusatt);
	odm_close_class(preatt);
	return 0;
}

/*
 * NAME : download_microcode
 * 
 * FUNCTION :
 *	As BAD adapter does not have any microcode to be downloaded
 *	it always returns success.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : logical_name
 *
 * RETURNS: Returns 0 if success else errno
 *
 * RECOVERY OPERATION:
 *
 */

int download_microcode(lname)
char	*lname;
{
	return 0;
}

/*
 * NAME	: query_vpd
 *
 * FUNCTION : this functions opens the raw badisk device and extracts
 *	the manufacturing header information using BAMFGH ioctl command
 *	and then stores the bar_code of the device in the vpd field of
 *	the CuDv object class. Leading spaces in barcode are skipped.
 *	vpd key word used: "Z0".
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called by generic config method.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : pointer to CuDv, driver kmid, devno
 *
 * RETURNS: Returns 0 if success else errno
 *
 * RECOVERY OPERATION:
 *
 */

int query_vpd(CuDv,kmid,devno,vpd_dest)
struct	CuDv *CuDv;
mid_t	kmid;
dev_t	devno;
char	*vpd_dest;
{
	int	fd;
	char	*p,*q;
	uchar	mfgh_len;
	char	device[32];
	char	barcode[32];
	struct	badisk_mfg_header vpd;
	struct	CuAt	*cusatt;
	struct	PdAt	pdat;
	char	sstring[256];
	int	how_many;
	int	rc;
	char	size_in_mb[10];
	struct	devinfo	disk_info;
	char	*msg_no[3];		/* pointers to text desc msg nos */
	int	index;			/* loop variable */
	long	size;			/* size of disk */

	sprintf(device,"/dev/r%s",CuDv->name);
	if((fd = openx(device,O_RDONLY,0,1)) < 0){
	    DEBUG_1("query_vpd: can't open %s\n",device)
	    return E_DEVACCESS;
	}

	/* The first descriptor is "*Z0" which is the barcode on the disk */

	if(ioctl(fd,BAMFGH,&vpd) < 0){
	    DEBUG_1("query_vpd: can't get vpd for %s\n",device)
	    return E_VPD;
	}
	p = &vpd.bar_code[0];
	q = &vpd.date_mf[0];
	while((*p == ' ') && (p < q))p++;
	*q='\0';
	add_descriptor( vpd_dest, "Z0", p );

	/* The second descriptor is "*SZ" which is the size in 10^6 bytes */

	/*
		get disk size using IOCINFO ioctl cmd
	*/
	if(ioctl(fd,IOCINFO,&disk_info) < 0){
		DEBUG_1("define_ch: can not get size for %s\n",CuDv->name)
		return E_DEVACCESS;
	}

	close( fd );

	/* Note DASD Mb are 10^6bytes, NOT 2^20bytes */
	size = (long)disk_info.un.dk.bytpsec *
				disk_info.un.dk.numblks / 1000000L;

	sprintf( size_in_mb, "%ld", size);
	add_descriptor( vpd_dest, "SZ", size_in_mb );

#ifdef	CFGDEBUG
	hexdump(vpd_dest,(long)VPDSIZE);
#endif	CFGDEBUG


	/* Set the "size" attribute */
	if( ( cusatt = getattr( CuDv->name, "size", 0, &how_many ))
		== (struct CuAt *)NULL ){
		DEBUG_0("ERROR: getattr() failed\n")
		return E_NOATTR;
	}
	DEBUG_1("Value received by getattr: %s\n", cusatt->value )

	/* Only rewrite if actually changed */
	if (strcmp(cusatt->value,size_in_mb)) {
		strcpy( cusatt->value, size_in_mb );
		putattr( cusatt );
	}

	/* Set the "desc" attribute */
	/* first get predefined description attribute */
	sprintf(sstring, "uniquetype = %s AND type = T", CuDv->PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdAt_CLASS,sstring,&pdat);
	if (rc==-1) {
		/* ODM failure */
		DEBUG_0("ODM failure getting description attribute \n")
		return(E_ODMGET);
	} else if (rc == 0) {
		/* No description attribute present */
		DEBUG_0("No description attribute\n")
		return(E_NOATTR);
	} else {
		msg_no[0] = strtok(pdat.values,",");
		for (index=1; index<3; index++) {
			msg_no[index] = strtok((char *)NULL,",");
		}
	}

	/* Determine desc attibute value */
	if (size == 120)
		/* set index to 120MB disk description */
		index = 1;
	else if (size == 160)
		/* set index to 160MB disk description */
		index = 2;
	else
		/* set index to default disk description */
		index = 0;

	if( ( cusatt = getattr( CuDv->name, "desc", 0, &how_many ))
		== (struct CuAt *)NULL ){
		DEBUG_0("ERROR: getattr() failed\n")
		return(E_NOATTR);
	}
	DEBUG_1("Value received by getattr: %s\n", cusatt->value )

	/* Only rewrite if actually changed */
	if (strcmp(cusatt->value,msg_no[index])) {
		strcpy( cusatt->value, msg_no[index]);
		putattr( cusatt );
	}

	return(0);
}
/*
 * NAME: disk_present
 *
 * FUNCTION: Device dependent routine that checks to see that the disk device
 *	     is still at the same connection location on the same parent as	
 *	     is indicated in the disk's CuDv object.
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:	0 if succeed, or errno if fail
 */

int
disk_present(cusobj,preobj,parobj,pvidattr)
struct	CuDv	*cusobj;	/* disk's customized object */
struct	PdDv	*preobj;	/* disk's predefined object */
struct	CuDv	*parobj;	/* disk's parent's customized object */
struct	CuAt	*pvidattr;	/* disk's CuAt pvid attribute value */
{
	char	sstr[256];
	int	rc;
	int	slot;
	ushort	devid;
	char	cfged_list[512];
	char	not_resolved[512];
	char	*pname;		/* parent's name */

	/* make sure parent is configured */
	if (parobj->status!=AVAILABLE) {
		DEBUG_1("disk_present: parent status=%d\n",parobj->status)
		return(E_PARENTSTATE);
	}

	slot = atoi(cusobj->connwhere);
	devid = (ushort)strtol(preobj->devid,(char **)NULL,0);
	pname = cusobj->parent;

	/* make sure device is in slot we think it is in */
	DEBUG_0("Calling chkslot\n")
	sprintf(sstr,"/dev/%s",pname);
	rc = chkslot(sstr,slot,devid);
	if (rc != 0) {
		DEBUG_2("disk %s not found in slot %d\n",cusobj->name,slot)
		return(E_NODETECT);
	}

	/* make sure bus attributes are allocated */
	rc = busresolve(cusobj->name,(int)0,cfged_list,not_resolved,pname);
	if (rc != 0) {
		DEBUG_0("bus resources could not be resolved\n")
		return(E_BUSRESOURCE);
	}

	DEBUG_0("disk_present: returning OK\n")
	return(E_OK);
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
get_pvidstr(disk,pvidstr)
struct	CuDv *disk;
char	*pvidstr;
{
	int 	rc;


	rc = read_pvid(disk->name,pvidstr,0);
	if (rc)
		return (rc);

	if( strcmp( pvidstr, NULLPVID ) == 0 )
		pvidstr[0] = '\0';


	DEBUG_0("get_pvidstr: returning\n")
	return(0);
}

/*
 * NAME: finddisk
 *
 * FUNCTION: Used to find a disk that has been moved.
 *
 * EXECUTION ENVIRONMENT:
 *    This code is designed to be linked with cfgdisk.
 *
 * RETURNS:
 * 0. if succeeded
 * 
 */

int
finddisk(disk,par)
struct	CuDv *disk,*par;
{
	/* We do not provide this function for bus attached disks */
	return(0);
}
