static char sccsid[] = { "@(#)85	1.2  src/bos/diag/tu/iop/todtest113.c, tu_iop, bos411, 9431A411a  7/8/94  09:24:44" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: pf_irq
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

	POWER FAILURE

	* Write to IRQ ROUTE Power Fail bit, D0, and check
	* Write to IRQ #1 Power Fail bit, D7, and check

                E R R O R   M E S S A G E S
        0x0dxx = todtest113
        0x0000 = OK
        0x0d01 = IRQROUTE Power Failed bit failed
        0x0d02 = IRQ # 1 Power Failed bit failed
        0x0d04 = MAIN Power Failed Bit failed
        0x0dFF = No ram memory
*/

#include "address.h"

extern void clear_tod_flags();	/* routine to clear flag array */
extern void save_tod_regs();	/* routine to save tod registers */
extern void restore_tod_regs(); /* routine to restore tod registers */

extern uchar tod_reg_flags[];   /* array of flags for saved TOD regs */
extern FILE *dbg_fd;

pf_irq(fdesc,tucb_ptr)
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
	tod_reg_flags[F_IRQROUTE] = 1;
	save_tod_regs(fdesc, tucb_ptr);	/* Save the registers. */
/* End of save reg code.  Remember to restore before return(). */
	      
	stat = 0;

	/* Write to IRQROUTE power failure bit, D0, and verify 
           that it gets set. */
	rval = PG0REG0;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 
	rval = 0x01;
	set_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);        
	get_tod(fdesc,IRQROUTE,&rval,MV_BYTE,tucb_ptr);       
	if(rval == 0xFF) {
		restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
		return(0x0dFF);
	}
	if(!(rval & 0x01)) stat = 0x01;

	/* Write to IRQ #1 power failure bit, D7, and verify 
           that it gets set. */
	rval = PG0REG1;
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 
	rval = 0x80;
	set_tod(fdesc,IRQ1,&rval,MV_BYTE,tucb_ptr); 
	get_tod(fdesc,IRQ1,&rval,MV_BYTE,tucb_ptr);
	if(!(rval & 0x80)) stat |= 0x02;

	if(stat) stat += 0x0d00;                        /* module ID */

	restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
	return(stat);
}
/*--------------------------------------------------------------------*/
