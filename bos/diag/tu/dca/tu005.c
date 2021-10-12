static char sccsid[] = "src/bos/diag/tu/dca/tu005.c, tu_tca, bos411, 9428A410j 6/19/91 15:30:47";
/*
 * COMPONENT_NAME: (TU_TCA) TCA/DCA Test Unit
 *
 * FUNCTIONS: c327DiagConnTest, tu005
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
 * NAME: c327DiagConnTest
 *
 * FUNCTION: 3270 Connection Adapter test for current connection
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: A zero (0) when successful or a non-zero on error condition
 *
 */
int c327DiagConnTest (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	C327DIAG_DATA diag_data;
	int rc;
	struct htx_data *htx_sp;
	extern int mktu_rc();

	/*
	 * set up a pointer to HTX data structure to
	 * increment counters in case tu was invoked by
	 * hardware exerciser.
	 */
	htx_sp = tucb_ptr->tca_htx_s.htx_sp;

	/* this test takes special code resident in c327dd because of timing */
	if (ioctl(fdes, C327DIAG_CONN_TEST, &diag_data, 0) == -1)
	   {
#ifdef C327DIAG_DEBUG_PRINT_MODE
		printf("Conn Test failed:  errno=%d  tst=6000  r=%x\n",
			errno, diag_data.recv_byte);
#endif

		if (htx_sp != NULL)
			(htx_sp->bad_others)++;
		
		if (errno == EIO)
			return(mktu_rc(tucb_ptr->header.tu, LOG_ERR, 0x0501));
		return(mktu_rc(tucb_ptr->header.tu, SYS_ERR, errno));
	   }
	if (htx_sp != NULL)
		(htx_sp->good_others)++;
	return(0);
   }

/*
 * NAME: tu005
 *
 * FUNCTION: Control Unit Connection Test
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:) 
 *
 * RETURNS: The return code from the c327DiagConnTest function call
 *
 */
int tu005 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
	return(c327DiagConnTest(fdes, tucb_ptr));
   }
