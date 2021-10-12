static char sccsid[] = "@(#)56  1.3  src/bos/diag/tu/fddi/tu005.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:41";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: end_session
 *              tu005
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

Function(s) Test Unit 005 - End Session

Module Name :  tu005.c
SCCS ID     :  1.10

Current Date:  5/29/90, 08:55:31
Newest Delta:  5/29/90, 08:54:52

This test unit issues a HALT command to the device driver. The device driver
deletes the NETID for that session. The device driver must be closed and
reopended to continue any additional testing.

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <errno.h>
#include "diagddfddiuser.h"
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif

/*****************************************************************************

end_session
   This routine issues a HALT command to the device driver.

*****************************************************************************/

int end_session(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int                     rc;
        struct session_blk      session;

        session.netid = FDDI_NETID;
        session.length = 1;
        session.status = 0;
        rc = ioctl(fdes, CIO_HALT, &session);
        if (rc < 0)
           {
#ifdef debugg
                detrace(0,"HALT failed.\n");
                detrace(0,"Returned value = %x\n", rc);
                detrace(0,"Returned status = %x\n", session.status);
#endif
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, HALT_ERR));
           }
        return(0);
   }


/*****************************************************************************

tu005

*****************************************************************************/

int tu005 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int     rc;

        rc = end_session(fdes, tucb_ptr);
        return(rc);
   }
