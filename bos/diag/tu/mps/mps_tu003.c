static char sccsid[] = "@(#)30  1.2  src/bos/diag/tu/mps/mps_tu003.c, tu_mps, bos411, 9437B411a 8/28/94 12:35:26";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS:   int tu003()
 *              int oncard_test()
 *              int get_srb_response()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
#include "mpstu_type.h"
#include "mps_regs.h"


/**************************************************************************** *
*
* FUNCTION NAME =  tu003()
*
* DESCRIPTION   =  This function initiates the oncard test for MPS
*
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
int tu003 (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  int rc = 0; /* return code */

  rc = oncard_test(adapter_info, tucb_ptr);

  return (rc);
}

/**************************************************************************** *
*
* FUNCTION NAME =  oncard_test
*
* DESCRIPTION   =  This function kicks off the MPC on-card diagnostics
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = tu003
*
*****************************************************************************/
int oncard_test (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  int         rc = 0,
  rcx = 0;

  /*****************************
   *   Set Up the MPS Adapter  *
   *****************************/
  if (rc = set_up_adapter(adapter_info, tucb_ptr, ONCARD_TEST)) {
	return (rc);
  }

  /***************************
   *  Initialize the Adapter
   ***************************/
  if (rc = initialize_adapter(adapter_info, tucb_ptr, CONNECT_TEST)) {
	if (tucb_ptr->header.adap_flag == TRUE) {
		mps_clean_up(adapter_info);
		tucb_ptr->header.adap_flag = FALSE;
	}
	return(rc);
  }

  /*******************************************************
   *  Clean up any resources allocated for initialization.
   *******************************************************/
  rcx = mps_clean_up(adapter_info);
  tucb_ptr->header.adap_flag = FALSE;  /* Set flag for HTX signal handler */

  /**************************************************
   * Determine which, if any, return code to return.
   **************************************************/
  if (rcx) {
	return (error_handler(tucb_ptr, ONCARD_TEST, rcx, CLEANUP_ERR, 0,0,0));
  } else {
	return(0);
  }

} /* end oncard_test */

