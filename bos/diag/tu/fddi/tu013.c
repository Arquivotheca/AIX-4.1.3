static char sccsid[] = "@(#)64  1.3  src/bos/diag/tu/fddi/tu013.c, tu_fddi, bos411, 9428A410j 11/4/93 11:07:10";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: busdata_three
 *              tu013
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

Function(s) Test Unit 013 - Read/Write NP Bus Data Store with Three Transfers

Module Name :  tu013.c
SCCS ID     :  1.2

Current Date:  5/23/90, 11:17:56
Newest Delta:  1/19/90, 16:31:35

This test unit will write data to and read data from the adapter NP bus data
store area using three buffers.

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

busdata_three

*****************************************************************************/

int busdata_three(fdes, tucb_ptr)
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
        detrace(0,"--- NP Bus Data Store (Three Transfers) Test begins ---\n");
#endif

        /*
         * Create a test pattern into three buffers and write these buffers to
         * NP Bus Data Store. Read NP Bus Data Store into another three buffers
         * and compare these buffers with the original three buffers.
         * If errors, report them.
         */

        rc = rw_allmem(fdes, FDDI_WR_MEM_NP_BUS_DATA, FDDI_RD_MEM_NP_BUS_DATA,
                       NP_BUS_DATA_OFF, 3, tucb_ptr);
        if (rc)
           {
              switch (rc)
               {
                case WRITE_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Data Store (Three Transfers) written ");
                   detrace(0, "incorrectly.   rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT3_WR_ERR));

                case READ_ERROR:
#ifdef debug
                   detrace(0,"NP Bus Data Store (Three Transfers) read");
                   detrace(0, "incorrectly.    rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT3_RD_ERR));

                case COMPARE79_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Data Store (Three Transfers - 1st ");
                   detrace(0,"buffer) compared incorrectly.   rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT1_CMP_ERR));

                case COMPARE35_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Data Store (Three Transfers - 2nd ");
                   detrace(0,"buffer) compared incorrectly.   rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT2_CMP_ERR));

                case COMPARE80_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Data Store (Three Transfers - 3rd ");
                   detrace(0,"buffer) compared incorrectly.    rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT3_CMP_ERR));

                default:
                   break;
               }
           }

#ifdef debugg
        detrace(0,"--- End of NP Bus Data Store (Three Transfers) Test ---\n");
#endif
        return(0);
   }


/*****************************************************************************

tu013

*****************************************************************************/

int tu013 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;

        /*
         * Call Read/Write NP Bus Data Store (Three Transfer) test.
         */

        rc = busdata_three(fdes, tucb_ptr);

        return(rc);
   }
