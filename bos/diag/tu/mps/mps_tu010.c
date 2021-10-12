static char sccsid[] = "@(#)35  1.3  src/bos/diag/tu/mps/mps_tu010.c, tu_mps, bos411, 9440E411e 10/11/94 16:57:13";
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS:   int tu010()
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

/****************************************************************************
*
* FUNCTION NAME =  tu010()
*
* DESCRIPTION   =  This function is the termination test unit.
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
int tu010 (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr) {
  int    rc = 0;                /* return code */
  struct cfg_load *cfg_ld;
  char	ddpath[512];

  /*******************
  *   Reset Adapter
  *******************/
  reset_adapter(adapter_info, tucb_ptr, TU_CLOSE);


  /**********************
   *  Close the Adapter
   **********************/
  rc = adapter_close(adapter_info);

  fclose(tucb_ptr->msg_file);
  if (rc) {
    return (error_handler(tucb_ptr, TU_CLOSE, CLOSE_ERR, CLOSE_ERR, 0, 0, 0));
  }

  /************************************
   *  Unload the Interrupt Handler 
   ************************************/

  cfg_ld = ( struct cfg_load *) malloc (sizeof (struct cfg_load));
#ifdef DIAGPATH
  sprintf(ddpath, "%s/%s", (char *)getenv("DIAGX_SLIH_DIR"), "tok32_intr");
  cfg_ld->path = ddpath;
#else
  cfg_ld->path=INTERRUPT_HANDLER_PATH;
#endif

  DEBUG_1("Interrupt Handler -> %s\n", cfg_ld->path);

  errno = 0;
  if (rc=sysconfig(SYS_QUERYLOAD, cfg_ld, (int)sizeof(cfg_ld))) {
        DEBUG_0("Error querying interrupt handler.")
        free(cfg_ld);
        return(rc);
  }

  if (cfg_ld->kmid) {
  	if (sysconfig(SYS_KULOAD, cfg_ld, (int)sizeof(cfg_ld))) {
        	DEBUG_0("sysconfig(SYS_KULOAD)");
        	DEBUG_0("ucfgdevice: Unloaded interrupt handler\n")
        	return(E_UNLOADEXT);
  	}
  }
  free(cfg_ld);

  return (rc);

  }

