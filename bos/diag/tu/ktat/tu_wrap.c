static char sccsid[] = "@(#)98	1.3  src/bos/diag/tu/ktat/tu_wrap.c, tu_ktat, bos41J, 9522A_all 5/24/95 10:56:21";
/*
 *   COMPONENT_NAME: tu_ktat
 *
 *   FUNCTIONS: tu_wrap
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
#include <memory.h>
#include <sys/diagex.h>
#include <sys/dma.h>
#include "kent_defs.h"
#include "kent_tu_type.h"

#define FAILED 0x00000001
#define INITIALIZATION_FAILED 0x00000090 
#define BUF0_MISCOMPARE 0x00000010
#define BUF1_MISCOMPARE 0x00000011
#define BUF2_MISCOMPARE 0x00000012
#define BUF3_MISCOMPARE 0x00000013
#define XMIT_TIMED_OUT  0x00000021
#define WRAP_FAILED     0x00000022

#define own_bit 0x00000080

extern diag_struc_t  *diagex_hdl;
extern diagex_dds_t   dds;
extern struct int_data idata;

init_block_t init_blk;
init_block_t *buf;
extern char w_pat[];
extern char *tbuf0, *tbuf1, *tbuf2, *tbuf3;
extern char *rbuf0, *rbuf1, *rbuf2, *rbuf3;
extern tx_desc_t *t_desc0, *t_desc1, *t_desc2, *t_desc3;
extern rx_desc_t *r_desc0, *r_desc1, *r_desc2, *r_desc3;
extern char *big_buf;
extern char *desc_malloc_buf;

uchar net_addr[6];    /*  adapter network address  */
/* 
  tx_desc_t t_desc[max_desc];
  rx_desc_t r_desc[max_desc];
*/
  ulong dma_addr;
  ulong r_desc0_dma_addr;
  ulong r_desc1_dma_addr;
  ulong r_desc2_dma_addr;
  ulong r_desc3_dma_addr;
  ulong t_desc0_dma_addr;
  ulong t_desc1_dma_addr;
  ulong t_desc2_dma_addr;
  ulong t_desc3_dma_addr;

tu_wrap(int wrap_mode, int port_select)
{
  int r_data, r_data1, r_data2;
  int w_data, work1, work2;
  int base_addr;
  int rap_addr;
  int rdp_addr;
  int bdp_addr;
  int junk;
/*  char *buf;  */
  int flag, count;
  int poll_flg;
  int i;
  int rc, exit_code;
#define buf_size 0x400
#define intr_msk 0x085f0000
#define idon_intr_enable 0x085e0000
#define tint_rint_enable 0x08590000
#define start_init_irq 0x41000000
#define start_xmit_irq 0x42000000


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


/****  turn off ASEL bit in bcr2  **************/

  if (rc = io_write(IOLONG, rap, bcr2))
    return(rc);
  if (rc = io_write(IOLONG, bdp, port_select))
    return(rc);

/* check interrupt mask    */
/*
  if (rc = io_write(IOLONG, rap, csr3))
    return(rc);
  if (rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
printf("csr3 int mask reg, = 0x%08x\n", r_data);
scanf("%d", &junk);
*/

/* Build the transmit and received descripters     */

   if (rc = bld_desc())
     return(rc);

/*
printf("T_DESC 0  before xmit\n");
printf("tbadr = 0x%08x\n", swap32(t_desc0->tbadr));
printf("tmd1 =  0x%08x\n", swap32(t_desc0->tmd1));
printf("tmd2 =  0x%08x\n", swap32(t_desc0->tmd2));
printf("rbadr = 0x%08x\n", swap32(r_desc0->rbadr));
printf("rmd1 =  0x%08x\n", swap32(r_desc0->rmd1));
printf("rmd2 =  0x%08x\n", swap32(r_desc0->rmd2));
scanf("%d", &junk);
*/
/*  build the initialization block                               */

  init_blk.tlen_rlen_mode = wrap_mode;
  if(rc = io_read(IOLONG, aprom_1, &r_data))
    return(rc);
  if(rc = io_read(IOLONG, aprom_0, &w_data))
    return(rc);

/*
printf("aprom_0 = 0x%08x\n", w_data);
printf("aprom_1 = 0x%08x\n", r_data);
*/

  init_blk.padr_hi2 = (r_data & 0xffff0000); 
  init_blk.padr_lo4 = w_data;

  init_blk.ladr_lo = ladr_lo4;
  init_blk.ladr_hi = ladr_hi4;

/*  store the network address in net_addr     */

  net_addr[0] = (uchar)((w_data & 0xff000000) >> 24);
  net_addr[1] = (uchar)((w_data & 0x00ff0000) >> 16);
  net_addr[2] = (uchar)((w_data & 0x0000ff00) >> 8);
  net_addr[3] = (uchar)(w_data & 0x000000ff);
  net_addr[4] = (uchar)((r_data & 0xff000000) >> 24);
  net_addr[5] = (uchar)((r_data & 0x00ff0000) >> 16);

/* put the network address in the transmit buffers    */

  COPY_NADR(net_addr, tbuf0);
  COPY_NADR(net_addr, tbuf0 + 6);
  COPY_NADR(net_addr, tbuf1);
  COPY_NADR(net_addr, tbuf1 + 6);
  COPY_NADR(net_addr, tbuf2);
  COPY_NADR(net_addr, tbuf2 + 6);
  COPY_NADR(net_addr, tbuf3);
  COPY_NADR(net_addr, tbuf3 + 6);

/* put fram length in buffers    */

  *(tbuf0 + 12) = 0x00000004;
  *(tbuf0 + 13) = 0x00000000;
  *(tbuf1 + 12) = 0x00000004;
  *(tbuf1 + 13) = 0x00000000;
  *(tbuf2 + 12) = 0x00000004;
  *(tbuf2 + 13) = 0x00000000;
  *(tbuf3 + 12) = 0x00000004;
  *(tbuf3 + 13) = 0x00000000;


/*
hexdump(tbuf0, 32);
scanf("%d", &junk);
*/
/*  set up dma for r_desc  and t_desc addresses         */

  if(rc = dma_setup(r_desc0, &r_desc0_dma_addr, DMA_READ, 16))
    return(rc);
  if(rc = dma_setup(r_desc1, &r_desc1_dma_addr, DMA_READ, 16))
    return(rc);
  if(rc = dma_setup(r_desc2, &r_desc2_dma_addr, DMA_READ, 16))
    return(rc);
  if(rc = dma_setup(r_desc3, &r_desc3_dma_addr, DMA_READ, 16))
    return(rc);
  if(rc = dma_setup(t_desc0, &t_desc0_dma_addr, DMA_READ, 16))
    return(rc);
  if(rc = dma_setup(t_desc1, &t_desc1_dma_addr, DMA_READ, 16))
    return(rc);
  if(rc = dma_setup(t_desc2, &t_desc2_dma_addr, DMA_READ, 16))
    return(rc);
  if(rc = dma_setup(t_desc3, &t_desc3_dma_addr, DMA_READ, 16))
    return(rc);
  init_blk.rdra = (ulong)swap32(r_desc0_dma_addr);
  init_blk.tdra = (ulong)swap32(t_desc0_dma_addr);

/*  set up dma for init_blk address         */

  if(rc = dma_setup(&init_blk, &dma_addr, DMA_READ, sizeof(init_blk)))
    return(rc);

/* kick off the initialization after putting the init_blk in csr1 and csr2 */

  work1 = swap32(dma_addr);
  w_data = work1 & 0xffff0000;
  if(rc = io_write(IOLONG, rap, csr1))
    return(rc);
  if(rc = io_write(IOLONG, rdp, w_data))
    return(rc);
  w_data = (work1 & 0x0000ffff) << 16;
  if(rc = io_write(IOLONG, rap, csr2))
    return(rc);
  if(rc = io_write(IOLONG, rdp, w_data))
    return(rc);
/*
printf("init_blk address = 0x%08x\n\n", dma_addr);
scanf("%d", &junk);
*/
/*  Turn on BMEN in cmd_stat reg       */

  if (rc= config_reg_write(cmd_stat_reg, ioen_bmen_set))
    return(rc);

/* enable IDON interrupt      */

  if (rc = io_write(IOLONG, rap, csr3))
    return(rc);
  if (rc = io_write(IOLONG, rdp, idon_intr_enable))
    return(rc);

  idata.expect = IDON;

  if(rc = io_write(IOLONG, rap, csr0))
    return(rc);
  if(rc = io_write(IOLONG, rdp, start_init_irq))
    return(rc);


#ifdef HAVESLIH
/*  printf("calling wait_4_intr for initializtion\n"); */

  rc = wait_4_intr(INTR_MSK, INIT_TIMEOUT);
/*  printf("rc         = 0x%08x\n", rc);  */

#else 
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
#endif

/*  disable interrupts   */

  if (rc = io_write(IOLONG, rap, csr3))
    return(rc);
  if (rc = io_write(IOLONG, rdp, intr_msk))
    return(rc);


/*  turn on stop bit in csr0                                            */

  if(rc = io_write(IOLONG, rap, csr0))
    return(rc);
  if(rc = io_write(IOLONG, rdp, stop))
    return(rc);

#ifdef HAVESLIH

/*  printf("IRQ pending = 0x%08x\n", idata.pending); */

  if(idata.pending & idata.expect != idata.expect)
  {
    return(INITIALIZATION_FAILED);
  }

#else
  if(i == 0)
  {
    return(INITIALIZATION_FAILED);
  } 
#endif

/*  clean up dma stuff                                           */

  if (rc = dma_finish(&dma_addr))
    return(rc);
/*
printf("r_desc addr = 0x%08x\n", r_desc_dma_addr);
printf("t_desc addr = 0x%08x\n", t_desc_dma_addr);

printf("initialization block .........\n");
printf("init_blk.tlen_rlen_mode = 0x%08x\n", swap32(init_blk.tlen_rlen_mode));
*/

/*
printf("init_blk.ladr_lo        = 0x%08x\n", init_blk.ladr_lo);
printf("init_blk.ladr_hi        = 0x%08x\n", init_blk.ladr_hi);
printf("init_blk.rdra           = 0x%08x\n", init_blk.rdra);
printf("init_blk.tdra           = 0x%08x\n", init_blk.tdra);
scanf("%d", &junk);
*/

/* the following checks are for debug    */
/*  check mode register, csr15    */
/*
  if(rc = io_write(IOLONG, rap, csr15))
    return(rc);
  if(rc = io_read(IOLONG, rdp,&r_data))
    return(rc);
printf("MODE register (csr15) = 0x%08x\n", swap32(r_data));
*/

/*  check physical address - debug only   */
/*
  if(rc = io_write(IOLONG, rap, csr14))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  printf("bits 48:32 csr14 of physical addr = 0x%08x\n", swap32(r_data));
  if(rc = io_write(IOLONG, rap, csr13))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  printf("bits 31:16 csr13 of physical addr = 0x%08x\n", swap32(r_data));
  if(rc = io_write(IOLONG, rap, csr12))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  printf("bits 15:0  csr12 of physical addr = 0x%08x\n", swap32(r_data));
*/  

/*   kick off wrap           */
/*
printf("Kick off wrap ****************\n\n");
printf("t_desc0 = 0x%08x\n", t_desc0_dma_addr);
printf("t_desc1 = 0x%08x\n", t_desc1_dma_addr);
printf("t_desc2 = 0x%08x\n", t_desc2_dma_addr);
printf("t_desc3 = 0x%08x\n", t_desc3_dma_addr);
printf("r_desc0 = 0x%08x\n", r_desc0_dma_addr);
printf("r_desc1 = 0x%08x\n", r_desc1_dma_addr);
printf("r_desc2 = 0x%08x\n", r_desc2_dma_addr);
printf("r_desc3 = 0x%08x\n", r_desc3_dma_addr);
printf("tbadr0  = 0x%08x\n", swap32(t_desc0->tbadr));
printf("tbadr1  = 0x%08x\n", swap32(t_desc1->tbadr));
printf("tbadr2  = 0x%08x\n", swap32(t_desc2->tbadr));
printf("tbadr3  = 0x%08x\n", swap32(t_desc3->tbadr));
printf("rbadr0  = 0x%08x\n", swap32(r_desc0->rbadr));
printf("rbadr1  = 0x%08x\n", swap32(r_desc1->rbadr));
printf("rbadr2  = 0x%08x\n", swap32(r_desc2->rbadr));
printf("rbadr3  = 0x%08x\n", swap32(r_desc3->rbadr));
scanf("%d", &junk);
*/

  if(rc = io_write(IOLONG, rap, csr0))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
/*  printf("csr0 before wrap start  =   0x%08x\n", r_data); */

#ifdef HAVESLIH
/*     enable TINT and RINT interrupts      */

  if (rc = io_write(IOLONG, rap, csr3))
    return(rc);
  if (rc = io_write(IOLONG, rdp, tint_rint_enable))
    return(rc);

  idata.expect = TINT_INTR | RINT_INTR;

  idata.intr_count = 0;
  idata.xmit_count = 0;
  idata.rec_count = 0;

  if(rc = io_write(IOLONG, rap, csr0))
    return(rc);
  if(rc = io_write(IOLONG, rdp, start_xmit_irq))
    return(rc);

  do
  {
    rc = wait_4_intr(INTR_MSK, XMIT_TIMEOUT);
  }
  while(((r_desc3->rmd1 & own_bit) == own_bit) && rc == 0);
  exit_code = rc;

/*
  printf("rc from interrupt  = 0x%08x\n", rc);
  printf("intr_count         = %d\n", idata.intr_count);
  printf("xmit_count         = %d\n", idata.xmit_count);
  printf("rec_count          = %d\n", idata.rec_count);
  printf("rmd3               =  0x%08x\n", r_desc3->rmd1);
  printf("rmd3 swap32        =  0x%08x\n", swap32(r_desc3->rmd1));
*/

#else 

  if(rc = io_write(IOLONG, rdp, start_xmit))
    return(rc);
   sleep(2); /*  <==============*/

/* Look for TINT and RINT bits in csr0    */
 
  poll_flg = 1;
  i = 2000;

  while(poll_flg == 1 && i != 0)
  {
    if(rc = io_read(IOLONG, rdp, &r_data))
      return(rc);
    if((r_data & T_RINT) == T_RINT)
      poll_flg = 0;
    i--;
  }
/*
printf("csr0 looking for wrap complete =      0x%08x\n", swap32(r_data));
scanf("%d", &junk);
*/

#endif  /*   end if HAVESLIH   */

/*  turn on stop bit in csr0                                            */
  if(rc = io_write(IOLONG, rdp, stop))
    return(rc);

/*  printf("IRQ pending = 0x%08x\n", idata.pending); */

/* clean up dma       */

  if(rc = dma_cleanup())
    return(rc);

/*
printf("T_DESC 0  after xmit\n");
printf("tbadr = 0x%08x\n", swap32(t_desc0->tbadr));
printf("tmd1 =  0x%08x\n", swap32(t_desc0->tmd1));
printf("tmd2 =  0x%08x\n", swap32(t_desc0->tmd2));
printf("tmd1-1 =  0x%08x\n", swap32(t_desc1->tmd1));
printf("tmd1-2 =  0x%08x\n", swap32(t_desc2->tmd1));
printf("tmd1-3 =  0x%08x\n", swap32(t_desc3->tmd1));
*/

/*
printf("rbadr = 0x%08x\n", swap32(r_desc0->rbadr));
printf("rmd1 =  0x%08x\n", swap32(r_desc0->rmd1));
printf("rmd2 =  0x%08x\n", swap32(r_desc0->rmd2));
scanf("%d", &junk);
*/

/*
printf("tbuf0  = 0x%08x\n", tbuf0);
scanf("%d", &junk);
printf("Transmit buffer 0\n");
hexdump(tbuf0, 64);
*/
/*
printf("Receive buffer 0\n");
hexdump(rbuf0, 64);
*/
/*  Compare transmit and receive buffers         */

  if((exit_code != 0) && ((r_desc3->rmd1 & own_bit) == own_bit))
  {
    free(big_buf);
    free(desc_malloc_buf);
    return(XMIT_TIMED_OUT);
  }
  if((r_desc3->rmd1 & own_bit) == own_bit)
  {
    free(big_buf);
    free(desc_malloc_buf);
    return(WRAP_FAILED);
  }

  if(rc = memcmp(tbuf0, rbuf0, buf_size))
  {
#ifdef DEBUG
    printf("IRQ pending = 0x%08x\n", idata.pending);
#endif
    free(big_buf);
    free(desc_malloc_buf);
    return(BUF0_MISCOMPARE);
  } 

  if(rc = memcmp(tbuf1, rbuf1, buf_size))
  {
    free(big_buf);
    free(desc_malloc_buf);
    return(BUF1_MISCOMPARE);
  } 

  if(rc = memcmp(tbuf2, rbuf2, buf_size))
  {
    free(big_buf);
    free(desc_malloc_buf);
    return(BUF2_MISCOMPARE);
  } 

  if(rc = memcmp(tbuf3, rbuf3, buf_size))
  {
    free(big_buf);
    free(desc_malloc_buf);
    return(BUF3_MISCOMPARE);
  } 

/*
  if(rc = io_write(IOLONG, rap, csr0))
    return(rc);
  if(rc = io_read(IOLONG, rdp, &r_data))
    return(rc);
  printf("csr0 after wrap complete  =   0x%08x\n", r_data);
*/

  free(big_buf);
  free(desc_malloc_buf);
  return(0);


}  /* end tu_wrap  */

