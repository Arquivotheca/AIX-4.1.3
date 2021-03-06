static char sccsid[] = "@(#)68  1.1.1.2  src/bos/diag/tu/fddi/tu017.c, tu_fddi, bos411, 9428A410j 11/4/93 11:07:28";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: wrap_single
 *              tu017
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

Function(s) Test Unit 017 - Wrap Data

Module Name :  tu017.c
SCCS ID     :  1.2

Current Date:  5/23/90, 11:17:56
Newest Delta:  1/19/90, 16:31:35

The Wrap Data test case transmits a frame which is wrapped through a wrap
plug and received. Once it is received, a check is made to verify that the
data received matches the data transmitted.

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

wrap_single

*****************************************************************************/

int wrap_single(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, reg_data;
        int             offset;
        ushort          hsr;
        unsigned char   buf1[RBC_BYTES];
        unsigned long   status;
        fddi_hcr_cmd_t  diag;
        fddi_mem_acc_t  ram;
        struct htx_data *htx_sp;
        int i;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
        detrace(0,"--- Wrap Test Class B begins ---\n");
#endif

        /*
         * Issue HCR command to begin loop test.
         */
        diag.hcr_val = CLASS_B_LOOP_TEST;
        diag.l_cpb = 0;

        status = 0;
        rc = ioctl(fdes, FDDI_HCR_CMD, &diag);
        status = diag.status;

        if (rc || diag.status)

           {
#ifdef debugg
                detrace(0,"HCR Class B Loop Test failed. rc =  %x\n", rc);
                detrace(0,"Returned status = %x\n", status);
#endif
                if (htx_sp != NULL)
                   (htx_sp->bad_others)++;
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, CLASS_B_LOOP_ER));
           }
        else
                if (htx_sp != NULL)
                   (htx_sp->good_others)++;

        hsr = diag.hsr_val;
        if (!(hsr & CCI_BIT))
          {
#ifdef debugg
                detrace(0,"Command Completion Interrupt bit not set. HSR = %x\n",
                           hsr);
#endif
                if (htx_sp != NULL)
                   (htx_sp->bad_others)++;
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, CLASS_B_INT_ER));
          }

        /*
         * Read Data Store Memory locations.
         */
        ram.opcode = FDDI_RD_SHARED_RAM;
        ram.ram_offset = 0;
        ram.num_transfer = 1;
        ram.buffer_1 = buf1;
        ram.buff_len1 = RBC_BYTES;

        status = 0;
        rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
        status = ram.status;

        if (rc)
         {
#ifdef debugg
           detrace(0,"Shared RAM read incorrectly. rc = %x\n", rc);
           detrace(0,"Returned status = %x\n", status);
#endif
           if (htx_sp != NULL)
              (htx_sp->bad_others)++;
           return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, SH_RAM_RD_ERR));
         }
        else
           if (htx_sp != NULL)
              (htx_sp->good_others)++;
#ifdef debugg
        detrace(0,"Shared RAM results of wrap test:\n");
        detrace(0, "buf1[0] = %02x,  buf1[1] = %02x\n", buf1[0], buf1[1]);
        detrace(0, "buf1[2] = %02x,  buf1[3] = %02x\n", buf1[2], buf1[3]);
#endif

        reg_data = buf1[0];
        reg_data = buf1[1] + ((reg_data & 0x00FF) << 8);
        if (reg_data != 0x0000)
          {
#ifdef debugg
            detrace(0,"FDDI Base Card wrap test failed. Return value = %x\n",
                       reg_data);
#endif
            if (htx_sp != NULL)
               (htx_sp->bad_others)++;
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, BASE_WRAP_ERR));
          }
        else
                if (htx_sp != NULL)
                   (htx_sp->good_others)++;

#ifdef debugg
        detrace(0,"--- End of Wrap Test Class B  ---\n");
#endif
        return(0);
   }


/*****************************************************************************

tu017

*****************************************************************************/

int tu017 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;

        /*
         * Call Base Card Wrap test.
         */

        rc = wrap_single(fdes, tucb_ptr);

        return(rc);
   }
