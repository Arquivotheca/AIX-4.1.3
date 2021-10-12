/*
 * COMPONENT_NAME: (CFGMETH) Generic Unconfigure Method.
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
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

int
unconfigure_device(pddv,cudv)

struct PdDv *pddv;
struct CuDv *cudv;

{
	long	majorno,minorno;	/* major and minor numbers */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */
	struct	cfg_dd cfg;		/* sysconfig command structure */


	if (pddv->DvDr[0] != '\0') {
		DEBUG_0( "Device has driver\n")

		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the driver MUST delete
		  the device even if it reports some other error.
		 ******************************************************/

		/* first, need to create devno for this device */
		majorno = genmajor(pddv->DvDr);
		if (majorno == -1) {
			DEBUG_0("failed to get major number.\n");
			return(E_MAJORNO);
		}
		DEBUG_1("Returned major number: %d\n",majorno)

		/* get minor number      */
		DEBUG_0("Calling getminor()\n")
		minor_list = getminor(majorno, &how_many, cudv->name);
		if (minor_list == NULL || how_many == 0) {
			DEBUG_0("failed to get minor number.\n");
			return(E_MINORNO);
		}
		minorno = *minor_list;
		DEBUG_1("minor number: %d\n",minorno)

		/* CFG.KMID */
    		if ((cfg.kmid = loadext(pddv->DvDr, FALSE, TRUE)) == (mid_t) NULL) {
        		DEBUG_1("Unable to get cfg.kmid for %s\n", pddv->DvDr);
			return(E_LOADEXT);
    		} /* End if ((cfg.kmid = loadext(...)) == (mid_t) NULL) */



		/* create devno for this device */
		cfg.devno = makedev(majorno,minorno);
		cfg.ddsptr = (caddr_t) NULL;
		cfg.ddslen = (int)0;
		cfg.cmd = CFG_TERM;	

		DEBUG_0("Calling sysconfig() to terminate Device\n")
		if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
			if (errno == EBUSY) {
				/* device is in use and can't be unconfigured */
				DEBUG_0("Device is busy\n")
				return(E_BUSY);
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
		DEBUG_1("Unloading the driver: %s\n",pddv->DvDr)
		cfg.kmid = loadext(pddv->DvDr,FALSE,FALSE);
		if (cfg.kmid == NULL) {
			/* error unloading device driver */
			DEBUG_0("Error unloading driver\n")
			return(E_UNLOADEXT);
		}
		DEBUG_0("Unloaded driver\n")
	}

	return(0);
}
