static char sccsid[] = "@(#)62  1.10  src/bos/usr/lib/methods/ucfgdevice/ucfgdevice.c, cfgmethods, bos411, 9431A411a 7/28/94 03:32:45";
/*
 * COMPONENT_NAME: (CFGMETH) Generic Unconfigure Method.
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27, 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

#include "cfgdebug.h"

extern	int errno;		/* System Error Number */


/*
 * NAME: main
 * 
 * FUNCTION: This process is executed to "unconfigure" a device and is generic
 *	     to most devices.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This process is invoked when a device instance is to be 
 *	"unconfigured". This involves terminating the device to the 
 *	driver and representing the unconfigured state of the device 
 *	in the database. It may also unload the appropriate device driver 
 *	if the device is the last configured device for the driver.
 *
 * NOTES: Interface:
 *              ucfgdevice -l <logical_name>
 *
 * RETURNS: Exits with > 0 if failure, 0 on success.
 */

main(argc,argv)
int argc;
char *argv[];
{

	struct	cfg_dd cfg;		/* sysconfig command structure */
	char    *logical_name;          /* logical name to unconfigure */
	char	sstring[256];		/* search criteria */
	char	errstring[256];		/* error string */

	struct	CuDv	cusobj;		/* customized devices object */
	struct	PdDv	preobj;		/* predefined devices object */
	struct  CuDv    childcusobj;    /* child cust. devices object */

	struct	Class	*cusdev;	/* customized devices class handle */

	long	majorno,minorno;	/* major and minor numbers */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	extern  int optind;             /* flag parsed by getopt() */
	extern  char *optarg;           /* argument parsed by getopt() */


	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	errflg = 0;
	logical_name = NULL;

	while ((c = getopt(argc,argv,"l:")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("ucfgdevice: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("ucfgdevice: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1( "Unconfiguring: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("ucfgdevice: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfgdevice: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfgdevice: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfgdevice: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* If device state is DEFINED, then already unconfigured */
	/* so just exit.                                         */
	if (cusobj.status == (short) DEFINED) {
		/* Device is already unconfigured */
		DEBUG_0("ucfgdevice: device is already DEFINED state")
		err_exit(E_OK);
	}

	/* See if this device has AVAILABLE children.  If it does, */
	/* it can not be unconfigured.                             */
	sprintf (sstring,"parent = '%s' AND status != %d",logical_name,
			       DEFINED);
	rc = (int)odm_get_first(cusdev,sstring,&childcusobj);
	if (rc == -1) {
		/* ODM error */
		DEBUG_0("ucfgdevice: ODM error looking for children\n")
		err_exit(E_ODMGET);
	}
	else if (rc != 0) {
		/* Device has children in configured state */
		DEBUG_0("ucfgdevice: Device has configured children\n")
		err_exit(E_CHILDSTATE);
	}
	
	DEBUG_0( "Device has no configured children\n")

	/*******************************************************
	 Note: May add checking for dependencies here.  For now
	 it is assumed that it is OK to attempt unconfiguring a device
	 that is listed in the CuDep object class as being a
	 dependent to another device.  If this device is in use
	 by the other device then this unconfig will fail when it tries
	 to terminate the device from the driver.  If the device is not
	 in use, then what does the other device care?
	 *******************************************************/


	/* get device's predefined object */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("chgdevice: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("chgdevice: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/**************************************************************
	  Perform device specific unconfigure steps.
	 **************************************************************/
	rc = unconfigure_device(&preobj,&cusobj);
	if (rc)
		err_exit(rc);

	DEBUG_0("ucfgdevice: Updating device state\n")
	/*************************************************
	  Change the status field of device to "DEFINED"
	 *************************************************/
	cusobj.status = (short)DEFINED;

	if (odm_change_obj(cusdev,&cusobj) == -1) {
		/* Could not change customized device object */
		DEBUG_0("ucfgdevice: update of CuDv object failed");
		err_exit(E_ODMUPDATE);
	}

	/*************************************************
	  Run additional device specific subroutine.
	 *************************************************/
	rc = dev_specific(&preobj,&cusobj);
	if (rc)
		err_exit(rc);
       

	/* close the CuDv object class */
	if (odm_close_class(CuDv_CLASS) == -1) {
		/* Error closing CuDv object class */
		DEBUG_0("ucfgdevice: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();

	DEBUG_0("ucfgdevice: Successful completion\n")
	exit(0);
}

/*
 * NAME: err_exit
 *                                                                    
 * FUNCTION: Error exit routine.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is the error/exit routine used by the generic 
 *	unconfigure method.
 *                                                                   
 * RETURNS: Function does not return.
 */  

err_exit(exit_code)
int     exit_code;              /* Error exit code */
{
	odm_close_class(CuDv_CLASS);
	odm_terminate();
	exit(exit_code);
}
