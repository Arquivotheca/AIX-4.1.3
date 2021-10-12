static char sccsid[] = "@(#)43  1.1.1.2  src/bos/diag/tu/ethsal/config.c, tu_ethsal, bos411, 9428A410j 10/21/93 09:39:56";
/*
 *   COMPONENT_NAME: tu_ethsal
 *
 *
 *   FUNCTIONS: config_ether
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/ciouser.h>
#include <sys/comio.h>
#include <sys/entuser.h>
#include <stdio.h>

#include "exectu.h"
#include "tu_type.h"

#define BYTE_COUNT      0xe  /* 14 bytes, prefetch off    */
#define MAX_FIFO_LIMIT  0xf  /* maximum FIFO limit        */
#define DEF_FIFO_LIMIT  0x8  /* default FIFO limit        */
#define DISABLE_MONITOR 0xc0 /* monitor function disabled */
#define DONT_SAVE_BF    0x40 /* don't save bad frames     */
#define PREAMBLE_LENGTH 0x20 /* 8 bytes                   */
#define ADDRESS_LENGTH  0x6
#define NO_SRC_ADDR_INS 0x8  /* tell 82596 not to insert src addr.   */
#define IFS_SPACING     0x60 /* interframe spacing in trasmit clocks */
#define SLOT_TIME_LOW   0    /* slot time, low byte                  */
#define SLOT_TIME_HIGH  2    /* slot time, high bits : 0x200         */
#define MAX_RETRIES     0xf  /* maximum # of transmission retries after collision */
#define MIN_FRAME_LEN   64   /* in bytes, preamble not included                   */

static struct
{
  ulong csc;           /* cmd, status, control fields                      */
  ulong next_cb;       /* link to next cmd block                           */
  uchar byte_count;    /* # of config. bytes to configure (including this) */
  uchar fifo_limit;    /* pt in FIFO to request bus                        */
  uchar save_bf;       /* save bad frames                                  */
  uchar loopback;      /* loopback, src. addr., preamble and addr. length  */
  uchar linear_pri;    /* backoff method, linear and exp. priority         */
  uchar spacing;       /* interframe spacing                               */
  uchar slot_time_low; /* slot time for the network                        */
  uchar slot_time_up;  /* max. retries and upper nibble of slot time       */
  uchar promiscuous;   /* accept all frames                                */
  uchar carrier_sense; /* carrier sense filter                             */
  uchar frame_len;     /* minimum frame length                             */
  uchar preamble;      /* preamble until carrier sense                     */
  uchar dcr_slot;      /* DCR slot number, full duplex operation           */
  uchar dcr_num;       /* # of stations in DCR mode                        */

} cfg = {
          0,
          0,
          BYTE_COUNT,
          DEF_FIFO_LIMIT | DISABLE_MONITOR,
          DONT_SAVE_BF,
          PREAMBLE_LENGTH | NO_SRC_ADDR_INS | ADDRESS_LENGTH,
          0,     /* dis. priorities, start backoff inmediately after jam. */
          IFS_SPACING,
          SLOT_TIME_LOW,
          (MAX_RETRIES << 4) | SLOT_TIME_HIGH,
          0,     /* dis. promisc. mode, enable broadcast, use NRZ encoding,*/
                 /* stop transm. if CS drops, 32-bit CRC, no padding       */
          0,     /* 0 bits CS, external CS, 0 bits CD, external CD         */
          MIN_FRAME_LEN,
          0xdb,  /* dis. monitor and multicast, enable CD by addr. comp.,  */
                 /* enable auto-retransmit, disable CRC transfer,          */
                 /* disable padding, disable preamble until CS             */
          0,     /* DCR disabled */
          0,     /* DCR disabled, dis. mult. indiv. addr., enable backoff  */
        };


int config_ether(uchar_t lb_type)
{
  int rc;

/* if the loopback is at the 82501 or external, set the */
/* FIFO limit to maximum to avoid too many underruns.   */

  cfg.fifo_limit = (lb_type == LB_82501 || lb_type == LB_EXT) ?
                     MAX_FIFO_LIMIT : DEF_FIFO_LIMIT;

  cfg.fifo_limit |= DISABLE_MONITOR;
  cfg.loopback &= 0x3f;
  cfg.loopback |= (lb_type << 6);

  rc = SUCCESS;

#ifdef DEBUG_ETHER
  dump_config();  /* debugging */
#endif

  if(ioctl(get_fd(ETHS_FD), ENT_CFG, &cfg) < 0)
  {
    set_tu_errno();
    incr_stat((uchar_t) BAD_OTHERS, 1);
    rc = ENT_CFG_ERR;
  }

  incr_stat((uchar_t) GOOD_OTHERS, 1);

  return(rc);
}




#ifdef DEBUG_ETHER
dump_config()
{
  char msg[80];

  sprintf(msg, "csc ............ 0x%08x", cfg.csc);
  DEBUG_MSG(msg);
  sprintf(msg, "next_cb ........ 0x%08x", cfg.next_cb);
  DEBUG_MSG(msg);
  sprintf(msg, "byte_count ..... 0x%02x", cfg.byte_count);
  DEBUG_MSG(msg);
  sprintf(msg, "fifo_limit ..... 0x%02x", cfg.fifo_limit);
  DEBUG_MSG(msg);
  sprintf(msg, "save_bf ........ 0x%02x", cfg.save_bf);
  DEBUG_MSG(msg);
  sprintf(msg, "loopback ....... 0x%02x", cfg.loopback);
  DEBUG_MSG(msg);
  sprintf(msg, "linear_pri ..... 0x%02x", cfg.linear_pri);
  DEBUG_MSG(msg);
  sprintf(msg, "spacing ........ 0x%02x", cfg.spacing);
  DEBUG_MSG(msg);
  sprintf(msg, "slot_time_low .. 0x%02x", cfg.slot_time_low);
  DEBUG_MSG(msg);
  sprintf(msg, "slot_time_up ... 0x%02x", cfg.slot_time_up);
  DEBUG_MSG(msg);
  sprintf(msg, "promiscuous .... 0x%02x", cfg.promiscuous);
  DEBUG_MSG(msg);
  sprintf(msg, "carrier_sense .. 0x%02x", cfg.carrier_sense);
  DEBUG_MSG(msg);
  sprintf(msg, "frame_len ...... 0x%02x", cfg.frame_len);
  DEBUG_MSG(msg);
  sprintf(msg, "preamble ....... 0x%02x", cfg.preamble);
  DEBUG_MSG(msg);
  sprintf(msg, "dcr_slot ....... 0x%02x", cfg.dcr_slot);
  DEBUG_MSG(msg);
  sprintf(msg, "dcr_num ........ 0x%02x", cfg.dcr_num);
  DEBUG_MSG(msg);

  return;
}
#endif

