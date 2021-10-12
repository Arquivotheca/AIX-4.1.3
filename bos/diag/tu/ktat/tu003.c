static char sccsid[] = "@(#)91	1.3  src/bos/diag/tu/ktat/tu003.c, tu_ktat, bos41J, 9522A_all 5/24/95 10:55:57";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu003
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
#include <sys/dma.h>
#include "kent_defs.h"
#include "kent_tu_type.h"
#define miscompare  0x30000001
#define tu3_fail1   0x30000001
#define tu3_fail2   0x30000002
#define tu3_fail3   0x30000003
#define tu3_fail4   0x30000004
#define tu3_fail5   0x30000005
#define tu3_fail6   0x30000006
#define tu3_fail7   0x30000007
#define tu3_fail8   0x30000008
#define tu3_fail9   0x30000009
#define tu3_fail10  0x3000000a
#define tu3_fail11  0x3000000b
#define tu3_fail12  0x3000000c
#define tu3_fail13  0x3000000d

extern diag_struc_t  *diagex_hdl;
extern diagex_dds_t   dds;
init_block_t init_blk;
init_block_t *buf;

tu003()
{
  int r_data, r_data1, r_data2;
  int w_data, work1;
  int base_addr;
  int rap_addr;
  int rdp_addr;
  int bdp_addr;
  tx_desc_t t_desc[max_desc];
  rx_desc_t r_desc[max_desc];
/*  char *buf;  */
  ulong dma_addr;
  int flag;
  int count;
  int poll_flg;
  int i;
  int rc;
  int junk;
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

/*  build the initialization block                               */
    DEBUG1("Build the initialization block\n");

  init_blk.tlen_rlen_mode = t_r_len_mode1;

  if(rc = io_read(IOLONG, aprom_1, &r_data))
    return(rc);

  DEBUG1("aprom_1 = 0x%08x\n", r_data);

  if(rc = io_read(IOLONG, aprom_0, &w_data))
    return(rc);
  DEBUG1("aprom_0 = 0x%08x\n", w_data);

  init_blk.padr_hi2 = ((w_data & 0xffff0000) >> 16);
  init_blk.padr_lo4 = ((w_data &0x0000ffff) << 16)
                      | ((r_data & 0xffff0000) >> 16);

  init_blk.ladr_lo = ladr_lo4;
  init_blk.ladr_hi = ladr_hi4;
  init_blk.rdra = (ulong)&r_desc;
  init_blk.tdra = (ulong)&t_desc;

/*  set up dma                                                     */

  buf = &init_blk;
  count = sizeof(init_blk);
  if(rc = dma_setup(buf, &dma_addr, DMA_READ, count))
    return(rc);
  work1 = swap32(dma_addr);

/* kick off the initialization after putting the init_blk in csr1 and csr2 */

  w_data = work1 & 0xffff0000;
  if(rc = io_write(IOLONG, rap, csr1))
    return(rc);
  if(rc = io_write(IOLONG, rdp, w_data))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data2))
    return(rc);
  w_data = (work1 & 0x0000ffff) << 16;
  if(rc = io_write(IOLONG, rap, csr2))
    return(rc);
  if(rc = io_write(IOLONG, rdp, w_data))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data1))
    return(rc);
  DEBUG1("csr1 = 0x%08x,  csr2 = 0x%08x\n", r_data2, r_data1);

/*  Turn on BMEN in cmd_stat reg      */

  if (rc= config_reg_write(cmd_stat_reg, ioen_bmen_set))
    return(rc);
  if(rc = io_write(IOLONG, rap, csr0))
    return(rc);
  if(rc = io_write(IOLONG, rdp, start_init))
    return(rc);
 
/* poll IDON bit of csr0 to determine when initialization is complete  */
/* may replace this pool with an interrupt later                       */
  poll_flg = 1;
  i = 2000;

  while(poll_flg == 1 && i != 0)
  {
    if(rc = io_read(IOLONG, rdp, &r_data))
      return(rc);
    if((r_data & IDON) == IDON) 
      poll_flg = 0;
    i--;
  }

/*  turn on stop bit in csr0                           */
  if(rc = io_write(IOLONG, rdp, stop))
    return(rc);
/*
printf("\nInitialization complete, poll_flag = %d, i = %d\n", poll_flg, i);
printf("\ninitialization block .......... \n");
printf("init_blk.tlen_rlen_mode = 0x%08x\n", init_blk.tlen_rlen_mode);
printf("init_blk.padr_lo4       = 0x%08x\n", init_blk.padr_lo4);
printf("init_blk.padr_hi2       = 0x%08x\n", init_blk.padr_hi2);
printf("init_blk.ladr_lo        = 0x%08x\n", init_blk.ladr_lo);
printf("init_blk.ladr_hi        = 0x%08x\n", init_blk.ladr_hi);
printf("init_blk.rdra           = 0x%08x\n", init_blk.rdra);
printf("init_blk.tdra           = 0x%08x\n", init_blk.tdra);
*/

/*  clean up dma stuff                                 */

  if (rc = dma_finish(&dma_addr))
    return(rc);

/*  check appropriate CSRs for correct initial values                   */
/*  check mode register, csr15    */

  if(rc = io_write(IOLONG, rap, csr15))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  DEBUG1("csr15 = 0x%08x\n", r_data);

  if ((r_data & lo_msk) != (init_blk.tlen_rlen_mode & lo_msk))
     return(tu3_fail1);

/*  check transmit and receive descripter table length, csr6    */

  if(rc = io_write(IOLONG, rap, csr6))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != csr6_initial)
    return(tu3_fail2);

/*  check physical address, csr12, csr13, csr14                  */

  if(rc = io_write(IOLONG, rap, csr12))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.padr_lo4 & lo_msk))
     return(tu3_fail3);

  if(rc = io_write(IOLONG, rap, csr13))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.padr_lo4 & hi_msk) << 16))
     return(tu3_fail4);

  if(rc = io_write(IOLONG, rap, csr14))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.padr_hi2 & lo_msk))
     return(tu3_fail5);

/* check logical address, csr8, csr9, csr10, csr11      */

  if(rc = io_write(IOLONG, rap, csr8))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.ladr_lo & lo_msk))
     return(tu3_fail6);
  if(rc = io_write(IOLONG, rap, csr9))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.ladr_lo & hi_msk) << 16))
     return(tu3_fail7);

  if(rc = io_write(IOLONG, rap, csr10))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.ladr_hi & lo_msk))
     return(tu3_fail8);
  if(rc = io_write(IOLONG, rap, csr11))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.ladr_hi & hi_msk) << 16))
     return(tu3_fail9);

/* check base address of receive ring, csr24, csr25         */

  if(rc = io_write(IOLONG, rap, csr24))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.rdra & lo_msk))
     return(tu3_fail10);
  if(rc = io_write(IOLONG, rap, csr25))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.rdra & hi_msk) << 16))
     return(tu3_fail11);

/* check base address of transmit ring, csr30, csr31        */

  if(rc = io_write(IOLONG, rap, csr30))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.tdra & lo_msk))
     return(tu3_fail12);
  if(rc = io_write(IOLONG, rap, csr31))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.tdra & hi_msk) << 16))
     return(tu3_fail13);

/* check current reveive descripter address, csr28, csr29     */
/*
  if(rc = io_write(IOLONG, rap, csr28))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.rdra & lo_msk))
     return(tu3_fail14);
  if(rc = io_write(IOLONG, rap, csr29))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.rdra & hi_msk) >> 16))
     return(tu3_fail15);
*/
/*  check next receive descripter address, csr26, csr27    */
/*
  if(rc = io_write(IOLONG, rap, csr26))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if (((r_data + 0x10) & lo_msk) != (init_blk.rdra & lo_msk))
     return(tu3_fail16);
  if(rc = io_write(IOLONG, rap, csr27))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.rdra & hi_msk) >> 16))
     return(tu3_fail17);
*/
/*  check current transmit descripter address, csr34, csr35   */
/*
  if(rc = io_write(IOLONG, rap, csr34))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != (init_blk.tdra & lo_msk))
     return(tu3_fail18);
  if(rc = io_write(IOLONG, rap, csr35))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.tdra & hi_msk) >> 16))
     return(tu3_fail19);
*/
/*  check next transmit descripter address, csr32, csr33    */
/*
  if(rc = io_write(IOLONG, rap, csr32))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if (((r_data + 0x10) & lo_msk) != (init_blk.tdra & lo_msk))
     return(tu3_fail20);
  if(rc = io_write(IOLONG, rap, csr33))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  if ((r_data & lo_msk) != ((init_blk.tdra & hi_msk) >> 16))
     return(tu3_fail21);
*/
  return(0);

}  /* end tu003  */

