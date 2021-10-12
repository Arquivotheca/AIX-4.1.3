static char sccsid[] = { "@(#)86	1.2  src/bos/diag/tu/iop/todtest114.c, tu_iop, bos411, 9431A411a  7/8/94  09:24:53" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: elapsed
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

	Elapsed Time Test

	* Clear Periodic bits by reading the Periodic Flag Register.
	* Set 1ms, 10ms, 100 Interrupt Enable Bits, D3-D5, in the
	     Interrupt Control Register 0.
	* Sleep .2 seconds.
	* Read Periodic Flag Register, bits D3-D5 should be set.
		- return error if bits not set. 

                E R R O R   M E S S A G E S
        0x0exx = todtest114
        0x0000 = OK
        0x0e01 = Time did not advance
        0x0eFF = No ram memory
*/

#include "address.h"

extern void clear_tod_flags();	/* routine to clear flag array */
extern void save_tod_regs();	/* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */
extern FILE *dbg_fd;

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */

elapsed(fdesc,tucb_ptr)
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
	tod_reg_flags[F_PERFLAG] = 1;	/* this TU.		       */
	tod_reg_flags[F_IRQ0] = 1;
	save_tod_regs(fdesc, tucb_ptr);	/* Save the registers. */
/* End of save reg code.  Remember to restore before return(). */
	      
	stat = 0;
	                    /* page 0 reg 0 */
	rval = PG0REG0;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 

	/* Clear Periodic Flag register by reading */
	get_tod(fdesc,PERFLAG,&xval,MV_BYTE,tucb_ptr);   


	rval = PG0REG1;		/* set reg=1 to access IRQ0 */
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 

	/* Set 1ms, 10ms, 100ms Interrupt Enable, D3-D5, in the
	   Interrupt Control Register 0 */
	rval= 0x38;
	set_tod(fdesc,IRQ0,&rval,MV_BYTE,tucb_ptr);     

	/* sleep .2 seconds */
	usleep(200000);

	rval = PG0REG0;		/* set reg=0 to access PERFLAG */
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 

	/* Read the Periodic Flag register.  1ms, 10ms, 100ms flags,
	   D3-D5, should all be set. */ 
	get_tod(fdesc,PERFLAG,&rval,MV_BYTE,tucb_ptr);       
	if(rval == 0xFF) {
		restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
		return(0x0eFF);
	}

	/* strip off all PERFLAG bits except 1ms, 10ms, 100ms, D3-D5 */
	rval &= 0x38;    
	if (rval == 0x38)
		stat = 0;
	else
		stat = 0x0e01;

	restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
	return(stat);
}
/*--------------------------------------------------------------------*/
