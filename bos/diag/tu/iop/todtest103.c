static char sccsid[] = { "@(#)75	1.2  src/bos/diag/tu/iop/todtest103.c, tu_iop, bos411, 9431A411a  7/8/94  09:22:42" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: interrrupt
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

	Enable Interrupts for 0 & 1 Timer by sending a 0xC0 to
        the Interrupt Control Register 0 [IRQ0].  Sleep for 2
	seconds and verify that an interrupt registered in the
	Main Status register: Timer 0 INTR (bit 4), Timer 1 INTR
 	(bit 5).
    
                E R R O R    M E S S A G E S
        0x03xx = todtest103
        0x0000 = OK
        0x0301 = Interrupt 0 failed
        0x0302 = Interrupt 1 failed
        0x03FF = No Ram Memory
*/

#include "address.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern void restore_tod_regs();

extern FILE *dbg_fd;
extern uchar tod_reg_flags[];

interrupt(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char q, rval, aval, bval;
u_int stat, ival;

	/* This TU is not applicable to Pegasus models: */
	/* their Timers are not enabled by hardware. */
	if (is_pegasus())
		return 0;

/* Save off the registers used by this test unit.           */
/*	The registers are referenced by indexes to a global */
/*      array called tod_reg_flags[].  The actual registers */
/*      are saved in an array called tod_reg_array[].  But  */
/*      this array is only accessed by save_tod_regs() and  */
/*      restore_tod_regs.                                   */
	clear_tod_flags();
	tod_reg_flags[F_MAIN] = 1;
	tod_reg_flags[F_TCR0] = 1;
	tod_reg_flags[F_TCR1] = 1;
	tod_reg_flags[F_IRQ0] = 1;
	tod_reg_flags[F_TIMER0L] = 1;
	tod_reg_flags[F_TIMER1L] = 1;
	tod_reg_flags[F_TIMER0M] = 1;
	tod_reg_flags[F_TIMER1M] = 1;
	save_tod_regs(fdesc, tucb_ptr);
/* End of save register code.  Remember to restore at end. */
	      
	stat = 0;

	rval = 0x7c;		/* Clear interrupt bits, also set reg=1 */
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 
	                    
	rval = 0xC0;  		/* enable timer 0 &1 interupts */
	set_tod(fdesc,IRQ0,&rval,MV_BYTE,tucb_ptr); 
	                    
	rval = PG0REG0;		/* page 0 reg 0 */
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);

	/* Set timer */
        rval = 0x00;
        set_tod(fdesc,TIMER0M,&rval,MV_BYTE,tucb_ptr);  /* reset timer 0 MSB */
        set_tod(fdesc,TIMER1M,&rval,MV_BYTE,tucb_ptr);  /* reset timer 1 MSB */

        rval = 0x09;
        set_tod(fdesc,TIMER0L,&rval,MV_BYTE,tucb_ptr);  /* reset timer 0 LSB */
        set_tod(fdesc,TIMER1L,&rval,MV_BYTE,tucb_ptr);  /* reset timer 1 LSB */

	/* Start the timers by setting start/stop bit D0, so an interrupt 
	   will be generated */
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

	rval = 0x21;		/* enable timer 0 & 1 cntrl reg */
	set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr);
	set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr);  

	/* Sleep 10ms */
	usleep(10000);

	/* read interrupt register */
	get_tod(fdesc,MAIN,&bval,MV_BYTE,tucb_ptr); 

	if(!(bval & 0x10)) stat = 1;
	if(!(bval & 0x20)) stat += 2;
	if(bval == 0xff) stat = 0xff;
	if(stat) stat += 0x0300;                   /* module ID */

	restore_tod_regs(fdesc, tucb_ptr);	/* restore TOD registers */
	return(stat);
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
