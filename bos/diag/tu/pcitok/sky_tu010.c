static char sccsid[] = "@(#)15	1.2  src/bos/diag/tu/pcitok/sky_tools.c, tu_pcitok, bos41J 3/30/95 14:04:33";
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: tu010
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
int tu010 (ADAPTER_STRUCT *adapter_info, TUTYPE *tucb_ptr)
  {
   int    rc = 0;                /* return code */
   struct cfg_load  *cfg_ld;
   char ipath[512];
   int i;

   /*******************
   *   Reset Adapter
   *******************/
   reset_adapter(adapter_info, tucb_ptr, TU_CLOSE);

   /**********************
    *  Close the Adapter
    **********************/
   rc = adapter_close(adapter_info);

#ifdef MPS_DEBUG
   fclose(tucb_ptr->msg_file);
#endif

   if (rc)
     return (error_handler(tucb_ptr, TU_CLOSE, CLOSE_ERR, CLOSE_ERR, 0, 0, 0));

   /************************************
    *  Unload the Interrupt Handler 
    ************************************/
 
   cfg_ld = ( struct cfg_load *) malloc (sizeof (struct cfg_load));
   if (cfg_ld == NULL)
   {
	return(MEMORY_ERR);
   }
#ifdef DIAGPATH
  sprintf(ipath,"%s/%s",(char *)getenv("DIAGX_SLIH_DIR"),"sky_intr");
  cfg_ld->path = ipath;
#else
  cfg_ld->path = INTERRUPT_HANDLER_PATH;
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
   free(tucb_ptr->errinfo);

   return (rc);

  }

