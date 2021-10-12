static char sccsid[] = "@(#)64	1.4  src/bos/kernext/rcm/gscdpm.c, rcm, bos41J, 9520A_all 5/3/95 11:44:15";

/*
 *   COMPONENT_NAME: (rcm) Rendering Context Manager Aixgsc Syscall Mgr.
 *
 *   FUNCTIONS: gsc_dpm 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>			/* includes for all lft related data */
#include <sys/user.h>
#include <sys/adspace.h>
#include "rcm_mac.h" 
#include "xmalloc_trace.h"


gsc_dpm(pd, arg, parm1)

struct phys_displays *pd; 

int arg, 
    parm1;          /* display power state to go to*/

{
	int		rc = 0;

	struct _rcmProc *pproc;         /* pointer to rcm process structure */
        gscDev *pdev;                   /* pointer to gsc device structure */

	RCM_ASSERT (pd != NULL, 0, 0, 0, 0, 0);

	SET_PDEV(pd, pdev);

        if (pdev == NULL)
                return (EINVAL);

        FIND_GP(pdev, pproc);
        if (pproc == NULL)
                return (EINVAL);


	/* switch on the command issued */
	switch ( arg )
	{
	   case CHANGE_POWER_STATE:	

		if (pd->vttpwrphase != NULL)
		{
			rc = (*pd->vttpwrphase)(pd, parm1);

		  	if (rc) rc = EIO;
		}

		break;

	   default: rc = EINVAL; 
	}

	return(rc);
}
