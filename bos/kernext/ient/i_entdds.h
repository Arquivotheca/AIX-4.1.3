/* @(#)38  1.5  src/bos/kernext/ient/i_entdds.h, sysxient, bos411, 9428A410j 2/28/94 15:25:27 */
/****************************************************************************/
/*
 *   COMPONENT_NAME: SYSXIENT
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************/
#ifndef _H_IENT_DDS
#define _H_IENT_DDS


/*
 *  common code section of the ddi/dds
 */
#define MAX_IENT_VPD_LEN        249             /* number of VPD bytes */

typedef struct {
        int   bus_type;                 /* for use with i_init */
        int   bus_id;                   /* for use with d_init */
        int   intr_level;               /* for use with i_init */
        int   intr_priority;            /* for use with i_init */
        int   xmt_que_size;             /* transmit que size */

        uchar  lname[ERR_NAMESIZE];      /* Logical name in ASCII characters */
        uchar  alias[ERR_NAMESIZE];      /* alias name in ASCII characters   */
        ulong  bus_mem_addr;             /* bus memory base address          */
        uchar *tcw_bus_mem_addr;         /* tcw bus memory base address      */
        long   tcw_bus_mem_size;         /* size of tcw bus memory region    */
        ulong  io_port;                  /* io port base for PS/2,etc        */
        int    slot;                     /* slot for this adapter            */
        int    dma_arbit_lvl;            /* DMA Arbitration Level            */
        int    use_alt_addr;             /* non-zero => use alt network addr */
        uchar  alt_addr[ENT_NADR_LENGTH];/* alternate network address        */
      uchar  eth_addr[ENT_NADR_LENGTH];  /* Original Eth. address.           */
} ient_dds_t;

#endif /* _H_IENT_DDS */
