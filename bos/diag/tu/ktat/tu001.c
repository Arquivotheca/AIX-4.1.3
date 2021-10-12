static char sccsid[] = "@(#)89	1.3  src/bos/diag/tu/ktat/tu001.c, tu_ktat, bos41J, 9522A_all 5/24/95 10:55:26";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu001
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

#include <stdio.h>
#include <sys/diagex.h>
#include "kent_defs.h"
#include "kent_tu_type.h"
#include <sys/errno.h>

extern diag_struc_t   *diagex_hdl;
extern diagex_dds_t    dds;
error_details *errs;

/* configuration register offsets                                    */

static int cfg_reg_offset[] = {0x00, 0x08, 0x0c, 0x04, 0x10, 0x3c};

/* The configuration registers at offsets 0x00, 0x08, and 0x0c
* are read only. The expected values are:                            */

static int rd_value[] = {0x22100020, 0x00000002, 0x00000000};
static int ba_w_value[] = {0x00ffffff, 0x00aaaaaa, 0x00555555};
static int ba_rd_value[] = {0x01ffffff, 0x01aaaaaa, 0x01555555};
static int int_w_value[] = {0xff000000, 0xaa000000, 0x55000000};
static int int_rd_value[] = {0xff010000, 0xaa010000, 0x55010000};
#define miscompare 0x10000001
#define fail1 0x10000001
#define fail2 0x10000002
#define fail3 0x10000003
#define fail4 0x10000004
#define fail5 0x10000005
#define fail6 0x10000006
#define ioen_reset 0x00000000

tu001()
{
  int junk;
  int i;
  int rc;
  int value;
  int expect;
  int r_data;
  int config_save[6]; /* save area for configuration registers  */

/* save the configuration registers                             */
  for(i = 0; i < 6; i++)
  {
/*
    junk = (int) &config_save[i];
    printf("address to place data = 0x%08x\n", junk);
*/
    if(rc = config_reg_read(cfg_reg_offset[i], &config_save[i]))
    {
      return(rc);
    }
 }

/* turn off the IOEN and BMEN bits in the command register       */
/*
  value = config_save[3] & ioen_reset;
  if(rc = config_reg_write(cmd_stat_reg, value))
    return(rc);
*/
/* varify the read only registers                                */

    if(config_save[0] != rd_value[0])
    {
      DEBUG1("expected value: 0x%08x\n", rd_value[0]);
      DEBUG1("actual value:   0x%08x\n", config_save[0]);
      return(fail1);
    }

    if((config_save[1] &0x00ffffff) != rd_value[1])
    {
      return(fail5);
    }

/* write and read the cmd and status registers                    */
/* need to read and write back first to put register in clean state  */

    if(rc = config_reg_read(cmd_stat_reg, &r_data))
      return(rc);
    if(rc = config_reg_write(cmd_stat_reg, r_data))
      return(rc);

/* write and read the int registers                               */

  for(i = 0; i < 3; i++)
  { 
    if(rc = config_reg_write(int_reg, int_w_value[i]))
      return(rc);

    if(rc = config_reg_read(int_reg, &r_data))
      return(rc);
/*
printf("r_data = 0x%08x\n", r_data);
scanf("%d", &junk);
*/
    if((r_data & 0xffff0000) != int_rd_value[i])
   {
      return(fail4);
    }
  }
/* restore the configuration registers                             */

  for(i=3; i<6; i++)
  {
    if(rc = config_reg_write(cfg_reg_offset[i], config_save[i]))
      return(rc);
  }
 
/* turn on the IOEN and BMEN bits in the command register          */
/*
  value = config_save[3] | ioen_bmen_set;
  if(rc = config_reg_write(cmd_stat_reg, value))
    return(rc);
  if(rc = config_reg_read(cmd_stat_reg, &r_data))
  return(rc);
*/
/*
  printf("r_data = 0x%08x\n", r_data);
  scanf("%d", &junk);
*/

  return(0);

} /* end tu001 */


