static char sccsid[] = { "@(#)78	1.2  src/bos/diag/tu/iop/todtest106.c, tu_iop, bos411, 9431A411a  7/8/94  09:23:19" };
/*
 *   COMPONENT_NAME: TU_IOP
 *
 *   FUNCTIONS: todnvram
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

	NVRAM

	* Locates the NVRAM bytes 1 through 31
	* Sends 0x00 to 0xFF to each byte 
	* Performs a read-after-write check for validity
	* Report errors and exits
	
                E R R O R   M E S S A G E S
        0x06xx = todtest106
        0x0000 = OK
        0x06XX = byte containing failed bit
*/

#include "address.h"

extern void clear_tod_flags();
extern void save_tod_regs();
extern void restore_tod_regs();

extern uchar tod_reg_flags[];
extern FILE *dbg_fd;

todnvram(fdesc,tucb_ptr)
int fdesc;
TUTYPE *tucb_ptr;
{
u_char rval, xval;
u_int i, j, stat;

/* Save off the registers used by this test unit.              */
/*	The registers are referenced by indexes to a global    */
/*      array called tod_reg_flags[].  The actual registers    */
/*      are saved in an array called tod_reg_array[].  But     */
/*      this array should only be accessed via save_tod_regs() */
/*      and restore_tod_regs().                                */
	clear_tod_flags();
	tod_reg_flags[F_MAIN] = 1;
	tod_reg_flags[F_PG1RAM] = 1;
	save_tod_regs(fdesc, tucb_ptr);
/* End of save register code.  Remember to restore at end. */
	      
	stat = 0;
	rval = PG1REG0;                                /* page select 1 */
	set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); /* Main Status Reg */
	for (i=1; i <= 31; i++ ) {
	  	for (j = 0; j <= 255; j++ ) {
			rval = (u_char)j;    
			/* write to ram */
			set_tod(fdesc,MAIN+i,&rval,MV_BYTE,tucb_ptr);  
			/* write to ram */
			get_tod(fdesc,MAIN+i,&xval,MV_BYTE,tucb_ptr);      
			if (xval != rval)
	         		{
	         		/* stat == ram address error + ID */
	         		stat = 0x0600 + j;
				rval = 0x00;   /* page select 0 */  
		 		/* write to control register*/ 
				set_tod(fdesc,MAIN,&rval,MV_BYTE,tucb_ptr); 
				/* restore TOD registers */
				restore_tod_regs(fdesc, tucb_ptr); 
				return(stat);
			} /* endif */
		} /* for */
	} /* for */

	restore_tod_regs(fdesc, tucb_ptr);  /* restore TOD registers */
	return(stat);
}
/*------------------------------------------------------------------*/
