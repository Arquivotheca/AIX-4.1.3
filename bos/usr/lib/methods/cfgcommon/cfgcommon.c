static char sccsid[] = "@(#)11  1.1  src/bos/usr/lib/methods/cfgcommon/cfgcommon.c, cfgmethods, bos411, 9428A410j 6/28/94 07:12:09";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfgcommon.c - Generic Config Method Code
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
extern int	configure_device();




/* global variables */
int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
struct	CuDv    cudv;		/* Customized object for device */
struct	PdDv    pddv;		/* Predefined object for device */
struct	CuDv    pcudv;		/* Customized object for parent */
mid_t	kmid;                   /* Device driver kmid */
dev_t	devno;                  /* Device driver devno */
int	devinst;		/* Device instance number */
int	loaded_dvdr=0;		/* Indicates driver to be unloaded on error */
int	inited_dvdr=0;		/* Indicates CFG_TERM needed on error */
char	dvdr[16]={'\0'};	/* Device driver load module name */

int	Dflag;			/* flag for defining children */



/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	char	*logical_name;		/* logical name to configure */
	char	sstring[256];		/* search criteria pointer */
	char    vpd[VPDSIZE];           /* vpd data */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *predev;		/* predefined devices class ptr */
	struct Class *cv;		/* customized vpd class ptr */
	struct Class *cusatt;           /* customized attribute class ptr */
	struct Class *preatt;		/* predefined attribute class ptr */

	struct CuDv tmpobj;		/* customized device object storage */
	struct CuVPD cvpd;		/* customized vpd object */

	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

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
		DEBUG_0("command line error\n");
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */
	/* logical name must be specified */
	if (logical_name == NULL) {
		DEBUG_0("logical name must be specified\n");
		exit(E_LNAME);
	}

	DEBUG_1 ("Configuring device: %s\n",logical_name)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("odm_initialize() failed\n")
		exit(E_ODMINIT);
	}

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		DEBUG_0("open class CuDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1) {
		DEBUG_0("open class PdDv failed\n");
		err_exit(E_ODMOPEN);
	}

	/* open customized attribute object class */
	if ((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1) {
		DEBUG_0("open class CuAt failed\n");
		err_exit(E_ODMOPEN);
	}

	/* open predefined attribute object class */
	if ((int)(preatt = odm_open_class(PdAt_CLASS)) == -1) {
		DEBUG_0("open class PdAt failed\n");
		err_exit(E_ODMOPEN);
	}


	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cudv);
	if (rc==0) {
		/* No CuDv object with this name */
		DEBUG_1("no CuDv for %s\n", logical_name);
		err_exit(E_NOCuDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("ODM failure getting CuDv object");
		err_exit(E_ODMGET);
	}

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cudv.PdDvLn_Lvalue);
	rc = (int)odm_get_first(predev, sstring, &pddv);
	if (rc==0) {
		/* No PdDv object for this device */
		DEBUG_0("no PdDv object for this device\n");
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("ODM failure getting PdDv object");
		err_exit(E_ODMGET);
	}

	/****************************************************************
	  If this device is being configured during an ipl phase, then
	  display this device's LED value on the system LEDs.
	  ****************************************************************/
	if (ipl_phase != RUNTIME_CFG)
		setleds(pddv.led);



	/* get the device's parent object */
	sprintf(sstring, "name = '%s'", cudv.parent);
	rc = (int)odm_get_first(cusdev,sstring,&pcudv);
	if (rc==0) {
		/* Parent device not in CuDv */
		DEBUG_0("no parent CuDv object\n");
		err_exit(E_NOCuDvPARENT);
	}
	else if (rc==-1) {
		/* ODM failure */
		DEBUG_0("ODM failure getting parent CuDv\n")
		err_exit(E_ODMGET);
	}



	/******************************************************************
	  Check to see if the device is already configured (AVAILABLE).
	  We actually go about the business of configuring the device
	  only if the device is not configured yet. Configuring the
	  device in this case refers to the process of checking parent
	  and sibling status building a DDS, loading the driver, etc...
	  ******************************************************************/

	if (cudv.status == DEFINED) {

		/*******************************************************
		  The device is not available to the system yet. Now
		  check to make sure that the device's relations will
		  allow it to be configured. In particular, make sure
		  that the parent is configured (AVAILABLE), and that
		  no other devices are configured at the same location.
		  *******************************************************/

		if (pcudv.status != AVAILABLE) {
			DEBUG_0("parent is not AVAILABLE")
			err_exit(E_PARENTSTATE);
		}

		/* make sure that no other devices are configured     */
		/* at this location                                   */
		sprintf(sstring, "parent=%s AND connwhere=%s AND status=%d",
			cudv.parent, cudv.connwhere, AVAILABLE);
		rc = (int)odm_get_first(cusdev,sstring,&tmpobj);
		if (rc == -1) {
			/* odm failure */
			err_exit(E_ODMGET);
		} 
		else if (rc) {
			/* Error: device config'd at this location */
			DEBUG_0("device already AVAILABLE at this connection\n")
			err_exit(E_AVAILCONNECT);
		}

		/***************************************************
		  If the device is an adapter being configured at
		  RUN TIME, then we must resolve any bus attribute
		  conflicts before configuring device to the driver.
		 ***************************************************/
		if (ipl_phase == RUNTIME_CFG) {

			rc = runtime_busresolve();
			if (rc)
				err_exit(rc);
		}


		/* Perform specific steps to configure device. */
		rc = configure_device();
		if (rc) {
			err_exit(rc);
		}

		/* if device has VPD data then go get it */
		if (pddv.has_vpd == TRUE) {
			/* get VPD for this device */
			memset( vpd, 0, sizeof(vpd) );
			DEBUG_0("Calling query_vpd()\n")
			rc = query_vpd(&cudv, kmid, devno, vpd);
			if (rc) {
				/* failed to get VPD */
				DEBUG_1("error getting VPD, rc=%d\n",rc)
				if ( rc < 0 || rc > 255)
					rc = E_VPD;
				err_exit(rc);
			}
			DEBUG_0("Returned from query_vpd()\n")

			/* open customized vpd object class */
			if ((int)(cv = odm_open_class(CuVPD_CLASS)) == -1) {
				DEBUG_0("open class CuVPD failed");
				err_exit(E_ODMOPEN);
			}

			/* search for customized vpd for this logical name */
			sprintf(sstring, "name = '%s' and vpd_type = '%d'",
				logical_name,HW_VPD);
			rc = (int)odm_get_first(cv,sstring,&cvpd);
			if (rc==-1) {
				/* ODM failure */
				DEBUG_0("ODM failure getting CuVPD object");
				err_exit(E_ODMGET);
			}
			if (rc==0) {
				/* need to add vpd object */
				DEBUG_0("Adding new VPD object\n");
				strcpy(cvpd.name,logical_name);
				cvpd.vpd_type = HW_VPD;
				memcpy(cvpd.vpd,vpd,sizeof(vpd));
				if (odm_add_obj(cv,&cvpd) == -1) {
					DEBUG_0("ODM error adding CuVPD obj\n")
					err_exit(E_ODMADD);
				}
				DEBUG_0("Successfully added new VPD object\n");
			} else {
				/* see if vpd object needs to be updated */
				if (memcmp(cvpd.vpd,vpd,sizeof(vpd))) {
					DEBUG_0("Updating VPD object\n");
					memcpy(cvpd.vpd,vpd,sizeof(vpd));
					if (odm_change_obj(cv,&cvpd) == -1) {
						DEBUG_0("ODM failure updating CuVPD object")
						err_exit(E_ODMUPDATE);
					}
					DEBUG_0("Successfully updated VPD object\n");
				}
			}
			/* close customized vpd object class */
			if (odm_close_class(CuVPD_CLASS) == -1) {
				DEBUG_0("error closing CuVPD object class\n");
				err_exit(E_ODMCLOSE);
			}
		}


	       /* 
		* Update customized device object.  Set current 
		* status to AVAILABLE and reset change status to
		* SAME only if it was MISSING
		*/

		cudv.status = AVAILABLE;
		if (cudv.chgstatus == MISSING) {
		    cudv.chgstatus = SAME ;
		}
		if (odm_change_obj(cusdev, &cudv) == -1) {
		    /* ODM failure */
		    DEBUG_0("ODM failure updating CuDv object\n");
		    err_exit(E_ODMUPDATE);
		}
	} /* end if (device is not AVAILABLE) then ... */

	/* call device specific routine to detect/manage child devices */
	DEBUG_0("Calling define_children()\n")
	if (define_children(logical_name, ipl_phase) != 0) {
		/* error defining children */
		DEBUG_0("error defining children\n");

		/* Do not want to terminate device nor unload driver */
		inited_dvdr = 0;
		loaded_dvdr = 0;
		err_exit(E_FINDCHILD);
	}

	/* Terminate ODM, this closes object classes */
	odm_terminate();
	exit(0);

}
