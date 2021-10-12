static char sccsid[] = "src/bos/diag/tu/dca/tu003.c, tu_tca, bos411, 9428A410j 8/7/92 10:58:31";
/*
 * COMPONENT_NAME: (TCATU) TCA/DCA Test Unit
 *
 * FUNCTIONS: c327DiagIntrTest, tu003
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/io3270.h>
#include <errno.h>

#include "tcatst.h"

#ifdef debugg
extern void detrace();
#endif

#define RESET_CMD 0x02

/*
 * NAME: c327DiagIntrTest
 *
 * FUNCTION: 3270 Connection Adapter test of interrupt function
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: A zero (0) if successful or a non-zero on error condition
 *
 */
int c327DiagIntrTest (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc,i;
	C327DIAG_DATA diag_data;
	struct htx_data *htx_sp;
	extern int writereg();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->tca_htx_s.htx_sp;

	/* Write to connection control register with bits to:	*/
	/*   - enable interrupts */
	if ( (rc = writereg(fdes, &diag_data, 3009,
	   adapter_conn_ctrl_reg, 0x00)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0301));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

        /* wait 1 second to allow any pending interrupts to surface  */
        sleep(1);

	/* Reset the Diagnostic Saved Register */
	if (ioctl(fdes, C327DIAG_I_STAT_RESET, &diag_data, 0) == -1)
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("Intr Stat Reset failed:  errno=%d  tst=3010\n", errno);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		
		if (errno == EIO)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0302));

		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR,
				errno));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Write RESET command to scan code register.  Remember	*/
	/* that you MUST put the ones complement of it there!	*/
	if ((rc = writereg(fdes, &diag_data, 3011, adapter_scan_code_reg,
		 ~RESET_CMD)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0303));
	   }

	/* Write to connection control register with bits to:	*/
	/*   - be in TEST mode,					*/
	/*   - specify COMMAND is in scan code register, and	*/
	/*   - tells chip that scan code register is loaded	*/
	if ( (rc = writereg(fdes, &diag_data, 3012,
	   adapter_conn_ctrl_reg, 0x38)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0304));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Allow 5 seconds between enabling/disabling interrupt to	*/
	/* insure that adapter has received the command and had time to */
	/* post some interrupt values in the interrupt status register. */
	sleep(5);

	/* Now check if we got an interrupt	*/
	if (ioctl(fdes, C327DIAG_I_STAT_READ, &diag_data, 0) == -1)
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("Intr Stat Read failed:  errno=%d  tst=3013  r=%x\n",
			errno, diag_data.recv_byte);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		
		if (errno == EIO)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0305));
				
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }
	if (diag_data.recv_byte == 0)
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("Intr Stat Read failed:  errno=%d  tst=3013  r=%x\n",
			errno, diag_data.recv_byte);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0308));
	   }
	
	/* Status Register should have bits set specifying:		*/
	/*   -  bit 7: interrupt					*/ 
	/*   -  bit 4: set internally at end of POWER-ON SEQUENCE	*/
	/*   -  bit 1: reset command received from the controller	*/
	if ((diag_data.recv_byte & 0x82) != 0x82)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0309));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(0);
   }

/*
 * NAME: tu003
 *
 * FUNCTION: Interrupt Test
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: The return code from the c327DiagIntrTest function call
 *
 */
int tu003 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(c327DiagIntrTest(fdes, tucb_ptr));
   }
