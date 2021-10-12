static char sccsid[] = "@(#)81  1.1  src/bos/usr/lib/methods/common/cfgddcom.c, cfgmethods, bos411, 9428A410j 10/26/93 16:37:16";
/*
 * COMPONENT_NAME: CFGMETHODS
 *
 * FUNCTIONS: main
 *            err_exit
 *            err_undo1
 *            err_undo2
 *            err_undo3
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   cfgddcom.c - Generic Diagnostic Configure Method Code
 *
 *      This method is used for configuring and unconfiguring the
 *      COMIO device drivers for use in diagnostics.
 *
 *
 *   interface:
 *      cfgddXXX -l <logical_name> -f <cmd>
 *
 *              <logical_name> -> device to be diagnosed
 *              <cmd> -> to configure (CFG) or unconfigure (UCFG)
 *
 */


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

/* Local header files */
#include "cfgdebug.h"
#include "cfg_diag.h"

/* external functions */
extern long	genmajor();
extern  int errno;              /* System Error Number */

/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	int    cmd_flag;                /* command to CFG or UCFG */

	struct cfg_dd cfg;		/* sysconfig command structure */

	char	*logical_name;		/* logical name to configure */
	char	sstring[256];		/* search criteria pointer */
	char	conflist[1024];		/* busresolve() configured devices */
	char	not_resolved[1024];	/* busresolve() not resolved devices */
	char    vpd[VPDSIZE];           /* vpd data */
	char    dvdr_name[128];         /* name of diagnostic driver */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *predev;		/* predefined devices class ptr */
	struct Class *cusvpd;           /* customized vpd class ptr */

	struct CuDv cusobj;		/* customized device object storage */
	struct PdDv preobj;		/* predefined device object storage */
	struct CuDv parobj;		/* customized device object storage */
	struct CuDv dmyobj;		/* customized device object storage */
	struct CuVPD vpdobj;            /* customized vpd object */

	int     majorno;                /* major number assigned to device */
	int     minorno;                /* minor number assigned to device */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */

	ushort	devid;			/* Device id - used at run-time */
	int	slot;			/* slot of adapters */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	errflg = 0;
	logical_name = NULL;
	cmd_flag = -1;

	while ((c = getopt(argc,argv,"l:f:")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case 'f':
			if (cmd_flag != -1)
				errflg++;
			cmd_flag = atoi(optarg);
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("cfgddcom: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("cfgddcom: logical name must be specified\n");
		exit(E_LNAME);
	}

	/* command must be specified */
	if (cmd_flag == -1) {
		DEBUG_0("cfgddcom: command must be specified\n");
		exit(E_ARGS);
	}

	DEBUG_1 ("cfgddcom: Configuring device: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("cfgddcom: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("cfgddcom: ODM initialized\n")

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		DEBUG_0("cfgddcom: open class CuDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* No CuDv object with this name */
		DEBUG_1("cfgddcom: failed to find CuDv object for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgddcom: ODM failure getting CuDv object\n");
		err_exit(E_ODMGET);
	}

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		DEBUG_0("cfgddcom: open class PdDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(predev, sstring, &preobj);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("cfgddcom: failed to find PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgddcom: ODM failure getting PdDv object\n");
		err_exit(E_ODMGET);
	}

	/* close predefined device object class */
	if (odm_close_class(predev) == -1) {
		DEBUG_0("cfgddcom: close object class PdDv failed\n");
		err_exit(E_ODMCLOSE);
	}

	/******************************************************************
	  Check to make sure this device is in the DIAGNOSE state.
	  ******************************************************************/

	DEBUG_1("cfgddcom: check device state of %d\n",cusobj.status);
	if (cusobj.status == DIAGNOSE)
	{

	    /*******************************************************
	     * OK, continue.  The diagnostic driver can only be
	     * configured if device to be tested is in the DIAGNOSE
	     * state.  This is the locking mechanism.
	     *********************************************************/

	    switch (cmd_flag) {
	      case CFG:
		  /***************************************************
		   need to load driver, build its DDS and pass to driver.
		   Then, a device dependent routine is called for
		   downloading microcode.
		  ***************************************************/
		/***************************************************
		  If device has a device driver, then need to load driver,
		  get major number, and call device dependent routines to
		  get minor number, make special files, and build DDS.
		  This code then passes the DDS to the driver.  Finally,
		  a device dependent routine is called for downloading
		  microcode.
		 ***************************************************/
		/* get name of device driver to load */
		rc = get_dvdr(&dvdr_name);
		if (rc != 0) {
		    /* error getting device driver name */
		    err_exit(E_LOADEXT);
		}
		DEBUG_1("cfgddcom: name of driver to load is %s\n", dvdr_name)

		/* call loadext to load the device driver */
		if ((cfg.kmid = loadext(dvdr_name, TRUE, FALSE))
		     == NULL)
		{
		    /* error loading device driver */
		    DEBUG_1("cfgddcom: error loading driver %s\n", dvdr_name)
		    err_exit(E_LOADEXT);
		}

		/* get major number      */
		DEBUG_0("cfgddcom: Calling genmajor()\n")
		if ((majorno = genmajor(preobj.DvDr)) == -1)
		{
		    DEBUG_0("cfgddcom: error generating major number\n");
		    err_undo1(dvdr_name);
		    err_exit(E_MAJORNO);
		}
		DEBUG_1("cfgddcom: Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("cfgddcom: Calling getminor()\n")
		minor_list = getminor(majorno,&how_many,logical_name);
		if (minor_list == NULL || how_many == 0)
		{
		    DEBUG_0("cfgddcom: Calling generate_minor()\n")
		    rc = generate_minor(logical_name, majorno, &minorno);
		    if (rc)
		    {
			DEBUG_1("cfgddcom: error generating minor number, rc=%d\n",rc)
			/* First make sure any minors that might */
			/* have been assigned are cleaned up */
			reldevno(logical_name, TRUE);
			err_undo1(dvdr_name);
			if ( rc < 0 || rc > 255)
			    rc = E_MINORNO;
			err_exit(rc);
		    }
		    DEBUG_0("cfgddcom: Returned from generate_minor()\n")
		}
		else
		    minorno = *minor_list;
		DEBUG_1("cfgddcom: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno, minorno);

		/* make special files      */
		DEBUG_0("cfgddcom: Calling make_special_files()\n")
		rc = make_special_files(logical_name, cfg.devno);
		if (rc) {
			/* error making special files */
			DEBUG_1("cfgddcom: error making special file(s), rc=%d\n",rc)
			err_undo1(dvdr_name);
			if ( rc < 0 || rc > 255)
				rc = E_MKSPECIAL;
			err_exit(rc);
		}
		DEBUG_0("cfgddcom: Returned from make_special_files()\n")

		/* build the DDS  */
		DEBUG_0("cfgddcom: Calling build_dds()\n")
		rc = build_dds(logical_name, &cfg.ddsptr, &cfg.ddslen);
		if (rc) {
			/* error building dds */
			DEBUG_1("cfgddcom: error building dds, rc=%d\n",rc)
			err_undo1(dvdr_name);
			if ( rc < 0 || rc > 255)
				rc = E_DDS;
			err_exit(rc);
		}
		DEBUG_0("cfgddcom: Returned from build_dds()\n")

		/* call sysconfig to pass DDS to driver */
		DEBUG_0("cfgddcom: Pass DDS to driver via sysconfig()\n")
		cfg.cmd = CFG_INIT;
		if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
			/* error configuring device */
			DEBUG_1("cfgddcom: error configuring device, errno =%d\n",errno)
			err_undo1(dvdr_name);
			err_exit(E_CFGINIT);
		}

		/* download microcode if necessary */
		DEBUG_0("cfgddcom: Calling download_microcode()\n")
		rc = download_microcode(logical_name);
		if (rc) {
			/* error downloading microcode */
			DEBUG_1("cfgddcom: error downloading microcode, rc=%d\n",rc)
			err_undo2(cfg.devno);
			err_undo1(dvdr_name);
			if ( rc < 0 || rc > 255)
				rc = E_UCODE;
			err_exit(rc);
		}
		DEBUG_0("cfgddcom: Returned from download_microcode()\n")
		/* if device has VPD data then go get it */
		if (preobj.has_vpd == TRUE) {
			/* get VPD for this device */
			memset( vpd, 0, sizeof(vpd) );
			DEBUG_0("cfgddcom: Calling query_vpd()\n")
			rc = query_vpd(&cusobj, cfg.kmid, cfg.devno, vpd);
			if (rc) {
				/* failed to get VPD */
				DEBUG_1("cfgddcom: error getting VPD, rc=%d\n",rc)
				err_undo3(dvdr_name, cfg.devno);
				if ( rc < 0 || rc > 255)
					rc = E_VPD;
				err_exit(rc);
			}
			DEBUG_0("cfgddcom: Returned from query_vpd()\n")

			/* open customized vpd object class */
			if ((int)(cusvpd = odm_open_class(CuVPD_CLASS)) == -1) {
				DEBUG_0("cfgddcom: open class CuVPD failed");
				err_undo3(dvdr_name, cfg.devno);
				err_exit(E_ODMOPEN);
			}

			/* search for customized vpd object with this logical name */
			sprintf(sstring, "name = '%s' and vpd_type = '%d'",
				logical_name,HW_VPD);
			rc = (int)odm_get_first(cusvpd,sstring,&vpdobj);
			if (rc==-1) {
				/* ODM failure */
				DEBUG_0("cfgddcom: ODM failure getting CuVPD object");
				err_undo3(dvdr_name, cfg.devno);
				err_exit(E_ODMGET);
			}
			if (rc==0) {
				/* need to add vpd object */
				DEBUG_0("cfgddcom: Adding new VPD object\n");
				strcpy(vpdobj.name,logical_name);
				vpdobj.vpd_type = HW_VPD;
				memcpy(vpdobj.vpd,vpd,sizeof(vpd));
				if (odm_add_obj(cusvpd,&vpdobj) == -1) {
					DEBUG_0("cfgddcom: ODM failure adding CuVPD object")
					err_undo3(dvdr_name, cfg.devno);
					err_exit(E_ODMADD);
				}
				DEBUG_0("cfgddcom: Successfully added new VPD object\n");
			} else {
				/* see if vpd object needs to be updated */
				if (memcmp(vpdobj.vpd,vpd,sizeof(vpd))) {
					DEBUG_0("cfgddcom: Updating VPD object\n");
					memcpy(vpdobj.vpd,vpd,sizeof(vpd));
					if (odm_change_obj(cusvpd,&vpdobj) == -1) {
						DEBUG_0("cfgddcom: ODM failure updating CuVPD object")
						err_undo3(dvdr_name, cfg.devno);
						err_exit(E_ODMUPDATE);
					}
					DEBUG_0("cfgddcom: Successfully updated VPD object\n");
				}
			}
			/* close customized vpd object class */
			if (odm_close_class(CuVPD_CLASS) == -1) {
				DEBUG_0("cfgddcom: error closing CuVPD object class\n");
				err_undo3(dvdr_name, cfg.devno);
				err_exit(E_ODMCLOSE);
			}
		}

		break;
	      case UCFG:
		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the driver MUST delete
		  the device even if it reports some other error.
		 ******************************************************/
		/* get name of device driver to unload */
		rc = get_dvdr(&dvdr_name);
		if (rc != 0) {
		    /* error getting device driver name */
		    err_exit(E_UNLOADEXT);
		}
		DEBUG_1("cfgddcom: name of driver to load is %s\n", dvdr_name)

		/* first, need to create devno for this device */
		majorno = genmajor(preobj.DvDr);
		if (majorno == -1) {
			DEBUG_0("cfgddcom: failed to get major number.\n");
			err_exit(E_MAJORNO);
		}
		DEBUG_1("cfgddcom: Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("cfgddcom: Calling getminor()\n")
		minor_list = getminor(majorno, &how_many, logical_name);
		if (minor_list == NULL || how_many == 0) {
			DEBUG_0("cfgddcom: failed to get minor number.\n");
			err_exit(E_MINORNO);
		}
		minorno = *minor_list;
		DEBUG_1("cfgddcom: minor number: %d\n",minorno)

		/* create devno for this device */
		cfg.devno = makedev(majorno,minorno);
		cfg.kmid = (mid_t)0;
		cfg.ddsptr = (caddr_t) NULL;
		cfg.ddslen = (int)0;
		cfg.cmd = CFG_TERM;

		DEBUG_0("cfgddcom: Calling sysconfig() to terminate Device\n")
		if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
			if (errno == EBUSY) {
				/* device is in use and can not be unconfigured */
				DEBUG_0("cfgddcom: Device is busy\n")
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
		DEBUG_1("cfgddcom: Unloading the driver: %s\n",dvdr_name)
		cfg.kmid = loadext(dvdr_name,FALSE,FALSE);
		if (cfg.kmid == NULL) {
			/* error unloading device driver */
			DEBUG_0("cfgddcom: error unloading driver\n")
			err_exit(E_UNLOADEXT);
		}
		DEBUG_0("cfgddcom: Unloaded driver\n")
		break;
	      default:
		/* Error, command must be to configure or unconfigure */
		err_exit(E_ARGS);
		break;
	    }

	}
	else {

	    /*******************************************************
	     * Error, do not continue.  The device to be tested must
	     * be in the DIAGNOSE state.  This is the locking mechanism.
	     ********************************************************/
	    DEBUG_0("cfgddcom: Device not in DIAGNOSE state\n");
	    err_exit(E_DEVSTATE);
	}

	/* close customized device object class */
	if (odm_close_class(cusdev) == -1) {
		DEBUG_0("cfgddcom: error closing CuDv object class\n");
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();
	exit(0);

}

/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char    exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}

/*
 * NAME: err_undo1
 *
 * FUNCTION: Unloads the device's device driver.  Used to back out on an
 *           error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_undo1( DvDr )
 *      DvDr = pointer to device driver name.
 *
 * RETURNS:
 *               None
 */

err_undo1(DvDr)
char    *DvDr;                  /* pointer to device driver name */
{
	/* unload driver */
	if (loadext(DvDr,FALSE,FALSE) == NULL) {
		DEBUG_0("cfgddcom: error unloading driver\n");
	}
	return;
}

/*
 * NAME: err_undo2
 *
 * FUNCTION: Terminates the device.  Used to back out on an error.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_undo2( devno )
 *      devno = The device's devno.
 *
 * RETURNS:
 *               None
 */

err_undo2(devno)
dev_t   devno;                  /* The device's devno */
{
	struct  cfg_dd cfg;             /* sysconfig command structure */

	/* terminate device */
	cfg.devno = devno;
	cfg.kmid = (mid_t)0;
	cfg.ddsptr = (caddr_t) NULL;
	cfg.ddslen = (int)0;
	cfg.cmd = CFG_TERM;

	if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
		DEBUG_1("cfgddcom: error unconfiguring device, errno = %d\n",errno);
	}
	return;
}

/*
 * NAME: err_undo3
 *
 * FUNCTION: Terminates the device and unloads the driver, if the device has
 *           a driver.  If the device does not have a driver, it simply
 *           returns.  This routine is used to back out on errors that
 *           occur while processing VPD.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.  The device
 *      specific routines for the various device specific config methods
 *      must not call this function.
 *
 * NOTES:
 *
 * void
 *   err_undo3( DvDr , devno )
 *      DvDr  = pointer to device driver name.
 *      devno = The device's devno.
 *
 * RETURNS:
 *               None
 */

err_undo3(DvDr, devno)
char    *DvDr;                  /* pointer to device driver name */
dev_t   devno;                  /* the device's devno */
{
	odm_close_class(CuVPD_CLASS);   /* make sure CuVPD is closed */

	if (strcmp(DvDr, "") != 0) {    /* If device has a driver then ... */
		err_undo2(devno);       /* terminate device */
		err_undo1(DvDr);        /* unload device driver */
	}
	return;
}
