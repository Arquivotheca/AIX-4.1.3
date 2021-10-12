static char sccsid[] = "@(#)20  1.5  src/bos/usr/lib/methods/ucfgcommo/ucfgcommo.c, cfgmethods, bos411, 9428A410j 3/16/94 16:50:06";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: err_exit
 *		main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   ucfgcommo.c - Generic Unconfigure Method Code
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
#include <sys/ndd.h>

#include "cfgdebug.h"
#include "cfg_ndd.h"            /* NDD device specific includes */

extern	int errno;		/* System Error Number */
extern  int conferrno;          /* Config (getattr) Error Number */

/* external functions */
extern  int   dev_specific();   /* device specific subroutine hook */


/*
 * NAME: main
 * 
 * FUNCTION: This process is executed to "unconfigure" a device and is
 *           generic to the CDLI devices.
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
 *              ucfgcommo -l <logical_name>
 *
 * RETURNS: Exits with > 0 if failure, 0 on success.
 */

main(argc,argv)
int argc;
char *argv[];
{

	struct cfg_kmod cfg_k;          /* sysconfig command structure */
	char    *logical_name;          /* logical name to unconfigure */
	char	sstring[256];		/* search criteria */
	char	errstring[256];		/* error string */

	struct	CuDv	cusobj;		/* customized devices object */
	struct  CuAt    *cusattobj;     /* customized devices object */
	struct	PdDv	preobj;		/* predefined devices object */
	struct  CuDv    childcusobj;    /* child cust. devices object */

	struct	Class	*cusdev;	/* customized devices class handle */
	struct	Class	*predev;	/* predefined devices class handle */

	char    addl_dvdr[128];         /* name of additional driver */
	int     how_many;               /* storage for getattr command */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	struct ndd_config ndd_cfg_struct; /* dvdr cfg init struc */
	int     inst_num;                /* instance number of device */

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
		DEBUG_0("ucfgcommo: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("ucfgcommo: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1( "Unconfiguring: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("ucfgcommo: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfgcommo: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfgcommo: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfgcommo: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* If device state is DEFINED, then already unconfigured */
	/* so just exit.                                         */
	if (cusobj.status == (short) DEFINED) {
		/* Device is already unconfigured */
		DEBUG_0("ucfgcommo: device is already DEFINED state")
		err_exit(E_OK);
	}

	/* See if this device has AVAILABLE children.  If it does, */
	/* it can not be unconfigured.                             */
	sprintf (sstring,"parent = '%s' AND status != %d",logical_name,
			       DEFINED);
	rc = (int)odm_get_first(cusdev,sstring,&childcusobj);
	if (rc == -1) {
		/* ODM error */
		DEBUG_0("ucfgcommo: ODM error looking for children\n")
		err_exit(E_ODMGET);
	}
	else if (rc != 0) {
		/* Device has children in configured state */
		DEBUG_0("ucfgcommo: Device has configured children\n")
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
	  If the device has a driver, then need to delete device from
	  driver and call loadext() to unload driver.
	 **************************************************************/
	if (strcmp(preobj.DvDr,"") != 0) {
		DEBUG_0( "Device has driver\n")

		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the driver MUST delete
		  the device even if it reports some other error.
		 ******************************************************/

		/************************************************
		  Call loadext() to query kernel module ID
		 ************************************************/
		DEBUG_1("ucfgcommo: Querying the driver: %s\n",preobj.DvDr)
		cfg_k.kmid = loadext(preobj.DvDr,FALSE,TRUE);
		if (cfg_k.kmid == NULL) {
			/* error querying device driver */
			DEBUG_0("ucfgcommo: error querying driver\n")
			err_exit(E_UNLOADEXT);
		}
		DEBUG_1("ucfgcommo: kmid of driver = %x\n",cfg_k.kmid)

		/* get instance number for this device */
		inst_num = lsinst(logical_name);
		if (inst_num == -1) {
		    DEBUG_0("cfgcommo: error getting instance number\n")
		    err_exit(E_INSTNUM);
		}
		DEBUG_1("cfgcommo: instance number = %d\n",inst_num);

		ndd_cfg_struct.seq_number = inst_num;

		cfg_k.mdiptr = (caddr_t) &ndd_cfg_struct;
		cfg_k.mdilen = sizeof (struct ndd_config);
		cfg_k.cmd = CFG_TERM;

		DEBUG_0("ucfgcommo: Calling sysconfig() to terminate Device\n")
		if (sysconfig(SYS_CFGKMOD,&cfg_k,sizeof(struct cfg_kmod)) == -1) {
			if (errno == EBUSY) {
				/* device is in use and can not be unconfigured */
				DEBUG_0("ucfgcommo: Device is busy\n")
				err_exit(E_BUSY);
			}
			/* Ignore other errno values because device driver */
			/* has to complete CFG_TERM operation except when  */
			/* device is busy. */
		}

		/************************************************
		  Call loadext() to unload device driver.
		  loadext() will unload the driver only if 
		  device is last instance of driver configured.
		 ************************************************/
		DEBUG_1("ucfgcommo: Unloading the driver: %s\n",preobj.DvDr)
		cfg_k.kmid = loadext(preobj.DvDr,FALSE,FALSE);
		if (cfg_k.kmid == NULL) {
			/* error unloading device driver */
			DEBUG_0("ucfgcommo: error unloading driver\n")
			err_exit(E_UNLOADEXT);
		}

		DEBUG_0("ucfgcommo: Unloaded driver, checking additional driver\n")

		/* check for additional driver */
		/* if present, remove it also */

		/**************************************************************
		  If the device has an additional driver, then need to delete
		  device from system and call loadext() to unload driver.
		 **************************************************************/
		strcpy ( addl_dvdr, NULL );      /* initialize values */
		/* get additional driver attribute from database */
		cusattobj = getattr( logical_name, "addl_dvdr", FALSE, &how_many );
		if (cusattobj != NULL) {
			strcpy( addl_dvdr, cusattobj->value );
		}
		else {
			/* NOTE: error to be indicated here           */
			/*       only if attribute is there but could */
			/*       not be read                          */
			if ( conferrno != E_NOPdOBJ ) {
				/* error getting addl dvdr name */
				DEBUG_0("ucfgcommo: error getting addl dvdr name\n")
				err_exit(E_ODMGET);
			}
		}

		if (strcmp(addl_dvdr,"") != 0) {
		    DEBUG_0( "Device has an additional driver\n")

		    /* call specified method with parameter "ADDL_UCFG"
		     * to unconfigure the additional driver
		     */

		    sprintf( sstring,
			    " %d %s %d ",
			    ADDL_UCFG, logical_name, inst_num);
		    DEBUG_2("ucfgcommo: calling %s %s\n",addl_dvdr, sstring)

		    if(odm_run_method(addl_dvdr,sstring,NULL,NULL)){
			    fprintf(stderr,"cfgcommo: can't run %s\n",
				    addl_dvdr);
			    err_exit(E_ODMRUNMETHOD);
		    }

		}  /* end if (device has an additional driver) then ... */

	}  /* end if (device has a driver) then ... */

	DEBUG_0("ucfgcommo: Updating device state\n")
	/*************************************************
	  Change the status field of device to "DEFINED"
	 *************************************************/
	cusobj.status = (short)DEFINED;

	if (odm_change_obj(cusdev,&cusobj) == -1) {
		/* Could not change customized device object */
		DEBUG_0("ucfgcommo: update of CuDv object failed");
		err_exit(E_ODMUPDATE);
	}

	/*
	 * Run device specific subroutine
	 */
	rc = dev_specific(logical_name);
	if ( rc > 0 ) {
	    /* error occured */
	    DEBUG_0("ucfgcommo: error in calling device specific routine\n")
	    err_exit(rc);
	}

	/* close the CuDv object class */
	if (odm_close_class(CuDv_CLASS) == -1) {
		/* Error closing CuDv object class */
		DEBUG_0("ucfgcommo: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();

	DEBUG_0("ucfgcommo: Successful completion\n")
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
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_terminate();
	exit(exit_code);
}
