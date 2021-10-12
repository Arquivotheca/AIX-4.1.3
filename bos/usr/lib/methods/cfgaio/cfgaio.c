static char sccsid[] = "@(#)49	1.3  src/bos/usr/lib/methods/cfgaio/cfgaio.c, cfgmethods, bos411, 9428A410j 9/19/91 14:22:15";
/*
 * COMPONENT_NAME: (CFGMETH) Configure Method for aio extension
 *
 * FUNCTIONS: main, err_exit
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <cf.h>
#include <malloc.h>

#include <errno.h>

#include <sys/cfgdb.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>

#include <aio_interface.h>

#include "cfgdebug.h"
#include "pparms.h"

#define DEFAULT_ODMDIR "/etc/objrepos"

char *prog;

/*
 * NAME: main
 * 
 * FUNCTION: This process is executed to "configure" a aio extension .
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This process is invoked when an aio extension is to be 
 *	"configured". 
 *
 * NOTES: Interface:
 *	cfgaio  -l aio0 <phase>
 *
 * RETURNS: Exits with 0 on success, >0 Error code.
 */


main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];
{
	struct cfg_kmod cfg;	       /* Parameter list for SYS_CFGKMOD command*/
	void err_exit();

	char	*logical_name;		/* logical name to configure */
	char	sstring[256];		/* search criteria pointer */
	char	hw_vpd[VPDSIZE];	/* vpd for parent adapter */

	struct  Class 	*cusdev;	/* customized devices class ptr */
	struct	Class	*predev;	/* predefined devices class ptr */
	struct	Class	*cusatt;	/* customized attribute class ptr */
	struct	Class	*preatt;	/* predefined attribute class ptr */
	struct	Class	*cuvpd;		/* customized vpd class ptr */

	struct	CuDv cusobj;		/* customized device object storage */
	struct	PdDv preobj;		/* predefined device object storage */
	struct	PdDv parpdobj;		/* predefined device object storage */
	struct	CuDv parobj;		/* customized device object storage */
	struct	CuDv dmyobj;		/* customized device object storage */
	struct	CuVPD vpdobj;		/* customized vpd object storage */

	struct	CuAt	*cuat;

	int 	ipl_phase = RUNTIME_CFG;	/* ipl phase */
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */
	int     errflg,c, rc;       /* used in parsing parameters   */

	char   *odmdir = getenv("ODMDIR");
	char	odmlockfile[MAXPATHLEN+1] = { '\0' };

	/* set up initial variable values */
	errflg = 0;
	logical_name = NULL;
	
	prog = argv[0];
	
	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	while ((c = getopt(argc,argv,"l:12?")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case '1':
			ipl_phase = PHASE1;
			break;
		case '2':
			ipl_phase = PHASE2;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		fprintf(stderr, "usage: %s -l aio0 [ -(1|2) ]\n", prog);
		exit(E_ARGS);
	}

	/*************************************************************
	  check command-line parameters for validity
	  The customized devices object class must also be searched
	  to see if the logical name is valid.
	 *************************************************************/
	/* start up odm */
	if (odm_initialize() < 0) {
		fprintf(stderr, "%s: odm initialization failed\n", prog);
		exit(E_ODMINIT);
	}

	/* open customized devices object class (CuDv) */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		fprintf(stderr, "%s: couldn't open customized object class\n",
			prog);
		err_exit(E_ODMOPEN);
	}
	
	/****************************************************
	  Get the Customized Devices Object for this device.
	 ****************************************************/
	sprintf(sstring,"name = '%s'",logical_name);
	if ((rc = (int)odm_get_obj(cusdev,sstring,&cusobj,ODM_FIRST)) == 0) {
		/* odm objects not found */
		fprintf(stderr, "%s: CuDv crit=\"%s\" found no objects.\n",
			prog, sstring);
		err_exit (E_NOCuDv);
	} else if (rc == -1) {
		/* odm error occurred */
		fprintf(stderr, "%s: get_obj failed, crit=\"%s\".\n",
			prog, sstring);
		err_exit (E_ODMGET);
	}

	/*******************************************************************
	  Get the predefined object for this device. This object is located
	  searching the predefined devices object class based on the unique
	  type link descriptor in the customized device.
	  *******************************************************************/
	/* open predefined devices object class (PdDv) */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		fprintf(stderr, "%s: couldn't open predefined object class\n",
			prog);
		err_exit (E_ODMOPEN);
	}

	/* search Predefined devices for object with this unique type */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	if ((rc = (int)odm_get_obj(predev,sstring,&preobj,ODM_FIRST)) == 0) {
		/* odm objects not found */
		fprintf(stderr, "%s: PdDv crit=\"%s\" found no objects.\n",
			prog, sstring);
		err_exit (E_NOPdDv);
	} else if (rc == -1) {
		/* odm error occurred */
		fprintf(stderr, "%s: get_obj failed, crit=\"%s\".\n",
			prog, sstring);
		err_exit (E_ODMGET);
	}
	
	/****************************************************************
	  Display this device's LED value on the system LEDs.
	 ****************************************************************/

	if (ipl_phase != RUNTIME_CFG)
		setleds (preobj.led);

	/***************************************************************
	  Check to see if the device is already configured.
	  We actually go about the business of configuring the device
	  only if the device is not configured yet i.e. DEFINED.
	 ***************************************************************/
	if (cusobj.status == (short)DEFINED) {
		
		/*******************************************************
		   Get the attributes for the aio object from
		  predefined and customized attribute object class.
		 *******************************************************/
		if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1) {
			fprintf(stderr, "%s: couldn't open CuAt class\n", prog);
			err_exit (E_ODMOPEN);
		}
		
		if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1) {
			fprintf(stderr, "%s: couldn't open PdAt class\n", prog);
			err_exit (E_ODMOPEN);
		}
		
		if (cuat = getattr(logical_name,"minservers", FALSE, &rc))
			aio_cfg.minservers = atoi(cuat->value);
		if (cuat = getattr(logical_name,"maxservers", FALSE, &rc))
			aio_cfg.maxservers = atoi(cuat->value);
		if (cuat = getattr(logical_name,"kprocprio", FALSE, &rc))
			aio_cfg.kprocprio = atoi(cuat->value);
		if (cuat = getattr(logical_name,"maxreqs", FALSE, &rc))
			aio_cfg.maxreqs = atoi(cuat->value);
		
		DEBUG_1(" minservers = %d \n", aio_cfg.minservers);
		DEBUG_1(" maxservers = %d \n", aio_cfg.maxservers);
		DEBUG_1(" kprocprio = %d \n", aio_cfg.kprocprio);
		DEBUG_1(" maxreqs   = %d \n", aio_cfg.maxreqs);
		
		if (odm_close_class(cusatt) < 0) {
			fprintf(stderr, "%s: error closing CuAt class\n", prog);
			err_exit (E_ODMCLOSE);
		}
		if (odm_close_class(preatt) < 0) {
			fprintf(stderr, "%s: error closing PdAt class\n", prog);
			err_exit (E_ODMCLOSE);
		}

		/***********************************************
		  Call loadext() to load aio extension.
		  loadext will load the extension only if needed,
		  and always returns the kmid of the extension.
		 ***********************************************/
		DEBUG_1 ("Loading %s\n",preobj.DvDr)
		if ((cfg.kmid = loadext(preobj.DvDr,TRUE,FALSE)) == NULL) {
			/* error loading device driver */
			fprintf(stderr, "%s: error %d loading %s\n",
				prog, errno, preobj.DvDr);
			err_exit (E_LOADEXT);
		}

		/*************************************************
		  Now call sysconfig() to configure the driver.
		 *************************************************/
		DEBUG_0 ("Initializing Device ...\n")
		cfg.cmd = CFG_INIT;	
		cfg.mdiptr = (caddr_t) &aio_cfg;
		cfg.mdilen = sizeof(aio_cfg); 

		if (sysconfig(SYS_CFGKMOD,&cfg,sizeof(struct cfg_kmod))==-1) {
			fprintf(stderr, "%s: error %d from sysconfig",
				prog, errno);
			err_undo1(logical_name);
			err_exit (E_CFGINIT);
		}

		DEBUG_0 ("Initialize Complete\n")

		/**********************************************************
		  Update the customized object to reflect the device's
		  current status. 
		  changed to AVAILABLE, and, if they were generated, the
		 **********************************************************/
		cusobj.status = (short) AVAILABLE;
		if ((rc = odm_change_obj(cusdev,&cusobj)) < 0) {
			/* change object failed */
			fprintf(stderr, "%s: update of %s failed.\n",
				prog, logical_name);
			err_exit (E_ODMUPDATE);
		}
	}

	/* close object classes */
	if (odm_close_class(predev) < 0) {
		fprintf(stderr, "%s: error closing PdDv class\n", prog);
		err_exit (E_ODMCLOSE);
	}
	if (odm_close_class(cusdev) < 0) {
		fprintf(stderr, "%s: error closing CuDv class\n", prog);
		err_exit (E_ODMCLOSE);
	}
	
	/***********************************************************
	  Config method is finished at this point. Terminate the
	  ODM, and exit with a good return code.
	 ***********************************************************/
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
void
err_exit(exitcode)
int	exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	fprintf(stderr, "%s: exiting with code %d\n", prog, exitcode);
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
		fprintf(stderr, "%s: error unloading %s\n", prog, DvDr);
	return;
}
