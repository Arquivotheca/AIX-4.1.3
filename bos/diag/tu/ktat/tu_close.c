static char sccsid[] = "@(#)96	1.4  src/bos/diag/tu/ktat/tu_close.c, tu_ktat, bos41J, 9523A_all 5/31/95 11:10:54";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu_close
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
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/dma.h>
#include <odmi.h>
#include <cf.h>
#include <sys/diagex.h>
#include <sys/access.h>
#include <stdio.h>
#include "kent_defs.h"
#include "kent_tu_type.h"
/* #include "extern.h" */

extern diagex_dds_t		dds;
extern diag_struc_t		*diagex_hdl;
extern struct cfg_load		cfg_ld;
extern busstring[];

/***********************************************************
 * NAME: tu_close
 *
 * FUNCTION: shutdown for Klickitat TUs.
 *
 * INPUT:
 *
 * OUTPUT: Error Code
 *
 ***********************************************************/

extern int open_cnt;
extern int mdd_fd;

/* #define TU_SYS 1  */ 

int tu_close(char *devid, TU_TYPE *tucb_ptr)
{
  int	rc;
  int exit_code = 0;
/*  printf("open_cnt = %d\n", open_cnt);  */

  if(open_cnt == 0)
  {
/*    printf("returning because open_cnt = 0\n"); */
    return(exit_code);
  }
  
  fclose(tucb_ptr->msg_file);

  if (rc = diag_close(diagex_hdl))
  {
/*    printf("diag_close failed rc = 0x%08x\n", rc);  */
    exit_code = TU_SYS;
  }

  if (rc = close(mdd_fd))
  {
/*  printf("mdd close failed, rc = 0x%08x\n", rc);  */
     exit_code = TU_SYS;
  }

#ifdef HAVESLIH
  if (sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld)) &&
      exit_code == 0)
  {
/*    printf("SYS_KULOAD failed\n");  */
    exit_code = TU_SYS;
  }
#endif

  if (rc = diagex_initial_state(devid)) /* && exit_code == 0) */
  {
/*    printf("diagex_initial_state failed, rc = 0x%08x\n", rc);  */
     exit_code = TU_SYS;
  }
   open_cnt--;
   return(exit_code);
}
