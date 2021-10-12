static char sccsid[] = "src/bos/diag/tu/dca/tu002.c, tu_tca, bos411, 9428A410j 6/19/91 15:31:08";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: testbyte, c327DiagMemTest, tu002
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

#include <time.h>
#include "tcatst.h"

#define MAX_RETRIES	3

#ifdef debugg
extern void detrace();
#endif

/*
 * NAME: testbyte
 *
 * FUNCTION: 3270 Connection Adapter test byte of adapter's RAM memory
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) This procedure uses the Device Driver IOCTL C327DIAG_TCA_TEST
 *          function.  It will test each byte of the adapter's RAM one
 *          byte at a time.  Each byte written will be different than the
 *          previous byte.  Should an error occur, up to two (2) additional
 *          retries will be attempted at the same location with different
 *          values before an error is logged.
 *
 * RETURNS: Any and all returns code will be returned the the calling routine.	
 *	    A zero if successful or a non-zero if unsuccessful.
 *
 */
int testbyte (filehandle, diag_data_ptr, TCA_size, tucb_ptr)
   int filehandle;
   C327DIAG_DATA *diag_data_ptr;
   int TCA_size;
   TUTYPE *tucb_ptr;
   {
	int ndx, i, j;
	struct htx_data *htx_sp;
	extern int mktu_rc();
/*
	extern struct htx_data *global_htx_sp;
	extern void htx_err();
*/
	i=0x00;
	j=0;
	/* Set up a pointer to HTX data structure to increment counters	*/
	/* in case the TU was invoked by the hardware exerciser.	*/
	htx_sp = tucb_ptr->tca_htx_s.htx_sp;

	/* For the size of the Adapter's RAM, test one (1) byte at a	*/
	/* time by writing then reading one byte from RAM.  To insure	*/
	/* that the byte read is the byte written, the byte written     */
	/* be incremented from 0x00 through 0xFE; therefore, decreasing	*/
	/* the chance that a random (unknown) alteration to memory will */
	/* somehow match the byte written.				*/
	for (ndx = 0; ndx < TCA_size; ndx++)
	{
	 	diag_data_ptr->address = ndx;
		diag_data_ptr->send_byte = i;
		diag_data_ptr->recv_byte = 0x00;
		if (ioctl(filehandle,C327DIAG_TCA_TEST,diag_data_ptr,0) == -1)
		{
#ifdef C327DIAG_DEBUG_PRINT_MODE
			printf("Test single byte cmd failed: errno = %d",
				errno);
#endif
		/* We are unable to tell if the failure was because of	*/
		/* the write or the read.  We will increment each bad	*/
		/* count by the value of one (1).			*/
		if (htx_sp != NULL)
			{
				(htx_sp->bad_writes)++;
				(htx_sp->bad_reads)++;
			}

		if (errno == EIO)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0203));

		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
		}

		/* We will increment the good counts by one (1) to	*/
		/* signal that a good write and read has been done.	*/
		if (htx_sp != NULL)
		{			
			(htx_sp->good_writes)++;	
			(htx_sp->bytes_writ)++;
			(htx_sp->good_reads)++;	
			(htx_sp->bytes_read)++;
		}

		/* Check to see if the byte read from RAM matches the	*/
		/* byte written to RAM.					*/	
		if (diag_data_ptr->recv_byte != i)
		{
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("Mem test failed: errno=%d  loc=%d  w=%x  r=%x\n",
			errno, ndx, i, diag_data_ptr->recv_byte);
#endif
			/* There is a mismatch.  Increment retry count.	*/
			j++;

			/** Debug statements - Writes 0xFF at C2000 **********
			diag_data_ptr->address = 0;
			diag_data_ptr->send_byte = 0xff;
			diag_data_ptr->recv_byte = 0x00;
			ioctl(filehandle, C327DIAG_TCA_TEST, diag_data_ptr,0);
			htx_err(global_htx_sp,0,0,
				"MEM TEST ERROR Size=%d Loc=%d W=%x R=%x",
				TCA_size,ndx,i,diag_data_ptr->recv_byte);
			 ******** End of debugging statement ****************/

			/* The bytes do not match.  Increment the bad 	*/
			/* other count to show this miscompare.		*/
			if (htx_sp != NULL)
				(htx_sp->bad_others)++;

			/* We tried 3 times.  Its really and error.	*/
                        if (j == MAX_RETRIES)
				return(mktu_rc(tucb_ptr->header.tu, LOG_ERR,
					 0x0206));
			/* The byte read does not match the byte written. */
			/* Reduce NDX (location counter) by one so the    */
			/* same location will be re-written with another  */
			/* byte value.					  */
			else {  
				ndx--;
			     }			
		}
		/* The byte read matches the byte written.	*/
		/* Reset the retry counter and continue.	*/
		else j=0;
		i++;
		/* I use the value 0xFF for debug purposes only.	*/
		if (i == 0xfe) i=0x00;
	}					
	return(0);
   }

/*
 * NAME: c327DiagMemTest
 *
 * FUNCTION: 3270 Connection Adapter test of adapter memory
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: A zero (0) when successful or a non-zero on error condition
 *
 */
int c327DiagMemTest (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	int rc;
	int TCA_size;     /* size of TCA */
	C327DIAG_DATA diag_data;
	struct htx_data *htx_sp;
	extern int mktu_rc();

	/* Set up a pointer to HTX data structure to increment counters	*/
	/* in case the TU was invoked by the hardware exerciser.	*/
	htx_sp = tucb_ptr->tca_htx_s.htx_sp;

	/* Find out the amount of Adapter RAM	*/
	if ((rc = ioctl(fdes, C327DIAG_TCA_SIZE, &diag_data)) == -1)
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("TCA Size request failed: errno=%d\n", errno);
#endif
		if (htx_sp != NULL)
			(htx_sp->bad_others)++;

		if (errno == EIO)
			return(mktu_rc(tucb_ptr->header.tu,
				LOG_ERR, 0x0207));
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;

	TCA_size = diag_data.address;

	/* Call testbyte to write and read/verify each byte of memory */
	rc = testbyte(fdes, &diag_data, TCA_size, tucb_ptr);

	return(rc);
   }

/*
 * NAME: tu002
 *
 * FUNCTION: Adapter's RAM test
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: The return code from the c327DiagMemTest function call
 *
 */
int tu002 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(c327DiagMemTest(fdes, tucb_ptr));
   }
