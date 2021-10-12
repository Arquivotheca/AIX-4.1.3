/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: chk_diag_tst
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

Function(s) Check self test results

Module Name :  chk_diag_tst.c
SCCS ID     :  1.0

Current Date:  10/1/91, 11:17:54
Newest Delta:  10/1/91, 16:30:32

This routine will read shared RAM inorder to check the self test results.

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


int chk_diag_tst(fdes, diag_lst, tucb_ptr)
   int fdes;
   unsigned int diag_lst;
   TUTYPE *tucb_ptr;
   {
        int             rc, reg_data, i, k;
        int diag_tst;
        unsigned long   status;
        unsigned char   buf1[DIAG_BYTES];
        fddi_mem_acc_t  ram;
        struct htx_data *htx_sp;
        ushort reg_val;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

     /*
      * Check self test results
      */

#ifdef debugg
        detrace(0,"diag test list: %x\n",diag_lst);
#endif
    ram.opcode = FDDI_RD_SHARED_RAM;
        ram.ram_offset = 0;
        ram.num_transfer = 1;
        ram.buffer_1 = buf1;
        ram.buff_len1 = DIAG_BYTES;

        status = 0;
        rc = ioctl(fdes, FDDI_MEM_ACC, &ram);
        status = ram.status;

        if (rc)
        {
#ifdef debugg
        detrace(0,"Shared RAM read incorrectly.    rc = %x\n", rc);
    detrace(0,"Returned status = %x\n", status);
#endif
            if (htx_sp != NULL)
                   (htx_sp->bad_others)++;
            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, SH_RAM_RD_ERR));
        }
/*
        buf1[16] = (char) 0xEA;
        buf1[17] = (char) 0x80;
        buf1[18] = (char) 0x01;
        buf1[19] = (char) 0x90;
        buf1[20] = (char) 0xFF;
        buf1[21] = (char) 0xFF;
*/
#ifdef debugg
        detrace(0, "Read buf1 from Shared RAM.\n");
        detrace(0, "buf1:                            Adapter Test # \n");
        detrace(0, " |   0  |   1  |   2  |   3  |   4  |   5  |   6  |");
        detrace(0, "   7  |   8  |   9  |  10  |\n");
        detrace(0, " | ");
        for (k = 0; k < DIAG_BYTES; k++)
        {
                detrace(0, "%02x", buf1[k]);
                if (k%2)
                        detrace(0," | ");
        }
        detrace(0, "\n\n");
#endif

        for (diag_tst = 0; diag_tst < DIAG_BYTES/2; diag_tst++)
        {
                if (diag_lst & (1 << diag_tst))
                {
                reg_data = (int) buf1[(diag_tst*2) + 1];
                reg_data = (reg_data & 0x00FF) << 8;
                reg_data = reg_data + (int) buf1[diag_tst*2];

                if (reg_data != 0x0000)
                        switch (diag_tst)
                        {
                            case 0:   /* Test 0 - Node Processor Instruction */
#ifdef debugg
                  detrace(0,"Diagnostic Test 0 (Node Processor Instruction)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                          if (htx_sp != NULL)
                             (htx_sp->bad_others)++;
                          return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_0_P_FRU));
                                  break;

                        case 1:   /* Test 1 - ALISA Interface */
#ifdef debugg
                  detrace(0,"Diagnostic Test 1 (ALISA Interface)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                                  if (htx_sp != NULL)
                                 (htx_sp->bad_others)++;
                                  return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_1_P_FRU));
                                  break;

                            case 2:   /* Test 2 - VPD CRC */
#ifdef debugg
                  detrace(0,"Diagnostic Test 2 (VPD CRC)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                              if (htx_sp != NULL)
                                 (htx_sp->bad_others)++;
                                  return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_2_P_FRU));
                                  break;

                            case 3:   /* Test 3 - Node Processor Data Memory */
#ifdef debugg
                  detrace(0,"Diagnostic Test 3 (Node Processor Data Memory)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                          if (htx_sp != NULL)
                             (htx_sp->bad_others)++;
                          return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_3_P_FRU));
                                  break;

                        case 4:   /* Test 4 - Skyline Interface */
#ifdef debugg
                  detrace(0,"Diagnostic Test 4 (Skyline Interface)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                           if (htx_sp != NULL)
                              (htx_sp->bad_others)++;
                           return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_4_P_FRU));
                           break;

                        case 5:   /* Test 5 - AMD Interface */
#ifdef debugg
                  detrace(0,"Diagnostic Test 5 (AMD Interface)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                          if (htx_sp != NULL)
                             (htx_sp->bad_others)++;
                                  if ((reg_data >= TST_5_MSK_LW) && (reg_data <= TST_5_MSK_UP))
                                         return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_5_SP_FRU));
                                  else
                                         return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_5_P_FRU));
                                  break;

                        case 6:   /* Test 6 - ALISA to FRB Data Path */
#ifdef debugg
                  detrace(0,"Diagnostic Test 6 (ALISA to FRB Data Path)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                          if (htx_sp != NULL)
                             (htx_sp->bad_others)++;
                          return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_6_P_FRU));
                                  break;

                        case 7:   /* Test 7 - Class B Data Path */
#ifdef debugg
                  detrace(0,"Diagnostic Test 7 (Class B Data Path)");
                  detrace(0," failed. Return value = %x\n", reg_data);
#endif
                          if (htx_sp != NULL)
                             (htx_sp->bad_others)++;
                          return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_7_P_FRU));
                          break;

                        case 8:   /* Test 8 - Class A Data Path */
                          if (reg_data != EXTENDER_ABSENT)
                          {
#ifdef debugg
                        detrace(0,"Diagnostic Test 8 (Class A Data Path)");
                        detrace(0," failed. Return value = %x\n", reg_data);
#endif
                                  if (htx_sp != NULL)
                                     (htx_sp->bad_others)++;
                                          if ((reg_data == TST_8_MSK_1) ||
                                                  (reg_data == TST_8_MSK_0) ||
                                                 ((reg_data >= TST_8_MSK_2L) && (reg_data <= TST_8_MSK_2U)) ||
                                                 ((reg_data >= TST_8_MSK_3L) && (reg_data <= TST_8_MSK_3U)) ||
                                                 ((reg_data >= TST_8_MSK_4L) && (reg_data <= TST_8_MSK_4U)) ||
                                                 ((reg_data >= TST_8_MSK_5L) && (reg_data <= TST_8_MSK_5U)))
                                     return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_8_SP_FRU));
                                          else
                                     return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_8_P_FRU));
                                  break;
                          }
#ifdef debugg
                  detrace(0,"Diagnostic Test 8 (Class A Data Path)");
                  detrace(0," failed. No Extender Card found. \n");
                  detrace(0,"Return value = %x\n", reg_data);
#endif
                          if (htx_sp != NULL)
                                     (htx_sp->bad_others)++;
                                  return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, reg_data));
                          break;

                        case 9:   /* Test 9 - Operational Microcode CRC */
#ifdef debugg
                    detrace(0,"Diagnostic Test 9 (Operational Microcode CRC)");
                        detrace(0," failed. Return value = %x\n", reg_data);
#endif
                          if (reg_data != TEST_NOT_INVOKED)
                                  {
                                  if (htx_sp != NULL)
                                     (htx_sp->bad_others)++;
                                  return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_9_P_FRU));
                                  break;
                              }
                                  else
                                  {
                                  if (htx_sp != NULL)
                                          (htx_sp->bad_others)++;
                                          return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, reg_data));
                                  break;


                                  }

                        case 10:   /* Test 10 - Extender VPD CRC */
/*
 * This test is not valid for Front Royal.  It should be utilized for
 * Scarborough only.
 */
                           if (reg_data != EXTEND_VPD_ABSENT)
                           {
#ifdef debugg
                        detrace(0,"Diagnostic Test A (Extender VPD CRC)");
                        detrace(0," failed. Return value = %x\n", reg_data);
#endif
                                    if (htx_sp != NULL)
                                       (htx_sp->bad_others)++;
                                    return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, TST_A_SP_FRU));
                                    break;
                           }
#ifdef debugg
                  detrace(0,"Diagnostic Test A (Extender VPD CRC)");
                  detrace(0," failed. No Extender Card found. \n");
                  detrace(0,"Return value = %x\n", reg_data);
#endif
                           if (htx_sp != NULL)
                               (htx_sp->bad_others)++;
                   return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, EXTEND_VPD_ABSENT));
                           break;
            }
                }
        }
        return(0);
    }
