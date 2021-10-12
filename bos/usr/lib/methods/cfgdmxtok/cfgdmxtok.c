static char sccsid[] = "@(#)69  1.1  src/bos/usr/lib/methods/cfgdmxtok/cfgdmxtok.c, sysxtok, bos411, 9428A410j 9/17/93 09:40:32";
/*
 * COMPONENT_NAME: (sysxtok) cfgdmxtok.c - Tokenring Demuxer Method
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
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

#include "cfgdebug.h"
#include "cfg_ndd.h"            /* NDD device specific includes */

/* device specific stuff */
#include "cfgdmxtok.h"          /* Ethernet specific information */

extern	int errno;		/* System Error Number */

/*
 * NAME: main
 * 
 * FUNCTION: This process is executed to "configure" an additional
 *           device specified
 * 
 * EXECUTION ENVIRONMENT:
 *
 *      This process is invoked when a device specifies an additional
 *      driver to be loaded and configured.  This additional device is
 *      to be "configured".
 *
 * NOTES: Interface:
 *              cfgdmx____.c <command> <logical_name> <instance_number>
 *
 * RETURNS: Exits with > 0 if failure, 0 on success.
 */

main(argc,argv)
int argc;
char *argv[];
{

	struct  cfg_kmod cfg_k;         /* sysconfig command structure */
	char    logical_name[40];       /* logical name to unconfigure */
	char	sstring[256];		/* search criteria */
	char	errstring[256];		/* error string */

	int     how_many;               /* storage for getattr command */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	struct ndd_config ndd_config;   /* dvdr cfg init struc */
	int     l_prefix;               /* length of prefix name in PdDv */
	int     inst_num;               /* instance number of device */
	int     command;        /* command to configure or unconfigure */

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	if (argc != 4) {
		DEBUG_1("cfgdmx: invalid number of arguments (argc=%d)\n",argc);
		exit(E_ARGS);
	}

	command = atoi(argv[1]);
	strcpy (logical_name, argv[2]);
	inst_num = atoi(argv[3]);

	DEBUG_3("cfgdmx: input = %d, %s, %d \n",command,logical_name,inst_num);
	DEBUG_2("cfgdmx: device specific = %s, %d \n",DEMUX_NAME,DEMUX_TYPE);

	switch (command) {
	    case ADDL_CFG:
		DEBUG_0("cfgdmx: configure additional driver\n");

		/* call loadext to load the device driver */
		if ((cfg_k.kmid = loadext(DEMUX_NAME, TRUE, FALSE)) == NULL)
		{
		    /* error loading device driver */
		    DEBUG_1("cfgdmx: error loading driver %s\n", DEMUX_NAME)
		    exit(E_LOADEXT);
		}
		DEBUG_1("cfgdmx: DEMUX_NAME kernel module id = %x\n",cfg_k.kmid)

		/* build the DDS  */
		DEBUG_0("cfgdmx: building alternates dds()\n")
		ndd_config.seq_number = inst_num;
		ndd_config.dds = (char * ) NULL;
		cfg_k.mdiptr = (char *) &ndd_config;
		cfg_k.mdilen = sizeof (struct ndd_config);

		DEBUG_0("cfgdmx: Completed building additional dds()\n")

		/* call sysconfig to pass DDS to driver */
		DEBUG_0("cfgdmx: Pass DDS to driver via sysconfig()\n")
		cfg_k.cmd = CFG_INIT;
		if (sysconfig(SYS_CFGKMOD, &cfg_k, sizeof(struct cfg_kmod )) == -1) {
			/* error configuring device */
			DEBUG_0("cfgdmx: error configuring device\n")
			/* unload additional device driver */
			if ( loadext(DEMUX_NAME, FALSE, FALSE) == NULL) {
			    DEBUG_0("cfgdmx: error unloading driver \n")
			}
			exit(E_CFGINIT);
		}

		break;
	    case ADDL_UCFG:
		DEBUG_0("cfgdmx: unconfigure additional driver\n");

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
		DEBUG_1("cfgdmx: Querying the driver: %s\n",DEMUX_NAME)
		cfg_k.kmid = loadext(DEMUX_NAME,FALSE,TRUE);
		if (cfg_k.kmid == NULL) {
			/* error querying additional device driver */
			DEBUG_0("cfgdmx: error querying additional driver\n")
			exit(E_UNLOADEXT);
		}
		DEBUG_1("cfgdmx: kmid of driver = %x\n",cfg_k.kmid)

		ndd_config.seq_number = inst_num;

		cfg_k.mdiptr = (caddr_t) &ndd_config;
		cfg_k.mdilen = sizeof(ndd_config.seq_number);
		cfg_k.cmd = CFG_TERM;

		DEBUG_0("cfgdmx: Calling sysconfig() to terminate Device\n")
		if (sysconfig(SYS_CFGKMOD,&cfg_k,sizeof(struct cfg_kmod)) == -1) {
			if (errno == EBUSY) {
				/* device is in use and can not be unconfigured */
				DEBUG_0("cfgdmx: Device is busy\n")
				exit(E_BUSY);
			}
			/* Any other error, unload the demuxer.            */
			/* Only an E_BUSY says not to unload.              */
		}

		/************************************************
		  Call loadext() to unload device driver.
		  loadext() will unload the driver only if
		  device is last instance of driver configured.
		 ************************************************/
		DEBUG_1("cfgdmx: Unloading the driver: %s\n",DEMUX_NAME)
		cfg_k.kmid = loadext(DEMUX_NAME,FALSE,FALSE);
		if (cfg_k.kmid == NULL) {
			/* error unloading additional device driver */
			DEBUG_0("cfgdmx: error unloading additional driver\n")
			exit(E_UNLOADEXT);
		}

		DEBUG_0("cfgdmx: Unloaded additional driver\n")

		break;
	    default:
		DEBUG_0("cfgdmx: invalid command\n");
		exit(E_ARGS);
	}

	exit(0);
}
