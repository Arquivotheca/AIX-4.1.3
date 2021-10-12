/* @(#)37       1.3  src/bos/kernext/mps_tok/mps_dds.h, sysxmps, bos411, 9432A411a 8/4/94 21:53:52 */
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_MPS_DDS
#define _H_MPS_DDS

#define MAX_MPS_VPD_LEN         256  /* max vpd size for mps adapter          */

/*
 *      Wildwood DDS structure
 */
typedef struct {                     /* common ddi for all the LAN devices    */
   int   bus_type;                   /* for use with i_init                   */
   int   bus_id;                     /* for use with i_init                   */
   int   intr_level;                 /* for use with i_init                   */
   int   intr_priority;              /* for use with i_init                   */
   int   xmt_que_size;               /* one queue per device for Tx buffering */

   uchar  dev_name[16];              /* logical name of device               */
   uchar  alias[16];                 /* alias  name of device                */
   uint   slot;                      /* card slot number                     */
   ulong  io_base_addr;              /* PIO bus address                      */
   ulong  mem_base_addr;             /* bus memory base address              */
   uint   mem_bus_size;              /* length of bus memory addr space      */
   uint   dma_lvl;                   /* DMA arbitration level                */
   ulong  dma_base_addr;             /* DMA base address                     */
   uint   dma_bus_length;            /* length of DMA address space          */

   uchar  ring_speed;                /* Ring Speed: 0=4Mb,1=16Mb,2=autosence */
                                     /* if autosence is selected then adapter 
				      * will set the ring speed according
        		              *	to the network ring speed.
				      */
   int    use_alt_addr;              /* non-zero => use alt network addr     */
   uchar  alt_addr[CTOK_NADR_LENGTH];/* alternate network address            */
   uchar  attn_mac;                  /* yes, no value for attention frames   */
   uchar  beacon_mac;                /* yes, no value for beacon frames      */
   uchar  priority_tx;               /* yes, no value for priority Tx chnl   */
} mps_dds_t;

typedef struct mps_vpd {
   ulong status;                     /* vpd status value                     */
   ulong length;                     /* number of bytes actually returned    */
   uchar vpd[MAX_MPS_VPD_LEN];       /* vital product data characters        */
} mps_vpd_t;

/*
 * vpd status codes
 */
#define VPD_NOT_READ   (0) /* the vpd data has not been obtained from adap   */
#define VPD_NOT_AVAIL  (1) /* the vpd data is not available for this adapter */
#define VPD_INVALID    (2) /* the vpd data was obtained but is invalid       */
#define VPD_VALID      (3) /* the vpd data was obtained and is valid         */

#endif /* _H_MPS_DDS */
