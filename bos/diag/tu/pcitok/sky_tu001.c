static char sccsid[] = "@(#)17	1.1  src/htx/usr/lpp/htx/lib/pcitok/sky_tu001.c, tu_pcitok, htx410, htx410_222 3/21/95 17:01:21";
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: cfg_chk
 *		cfg_chk_byte
 *		cfg_chk_range
 *		cfg_restore
 *		cfg_restore_range
 *		cfg_save
 *		cfg_save_range
 *		tu001
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

/****************************************************************************
*
* FUNCTION NAME =  tu001()
*
* DESCRIPTION   =  This function calls the necessary routines to perform
*                  the Configuration Register Test.
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

  rc = cfg_chk(adapter_info, tucb_ptr);

  return (rc);
}

/*****************************************************************************
*
* FUNCTION NAME = cfg_chk_byte
*
* DISCRIPTION   = This function checks a configuration register to make sure
*                 that all the bits specified by the writ_mask are writable. 
*
* INPUT         = address        - address of the byte to test.
*                 write_mask     - bits of the byte that are to be tested.
*                 save_area      - original register values.
*                 adapter_info   - ptr to a struct containing diagex info
*                 tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL = 0
*
* RETURN-ERROR  = non-zero
*
* INVOKED FROM  = cfg_chk
*
*****************************************************************************/
int cfg_chk_byte (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr,
                  ulong address, uchar write_mask, uchar save_area[])
{
  int i, retries, rc = 0;
  static uchar tval[] = { 0xaa, 0x55, 0xff, 0x33 	};
  uchar test_val, rd_data;
  BOOLEAN        SUCCESS;

    for (i = 0; i < 4; i++) {
      test_val = (tval[i] & write_mask) | (save_area[address] & (~write_mask));

           /* The bits represented in the mask are to be tested, the others
              are hardwired to a certain value, so the value read from those
              bit are written back to them.
           */ 

	SUCCESS = FALSE;

	for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
		/* retry loop */
		if (rc = config_reg_write(address, test_val,adapter_info->dds)) {
			return (error_handler(tucb_ptr, CFG_TEST,
			        CFG_ERR + WRITE_ERR,
			        REGISTER_ERR, address, test_val, 0));
		}

		if (rc = config_reg_read(address, &rd_data, adapter_info->dds)) {
			return (error_handler(tucb_ptr, CFG_TEST,
				    CFG_ERR + READ_ERR,
				    REGISTER_ERR, address, test_val, 0));
		}

		if (rd_data == test_val) {
			SUCCESS = TRUE;
		} else if (retries >= 3) {
			return (error_handler(tucb_ptr, CFG_TEST,
				    CFG_ERR + COMPARE_ERR,
				    REGISTER_ERR, address,
				    test_val, rd_data));
		}
	} /* end retry loop */
 
    } /* end test value loop */ 
  return(rc);
} /* end cfg_chk_byte */






/*****************************************************************************
*
* FUNCTION NAME = cfg_chk_range
*
* DISCRIPTION   = This function loops through a range of the configuration
*                 space to test the registers.
*
* INPUT         = start_address  - address of the first register.
*                 finish_address - address of the last registes.
*                 adapter_info   - ptr to a struct containing diagex info
*                 tucb_ptr       - ptr to test unit control block
*
* RETURN-NORMAL = 0
*
* RETURN-ERROR  = non-zero
*
* INVOKED FROM  = cfg_chk
*
*****************************************************************************/
int cfg_chk_range (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr,
                  ulong start_address, ulong finish_address)
{
  int i, j, retries, rc = 0;
  ulong cfg_addr; 
  static uchar tval[] = { 0xaa, 0x55, 0xff, 0x33 	};
  uchar test_val, rd_data;
  BOOLEAN        SUCCESS;

  for (j=0, cfg_addr=start_address; cfg_addr<=finish_address; j++,cfg_addr++){
    for (i = 0; i < 4; i++) {
      test_val = tval[i];
 
	SUCCESS = FALSE;

	for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
		/* retry loop */
		if (rc = config_reg_write(cfg_addr, test_val,adapter_info->dds)) {
			return (error_handler(tucb_ptr, CFG_TEST,
			        CFG_ERR + WRITE_ERR,
			        REGISTER_ERR, cfg_addr, test_val, 0));
		}

		if (rc = config_reg_read(cfg_addr, &rd_data, adapter_info->dds)) {
			return (error_handler(tucb_ptr, CFG_TEST,
				    CFG_ERR + READ_ERR,
				    REGISTER_ERR, cfg_addr, test_val, 0));
		}

		if (rd_data == test_val) {
			SUCCESS = TRUE;
		} else if (retries >= 3) {
			return (error_handler(tucb_ptr, CFG_TEST,
				    CFG_ERR + COMPARE_ERR,
				    REGISTER_ERR, cfg_addr,
				    test_val, rd_data));
		}
	} /* end retry loop */
 
    } /* end test value loop */ 
  } /* end configuration space loop */
  return(rc);
} /* end cfg_chk_range */







/*****************************************************************************
*
* FUNCTION NAME = cfg_save_range
*
* DISCRIPTION   = This function loops through a range of the configuration
*                 space to save the registers.
*
* INPUT         = start_address  - address of the first register.
*                 finish_address - address of the last registes.
*                 adapter_info   - ptr to a struct containing diagex info
*                 tucb_ptr       - ptr to test unit control block
*                 save_area      - area for saving cfg registers
*
* RETURN-NORMAL = 0
*
* RETURN-ERROR  = non-zero
*
* INVOKED FROM  = cfg_save
*
*****************************************************************************/
int cfg_save_range (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr,
                  uchar start_addr, uchar finish_addr,uchar save_area[])
{
  int rc;
  int i;
  
  DEBUG_2("Save range, start = 0x%0x, finish = 0x%0x\n",start_addr,finish_addr);
  for(i = 0; start_addr <= finish_addr; i++)
  {
  	if (rc = config_reg_read((int)start_addr,&save_area[start_addr],
                   adapter_info->dds)) {
    	error_handler(tucb_ptr, CFG_TEST, CFG_ERR + READ_ERR, 
                       REGISTER_ERR, start_addr, 0 , 0);
  	}
	(int)start_addr++;
  }
  return(rc);

} /* end cfg_save_range */

/*****************************************************************************
*
* FUNCTION NAME = cfg_restore_range
*
* DISCRIPTION   = This function loops through a range of the configuration
*                 space to restore the registers.
*
* INPUT         = start_address  - address of the first register.
*                 finish_address - address of the last registes.
*                 adapter_info   - ptr to a struct containing diagex info
*                 tucb_ptr       - ptr to test unit control block
*                 save_area      - area where cfg registers are stored
*
* RETURN-NORMAL = 0
*
* RETURN-ERROR  = non-zero
*
* INVOKED FROM  = cfg_restore
*
*****************************************************************************/
int cfg_restore_range (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr,
                  ulong start_addr, ulong finish_addr,uchar save_area[])
{
  int rc = 0;
  int i;

  DEBUG_2("Restore range, start = 0x%0x, finish = 0x%0x\n",start_addr,finish_addr);
  for (i = 0; start_addr <= finish_addr; i++)
  {
  	if ( rc = config_reg_write(start_addr,save_area[start_addr],
                   adapter_info->dds)) {
    	error_handler(tucb_ptr, CFG_TEST, CFG_ERR + WRITE_ERR, 
                       REGISTER_ERR, start_addr, 0, 0);
  	}
	start_addr++;
  }
  return(rc);

} /* end cfg_restore_range */







/*****************************************************************************
*
* FUNCTION NAME =  cfg_save
*
* DESCRIPTION   =  This function saves cfg registers in the given
*                  'save area'.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              save_area      - area for saving cfg registers
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = cfg_chk
*
*****************************************************************************/
int cfg_save (ADAPTER_STRUCT *adapter_info, uchar save_area[], TUTYPE *tucb_ptr)
{
  int    rc = 0;

  DEBUG_0("Saving initial config value\n");

  /* PCI command register */
  if (rc = cfg_save_range(adapter_info, tucb_ptr,PCR, PCR+1,save_area)) {
	return (rc);
  }

  /* byte 1 of the PCI Status Register */
/*  if (rc = cfg_save_range(adapter_info, tucb_ptr, PSR+1 , PSR+1, save_area)) {
	return (rc);
  }
*/
  /* CLS and LTR registers */
  if (rc = cfg_save_range(adapter_info, tucb_ptr, CLS , LTR, save_area)) {
	return (rc);
  }

  /* bytes 1,2, and 3 of BA0 */
  if (rc = cfg_save_range(adapter_info, tucb_ptr, BA0+1 , BA0+3, save_area)) {
	return (rc);
  }
  
  /* bytes 1,2, and 3 of BA1 */
  if (rc = cfg_save_range(adapter_info, tucb_ptr, BA1+1 , BA1+3, save_area)) {
	return (rc);
  }
  
  /* bytes 1,2, and 3 of BAR */
  if (rc = cfg_save_range(adapter_info, tucb_ptr, BAR+1 , BAR+3, save_area)) {
	return (rc);
  }

  /* ILR register */
  if (rc = cfg_save_range(adapter_info, tucb_ptr,ILR, ILR, save_area)) {
	return (rc);
  }

  /* GPR register */
  if (rc = cfg_save_range(adapter_info, tucb_ptr, GPR, GPR, save_area)) {
	return (rc);
  }

  /* the rest of the configuration space after the header */
  /*if (rc = cfg_save_range(adapter_info, tucb_ptr, 0x3F , 0xFF,save_area)) {
	return (rc);
  }
  */ 
  return (rc);
} /* end cfg_save */







/*****************************************************************************
*
* FUNCTION NAME =  cfg_restore
*
* DESCRIPTION   =  This function restores cfg registers from the given
*                  'save area'.
*
* INPUT   =    adapter_info   - ptr to a struct containing diagex info
*              save_area      - area for saving cfg registers
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   error code
*
* INVOKED FROM = cfg_chk
*
*****************************************************************************/
int cfg_restore (ADAPTER_STRUCT *adapter_info, uchar save_area[],
TUTYPE *tucb_ptr)
{
  uint addr;
  int j, rc = 0;

  /* PCI command register */
  if (rc = cfg_restore_range(adapter_info, tucb_ptr,PCR, PCR+1, save_area)) {
	return (rc);
  }

  /* byte 1 of the PCI Status Register */
/*  if (rc = cfg_restore_range(adapter_info, tucb_ptr, PSR+1, PSR+1, save_area)) {
	return (rc);
  }
*/

  /* CLS and LTR registers */
  if (rc = cfg_restore_range(adapter_info, tucb_ptr, CLS , LTR, save_area)) {
	return (rc);
  }

  /* bytes 1,2, and 3 of BA0 */
  if (rc = cfg_restore_range(adapter_info,tucb_ptr,BA0+1,BA0+3, save_area)) {
	return (rc);
  }
  
  /* bytes 1,2, and 3 of BA1 */
  if (rc = cfg_restore_range(adapter_info,tucb_ptr,BA1+1,BA1+3, save_area)) {
	return (rc);
  }
  
  /* bytes 1,2, and 3 of BAR */
  if (rc = cfg_restore_range(adapter_info,tucb_ptr,BAR+1,BAR+3, save_area)) {
	return (rc);
  }

  /* ILR register */
  if (rc = cfg_restore_range(adapter_info,tucb_ptr,ILR,ILR,save_area)) {
	return (rc);
  }

  /* GPR register */
  if (rc = cfg_restore_range(adapter_info, tucb_ptr, GPR, GPR, save_area)) {
	return (rc);
  }

  /* the rest of the configuration space after the header */
  /*if (rc = cfg_restore_range(adapter_info,tucb_ptr,0x3F,0xFF,save_area)) {
	return (rc);
  }
  */ 

  return (rc);
} /* end cfg_restore */

/****************************************************************************
*
* FUNCTION NAME =  cfg_chk()
*
* DESCRIPTION   =  This function test writes/reads/compares byte values to
*                  check all bit positions in writable configuration
*                  registers.
*		   The config registers PCR and BA0 can not be tested
*		   These registers must be enabled in case we get an 
*		   interrupt.
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
int cfg_chk (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
{
  ulong   cfg_addr;
  int     i, j, retries, rc = 0;
  uchar   save_bconfig,rd_data, test_val, cfg_save_area[255];
  BOOLEAN        SUCCESS;

 /*******************
 *   Reset Adapter
 *******************/
  if (rc = reset_adapter(adapter_info, tucb_ptr, CFG_TEST)) {
	return (rc);
  }
  /****************************************
   * Save the cfg registers first before
   * doing any writing.
   ****************************************/
  if (rc = cfg_save(adapter_info, cfg_save_area, tucb_ptr)) {
		return (rc);
  }


  /***********************************************************
   * make sure  (non-writable) have legal values.
   ***********************************************************/

  /********************/
  /* Check Vendor ID */
  /******************/

  SUCCESS = FALSE;
  for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
	/* retry loop */
	if (rc = config_reg_read(VID_REG, &rd_data, adapter_info->dds)) {
                 return (error_handler(tucb_ptr, CFG_TEST, VID_ERR + READ_ERR,
			    REGISTER_ERR, VID_REG, 0, 0));
	}

	if (rd_data == VID_VALUE_LO) {
		SUCCESS = TRUE;
	} else if (retries >= 3) {
		return (error_handler(tucb_ptr, CFG_TEST, VID_ERR+COMPARE_ERR,
			    REGISTER_ERR, VID_REG, VID_VALUE_LO, rd_data));
	}
  } /* end retry loop */
  
  SUCCESS = FALSE;
  for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
	/* retry loop */
	if (rc = config_reg_read(VID_REG + 1, &rd_data, adapter_info->dds)) {
                 return (error_handler(tucb_ptr, CFG_TEST, VID_ERR + READ_ERR,
			    REGISTER_ERR, VID_REG + 1, 0, 0));
	}

	if (rd_data == VID_VALUE_HI) {
		SUCCESS = TRUE;
	} else if (retries >= 3) {
		return (error_handler(tucb_ptr, CFG_TEST, VID_ERR+COMPARE_ERR,
			    REGISTER_ERR, VID_REG + 1, VID_VALUE_HI, rd_data));
	}
  } /* end retry loop */


  /***********************/
  /*** Check device ID ***/
  /***********************/

  SUCCESS = FALSE;
  for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
	/* retry loop */
	if (rc = config_reg_read(DID_REG, &rd_data, adapter_info->dds)) {
                 return (error_handler(tucb_ptr, CFG_TEST, DID_ERR + READ_ERR,
			    REGISTER_ERR, DID_REG, 0, 0));
	}

	if (rd_data == DID_VALUE_LO) {
		SUCCESS = TRUE;
	} else if (retries >= 3) {
		return (error_handler(tucb_ptr, CFG_TEST, DID_ERR+COMPARE_ERR,
			    REGISTER_ERR, DID_REG, DID_VALUE_LO, rd_data));
	}
  } /* end retry loop */
  
  SUCCESS = FALSE;
  for (retries = 0; !SUCCESS && (retries < 4); retries++) { 
	/* retry loop */
	if (rc = config_reg_read(DID_REG + 1, &rd_data, adapter_info->dds)) {
                 return (error_handler(tucb_ptr, CFG_TEST, DID_ERR + READ_ERR,
			    REGISTER_ERR, DID_REG + 1, 0, 0));
	}

	if (rd_data == DID_VALUE_HI) {
		SUCCESS = TRUE;
	} else if (retries >= 3) {
		return (error_handler(tucb_ptr, CFG_TEST, DID_ERR+COMPARE_ERR,
			    REGISTER_ERR, DID_REG + 1, DID_VALUE_HI, rd_data));
	}
  } /* end retry loop */



  /***************************************************************
   * try to write 0xAA, 0x55, 0xFF, 0x33 in cfg regs.
   ***************************************************************/

  /* byte 1 of the PSR Status Register */
  /*
  DEBUG_0("Testing PSR hi reg\n");
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, PSR + 1, PSR_WRITE_MASK_HI,
      cfg_save_area)) {
	return (rc);
  }
  */

  DEBUG_0("Testing CLS reg\n");
  /* CLS register */ 
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, CLS , CLS_WRITE_MASK,
      cfg_save_area)) {
  	return (rc);
  }

  DEBUG_0("Testing LTR reg\n");
  /* LTR register */
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, LTR , LTR_WRITE_MASK,
      cfg_save_area)) {
  	return (rc);
  }
  
  /* bytes 1,2, and 3 of BA1 */
  DEBUG_0("Testing BA1 (1,2,3) reg\n");
  if (rc = cfg_chk_range(adapter_info, tucb_ptr, BA1 + 1 , BA1 + 3)) {
	return (rc);
  }
  
  /* bytes 1,2, and 3 of BAR */
  DEBUG_0("Testing BAR (1) reg\n");
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, BAR + 1 , BAR1_WRITE_MASK,
      cfg_save_area)) {
	return (rc);
  }
  DEBUG_0("Testing BAR (1,2,3) reg\n");
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, BAR + 2 , WRITE_ALL_BITS_MASK,
      cfg_save_area)) {
	return (rc);
  }
  DEBUG_0("Testing BAR (1,2,3) reg\n");
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, BAR + 3 , WRITE_ALL_BITS_MASK,
      cfg_save_area)) {
	return (rc);
  }

  /* ILR register */
  DEBUG_0("Testing ILR reg\n");
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, ILR, ILR_WRITE_MASK,
      cfg_save_area)) {
	return (rc);
  }
 
  /* GPR register */
  DEBUG_0("Testing GPR reg\n");
  if (rc = cfg_chk_byte(adapter_info, tucb_ptr, GPR, GPR_WRITE_MASK,
      cfg_save_area)) {
	return (rc);
  }

  /* the rest of the configuration space after the header */
 /* if (rc = cfg_chk_range(adapter_info, tucb_ptr, 0x3F , 0xFF)) {
	return (rc);
  }
 */  

  /***************** cfg register test end ****************/

  /****************************************************
   * prior to returning, restore original cfg reg vals.
   ****************************************************/

  DEBUG_0("Restoring original cfg reg values\n");
  if (rc = cfg_restore(adapter_info, cfg_save_area, tucb_ptr)) {
	return (rc);
  }
 
  DEBUG_1("Leaving cfg_chk rc = %x\n",rc);
  return (rc);
} /* end cfg_chk */



