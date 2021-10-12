static char sccsid[] = "@(#)04  1.1  src/bos/usr/lib/methods/cfgcommon/cfg_dvdr.c, cfgmethods, bos411, 9428A410j 6/28/94 07:10:07";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfg_dvdr.c - used for devices that have 
 *   				             device drivers
 *
 * FUNCTIONS: configure_device, err_exit
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
#include "cfgcommon.h"
#include "cfgdebug.h"

/* external functions */
extern long	genmajor();


int
configure_device()

{
	struct cfg_dd cfg;		/* sysconfig command structure */
	struct CuDv tmpobj;		/* customized device object storage */
	int     majorno;                /* major number assigned to device */
	int     minorno;                /* minor number assigned to device */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */
	int	rc;			/* return codes go here */


	rc = get_dvdr_name();
	if (rc) {
		err_exit(rc);
	}
		
	if (dvdr[0] != '\0') {

		/* call loadext to load the device driver */
		if ((kmid = loadext(dvdr, TRUE, FALSE)) == NULL) {
			/* error loading device driver */
			err_exit(E_LOADEXT);
		}
		loaded_dvdr = 1;

		/* get major number      */
		DEBUG_0("Calling genmajor()\n")
		if ((majorno = genmajor(dvdr)) == -1) {
		    DEBUG_0("error generating major number");
		    err_exit(E_MAJORNO);
		}
		DEBUG_1("Major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("Calling getminor()\n")
		minor_list = getminor(majorno,&how_many,cudv.name);
		if (minor_list == NULL || how_many == 0) {
		    DEBUG_0("Calling generate_minor()\n")
		    rc = generate_minor(cudv.name,majorno,&minorno);
		    if (rc) {
			DEBUG_1("error generating minor number, rc=%d\n",rc)
			/* First make sure any minors that might */
			/* have been assigned are cleaned up */
			reldevno(cudv.name, TRUE);
			if ( rc < 0 || rc > 255)
			    rc = E_MINORNO;
			err_exit(rc);
		    }
		}
		else
		    minorno = *minor_list;
		DEBUG_1("minor number: %d\n",minorno)

		/* create devno for this device */
		devno = makedev(majorno, minorno);

		/* make special files      */
		DEBUG_0("Calling make_special_files()\n")
		rc = make_special_files(cudv.name, devno);
		if (rc) {
			/* error making special files */
			DEBUG_1("error making special file(s), rc=%d\n",rc)
			if ( rc < 0 || rc > 255)
				rc = E_MKSPECIAL;
			err_exit(rc);
		}

		/* build the DDS  */
		DEBUG_0("Calling build_dds()\n")
		rc = build_dds(cudv.name, &cfg.ddsptr, &cfg.ddslen);
		if (rc) {
			/* error building dds */
			DEBUG_1("error building dds, rc=%d\n",rc)
			if ( rc < 0 || rc > 255)
				rc = E_DDS;
			err_exit(rc);
		}

		/* call sysconfig to pass DDS to driver */
		DEBUG_0("Pass DDS to driver via sysconfig()\n")
		cfg.kmid = kmid;
		cfg.devno = devno;
		cfg.cmd = CFG_INIT;
		if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
			/* error configuring device */
			DEBUG_0("error configuring device\n")
			err_exit(E_CFGINIT);
		}

		/* Set flag to indicate that CFG_TERM needed on error */
		inited_dvdr = 1;

		/* download microcode if necessary */
		DEBUG_0("Calling download_microcode()\n")
		rc = download_microcode(cudv.name);
		if (rc) {
			/* error downloading microcode */
			DEBUG_1("error downloading microcode, rc=%d\n",rc)
			if ( rc < 0 || rc > 255)
				rc = E_UCODE;
			err_exit(rc);
		}

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
	struct  cfg_dd cfg;             /* sysconfig command structure */

	/* Terminate the ODM */
	odm_terminate();

	if (inited_dvdr) {
		/* terminate device */
		cfg.devno = devno;
		cfg.kmid = (mid_t)0;
		cfg.ddsptr = (caddr_t) NULL;
		cfg.ddslen = (int)0;
		cfg.cmd = CFG_TERM;

		if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
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
