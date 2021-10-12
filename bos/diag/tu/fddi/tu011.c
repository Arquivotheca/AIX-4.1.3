static char sccsid[] = "@(#)62  1.3  src/bos/diag/tu/fddi/tu011.c, tu_fddi, bos411, 9428A410j 11/4/93 11:07:04";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: busdata_one
 *              tu011
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

Function(s) Test Unit 011 - Read/Write NP Bus Data Store using One Transfer

Module Name :  tu011.c
SCCS ID     :  1.2

Current Date:  5/23/90, 11:17:56
Newest Delta:  1/19/90, 16:31:35

This test unit will write data to and read data from the adapter NP bus
data store area using one buffer.

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

busdata_one

*****************************************************************************/

int busdata_one(fdes, tucb_ptr)
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
        detrace(0,"--- NP Bus Data Store (One Transfer) Test begins ---\n");
#endif

        /*
         * Create a test pattern into a buffer and write this buffer to
         * NP Bus Data Store. Read NP Bus Data Store into another buffer
         * and compare this buffer with the original buffer.
         * If errors, report them.
         */

        rc = rw_allmem(fdes, FDDI_WR_MEM_NP_BUS_DATA, FDDI_RD_MEM_NP_BUS_DATA,
                       NP_BUS_DATA_OFF, 1, tucb_ptr);
        if (rc)
           {
              switch (rc)
               {
                case WRITE_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Data Store (One Transfer) written ");
                   detrace(0, "incorrectly.   rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT1_WR_ERR));

                case READ_ERROR:
#ifdef debug
                   detrace(0,"NP Bus Data Store (One Transfer) read");
                   detrace(0, "incorrectly.    rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT1_RD_ERR));

                case COMPARE12_ERROR:
#ifdef debugg
                   detrace(0,"NP Bus Data Store (One Transfer) compared ");
                   detrace(0,"incorrectly.    rc = %x\n", rc);
#endif
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                  BUSDAT1_CMP_ERR));

                default:
                   break;
               }
           }

#ifdef debugg
        detrace(0,"--- End of NP Bus Data Store (One Transfer) Test ---\n");
#endif
        return(0);
   }


/*****************************************************************************

tu011

*****************************************************************************/

int tu011 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, result;

        /*
         * Call Read/Write NP Bus Data Store (One Transfer) test.
         */

        rc = busdata_one(fdes, tucb_ptr);

        return(rc);
   }
