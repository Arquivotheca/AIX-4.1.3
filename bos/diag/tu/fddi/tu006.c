static char sccsid[] = "@(#)57  1.1.1.4  src/bos/diag/tu/fddi/tu006.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:45";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: sif_tst
 *              tu006
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

Function(s) Test Unit 006 - Write/Read SIF Registers

Module Name :  tu006.c
SCCS ID     :  1.8

Current Date:  5/23/90, 11:17:54
Newest Delta:  1/19/90, 16:30:32

This test unit writes data to and reads data from the SIF registers. The data
will be compared to insure the write/reads occured correctly.

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

extern int sif_wr();
extern ushort sif_rd();
extern int reg_save();
extern int reg_restore();
extern int sif_addr();


/*****************************************************************************

sif_tst

*****************************************************************************/
int sif_tst(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             i, j, l, reg;
        int             rc;
        int             num_patterns;
        ushort          reg_val;
        ushort          chk_pattern;
        ushort          save_area[NUM_SIF_REG];
        int             status;
        int             bus_io_addr;
        ulong           sif_address;
        static ushort   pattern[] =
           {
                0x0000, 0x0001, 0x5555, 0xAAAA, 0xFFFF
           };
        fddi_hcr_cmd_t  diag;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment counters
         * in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
        detrace(0,"\n----- System Interface Registers (SIF) test begins -----\n");
#endif

        /*
         * Determine bus_io_addr
         */

        rc = sif_addr(&bus_io_addr,tucb_ptr);
        if (rc != 0)
        {
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
        }

        /*
         * Save all the SIF Registers in ushort array save_area.
         */
        rc = reg_save(save_area, bus_io_addr, &status, tucb_ptr);
        if (rc != 0)
          {
#ifdef debugg
             detrace(0,"Error in saving SIF registers.\n");
             detrace(0,"Returned value = %x\n", rc);
             detrace(0,"Returned status = %x\n", status);
#endif
             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, SIF_SAVE_REG));
          }

        /*
         * Select a SIF Register, Write a pattern to it, Read the pattern back
         * and compare the value written with the value read. If an error occurs,
         * return the appropiate error code. Otherwise, continue testing with another
         * pattern and/or SIF Register.
         * Since the SIF Registers are 16 bit quanities, they are accessed with even
         * numbers values (i). (ie. HSR is value #0, HCR is value #2, NS1 is value #4).
         */
        num_patterns = sizeof(pattern)/2;
        for (i = 0; i < (NUM_SIF_REG * 2); i+=2)
         {
            if ((i == HSR_REG) || (i == HCR_REG))
              {
                /*
                 * Initialize SIF Mask Registers.
                 * If testing the HSR Register, write a 0xFFFF to all the mask
                 * registers. This will inhibit interrupts while writing and reading
                 * the HSR.
                 * If testing the HCR Register, write a 0x0000 to all the mask
                 * registers. This will enable interrupts while writing and reading
                 * the other registers.
                 */
                chk_pattern = 0xFFFF;
                if (i == HCR_REG)
                   chk_pattern = 0x0000;
                for (l = HSR_MK_REG; l < ALISA_REG; l+=2)
                  {
                        sif_address = bus_io_addr + l;
                        reg = l >> 1;
                        rc = sif_wr(sif_address, chk_pattern, &status, tucb_ptr);
                        if (rc)
                         {
#ifdef debugg
                           detrace(0,"Error writing to register = %x\n", l);
                           detrace(0,"Returned value, rc = %x\n", rc);
                           detrace(0,"Returned status = %x\n", status);
#endif
                           return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                          SIF_WR_ERR + reg));
                         }
                  }
               }
           for (j = 0; j < num_patterns; j++)
             {
                reg = i >> 1;
                chk_pattern = pattern[j];
                switch (i)
                  {
                    case HSR_REG:
                    case HSR_MK_REG:
                    case NS1_MK_REG:
                    case NS2_MK_REG:
                        sif_address = bus_io_addr + i;
                        rc = sif_wr(sif_address, chk_pattern, &status, tucb_ptr);
                        if (rc)
                          {
#ifdef debugg
                             detrace(0,"Error writing to register = %x\n", i);
                             detrace(0,"Returned value, rc = %x\n", rc);
                             detrace(0,"Returned status = %x\n", status);
#endif
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_WR_ERR + reg));
                          }
                        sif_address = bus_io_addr + i;
                        reg_val = sif_rd(sif_address, &status, tucb_ptr);
                        if (status)
                          {
#ifdef debugg
                             detrace(0,"Error reading register = %x\n", i);
                             detrace(0,"Returned value, reg_val = %x\n", reg_val);
                             detrace(0,"Returned status = %x\n", status);
#endif
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_RD_ERR + reg));
                          }

                        if (chk_pattern != reg_val)
                          {
#ifdef debugg
                             detrace(0,"Error comparing register = %x\n", i);
                             detrace(0,"Returned value = %x, pattern written = %x\n",
                                        reg_val,  pattern[j]);
#endif
                             if (htx_sp != NULL)
                                (htx_sp->bad_others)++;
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_CMP_ERR + reg));
                          }
                        break;

                    case HCR_REG:    /* Currently, this test does NOT WORK. */
                        break;
                        diag.hcr_val = pattern[j];
                        diag.l_cpb = 0;

                        status = 0;
                        rc = ioctl(fdes, FDDI_HCR_CMD, &diag);
                        status = diag.status;

                        if (rc || diag.status || diag.cpb[FDDI_CPB_SIZE-1])
                          {
#ifdef debugg
        detrace(0,"HCR Command written incorrectly. rc = %x\n", rc);
        detrace(0,"Returned status = %x\n", status);
        detrace(0,"Returned status in cpb = %x\n",diag.cpb[FDDI_CPB_SIZE-1]);
#endif
                            if (htx_sp != NULL)
                               (htx_sp->bad_others)++;
                            return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                           HCR_CMD_WR_ERR));
                          }
                        else
                          if (htx_sp != NULL)
                             (htx_sp->good_others)++;
                        sif_address = bus_io_addr + i;
                        reg_val = sif_rd(sif_address, &status, tucb_ptr);
                        if (status)
                          {
#ifdef debugg
                             detrace(0,"Error reading register = %x\n", i);
                             detrace(0,"Returned value, reg_val = %x\n", reg_val);
                             detrace(0,"Returned status = %x\n", status);
#endif
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_RD_ERR + reg));
                          }

                        if (pattern[j] != reg_val)
                          {
#ifdef debugg
                             detrace(0,"Error comparing register = %x\n", i);
                             detrace(0,"Returned value = %x, pattern written = %x\n",
                                        reg_val,  pattern[j]);
#endif
                             if (htx_sp != NULL)
                                (htx_sp->bad_others)++;
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_CMP_ERR + reg));
                          }
                        break;

                    case NS1_REG:  /* Do NOT test this Register */
                    case NS2_REG:  /* Do NOT test this Register */
                        break;

                    case ALISA_REG:
                        /*
                         * Currently, the only bit accessable in the ALISA Control
                         * Register is the LDM bit. For testing, all the other bits
                         * must be masked out.
                         */
                        chk_pattern = pattern[j] & MASK_ALISA_LDM_BIT;
                        sif_address = bus_io_addr + i;
                        rc = sif_wr(sif_address, chk_pattern, &status, tucb_ptr);
                        if (rc)
                          {
#ifdef debugg
                             detrace(0,"Error writing to register = %x\n", i);
                             detrace(0,"Returned value, rc = %x\n", rc);
                             detrace(0,"Returned status = %x\n", status);
#endif
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_WR_ERR + reg));
                          }
                        sif_address = bus_io_addr + i;
                        reg_val = sif_rd(sif_address, &status, tucb_ptr);
                        if (status)
                          {
#ifdef debugg
                             detrace(0,"Error reading register = %x\n", i);
                             detrace(0,"Returned value, reg_val = %x\n", reg_val);
                             detrace(0,"Returned status = %x\n", status);
#endif
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_RD_ERR + reg));
                          }
                        reg_val = reg_val & MASK_ALISA_LDM_BIT;
                        if (chk_pattern != reg_val)
                          {
#ifdef debugg
                             detrace(0,"Error comparing register = %x\n", i);
                             detrace(0,"Returned value = %x, pattern written = %x\n",
                                        reg_val,  pattern[j]);
#endif
                             if (htx_sp != NULL)
                                (htx_sp->bad_others)++;
                             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
                                            SIF_CMP_ERR + reg));
                          }
                        break;

                  }
             }     /* End of Pattern loop */
#ifdef debugg
           if ((i == 4) || (i == 6))
             {
               if (i == 4)
                  detrace(0,"No test for SIF Register 4 (NS1 register).\n");
               else
                  detrace(0,"No test for SIF Register 6 (NS2 register).\n");
             }
           else
              detrace(0,"SIF Register '%x' compared SUCCESSFULLY with all patterns.\n", i);
#endif
         }    /* End of SIF Register loop. */

        /*
         * Restore all the SIF Registers.
         * This will not restore the Host Status Register (HSR), the Host Command
         * Register, the Node Processor Status 1 Register (NS1) and the Node
         * Processor Status 2 Register (NS2).
         */
        rc = reg_restore(save_area, bus_io_addr, &status, tucb_ptr);
        if (rc != 0)
          {
#ifdef debugg
             detrace(0,"Error in restoring SIF registers.\n");
#endif
             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, SIF_RESTORE_REG));
          }

#ifdef debugg
        detrace(0,"----- End of System Interface Registers (SIF) test -----\n");
#endif
        return(0);
   }


/*****************************************************************************

tu006

*****************************************************************************/

int tu006 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;

        /*
         * Call SIF Register test.
         */

        rc = sif_tst(fdes, tucb_ptr);

        return(rc);
   }
