static char sccsid[] = { "@(#)82        1.2  src/bos/diag/tu/iop/todtest110.c, tu_iop, bos411, 9431A411a  7/8/94  09:24:12" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: clock_stop
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

        * Stop clock
        * Set IRQROUTE to enable time save
        * read second save register
        * sleep 2 seconds
        * re-read second counter
                - should match second save register
        * start clock
        * sleep 2 seconds
        * read second counter
                - should now differ from second save register

*******************************************************************

        NOTE:
                THIS TEST SHOULD PROBABLY NOT BE RUN IN A
                DAILY TEST CASE SCENARIO.  IT TURNS OFF THE
                REAL TIME CLOCK - SO A LOSS OF TIME IS TO
                BE EXPECTED.

*******************************************************************


                E R R O R   M E S S A G E S
        0x0axx = todtest101
        0x0000 = OK
        0x0a01 = clock failed to stop
        0x0a02 = clock failed to restart
        0x0aFF = no ram memory
*/

#include "address.h"

extern void clear_tod_flags();  /* routine to clear flag array */
extern void save_tod_regs();    /* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */
extern FILE *dbg_fd;

clock_stop(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char rval, xval, sec_save;
u_int i, j, stat;

/* Save off the registers used by this test unit.              */
/*      The registers are referenced by indexes to a global    */
/*      array called tod_reg_flags[].  The actual registers    */
/*      are saved in an array called tod_reg_array[].  But     */
/*      this array should only be accessed via save_tod_regs() */
/*      and restore_tod_regs().                                */
        clear_tod_flags();              /* First clear flag array.     */
        tod_reg_flags[F_MAIN] = 1;      /* Set the reg flags used in   */
        tod_reg_flags[F_IRQROUTE]=1;
        tod_reg_flags[F_REALTIME] = 1;
        save_tod_regs(fdesc, tucb_ptr); /* Save the registers. */
/* End of save reg code.  Remember to restore before return(). */

        stat = 0;
                   /* page 0 reg 1 */
        rval = PG0REG1;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);/* write to control reg */
        get_tod(fdesc,REALTIME,&xval,MV_BYTE,tucb_ptr);/* read control reg */
        rval = xval & 0xf7;                             /* Clear D7 bit */
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);      /* stop clock */

                  /* page 0 reg 0 */
        rval = PG0REG0;                                /* reg select 1 */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);/* write to control reg */
        rval = 0x80;
        set_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);/* time save enable */

                /* set time save enable latch */
        rval = 0x00;
        set_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);

        /*  Clock should not advance since it is in "stop" mode */
        /* sleep(2) must happen before you read SECCOUNT because if you */
        /* don't, you will never give it time for the second to advance */
        sleep(2);
                /* compare times for error */
        get_tod(fdesc,SECCOUNT,&rval,MV_BYTE,tucb_ptr);       /* get seconds */
        get_tod(fdesc,SECSAVE,&sec_save,MV_BYTE,tucb_ptr);    /* get seconds */
        if(rval == 0xFF && sec_save == 0xFF) {
                restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
                return(0x0aFF);
        }
        if(sec_save != rval) stat = 1;

        /* Now start the clock by setting bit #7 in REALTIME reg, sleep
           1 second and then check if the SECCOUNT register advanced.
           If not, signal error condition. */

                   /* page 0 reg 1 */
        rval = PG0REG1;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);
        xval |= 0x08;                                   /* Clock start */
        set_tod(fdesc,REALTIME,&xval,MV_BYTE,tucb_ptr);
        sleep(2);                          /* wait, alow clock to advance... */

                  /* page 0 reg 0 */
        rval = PG0REG0;                                     /* reg select 0 */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);/* write to control reg */
        get_tod(fdesc,SECCOUNT,&rval,MV_BYTE,tucb_ptr);    /* get seconds */
        if(sec_save == rval) stat += 2;

        if(stat) stat += 0x0a00;                                /* module ID */

        restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
        return(stat);
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
