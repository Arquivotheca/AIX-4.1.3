/* @(#)49  1.3.1.1  src/bos/kernext/ientdiag/i_cioddi.h, diagddient, bos411, 9428A410j 11/10/93 14:09:24 */
#ifndef _H_CIODDI
#define _H_CIODDI

/*
 * COMPONENT_NAME: sysxient -- Device Driver for the native Ethernet controller
 *
 * FUNCTIONS: cioddi.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* 
 *  common code section of the ddi/dds 
 */
typedef struct {    
	int   bus_type;			/* for use with i_init */
	int   bus_id;         		/* for use with d_init */
	int   intr_level;     		/* for use with i_init */
	int   intr_priority;  		/* for use with i_init */
	int   xmt_que_size;   		/* transmit que size */
	int   rec_que_size;   		/* 1 for each open from a user proc */
	int   sta_que_size;   		/* 1 for each open from a user process*/
	int   rdto;           		/* received data transfer offset */
} ddi_cc_section_t;


/*
 *  DDI section of the DDS
 */
typedef struct {      
	ddi_cc_section_t cc; 		/* common section */
	ddi_ds_section_t ds; 		/* device-specific section XXXddi.h */
} ddi_t;

/*
 *  Macros for accessing DDI contents
 *  requires that the ddi pointer be named ddi_ptr
 *  example: to access the bus id in the ddi, use "DDI_CC.bus_id"
 */

#define DDI_CC ddi_ptr->cc
#define DDI_DS ddi_ptr->ds

#endif /* ! _H_CIODDI */
