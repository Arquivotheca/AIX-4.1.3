static char sccsid[] = "@(#)07  1.19.1.4  src/bos/usr/lib/methods/common/cfgdevice.c, cfgmethods, bos412, 9446B 10/28/94 10:45:59";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfgdevice.c - Generic Config Method Code
 *
 * FUNCTIONS: main(), err_exit(), err_undo1(), err_undo2(), err_undo3()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/* interface:
   cfgXXX -l <logical_name> [-<1|2>] [-D]
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

/* global variables */
int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
int	Dflag;			/* flag for defining children */

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

	struct CuAt cuattr;		/* customized device attribute      */
					/* storage 			    */
	struct CuDv cusobj;		/* customized device object storage */
	struct PdAt pdattr;		/* customized device attribute      */
					/* storage 			    */
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
	int	led_no;			/* Displayable LED Value	*/

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	ipl_phase = RUNTIME_CFG;
	errflg = 0;
	logical_name = NULL;
	Dflag = FALSE;

	while ((c = getopt(argc,argv,"l:12D")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case 'D':
			if (Dflag != FALSE)
				errflg++;
			Dflag = TRUE;
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
		DEBUG_0("cfgdevice: command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("cfgdevice: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1 ("Configuring device: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("cfgdevice: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		DEBUG_0("cfgdevice: open class CuDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* No CuDv object with this name */
		DEBUG_1("cfgdevice: failed to find CuDv object for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdevice: ODM failure getting CuDv object");
		err_exit(E_ODMGET);
	}

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		DEBUG_0("cfgdevice: open class PdDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(predev, sstring, &preobj);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("cfgdevice: failed to find PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("cfgdevice: ODM failure getting PdDv object");
		err_exit(E_ODMGET);
	}

	/* close predefined device object class */
	if (odm_close_class(predev) == -1) {
		DEBUG_0("cfgdevice: close object class PdDv failed");
		err_exit(E_ODMCLOSE);
	}

	/****************************************************************
	  If this device is being configured during an ipl phase, then
	  display this device's LED value on the system LEDs.
	  ****************************************************************/
	if (ipl_phase != RUNTIME_CFG) {
		led_no = (int)preobj.led;
		if(led_no == 0) {
			/* No LED number in PdDv so check for type Z */
			/* attribute 				     */

			sprintf(sstring,
			   "name=%s AND type=Z AND attribute=led",
			    cusobj.name);

			rc = (int)odm_get_first(CuAt_CLASS,sstring,&cuattr);
			if(rc != 0 && rc != -1)
				led_no = (int)strtoul(cuattr.value,NULL,0);


			/* if PdDv class of led_no is STILL 0, look for */
			/* PdAt attr 					*/

			if(led_no == 0) {
				sprintf(sstring,
			       "uniquetype=%s AND type=Z AND attribute=led",
				preobj.uniquetype);

				rc = (int)odm_get_first(PdAt_CLASS,sstring,
							&pdattr);
				if(rc != 0 && rc != -1)
					led_no = (int)strtoul(pdattr.deflt,
							     NULL,0);
			}

		
		} 
		setleds(led_no);
	}
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
		rc = (int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0) 
		{
			/* Parent device not in CuDv */
			DEBUG_0("cfgdevice: no parent CuDv object\n");
			err_exit(E_NOCuDvPARENT);
		}
		else if (rc==-1) 
		{
			/* ODM failure */
			DEBUG_0("cfgdevice: ODM failure getting parent CuDv object\n")
			err_exit(E_ODMGET);
		}

		if (parobj.status != AVAILABLE) 
		{
			DEBUG_0("cfgdevice: parent is not AVAILABLE")
			err_exit(E_PARENTSTATE);
		}

		/* make sure that no other devices are configured     */
		/* at this location                                   */
		sprintf(sstring, "parent = '%s' AND connwhere = '%s' AND status = %d",
			cusobj.parent, cusobj.connwhere, AVAILABLE);
		rc = (int)odm_get_first(cusdev,sstring,&dmyobj);
		if (rc == -1) 
		{
			/* odm failure */
			err_exit(E_ODMGET);
		} 
		else if (rc) 
		{
			/* Error: device config'd at this location */
			DEBUG_0("cfgdevice: device already AVAILABLE at this connection\n")
			err_exit(E_AVAILCONNECT);
		}

		/***************************************************
		  If the device is an adapter being configured at
		  RUN TIME, then we must resolve any bus attribute
		  conflicts before configuring device to the driver.
		 ***************************************************/
		if (ipl_phase == RUNTIME_CFG) 
		{
			if (!strcmp(preobj.subclass,"mca")) {
				/* Make sure card is in specified slot */
				slot = atoi(cusobj.connwhere);
				devid = (ushort) strtol(preobj.devid,(char **) NULL,0);
				sprintf (sstring,"/dev/%s",cusobj.parent);
				rc = chkslot(sstring,slot,devid);
				if (rc != 0) {
					DEBUG_2("cfgdevice: card %s not found in slot %d\n",
						logical_name,slot);
					err_exit(rc);
				}
			}
		       /*
			* Get the bus that this devices is connected to.
			*/
			rc = Get_Parent_Bus(cusdev, cusobj.parent, &dmyobj) ;
			if (rc == 0)
			{
			    /* Invoke Bus Resolve  */
			    rc = busresolve(logical_name,(int)0,conflist,
				not_resolved, dmyobj.name);
			    if (rc != 0) 
			    {
				DEBUG_0("cfgdevice: bus resources could not be resolved\n")
				err_exit(rc);
			    }
			}
			else if (rc == E_PARENT)
			{
			    DEBUG_0("cfgdevice: device not on bus\n")
			}
			else 
			{
			    DEBUG_0("cfgdevice: ODM err getting parent bus\n")
                            err_exit(rc) ;
			}
		}

		/***************************************************
		  If device has a device driver, then need to load driver,
		  get major number, and call device dependent routines to
		  get minor number, make special files, and build DDS.
		  This code then passes the DDS to the driver.  Finally,
		  a device dependent routine is called for downloading
		  microcode.
		 ***************************************************/
		if (strcmp(preobj.DvDr, "") != 0) 
                {
			/* call loadext to load the device driver */
			if ((cfg.kmid = loadext(preobj.DvDr, TRUE, FALSE)) 
                             == NULL) 
                        {
			    /* error loading device driver */
			    DEBUG_1("cfgdevice: error loading driver %s\n", preobj.DvDr)
			    err_exit(E_LOADEXT);
			}

			/* get major number      */
			DEBUG_0("cfgdevice: Calling genmajor()\n")
			if ((majorno = genmajor(preobj.DvDr)) == -1) 
			{
			    DEBUG_0("cfgdevice: error generating major number");
			    err_undo1(preobj.DvDr);
			    err_exit(E_MAJORNO);
			}
			DEBUG_1("cfgdevice: Returned major number: %d\n",majorno)

			/* get minor number      */
			DEBUG_0("cfgdevice: Calling getminor()\n")
			minor_list = getminor(majorno,&how_many,logical_name);
			if (minor_list == NULL || how_many == 0) 
			{
			    DEBUG_0("cfgdevice: Calling generate_minor()\n")
			    rc = generate_minor(logical_name, majorno, &minorno);
			    if (rc) 
			    {
				DEBUG_1("cfgdevice: error generating minor number, rc=%d\n",rc)
				/* First make sure any minors that might */
				/* have been assigned are cleaned up */
				reldevno(logical_name, TRUE);
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
				    rc = E_MINORNO;
				err_exit(rc);
			    }
			    DEBUG_0("cfgdevice: Returned from generate_minor()\n")
			}
			else
			    minorno = *minor_list;
			DEBUG_1("cfgdevice: minor number: %d\n",minorno)

			/* create devno for this device */
			cfg.devno = makedev(majorno, minorno);

			/* make special files      */
			DEBUG_0("cfgdevice: Calling make_special_files()\n")
			rc = make_special_files(logical_name, cfg.devno);
			if (rc) {
				/* error making special files */
				DEBUG_1("cfgdevice: error making special file(s), rc=%d\n",rc)
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_MKSPECIAL;
				err_exit(rc);
			}
			DEBUG_0("cfgdevice: Returned from make_special_files()\n")

			/* build the DDS  */
			DEBUG_0("cfgdevice: Calling build_dds()\n")
			rc = build_dds(logical_name, &cfg.ddsptr, &cfg.ddslen);
			if (rc) {
				/* error building dds */
				DEBUG_1("cfgdevice: error building dds, rc=%d\n",rc)
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_DDS;
				err_exit(rc);
			}
			DEBUG_0("cfgdevice: Returned from build_dds()\n")

			/* call sysconfig to pass DDS to driver */
			DEBUG_0("cfgdevice: Pass DDS to driver via sysconfig()\n")
			cfg.cmd = CFG_INIT;
			if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
				/* error configuring device */
				DEBUG_0("cfgdevice: error configuring device\n")
				err_undo1(preobj.DvDr);
				err_exit(E_CFGINIT);
			}

			/* download microcode if necessary */
			DEBUG_0("cfgdevice: Calling download_microcode()\n")
			rc = download_microcode(logical_name);
			if (rc) {
				/* error downloading microcode */
				DEBUG_1("cfgdevice: error downloading microcode, rc=%d\n",rc)
				err_undo2(cfg.devno);
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_UCODE;
				err_exit(rc);
			}
			DEBUG_0("cfgdevice: Returned from download_microcode()\n")

		} /* end if (device has a driver) then ... */

		/* if device has VPD data then go get it */
		if (preobj.has_vpd == TRUE) {
			/* get VPD for this device */
			memset( vpd, 0, sizeof(vpd) );
			DEBUG_0("cfgdevice: Calling query_vpd()\n")
			rc = query_vpd(&cusobj, cfg.kmid, cfg.devno, vpd);
			if (rc) {
				/* failed to get VPD */
				DEBUG_1("cfgdevice: error getting VPD, rc=%d\n",rc)
				err_undo3(preobj.DvDr, cfg.devno);
				if ( rc < 0 || rc > 255)
					rc = E_VPD;
				err_exit(rc);
			}
			DEBUG_0("cfgdevice: Returned from query_vpd()\n")

			/* open customized vpd object class */
			if ((int)(cusvpd = odm_open_class(CuVPD_CLASS)) == -1) {
				DEBUG_0("cfgdevice: open class CuVPD failed");
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMOPEN);
			}

			/* search for customized vpd object with this logical name */
			sprintf(sstring, "name = '%s' and vpd_type = '%d'",
				logical_name,HW_VPD);
			rc = (int)odm_get_first(cusvpd,sstring,&vpdobj);
			if (rc==-1) {
				/* ODM failure */
				DEBUG_0("cfgdevice: ODM failure getting CuVPD object");
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
					DEBUG_0("cfgdevice: ODM failure adding CuVPD object")
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
						DEBUG_0("cfgdevice: ODM failure updating CuVPD object")
						err_undo3(preobj.DvDr, cfg.devno);
						err_exit(E_ODMUPDATE);
					}
					DEBUG_0("Successfully updated VPD object\n");
				}
			}
			/* close customized vpd object class */
			if (odm_close_class(CuVPD_CLASS) == -1) {
				DEBUG_0("cfgdevice: error closing CuVPD object class\n");
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
		    DEBUG_0("cfgdevice: ODM failure updating CuDv object\n");
		    err_exit(E_ODMUPDATE);
		}
	} /* end if (device is not AVAILABLE) then ... */

	/* call device specific routine to detect/manage child devices */
	DEBUG_0("cfgdevice: Calling define_children()\n")
	if (define_children(logical_name, ipl_phase) != 0) {
		/* error defining children */
		DEBUG_0("cfgdevice: error defining children\n");
		err_exit(E_FINDCHILD);
	}
	DEBUG_0("cfgdevice: Returned from define_children()\n")

	/* close customized device object class */
	if (odm_close_class(cusdev) == -1) {
		DEBUG_0("cfgdevice: error closing CuDv object class\n");
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
	struct   CuAt  sys_cuattr;
	int rc;

	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);

#ifdef _CFG_RDS
	rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
				 &sys_cuattr);
	if (rc == -1) {
		DEBUG_0("ODM error getting rds_facility attribute\n")
		return(E_ODMGET);
	} else if (rc != 0 && sys_cuattr.value[0]=='y') {
		rds_switch_off_device();
	}
#endif

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
		DEBUG_0("cfgdevice: error unloading driver\n");
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
		DEBUG_0("cfgdevice: error unconfiguring device\n");
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
