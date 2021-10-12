static char sccsid[] = "@(#)03  1.2  src/bos/usr/lib/methods/cfgcommon/cfg_cdli.c, cfgmethods, bos41J, 9509A_all 2/14/95 10:53:33";
/*
 *   COMPONENT_NAME: CFGMETHODS - cfg_cdli.c - used for CDLI (commo) devices
 *
 *   FUNCTIONS: configure_device, err_exit
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
#include <sys/ndd.h>

/* Local header files */
#include "cfgdebug.h"
#include "cfg_ndd.h"
#include "cfgcommon.h"

/* external functions */
extern  int conferrno;          /* Config (getattr) Error Number */


int
configure_device()

{
    struct cfg_kmod cfg_k;          /* sysconfig command structure */
    char    sstring[256];           /* search criteria pointer */
    struct CuAt *cusattobj;         /* device customized attribute storage */
    char    addl_dvdr[128];         /* name of additional driver */
    int     how_many;               /* storage for getattr command */
    ndd_config_t    ndd_config;     /* devices cfg init struct */
    int     l_dds;                  /* length of user defined dds */
    int     rc;                     /* return codes go here */




	kmid = 0;           /* initialize values */

	DEBUG_1("Checking for DvDr entry, DvDr=%s\n",pddv.DvDr);
	if (strcmp(pddv.DvDr, "") != 0) {

		/* put driver name into dvdr global variable */
		rc=get_dvdr_name();

		/* get instance number for this device */
		devinst = geninst(dvdr,cudv.name);
		if (devinst == -1) {
			DEBUG_0("Error getting instance number\n")
			err_exit(E_INSTNUM);
		}
		DEBUG_1("Instance number = %d\n",devinst);

		DEBUG_0("Load a device driver\n")
		/* call loadext to load the device driver */
		cfg_k.kmid = loadext(pddv.DvDr, TRUE, FALSE);
		if (cfg_k.kmid == NULL) {
			/* error loading device driver */
			DEBUG_1("Error loading driver %s\n", pddv.DvDr)
			err_exit(E_LOADEXT);
		}
		loaded_dvdr = 1;
		kmid = cfg_k.kmid;
		DEBUG_1("Kernel module id = %x\n",kmid)

		ndd_config.seq_number = devinst;
		ndd_config.dds = (char * ) NULL;
		cfg_k.mdiptr = (char *) &ndd_config;
		cfg_k.mdilen = sizeof (struct ndd_config);

		/* build the DDS  */
		DEBUG_0("Calling build_dds()\n")
		rc = build_dds(cudv.name,&ndd_config.dds,&l_dds );

/* NOTE: l_dds is never used !!!!!!!!!!  Ask chuck. */


		if (rc) {
	 		/* error building dds */
	 		DEBUG_1("Error building dds, rc=%d\n",rc)
	 		if ( rc < 0 || rc > 255)
	  			rc = E_DDS;
	 		err_exit(rc);
		}
		DEBUG_0("Returned from build_dds()\n")

		/* call sysconfig to pass DDS to driver */
		DEBUG_0("Pass DDS to driver via sysconfig()\n")
		cfg_k.cmd = CFG_INIT;
		rc = sysconfig(SYS_CFGKMOD, &cfg_k, sizeof(struct cfg_kmod ));
		if (rc == -1) {
			/* error configuring device */
			DEBUG_0("Error configuring device\n")
			err_exit(E_CFGINIT);
		}
		/* Set flag to indicate that CFG_TERM needed on error */
		inited_dvdr = 1;


		/* download microcode if necessary */
		DEBUG_0("Calling download_microcode()\n")
		ndd_config.seq_number = devinst;
		ndd_config.ucode = (char * ) NULL;
		cfg_k.mdiptr = (char *) &ndd_config;
		cfg_k.mdilen = sizeof (struct ndd_config);
		rc = download_microcode(cudv.name, &cfg_k);
		if (rc) {
			/* error downloading microcode */
			DEBUG_1("Error downloading microcode, rc=%d\n",rc)
			if ( rc < 0 || rc > 255)
				rc = E_UCODE;
			err_exit(rc);
		}
		DEBUG_0("Returned from download_microcode()\n")

		/***************************************************
	  	If device has an additional driver, then need to
	  	call the specified configure method for that driver
	  	to load and configure the driver
	  	Attribute addl_dvdr is the method to run
	 	***************************************************/

		strcpy ( addl_dvdr, NULL );      /* initialize values */
		cusattobj = getattr( cudv.name, "addl_dvdr", FALSE,&how_many );
		if (cusattobj != NULL) {
	 		strcpy ( addl_dvdr, cusattobj->value);
		}
		else {
			/* NOTE: error to be indicated here           */
			/*       only if attribute is there but could */
			/*       not be read                          */
			if ( conferrno != E_NOPdOBJ ) {
				/* error getting addl dvdr name */
				DEBUG_0("Error getting addl dvdr name\n")
				err_exit(E_ODMGET);
			}
		}

		if (strcmp(addl_dvdr, "") != 0) {

			/* call specified method with parameter "ADDL_CFG"
		 	* to configure the additional driver
		 	*/

			sprintf( sstring, " %d %s %d ",
				ADDL_CFG, cudv.name, devinst);
			DEBUG_2("Calling %s %s\n",addl_dvdr, sstring)

			if (odm_run_method(addl_dvdr,sstring,NULL,NULL)) {
				err_exit(E_ODMRUNMETHOD);
			}

		} /* end if (device has an additional driver) then ... */

	} /* end if (device has a driver) then ... */

	/* Call device specific routine for additional cfg steps */
	rc = device_specific();
	if (rc) {
		/* error returned */
		err_exit(rc);
	}

	return(0);
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
	int	rc;
	struct  cfg_kmod cfg_k;         /* sysconfig command structure */
	ndd_config_t    ndd_config;     /* devices cfg init struct */

	/* Terminate the ODM */
	odm_terminate();

	if (inited_dvdr) {
                /* terminate device */
		cfg_k.kmid = kmid;
		cfg_k.cmd = CFG_TERM;
		ndd_config.seq_number = devinst;
		cfg_k.mdiptr = (char *) &ndd_config;
		cfg_k.mdilen = sizeof (struct ndd_config);

		rc = sysconfig(SYS_CFGKMOD,&cfg_k,sizeof(struct cfg_kmod));
		if (rc == -1) {
			DEBUG_0("error unconfiguring device\n");
		}
	}

	if (loaded_dvdr) {
                /* unload driver */
                if (loadext(dvdr,FALSE,FALSE) == NULL) {
                        DEBUG_0("error unloading driver\n");
                }
        }

	exit(exitcode);
}
