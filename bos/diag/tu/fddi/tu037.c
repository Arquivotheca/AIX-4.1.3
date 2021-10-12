static char sccsid[] = "@(#)88	1.1  src/bos/diag/tu/fddi/tu037.c, tu_fddi, bos411, 9428A410j 7/9/91 12:48:09";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: disconnect
 *		tu037
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Function(s) Test Unit 037 - Disconnect

Module Name :  tu037.c
SCCS ID     :  1.11

Current Date:  5/23/90, 11:17:52
Newest Delta:  4/10/90, 18:48:20

This test will issue a close command.  A close will disconnect the adapter
from the network.  An open will be issued to re-establish the communication
between the application and the device driver.

*****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif


/*****************************************************************************

disconnect
This routine will disconnect the device driver from the adapter.

*****************************************************************************/

int disconnect(fdes, tucb_ptr)
   int    *fdes;
   TUTYPE *tucb_ptr;
   {
	unsigned long   status;
	struct htx_data *htx_sp;
	int             rc;

	/*
         * Set up a pointer to HTX data structure to increment
	 * counters in case TU was invoked by hardware exerciser.
	 */

	htx_sp = tucb_ptr->fddi_s.htx_sp;

	/*
	 * Set up the Disconnect command and write this to the hcr via ioctl.
	 */

	close(*fdes);

	*fdes = open(htx_sp->sdev_id,O_RDWR | O_NDELAY);
	if (*fdes < 0)
	{
#ifdef debugg
	detrace(0,"tu037: Open FAILED on device \"%s\",\n",htx_sp->sdev_id);
#endif
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, OPEN_DD_ERR));

	    return(1);
	}
#ifdef debugg
	detrace(0,"tu037: Open PASSED on device \"%s\",\n",htx_sp->sdev_id);
#endif
        if (tucb_ptr->fddi_s.htx_sp != NULL)
		tucb_ptr->fddi_s.htx_sp->good_others++;
	return(0);
}

/*****************************************************************************

tu037

*****************************************************************************/

int tu037 (fdes, tucb_ptr)
   int    *fdes;
   TUTYPE *tucb_ptr;
   {
	int       rc;

	rc = disconnect(fdes, tucb_ptr);
	return(rc);
   }
