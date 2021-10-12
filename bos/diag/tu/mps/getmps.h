/* @(#)36       1.1  src/bos/diag/tu/mps/getmps.h, tu_mps, bos411, 9437B411a 8/23/94 16:26:59 */
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: MPS Test Unit Header File
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/************************************************************************
*      Define MPS dds Structure                                         *
*      This is the minimum set required for any adapter                  *
*      If you want to add more you can add at the end of the present     *
*      structure. This is what you get back when you do diag_query       *
 ************************************************************************/
typedef struct {
   char                  adpt_name[16];  /* logical adpater name */
   char                  par_name[16];    /* logical device name */
   unsigned int          slot_num;       /* slot number of adapter */
   unsigned int          bus_intr_lvl;   /* interrupt level */
   unsigned int          intr_priority;  /* interrupt priority */
   unsigned int          intr_flags;     /* look at intr.h */
   unsigned int          dma_lvl;        /* this is the bus arbitration level */
   unsigned int          dma_chan_id;
   unsigned long         bus_io_addr;    /* base of Bus I/O area for this */
   unsigned long         bus_io_length;
   unsigned long         bus_mem_addr;   /* base of Bus Memory "Shared" */
   unsigned long         bus_mem_length;
   unsigned long         dma_bus_mem;   /* base of Bus Memory DMA */
   unsigned long         dma_bus_length;
   unsigned int          dma_flags;     /* look at dma.h */
   unsigned long         bus_id;
   unsigned long         bus_type;
   unsigned int          ver_num;        /* Version Number */
   unsigned long         xmt_que_size;
   unsigned long         ring_speed;
   char                  alt_addr[6];
   char                  intr_func[80];  /* path of the interrupt function */
   char                  attn_mac[4];
   char                  beacon_mac[4];
   char                  use_alt_addr[4];
} mps_dds;

