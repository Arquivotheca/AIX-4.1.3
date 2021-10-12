/* static char sccsid[] = "@(#)14       1.2  src/bos/usr/lib/methods/ucfggraphics/ucfggraphics.c, dispcfg, bos411, 9428A410j 4/11/94 17:36:10"; */
/*
 * COMPONENT_NAME: (DISPCFG) Generic Functions for Display Configuration
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
 * FUNCTION: This process is executed to "unconfigure" a graphics device.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This process is invoked when a device instance is to be 
 *	"unconfigured". This involves terminating the device to the 
 *	driver and representing the unconfigured state of the device 
 *	in the database. It may also unload the appropriate device driver 
 *	if the device is the last configured device for the driver.
 *      In the case of CCM which is loaded for each adapter, ccmdd
 *      will always be unloaded. The case as to whether CCM is to be
 *      unloaded or the real driver is made by examining the ODM attriubute
 *      ccmdd_kmid. If a valid value is retrieved then it is known that
 *      CCM was loaded in place of the real driver.
 *
 *
 * NOTES: Interface:
 *              ucfggraphics -l <logical_name>
 *
 * RETURNS: Exits with > 0 if failure, 0 on success.
 */

main(argc,argv)
int argc;
char *argv[];
{

	struct	cfg_dd cfg;		/* sysconfig command structure */
        struct  cfg_load cfgload;       /* used to unload driver       */
	char    *logical_name;          /* logical name to unconfigure */
	char	sstring[256];		/* search criteria */
	char	errstring[256];		/* error string */

	struct	CuDv	cusobj;		/* customized devices object */
	struct	PdDv	preobj;		/* predefined devices object */
	struct  CuDv    childcusobj;    /* child cust. devices object */

	struct	Class	*cusdev;	/* customized devices class handle */
	struct	Class	*predev;	/* predefined devices class handle */
        struct  CuAt    *p_CuAt;        /* customized attribute object ptr */

	long	majorno,minorno;	/* major and minor numbers */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */
        int     unloaded_ccm;           /* flag set if ccmdd unloaded */
	extern  int optind;             /* flag parsed by getopt() */
	extern  char *optarg;           /* argument parsed by getopt() */

char dd_name[50];
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
		DEBUG_0("ucfggraphics: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("ucfggraphics: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1( "ucfggraphics: unconfiguring: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("ucfggraphics: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ucfggraphics: ODM initialized\n")

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfggraphics: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* open customized attribute object class */
	if ((int)(odm_open_class(CuAt_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("ucfggraphics: open of CuAt failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("ucfggraphics: Device does not exist\n")
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfggraphics: ODM error getting CuDv object\n")
		err_exit(E_ODMGET);
	}

	/* If device state is DEFINED, then already unconfigured */
	/* so just exit.                                         */
	if (cusobj.status == (short) DEFINED) {
		/* Device is already unconfigured */
		DEBUG_0("ucfggraphics: device is already DEFINED state")
		err_exit(E_OK);
	}

	/* See if this device has AVAILABLE children.  If it does, */
	/* it can not be unconfigured.                             */
	sprintf (sstring,"parent = '%s' AND status != %d",logical_name,
			       DEFINED);
	rc = (int)odm_get_first(cusdev,sstring,&childcusobj);
	if (rc == -1) {
		/* ODM error */
		DEBUG_0("ucfggraphics: ODM error looking for children\n")
		err_exit(E_ODMGET);
	}
	else if (rc != 0) {
		/* Device has children in configured state */
		DEBUG_0("ucfggraphics: Device has configured children\n")
		err_exit(E_CHILDSTATE);
	}
	
	DEBUG_0( "ucfggraphics: device has no configured children\n")

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
		DEBUG_0("ucfggraphics: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("ucfggraphics: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/**************************************************************
	  If the device has a driver, then need to delete device from
	  driver and call loadext() to unload driver.
	 **************************************************************/
	if (strcmp(preobj.DvDr,"") != 0) {
		DEBUG_1( "ucfggraphics: unconfiguring %s\n",preobj.DvDr )

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
			DEBUG_0("ucfggraphics: failed to get major number.\n");
			err_exit(E_MAJORNO);
		}
		DEBUG_1("ucfggraphics: Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("ucfggraphics: Calling getminor()\n")
		minor_list = getminor(majorno, &how_many, logical_name);
		if (minor_list == NULL || how_many == 0) {
			DEBUG_0("ucfggraphics: failed to get minor number.\n");
			err_exit(E_MINORNO);
		}
		minorno = *minor_list;
		DEBUG_1("ucfggraphics: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno,minorno);
		cfg.kmid = (mid_t)0;
		cfg.ddsptr = (caddr_t) NULL;
		cfg.ddslen = (int)0;
		cfg.cmd = CFG_TERM;	

		DEBUG_0("ucfggraphics: Calling sysconfig() to terminate Device\n")
		if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
			if (errno == EBUSY) {
				/* device is in use and can not be unconfigured */
				DEBUG_0("ucfggraphics: Device is busy\n")
				err_exit(E_BUSY);
			}
			/* Ignore other errno values because device driver */
			/* has to complete CFG_TERM operation except when  */
			/* device is busy. */
		}

                /*              
                     Check to see if this driver is the "real thing" or is
                     really ccmdd masquerading as "the real thing"...
                     We do this by checking to see if the custom attribute ccmdd_kmid
                     exists and has a valid kmid. If we get a kmid we unload
                     ccmdd. If we don't, then we know we are dealing with the
                     real driver. NOTE: the ccmdd_kmid attribute will not exist
                     for the older drivers not supported by ccm...
                */

                /* get object from ODM */
                DEBUG_1( "ucfggraphics: getting attribute ccmdd_kmid for %s\n",preobj.DvDr )
                unloaded_ccm = FALSE;
                if ( (p_CuAt = getattr( logical_name, "ccmdd_kmid", FALSE, &how_many )) != NULL ) {
                   /* This driver is supported by ccm, check and see if this attribute 
                      has a non-null value. If so, then we are dealing with ccmdd... */
                      DEBUG_1( "ucfggraphics: ccmdd_kmid = p_CuAt->value = %s\n",p_CuAt->value )
                      if ( *(p_CuAt->value) != '\0' ) {
                         /* We need to unload ccmdd since we found a kmid */
                         DEBUG_0("ucfggraphics: Unloading ccmdd\n")
                         /* convert string representation of kmid to binary */
                         if ( (cfgload.kmid = (mid_t)atoi(p_CuAt->value)) == 0 ) {
                              DEBUG_1( "ucfggraphics: atoi failed to convert kmid %s\n", p_CuAt->value )
                              err_exit( E_ATTRVAL );
                              }
                         /* use sysconfig() to unload the ccmdd using the kmid */
                         DEBUG_1( "ucfggraphics: cfgload.kmid = 0x%x\n",cfgload.kmid )
                         if ( sysconfig(SYS_KULOAD, &cfgload, sizeof(struct cfg_load)) == -1 ) {
                            DEBUG_2( "ucfggraphics: SYS_KULOAD failed on ccmdd kmid %u , errno=%d\n",cfgload.kmid,errno )
			    err_exit(E_UNLOADEXT);
                            }
                         else {
                            /* set flag indicating we are dealing with CCM */
                            unloaded_ccm = TRUE;
                            DEBUG_0( "ucfggraphics: successfully unloaded ccmdd\n" )
                            }
                      }

                /* if we didn't unload the ccmdd, it must be we are dealing with the real driver */
                if ( !unloaded_ccm ) {
                     /*
                         It appears we are dealing with "the real driver".
                         Call loadext() to unload device driver.
                         loadext() will unload the driver only if
                         device is last instance of driver configured.
                     */ 
                     DEBUG_1("ucfggraphics: Unloading %s\n",preobj.DvDr)
	    	     strcat( strcpy(dd_name,"/usr/lib/drivers/"),preobj.DvDr);
		     DEBUG_1("ucfggraphics: unloadding %s\n",dd_name)
                     /* cfg.kmid = loadext(preobj.DvDr,FALSE,FALSE); */
                     cfg.kmid = loadext(dd_name,FALSE,FALSE);
		     if (cfg.kmid == NULL) {
			/* error unloading device driver */
			DEBUG_0("ucfggraphics: error unloading driver\n")
			err_exit(E_UNLOADEXT);
                        }
                }

		DEBUG_0("ucfggraphics: successfully unloaded driver\n")
	}

	DEBUG_0("ucfggraphics: Updating device state\n")
	/*************************************************
	  Change the status field of device to "DEFINED"
	 *************************************************/
	cusobj.status = (short)DEFINED;

	if (odm_change_obj(cusdev,&cusobj) == -1) {
		/* Could not change customized device object */
		DEBUG_0("ucfggraphics: update of CuDv object failed");
		err_exit(E_ODMUPDATE);
	}

        /*
           If we unloaded ccmdd, remove its kmid entry from CuAt 
         */
         if ( unloaded_ccm ) {
            /* Set "value" to NULL and write back to ODM */
            /* The effect will be to remove the CuAt entry */
            /* since the default value for the PdAt entry  */
            /* is null.                                    */
            *(p_CuAt->value) = '\0';
            if ( putattr( p_CuAt ) != E_OK ) {
                /* Could not change customized attribute object */
                DEBUG_0("ucfggraphics: update of PdAt object failed");
                err_exit(E_ODMUPDATE);
                }
            }  

	/* close the CuDv object class */
	if (odm_close_class(CuDv_CLASS) == -1) {
		/* Error closing CuDv object class */
		DEBUG_0("ucfggraphics: error closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	/* close the CuAt object class */
	if (odm_close_class(CuAt_CLASS) == -1) {
		/* Error closing CuDv object class */
		DEBUG_0("ucfggraphics: error closing CuAt object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();

	DEBUG_0("ucfggraphics: Successful completion\n")
	exit(0);
        }
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
        odm_close_class(CuAt_CLASS);
	odm_terminate();
	exit(exit_code);
}
