/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: dev_specific
 *
 *   ORIGINS: 27, 83
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 *   linked with ucfgcommo for generic unconfigure routine
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

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

int dev_specific(pddv,cudv)
struct PdDv *pddv;
struct CuDv *cudv;
{
#ifdef _CFG_RDS
	struct   CuAt  sys_cuattr;
	int rc;
	rc = (int)odm_get_first(CuAt_CLASS, "name=sys0 AND attribute=rds_facility",
                                 &sys_cuattr);
        if (rc == -1) {
            DEBUG_0("ODM error getting rds_facility attribute\n")
            return(rc);
        } else if (rc != 0 && sys_cuattr.value[0]=='y') {
                rc = rds_power_off_device(cudv);
                if (rc != 0)
                        DEBUG_1("Can not switch off the device %s\n", cudv.name);
                        return(rc);
        }
	else {
		DEBUG_0("dev_specific: Null routine\n");
		return(0);
	}
#else
	DEBUG_0("dev_specific: Null routine\n");
	return(0);
#endif
}
