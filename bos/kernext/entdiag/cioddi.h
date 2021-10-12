/* @(#)95	1.1  src/bos/kernext/entdiag/cioddi.h, diagddent, bos411, 9428A410j 11/29/90 21:10:57 */
#ifndef _H_CIODDI
#define _H_CIODDI

/*
 * COMPONENT_NAME: sysxcio -- Common Communications Code Device Driver Head
 *
 * FUNCTIONS: cioddi.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************/
   typedef struct {         /* common code section of the ddi/dds (r/o to dd)*/
      int   bus_type;       /* for use with i_init                           */
      int   bus_id;         /* for use with i_init                           */
      int   intr_level;     /* for use with i_init                           */
      int   intr_priority;  /* for use with i_init                           */
      int   xmt_que_size;   /* one queue for the adapter shared by all opens */
      int   rec_que_size;   /* one for each open from a user process         */
      int   sta_que_size;   /* one for each open from a user process         */
      int   rdto;           /* received data transfer offset                 */
   } ddi_cc_section_t;

/*****************************************************************************/
   typedef struct {        /* the complete DDI which is also part of the DDS */
      ddi_cc_section_t cc; /* common section (defined above)                 */
      ddi_ds_section_t ds; /* device-specific section (defined in XXXddi.h)  */
   } ddi_t;

/*
 *  Macros for accessing DDI contents
 *  requires that the ddi pointer be named ddi_ptr
 *  example: to access the bus id in the ddi, use "DDI_CC.bus_id"
 */

#define DDI_CC ddi_ptr->cc
#define DDI_DS ddi_ptr->ds

#endif /* ! _H_CIODDI */
