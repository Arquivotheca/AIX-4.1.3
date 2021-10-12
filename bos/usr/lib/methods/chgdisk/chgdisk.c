static char sccsid[] = "@(#)62	1.3  src/bos/usr/lib/methods/chgdisk/chgdisk.c, cfgmethods, bos411, 9428A410j 2/24/93 08:53:12";
/* 
 * COMPONENT_NAME : (CFGMETHODS) Device dependent change routine for disks
 *			         and read/write optical devices.
 *
 * FUNCTIONS : 	check_parms, process_disk, process_pv
 * 
 * ORIGINS : 27 
 * 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "pparms.h"
#include <cf.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <errno.h>
#include <sys/bootrecord.h>
#include <sys/scsi.h>
#include "cfgdebug.h"

#define NULLPVID "00000000000000000000000000000000"

extern int Pflag;
extern char allattrs[];

struct CuAt *getattr();

/*
 * Function prototypes
 */

int check_parms(struct attr *,int ,int ,char *,char *,char *,char *);
void process_disk(struct CuDv	*,struct PdDv *, char *);
int process_pv(struct CuDv  *,struct attr  *,int );

/*
 * NAME     : check_parms 
 *
 * FUNCTION : 
 *	This routine is responsible for creating or deleting PVID from
 *	disks and read/write optical devices.  This routine has no action 
 *	(except to return "SUCCEED"), unless the altered attribute list 
 *	contains pv  as one of its attributes.  If the device is a SCSI
 *	device, check_parms will take the appropriate action for the
 *	pv attribute with out relying on an chgdevice.  If the device is not 
 *	a SCSI device then check_parms may require chgdevice to process the 
 *	pv attribute via unconfiguration and reconfiguration.
 *     
 *
 * EXECUTION ENVIRONMENT :
 *	This function is called by the change_device function to  
 *	check the validity of the attributes of the device
 * 
 * NOTES :
 *
 *	There are two types of PVID (physical volume identifiers):
 *
 *		1. A device's PVID -  	This is the PVID in the
 *		      		     	database associated with the
 *				     	device.
 *
 *		2.  The media's PVID - 	This is the PVID written on the
 *					the media itself.
 *
 *      Disk and read/write optical use PVID differently:
 *
 *	Disks:  	For an available disk, the media's PVID is identical 
 *			to the device's PVID.  Both the disk's 
 *			configuration method and this change method routine
 *			can create and delete both of these PVID's. 
 *
 *	R/W optical: 	For an available read/write optical drive, the media's
 *			PVID is different from the device's PVID.  This is due
 *			to the removable nature of read/write optical media.
 *			The read/write optical drive's configuration method
 *			will only create the device's PVID, not the
 *			media's PVID.  This change method routine will
 *			only create or delete the media's PVID.
 *	
 *	
 * RETURNS : 
 *	0  = success
 *	E_NOPdOBJ 	= No "pv" predefined attribute found
 *	E_ODMGET	= odm_get  failed 
 *	E_NOCuDv	= Device not found in CuDv
 *	E_NOPdDv	= Device uniquetype not found in PdDv
 *	E_BUSY		= Device is already open by another process
 *	E_DEVACCESS	= Can not access device
 */

int check_parms(
struct attr *attrs,
int	Pf,	        /* if Pf == 1 then the -P flag was passed             */
		        /*       0 then the -P flag was NOT passed            */
int	Tf,		/* if Tf == 1 then the -T flag was passed             */
			/*             0 then the -P flag was NOT passed      */
char	*lname,		/* logical name of device being changed               */
char	*parent,	/* parent(logical name) of device, NULL if not changed*/
char	*location,	/* location of device, NULL if not changed            */
char	*badattr)	/* Place to report list of bad attributes             */
{
	struct CuDv	diskcudv;
	struct PdDv	diskpddv;
	char		sstr[256];
	char		pvidstr[33];
	int		rc;
	int		index = 0;
	int		index_pv = -1;


	/*
	 * Walk list of attributes to see if the "pv" attribute
	 * is in the list.
	 */

	while (attrs[index].attribute != (char *) NULL ) {
		if (!strcmp(attrs[index].attribute,"pv")) {
			/*
			 * pv attribute found.  Lets save
			 * its index.
			 */
			index_pv = index;
			break;
		}
		index++;
	}

	if (index_pv == -1 ) {
		/*
		 * "pv" attribute not in attribute list
		 * so use normal change method procedures.
		 */
		return 0;
	}

	sprintf( sstr, "name = %s", lname );
	rc = (int)odm_get_first( CuDv_CLASS, sstr, &diskcudv);
	if( rc == -1 )
		return E_ODMGET;
	if( rc == 0 )
		return E_NOCuDv;


	if( diskcudv.status != AVAILABLE ) {
		/* 
		 * Use normal procedures if device is not configured 
		 */
		DEBUG_0("Device not configured, continuing with normal change\n")
		return 0;
	}

	sprintf( sstr, "uniquetype = %s", diskcudv.PdDvLn_Lvalue );
	rc = (int)odm_get_first( PdDv_CLASS, sstr, &diskpddv);
	if( rc == -1 )
		return E_ODMGET;
	if( rc == 0 )
		return E_NOPdDv;


	if (strcmp(attrs[index_pv].value, "no") == 0) {
		/*
		 * If pv=no then we will not do anything here.
		 * Furthermore if pv=no is the only member of the
		 * attibute list then we must insure that the
		 * change method does not unconfigure and reconfigure.
		 */
		rc = process_pv(&diskcudv,attrs,index_pv);
		return rc;
	}
	if ((strcmp(attrs[index_pv].value, "yes") != 0) &&
	   (strcmp(attrs[index_pv].value, "clear") != 0)) {
		/* 
		 * Use normal procedures if invalid pv value
		 */
		return 0;
	}

	/*
	 * Read the media's PVID from the device.  We can not look at the 
	 * database for the PVID, since the device may be a read/write optical 
	 * drive.  In the case of read/write optical drives, the device's PVID
	 * (the PVID in the database associated with device) is different from
	 * the media's PVID.  For disks the device's PVID will be equal to 
	 * the media's PVID.  So for both disk and read/write optical we will 
	 * read the media's PVID.
	 */

	rc = read_pvid(lname,pvidstr,0);
	DEBUG_1("check_parms read_pvid returned pvidstr = %s\n",pvidstr)
	if (rc) {
		/*
		 * Read PVID failed. 
		 */
		return(rc);
		 
	}
	


	if ((strcmp(attrs[index_pv].value, "yes") == 0) &&
	    (strcmp( pvidstr, NULLPVID ) == 0 )) {
		/*
		 * If pv=yes and the device has a NULLPVID then
		 * a PVID needs to be created.
		 */
		if (strcmp(diskpddv.subclass,"scsi")) {
			/*
			 * For non SCSI devices we require 
			 * them to use normal change method procedures
			 * in this case (i.e. pv=yes and NULLPVID).
			 */
			return 0;
		}
		/*
		 * If this is a SCSI device then
		 * we will create the PVID here and not
		 * require the change method to handle it.
		 *
		 * We will use the SC_SINGLE openx to prevent
		 * modification of PVID in the case where the
		 * device may be in use.
		 */
		if ((rc=pvid_to_disk(lname,pvidstr,SC_SINGLE))) {
			/*
			 * If updating the media's PVID
			 * fails.
			 */

			return (rc);
		}
		process_disk(&diskcudv,&diskpddv,pvidstr);

		
	}
	else if ((strcmp(attrs[index_pv].value, "clear") == 0) &&
	    (strcmp( pvidstr, NULLPVID ) != 0 )) {
		/*
		 * If pv=clear and the device has a non NULLPVID then
		 * the NULLPVID needs to used for this device.
		 */
		if (strcmp(diskpddv.subclass,"scsi")) {
			/*
			 * For non SCSI devices we require 
			 * them to use normal change method procedures
			 * in this case (i.e. pv=clear  and not NULLPVID).
			 */
			return 0;
		}
		/*
		 * If this is a SCSI device then
		 * we will change it to the NULLPVID
		 * and not require the change method to handle it.
		 *
		 * We will use the SC_SINGLE openx to prevent
		 * modification of PVID in the case where LVM may
		 * have a volume group varied on.
		 */

		if ((rc=clear_pvid(lname,SC_SINGLE))) {
			/*
			 * If clearing the media's PVID
			 * fails.
			 */

			return (rc);
		}
		pvidstr[0] = '\0';
		process_disk(&diskcudv,&diskpddv,pvidstr);

	}
	
	rc = process_pv(&diskcudv,attrs,index_pv);
	return rc;

	

}
/*
 * NAME     : process_disk
 *
 * FUNCTION :  This routine will determine if the device
 *	       is a disk or read/write optical device.  If it
 *	       is a disk then it will update the CuAt with 
 *	       the new PVID.
 *
 * EXECUTION ENVIRONMENT :
 * 
 * NOTES :
 *	
 * RETURNS : 
 *	None
 */
void process_disk(struct CuDv	*diskcudv, 
		 struct PdDv *diskpddv,
		 char *pvidstr)
{
	
	if (!strcmp(diskpddv->class,"disk")) {
		/*
		 * If this is a disk then we need to update the
		 * database with the new PVID.
		 */
		putpvidattr(diskcudv->name,pvidstr);

	}
	
	return;
}

/*
 * NAME     : process_pv 
 *
 * FUNCTION : 
 *
 * EXECUTION ENVIRONMENT :
 * 
 * NOTES :
 *	
 * RETURNS : 
 *	0  		= success
 *	E_NOPdOBJ 	= No "pv" predefined attribute found
 *	E_ODMGET	= odm_get_first failed
 */
int process_pv(struct CuDv	*diskcudv,
	       struct attr 	*attrs,
	       int		index_pv)
{

	char		sstr[256];
	int		rc;
        struct PdAt     pvpdat;


	/*	
	 * Change database to pv=(default val) 
	 */
	sprintf( sstr,
		"attribute = 'pv' AND uniquetype = '%s'",
		diskcudv->PdDvLn_Lvalue );

	DEBUG_1("Searching for default pv using '%s'\n", sstr )

	rc = (int)odm_get_first( PdAt_CLASS, sstr, &pvpdat);
	if( rc == 0 ) {
		return E_NOPdOBJ;
	}
	else if( rc == -1 ) {
		return E_ODMGET;
	}

	strcpy( attrs[index_pv].value, pvpdat.deflt );
	if ((!index_pv) && (attrs[1].attribute == NULL)) {
		/* 
	  	 * Prevent unconfig / reconfig if pv is the only attribute
	 	 * in the attribute list.  
	 	 */
		allattrs[0] = '\0';
		Pflag = 1;
	}

	return 0;
}
