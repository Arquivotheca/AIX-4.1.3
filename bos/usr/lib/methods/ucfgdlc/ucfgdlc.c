static char sccsid[] = "@(#)98  1.13  src/bos/usr/lib/methods/ucfgdlc/ucfgdlc.c, dlccfg, bos411, 9428A410j 10/19/93 09:44:51";
/*
 * COMPONENT_NAME: (DLCCFG) DLC Configuration
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <string.h>

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"
extern  int errno;              /* System Error Number */

/* miscellaneous defines */


/***************************************************************************
 * NAME: main
 *
 * FUNCTION: This process is executed to "unconfigure" a device and is generic
 *           to all devices.
 *
 * EXECUTION ENVIRONMENT:
 *
 * This process is invoked when a device instance is to be "unconfigured".
 * This involves terminating the device to the driver and representing the
 * unconfigured state of the device in the database. It may also unload
 * the appropriate device driver if the device is the last configured device
 * for the driver.
 *
 * NOTES: Interface:
 *              ucfgdlc -l <logical_name>
 *
 * RETURNS: Exits with > 0 if failure, 0 on success.
 */

extern mid_t    loadext();
extern int              odm_initialize();
extern int              odm_terminate();
extern struct Class    *odm_open_class();
extern int              odm_close_class();
extern void            *odm_get_obj();
extern int              odm_change_obj();


main(argc,argv)
int argc;
char *argv[];
{

	struct  cfg_dd cfg;             /* sysconfig command structure */
	char    *logical_name;          /* logical name to unconfigure */
	char    sstring[MAX_ODMI_CRIT]; /* search criteria */

	struct  CuDv    cusobj;         /* customized devices object */
	struct  PdDv    preobj;         /* predefined devices object */

	struct  Class   *cusdev;        /* customized devices class handle */
	struct  Class   *predev;        /* predefined devices class handle */

	long    majorno,minorno;        /* major and minor numbers */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */
	int     rc;                     /* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	extern int optind;              /* flag parsed by getopt() */
	extern char *optarg;            /* argument parsed by getopt() */



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
		DEBUG_0("ucfgdlc: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("ucfgdlc: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1( "Unconfiguring: %s\n",logical_name)

	/*****************
	  Initialize ODM
	 *****************/
	rc = odm_initialize();
	if (rc == -1)
	{
		/* init failed */
		DEBUG_0("ucfgdlc: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}

	if ((rc = odm_lock("/etc/objrepos/config_lock",0)) == -1)
	{
		DEBUG_0("ucfgdlc: odm_lock() failed\n")
		err_exit(E_ODMLOCK);
	}
	DEBUG_0 ("ODM initialized and locked\n")

	/***********************************************
	   Open customized devices object class (CuDv)
	 ***********************************************/
	if ((int) (cusdev = odm_open_class(CuDv_CLASS)) == -1)
	{
		/* open failed */
		DEBUG_0("ucfgdlc: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/*******************************
	  Get customized device object
	 *******************************/
	sprintf (sstring,"name = '%s'",logical_name);

	rc = (int) odm_get_obj(cusdev,sstring,&cusobj,ODM_FIRST);
	if (rc == 0)
	{
		/* No object */
		DEBUG_0("ucfgdlc: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc == -1)
	{
		/* Can't get object */
		DEBUG_0("ucfgdlc: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/********************************************************
	  Make sure device is configured (AVAILABLE or STOPPED)
	 ********************************************************/
	if (cusobj.status == (short) DEFINED)
	{
		DEBUG_0("ucfgdlc: device is already DEFINED state")
		err_exit(E_OK);
	}

	/* there are no children for this device */
	DEBUG_0( "Device has no configured children\n")

/******************************************************
  Make sure there are no dependencies on this device
 ******************************************************/
 /* there are no dependencies */

/**********************************************************
   Check to see if the device has a device driver. If it
   does, then call loadext(). If needed, this function
   will unload the driver.
 **********************************************************/

	/********************************************
	  Open the predefined devices object class
	 ********************************************/
	if ((int) (predev=odm_open_class(PdDv_CLASS)) == -1)
	{
		/* open failed */
		DEBUG_0("ucfgdlc: open of PdDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/************************************
	  Get the predefined devices object
	 ************************************/
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);

	rc = (int) odm_get_obj(predev,sstring,&preobj,ODM_FIRST);
	if (rc == 0)
	{
		/* No object */
		DEBUG_0("chgdevice: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc == -1)
	{
		/* failed to get object */
		DEBUG_0("chgdevice: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/**************************************************************
	  If the device has a driver, then need to delete device from
	  driver and call loadext() to unload driver.
	 **************************************************************/
	if (strcmp(preobj.DvDr,"") != 0)
	{
		DEBUG_0( "Device has driver\n")

		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the MUST delete the
		  device even if it reports some other error.
		 ******************************************************/

		/* create devno for this device */
		majorno = genmajor(preobj.DvDr);
		if (majorno == -1)
		{
			DEBUG_0("ucfgdlc: failed to get major number.\n");
			err_exit(E_MAJORNO);
		}
		DEBUG_1("ucfgdlc: Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("ucfgdlc: Calling getminor()\n")
		minor_list = getminor(majorno, &how_many, logical_name);
		if (minor_list == NULL || how_many == 0)
		{
			DEBUG_0("ucfgdlc: failed to get minor number.\n");
			err_exit(E_MINORNO);
		}
		minorno = *minor_list;
		DEBUG_1("ucfgdlc: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno,minorno);
		cfg.kmid = (mid_t)0;
		cfg.ddsptr = (caddr_t) NULL;
		cfg.ddslen = (int)0;
		cfg.cmd = CFG_TERM;

		DEBUG_0("ucfgdlc: Calling sysconfig() to terminate Device\n")
		rc = sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd));
		if (rc == -1)
		{
			if (errno == EBUSY) {
				/* device is in use and can not be unconfigured */
				DEBUG_0("ucfgdlc: Device is busy\n")
			     /*   err_exit(E_BUSY); */
				err_exit(E_SYSCONFIG);
			}
		}

		/************************************************
		  Call loadext() to unload device driver.
		  loadext() will unload the driver only if
		  device is last instance of driver configured.
		 ************************************************/

		DEBUG_1("ucfgdlc: Unloading the driver: %s\n",preobj.DvDr)
		cfg.kmid = loadext(preobj.DvDr, FALSE, FALSE);
		if( cfg.kmid == NULL)
		{
			/* error loading device driver */
			DEBUG_0("ucfgdlc: error unloading driver\n")
			err_exit(E_UNLOADEXT);
		}
		DEBUG_0("ucfgdlc: Unloaded driver\n")

	}

	DEBUG_0("ucfgdlc: Updating device state\n")
	/*************************************************
	  Change the status field of device to "DEFINED"
	 *************************************************/
	cusobj.status = (short)DEFINED;


	if ((rc = odm_change_obj(cusdev,&cusobj)) == -1)
	{
		/* Could not change customized device object */
		DEBUG_0("ucfgdlc: update of CuDv object failed");
		err_exit(E_ODMUPDATE);
	}

	/*************************
	  Close all open classes
	 *************************/
	if ((rc = odm_close_class(cusdev)) == -1)
	{
		/* error closing class */
		DEBUG_0("ucfgdlc: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}
	if ((rc = odm_close_class(predev)) == -1)
	{
		/* error closing class */
		DEBUG_0("ucfgdlc: error closing PdDv object class\n")
		err_exit(E_ODMCLOSE);
	}


	(void) odm_terminate();

	DEBUG_0("ucfgdlc: Successful completion\n")
	exit(0);
}


/*
 * NAME: err_exit
 *
 * FUNCTION: Error exit routine.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This function is the error/exit routine used by the generic
 *      unconfigure method.
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
