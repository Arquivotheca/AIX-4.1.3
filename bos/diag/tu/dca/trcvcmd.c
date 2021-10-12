static char sccsid[] = "src/bos/diag/tu/dca/trcvcmd.c, tu_tca, bos411, 9428A410j 6/19/91 15:31:03";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: testrcvcmd
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/io3270.h>
#include "tcatst.h"

#ifdef debugg
extern void detrace();
#endif

/*
 * NAME: testrcvcmd
 *
 * FUNCTION: Read Receive/Command Registers
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: A zero (0) when successful or a 0x020D on error condition
 *
 */
int testrcvcmd (filehandle, diag_data_ptr, tst, reg, mask, expect)
   int filehandle;
   C327DIAG_DATA *diag_data_ptr;
   int tst;
   BYTE reg;
   BYTE mask;
   BYTE expect;
   {
	diag_data_ptr->address = reg;

	if ( (ioctl(filehandle,C327DIAG_GET_RCV_CMD,diag_data_ptr, 0) == -1) ||
	     ( (diag_data_ptr->recv_byte & mask) != expect) )
	   {
#ifdef debugg
    detrace(1,"Test Rcv Cmd failed: tst=%d reg=%x r=%x m=%x e=%x\n",
	 	tst, reg, diag_data_ptr->recv_byte, mask, expect);
#endif
#ifdef C327DIAG_DEBUG_PRINT_MODE
    printf("Test Rcv Cmd failed: errno=%d  tst=%d  reg=%x  r=%x  m=%x  e=%x\n",
		errno, tst, reg, diag_data.recv_byte, mask, expect);
#endif
		return(0x020D);
	   }
	return(0);
   }
