static char sccsid[] = { "@(#)83	1.2  src/bos/diag/tu/iop/todtest111.c, tu_iop, bos411, 9431A411a  7/8/94  09:24:27" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: low_battery
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

	Low Battery Test

	*  Set Power Fail Interrupt Enable bit, D7, in
	   the Interrupt Control Register 1.  This allows
	   the Low Battery Bit to be read.
	*  Read the Low Battery Bit, D6, on the Interrupt       
           Control Register.  Return error if set.

                E R R O R   M E S S A G E S
        0x0bxx = todtest11
        0x0000 = OK
        0x0b01 = battery low
        0x0bFF = no ram memory
*/

#include "address.h"

extern void clear_tod_flags();	/* routine to clear flag array */
extern void save_tod_regs();	/* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */
extern FILE *dbg_fd;

low_battery(fdesc,tucb_ptr)
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
	  tod_reg_flags[F_IRQ1] = 1;
	  save_tod_regs(fdesc, tucb_ptr);	/* Save the registers. */
/* End of save reg code.  Remember to restore before return(). */
	      
	stat = 0;
	
	/* Must first set Power Fail Interrupt Enable bit, D7, in 
	   the Interrupt Control Register 1 (IRQ1) to read the low 
           battery bit on the Interrupt Route Register (IRQROUTE). */

	rval = PG0REG1;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* Main Status Reg */
	rval = 0x80;
	set_tod(fdesc,IRQ1,&rval,MV_BYTE,tucb_ptr);  

	/* read Low Battery Bit, D6, on Interrupt Route Register. */
	rval = PG0REG0;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 
	get_tod(fdesc,IRQROUTE,&xval,MV_BYTE,tucb_ptr); 
	if(xval == 0xFF) {
		restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD reg's */
		return(0x0bFF);
	}
	xval &= 0x40;
	if(xval)
		stat = 0x0b01;  	    /* module ID & error */

	restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
	return(stat);
}
/*-------------------------------------------------------------------------*/
