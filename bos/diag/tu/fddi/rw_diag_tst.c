static char sccsid[] = "@(#)47  1.1.1.3  src/bos/diag/tu/fddi/rw_diag_tst.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:19";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: rw_diag_tst
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

Function(s) rw_diag_tst - Write/Read to adapter diagnostic tests

Module Name :  rw_diag_tst.c
SCCS ID     :  1.20

Current Date:  5/23/90, 11:17:55
Newest Delta:  3/26/90, 16:25:54


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

extern int chk_diag_tst();


/*****************************************************************************

rw_diag_tst

*****************************************************************************/

int rw_diag_tst(fdes, diag_tst, tucb_ptr)
   int fdes;
   ushort diag_tst;
   TUTYPE *tucb_ptr;
   {
        int             rc, i, k, reg_data;
        unsigned int    diag_lst;
        unsigned long   status;
        fddi_hcr_cmd_t  diag;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * Set up ioctl for write, then write buffer(s) with test pattern
         * to appropiate area.  Sizeof returns bytes while cpb is made up
         * of 2 byte entities, therefore divide by 2.
         */
        diag.hcr_val = diag_tst;
        diag.l_cpb = 0;

        status = 0;
        rc = ioctl(fdes, FDDI_HCR_CMD, &diag);
        status = diag.status;

        if (rc || diag.status )
         {
#ifdef debugg
           detrace(0,"HCR Command written incorrectly.    rc = %x\n", rc);
           detrace(0,"Returned status = %x\n", status);
#endif
           if (htx_sp != NULL)
              (htx_sp->bad_others)++;
           return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, HCR_CMD_WR_ERR));
         }
        else
         if (htx_sp != NULL)
            (htx_sp->good_others)++;
#ifdef debugg
        detrace(0,"Executed HCR Command = %x.\n", diag_tst);
#endif
        /*
         * Read Memory Transfer.  Set up list of diagnostic results to check;
         */

        diag_lst = 1 << diag_tst;
        rc = chk_diag_tst (fdes, diag_lst, tucb_ptr);
        if (rc)
                return(rc);

        return(0);
   }
