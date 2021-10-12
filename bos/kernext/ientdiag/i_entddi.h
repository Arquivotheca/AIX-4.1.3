/* @(#)53  1.3.1.1  src/bos/kernext/ientdiag/i_entddi.h, diagddient, bos411, 9428A410j 11/10/93 14:55:26 */
/*
 *
 * COMPONENT_NAME: sysxient - Integrated Ethernet Device Driver     	     
 *                                                                          
 * FUNCTIONS: entddi.h                                                     
 *                                                                        
 * ORIGINS: 27                                                           
 *                                                                      
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when               
 * combined with the aggregated modules for this product)             
 *                  SOURCE MATERIALS                                 
 * (C) COPYRIGHT International Business Machines Corp. 1990         
 * All Rights Reserved                                             
 * Licensed Materials - Property of IBM                           
 *                                                                       
 * US Government Users Restricted Rights - Use, duplication or          
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.   
 *                                                                    
 */


typedef struct {
      uchar  lname[4];                   /* Logical name in ASCII characters */
      ulong  bus_mem_addr;               /* bus memory base address          */
      int    bus_mem_size;               /* size of bus memory region        */
      uchar *tcw_bus_mem_addr;           /* tcw bus memory base address      */
      long   tcw_bus_mem_size;           /* size of tcw bus memory region    */
      ulong  io_port;                    /* io port base for PS/2,etc        */
      int    slot;                       /* slot for this adapter            */
      int    dma_arbit_lvl;              /* DMA Arbitration Level            */
      int    bnc_select;                 /* Transceiver Select               */
      int    use_alt_addr;               /* non-zero => use alt network addr */
      uchar  alt_addr[ent_NADR_LENGTH];  /* alternate network address        */
      ushort type_field_off;             /* Adapter Type Field Displacement  */
      ushort net_id_offset;              /* IEEE one byte netid offset       */
      uchar  eth_addr[ent_NADR_LENGTH];  /* Original Eth. address. 	     */
} ddi_ds_section_t;
