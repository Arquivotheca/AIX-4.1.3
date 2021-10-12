static char sccsid[] = { "@(#)74        1.2  src/bos/diag/tu/iop/todtest102.c, tu_iop, bos411, 9431A411a  7/8/94  09:22:32" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: suspend
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

        Suspend Test.

        This test sets both timers to 0xff.  It then starts both
        timers at 100 HZ (1/100 Sec) in #2 mode for .1 seconds.
        The timers are then supsended by setting the Count Hold/Gate
        Bit in both Timer Control Registers.  A read is taken of
        the timers before and after a sleep.  Since the timers have
        been supsended, no change in the counters should be seen.

                E R R O R   M E S S A G E S
        0x02xx = todtest102
        0x0201 = Timer 0 failed
        0x0202 = Timer 1 failed
        0x0204 = Timer 0 did not decrement
        0x0208 = Timer 1 did not decrement
        0x02FF = no ram memory
*/

#include "address.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern void restore_tod_regs();

extern uchar tod_reg_flags[];
extern FILE *dbg_fd;

suspend(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char q, rval, aval, bval0a, bval1a, bval0b, bval1b;
u_int stat;

	/* This TU is not applicable to Pegasus models: */
	/* their Timers are not enabled by hardware. */
	if (is_pegasus())
		return 0;

/* Save off the registers used by this test unit.           */
/*      The registers are referenced by indexes to a global */
/*      array called tod_reg_flags[].  The actual registers */
/*      are saved in an array called tod_reg_array[].  But  */
/*      this array is only accessed by save_tod_regs() and  */
/*      restore_tod_regs.                                   */
        clear_tod_flags();
        tod_reg_flags[F_MAIN] = 1;
        tod_reg_flags[F_TCR0] = 1;
        tod_reg_flags[F_TCR1] = 1;
        tod_reg_flags[F_TIMER0L] = 1;
        tod_reg_flags[F_TIMER1L] = 1;
        save_tod_regs(fdesc, tucb_ptr);
/* End of save register code.  Remember to restore at end. */

        stat = 0;

/* Set the counters at 0xFF */
        rval = 0x30;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);
        if(rval == 0xFF) {
                restore_tod_regs(fdesc, tucb_ptr);
                return(0x02ff);
        } /* end if */

        rval = 0xff;
        set_tod(fdesc,TIMER0L,&rval,MV_BYTE,tucb_ptr);    /* counter 0 */
        set_tod(fdesc,TIMER1L,&rval,MV_BYTE,tucb_ptr);    /* counter 1 */

/* Set to mode select #2 and clock select at 100HZ, 1/100 second */
/* Set the timer start/stop flag (bit 0) to start clock */
        rval = 0x2d;
        set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr); /* write cntrl reg 0 */
        set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr); /* write cntrl reg 1 */

/* sleep for a 1/10 second to let timer run */
        usleep(100000);

/* Set the Count Hold/Gate Bit, bit #7, in the control registers
   to supsend timers */
        rval = 0xad;
        set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr);
        set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr);

/* Set timer 0 & 1 latches to 'read', by setting bit #6 in
   the control register.  Keep timers supsended by keeping the
   CHG (#7) bit set. */
        rval = 0xed;
        set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr);
        set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr);

/* Read counter 0 - should be decremented */
        get_tod(fdesc,TIMER0L,&bval0a,MV_BYTE,tucb_ptr);

 /* If timer 0 did not decrement, return error */
        if (bval0a == 0xff) {
        restore_tod_regs(fdesc, tucb_ptr);      /* restore TOD registers */
        return(0x204);
        }

/* Read counter 1 - should be decremented */
        get_tod(fdesc,TIMER1L,&bval1a,MV_BYTE,tucb_ptr);

/* If timer 1 did not decrement, return error */
        if (bval1a == 0xff) {
        restore_tod_regs(fdesc, tucb_ptr);      /* restore TOD registers */
        return(0x208);
        }

/* Sleep for .1 second, then re-read timers.  Since the
   timers haved been "suspended",  there should be no
   change in ther timer values. */

        usleep(100000);

        /* Set Control Register to read the timers */
        rval = 0xed;
        set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr);
        set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr);

/* Read counter 0 - should be same as last read */
        get_tod(fdesc,TIMER0L,&bval0b,MV_BYTE,tucb_ptr);

/* Read counter 1 - should be same as last read */
        get_tod(fdesc,TIMER1L,&bval1b,MV_BYTE,tucb_ptr);

        if(bval0a != bval0b) stat += 1;
        if(bval1a != bval1b) stat += 2;

        if(stat) stat += 0x0200;
        restore_tod_regs(fdesc, tucb_ptr);      /* restore TOD registers */
        return(stat);
}
/*----------------------------------------------------------------------------*/
