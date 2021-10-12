static char sccsid[] = "@(#)97  1.13  src/bos/usr/lib/methods/cfgdlc/cfgdlc.c, dlccfg, bos411, 9428A410j 10/19/93 09:38:12";
/*
 * COMPONENT_NAME: (DLCCFG) DLC Configuration
 *
 * FUNCTIONS: main(), err_exit(), err_undo1(), err_undo2(), err_undo3()
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

/* interface:
   cfgdlc -l <logical_name> [-<1|2>]
*/


/*****									*/
/***** Device Config Method						*/
/*****									*/

/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include <sys/errno.h>

/* Local header files */
#include "cfgdebug.h"


/* external functions */
extern int      pparse();
extern void     *malloc();
extern long     genmajor();
extern long     generate_minor();
extern mid_t    loadext();

extern int              odm_initialize();
extern int              odm_terminate();
extern struct Class    *odm_open_class();
extern int              odm_close_class();
extern void            *odm_get_obj();
extern int              odm_change_obj();


/* main function code */
main(argc,argv,envp)
int     argc;
char    *argv[];
char    *envp[];
{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	struct  cfg_dd  cfg;           /* sysconfig command structure */

	char    *logical_name;          /* logical name to configure */
	char    sstring[MAX_ODMI_CRIT];     /* search criteria pointer */
	char    vpd[VPDSIZE];           /* vpd data */

	struct Class *cusdev;           /* customized devices class ptr */
	struct Class *predev;           /* predefined devices class ptr */
	struct Class *precon;           /* predefined connection ptr */
	struct Class *cusvpd;           /* customized vpd class ptr */

	struct CuDv cusobj;             /* customized device object storage */
	struct PdDv preobj;             /* predefined device object storage */
	struct CuVPD vpdobj;            /* customized vpd object */

	int     majorno;                /* major number assigned to device */
	int     minorno;                /* minor number assigned to device */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */

	int     ipl_phase;              /* ipl phase: 0=run,1=phase1,2=phase2 */
	int     rc;                     /* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */
	

	/* set up initial variable values */
	ipl_phase = RUNTIME_CFG;
	errflg = 0;
	logical_name = NULL;



	while ((c = getopt(argc,argv,"l:12")) != EOF) {
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
		DEBUG_0("cfgdlc: command line error\n");
		exit(E_ARGS);
	}

	/*************************************************************
	  check command-line parameters for validity
	  For config methods this includes making sure that a logical
	  name was specified, and that both phase flags were not set.
	  The customized devices object class must also be searched
	  to see if the logical name is valid.
	  *************************************************************/


	if (logical_name == NULL)
	{
		DEBUG_0("cfgdlc: logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1 ("Configuring device: %s\n",logical_name)

	/* start up odm */
	rc = odm_initialize();
	if (rc == -1)
	{
		/* init failed */
		DEBUG_0("cfgdlc: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}

	/* lock the database */
	if ((rc = odm_lock("/etc/objrepos/config_lock",0)) == -1)
	{
		DEBUG_0("cfgdlc: odm_lock() failed\n")
		err_exit(E_ODMLOCK);
	}

	/* open customized devices object class (CusDevices) */
	cusdev = odm_open_class(CuDv_CLASS);
	if ((int) cusdev == -1)
	{
		/* open failed */
		DEBUG_0("cfgdlc: open class CuDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* search CusDevices for customized object with this logical name */
	sprintf(sstring,"name = '%s'",logical_name);
	if ((rc = (int)odm_get_obj(cusdev, sstring, &cusobj, ODM_FIRST)) ==0)
	{
		DEBUG_1("cfgdlc: failed to find CuDv object for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc == -1)
	{
		/* odm error occurred */
		DEBUG_0("cfgdlc: ODM failure getting CuDv object");
		err_exit(E_ODMGET);
	}


	/*******************************************************************
	  Get the predefined object for this device. This object is located
	  searching the predefined devices object class based on the unique
	  type link descriptor in the customized device.
	  *******************************************************************/
	/* open customized devices object class (CusDevices) */
	predev = odm_open_class(PdDv_CLASS);
	if (predev == -1)
	{
		/* open failed */
		DEBUG_0("cfgdlc: open class PdDv failed\n");
		err_exit(E_ODMOPEN);
	}


	/* search Predefined devices for object with this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	if ((rc = (int)odm_get_obj(predev, sstring, &preobj, ODM_FIRST)) == 0)
	{
		/* odm objects not found */
		DEBUG_0("cfgdlc: failed to find PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc == -1)
	{
		/* odm objects not found */
		DEBUG_0("cfgdlc: ODM failure getting PdDv object");
		err_exit(E_ODMGET);
	}


	/* close object class */
	if ((rc=odm_close_class(predev)) == -1)
	{
		/* error closing object class */
		DEBUG_0("cfgdlc: close object class PdDv failed");
		err_exit(E_ODMCLOSE);
	}
	/****************************************************************
	  If this device is being configured during an ipl phase, then
	  display this device's LED value on the system LEDs.
	  ****************************************************************/

	if (ipl_phase != RUNTIME_CFG)
		/* display LED value */
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


		/* there are no parents associated with this device */

		/* NOTE: build dds moved to allow for passing of devno */

		/**********************************************************
		  Check to see if the device has a device driver. If it
		  does, then call osconfig(). If needed, this function
		  will load the driver and return the driver's kernel
		  module id (kmid). If the device has a driver, then the
		  device driver instance field in the device's customized
		  object will have a key into the predefined device driv-
		  er object class.
		  **********************************************************/



		if (strcmp(preobj.DvDr,"") != 0)
		{


			/***********************************************
			  Call loadext() to load device driver.
			  loadext will load the driver only if needed,
			  and always returns the kmid of the driver.
			  ***********************************************/

			cfg.kmid = loadext(preobj.DvDr,TRUE,FALSE);
			if ((int) cfg.kmid == NULL)
			{
				/* error loading device driver */
				DEBUG_1("cfgdlc: error loading driver %s\n", preobj.DvDr)
				err_exit(E_LOADEXT);
			}


			DEBUG_0("cfgdlc: Calling genmajor()\n")
			if ((majorno=genmajor(preobj.DvDr)) == -1)
			{
			    /* error allocating major number */
				DEBUG_0("cfgdlc: error generating major number");
			    err_undo1(preobj.DvDr);
			    err_exit(E_MAJORNO);
			}
			DEBUG_1("cfgdlc: Returned major number: %d\n",majorno)

			/* get minor number      */
			DEBUG_0("cfgdlc: Calling getminor()\n")
			minor_list = getminor(majorno, &how_many,
					logical_name);
			if (minor_list == NULL || how_many == 0)
			{
				rc = generate_minor(logical_name,
						majorno, &minorno);
				DEBUG_0("cfgdlc: Calling generate_minor()\n")
				if (rc)
				{
					DEBUG_1("cfgdlc: error generating minor number, rc=%d\n",rc)
					/* First make sure any minors that might have */
					/* been assigned are cleaned up */
					reldevno(logical_name, TRUE);
					err_undo1(preobj.DvDr);
					if ( rc < 0 || rc > 255)
						rc = E_MINORNO;
					err_exit(rc);
				}
				DEBUG_0("cfgdlc: Returned from generate_minor()\n")
			}
			else minorno = *minor_list;
			DEBUG_1("cfgdlc: minor number: %d\n",minorno)

			/* create devno for this device */
			cfg.devno = makedev(majorno,minorno);

			/****************************************************
			  Now make the special files that this device will
			  be accessed through.
			  ****************************************************/

			DEBUG_0("cfgdlc: Calling make_special_files()\n")
			rc = make_special_files(logical_name, cfg.devno);
			if (rc)
			{
			       /* error making special files */
				DEBUG_1("cfgdlc: error making special file(s), rc=%d\n",rc)
				err_undo1(preobj.DvDr);
				if ( rc < 0 || rc > 255)
					rc = E_MKSPECIAL;
				err_exit(rc);
			}
			DEBUG_0("cfgdlc: Returned from make_special_files()\n")


			/* build DDS structure */
			DEBUG_0("cfgdlc: Calling build_dds()\n")
			rc = build_dds(logical_name,&cfg.ddsptr,&cfg.ddslen,cfg.devno);

			if (rc)
			{
				/* error building dds */
				err_undo1(preobj.DvDr);
				DEBUG_1("cfgdlc: error building dds, rc=%d\n",rc)
				if ( rc < 0 || rc > 255)
					rc = E_DDS;
				err_exit(rc);
			}
			DEBUG_0("cfgdlc: Returned from build_dds()\n")

			/*************************************************
			  Now call sysconfig() to configure the driver.
			  *************************************************/
			DEBUG_0("cfgdlc: Pass DDS to driver via sysconfig()\n")

			/* cfg.kmid set by loadext */
			/* cfg.ddsptr set by build_dds */
			/* cfg_ddslen set by build_dds */
			cfg.cmd = CFG_INIT;

			rc =sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd ));
			if (rc == -1)
			{
				DEBUG_0("cfgdlc: error configuring device\n")
				err_undo1(preobj.DvDr);
				err_exit(E_CFGINIT);

			}
		  /* there won't be microcode */

		} /* end if (device has a driver) then ... */

		/**********************************************************
		  If the device has VPD data that can be extracted, then
		  get it and store it in the customized object for this
		  device. The predefined object for this device says if
		  the device has VPD or not.
		  **********************************************************/


		if (preobj.has_vpd == TRUE)
		{

			vpd[0] = '\0';
			/* get VPD for this device */
			DEBUG_0("cfgdlc: Calling query_vpd()\n")

			rc = query_vpd(&cusobj, cfg.kmid, cfg.devno, vpd);
			if (rc)
			{
				/* failed to get VPD */
				DEBUG_1("cfgdlc: error getting VPD, rc=%d\n",rc)
				err_undo3(preobj.DvDr, cfg.devno);
				if ( rc < 0 || rc > 255)
					rc = E_VPD;
				err_exit(rc);
			}
			DEBUG_0("cfgdlc: Returned from query_vpd()\n")

			/* open customized vpd object class */
			if ((cusvpd = odm_open_class(CuVPD_CLASS)) == -1)
			{
				DEBUG_0("cfgdlc: open class CuVPD failed");
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMOPEN);
			}

			/* search for customized vpd object with this logical name */
			sprintf(sstring, "name = '%s' and vpd_type = '%d'",
			       logical_name, USER_VPD);
			rc = (int)odm_get_obj(cusvpd,sstring,&vpdobj,ODM_FIRST);
			if (rc==-1) {
				/* failed to get object */
				DEBUG_0("cfgdlc: ODM failure getting CuVPD object");
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMGET);
			}
			if (rc==0) {
				/* need to add vpd object */
				DEBUG_0("Adding new VPD object\n");
				strcpy(vpdobj.name,logical_name);
				vpdobj.vpd_type = USER_VPD;
				strcpy(vpdobj.vpd,vpd);
				if (odm_add_obj(cusvpd,&vpdobj) < 0)
				{
					DEBUG_0("cfgdlc: ODM failure adding CuVPD object")
					err_undo3(preobj.DvDr, cfg.devno);
					err_exit(E_ODMADD);
				}
				DEBUG_0("Successfully added new VPD object\n");
			}
			else
			{
				/* see if vpd object needs to be updated */
				if (strcmp(vpdobj.vpd,vpd))
				{
					DEBUG_0("Updating VPD object\n");
					strcpy(vpdobj.vpd,vpd);
					if (odm_change_obj(cusvpd,&vpdobj) < 0)
					{
						DEBUG_0("cfgdlc: ODM failure updating CuVPD object")
						err_undo3(preobj.DvDr, cfg.devno);
						err_exit(E_ODMUPDATE);
					}
					DEBUG_0("Successfully updated VPD object\n");
				}
			}
			/* close customized vpd object class */
			if (odm_close_class(CuVPD_CLASS) < 0)
			{
				DEBUG_0("cfgdlc: error closing CuVPD object class\n");
				err_undo3(preobj.DvDr, cfg.devno);
				err_exit(E_ODMCLOSE);
			}
		}

		/**********************************************************
		  Update the customized object to reflect the device's
		  current status. The device status field should be
		  changed to AVAILABLE, and, if they were generated, the
		  device's major & minor numbers should be updated, as
		  well as any VPD data that was retrieved.
		  **********************************************************/


		/* update customized device object with a change operation */
		cusobj.status = AVAILABLE;


		if ((rc = odm_change_obj(cusdev, &cusobj)) < 0)
		{
			DEBUG_0("cfgdlc: ODM failure updating CuDv object\n");
			err_exit(E_ODMUPDATE);
		}

	} /* end if (device is not AVAILABLE) then ... */

	/* close object class */
	if ((rc = odm_close_class(cusdev)) < 0)
	{
		/* error closing class */
		DEBUG_0("cfgdlc: error closing CuDv object class\n");
		err_exit(E_ODMCLOSE);
	}

	/* there are no children associated with this device */

	/***********************************************************
	  config method is finished at this point. Terminate the
	  ODM, and exit with a good return code.
	  ***********************************************************/

	rc = odm_terminate();
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
	if (loadext(DvDr,FALSE,FALSE) == NULL)
	{
		DEBUG_0("cfgdlc: error unloading driver\n");
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
		DEBUG_0("cfgdlc: error unconfiguring device\n");
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




