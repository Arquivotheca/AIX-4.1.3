static char sccsid[] = "@(#)61  1.2  src/bos/usr/lib/methods/ucfgdiag/ucfgdiag.c, cfgmethods, bos411, 9428A410j 2/14/94 16:12:58";
/*
 * COMPONENT_NAME: (CFGMETHODS) Diagnose Unconfigure Method.
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
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
 * FUNCTION: This process is executed to change a device's state from
 *	     DIAGNOSE to DEFINED.   
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This process is invoked when a device instance is to be 
 *	taken out of the DIAGNOSE state and put into the DEFINED state.
 *
 * NOTES: Interface:
 *              ucfgdiag -l <logical_name>
 *
 * RETURNS: Exits with > 0 if failure, 0 on success.
 */

main(argc,argv)
int argc;
char *argv[];
{

	char    *logical_name;          /* logical name to unconfigure */
	char	sstring[256];		/* search criteria */
	char	errstring[256];		/* error string */

	struct	CuDv	cusobj;		/* customized devices object */
	struct	PdDv	preobj;		/* predefined devices object */

	struct	Class	*cusdev;	/* customized devices class handle */
	struct	Class	*predev;	/* predefined devices class handle */

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
		DEBUG_0("ucfgdiag: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("ucfgdiag: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1( "ucfgdiag:  Unconfiguring: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("ucfgdiag: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ucfgdiag:  ODM initialized\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfgdiag: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfgdiag: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfgdiag: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* If device state is DEFINED, then already unconfigured 	*/
	/* so just exit.  Otherwise, state must be Diagnose.	        */
	if (cusobj.status == (short) DEFINED) {
		/* Device is already unconfigured */
		DEBUG_0("ucfgdiag: device is already in DEFINED state\n");
		err_exit(E_OK);
	}
	else if ( cusobj.status != DIAGNOSE )
		err_exit(E_DEVSTATE); 


	DEBUG_0("ucfgdiag:  Updating device state\n");

	/*************************************************
	  Change the status field of device to "DEFINED"
	 *************************************************/
	cusobj.status = (short)DEFINED;

	if (odm_change_obj(cusdev,&cusobj) == -1) {
		/* Could not change customized device object */
		DEBUG_0("ucfgdiag: update of CuDv object failed");
		err_exit(E_ODMUPDATE);
	}

	/* close the CuDv object class */
	if (odm_close_class(CuDv_CLASS) == -1) {
		/* Error closing CuDv object class */
		DEBUG_0("ucfgdiag: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();

	DEBUG_0("ucfgdiag: Successful completion\n")
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





