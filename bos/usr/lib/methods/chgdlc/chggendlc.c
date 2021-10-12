static char sccsid[] = "@(#)04  1.14  src/bos/usr/lib/methods/chgdlc/chggendlc.c, dlccfg, bos411, 9428A410j 10/19/93 09:19:25";
/*
 * COMPONENT_NAME : DLCCFG (change method for Data Link Control
 *
 * FUNCTIONS :  check_parms
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

#include        <stdio.h>
#include        <cf.h>
#include        <sys/cfgdb.h>
#include        <sys/cfgodm.h>
#include "cfgdebug.h"


/* external functions */


/*
 * NAME     : check_parms
 *
 * FUNCTION : This function checks the validity of the attributes.
 *
 * EXECUTION ENVIRONMENT :
 *      This function is called by the change_device function to
 *      check the validity of the attributes of the adapter.
 *
 * NOTES :
 *
 *
 * RECOVERY OPERATION :
 *
 * DATA STRUCTURES :
 *
 * RETURNS :
 *      Returns  0 on success, < 0 on failure.
 */


int
check_parms(attrs, Pflag, Tflag )
struct attr *attrs ;
int     Pflag;          /* if Pflag == 1 then the -P flag was passed */
			/*             0 then the -P flag was NOT passed */
int     Tflag;          /* if Tflag == 1 then the -T flag was passed */
			/*             0 then the -P flag was NOT passed */
{
	return(0);
}

