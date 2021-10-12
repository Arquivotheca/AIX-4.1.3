static char sccsid[] = { "@(#)80        1.2  src/bos/diag/tu/iop/todtest108.c, tu_iop, bos411, 9431A411a  7/8/94  09:23:43" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: Leap Year
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

        LEAP YEAR

                Set, confirm Leap Year "Last Year" condition, 0x01
                Set, confirm Leap Year "2 years ago" condition, 0x02
                Set, confirm Leap Year "3 years ago" condition, 0x02
                Set for leap year "current", 0x00
                Set date for Feb 28, 1 second before midnight
                Sleep 2 seconds
                Confirm day number is now 29.


*******************************************************************

        NOTE:
                THIS TEST SHOULD PROBABLY NOT BE RUN IN A
                DAILY TEST CASE SCENARIO.  IT TURNS OFF THE
                REAL TIME CLOCK - SO A LOSS OF TIME IS TO
                BE EXPECTED.

*******************************************************************

                E R R O R   M E S S A G E S
        0x08xx = todtest108
        0x0000    no problem found
        0x0801    leap year bit 0 unable to set
        0x0802    leap year bit 1 unable to set
        0x0804    leap year bit 0 unable to clear
        0x0808    leap year bit 1 unable to clear
        0x08F0    date did not advance to Feb. 29
        0x08FF    no ram memory
*/
#include "address.h"

extern void clear_tod_flags();  /* routine to clear flag array */
extern void save_tod_regs();    /* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */
extern FILE *dbg_fd;


leap_year(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char rval, xval, cval;
u_int i, j, stat,second,minute,hour,day,month,year,s,m,h,d,mo,y;

/* Save off the registers used by this test unit.              */
/*      The registers are referenced by indexes to a global    */
/*      array called tod_reg_flags[].  The actual registers    */
/*      are saved in an array called tod_reg_array[].  But     */
/*      this array should only be accessed via save_tod_regs() */
/*      and restore_tod_regs().                                */
        clear_tod_flags();              /* First clear flag array.     */
        tod_reg_flags[F_MAIN] = 1;      /* Set the reg flags used.     */
        tod_reg_flags[F_REALTIME] = 1;
        tod_reg_flags[F_IRQROUTE] = 1;
        tod_reg_flags[F_SECCOUNT] = 1;
        tod_reg_flags[F_MINCOUNT] = 1;
        tod_reg_flags[F_HURCOUNT] = 1;
        tod_reg_flags[F_DAYCOUNT] = 1;
        tod_reg_flags[F_MONCOUNT] = 1;
        tod_reg_flags[F_YERCOUNT] = 1;
        tod_reg_flags[F_DYWKCOUNT] = 1; /* Day of week counter */
        tod_reg_flags[F_UJULCOUNT] = 1;
        save_tod_regs(fdesc, tucb_ptr);  /* Save the TOD registers. */
/* End of save reg code.  Remember to restore before return(). */

        stat = 0;

        rval = PG0REG1;          /* page 0 reg 1 */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* main status reg */

/* Get the Real Time Mode Reg and do the no RAM test */
        get_tod(fdesc,REALTIME,&xval,MV_BYTE,tucb_ptr); /* read  REALTIME */
        if(xval == 0xFF) {
                restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
                return(0x08FF);
        }

          /* test leap bits 0 & 1 */
        rval = xval;
        rval &= 0xfc;                /* strip off leap year bits */
        rval |= 0x03;                /* add high leap bits */
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);/* set new leap bits */
        get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr); /* read  control reg */
        if((rval & 0x03)!=0x03)  {

                /* test leap bits 0 */
                rval = xval;
                rval &= 0xfc;               /* strip off leap year bits */
                rval |= 0x01;               /* add 1 year leap bits */
                set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                if((rval & 0x01)!=0x01) stat += 1;

                /* test leap bits 1 */
                rval = xval;
                rval &= 0xfc;               /* strip off leap year bits */
                rval |= 0x02;               /* add 1 year leap bits */
                set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                if((rval & 0x02)!=0x02) stat += 2;

        } /* endif */

          /* test if leap bits can be cleared to 0 */
        rval = xval;
        rval &= 0xfc;              /* strip off leap year bits */
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);/* set new leap bits */
        get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr); /* read  control reg */
        if((rval & 0x03)!=0x00)  {

                /* test leap bit 0 */
                rval = xval;
                rval &=0xfc;
                rval |= 0x02;
                set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                if((rval & 0x01) != 0x00) stat += 4;

                /* test leap bit 1 */
                rval = xval;
                rval &= 0xfc;
                rval |= 0x01;
                set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                get_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);
                if((rval & 0x02) != 0x00) stat += 8;
        } /* endif */

        /* restore leap year reg to orginal state */
        set_tod(fdesc,REALTIME,&xval,MV_BYTE,tucb_ptr);

        /* if unable to clear or set leap year bits return with
            appropriate error */
        if(stat)
        {
                stat += 0x0800;
                restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
                return(stat);
        }

        rval = PG0REG1;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to  control reg */

/* Clear the Clock stop bit, #2, to reset the date and time.  Also, set the */
/* leap year to current year, bit0 = 0, bit1 = 1.                           */
        rval = 0xf4 & xval;
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);

        rval = PG0REG0;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to  control reg */

         /* set the time & date to 23:59:59 28 Feb 1992 */
        rval = 0x22;    /* set year to 1992 (1992 - 1970 in BCD */
        set_tod(fdesc,YERCOUNT,&rval,MV_BYTE,tucb_ptr);
        rval = 0x02;    /* set month to February in BCD */
        set_tod(fdesc,MONCOUNT,&rval,MV_BYTE,tucb_ptr);
        rval = 0x28;    /* set date to February 28 in BCD */
        set_tod(fdesc,DAYCOUNT,&rval,MV_BYTE,tucb_ptr);
        rval = 0x23;    /* set hour  in BCD */
        set_tod(fdesc,HURCOUNT,&rval,MV_BYTE,tucb_ptr);
        rval = 0x59;    /* set minutes  in BCD */
        set_tod(fdesc,MINCOUNT,&rval,MV_BYTE,tucb_ptr);
        rval = 0x59;    /* set seconds  in BCD */
        set_tod(fdesc,SECCOUNT,&rval,MV_BYTE,tucb_ptr);

        /* Set MAIN to Page 0 Reg 1 */
        rval = PG0REG1;
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to  control reg */

        /* Clear the leap year bit and make sure Start/Stop bit is set to 1 */
        rval = 0xf4 & xval;
        rval |= 0x08;    /* restart the clock to reset the time & date */
        set_tod(fdesc,REALTIME,&rval,MV_BYTE,tucb_ptr);

         /* wait for new day */
        sleep(2);
        rval = PG0REG0; /* write to  control reg */
        set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);

        /* Get the value of IRQROUTE and set/mask D7 (Time Save Enable bit). */
        get_tod(fdesc,IRQROUTE,&xval,MV_BYTE,tucb_ptr);
        rval = xval | 0x80;
        set_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);

        rval = xval & 0x7f;
        set_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);
        get_tod(fdesc,DAYSAVE,&rval,MV_BYTE,tucb_ptr);
        if(rval != 0x29) stat += 0xF0;        /* leap year gear not working */

        if(stat) stat += 0x0800;                           /* module ID */

        restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */

        return(stat);
}
/*---------------------------------------------------------------*/
