static char sccsid[] = { "@(#)84	1.2  src/bos/diag/tu/iop/todtest112.c, tu_iop, bos411, 9431A411a  7/8/94  09:24:35" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: tod_irq_stat
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

	CLEAR TOD INTERRUPTS

	* In the Timer Control Registers, set bits to start counters 
            and set clock select bits to .001 seconds.
	* Set Interrupt Control Register 0 to enable Timer 0, Timer 1 
            and 1 millisecond periodic interrupts.
	* Set Timer value to 0x09.
	* Sleep for .01 seconds.
        * Check if Timer 0, Timer 1, Period and Interrupt bits
            are set in Main Status Register.
        * Clear Periodic Interrupt Enables D5-D7 in Interrupt Control
	    Register 0.
        * Clear all Interrupt bits in Main Status register
            by writing a 1 to D1-D5.
	* Check to see that the interrupt bits in the Main Status Register 
            have been cleared.

	NOTE: for Pegasus models, Timers Interrupts are not checked
       
                E R R O R   M E S S A G E S
        0x0cxx = todtest112
        0x0000 = OK
        0x0c01 = Interrupt Status not cleared
        0x0c02 = Power fail interrupt
        0x0c04 = Periodic interrupt
        0x0c08 = Alarm interrupt
        0x0c10 = Timer 0 interrupt
        0x0c20 = Timer 1 interrupt
	0x0c40 = Interrupts not set correctly
        0x0cFF = No ram memory
*/

#include "address.h"

extern void clear_tod_flags();	/* routine to clear flag array */
extern void save_tod_regs();	/* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */
extern FILE *dbg_fd;

tod_irq_stat(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char rval, xval, cval;
u_int i, j, stat;

/* Save off the registers used by this test unit.              */
/*	The registers are referenced by indexes to a global    */
/*      array called tod_reg_flags[].  The actual registers    */
/*      are saved in an array called tod_reg_array[].  But     */
/*      this array should only be accessed via save_tod_regs() */
/*      and restore_tod_regs().                                */
	clear_tod_flags();		/* First clear flag array.     */
	tod_reg_flags[F_MAIN] = 1;	/* Set the reg flags used in   */
	tod_reg_flags[F_TCR0] = 1;	/* this TU.		       */
	tod_reg_flags[F_TCR1] = 1;
	tod_reg_flags[F_IRQ0] = 1;
	tod_reg_flags[F_IRQ1] = 1;
	tod_reg_flags[F_TIMER0L] = 1;
	tod_reg_flags[F_TIMER1L] = 1;
	tod_reg_flags[F_TIMER0M] = 1;
	tod_reg_flags[F_TIMER1M] = 1;
	save_tod_regs(fdesc, tucb_ptr);	/* Save the registers. */
/* End of save reg code.  Remember to restore before return(). */
	      
	stat = 0;

	rval = PG0REG0;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */

        /* TCR0/1
                D7 = 0          Count/Hold Gate Bit off
                D6 = 0          Latch Read bit off
                D5 = 1
                D4 = 0
                D3 = 0          D3-5, Clock Speed, 1ms
                D2 = 0
                D1 = 0          D1-2, Timer Mode 0
                D0 = 1          Start/Stop bit on
        */

	rval = 0x21;
	set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */
	set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */

	rval = PG0REG1;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */

	/* Enable Timer 0, Timer 1 and 1 millisecond periodic interrupts */
	rval = 0xe0;
	set_tod(fdesc,IRQ0,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */

       /* Set timer */
        rval = 0x00;
        set_tod(fdesc,TIMER0M,&rval,MV_BYTE,tucb_ptr);  /* reset timer 0 MSB */
        set_tod(fdesc,TIMER1M,&rval,MV_BYTE,tucb_ptr);  /* reset timer 1 MSB */

        rval = 0x09;
        set_tod(fdesc,TIMER0L,&rval,MV_BYTE,tucb_ptr);  /* reset timer 0 LSB */
        set_tod(fdesc,TIMER1L,&rval,MV_BYTE,tucb_ptr);  /* reset timer 1 LSB */

	/* Sleep for .01 seconds */
	usleep(10000);

	get_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);         

	/* The following interrupt bits should be set:
		Timer 0, Timer 1, Periodic and Status */
	cval = 0x35;
	/* For Pegasus models, Timers are disabled by hardware */
	if (is_pegasus())
		cval = 0x05;
	if ((rval & cval) != cval) {
		restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
		return(0xc40);
	}

	/* Since the periodic bit has been set, clear all interrupt 
	   periodic enables so the timer won't set the periodic 
	   interrupt bit after we cleared the interrupt bits in 
	   the Main Status register */
	rval = PG0REG1;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */

		/* Clear all periodic intterupt enables */
	rval = 0x00;
	set_tod(fdesc,IRQ0,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */
	set_tod(fdesc,IRQ1,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */

	/* Clear all interrupts in the Main Status Register by writing
	   a 1 to D2-D5 to all the IRQ bits.  Note: the PF bit, D1, can
           not be cleared via writing a 1. */
	rval = 0x3c;                       
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* write to cntrl reg */

		/* see if interrupts are clear */
	get_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);         
	if(rval == 0xff) {
		restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
		return(0x0cff);                       /* no ram memory */
	}

	/* Check main status interrupt bits, D0-D5.  All should be cleared */
	if(rval & 0x01) stat = 0x01;       /* interrupt status not cleared */
	if(rval & 0x02) stat |= 0x02;      /* interrupt status not cleared */
	if(rval & 0x04) stat |= 0x04;      /* interrupt status not cleared */
	if(rval & 0x08) stat |= 0x08;      /* interrupt status not cleared */
	if(rval & 0x10) stat |= 0x10;      /* interrupt status not cleared */
	if(rval & 0x20) stat |= 0x20;      /* interrupt status not cleared */

	if(stat) stat += 0x0c00;                        /* module ID */

	restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
	return(stat);
}
/*--------------------------------------------------------------------*/
