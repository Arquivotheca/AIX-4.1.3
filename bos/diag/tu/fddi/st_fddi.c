static char sccsid[] = "@(#)51  1.3  src/bos/diag/tu/fddi/st_fddi.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:29";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: start_fddi
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

Function(s) Start FDDI Adapter

Module Name :  st_fddi.c
SCCS ID     :  1.9

Current Date:  5/23/90, 11:17:52
Newest Delta:  2/27/90, 14:36:14

*****************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <errno.h>
#include "diagddfddiuser.h"
#include "fdditst.h"    /* note that this also includes hxihtx.h */
#define MAX_START_ATTEMPTS      16
#define SLEEP_TIME               1

#ifdef debugg
extern void detrace();
#endif

int start_fddi (fdes, sess_sp, tucb_ptr)
   int fdes;
   struct session_blk *sess_sp;
   TUTYPE *tucb_ptr;
   {
        register int i;
        int rc;
        unsigned long status;
        struct status_block stat_s;
        extern int mktu_rc();

        /*
         * start up the adapter with the session block
         * PREVIOUSLY initialized (pointed to by sess_sp).
         */
        if (ioctl(fdes, CIO_START, sess_sp) < 0)
           {
                if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
                    (tucb_ptr->fddi_s.htx_sp != NULL))
                        (tucb_ptr->fddi_s.htx_sp->bad_others)++;

                if (errno == EIO)
                        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                                        START_FAIL));

                return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
           }
        if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
            (tucb_ptr->fddi_s.htx_sp != NULL))
                (tucb_ptr->fddi_s.htx_sp->good_others)++;

        for (i = 0; i < MAX_START_ATTEMPTS; i++)
           {
                /*
                 * insure that adapter started properly AND
                 * grab the network address from the status block
                 */
                if (ioctl(fdes, CIO_GET_STAT, &stat_s) < 0)
                   {
                        if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
                            (tucb_ptr->fddi_s.htx_sp != NULL))
                                (tucb_ptr->fddi_s.htx_sp->bad_others)++;

                        if (errno == EIO)
                                return(mktu_rc(tucb_ptr->header.tu,
                                                LOG_ERR, GET_STATUS_ERR));

                        return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
                   }
                if ((tucb_ptr->header.mfg == INVOKED_BY_HTX) &&
                    (tucb_ptr->fddi_s.htx_sp != NULL))
                        (tucb_ptr->fddi_s.htx_sp->good_others)++;

                if ((stat_s.code & 0x0000ffff) == CIO_START_DONE)
                        break;

                sleep(SLEEP_TIME);
           }

        if (i == MAX_START_ATTEMPTS)
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, START_TIME_ERR));

        if (stat_s.option[0] != CIO_OK)
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, START_DONE_ERR));

        return(0);
   }
