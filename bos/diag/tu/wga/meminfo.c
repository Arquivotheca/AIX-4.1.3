static char sccsid[] = "@(#)73  1.2  src/bos/diag/tu/wga/meminfo.c, tu_wga, bos411, 9428A410j 4/23/93 15:29:53";
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: set_mem_info
 *              update_mem_info
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <stdio.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>

#include "exectu.h"
#include "tu_type.h"
#include "wgamisc.h"
#include "wga_reg.h"
#include "vpd.h"




/* Next struct. used to return memory info when memory tests fail */

static MEM mt = { 0, 0, 0 };




/*
 * NAME : set_mem_info
 *
 * DESCRIPTION :
 *
 *   Fills in memory test information. This function
 *   is called when a memory test fails.
 *
 * INPUT :
 *
 *   1. Memory address at which test failed (x coord.).
 *   2. Data written.
 *   3. Data read.
 *   4. Return code associated with failure.
 *   5. Description of test method for writing to VRAM
 *   6. Description of test method for reading from VRAM
 *
 * OUTPUT :
 *
 *  Memory test information variable mt updated.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void set_mem_info (ulong_t x_addr, ulong_t write, ulong_t read, int rc,
                   char *write_method, char *read_method)
{
#ifdef LOGMSGS
  char msg[255];
#endif

  TITLE("set_mem_info");

  mt.addr    = x_addr;
  mt.write   = write;
  mt.read    = read;

#ifdef LOGMSGS
  sprintf(msg, "addr. = 0x%08x  %s  %s  wrote : 0x%08x  read : 0x%08x  (%s)",
          mt.addr, write_method, read_method, mt.write, mt.read,
          get_err_desc(rc));
  LOG_ERROR(msg);
#endif


  return;
}



/*
 * NAME : update_mem_info
 *
 * DESCRIPTION :
 *
 *   Retrieves memory test information from mt variable.
 *   Should be called after every memory TU to get
 *   memory failure information.
 *
 * INPUT :
 *
 *   1. Pointer to MEM structure where information
 *      will be stored.
 *
 * OUTPUT :
 *
 *  Memory test information.
 *
 * RETURNS :
 *
 *   None.
 *
*/

void update_mem_info (MEM *info)
{
  TITLE("update_mem_info");

  info->addr    = mt.addr;
  info->write   = mt.write;
  info->read    = mt.read;

  return;
}
