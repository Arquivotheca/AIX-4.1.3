static char sccsid[] = "@(#)18	1.3  src/bos/diag/tu/pcitok/sky_tu002.c, tu_pcitok, bos41J, 9519B_all 5/12/95 11:13:00";
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: io_test
 *		tu002
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/

/*** header files ***/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mdio.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <cf.h>

/* local header files */
#include "skytu_type.h"
#include "sky_regs.h"
#include "skytabs.h"


/*****************************************************************************
*
* FUNCTION NAME =  tu002()
*
* DESCRIPTION   =  This function calls the necessary routines to perform
*                  the I/O register tests.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = exectu
*
*****************************************************************************/
int tu002 (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  int rc = 0; /* return code */

  rc = io_test(adapter_info, tucb_ptr);

  DEBUG_1("Return from I/O test 0x%x\n",rc);
  return (rc);
}


/****************************************************************************
*
* FUNCTION NAME =  io_test()
*
* DESCRIPTION   =  This function test writes/reads/compares byte values to
*                  check all bit positions in writable I/O registers.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = tu002
*
*****************************************************************************/
int io_test (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  uint           io_addr;
  ushort         rd_data, test_val;
  int            i, j, k, retries, rc = 0;
  BOOLEAN        SUCCESS, OK;

  /*********************
   * Reset adapter
   *********************/

  if (rc = reset_adapter(adapter_info, tucb_ptr, IO_TEST)) {
	return (rc);
  }

  /***************************************************************
   *                Test I/O registers                           *
   ***************************************************************/
  for (j=1  ;  j <  NUMREGS ; j++  ) { 
	/* register loop */

  	/*******************
         *   Reset Adapter
         *******************/
	if (rc = reset_adapter(adapter_info, tucb_ptr, IO_TEST)) {
		return (rc);
	}

	DEBUG_1("Testing I/O reg at addr 0x%08x\n",io_table[j].write_addr);
	DEBUG_0("Initial value: ");
	io_rd_swap(adapter_info, IOSHORT, io_table[j].read_addr, &rd_data);
	for (i = 0; i < 4; i++) { 
		/* pattern loop */
		SUCCESS = FALSE;

		for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
			/* retry loop */

			if (rc = io_wr_swap(adapter_info, IOSHORT, 
						io_table[j].write_addr,
				    		io_table[j].test_val[i])) {
				DEBUG_1("rc = %x\n",rc);	
				return (error_handler(tucb_ptr, IO_TEST,
				           io_table[j].errcode + WRITE_ERR,
				           REGISTER_ERR, io_table[j].write_addr,
					   io_table[j].test_val[i], 0));
			}

			if (rc = io_rd_swap(adapter_info, IOSHORT,
				            io_table[j].read_addr, &rd_data)) {
				return (error_handler(tucb_ptr, IO_TEST,
				    io_table[j].errcode + READ_ERR,
				    REGISTER_ERR, io_table[j].read_addr, 0, 0));
			}

			if (rd_data == io_table[j].exp_val[i]) {
				SUCCESS = TRUE;
			} else if (retries >= 3) {
				return (error_handler(tucb_ptr, IO_TEST,
					    io_table[j].errcode + COMPARE_ERR,
					    REGISTER_ERR,io_table[j].write_addr,
					    io_table[j].exp_val[i], rd_data));
			}

		} /* end retry loop */
	} /* end pattern loop */
  } /* end io register loop */


  /******************************
   *     Test I/O RUM Registers
   ******************************/
  for (j=1  ;  j <  NUM_RUM_REGS ; j++  ) { 
	/* io rum register loop */

	SUCCESS = FALSE;

	DEBUG_1("Testing I/O RUM reg 0x%08x\n",io_rum_table[j].write_addr);
	for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
		/* retry loop */

		/*******************
                 *   Reset Adapter
                 *******************/
		if (rc = reset_adapter(adapter_info, tucb_ptr, IO_TEST)) {
			return (rc);
		}

		/*****************************************
           	 *   Initialize RUM register to all 1's  *
           	 *****************************************/
		if (rc = io_wr(adapter_info,IOSHORT,io_rum_table[j].read_addr,
			    0xffff)) {
			return (error_handler(tucb_ptr, IO_TEST,
				    io_rum_table[j].errcode + WRITE_ERR,
				    REGISTER_ERR, io_rum_table[j].read_addr,
				    0xffff, 0));
		}
		OK = TRUE;

		for (i = 0; i < 4 && OK; i++) { 
			/* pattern loop */

			if (rc = io_wr_swap(adapter_info, IOSHORT,
				    io_rum_table[j].write_addr,
				    io_rum_table[j].test_val[i])) {
				return (error_handler(tucb_ptr, IO_TEST,
				    io_rum_table[j].errcode + WRITE_ERR,
				    REGISTER_ERR, io_rum_table[j].write_addr,
				    io_rum_table[j].test_val[i], 0));
			}
			if (rc = io_rd_swap(adapter_info, IOSHORT,
				    io_rum_table[j].read_addr, &rd_data)) {
				return (error_handler(tucb_ptr, IO_TEST,
				    io_rum_table[j].errcode + READ_ERR,
				    REGISTER_ERR, io_rum_table[j].read_addr,
				    0, 0 ));
			}

			if (rd_data == io_rum_table[j].exp_val[i]) {
				SUCCESS = TRUE;
			} else if (retries >= 3) {
				return (error_handler(tucb_ptr, IO_TEST,
				    io_rum_table[j].errcode + COMPARE_ERR,
				    REGISTER_ERR,io_rum_table[j].write_addr,
				    io_rum_table[j].exp_val[i], rd_data));
			} else {
				SUCCESS = FALSE;
				OK = FALSE;
			}

		} /* end pattern loop */

	} /* end retry loop */
  } /* end io_rum register loop */


  /************************************
   *     Test I/O BUS MASTER Registers
   ************************************/
  for (j=0  ;  j <  NUM_BUSM_REGS ; j=j+2  ) { 
	/* register loop */

	DEBUG_1("Testing I/O BUS reg at addr 0x%08x\n",io_busm_table[j].write_addr);
	/*******************
         *   Reset Adapter
         *******************/
	if (rc = reset_adapter(adapter_info, tucb_ptr, IO_TEST)) {
			return (rc);
	}

	/*********************************
         *  Must write 32 bits at a time.
         *********************************/
	for (i = 0; i < 4; i++) { 
		/* pattern loop */
		SUCCESS = FALSE;

		for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
			/* retry loop */

			for ( k = 0; k < 2; k++) {
				if (rc = io_wr_swap(adapter_info, IOSHORT,
					    io_busm_table[j+k].write_addr,
					    io_busm_table[j+k].test_val[i])) {
				     return (error_handler(tucb_ptr, IO_TEST,
				       io_busm_table[j+k].errcode + WRITE_ERR,
				       REGISTER_ERR,io_busm_table[j].write_addr,
				       io_busm_table[j+k].test_val[i], 0));
				}
			} /* end word loop */

			for (k = 0; k < 2; k++) { 
				/* word loop */
				if (rc = io_rd_swap(adapter_info, IOSHORT,
				      io_busm_table[j+k].read_addr, &rd_data)) {
				    return (error_handler(tucb_ptr, IO_TEST,
				       io_busm_table[j+k].errcode + READ_ERR,
				       REGISTER_ERR, io_busm_table[j].read_addr,
				       0, 0));
				}

				if (rd_data == io_busm_table[j+k].exp_val[i]) {
				     SUCCESS = TRUE;
				} else if (retries >= 3) {
				     return (error_handler(tucb_ptr, IO_TEST,
				       io_busm_table[j+k].errcode + COMPARE_ERR,
				       REGISTER_ERR,io_busm_table[j].write_addr,
				       io_busm_table[j+k].exp_val[i], rd_data));
				}
			} /* end word loop */

		} /* end retry loop */
	} /* end pattern loop */

  } /* end io register loop */

  return (0);

} /* end io_test() */


