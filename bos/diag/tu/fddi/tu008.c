static char sccsid[] = "@(#)59  1.3  src/bos/diag/tu/fddi/tu008.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:54";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: fddiram_one
 *              tu008
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

Function(s) Test Unit 008 - Write/Read RAM Buffer with One Transfer

Module Name :  tu008.c
SCCS ID     :  1.20

Current Date:  5/23/90, 11:17:55
Newest Delta:  3/26/90, 16:25:54

This test unit writes and reads data from the adapter RAM buffer using a
single data buffer. The data written and read will be compared to insure
correct operation.

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

fddiram_one

*****************************************************************************/

int fddiram_one(fdes, tucb_ptr)
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
        detrace(0,"--- FDDI RAM (One Transfer) Test begins ---\n");
#endif

        /*
         * Create a test pattern in buf1 and write buf1 to FDDI RAM.
         * Read FDDI RAM into buf2 and compare buf1 with buf2.
         * If errors, report them. If correct, continue until all
         * patterns have been tested.
         */

        rc = rw_allmem(fdes, FDDI_WR_MEM_FDDI_RAM, FDDI_RD_MEM_FDDI_RAM, 0, 1, tucb_ptr);
        if (rc)
           {
              switch (rc)
               {
                case WRITE_ERROR:
#ifdef debugg
                   detrace(0,"FDDI RAM (One Transfer) written incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  FDDIRAM1_WR_ERR));

                case WR_ER_DATASTORE:
#ifdef debugg
                   detrace(0,"Set up of FDDI RAM (One Transfer) written ");
                   detrace(0,"incorrectly. rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  DATASTOR_WR_ERR));

                case READ_ERROR:
#ifdef debug
                   detrace(0,"FDDI RAM (One Transfer) read incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  FDDIRAM1_RD_ERR));

                case RD_ER_DATASTORE:
#ifdef debug
                   detrace(0,"Completed set up of FDDI RAM (One Transfer) ");
                   detrace(0,"read incorrectly.   rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  DATASTOR_RD_ERR));

                case COMPARE12_ERROR:
#ifdef debugg
                   detrace(0,"FDDI RAM (One Transfer) compared incorrectly.");
                   detrace(0," rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  FDDIRAM1_CMP_ERR));

                case COMPARE_RBC:
#ifdef debugg
                   detrace(0,"Return status of RBC compared incorrectly.");
                   detrace(0,"  rc = %x\n", rc);
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
        detrace(0,"--- End of FDDI RAM (One Transfer) Test ---\n");
#endif
        return(0);
   }


/*****************************************************************************

tu008

*****************************************************************************/

int tu008 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;

        /*
         * Call Read/Write FDDI RAM (One Transfer) test.
         */

        rc = fddiram_one(fdes, tucb_ptr);

        return(rc);
   }
