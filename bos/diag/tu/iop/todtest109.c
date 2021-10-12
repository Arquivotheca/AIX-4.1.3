static char sccsid[] = { "@(#)81        1.2  src/bos/diag/tu/iop/todtest109.c, tu_iop, bos411, 9431A411a  7/8/94  09:24:03" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: hour12
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

        12/24 MODE

        * Clear 12/24 hour bit, #2, check if cleared
        * Set 12/24 hour bit, #2, check if Set


                E R R O R   M E S S A G E S
        0x09xx = todtest109
        0x0000 = OK
        0x0901 = 12/24 hour bit 2 failed to clear
        0x0902 = 12/24 hour bit 2 failed to set
        0x09FF = no ram memory
*/

#include "address.h"

extern void clear_tod_flags();  /* routine to clear flag array */
extern void save_tod_regs();    /* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */
extern FILE *dbg_fd;

hour12(fdesc,tucb_ptr)
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
        tod_reg_flags[F_MAIN] = 1;      /* Set the reg flags used in TU */
        tod_reg_flags[F_REALTIME] = 1;
        save_tod_regs(fdesc, tucb_ptr); /* Save the registers. */
/* End of save reg code.  Remember to restore before return(). */

        stat = 0;

        rval = PG0REG1;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to control reg */

                /* Get original REALTIME state (xval) */
        get_tod(fdesc,REALTIME,&xval,MV_BYTE,tucb_ptr);  /* read */
        if(xval == 0xFF) {
                restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
                return(0x09FF);
        }

                /* test 12/24 hour bit CLEAR test */
        rval = xval & 0xfb;                    /* strip off 12/24 hour bits */
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);    /* set new bits */
        get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);    /* read reg */
        if((rval & 0x04)!=0x00) stat += 1;

                /* test 12/24 hour bit SET test */
        rval = xval | 0x04;                          /* add 12/24 hour bit */
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);  /* set new bits */
        get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);  /* read control reg */
        if((rval & 0x04)!=0x04) stat += 2;

        if(stat) stat += 0x0900;                       /* module ID */

        restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
        return(stat);
}
/*--------------------------------------------------------------------*/
