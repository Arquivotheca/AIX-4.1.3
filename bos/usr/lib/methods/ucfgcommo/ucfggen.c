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
 *   linked with ucfgcommo for generic unconfigure routine
 */

#include <stdio.h>
#include <sys/types.h>

#include "cfgdebug.h"

/*
 * NAME: dev_specific()
 *
 * FUNCTION: Null function, used as generic routine
 *
 * EXECUTION ENVIRONMENT:
 *      This is a generic specific routine for unconfigure
 *      operation.
 *
 * RETURNS: Exits with 0
 */

int dev_specific(logical_name)
char *logical_name;
{

    DEBUG_0("dev_specific: Null routine\n");
    return 0;

}
