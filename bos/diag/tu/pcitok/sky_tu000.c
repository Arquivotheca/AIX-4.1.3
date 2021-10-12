static char sccsid[] = "@(#)16	1.4  src/bos/diag/tu/pcitok/sky_tu000.c, tu_pcitok, bos41J, 9519B_all 5/12/95 11:12:55";
/***************************************************************************
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: tu000
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 ****************************************************************************/

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
* FUNCTION NAME =  tu000()
*
* DESCRIPTION   =  This function is the initialization test unit.
*
* INPUT   =    adapter_name   - ptr to a adapter name 
*              tucb_ptr       - ptr to test unit control block
* 	       adapter_info   - ptr to a struct containing diagex info
*
* RETURN-NORMAL =  0
*
* RETURN-ERROR =   non-zero
*
* INVOKED FROM = exectu
*
*****************************************************************************/
int tu000 (char *adapter_name,  TUTYPE *tucb_ptr, ADAPTER_STRUCT *adapter_info)
{
  int rc = 0; /* return code */
  char filename[30] = "/tmp/skymsg";

  /**************************
   * Initialize Message File
   **************************/
  filename[11] = adapter_name[8];
#ifdef DEBUG
  freopen(filename, "w",stdout);
#endif
  DEBUG_1("Opening file %s\n",filename);

  /*****************************
   * Initialize Error Structure
   *****************************/
  tucb_ptr->errinfo = (error_details *) malloc(sizeof(error_details));
  if (tucb_ptr->errinfo == NULL)
  {
	return (error_handler(tucb_ptr, TU_OPEN, MEMORY_ERR, MEMORY_ERR,
		0, 0, 0));
  }
  memset(tucb_ptr->errinfo, 0, sizeof(error_details));

  /***************
   *  Open adapter
   ****************/
  rc = adapter_open(tucb_ptr, adapter_info, adapter_name);
  if (rc) {
  	return (error_handler(tucb_ptr, TU_OPEN, OPEN_ERR, OPEN_ERR, 0, 0, 0));
  }

  DEBUG_1("Adapter card id = %x \n", adapter_info->card_id);

  return (rc);
}

