static char sccsid[] = "@(#)66  1.3  src/bos/diag/tu/fddi/tu015.c, tu_fddi, bos411, 9428A410j 11/4/93 11:07:18";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: busprg_two
 *              tu015
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

Function(s) Test Unit 015 - Read/Write NP Bus Program Store with Two Transfers

Module Name :  tu015.c
SCCS ID     :  1.2

Current Date:  5/23/90, 11:17:56
Newest Delta:  1/19/90, 16:31:35

This test unit will write data to and read data from the adapter NP bus
program store area using two buffers.

*****************************************************************************/

#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include "diagddfddiuser.h"
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif
extern int rw_allmem();


/*****************************************************************************

busprg_two

*****************************************************************************/

int busprg_two(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
        detrace(0,"--- NP Bus Program Store (Two Transfers) Test begins ---\n");
#endif

        /*
         * Create a test pattern into two buffers and write these buffers to
         * NP Bus Program Store. Read NP Bus Program Store into another two
         * buffers and compare these buffers with the original two buffers.
         * If errors, report them.
         */

        rc = rw_allmem(fdes, FDDI_WR_MEM_NP_BUS_PROGRAM, FDDI_RD_MEM_NP_BUS_PROGRAM, 0,
                                2, tucb_ptr);
        if (rc)
           {
              switch (rc)
               {
                case WRITE_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Program Store (Two Transfers) written ");
                   detrace(0, "incorrectly.   rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSPRG2_WR_ERR));

                case READ_ERROR:
#ifdef debug
                   detrace(0,"NP Bus Program Store (Two Transfers) read");
                   detrace(0, "incorrectly.    rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSPRG2_RD_ERR));

                case COMPARE35_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Program Store (Two Transfers - 1st ");
                   detrace(0,"buffer) compared incorrectly.  rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSPRG1_CMP_ERR));

                case COMPARE46_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Program Store (Two Transfers - 2nd ");
                   detrace(0,"buffer) compared incorrectly.  rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSPRG2_CMP_ERR));

                default:
                   break;
               }
           }

#ifdef debugg
        detrace(0,"--- End of NP Bus Data Program (Two Transfers) Test ---\n");
#endif
        return(0);
   }


/*****************************************************************************

tu015

*****************************************************************************/

int tu015 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;

        /*
         * Call Read/Write NP Bus Program Store (Two Transfers) test.
         */

        rc = busprg_two(fdes, tucb_ptr);

        return(rc);
   }
