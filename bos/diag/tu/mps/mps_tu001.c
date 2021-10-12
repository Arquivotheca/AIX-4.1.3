static char sccsid[] = "@(#)28  1.1  src/bos/diag/tu/mps/mps_tu001.c, tu_mps, bos411, 9437B411a 8/23/94 16:25:14";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS:   int tu001()
 *              int pos_chk()
 *              int pos_save()
 *              int pos_restore()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
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

/****************************************************************************
*
* FUNCTION NAME =  tu001()
*
* DESCRIPTION   =  This function calls the necessary routines to perform
*                  the POS Register Test.
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
int tu001 (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  int rc = 0; /* return code */

  rc = pos_chk(adapter_info, tucb_ptr);

  return (rc);
}


/****************************************************************************
*
* FUNCTION NAME =  pos_chk()
*
* DESCRIPTION   =  This function test writes/reads/compares byte values to
*                  check all bit positions in writable POS registers (2-5).
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = tu001
*
*****************************************************************************/
int pos_chk (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  ulong   pos_addr;
  int     i, j, retries, rc = 0;
  uchar   rd_data, test_val, pos_save_area[8];
  static uchar tval[] = { 0xaa, 0x55, 0xff, 0x33 	};
  BOOLEAN        SUCCESS;

  /****************************************
   * Save the POS registers first before
   * doing any writing.
   ****************************************/
  if (rc = pos_save(adapter_info, pos_save_area, tucb_ptr)) {
		return (rc);
  }

  /**********************
   * Disable chip enable
   **********************/
  if (rc = pos_rd(adapter_info, POS2, &rd_data)) {
	return (error_handler(tucb_ptr, POS_TEST, POS2_ERR + READ_ERR,
		    REGISTER_ERR, POS2, 0, 0));
  }

  rd_data &= 0xfe;
  if (rc = pos_wr(adapter_info, POS2, rd_data)) {
	return (error_handler(tucb_ptr, POS_TEST, POS2_ERR + WRITE_ERR,
		    REGISTER_ERR, POS2, rd_data, 0));
  }

  /***********************************************************
   * make sure POS0 and POS1 (non-writable) have legal values.
   ***********************************************************/

  /* Check POS0 */
  SUCCESS = FALSE;

  for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
	/* retry loop */
	if (rc = pos_rd(adapter_info, POS0, &rd_data)) {
		return (error_handler(tucb_ptr, POS_TEST, POS0_ERR + READ_ERR,
			    REGISTER_ERR, POS0, 0, 0));
	}

	if (rd_data == INTPOS0) {
		SUCCESS = TRUE;
	} else if (retries >= 3) {
		return (error_handler(tucb_ptr, POS_TEST, POS0_ERR+COMPARE_ERR,
			    REGISTER_ERR, POS0, INTPOS0, rd_data));
	}
  } /* end retry loop */

  /*** Check POS1 ***/
  SUCCESS = FALSE;

  for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
	/* retry loop */
	if (rc = pos_rd(adapter_info, POS1, &rd_data)) {
		return (error_handler(tucb_ptr, POS_TEST, POS1_ERR + READ_ERR,
			    REGISTER_ERR, POS1, 0, 0));
	}

	if (rd_data == INTPOS1) {
		SUCCESS = TRUE;
	} else if (retries >= 3) {
		return (error_handler(tucb_ptr, POS_TEST, POS1_ERR+COMPARE_ERR,
			    REGISTER_ERR, POS1, INTPOS1, rd_data));
	}
  } /* end retry loop */

  /***************************************************************
   * try to write 0xAA, 0x55, 0xFF, 0x33 in pos regs. 2 through 5.
   ***************************************************************/

  /********       Test POS Registers 2 - 5             ******************/

  for (j=0, pos_addr = POS2; pos_addr < POS6; j++, pos_addr++) { 
	/* register loop */
	for (i = 0; i < 4; i++) { 
		/* pattern loop */

		switch(pos_addr) {
			/*************************************
                         * mask out card enable to insure that
                         * card remains disabled.
                         *************************************/
			case POS2:   
				test_val = tval[i] & 0xfe;
				break;

			/*******************************************
                         * Ensure high order bits are always set to 1
                         ******************************************/
			case POS5:   
				test_val = tval[i] | 0xc0;
				break;

			default:     
				test_val = tval[i];
				break;

		}; /* end of case */

		SUCCESS = FALSE;

		for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
			/* retry loop */
			if (rc = pos_wr(adapter_info, pos_addr, test_val)) {
				return (error_handler(tucb_ptr, POS_TEST,
				        POS2_ERR + j + WRITE_ERR,
				        REGISTER_ERR, pos_addr, test_val, 0));
			}

			if (rc = pos_rd(adapter_info, pos_addr, &rd_data)) {
				return (error_handler(tucb_ptr, POS_TEST,
					    POS2_ERR + j + READ_ERR,
					    REGISTER_ERR, pos_addr, 0, 0));
			}

			if (rd_data == test_val) {
				SUCCESS = TRUE;
			} else if (retries >= 3) {
				return (error_handler(tucb_ptr, POS_TEST,
					    POS2_ERR + j + COMPARE_ERR,
					    REGISTER_ERR, pos_addr,
					    test_val, rd_data));
			}
		} /* end retry loop */

	} /* end pattern loop */
  } /* end pos register loop */

  /***************** POS register test end ****************/

  /****************************************************
   * prior to returning, restore original POS reg vals.
   ****************************************************/

  if (rc = pos_restore(adapter_info, pos_save_area, tucb_ptr)) {
	return (rc);
  }

  return (0);
} /* end pos_chk() */


/*****************************************************************************
*
* FUNCTION NAME =  pos_save
*
* DESCRIPTION   =  This function saves POS registers 2 - 5 in the given
*                  'save area'.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              save_area      - area for saving pos registers
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = pos_chk
*
*****************************************************************************/
int pos_save (ADAPTER_STRUCT *adapter_info, uchar save_area[], TUTYPE *tucb_ptr)
{
  int    j, rc = 0;
  ulong  addr;
  uchar  rd_data;


  for (j=0, addr = POS2; addr < POS6; addr++, j++) {
	if ( rc = pos_rd(adapter_info, addr, &rd_data)) {
		rc = error_handler(tucb_ptr, POS_TEST, POS2_ERR + j + READ_ERR,
				    REGISTER_ERR, POS2 + j, 0, 0);
		break;
	} else {
		save_area[j] = rd_data;
		rc = 0;
	}
  } /* end for loop */

  return (rc);
} /* end pos_save */

/*****************************************************************************
*
* FUNCTION NAME =  pos_restore
*
* DESCRIPTION   =  This function restores POS registers 2 - 5 from the given
*                  'save area'.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              save_area      - area for saving pos registers
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = pos_chk
*
*****************************************************************************/
int pos_restore (ADAPTER_STRUCT *adapter_info, uchar save_area[],
TUTYPE tucb_ptr)
{
  uint addr;
  int j, rc = 0;

  /******************************************
   * first, we restore POS registers 3 to 5
   ******************************************/

  for (j=1, addr = POS3 ; addr < POS6 ; addr++, j++) {
  	if (rc = pos_wr(adapter_info, addr, save_area[j])) {
		rc = error_handler(tucb_ptr, POS_TEST, POS3_ERR + j + WRITE_ERR,
			    REGISTER_ERR, addr, save_area[j], 0);
		break;
	}
  }

  /**************************************************
   * restore 2 last since it has the card enable bit
   **************************************************/

  if (!rc) {
	if (rc = pos_wr(adapter_info, POS2, save_area[0])) {
		rc = error_handler(tucb_ptr, POS_TEST, POS2_ERR + WRITE_ERR,
			    REGISTER_ERR, addr, save_area[0], 0);
	}
  }

  return (rc);
} /* end pos_restore */


