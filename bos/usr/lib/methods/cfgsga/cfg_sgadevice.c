static char sccsid[] = "@(#)55  1.1  src/bos/usr/lib/methods/cfgsga/cfg_sgadevice.c, sgadd, bos411, 9428A410j 11/3/93 13:45:10";
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
 *
 * FUNCTIONS: main(), err_exit(), err_undo1(), err_undo2(), err_undo3()
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

/* interface:
   cfgsga -l sga0 [-<1|2>] [-D]
*/

/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

/* Local header files */
#include "cfgdebug.h"

/* external functions */
extern long	genmajor();



/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	struct cfg_dd cfg;		/* sysconfig command structure */

	char	*logical_name;		/* logical name to configure */
	char	sstring[256];		/* search criteria pointer */
	char	conflist[1024];		/* busresolve() configured devices */
	char	not_resolved[1024];	/* busresolve() not resolved devices */
	char    vpd[VPDSIZE];           /* vpd data */

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
	int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
	int	slot;			/* slot of adapters */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	ipl_phase = RUNTIME_CFG;
	errflg = 0;
	logical_name = NULL;

	while ((c = getopt(argc,argv,"l:12D")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case '1':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE1;
			break;
		case '2':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE2;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("cfg_sgadevice: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("cfg_sgadevice: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1 ("Configuring device: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("cfg_sgadevice: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	DEBUG_0 ("cfg_sgadevice: calling odm_open_class on CuDv\n")
	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		DEBUG_0("cfg_sgadevice: open class CuDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	DEBUG_1("cfg_sgadevice: searching CuDv object for %s\n", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* No CuDv object with this name */
		DEBUG_1("cfg_sgadevice: failed to find CuDv object for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfg_sgadevice: ODM failure getting CuDv object");
		err_exit(E_ODMGET);
	}

	/* open predefined devices object class */
	DEBUG_0("cfg_sgadevice: open class PdDv\n");
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		DEBUG_0("cfg_sgadevice: open class PdDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	DEBUG_1("cfg_sgadevice: search PdDv object for this device %s\n",sstring);
	rc = (int)odm_get_first(predev, sstring, &preobj);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("cfg_sgadevice: failed to find PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfg_sgadevice: ODM failure getting PdDv object");
		err_exit(E_ODMGET);
	}

	/* close predefined device object class */
	if (odm_close_class(predev) == -1) {
		DEBUG_0("cfg_sgadevice: close object class PdDv failed");
		err_exit(E_ODMCLOSE);
	}

	/****************************************************************
	  If this device is being configured during an ipl phase, then
	  display this device's LED value on the system LEDs.
	  ****************************************************************/
	if (ipl_phase != RUNTIME_CFG)
		setleds(preobj.led);

	/******************************************************************
	  Check to see if the device is already configured (AVAILABLE).
	  We actually go about the business of configuring the device
	  only if the device is not configured yet. Configuring the
	  device in this case refers to the process of checking parent
	  and sibling status, checking for attribute consistency, build-
	  ing a DDS, loading the driver, etc...
	  ******************************************************************/

	if (cusobj.status == DEFINED) 
	{
		/*******************************************************
		  The device is not available to the system yet. Now
		  check to make sure that the device's relations will
		  allow it to be configured. In particular, make sure
		  that the parent is configured (AVAILABLE), and that
		  no other devices are configured at the same location.
		  *******************************************************/

		/* get the device's parent object */
		sprintf(sstring, "name = '%s'", cusobj.parent);
                DEBUG_1("cfg_sgadevice: get parent obj %s\n",sstring);
		rc = (int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0) 
		{
			/* Parent device not in CuDv */
			DEBUG_0("cfg_sgadevice: no parent CuDv object\n");
			err_exit(E_NOCuDvPARENT);
		}
		else if (rc==-1) 
		{
			/* ODM failure */
			DEBUG_0("cfg_sgadevice: ODM failure getting parent CuDv object\n")
			err_exit(E_ODMGET);
		}

		if (parobj.status != AVAILABLE) 
		{
			DEBUG_0("cfg_sgadevice: parent is not AVAILABLE")
			err_exit(E_PARENTSTATE);
		}

		/* make sure that no other devices are configured     */
		/* at this location                                   */
		sprintf(sstring, "parent = '%s' AND connwhere = '%s' AND status = %d",
			cusobj.parent, cusobj.connwhere, AVAILABLE);
                DEBUG_1("cfg_sgadevice: check for other devices configured with %s\n",sstring);
		rc = (int)odm_get_first(cusdev,sstring,&dmyobj);
		if (rc == -1) 
		{
			/* odm failure */
			err_exit(E_ODMGET);
		} 
		else if (rc) 
		{
			/* Error: device config'd at this location */
			DEBUG_0("cfg_sgadevice: device already AVAILABLE at this connection\n")
			err_exit(E_AVAILCONNECT);
		}


		/***************************************************
		  Need to load sga device driver,
		  get major number, and call device dependent routines to
		  get minor number, make special files, and build DDS.
		  This code then passes the DDS to the driver.  Finally,
		  a device dependent routine is called for downloading
		  microcode.
		 ***************************************************/
		if (strcmp(preobj.DvDr, "") != 0) 
                {
			/* call loadext to load the device driver */
                        DEBUG_1("cfg_sgadevice: loading %s\n",preobj.DvDr);
			if ((cfg.kmid = loadext(preobj.DvDr, TRUE, FALSE)) 
                             == NULL) 
                        {
			    /* error loading device driver */
			    DEBUG_1("cfg_sgadevice: error loading driver %s\n", preobj.DvDr)
			    err_exit(E_LOADEXT);
			}

			/* get major number      */
			DEBUG_0("cfg_sgadevice: Calling genmajor()\n")
			if ((majorno = genmajor(preobj.DvDr)) == -1) 
			{
			    DEBUG_0("cfg_sgadevice: error generating major number");
			    err_undo1(preobj.DvDr);
			    err_exit(E_MAJORNO);
			}
			DEBUG_1("cfg_sgadevice: Returned major number: %d\n",majorno)

			/* get minor number      */
			DEBUG_0("cfg_sgadevice: Calling getminor()\n")
			minor_list = getminor(majorno,&how_many,logical_name);
			if (minor_list == NULL || how_many == 0) 
			{
			    DEBUG_0("cfg_sgadevice: Calling generate_minor()\n")
			    rc = generate_minor(logical_name, majorno, &minorno);
			    if (rc) 
			    {
				DEBUG_1("cfg_sgadevice: error generating minor number, rc=%d\n",rc)
				/* First make sure any minors that might */
				/* have been assigned are cleaned up */
				reldevno(logical_name, TRUE);
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
				    rc = E_MINORNO;
				err_exit(rc);
			    }
			    DEBUG_0("cfg_sgadevice: Returned from generate_minor()\n")
			}
			else
			    minorno = *minor_list;
			DEBUG_1("cfg_sgadevice: minor number: %d\n",minorno)

			/* create devno for this device */
			cfg.devno = makedev(majorno, minorno);

			/* build the DDS  */
			DEBUG_0("cfg_sgadevice: Calling build_dds()\n")
			rc = build_dds(logical_name, &cfg.ddsptr, &cfg.ddslen);
			if (rc) {
				/* error building dds */
				DEBUG_1("cfg_sgadevice: error building dds, rc=%d\n",rc)
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_DDS;
				err_exit(rc);
			}
			DEBUG_0("cfg_sgadevice: Returned from build_dds()\n")

			/* call sysconfig to pass DDS to driver */
			DEBUG_0("cfg_sgadevice: Pass DDS to driver via sysconfig()\n")
			cfg.cmd = CFG_INIT;
			if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
				/* error configuring device */
				DEBUG_0("cfg_sgadevice: error configuring device\n")
				err_undo1(preobj.DvDr);
				err_exit(E_CFGINIT);
			}

		} /* end if (device has a driver) then ... */
                else { /* total bummer dude....*/
                       err_exit( E_NOATTR );
                      }
 
		/* if device has VPD data then go get it */
		if (preobj.has_vpd == TRUE) {
			/* get VPD for this device */
			memset( vpd, 0, sizeof(vpd) );
			DEBUG_0("cfg_sgadevice: Calling query_vpd()\n")
			rc = query_vpd(&cusobj, cfg.kmid, cfg.devno, vpd);
			if (rc) {
				/* failed to get VPD */
				DEBUG_1("cfg_sgadevice: error getting VPD, rc=%d\n",rc)
				err_undo3(preobj.DvDr, cfg.devno);
				if ( rc < 0 || rc > 255)
					rc = E_VPD;
				err_exit(rc);
			}
			DEBUG_0("cfg_sgadevice: Returned from query_vpd()\n")

			/* open customized vpd object class */
			if ((int)(cusvpd = odm_open_class(CuVPD_CLASS)) == -1) {
				DEBUG_0("cfg_sgadevice: open class CuVPD failed");
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMOPEN);
			}

			/* search for customized vpd object with this logical name */
			sprintf(sstring, "name = '%s' and vpd_type = '%d'",
				logical_name,HW_VPD);
			rc = (int)odm_get_first(cusvpd,sstring,&vpdobj);
			if (rc==-1) {
				/* ODM failure */
				DEBUG_0("cfg_sgadevice: ODM failure getting CuVPD object");
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMGET);
			}
			if (rc==0) {
				/* need to add vpd object */
				DEBUG_0("Adding new VPD object\n");
				strcpy(vpdobj.name,logical_name);
				vpdobj.vpd_type = HW_VPD;
				memcpy(vpdobj.vpd,vpd,sizeof(vpd));
				if (odm_add_obj(cusvpd,&vpdobj) == -1) {
					DEBUG_0("cfg_sgadevice: ODM failure adding CuVPD object")
					err_undo3(preobj.DvDr, cfg.devno);
					err_exit(E_ODMADD);
				}
				DEBUG_0("Successfully added new VPD object\n");
			} else {
				/* see if vpd object needs to be updated */
				if (memcmp(vpdobj.vpd,vpd,sizeof(vpd))) {
					DEBUG_0("Updating VPD object\n");
					memcpy(vpdobj.vpd,vpd,sizeof(vpd));
					if (odm_change_obj(cusvpd,&vpdobj) == -1) {
						DEBUG_0("cfg_sgadevice: ODM failure updating CuVPD object")
						err_undo3(preobj.DvDr, cfg.devno);
						err_exit(E_ODMUPDATE);
					}
					DEBUG_0("Successfully updated VPD object\n");
				}
			}
			/* close customized vpd object class */
			if (odm_close_class(CuVPD_CLASS) == -1) {
				DEBUG_0("cfg_sgadevice: error closing CuVPD object class\n");
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMCLOSE);
			}
		}
	       /* 
		* Update customized device object.  Set current 
		* status to AVAILABLE and reset change status to
		* SAME only if it was MISSING
		*/

		cusobj.status = AVAILABLE;
		if (cusobj.chgstatus == MISSING)
		{
		    cusobj.chgstatus = SAME ;
		}
		if (odm_change_obj(cusdev, &cusobj) == -1) 
		{
		    /* ODM failure */
		    DEBUG_0("cfg_sgadevice: ODM failure updating CuDv object\n");
		    err_exit(E_ODMUPDATE);
		}
	} /* end if (device is not AVAILABLE) then ... */

	/* close customized device object class */
	if (odm_close_class(cusdev) == -1) {
		DEBUG_0("cfg_sgadevice: error closing CuDv object class\n");
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
		DEBUG_0("cfg_sgadevice: error unloading driver\n");
	}
	return;
}

/*
 * NAME: err_undo2
 *
 * FUNCTION: Terminates the device.  Used to back out on an error.
 *
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
		DEBUG_0("cfg_sgadevice: error unconfiguring device\n");
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
