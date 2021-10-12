static char sccsid[] = "@(#)70  1.3  src/bos/diag/tu/fddi/tu019.c, tu_fddi, bos411, 9428A410j 11/4/93 11:07:37";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: issue_start
 *              tu019
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

Function(s) Test Unit 019 - Issue a START command

Module Name :  tu019.c
SCCS ID     :  1.8

Current Date:  5/23/90, 11:17:54
Newest Delta:  1/19/90, 16:30:32


This test unit will issue a START.

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/err_rec.h>
#include <sys/intr.h>
#include <errno.h>
#include "diagddfddiuser.h"
#include "fdditst.h"    /* note that this also includes hxihtx.h */

#ifdef debugg
extern void detrace();
#endif

extern int start_fddi();
/*****************************************************************************

issue_start

*****************************************************************************/

int issue_start(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;
        struct session_blk session;

#ifdef debugg
        detrace(0,"\n--- Issue START Command test begins. --------\n");
#endif

        /*
         * Set up netid and call start_fddi to issue START command.
         */
        session.netid = FDDI_NETID;
        session.length = 1;
        session.status = 0;

        if (rc = start_fddi(fdes, &session, tucb_ptr))
         {
#ifdef debugg
            detrace(0,"Failed START command.\n");
            detrace(0,"Returned status = %x\n", session.status);
            detrace(0,"Issue a HALT command.\n");
#endif
            (void) ioctl(fdes, CIO_HALT, &session);
            return(rc);
         }

#ifdef debugg
        detrace(0,"START Command operated SUCCESSFULLY.\n");
        detrace(0,"--- End of Issue START test. --------\n");
#endif
        return(0);
   }


/*****************************************************************************

tu019

*****************************************************************************/

int tu019 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;

        rc = issue_start(fdes, tucb_ptr);
        return(rc);
   }
