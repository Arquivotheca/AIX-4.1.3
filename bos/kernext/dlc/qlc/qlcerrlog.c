static char sccsid[] = "@(#)12  1.4  src/bos/kernext/dlc/qlc/qlcerrlog.c, sysxdlcq, bos411, 9428A410j 11/2/93 09:35:35";

/*
 * COMPONENT_NAME: (SYSXDLCQ) X.25 QLLC module
 *
 * FUNCTIONS: errlogger
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/err_rec.h>

void  qlcerrlog(
  unsigned      error_id,
  char	       *resource_name,
  unsigned int  user_ls_correlator,
  char          *calling_address,
  char          *called_address)

{
  struct err_rec   err_buf;
  char            *write_ptr;
  int              res_name_len;
  
  outputf("QLLC ERROR LOG \n");
  outputf(" error_id           = %x\n", error_id);
  outputf(" resource_name      = %s\n", resource_name);
  outputf(" user_ls_correlator = %x\n", user_ls_correlator);
  outputf(" calling_address    = %s\n", calling_address);
  outputf(" called_address     = %s\n", called_address);
  
  err_buf.error_id = error_id;

  res_name_len = strlen(resource_name);
  outputf("res_name_len = %d\n",res_name_len);
  if (res_name_len > 8)
    res_name_len = 8;
  memset(err_buf.resource_name, ' ', ERR_NAMESIZE);
  strncpy(err_buf.resource_name, resource_name, res_name_len);
  outputf("strncpy done\n");
  write_ptr = err_buf.detail_data;
  outputf("bcopy\n");
  bcopy(&user_ls_correlator,write_ptr,4);
  write_ptr += 4;
  if (calling_address != 0)
  {
    outputf("calling address copy \n");
    strncpy(write_ptr,calling_address,15);
    write_ptr += 15;
  }
  if (called_address != 0)
  {
    outputf("called address copy \n");
    strncpy(write_ptr,called_address,15);
    write_ptr += 15;
  }
  outputf("call errsave: buf addr = %x, length = %d\n",&err_buf,
    (int)(write_ptr - (char *)&err_buf) );
  errsave(&err_buf, (int)(write_ptr - (char *)&err_buf));
  outputf("back\n");
  return;
}
