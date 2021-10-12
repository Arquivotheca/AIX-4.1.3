/* @(#)38 1.10 11/29/90 20:52:11 */
#ifndef _H_ENTDDI
#define _H_ENTDDI

/*****************************************************************************/
/*                                                                           */
/* COMPONENT_NAME: sysxent -- Ethernet Communications Code Device Driver     */
/*                                                                           */
/* FUNCTIONS: entddi.h                                                       */
/*                                                                           */
/* ORIGINS: 27                                                               */
/*                                                                           */
/* (C) COPYRIGHT International Business Machines Corp. 1990                  */
/* All Rights Reserved                                                       */
/* Licensed Materials - Property of IBM                                      */
/*                                                                           */
/* US Government Users Restricted Rights - Use, duplication or               */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*
 * NAME: entddi.h
 *
 * FUNCTION: see discussion below
 *
 * NOTES:
 * The include files that make up the communications device drivers which
 * use the ciodd.c common code have the following heirarchy (where XXX is
 * the device-specific name such as tok or ent):
 *    comio.h   -- standard communications i/o subsystem declarations (Vol3Ch5)
 *                 needed by ciodd.c and XXXds.c, also needed by users
 *    XXXuser.h -- device-specific declarations (Vol3Ch6)
 *                 needed by ciodd.c and XXXds.c, also needed by users
 *    XXXddi.h  -- device specific part of DDI (defines used by cioddi.h)
 *    cioddi.h  -- common part of DDI definition that is part of DDS
 *                 needed by ciodd.c, XXXds.c, XXXconf.c, not needed by users
 *    cioddhi.h -- high-level independent common declarations
 *                 needed by ciodd.c, XXXds.c, ciodds.h, not needed by users
 *    XXXdshi.h -- high-level independent device-specific declarations
 *                 needed by ciodds.h, ciodd.c, XXXds.c, not needed by users
 *    ciodds.h  -- common part of DDS which depends on all preceding includes
 *                 needed by ciodd.c and XXXds.c, not needed by users
 *    cioddlo.h -- low-level common declarations that may depend on the DDS
 *                 needed by ciodd.c and XXXds.c, not needed by users
 *    XXXdslo.h -- low-level device-specific declarations
 *                 needed by XXXds.c ONLY (not needed by ciodd.c or by users)
 */

/*****************************************************************************/


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
} ddi_ds_section_t;

#endif /* ! _H_ENTDDI */
