static char sccsid[] = "@(#)07  1.1  src/bos/usr/lib/methods/cfgcommon/cfg_no_dvdr.c, cfgmethods, bos411, 9428A410j 6/28/94 07:10:58";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfg_no_dvdr.c - used for devices that have NO
 *						load module
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


int
configure_device()

{
	/* This device has no device driver and nothing to configure */
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
	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}
