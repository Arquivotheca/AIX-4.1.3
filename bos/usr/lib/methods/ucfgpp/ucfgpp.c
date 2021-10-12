static char sccsid[] = "@(#)26	1.1  src/bos/usr/lib/methods/ucfgpp/ucfgpp.c, cfgmethods, bos411, 9428A410j 5/3/94 17:49:13";
/*
 * COMPONENT_NAME: (SYSXPRNT) Parallel Printer Unconfigure Method.
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
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

#include "cfgdebug.h"

extern	int errno;		/* System Error Number */


/*
 * NAME: main
 * 
 * FUNCTION: This process is executed to "unconfigure" a printer device.
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
 *              ucfgpp -l <logical_name>
 *
 * The generic unconfigure could not be used because we want to terminate the
 * driver here (but not unload it); however, since it is our parent that has the
 * DvDr predefine, the generic ucfg would skip the terminate.
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
	struct  CuDv	parobj;         /* parent's customized device object */
	struct	PdDv	ppreobj;	/* parent's predefined device object */
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
		DEBUG_0("ucfgpp: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("ucfgpp: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1( "Unconfiguring: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("ucfgpp: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfgpp: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* open predefined device object class */
	if ((int)(predev=odm_open_class(PdDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfgpp: open of PdDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfgpp: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfgpp: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* If device state is DEFINED, then already unconfigured */
	/* so just exit.                                         */
	if (cusobj.status == (short) DEFINED) {
		/* Device is already unconfigured */
		DEBUG_0("ucfgpp: device is already DEFINED state")
		err_exit(E_OK);
	}

	/* get device's predefined object */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfgpp: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfgpp: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/* Get the parent's device driver so we can terminate it. */

	/* get the device's parent object */
	sprintf(sstring, "name = '%s'", cusobj.parent);
	rc = (int)odm_get_first(cusdev,sstring,&parobj);
	if (rc==0) 
	{
		/* Parent device not in CuDv */
		DEBUG_0("ucfgpp: no parent CuDv object\n");
		err_exit(E_NOCuDvPARENT);
	}
	else if (rc==-1) 
	{
		/* ODM failure */
		DEBUG_0("ucfgpp: ODM failure getting parent CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* Get the parent's Predefined Devices Object */
	sprintf(sstring, "uniquetype = '%s'", parobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(predev,sstring, &ppreobj);
	switch (rc) {
	  case 0: /* odm objects not found */
	    DEBUG_1("cfgpp: PdDv %s found no objects.\n",
		     sstring);
	    err_exit(E_NOPdOBJ);
	    break;  /* To avoid bug if err_exit is suppressed */
	    
	  case -1 : /* odm error occurred */
	    DEBUG_1("cfgpp: get_obj failed, %s.\n",
		     sstring);
	    err_exit(E_ODMGET);
	    break;  /* To avoid bug if err_exit is suppressed */
	    
	  default: /* odm objects found ==> That's OK */
	    DEBUG_1("cfgpp: PdDv %s found.\n",
		     sstring);
	}

	if (strcmp(ppreobj.DvDr,"") != 0) {
		DEBUG_1( "Device has driver: %s\n", ppreobj.DvDr)

		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the driver MUST delete
		  the device even if it reports some other error.
		 ******************************************************/

		/* first, need to create devno for this device */
		majorno = genmajor(ppreobj.DvDr);
		if (majorno == -1) {
			DEBUG_0("ucfgpp: failed to get major number.\n");
			err_exit(E_MAJORNO);
		}
		DEBUG_1("ucfgpp: Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("ucfgpp: Calling getminor()\n")
		minor_list = getminor(majorno, &how_many, logical_name);
		if (minor_list == NULL || how_many == 0) {
			DEBUG_0("ucfgpp: failed to get minor number.\n");
			err_exit(E_MINORNO);
		}
		minorno = *minor_list;
		DEBUG_1("ucfgpp: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno,minorno);
		cfg.kmid = (mid_t)0;
		cfg.ddsptr = (caddr_t) NULL;
		cfg.ddslen = (int)0;
		cfg.cmd = CFG_TERM;	

		DEBUG_0("ucfgpp: Calling sysconfig() to terminate Device\n")
		if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
			if (errno == EBUSY) {
				/* device is in use and can not be unconfigured */
				DEBUG_0("ucfgpp: Device is busy\n")
				err_exit(E_BUSY);
			}
			/* Ignore other errno values because device driver */
			/* has to complete CFG_TERM operation except when  */
			/* device is busy. */
		}
	} else {	/* Something's wrong - the parent has no DvDr */
		DEBUG_0("ucfgpp: parent has no device driver\n");
		err_exit(E_PARENT);
	}

	DEBUG_0("ucfgpp: Updating device state\n")
	/*************************************************
	  Change the status field of device to "DEFINED"
	 *************************************************/
	cusobj.status = (short)DEFINED;

	if (odm_change_obj(cusdev,&cusobj) == -1) {
		/* Could not change customized device object */
		DEBUG_0("ucfgpp: update of CuDv object failed");
		err_exit(E_ODMUPDATE);
	}

	/* close the PdDv object class */
	if (odm_close_class(PdDv_CLASS) == -1) {
		/* Error closing PdDv object class */
		DEBUG_0("ucfgpp: error closing PdDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	/* close the CuDv object class */
	if (odm_close_class(CuDv_CLASS) == -1) {
		/* Error closing CuDv object class */
		DEBUG_0("ucfgpp: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();

	DEBUG_0("ucfgpp: Successful completion\n")
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
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuDv_CLASS);
	odm_terminate();
	exit(exit_code);
}
