static char sccsid[] = "@(#)58  1.3  src/bos/diag/tu/fddi/tu007.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:49";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: rw_shram
 *              tu007
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

Function(s) Test Unit 007 - Write/Read Adapter Shared RAM

Module Name :  tu007.c
SCCS ID     :  1.13

Current Date:  5/23/90, 11:17:54
Newest Delta:  1/19/90, 16:30:44

This test unit writes and reads data from the adapter shared RAM. The data
written and read will be compared to insure correct operation.

POS registers will be accessed via the machine device driver.
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


/*****************************************************************************

rw_shram

*****************************************************************************/

int rw_shram(fdes,tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {

        int             rc, i, j, k;
        int             num_patterns;
        unsigned long   status;
        unsigned char   buf1[TOT_BYTES], buf2[TOT_BYTES];
        static unsigned char pattern[] =
           {
                0xDF, 0x1, 0x55, 0xAA, 0xFF
           };
        fddi_mem_acc_t  ram;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
        detrace(0,"\n-----  Shared RAM Test begins   ------\n");
#endif

        /*
         * Create a test pattern in buf1 and write buf1 to Shared RAM.
         * Read Shared RAM into buf2 and compare buf1 with buf2.
         * If errors, report them. If correct, continue until all
         * patterns have been tested.
         */

        num_patterns = sizeof(pattern);
        for (i = 0; i <= num_patterns; i++)
          {
            if (i < num_patterns)
                /* Patterns in character array pattern[]. */
                for (j = 0; j < TOT_BYTES; j++)
                    buf1[j] = pattern[i];
            else        /* Incrementing pattern */
                for (j = 0; j < TOT_BYTES; j++)
                            buf1[j] = j;

            /*
             * Set up ioctl for write, then write buf1 with test pattern
             * to Shared RAM area.
             */

            ram.opcode = FDDI_WR_SHARED_RAM;
            ram.ram_offset = 0;
            ram.num_transfer = 1;
            ram.buffer_1 = buf1;
            ram.buff_len1 = TOT_BYTES;
#ifdef debugg
            detrace(0,"Write to Shared RAM.\n");
            detrace(0,"Opcode = %x, Offset = %x, Transfers = %x\n",
                        ram.opcode, ram.ram_offset, ram.num_transfer);
            detrace(0,"Buffer_ptr = %x, Buffer_length = %x\n",
                        ram.buffer_1, ram.buff_len1);
#endif

            status = 0;
            rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
            status = ram.status;
            if (rc)
              {
#ifdef debugg
                detrace(0,"Shared RAM Transfer written incorrectly.\n");
                detrace(0,"Returned value, rc = %x\n", rc);
                detrace(0,"Returned status = %x\n", status);
#endif
                if (htx_sp != NULL)
                   (htx_sp->bad_others)++;
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                SH_RAM_WR_ERR));
              }
            else
                if (htx_sp != NULL)
                   (htx_sp->good_others)++;
#ifdef debugg
            detrace(0, "Wrote buf1 to Shared RAM.\n");
            detrace(0, "buf1:         ");
            if (i < num_patterns)
                detrace(0, "Pattern = %02x \n", pattern[i]);
            else
                detrace(0, "Incrementing Pattern. \n");
            for (k = 0; k < SHOW_BYTES; k++)
                detrace(0, "%02x ", buf1[k]);
            detrace(0, "\n");
#endif
            /*
             * Read Shared RAM.
             */
            ram.opcode = FDDI_RD_SHARED_RAM;
            ram.ram_offset = 0;
            ram.num_transfer = 1;
            ram.buffer_1 = buf2;
            ram.buff_len1 = TOT_BYTES;
#ifdef debugg
            detrace(0,"Read from Shared RAM.\n");
            detrace(0,"Opcode = %x, Offset = %x, Transfers = %x\n",
                        ram.opcode, ram.ram_offset, ram.num_transfer);
            detrace(0,"Buffer_ptr = %x, Buffer_length = %x\n",
                        ram.buffer_1, ram.buff_len1);
#endif

            status = 0;
            rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
            status = ram.status;

            if (rc)
              {
#ifdef debugg
                detrace(0,"Shared RAM read incorrectly.\n");
                detrace(0,"Returned value, rc = %x\n", rc);
                detrace(0,"Returned status = %x\n", status);
#endif
                if (htx_sp != NULL)
                        (htx_sp->bad_others)++;
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                        SH_RAM_RD_ERR));
                }
            else
                if (htx_sp != NULL)
                   (htx_sp->good_others)++;
#ifdef debugg
            detrace(0, "Read buf2 from Shared RAM.\n");
            detrace(0, "buf2: \n");
            for (k = 0; k < SHOW_BYTES; k++)
                detrace(0, "%02x ", buf2[k]);
            detrace(0, "\n");
#endif
            /*
             * Compare data in buf1 and buf2.
             * If errors, report them and stop test.
             */

            if (memcmp(buf1, buf2, TOT_BYTES) != 0)
              {
#ifdef debugg
                detrace(0,"Shared RAM compared incorrectly.\n");
#endif
                if (htx_sp != NULL)
                      (htx_sp->bad_others)++;
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                        SH_RAM_CMP_ERR));
              }

#ifdef debugg
            if (i < num_patterns)
                detrace(0,"Compared SUCCESSFULLY with pattern = %x\n",
                                   pattern[i]);
            else
                detrace(0,"Compared SUCCESSFULLY with incrementing pattern.\n");
            detrace(0,"\n");
#endif
           }
#ifdef debugg
        detrace(0, "-----   End of Shared RAM Test   -------\n");
#endif
        return(0);
   }


/*****************************************************************************

tu007

*****************************************************************************/

int tu007 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, result;

        /*
         * Call TU002 to insure adapter is in Normal mode.
         */
        rc = tu002(fdes, tucb_ptr);
        if (rc)
           {
#ifdef debugg
                detrace(0,"Failed placing adapter in normal mode.\n");
#endif
                return(rc);
           }

        /*
         * Call Read/Write Shared RAM test.
         */

        rc = rw_shram(fdes, tucb_ptr);

        return(rc);
   }
