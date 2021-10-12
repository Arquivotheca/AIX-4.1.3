static char sccsid[] = "@(#)09  1.2  src/bos/usr/lib/methods/cfgcommon/cfg_runtime_br_rs6k.c, cfgmethods, bos411, 9430C411a 7/26/94 12:22:35";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfg_runtime_br_rs6k.c - used for mca cards or
 *							any device having mca
 *							bus resources
 *
 * FUNCTIONS: runtime_busresolve
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

#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>

/* Local header files */
#include "cfgcommon.h"
#include "cfgdebug.h"

int
runtime_busresolve()
{
	char	sstring[256];		/* search criteria pointer */
	char	conflist[1024];		/* busresolve() configured devices */
	char	not_resolved[1024];	/* busresolve() not resolved devices */
	ushort	devid;			/* Device id - used at run-time */
	int	slot;			/* slot of adapters */
	int	rc;			/* return codes go here */



	if (!strcmp(pddv.subclass,"mca")) {
		/* Make sure card is in specified slot */
		slot = atoi(cudv.connwhere);
		devid = (ushort) strtol(pddv.devid,(char **) NULL,0);
		sprintf (sstring,"/dev/%s",cudv.parent);
		rc = chkslot(sstring,slot,devid);
		if (rc != 0) {
			DEBUG_2("card %s not found in slot %d\n",
						cudv.name,slot);
			return(rc);
		}
	}

        /* Invoke Bus Resolve  */
        rc = busresolve(cudv.name,(int)0,conflist, not_resolved, NULL);
        DEBUG_1("runtime_busresolve: rc from bus_resolve is %d\n",rc)
        return(rc);

}
