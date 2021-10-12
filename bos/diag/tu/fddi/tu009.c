static char sccsid[] = "@(#)60  1.3  src/bos/diag/tu/fddi/tu009.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:57";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: fddiram_two
 *              tu009
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

Function(s) Test Unit 009 - Write/Read RAM Buffer using Two Transfers

Module Name :  tu009.c
SCCS ID     :  1.2

Current Date:  5/23/90, 11:17:55
Newest Delta:  1/19/90, 16:31:13

This test unit writes and reads data from the adapter RAM buffer using two
data buffers. Data written and read will be compared for correct operation.

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
extern int rw_allmem();


/*****************************************************************************

fddiram_two

*****************************************************************************/

int fddiram_two(fdes, tucb_ptr)
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
        detrace(0,"--- FDDI RAM (Two Transfers) Test begins ---\n");
#endif

        /*
         * Create a test pattern in two buffers and write these two
         * buffers to FDDI RAM. Read FDDI RAM into another two buffers
         * and compare the read buffers with the write buffers.
         * If errors, report them. If correct, continue until all
         * patterns have been tested.
         */

        rc = rw_allmem(fdes, FDDI_WR_MEM_FDDI_RAM, FDDI_RD_MEM_FDDI_RAM, 0, 2, tucb_ptr);
        if (rc)
           {
              switch (rc)
               {
                case WRITE_ERROR:
#ifdef debugg
                   detrace(0,"FDDI RAM (Two Transfer) written incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  FDDIRAM2_WR_ERR));

                case WR_ER_DATASTORE:
#ifdef debugg
                   detrace(0,"Set up of FDDI RAM (Two Transfers) written ");
                   detrace(0,"incorrectly. rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  DATASTOR_WR_ERR));

                case READ_ERROR:
#ifdef debug
                   detrace(0,"FDDI RAM (Two Transfers) read incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  FDDIRAM2_RD_ERR));


                case RD_ER_DATASTORE:
#ifdef debug
                   detrace(0,"Completed set up of FDDI RAM (Two Transfers) ");
                   detrace(0,"read incorrectly.   rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  DATASTOR_RD_ERR));

                case COMPARE35_ERROR:
#ifdef debugg
                   detrace(0,"FDDI RAM (Two Transfers) compared incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  FDDIRAM1_CMP_ERR));

                case COMPARE46_ERROR:
#ifdef debugg
                   detrace(0,"FDDI RAM (Two Transfers) compared incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  FDDIRAM2_CMP_ERR));


                case COMPARE_RBC:
#ifdef debugg
                   detrace(0,"Return status of RBC compared incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  RBC_CMP_ERR));

                case COMPARE_SRESET:
#ifdef debugg
                   detrace(0,"Return status of Software Reset compared ");
                   detrace(0,"incorrectly.    rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  SWRESET_CMP_ERR));

                default:
                   break;
               }
           }

#ifdef debugg
        detrace(0,"--- End of FDDI RAM (Two Transfers) Test ---\n");
#endif
        return(0);
   }


/*****************************************************************************

tu009

*****************************************************************************/

int tu009 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;

        /*
         * Call Read/Write FDDI RAM (Two Transfers) test.
         */

        rc = fddiram_two(fdes, tucb_ptr);

        return(rc);
   }
