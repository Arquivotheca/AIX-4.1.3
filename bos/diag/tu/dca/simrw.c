static char sccsid[] = "src/bos/diag/tu/dca/simrw.c, tu_tca, bos411, 9428A410j 6/19/91 15:30:26";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: simrdcmd, simwrcmd
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
 * NAME: simrdcmd
 *
 * FUNCTION: Read Simulation from Control Unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function calls the Device Driver via the IOCTL 
 *          C327DIAG_SIM_RD_CMD.  It will simulate the receipt of
 *	    a read type command from the Control Unit.
 *
 * RETURNS: A zero (0) when successful or a 0x020C on error condition
 *
 */
int simrdcmd (filehandle, diag_data_ptr, tst, send)
   int filehandle;
   C327DIAG_DATA *diag_data_ptr;
   int tst;
   BYTE send;
   {
	diag_data_ptr->send_byte = send;

	if (ioctl(filehandle, C327DIAG_SIM_RD_CMD, diag_data_ptr, 0) == -1)
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("Sim Rd Cmd failed: errno=%d  tst=%d  w=%x\n",
                        errno, tst, send);
#endif
		return(0x020C);
	   }
	return(0);
   }

/*
 * NAME: simwrcmd
 *
 * FUNCTION: Write Simulation from Control Unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This function calls the Device Driver via the IOCTL 
 *          C327DIAG_SIM_WR_CMD.  It will simulate the receipt of
 *	    a write type command from the Control Unit.
 *
 * RETURNS: A zero (0) when successful or a 0x020B on error condition
 *
 */
int simwrcmd (filehandle, diag_data_ptr, tst, send)
   int filehandle;
   C327DIAG_DATA *diag_data_ptr;
   int tst;
   BYTE send;
   {
	diag_data_ptr->send_byte = send;

	if (ioctl(filehandle, C327DIAG_SIM_WR_CMD, diag_data_ptr, 0) == -1)
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("Sim Wr Cmd failed: errno=%d  tst=%d  w=%x\n",
			errno, tst, send);
#endif
                return(0x020B);
	   }
	return(0);
   }
