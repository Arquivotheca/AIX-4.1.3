static char sccsid[] = "@(#)71  1.2.2.4  src/bos/diag/tu/fddi/tu020.c, tu_fddi, bos41J, 9523C_all 6/9/95 14:56:09";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: reset_mode
 *              tu020
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

Function(s) Test Unit 020 - Issue a RESET command.

Module Name :  tu020.c
SCCS ID     :  1.8

Current Date:  5/23/90, 11:17:54
Newest Delta:  1/19/90, 16:30:32

This test unit will issue a RESET to the FDDI adapter card.

POS registers will be accessed via the machine device driver.
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

extern int pos_wr();
extern ushort sif_rd();
extern int sif_addr();
extern unsigned char pos_rd();
extern int pos_save();
extern int pos_restore();
extern unsigned char global_pos[10];
extern int save_pos;
extern int chk_diag_tst();


/*****************************************************************************

reset_mode
This routine will read POS 2, set bit 2 of POS 2, and reread POS 2 to
confirm this occured correctly.

*****************************************************************************/

int reset_mode(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, i, k, diag_tst;
        int             bus_io_addr;
        int             sleep_cnt;
        unsigned int    diag_lst;
        unsigned long   status;
        unsigned char   p0[2];
        static ushort   enab_int_pattern = 0x807b;
        struct htx_data *htx_sp;
        ushort reg_val;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

        /*
         * If the POS registers have not been saved do so now.
         * This information is needed so that the adapter can be restored
         * to working conditions after the reset is complete.
         */
        if (save_pos)
        {

                if (rc = pos_save(global_pos, &status, tucb_ptr))
                {
#ifdef debugg
                        detrace(0,"Error saving POS Registers.");
#endif
                        return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
                }
                global_pos[0] = global_pos[0] & MASK_POS2_CLEAR_RESET;
                save_pos = 0;
        }
        /*
         * read POS register 2.
         */

        p0[0] = pos_rd(2, &status, tucb_ptr);
        if (status)
           {
#ifdef debugg
                detrace(0,"Initial read of POS2 failed. POS2 = %x\n", p0[0]);
                detrace(0,"POS2 read returned status = %x\n", status);
#endif
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_RD_ERR));
           }

        /*
         * Set POS 2 Reset bit to 1.
         */

        p0[0] = 0x02;
        if (pos_wr(2, p0, &status, tucb_ptr))
           {
#ifdef debugg
                detrace(0,"Write of POS2 failed. p0 = %x\n", p0[0]);
                detrace(0,"POS2 write returned status = %x\n", status);
#endif
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_WR_ERR));
           }

        /*
         * Check new value by rereading POS register 2.
         */

        p0[0] = pos_rd( 2, &status, tucb_ptr);
        if (status)
           {
#ifdef debugg
                detrace(0,"2nd read of POS2 failed. POS2 = %x\n", p0[0]);
                detrace(0,"POS2 read returned status = %x\n", status);
#endif
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, POS2_RD_ERR));
           }

        /*
         * Check the Interrupt Level value. If Reset occured, Interrupt
         * Level should read 0xF.
         */
        if (!(p0[0] & MASK_POS2_INTR_RESET))
           {
#ifdef debugg
                detrace(0,"The Interrupt Level was not reset correctly.");
                detrace(0,"Read of POS2. p0 = %x\n", p0[0]);
                detrace(0,"POS2 read returned status = %x\n", status);
#endif
                if (htx_sp != NULL)
                   (htx_sp->bad_others)++;
                return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, INTR_LVL_ERR));
           }

#ifdef debugg
        detrace(0, "Sleeping to allow adapter to run diagnostic tests.\n");
#endif
        /*
         * Sleep for ADAPTER_SLEEP seconds to allow the adapter to run all
         * the internal diagnostic tests.
         */
        sleep(ADAPTER_SLEEP);

        /*
         * Set the card enable bit of POS register 2
         * Restore POS Registers 2 thru 7 from locations
         * global_pos[0] thru global_pos[5].
         */

         global_pos[0] = (global_pos[0] | 0x01) & 0xfd;
         if (rc = pos_restore(global_pos, &status, tucb_ptr))
           {
#ifdef debugg
             detrace(0,"Error restoring POS Registers.");
#endif
             return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, rc));
           }

        rc = sif_addr(&bus_io_addr,tucb_ptr);
        sleep_cnt = 0;
        do {
            reg_val = sif_rd(bus_io_addr, &status, tucb_ptr);
            sleep(1);
            sleep_cnt++;
#ifdef debugg
            detrace(0,"reg_val = %x, sleep_cnt = %d\n", reg_val, sleep_cnt);
#endif
        } while ((sleep_cnt < 40) && (reg_val != 0x80));

/*
 * Check self test results
 * Set up list of self tests to check.  Self test 0xA should be included for
 * Scarborough only. Each test results has 2 bytes.
 */

                diag_lst = 0;
                for (diag_tst = 0; diag_tst < (DIAG_BYTES/2) - 1; diag_tst++)
                        diag_lst = diag_lst | (1 << diag_tst);
#ifdef debugg
                detrace(0,"calling chk_diag_tst %x\n",diag_lst);
#endif
                rc = chk_diag_tst (fdes, diag_lst, tucb_ptr);
                if (rc)
                {
#ifdef debugg
        detrace(0,"check rc = %x,%x\n",rc,rc & 0xFFFF);
#endif
                        if (((rc & 0xFFFF) == EXTENDER_ABSENT) ||
                                ((rc & 0xFFFF) == TEST_NOT_INVOKED) ||
                                ((rc & 0xFFFF) == EXTEND_VPD_ABSENT))
                        {
                                if (htx_sp != NULL)
                                   (htx_sp->bad_others)--; /* These errors are okay for reset */
#ifdef debugg
        detrace(0,"valid error\n");
#endif
                                return(0);
                        }
                        else
                                return(rc);
        }
        return(0);
   }

/*****************************************************************************

tu020

*****************************************************************************/
int tu020 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int     rc;

        rc = reset_mode(fdes, tucb_ptr);
        return(rc);
   }
