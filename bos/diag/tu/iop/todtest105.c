static char sccsid[] = { "@(#)77	1.3  src/bos/diag/tu/iop/todtest105.c, tu_iop, bos411, 9431A411a  7/8/94  09:23:06" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: periodic
 *
 *   ORIGINS: 27, 83
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

/*

        Periodic Interrupt

        * Clear Main Register
        * Set Interrupt Control Register to send a periodic
          interrupt on 10 msec.
        * sleep for 20 msec.
        * Read Main Register and check if a periodic interrupt
          occured.

                E R R O R   M E S S A G E S
        0x05xx = todtest105
        0x0000 = OK
        0x0501 = Periodic IRQ failed
        0x05FF = no ram memory
*/

#include "address.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern void restore_tod_regs();

extern uchar tod_reg_flags[];
extern FILE *dbg_fd;

periodic(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char rval;
u_int stat;

/* Save off the registers used by this test unit.              */
/*      The registers are referenced by indexes to a global    */
/*      array called tod_reg_flags[].  The actual registers    */
/*      are saved in an array called tod_reg_array[].  But     */
/*      this array should only be accessed via save_tod_regs() */
/*      and restore_tod_regs().                                */
        clear_tod_flags();
        tod_reg_flags[F_MAIN] = 1;
        tod_reg_flags[F_IRQ0] = 1;
        save_tod_regs(fdesc, tucb_ptr);
/* End of save register code.  Remember to restore at end. */

/* Set MAIN to Page 0 Reg 1.  In addition clear out the Periodic Interrupt */
/* by writing a 1 to bit D2.  This will be checked later to insure it has  */
/* been set.                                                               */

        rval = 0x44;                                    /* reg select 0 */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);    /* main status reg */

        /* Set the interrupt control register 0 to issue a periodic
           interrupt on 10 msec, bit #4 */
        rval = 0x10;                  /* 10 ms delay for periodic IRQ */
        set_tod(fdesc,IRQ0,&rval,MV_BYTE,tucb_ptr);

        usleep(20000);

        /* read if Periodic IRQ is active */
        get_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);
        if(rval == 0xFF) {                          /* no NVRAM test */
                restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
                return(0x05FF);
        }
        rval &= 0x04;       /* strip off all except periodic intr bit */
        if (!rval) {
                stat = 0x501;
        }
        else {
                stat = 0;
        } /* endif */

        restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
        return(stat);
}
/*-----------------------------------------------------------------*/
