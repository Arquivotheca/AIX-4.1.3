static char sccsid[] = "src/bos/diag/tu/dca/tu004.c, tu_tca, bos411, 9428A410j 6/19/91 15:31:13";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: c327DiagDftTest, c327Diag87ETest, tu004
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
#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/io3270.h>
#include <errno.h>
#include "tcatst.h"

#ifdef debugg
extern void detrace();
#endif

/*
 * NAME: c327DiagDftTest
 *
 * FUNCTION: 3270 Connection Adapter test of DFT mode
 *
 * EXECUTION ENVIRONMENT: In Test and DFT Modes
 *
 * (NOTES:) This function will simulate a Power-On-Reset and then enter
 *	    the Test and DFT modes for the rest of the function.  Once
 *	    it has entered the Test and DFT mode, it will reset the Interrupt
 *	    Status Register before and/or between all additional tests
 * 
 *          Commands Simulated:
 *
 *	       1 - Start Operation Command with verification checks
 *	           made in the Receive, Command and Interrupt Registers.  
 *
 *	       2 - Diagnostic Reset Command with verification checks
 *	           made in the Command and Interrupt Registers.
 *
 *	       3 - Read Terminal ID Command with verification checks
 *	           made in the Command and Interrupt Registers.
 *
 *	    After all the commands are simulated, the function will exit the
 *	    Test and DFT modes.
 *
 * RETURNS: A zero (0) when successful or a non-zero on error condition
 *
 */
int c327DiagDftTest (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	C327DIAG_DATA diag_data;
	int rc;
	struct htx_data *htx_sp;

	extern int writereg();
	extern void short_delay();
	extern int simwrcmd();
	extern int testrcvcmd();
	extern int readreg();
	extern int simrdcmd();
	extern int mktu_rc();

	/* Set up a pointer to HTX data structure to increment counters	*/
	/* in case the TU was invoked by the hardware exerciser.	*/
	htx_sp = tucb_ptr->tca_htx_s.htx_sp;

	/* Enter into Test Mode and DFT Mode	*/
	if ((rc = writereg(fdes, &diag_data, 4005, adapter_conn_ctrl_reg,
		   CONN_CTRL_INT_INH | CONN_CTRL_TEST | CONN_CTRL_DFT_MODE )
				   ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0402));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	short_delay(50);

	/* Reset the Interrupt Status Register	*/
	if ( (rc = writereg(fdes, &diag_data, 4010,
			adapter_intr_stat_reg, 0x3F)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0403));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Simulate a Start Operation command then check the Receive	*/
	/* Register, Command Register and the Interrupt Register.	*/
	if ( ( rc = simwrcmd(fdes, &diag_data, 4011,
			0x08 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0404));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Check the Receive Register for the value 0x08	*/
	if ( ( rc = testrcvcmd(fdes, &diag_data, 4012,
			0x0A, 0xFF, 0x08) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0405));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Check the Command Register for the value 0x08	*/
	if ( ( rc = testrcvcmd(fdes, &diag_data, 4013,
			0x0B, 0x7F, 0x08)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0406));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	short_delay(50);

	/* Check the Interrupt Register for the value 0x01	*/
	if ( ( rc = readreg(fdes, &diag_data, 4014,
			adapter_intr_stat_reg, 0x01, 0x01 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0407));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Reset the Interrupt Status Register	*/
	if ( ( rc = writereg(fdes, &diag_data, 4020,
			adapter_intr_stat_reg, 0x3F ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0408));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Simulate a Diagnostic Reset Command then check the Command	*/
	/* Register and the Interrupt Register				*/
	if ( ( rc = simwrcmd(fdes, &diag_data, 4021,
			0x1C ) ) != 0 )
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return (mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0409));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Check the Command Register for a value of 0x1C	*/
	if ( ( rc = testrcvcmd(fdes, &diag_data, 4022,
			0x0B, 0x7F, 0x1C)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x040A));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	short_delay(50);

	/* Check the Interrupt Register for a value of 0x04	*/
	if ( ( rc = readreg(fdes, &diag_data, 4023,
			adapter_intr_stat_reg, 0x04, 0x04 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x040B));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Escape to clear the Command Register	*/
	if ( ( rc = simwrcmd(fdes, &diag_data, 4029,
			0x1E ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x040C));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Reset the Interrupt Status Register	*/
	if ( ( rc = writereg(fdes, &diag_data, 4030,
			adapter_intr_stat_reg, 0x3F)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x040D));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Simulate a Read Terminal Id Command the check the Command	*/
	/* Register and the Interrupt Register				*/
	if ( ( rc = simrdcmd(fdes, &diag_data, 4031,
			0x09 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x040E));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Check the Command Register for the value 0x09	*/
	if ( ( rc = testrcvcmd(fdes, &diag_data, 4032,
			0x0B, 0x7F, 0x09 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x040F));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	short_delay(50);

	/* Check the Interrupt Register for the value 0x08	*/
	if ( ( rc = readreg(fdes, &diag_data, 4033,
			adapter_intr_stat_reg, 0x08, 0x08 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0410));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* Exit out of Test Mode and DFT Mode	*/
	if ( ( rc = writereg(fdes, &diag_data, 4099,
			adapter_conn_ctrl_reg, CONN_CTRL_INT_INH ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0411));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(0);
   }

/*
 * NAME: c327Diag87Test
 *
 * FUNCTION: 3270 Connection Adapter test of 3287 mode
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: A zero (0) when successful or a non-zero on error condition
 *
 */
int c327Diag87ETest (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	C327DIAG_DATA diag_data;
	int rc;
	struct htx_data *htx_sp;

	extern int writereg();
	extern void short_delay();
	extern int readreg();
	extern int simwrcmd();
	extern int testrcvcmd();
	extern int mktu_rc();

	/* Set up a pointer to the HTX data structure to increment	*/ 
	/* counters in case the TU was invoked by hardware exerciser	*/
	htx_sp = tucb_ptr->tca_htx_s.htx_sp;

	/* Enter into Test Mode and 3287 Mode */
	if ( ( rc = writereg(fdes, &diag_data, 5005,
		adapter_conn_ctrl_reg,
		CONN_CTRL_INT_INH | CONN_CTRL_TEST | CONN_CTRL_87E_MODE)) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0413));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	short_delay(50);
	/* test the 3287 status register */
	if ( (rc = writereg(fdes, &diag_data, 5011,
			adapter_87e_stat_reg, 0xE0 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0414));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	if ( (rc = readreg(fdes, &diag_data, 5012,
			adapter_87e_stat_reg, 0xFF, 0x00 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0415));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* reset the cursor and test it */
	if ( (rc = writereg(fdes, &diag_data, 5021,
		adapter_conn_ctrl_reg, CONN_CTRL_INT_INH | CONN_CTRL_TEST |
		CONN_CTRL_87E_MODE | CONN_CTRL_RST_CUR ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0416));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	if ( (rc = writereg(fdes, &diag_data, 5022,
		adapter_conn_ctrl_reg, CONN_CTRL_INT_INH | CONN_CTRL_TEST |
		CONN_CTRL_87E_MODE ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0417));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	if ( (rc = readreg(fdes, &diag_data, 5023,
		adapter_lsb_cur_reg, 0xFF, 0x00 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0417));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* reset interrupt/status register */
	if ( (rc = writereg(fdes, &diag_data, 5030,
			0x00, 0x3F ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0418));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/*
	 * simulate a start op, chk recv reg, cmd reg,
	 * 87e stat reg, & intr reg
	 */
	if ( (rc = simwrcmd(fdes, &diag_data, 5031,
			0x08 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0419));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	if ( (rc = testrcvcmd(fdes, &diag_data, 5032,
			0x0A, 0xFF, 0x08 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x041A));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	if ( (rc = testrcvcmd(fdes, &diag_data, 5033,
			0x0B, 0x7F, 0x08 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x041B));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	short_delay(50);

/* old test looks for an 80 not 00 */

	if ( (rc = readreg(fdes, &diag_data, 5034,
			adapter_87e_stat_reg, 0xFF, 0x00 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x041C));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

/* old test looks for an 01 not 00 */

	if ( (rc = readreg(fdes, &diag_data, 5035,
			adapter_intr_stat_reg, 0x01, 0x00 ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x041D));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	/* get out of test mode and out of 3287 mode */
	if ( (rc = writereg (fdes, &diag_data, 5099,
			adapter_conn_ctrl_reg, CONN_CTRL_INT_INH ) ) != 0)
	   {
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x041E));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	return(0);
   }

/*
 * NAME: tu004
 *
 * FUNCTION: DFT Mode and 87E Mode Tests
 *
 * EXECUTION ENVIRONMENT: Works under the DFT Mode and then under the 87E 
 *			  mode.
 *
 * (NOTES:) 
 *
 * RETURNS: The return code from the c327DiagDftTest function call if an error
 *          occured or the return code from the c327Diag87ETest function call. 
 *
 */
int tu004 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;

	rc = c327DiagDftTest(fdes, tucb_ptr);
	if (rc)
		return(rc);

	return(c327Diag87ETest(fdes, tucb_ptr));
   }
