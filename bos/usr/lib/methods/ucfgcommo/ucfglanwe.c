/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: dev_specific
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *   this file is linked with ucfgcommo.c to create an unconfigure method
 *   for each lan device that needs to check for a comio emulator to be
 *   installed prior to calling its unconfigure method.
 *   See ./src/bos/usr/lib/methods/cfgcie/ciepkg.h" file for DEFINEs
 *   of package to check, and unconfigure method to call.
 */

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/device.h>

#include "cfgdebug.h"
#include "ciepkg.h"     /* header file for cfgcie method, see define_children() */

/*
 * NAME: dev_specific()
 *
 * FUNCTION: Generic routine for lan devices with comio emulator support
 *
 * EXECUTION ENVIRONMENT:
 *      This is a lan device specific routine for unconfigure
 *      operation.
 *
 * RETURNS: Returns  0
 *              errors are not returned here, the primary device
 *              has already been unloaded, print error to screen
 *              and return 0.
 */

int dev_specific(logical_name)
char *logical_name;
{

    int rtn;
    struct stat stat_buf;
    char    sstring[256];           /* parameter temp space */

    DEBUG_0("dev_specific: BEGIN routine\n");

    /*
     * Begin section for unconfiguring comio emulator for this device
     * if the emulator has been installed.  If unconfiguration method for
     * emulator is present (code has been installed), execute it.
     * If file has not been installed, continue with no error.
     */

    sprintf(sstring,"lslpp -l %s > /dev/null 2>&1",CFG_EMULATOR_LPP);
    rtn = system(sstring);
    if (rtn == 0) {     /* directory exists */

	rtn = stat(UCFG_COMIO_EMULATOR,&stat_buf);
	if (rtn == 0) {     /* file exists */

	    /* call specified method with parameters */
	    sprintf( sstring, " -l %s ", logical_name);
	    DEBUG_2("ucfglanwe: calling %s %s\n",UCFG_COMIO_EMULATOR, sstring)

	    if(rtn=odm_run_method(UCFG_COMIO_EMULATOR,sstring,NULL,NULL)){
		fprintf(stderr,"ucfglanwe: can't run %s, rtn=%d\n", UCFG_COMIO_EMULATOR,rtn);
		/* errors are not returned here, the primary device */
		/* has already been unloaded, print error to screen */
		/* and return 0.                                    */
	    }
	}
	else {
	    /* package installed, but file missing, return error */
	    fprintf(stderr,"ucfglanwe: %s does not exist when it should\n", UCFG_COMIO_EMULATOR);
	    /* errors are not returned here, the primary device */
	    /* has already been unloaded, print error to screen */
	    /* and return 0.                                    */
	}
    }
    /* End section for unconfiguring comio emulator for token ring */
    return 0;

}
