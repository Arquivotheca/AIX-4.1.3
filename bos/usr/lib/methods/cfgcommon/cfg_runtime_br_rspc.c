static char sccsid[] = "@(#)10  1.2  src/bos/usr/lib/methods/cfgcommon/cfg_runtime_br_rspc.c, cfgmethods, bos411, 9430C411a 7/26/94 12:23:05";
/*
 * COMPONENT_NAME: (CFGMETHODS) cfg_runtime_br_rspc.c - used for any device 
 *						        having PCI or ISA bus
 *						        resources
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
#include <sys/mdio.h>
#include <fcntl.h>
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
	ulong	devid;			/* Device id - used at run-time */
	int	slot;			/* slot of adapters */
	int	rc;			/* return codes go here */
        MACH_DD_IO      mddRecord;
        int     fd;
	ulong   cardid;


	if (!strcmp(pddv.subclass,"pci")) {
		/* Make sure card is in specified slot */

		slot = atoi(cudv.connwhere);
		devid = (ulong) strtoul(pddv.devid,(char **) NULL,0);
		sprintf (sstring,"/dev/%s",cudv.parent);

        	if (0 > (fd = open(sstring, O_RDWR))) {
                	return (fd);
       		}
		/* open machine dd and get devid from slot
		   and compare with above devid */

        	mddRecord.md_size = 1;  /* build mdd record */
        	mddRecord.md_incr = MV_WORD;
        	mddRecord.md_addr = 0;
        	mddRecord.md_sla  = slot;
                mddRecord.md_data = (uchar*)&cardid;

                if (ioctl(fd, MIOPCFGET, &mddRecord) < 0   ||
		   (cardid != devid) ) {
			DEBUG_2("card %s not found in slot %d\n",
					        pddv.devid, slot);	
			close(fd);
			return(E_NODETECT);
		}
		close(fd);
	}

	/* Invoke Bus Resolve  */
	rc = busresolve(cudv.name,(int)0,conflist, not_resolved, NULL);
	DEBUG_1("runtime_busresolve: rc from bus_resolve is %d\n",rc)
	return(rc);
}
