static char sccsid[] = "@(#)68	1.1  src/bos/usr/lib/methods/ucfglft/ucfglft.c, lftdd, bos411, 9428A410j 10/25/93 16:56:55";
/*
 * COMPONENT_NAME: (LFTDD)	LFT configuration routine
 *
 * FUNCTIONS:
 *
 *	main, err_exit
 *
 * ORIGIN: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
#include <lftras.h>
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
 *              ucfglft -l <logical_name>
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
	struct	Class	*predev;	/* predefined devices class handle */

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
		DEBUG_0("ucfglft: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("ucfglft: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1( "ucfglft: Unconfiguring: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("ucfglft: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ucfglft: ODM initialized\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfglft: open of CuDv failed\n")
		err_exit(E_ODMOPEN,"odm_open_class",UCFG_OPEN_CUDV,UNIQUE_1);
	}

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfglft: Device does not exist\n")
		err_exit(E_NOCuDv,"odm_get_first",UCFG_NO_CUDV,UNIQUE_2);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfglft: ODM error getting CuDv object\n")
		err_exit(E_ODMGET,"odm_get_first",UCFG_BAD_CUDV,UNIQUE_3);
	}

	/* If device state is DEFINED, then already unconfigured */
	/* so just exit.                                         */
	if (cusobj.status == (short) DEFINED) {
		/* Device is already unconfigured */
		DEBUG_0("ucfglft: device is already DEFINED state")
		err_exit(E_OK,NULL,0,0);
	}


	/* get device's predefined object */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfglft: No PdDv object\n")
		err_exit(E_NOPdDv,"odm_get_first",UCFG_NO_PDDV,UNIQUE_4);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfglft: ODM error getting PdDv object\n")
		err_exit(E_ODMGET,"odm_get_first",UCFG_BAD_PDDV,UNIQUE_5);
	}

	/**************************************************************
	  If the device has a driver, then need to delete device from
	  driver and call loadext() to unload driver.
	 **************************************************************/
	if (strcmp(preobj.DvDr,"") != 0) {
		DEBUG_0( "ucfglft: Device has driver\n")

		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the driver MUST delete
		  the device even if it reports some other error.
		 ******************************************************/

		/* first, need to create devno for this device */
		majorno = genmajor(preobj.DvDr);
		if (majorno == -1) {
			DEBUG_0("ucfglft: failed to get major number.\n");
			err_exit(E_MAJORNO,"genmajor",UCFG_NO_MAJOR,UNIQUE_6);
		}
		DEBUG_1("ucfglft: Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("ucfglft: Calling getminor()\n")
		minor_list = getminor(majorno, &how_many, logical_name);
		if (minor_list == NULL || how_many == 0) {
			DEBUG_0("ucfglft: failed to get minor number.\n");
			err_exit(E_MINORNO,"getminor",UCFG_NO_MINOR,UNIQUE_7);
		}
		minorno = *minor_list;
		DEBUG_1("ucfglft: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno,minorno);
		cfg.kmid = (mid_t)0;
		cfg.ddsptr = (caddr_t) NULL;
		cfg.ddslen = (int)0;
		cfg.cmd = CFG_TERM;	

		DEBUG_0("ucfglft: Calling sysconfig() to terminate Device\n")
		if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
			if (errno == EBUSY) {
				/* device is in use and can't be unconfigured */
				DEBUG_0("ucfglft: Device is busy\n")
				err_exit(E_BUSY,NULL,0,0);
			}
			/* Ignore other errno values because device driver */
			/* has to complete CFG_TERM operation except when  */
			/* device is busy. */
		}

		/* ******************************************* */
		/* Suppress the 'autopush' of modules for this */
		/* line */
		/* ******************************************* */
		if (rc = pop_modules(majorno, minorno)) {
			DEBUG_2("ucfglft: Unable to suppress the 'autopush' for maj %ld min %ld\n",
				majorno, minorno);
			err_exit(rc,NULL,0,0);
		};

		/************************************************
		  Call loadext() to unload device driver.
		  loadext() will unload the driver only if 
		  device is last instance of driver configured.
		 ************************************************/
		DEBUG_1("ucfglft: Unloading the driver: %s\n",preobj.DvDr)
		cfg.kmid = loadext(preobj.DvDr,FALSE,FALSE);
		if (cfg.kmid == NULL) {
			/* error unloading device driver */
			DEBUG_0("ucfglft: error unloading driver\n")
			err_exit(E_UNLOADEXT,"loadext",UCFG_UNLOAD,UNIQUE_8);
		}
		DEBUG_0("ucfglft: Unloaded driver\n")
	}

	DEBUG_0("ucfglft: Updating device state\n")
	/*************************************************
	  Change the status field of device to "DEFINED"
	 *************************************************/
	cusobj.status = (short)DEFINED;

	if (odm_change_obj(cusdev,&cusobj) == -1) {
		/* Could not change customized device object */
		DEBUG_0("ucfglft: update of CuDv object failed");
		err_exit(E_ODMUPDATE,"odm_change_obj",UCFG_CHNG_CUDV,UNIQUE_9);
	}

	/* close the CuDv object class */
	if (odm_close_class(CuDv_CLASS) == -1) {
		/* Error closing CuDv object class */
		DEBUG_0("ucfglft: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE,"odm_close_class",UCFG_ODMCLOSE,UNIQUE_10);
	}

	odm_terminate();

	DEBUG_0("ucfglft: Successful completion\n")
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

err_exit(exit_code,bad_func,ras_err,unique_err)
int     exit_code;              /* Error exit code */
char    *bad_func;		/* ptr to failing function name */
int	ras_err;		/* RAS error code */
int	unique_err;		/* unique error number */
{
	odm_close_class(CuDv_CLASS);
	odm_terminate();
	if( ras_err )
	{
		cfg_lfterr(NULL,"UCFGLFT","ucfglft",bad_func,exit_code,
				ras_err, unique_err);
	}
	exit(exit_code);
}
