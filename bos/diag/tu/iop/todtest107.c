static char sccsid[] = { "@(#)79        1.2  src/bos/diag/tu/iop/todtest107.c, tu_iop, bos411, 9431A411a  7/8/94  09:23:32" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: save_enable
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

        TIME SAVE ENABLE

        * Stop Real Time Clock by masking out Clock Stop Bit,
            D3, in the Real Time Register.
        * Enable clock read by latching Time Save Enable Bit,
            D7, in IRQROUTE.
        * Read Seconds, Minutes, Hours, Days, Hours, Months
            from latched memory and compare to current value.
        * Set error status if values differ.
        * Start Real time Clock, by reseting D3 in the
           Real Time Register.
        * Sleep 2 second.
        * Read current second value.
        * Latched second value should differ from current
           second value, return error if same.

*******************************************************************

        NOTE:
                THIS TEST SHOULD PROBABLY NOT BE RUN IN A
                DAILY TEST CASE SCENARIO.  IT TURNS OFF THE
                REAL TIME CLOCK - SO A LOSS OF TIME IS TO
                BE EXPECTED.

*******************************************************************

                E R R O R   M E S S A G E S
        0x07xx = todtest107
        0x0000 = OK
        0x0701 = Seconds Time does not compare
        0x0702 = Minutes Time does not compare
        0x0704 = Hours Time does not compare
        0x0708 = Days Time does not compare
        0x0710 = Months time does not compare
        0x0720 = Timer restart failed
        0x07FF = no ram memory
*/

#include "address.h"

extern void clear_tod_flags();  /* routine to clear flag array */
extern void save_tod_regs();    /* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */
extern FILE *dbg_fd;

save_enable(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char rval, xval, cval;
u_int i, j, stat;

/* Save off the registers used by this test unit.              */
/*      The registers are referenced by indexes to a global    */
/*      array called tod_reg_flags[].  The actual registers    */
/*      are saved in an array called tod_reg_array[].  But     */
/*      this array should only be accessed via save_tod_regs() */
/*      and restore_tod_regs().                                */
        clear_tod_flags();              /* First clear flag array.     */
        tod_reg_flags[F_MAIN] = 1;      /* Set the reg flags used in   */
        tod_reg_flags[F_IRQROUTE] = 1;  /* this TU.                    */
        tod_reg_flags[F_REALTIME] = 1;
        save_tod_regs(fdesc, tucb_ptr); /* save the registers. */
/* End of save register code.  Remember to restore at end. */

        stat = 0;
                     /* page 0 reg 1 */
        rval = PG0REG1;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* Main Status Register */
        get_tod(fdesc,REALTIME,&xval,MV_BYTE,tucb_ptr); /* read Realtime */

        rval = 0xF7 & xval;     /* mask out Stop Clock bit, D3 */
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);    /* stop clock */

                    /* page 0 reg 0 */
        rval = PG0REG0;        /* reg select 1 */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);
        /* Enable the Time Save enable bit, D7 in IRQROUTE */
        rval = 0x80;
        set_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);

        /* Now freeze the Time, by clearing D7 in IRQROUTE */
        rval = 0x00;
        set_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);

                    /* compare times for error */
        get_tod(fdesc,SECCOUNT,&rval,MV_BYTE,tucb_ptr); /* get seconds */
        get_tod(fdesc,SECSAVE,&cval,MV_BYTE,tucb_ptr);  /* get seconds */

        if(rval == 0xFF || cval == 0xFF) {
                restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
                return(0x07FF);
        }

        if(cval != rval) stat = 1;
        get_tod(fdesc,MINCOUNT,&rval,MV_BYTE,tucb_ptr); /* get minutes */
        get_tod(fdesc,MINSAVE,&cval,MV_BYTE,tucb_ptr);  /* get minutes */
        if(cval != rval) stat += 2;

        get_tod(fdesc,HURCOUNT,&rval,MV_BYTE,tucb_ptr); /* get hours */
        get_tod(fdesc,HURSAVE,&cval,MV_BYTE,tucb_ptr);  /* get hours */
        if(cval != rval) stat += 4;

        get_tod(fdesc,DAYCOUNT,&rval,MV_BYTE,tucb_ptr); /* get days */
        get_tod(fdesc,DAYSAVE,&cval,MV_BYTE,tucb_ptr);  /* get days */
        if(cval != rval) stat += 8;

        get_tod(fdesc,MONCOUNT,&rval,MV_BYTE,tucb_ptr);  /* get months */
        get_tod(fdesc,MONSAVE,&cval,MV_BYTE,tucb_ptr);   /* get months */
        if(cval != rval) stat += 16;

                     /* page 0 reg 1 */
        rval = PG0REG1;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);

        xval |= 0x08;     /* Clock start */
        set_tod(fdesc,REALTIME,&xval,MV_BYTE,tucb_ptr);
        sleep(2);        /* wait, alow clock to advance... */

                    /* page 0 reg 0 */
        rval = PG0REG0;          /* reg select 0 */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);
        get_tod(fdesc,SECCOUNT,&rval,MV_BYTE,tucb_ptr);  /* get seconds */
        get_tod(fdesc,SECSAVE,&cval,MV_BYTE,tucb_ptr);   /* get seconds */
        if(cval == rval) stat += 0x0020;

        if(stat) stat += 0x0700;

        restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
        return(stat);
}
/*------------------------------------------------------------*/
