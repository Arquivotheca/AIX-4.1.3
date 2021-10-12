static char sccsid[] = { "@(#)76	1.2  src/bos/diag/tu/iop/todtest104.c, tu_iop, bos411, 9431A411a  7/8/94  09:22:56" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: osc_fail
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

	OSCILLATOR BIT TEST

	* Read Oscillator bit, D6, in Periodic Flag Register
		- if bit set, return error.

                E R R O R   M E S S A G E S
        0x04xx = todtest104
        0x0000 = OK
        0x0401 = Oscillator failed
        0x04FF = no ram memory
*/

#include "address.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern void restore_tod_regs();

extern uchar tod_reg_flags[];
extern FILE *dbg_fd;

osc_fail(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char rval;
u_int stat;

/* Save off the registers used by this test unit.           */
/*	The registers are referenced by indexes to a global */
/*      array called tod_reg_flags[].  The actual registers */
/*      are saved in an array called tod_reg_array[].  But  */
/*      this array is only accessed by save_tod_regs() and  */
/*      restore_tod_regs.                                   */
	clear_tod_flags();
	tod_reg_flags[F_MAIN] = 1;
	tod_reg_flags[F_PERFLAG] = 1;
	save_tod_regs(fdesc, tucb_ptr);
/* End of save register code.  Remember to restore at end. */
        
	rval = PG0REG0;   /* page select 0, reg select 0 */
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 

	/* Get Periodic Flag bit */
	get_tod(fdesc,PERFLAG,&rval,MV_BYTE,tucb_ptr);  
	if(rval == 0xFF) {
		restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
		return(0x04FF);
	}
	rval &= 0x40;                  /* strip off all except osc fail bit */
	if (rval == 0)
		stat = 0;
	else
		stat = 0x0401;

	restore_tod_regs(fdesc, tucb_ptr);	/* restore TOD registers */
	return(stat);
}
/*----------------------------------------------------------------*/
