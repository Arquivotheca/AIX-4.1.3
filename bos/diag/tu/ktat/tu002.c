static char sccsid[] = "@(#)90	1.3  src/bos/diag/tu/ktat/tu002.c, tu_ktat, bos41J, 9522A_all 5/24/95 10:55:45";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu002
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
#define miscompare 0x20000001
#define tu2_fail1  0x20000001
#define tu2_fail2  0x20000002
#define tu2_fail3  0x20000003
#define tu2_fail4  0x20000004
#define tu2_fail5  0x20000005
#define tu2_fail6  0x20000006
#define tu2_fail7  0x20000007
#define tu2_fail8  0x20000008
#define tu2_fail9  0x20000009
#define tu2_fail10 0x2000000a
#define tu2_fail11 0x2000000b
#define tu2_fail12 0x2000000c


extern diag_struc_t  *diagex_hdl;
extern diagex_dds_t   dds;

static int group1[] = {csr1, csr2, csr8, csr9, csr10, csr11, csr12, csr13,
                       csr14, csr15, csr16, csr17, csr18, csr19, csr20,
                       csr21, csr22, csr23, csr24, csr25, csr26, csr27,
                       csr28, csr29, csr30, csr31, csr32, csr33, csr34,
                       csr35, csr36, csr37, csr38, csr39, csr46, csr47,
                       csr60, csr61, csr64, csr65, csr72, csr74, csr76,
                       csr78, csr84, csr85};
static int pat1[] = {0xffff0000, 0xaaaa0000, 0x55550000, 0x00000000};
#define pat1mask 0xffff0000

static int group2[] = {csr40, csr42, csr44, csr62, csr66, csr86};
static int pat2[] = {0xff0f0000, 0xaa0a0000, 0x55050000, 0x00000000};
#define pat2mask 0xff0f0000 

static int group3[] = {csr41, csr43, csr45, csr63, csr67};
static int pat3[] = {0x00ff0000, 0x00aa0000, 0x00550000, 0x00000000};
#define pat3mask 0x00ff0000

static int group5[] = {csr112, csr114};
static int group5_save[2];

static int csr3_pat[] = {0x3c5f0000, 0x00050000, 0x000a0000, 0x00000000};
static int csr4_pat[] = {0xffff0000, 0x55550000, 0xaaaa0000, 0x00000000};
static int csr4_expect[] = {0x15fd0000, 0x15550000, 0x00280000, 0x00000000};
static int csr80_pat[] = {0xff3f0000, 0x55150000, 0xaa2a0000, 0x00000000};
static int csr80_expect[] = {0xffff0000, 0x55d50000, 0xaaea0000, 0x00c00000};


tu002()
{
  int junk;
  int rc;
  int i;
  int j;
  int r_data;
  int w_data;
  int rap_addr;
  int base_addr;
  int rdp_addr;
  int bdp_addr;

#define intr_msk 0x085f0000

/*  issue s_reset to adapter                     */

  if (rc = io_read(IOLONG, reset_reg, &r_data))
    return(rc);
  sleep(1);
/*  turn on the software style bits in bcr20 to allow 32 bits   */

  if (rc = io_write(IOLONG, rap, bcr20))
    return(rc);
  if (rc = io_write(IOLONG, bdp, software_style))
    return(rc);

/* read and write csr0 and leave the stop bit on  */

  if (rc = io_write(IOLONG, rap, csr0))
    return(rc);
  if (rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  r_data = r_data | csr0_stop;
  if (rc = io_write(IOLONG, rdp, r_data))
    return(rc);

/*  set the interrupt mask in csr3   */

  if (rc = io_write(IOLONG, rap, csr3))
    return(rc);
  if (rc = io_write(IOLONG, rdp, intr_msk))
    return(rc);

/*  write and read csr group 1                                            */

  for(j = 0; j < (sizeof(pat1)/4); j++)
  {
    for (i = 0; i < (sizeof(group1)/4); i++)
    {
      if(rc = io_write(IOLONG, rap, group1[i]))
        return(rc);
      if (rc = io_write(IOLONG, rdp, pat1[j]))
        return(rc);
      if (rc = io_read(IOLONG, rdp, &r_data))
        return(rc);
      if ((r_data & pat1mask) != pat1[j])
      {
        DEBUG1("failing reg GROUP1 = 0x%08x  pat = %d\n", group1[i], j);
        return(tu2_fail2);
      }
    }  /* end of i for loop        */
  }   /* end of pattern for loop   */

/*  write and read csr group 2                         */

  for(j = 0; j < (sizeof(pat2)/4); j++)
  {
    for (i = 0; i < (sizeof(group2)/4); i++)
    {
      if(rc = io_write(IOLONG, rap, group2[i]))
        return(rc);
      if (rc = io_write(IOLONG, rdp, pat2[j]))
        return(rc);
      if (rc = io_read(IOLONG, rdp, &r_data))
        return(rc);
      if ((r_data & pat2mask) != pat2[j])
      {
        DEBUG1("Failing reg GROUP2 = 0x%08x  pat = %d\n", group2[i], j);
        return(tu2_fail3);
      }
    }  /* end of i for loop        */
  }   /* end of pattern for loop   */

/*  write and read csr group 3                                            */

  for(j = 0; j < (sizeof(pat3)/4); j++)
  {
    for (i = 0; i < (sizeof(group3)/4); i++)
    {
      if(rc = io_write(IOLONG, rap, group3[i]))
        return(rc);
      if (rc = io_write(IOLONG, rdp, pat3[j]))
        return(rc);
      if (rc = io_read(IOLONG, rdp, &r_data))
        return(rc);
      if ((r_data & pat3mask) != pat3[j])
      {
        DEBUG1("failing reg GROUP3 = 0x%08x  pat = %d\n", group3[i], j);
        return(tu2_fail4);
      }
    }  /* end of i for loop        */
  }   /* end of pattern for loop   */

/*  write and read csr3                                                  */

  if(rc = io_write(IOLONG, rap, csr3))
    return(rc);
  for (i = 0; i < (sizeof(csr3_pat)/4); i++)
  {
    if (rc = io_write(IOLONG, rdp, csr3_pat[i]))
      return(rc);
    if (rc = io_read(IOLONG, rdp, &r_data))
      return(rc);
    if ((r_data & csr_rmsk) != csr3_pat[i])
    {
      DEBUG1("expected data: 0x%08x/n", csr3_pat[i]);
      DEBUG1("actual data:   0x%08x/n", r_data);
      return(tu2_fail5);
    }
  }  /* end of i for loop        */

/*  write and read csr4                  */

  if(rc = io_write(IOLONG, rap, csr4))
    return(rc);
  for (i = 0; i < (sizeof(csr4_pat)/4); i++)
  {
    if (rc = io_write(IOLONG, rdp, csr4_pat[i]))
      return(rc);
    if (rc = io_read(IOLONG, rdp, &r_data))
      return(rc);
    if ((r_data & 0x3fff0000) != csr4_expect[i])
      return(tu2_fail6);
  }  /* end of i for loop        */

/*  write and read csr80                      */

  if(rc = io_write(IOLONG, rap, csr80))
    return(rc);
  for (i = 0; i < (sizeof(csr80_pat)/4); i++)
  {
    if (rc = io_write(IOLONG, rdp, csr80_pat[i]))
      return(rc);
    if (rc = io_read(IOLONG, rdp, &r_data))
      return(rc);
    if ((r_data & csr_rmsk) != csr80_expect[i])
      return(tu2_fail7);
  }  /* end of i for loop        */
return(0);

} /*  end tu002         */


