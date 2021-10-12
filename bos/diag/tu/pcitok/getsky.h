/* @(#)05	1.1  src/bos/diag/tu/pcitok/getsky.h, tu_pcitok, bos41J, 9512A_all 3/21/95 17:00:48  */
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: none
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


  typedef struct {
    char                  adpt_name[16];  /* logical name "lname", eg. tok0 */
    char		  alias[16];      /* "alias", eg. tr0               */
    char                  par_name[16];   /* logical device name            */
    unsigned int          slot_num;       /* slot number of adapter         */
    unsigned int          bus_intr_lvl;   /* interrupt level "intr_level"   */
    unsigned int          intr_priority;  /* interrupt priority             */
    unsigned int          dma_chan_id;
    unsigned long         bus_io_addr;    /* I/O base "io_addr"             */
    unsigned long         bus_io_length;
    unsigned long         dma_bus_mem;    /* DMA base                       */
    unsigned long         dma_bus_length;
    unsigned int          dma_flags;      /* not used                       */
    unsigned long         bus_id;         /* "bus_id"                       */
    unsigned long         xmt_que_size;   /* "xmt_que_size"                 */
    unsigned long         rcv_que_size;   /* "rcv_que_size"                 */
    unsigned long         ring_speed;     /* "ring_speed"                   */
    char                  alt_addr[6];    /* "alt_addr"                     */
    char                  intr_func[80];  /* not used                       */
    char                  attn_mac[4];    /* "attn_mac"                     */
    char                  beacon_mac[4];  /* "beacon_mac"                   */
    char                  use_alt_addr[4];/* "use_alt_addr"                 */
  } mps_dds;


