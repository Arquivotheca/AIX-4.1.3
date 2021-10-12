static char sccsid[] = "src/bos/diag/tu/dca/rwtreg.c, tu_tca, bos411, 9428A410j 6/19/91 15:30:21";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: readreg, writereg, testreg
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
 * NAME: readreg
 *
 * FUNCTION: Read Register
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will read the register.
 *
 * RETURNS: A zero (0) if successful or 0x0102 on error condition
 *
 */
int readreg (filehandle, diag_data_ptr, tst, reg, mask, expect)
   int filehandle;
   C327DIAG_DATA *diag_data_ptr;
   int tst;
   BYTE reg;
   BYTE mask;
   BYTE expect;
   {
	diag_data_ptr->address   = reg;

	if ( (ioctl(filehandle, C327DIAG_REG_READ, diag_data_ptr, 0) == -1) ||
             ((diag_data_ptr->recv_byte & mask) != expect)  )
           {
#ifdef C327DIAG_DEBUG_PRINT_MODE
        printf("Reg Read failed: tst=%d  reg=%x  r=%x  m=%x  e=%x\n",
                tst, reg, diag_data_ptr->recv_byte, mask, expect);
#endif
		return(0x0102);
	   }
	return(0);
   }

/*
 * NAME: writereg
 *
 * FUNCTION: Write register
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will write the register.
 *
 * RETURNS: A zero (0) if successful or 0x0103 on error condition
 *
 */
int writereg (filehandle, diag_data_ptr, tst, reg, send)
   int filehandle;
   C327DIAG_DATA *diag_data_ptr;
   int tst;
   BYTE reg;
   BYTE send;
   {
        diag_data_ptr->address   = reg;
        diag_data_ptr->send_byte = send;

	if (ioctl(filehandle, C327DIAG_REG_TEST, diag_data_ptr, 0) == -1)
           {
#ifdef C327DIAG_DEBUG_PRINT_MODE
             printf("Reg Write failed: errno=%d  tst=%d  reg=%x  w=%x\n",
                        errno, tst, reg, send);
#endif
		return (0x0103);
	   }
	return(0);
   }

/*
 * NAME: testreg
 *
 * FUNCTION: Test register
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure will test the register.
 *
 * RETURNS: A zero (0) if successful or 0x0101 on error condition
 *
 */
int testreg (filehandle, diag_data_ptr, tst, reg, send, mask, expect)
   int filehandle;
   C327DIAG_DATA *diag_data_ptr;
   int tst;
   BYTE reg;
   BYTE send;
   BYTE mask;
   BYTE expect;
   {
	diag_data_ptr->address   = reg;
	diag_data_ptr->send_byte = send;

	if ( (ioctl(filehandle, C327DIAG_REG_TEST, diag_data_ptr, 0) == -1) ||
	     ( (diag_data_ptr->recv_byte & mask) != expect)  )
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
 printf("Reg Test failed: errno=%d  tst=%d  reg=%x  w=%x  r=%x  m=%x  e=%x\n",
         errno, tst, reg, send, diag_data_ptr->recv_byte, mask, expect);
#endif
		return(0x0101);
	   }
	return(0);
   }
