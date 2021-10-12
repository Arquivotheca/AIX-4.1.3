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
#include <sys/ndd.h>

#include "cfg_ndd.h"            /* NDD device specific includes */
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
	struct	cfg_kmod cfg_k;		/* sysconfig command structure */
	struct ndd_config ndd_cfg_struct; /* dvdr cfg init struc */
	struct  CuAt    *cusattobj;     /* customized devices object */
	char	sstring[256];		/* search criteria */
	char	addl_dvdr[128];		/* name of additional driver */
	int	inst_num;		/* instance number of device */
	int	rc;


	if (pddv->DvDr[0] != '\0') {
		DEBUG_0( "Device has driver\n")
		/******************************************************
		  Call sysconfig() to "terminate" the device
		  If fails with EBUSY, then device instance is "open",
		  and device cannot be "unconfigured".  Any other errno
		  returned will be ignored since the driver MUST delete
		  the device even if it reports some other error.
		 ******************************************************/

		/************************************************
		  Call loadext() to query kernel module ID
		 ************************************************/
		DEBUG_1("Querying the driver: %s\n",pddv->DvDr)
		cfg_k.kmid = loadext(pddv->DvDr,FALSE,TRUE);
		if (cfg_k.kmid == NULL) {
			/* error querying device driver */
			DEBUG_0("Error querying driver\n")
			return(E_UNLOADEXT);
		}
		DEBUG_1("kmid of driver = %x\n",cfg_k.kmid)

		/* get instance number for this device */
		inst_num = lsinst(cudv->name);
		if (inst_num == -1) {
		    DEBUG_0("cfgcommo: error getting instance number\n")
		    return(E_INSTNUM);
		}
		DEBUG_1("cfgcommo: instance number = %d\n",inst_num);

		ndd_cfg_struct.seq_number = inst_num;

		cfg_k.mdiptr = (caddr_t) &ndd_cfg_struct;
		cfg_k.mdilen = sizeof (struct ndd_config);
		cfg_k.cmd = CFG_TERM;

		DEBUG_0("Calling sysconfig() to terminate Device\n")
		rc = sysconfig(SYS_CFGKMOD,&cfg_k,sizeof(struct cfg_kmod));
		if (rc == -1) {
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
		cfg_k.kmid = loadext(pddv->DvDr,FALSE,FALSE);
		if (cfg_k.kmid == NULL) {
			/* error unloading device driver */
			DEBUG_0("Error unloading driver\n")
			return(E_UNLOADEXT);
		}

		DEBUG_0("Unloaded driver, checking additional driver\n")

		/* check for additional driver */
		/* if present, remove it also */

		/**************************************************************
		  If the device has an additional driver, then need to delete
		  device from system and call loadext() to unload driver.
		 **************************************************************/
		strcpy ( addl_dvdr, NULL );      /* initialize values */
		/* get additional driver attribute from database */
		cusattobj = getattr(cudv->name, "addl_dvdr", FALSE, &how_many );
		if (cusattobj != NULL) {
			strcpy( addl_dvdr, cusattobj->value );
		}
		else {
			/* NOTE: error to be indicated here           */
			/*       only if attribute is there but could */
			/*       not be read                          */
			if ( conferrno != E_NOPdOBJ ) {
				/* error getting addl dvdr name */
				DEBUG_0("Error getting addl dvdr name\n")
				return(E_ODMGET);
			}
		}

		if (strcmp(addl_dvdr,"") != 0) {
		    DEBUG_0( "Device has an additional driver\n")

		    /* call specified method with parameter "ADDL_UCFG"
		     * to unconfigure the additional driver
		     */

		    sprintf( sstring, " %d %s %d ",
			    ADDL_UCFG, cudv->name, inst_num);
		    DEBUG_2("Calling %s %s\n",addl_dvdr, sstring)

		    if (odm_run_method(addl_dvdr,sstring,NULL,NULL)) {
			    return(E_ODMRUNMETHOD);
		    }

		}  /* end if (device has an additional driver) then ... */

	}  /* end if (device has a driver) then ... */

	return(0);
}
