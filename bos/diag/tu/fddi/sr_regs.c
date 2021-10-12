static char sccsid[] = "@(#)90  1.5  src/bos/diag/tu/fddi/sr_regs.c, tu_fddi, bos411, 9428A410j 11/4/93 11:06:27";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: reg_save
 *              reg_restore
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

Function(s) Save/Restore ALL SIF registers

Module Name :  sr_regs.c
SCCS ID     :  1.8

Current Date:  5/23/90, 11:17:51
Newest Delta:  1/19/90, 16:29:01

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

reg_save

Function saves off the System Interface Registers (SIF) in the passed-in
unsigned short integer array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read of specific register

*****************************************************************************/

int reg_save (save_area, bus_io_addr, status, tucb_ptr)
   ushort       save_area[];
   int          bus_io_addr;
   ulong        *status;
   TUTYPE       *tucb_ptr;
   {
        int             rc, reg;
        ushort          i,reg_val;
        extern ushort   sif_rd();
        ulong           sif_addr;

        /*
         * Read the appropiate SIF Register and save in locations
         * save_area[0] thru save_area[7].
         */
        for (i = 0; i < (NUM_SIF_REG * 2); i+=2)
          {
            reg = i >> 1;
            sif_addr = bus_io_addr + i;
            reg_val = sif_rd(sif_addr, status, tucb_ptr);
            if (*status)
              {
                switch(i)
                   {
                        case HSR_REG:
#ifdef debugg
                           detrace(0,"reg_save:  HSR is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RHSR_RD_ERR);

                        case HCR_REG:
#ifdef debugg
                           detrace(0,"reg_save:  HCR is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RHCR_RD_ERR);

                        case NS1_REG:
#ifdef debugg
                           detrace(0,"reg_save:  NS1 is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RNS1_RD_ERR);

                        case NS2_REG:
#ifdef debugg
                           detrace(0,"reg_save:  NS2 is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RNS2_RD_ERR);

                        case HSR_MK_REG:
#ifdef debugg
                           detrace(0,"reg_save:  HSR Mask is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RHSR_MK_RD_ERR);

                        case NS1_MK_REG:
#ifdef debugg
                           detrace(0,"reg_save:  NS1 Mask is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RNS1_MK_RD_ERR);

                        case NS2_MK_REG:
#ifdef debugg
                           detrace(0,"reg_save:  NS2 Mask is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RNS2_MK_RD_ERR);

                        case ALISA_REG:
#ifdef debugg
                           detrace(0,"reg_save:  ALISA is 0x%02x\n", reg_val);
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RALISA_RD_ERR);

                   }
           }

            save_area[reg] = reg_val;
         }    /* End FOR loop */
        return(0);
   }

/*****************************************************************************

reg_restore

Function restores the System Interface Registers (SIF) from the
passed-in unsigned short integer array, "save_area". This function will not
restore the Host Status Register (HSR), the Host Command Register (HCR), the
Node Processor Status 1 Register (NS1) and the Node Processor Status 2
Register (NS2).

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to write of specific register

*****************************************************************************/

int reg_restore (save_area, bus_io_addr, status, tucb_ptr)
   ushort save_area[];
   int    bus_io_addr;
   ulong  *status;
   TUTYPE *tucb_ptr;
   {
        ushort i;
        int     reg, rc;
        ulong  sif_addr;
        extern int sif_wr();

        /*
         * SIF Registers are restored from locations
         * save_area[0] thru save_area[7].
         */
        for (i = 0; i < (NUM_SIF_REG * 2); i+=2)
          {
            reg = i >> 1;
            sif_addr = bus_io_addr + i;
            switch(i)
              {
                case HSR_REG:     /* Do not restore these registers */
                case HCR_REG:     /* "   "     "      "       "     */
                case NS1_REG:     /* "   "     "      "       "     */
                case NS2_REG:     /* "   "     "      "       "     */
                   break;

                case HSR_MK_REG:
                   if (rc = sif_wr(sif_addr, save_area[reg], status, tucb_ptr))
                     {
#ifdef debugg
                        detrace(0,"reg_restore:  HSR Mask is 0x%02x\n", rc);
                        detrace(0,"Returned status = %x\n", *status);
#endif
                        return(RHSR_MK_WR_ERR);
                     }
                   break;

                case NS1_MK_REG:
                   if (rc = sif_wr(sif_addr,  save_area[reg], status, tucb_ptr))
                     {
#ifdef debugg
                        detrace(0,"reg_restore:  NS1 Mask is 0x%02x\n", rc);
                        detrace(0,"Returned status = %x\n", *status);
#endif
                        return(RNS1_MK_WR_ERR);
                     }
                   break;

                case NS2_MK_REG:
                   if (rc = sif_wr(sif_addr,  save_area[reg], status, tucb_ptr))
                     {
#ifdef debugg
                        detrace(0,"reg_restore:  NS2 Mask is 0x%02x\n", rc);
                        detrace(0,"Returned status = %x\n", *status);
#endif
                        return(RNS2_MK_WR_ERR);
                     }
                   break;

                case ALISA_REG:
                   if (rc = sif_wr(sif_addr,  save_area[reg], status, tucb_ptr))
                     {
#ifdef debugg
                        detrace(0,"reg_restore:  ALISA is 0x%02x\n", rc);
                        detrace(0,"Returned status = %x\n", *status);
#endif
                        return(RALISA_WR_ERR);
                     }
                   break;

                default:
#ifdef debugg
                           detrace(0,"reg_restore:  default condition.\n");
                           detrace(0,"Returned status = %x\n", *status);
#endif
                           return(RDEF_WR_ERR);
                }
          }    /* End FOR loop */
        return(0);
   }
