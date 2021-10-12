static char sccsid[] = "@(#)87  1.1.1.3  src/bos/diag/tu/fddi/tu036.c, tu_fddi, bos411, 9428A410j 11/4/93 11:08:44";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: start_test
 *              tu036
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

Function(s) Test Unit 036 - Connect

Module Name :  tu036.c
SCCS ID     :  1.11

Current Date:  5/23/90, 11:17:52
Newest Delta:  4/10/90, 18:48:20

This test will issue a connect to the device driver.

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

start_test
This function will issue a CIO_START.  The device driver will automatically
issue a connect upon reciept of the first start.
*****************************************************************************/

int start_test(fdes , tucb_ptr)
   int    fdes;
   TUTYPE *tucb_ptr;
   {
        unsigned long   status;
        struct   htx_data *htx_sp;
        struct   session_blk      session;
        int      rc = TRUE;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */

        htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
        detrace(0,"\n--- Issue START Command test begins. --------\n");
#endif

        /*
         * Set up netid and call fddi_start to issue START command
         */

        session.netid = FDDI_NETID;
        session.length = 1;
        session.status = 0;

        rc = ioctl(fdes,CIO_START,&session);
        if ((rc < 0) && (session.status != 0))
        {                                       /* if error */

#ifdef debugg
        detrace(0,"Failed START command.\n");
        detrace(0,"Returned status = %x\n", session.status);
        detrace(0,"Issue a HALT command.\n");
#endif
            if (tucb_ptr->fddi_s.htx_sp != NULL)
                tucb_ptr->fddi_s.htx_sp->bad_others++;

            if (errno == EIO)
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, START_FAIL));

            return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));

        }
        else
        {
#ifdef debugg
            detrace(0,"\n\t -- IOCTL Start succeeded --");
#endif
        if (tucb_ptr->fddi_s.htx_sp != NULL)
                tucb_ptr->fddi_s.htx_sp->good_others++;

        }

        return(0);

}

/*****************************************************************************

tu036

*****************************************************************************/

int tu036 (fdes, tucb_ptr)
   int    fdes;
   TUTYPE *tucb_ptr;
   {
        int       rc;

        rc = start_test(fdes, tucb_ptr);
        return(rc);
   }
