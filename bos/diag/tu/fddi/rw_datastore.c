static char sccsid[] = "@(#)46  1.1.1.5  src/bos/diag/tu/fddi/rw_datastore.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:15";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: wr_datastore
 *              rd_datastore
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

Function(s) rw_datastore - Initialize NP Bus Data Store locations for
                           FDDI RAM Buffer tests.

Module Name :  rw_datastore.c
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


/*****************************************************************************

wr_datastore

*****************************************************************************/

int wr_datastore(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, i, reg_data;
        int             offset;
        unsigned long   status;
        unsigned char   buf1[RBC_BYTES];
        fddi_mem_acc_t  ram;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * Create patterns and place them in appropiate buffers.
         */
        for (i = 1; i <= NUM_INSTRUCT; i++)
          {
           switch (i)
            {
             case 1:     /* Case -  Software Reset of RBC   */
              reg_data = SOFT_RESET_RBC;
              offset = SOFT_RESET_OFF;
#ifdef debugg
              detrace(0,"Writing Software Reset of RBC.\n");
#endif
              break;

             case 2:     /* Case - RBC Mode Register */
              reg_data = RBC_MODE;
              offset = RBC_OFFSET;
#ifdef debugg
              detrace(0,"Writing RBC Mode Register.\n");
#endif
              break;

             case 3:       /* Case - Read Pointer for Receive frames. (RPR) */
              reg_data = RPR;
              offset = RPR_OFFSET;
#ifdef debugg
              detrace(0,"Writing Read Pointer for receive frames. (RPR)\n");
#endif
              break;

             case 4:     /* Case - Write Pointer for transmit frames. (WPX) */
              reg_data = WPX;
              offset = WPX_OFFSET;
#ifdef debugg
              detrace(0,"Writing Write Pointer for transmit frames. (WPX)\n");
#endif
              break;

             case 5:    /* Case - Write Pointer for receive frames. (WPR) */
              reg_data = WPR;
              offset = WPR_OFFSET;
#ifdef debugg
              detrace(0,"Writing Write Pointer for receive frames. (WPR)\n");
#endif
              break;

             case 6:     /* Case - Start Address for receive FIFO. (SAR) */
              reg_data = SAR;
              offset = SAR_OFFSET;
#ifdef debugg
              detrace(0,"Writing Start Address for receive FIFO. (SAR)\n");
#endif
              break;

             case 7:     /* Case - End Address for receive FIFO. (EAR) */
              reg_data = EAR;
              offset = EAR_OFFSET;
#ifdef debugg
              detrace(0,"Writing End Address for receive FIFO. (EAR)\n");
#endif
              break;

             default:
              break;

             }
            /*
             * Create buffer with appropiate command to write to Data Store
             * locations.
             */

#ifdef debugg
            detrace(0,"reg_data = %x\n", reg_data);
#endif
            buf1[0] = (char) ((reg_data & 0xFF00) >> 8);
            buf1[1] = (char) reg_data & 0x00FF;
            buf1[2] = 0;
            buf1[3] = 0;

            /*
             * Set up ioctl for write, then write buffer(s) with test pattern
             * to appropiate area.
             */

            ram.opcode = FDDI_WR_MEM_NP_BUS_DATA;
            ram.ram_offset = offset;
            ram.num_transfer = 1;
            ram.buffer_1 = buf1;
            ram.buff_len1 = RBC_BYTES;

            status = 0;
            rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
            status = ram.status;
            if (rc)
             {
#ifdef debugg
                detrace(0,"Data Store locations written incorrectly.  ");
                detrace(0,"rc = %x\n", rc);
#endif
                if (htx_sp != NULL)
                   (htx_sp->bad_others)++;
                return(WR_ER_DATASTORE);
             }
            else
                if (htx_sp != NULL)
                   (htx_sp->good_others)++;
#ifdef debugg
           detrace(0, "buf1[0] = %02x, buf1[1] =  %02x\n", buf1[0], buf1[1]);
#endif
          }
        return (0);
}

/*****************************************************************************

rd_datastore

*****************************************************************************/

int rd_datastore(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, i, reg_data;
        int             offset;
        unsigned long   status;
        unsigned char   buf1[RBC_BYTES];
        fddi_mem_acc_t  ram;
        struct htx_data *htx_sp;


        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        for (i = 1; i <= NUM_INSTRUCT; i++)
         {
          switch (i)
           {
            case 1:   /* Software Reset of RBC */
              offset = SOFT_RESET_OFF;
              break;

            case 2:   /* RBC Mode Register */
              offset = RBC_OFFSET;
              break;

            case 3: /* Read Pointer for Receive Frames. (RPR)*/
              offset = RPR_OFFSET;
              break;

            case 4:   /* Write Pointer for Transmit Frames (WPX) */
              offset = WPX_OFFSET;
              break;

            case 5:   /* Write Pointer for Receive Frames (WPR) */
              offset = WPR_OFFSET;
              break;

            case 6:   /* Start Address for Receive FIFO (SAR) */
              offset = SAR_OFFSET;
              break;

            case 7:   /* End Address for Receive FIFO (EAR) */
              offset = EAR_OFFSET;
              break;

            default:
              break;
           }

          /*
           * Read Data Store Memory locations.
           */

          ram.opcode = FDDI_RD_MEM_NP_BUS_DATA;
          ram.ram_offset = offset;
          ram.num_transfer = 1;
          ram.buffer_1 = buf1;
          ram.buff_len1 = RBC_BYTES;

          status = 0;
          rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
          status = ram.status;

          if (rc)
           {
#ifdef debugg
             detrace(0,"Data Store Memory locations read ");
             detrace(0,"incorrectly. rc = %x\n", rc);
#endif
             if (htx_sp != NULL)
                (htx_sp->bad_others)++;
             return(RD_ER_DATASTORE);
           }
          else
             if (htx_sp != NULL)
                (htx_sp->good_others)++;
#ifdef debugg
          detrace(0,"Read - Data Store Memory locations.\n");
                  detrace(0,"Status = %x\n",ram.status);
          detrace(0, "buf1[0] = %02x,  buf1[1] = %02x\n", buf1[0], buf1[1]);
          detrace(0, "buf1[2] = %02x,  buf1[3] = %02x\n", buf1[2], buf1[3]);
#endif
          /*
           * Compare data in Software Reset of RBC and the RBC register.
           * If errors, report them and stop test.
           */
          reg_data = (int) buf1[0];
          reg_data = (reg_data & 0x00FF) << 8;
          reg_data = reg_data + buf1[1];

          switch (i)
            {
             case 1:      /* Software Reset of RBC  */
          /* bit 7 of the software reset register is unused.
           * make sure it is zero here before we compare so that it
           * will not cause any problems
           */
/* due to an AMD change in the REV B RBC module the 12th bit of the static  */
/* status register can not be tested.  REV A returns this bit as a 0 and    */
/* REV B returns this bit as a 1.  Therefor it is masked off                */
/* Might as well mask off all of the unused bits (15 - 7) of static reg     */
                reg_data = reg_data & 0x7F00;
                if (reg_data != SOFT_RESET_RBC)
                  {
                    if (htx_sp != NULL)
                       (htx_sp->bad_others)++;
/*                  return(COMPARE_SRESET);*/
                  }
#ifdef debugg
                detrace(0,"Software Reset of RBC compared correctly.\n");
#endif
                break;

             case 2:      /* Dynamic status and RBC Mode Register */
                if (reg_data != RBC_MODE)
                  {
                    if (htx_sp != NULL)
                       (htx_sp->bad_others)++;
/*                  return(COMPARE_RBC);*/
                  }
#ifdef debugg
                detrace(0,"RBC Mode Register compared correctly.\n");
#endif
                break;

             case 3:   /* RPR case */
#ifdef debugg
                detrace(0,"reg data for rpr reg = %x\n",reg_data);
                detrace(0,"RPR read.\n");
#endif
                break;

             case 4:   /* WPX case */
#ifdef debugg
                detrace(0,"reg data for wpx reg = %x\n",reg_data);
                detrace(0,"WPX read.\n");
#endif
                break;

             case 5:   /* WPR case */
#ifdef debugg
                detrace(0,"reg data for wpr reg = %x\n",reg_data);
                detrace(0,"WPR read.\n");
#endif
                break;

             case 6:   /* SAR case */
#ifdef debugg
                detrace(0,"reg data for sar reg = %x\n",reg_data);
                detrace(0,"SAR read.\n");
#endif
                break;

             case 7:   /* EAR case */
#ifdef debugg
                detrace(0,"reg data for ear reg = %x\n",reg_data);
                detrace(0,"EAR read.\n");
#endif
                break;

              default:
                break;

            }
          }
        return(0);
   }
