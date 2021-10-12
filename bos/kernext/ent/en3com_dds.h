/* @(#)33  1.3  src/bos/kernext/ent/en3com_dds.h, sysxent, bos411, 9428A410j 1/28/94 14:57:32 */
/*
 * COMPONENT_NAME: sysxent --  High Performance Ethernet Device Driver
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_EN3COM_DDS
#define _H_EN3COM_DDS

#define MAX_3COM_VPD_LEN	128	/* max vpd size for 3com adapter */

typedef struct {	
      int   bus_type;       /* for use with i_init                           */
      int   bus_id;         /* for use with i_init and bus io                */
      int   intr_level;     /* for use with i_init                           */
      int   intr_priority;  /* interruct priority, not used                  */
      int   xmt_que_size;   /* one queue per device for transmit buffering   */

      uchar  lname[ERR_NAMESIZE];        /* logical name in ASCII characters */
      uchar  alias[ERR_NAMESIZE];        /* alias in ASCII characters */
      ulong  bus_mem_addr;               /* bus memory base address          */
      int    bus_mem_size;               /* size of bus memory region        */
      uchar *tcw_bus_mem_addr;           /* tcw bus memory base address      */
      long   tcw_bus_mem_size;           /* size of tcw bus memory region    */
      ulong  io_port;                    /* io port base for PS/2,etc        */
      int    slot;                       /* slot for this adapter            */
      int    dma_arbit_lvl;              /* DMA Arbitration Level            */
      int    use_alt_addr;               /* non-zero => use alt network addr */
      uchar  alt_addr[ENT_NADR_LENGTH];  /* alternate network address        */
      int    rv_pool_size;		 /* no. of buffers in hw receive pool*/
      int    bnc_select;                 /* transceiver select               */
} en3com_dds_t;


typedef struct en3com_vpd {  
   ulong status;             		/* vpd status value       */
   ulong length;             		/* number of bytes actually returned */
   uchar vpd[MAX_3COM_VPD_LEN];		/* vital product data characters     */
} en3com_vpd_t;

/*
 * vpd status codes 
 */
#define VPD_NOT_READ   (0) /* the vpd data has not been obtained from adap   */
#define VPD_NOT_AVAIL  (1) /* the vpd data is not available for this adapter */
#define VPD_INVALID    (2) /* the vpd data was obtained but is invalid       */
#define VPD_VALID      (3) /* the vpd data was obtained and is valid         */

#endif /* _H_EN3COM_DDS */
