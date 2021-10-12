static char sccsid[] = "@(#)50  1.1.1.1  src/bos/diag/tu/fddi/sr_pos.c, tu_fddi, bos41J, 9523C_all 6/9/95 14:07:17";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: pos_save
 *              pos_restore
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

Function(s) Save/Restore POS Registers

Module Name :  sr_pos.c
SCCS ID     :  1.9

Current Date:  5/23/90, 11:17:51
Newest Delta:  1/19/90, 16:28:49

*****************************************************************************/
#include <stdio.h>
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif

extern ulong pos_addr();
/*****************************************************************************

pos_save

Function saves off POS registers 2 through 7 in the passed-in unsigned char
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to read of specific POS register

*****************************************************************************/

int pos_save (save_area, status, tucb_ptr)
   unsigned char save_area[];
   unsigned char *status;
   TUTYPE *tucb_ptr;
   {
        int i;
        unsigned char p0[2];

        extern unsigned char pos_rd();

        for (i = 2; i < 8; i++)
           {
                p0[0] = pos_rd(i, status, tucb_ptr);
                if (*status)
                   {
#ifdef debugg
                        detrace(0,"pos_rd of %i returned status of %x\n",
                                status);
#endif
                        return(POS0_RD_ERR + i);
                   }
#ifdef debugg
                detrace(0,"pos_rd:  POS %d = 0x%02x\n", i, p0[0]);
#endif
                save_area[i - 2] = p0[0];
                usleep(500000);
           }
        return(0);
   }

/*****************************************************************************

pos_restore

Function restores POS registers 2 through 7 from the passed in unsigned char
array, "save_area".

IF successful
THEN RETURNs 0
ELSE RETURNs error code relating to write of specific POS register

*****************************************************************************/

int pos_restore (save_area, status, tucb_ptr)
   unsigned char save_area[];
   unsigned char *status;
   TUTYPE *tucb_ptr;
   {
        int i, rc;
        unsigned char p0[2];

        extern int pos_wr();

        /*
         * first, we restore POS registers 6 and 7.
         */
        if (pos_wr(7, &save_area[5], status, tucb_ptr))
           {
#ifdef debugg
                detrace(0,"pos_wr of %x to pos reg %d failed.\n",
                                save_area[5], 7);
#endif
                return(POS7_WR_ERR);
           }
        if (pos_wr(6, &save_area[4], status, tucb_ptr))
           {
#ifdef debugg
                detrace(0,"pos_wr of %x to pos reg %d failed.\n",
                                save_area[4], 6);
#endif
                return(POS6_WR_ERR);
           }

        /*
         * Now, if POS 6 and 7 are zero, then no subaddressing
         * is occurring, so we can SAFELY restore the value
         * of POS register 3 (set beginning index (i) to 3).
         *
         * Otherwise, POS register 6 and 7 are pointing to a
         * subaddress memory location whose value is accessed
         * through POS 3 so we do NOT want to restore the value
         * of POS 3 because it will WRITE that value to the
         * subaddressed memory area!!!  So, start beginning
         * index (i) to 4).
         */
        if ((save_area[5] == 0x00) &&
            (save_area[4] == 0x00))
                i = 3;
        else
                i = 4;

        for (; i < 6; i++)
           {
                if (rc = pos_wr(i, &save_area[i - 2], status, tucb_ptr))
                   {
#ifdef debugg
                        detrace(0,"pos_wr of %x to pos reg %d failed.\n",
                                        save_area[i - 2], i);
#endif
                        return(POS0_WR_ERR + i);
                   }
           }
        /*
         * restore 2 last since it has the card enable bit
         * Note: Before restoring Pos 2, Mask out the Reset Bit.
         * (The Reset Bit always reads set when read).
         */
        p0[0]= save_area[0] & MASK_POS2_CLEAR_RESET;
        if (pos_wr(2, p0, status, tucb_ptr))
           {
#ifdef debugg
                detrace(0,"pos_wr of %x to pos reg 2 failed.\n",
                                        save_area[0]);
#endif
                return(POS2_WR_ERR);
           }

        return(0);
   }
