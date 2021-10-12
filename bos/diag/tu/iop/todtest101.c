static char sccsid[] = { "@(#)73	1.3  src/bos/diag/tu/iop/todtest101.c, tu_iop, bos411, 9431A411a  7/8/94  09:22:23" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: start
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
	START/STOP Test

	* Stop the timers.
	* Set the lower timers w/ 0xFF.
	* Read initial timer value to confirm they are set to 0xFF.
	* Start the timers for .005 seconds.
	* Latch the timers for a read.
	* Read the timers.
	* Counters should be decremented.

                E  R  R  O  R    M  E  S  S  A  G  E  S
        0x01xx = todtest101
        0x0000 = OK
        0x0101 = timer 0 faulty
        0x0102 = timer 1 faulty
        0x010F = timer 0/1 won't start
        0x01F0 = timer 0/1 input register faulty
        0x01FF = no ram
*/

#include "address.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern void restore_tod_regs();

extern uchar tod_reg_flags[];
extern FILE *dbg_fd;


start(fdesc, tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char q, rval, aval, bval, xval;
u_int stat;

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
	tod_reg_flags[F_TIMER0L] = 1;
	tod_reg_flags[F_TIMER1L] = 1;
	tod_reg_flags[F_TIMER0M] = 1;
	tod_reg_flags[F_TIMER1M] = 1;
	save_tod_regs(fdesc, tucb_ptr);
/* End of save register code.  Remember to restore at end. */
	      
	stat = 0;
	rval = PG0REG0;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* Main Status Reg */

	/* Lets do a no RAM test */
	get_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr);
	if(rval == 0xFF) {
		restore_tod_regs(fdesc, tucb_ptr);
		return(0x01ff);
	}

	/* TCR0/1
		D7 = 0 		Count/Hold Gate Bit off
		D6 = 0		Latch Read bit (initially) off
		D5 = 1
		D4 = 0
		D3 = 0		D3-5, Clock Speed, 1ms	
		D2 = 0
		D1 = 0		D1-2, Timer Mode 0
		D0 = 0		Start/Stop bit (initially) off		
	*/


	rval = 0x20;
	set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr);    /* stop timer 0 */
	set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr);    /* stop timer 1 */

       /* Set timer */
	rval = 0xFF;
	set_tod(fdesc,TIMER0L,&rval,MV_BYTE,tucb_ptr);  /* reset timer 0 LSB */
	set_tod(fdesc,TIMER1L,&rval,MV_BYTE,tucb_ptr);  /* reset timer 1 LSB */
	
        rval = 0x00;
        set_tod(fdesc,TIMER0M,&rval,MV_BYTE,tucb_ptr);  /* reset timer 0 MSB */
        set_tod(fdesc,TIMER1M,&rval,MV_BYTE,tucb_ptr);  /* reset timer 1 MSB */

 	/* Confirm initial counter value is 0xFF in both counters */

	get_tod(fdesc,TIMER0L,&aval,MV_BYTE,tucb_ptr);  /* get timer 0 LSB */
	get_tod(fdesc,TIMER1L,&bval,MV_BYTE,tucb_ptr);  /* get timer 1 LSB */

        if((u_int)aval != 0xFF || (u_int)bval != 0xFF) {
		restore_tod_regs(fdesc, tucb_ptr); /* restore TOD registers */
                return(0x1F0);
        }

	/* Start Timer */
	rval = 0x21;
	set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr);    /* start timer 0 */
	set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr);    /* start timer 1 */

	usleep(5000);		/* sleep .005 seconds, 5 ms */

	rval = 0x61;
	set_tod(fdesc,TCR0,&rval,MV_BYTE,tucb_ptr);    /* latch timer 0 */
	set_tod(fdesc,TCR1,&rval,MV_BYTE,tucb_ptr);    /* latch timer 1 */

	/* Read Timers */
	get_tod(fdesc,TIMER0L,&aval,MV_BYTE,tucb_ptr);  /* reset timer 0 LSB */
	get_tod(fdesc,TIMER1L,&bval,MV_BYTE,tucb_ptr);  /* reset timer 1 LSB */

	if((u_int)aval == 0xff || (u_int)bval == 0xff) {
		restore_tod_regs(fdesc, tucb_ptr);
		return(0x010F);
	} /* end if */

	/* Check if counter has been decremented. */
	if (aval > 0xFC) stat +=1;
	if (bval > 0xFC) stat +=2;

	restore_tod_regs(fdesc, tucb_ptr);	
	if (stat) stat += 0x100;
	return(stat);
}
