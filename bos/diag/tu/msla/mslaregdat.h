/* @(#)43	1.2  src/bos/diag/tu/msla/mslaregdat.h, tu_msla, bos411, 9428A410j 6/15/90 17:23:54 */
/*
 * COMPONENT_NAME: (mslaregdat.h) header file for MSLA diagnostic
 *			application.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* This include file contains M68k codes for MSLA IO register testing */
static unsigned short  msla_count_rout[MSLA_COUNT_ROUT_LEN] = { 
                        0X0000,     /* STACK  DC.L  STACK    */
			0x0000,
			0x0000,     /*        DC.L  BEGIN    */
			0x0008,
			0x4E70,     /* RESET */
			0x41F9,     /* BEGIN  LEA   FLAG,A0  */
			0x0000,
			0x0016,
			0x4290,     /*        CLR.L  (A0)    */
			0x5290,     /* LOOP   ADDQ.L #1,(A0) */
			0x60FC,     /*        BRA    LOOP    */
			0x0000,     /* FLAG   DC.L   0       */
			0x0000,
			0x0000 };
/*********************************************************************/
static unsigned short msla_intrupt_rout1[MSLA_INTRUPT_ROUT1_LEN] = { 0X0000,
						          	   0x8000,
						          	   0x0000,
						          	   0x1000 };

static unsigned short msla_intrupt_rout2[MSLA_INTRUPT_ROUT2_LEN] = { 0X0000,
						          	   0x1034 };
	   
static unsigned short msla_intrupt_rout3[MSLA_INTRUPT_ROUT3_LEN] = { 0X0000,
						          	   0x0000,
						          	   0x0000,
						          	   0x0000 };

static unsigned short msla_intrupt_rout4[MSLA_INTRUPT_ROUT4_LEN] = { 0X13FC,
						          	   0x0000,
						          	   0x00FC,
						          	   0xE090,
						          	   0x13FC,
						          	   0x0000,
						          	   0x0000,
						          	   0xE090,
						          	   0x13FC,
						          	   0x0000,
						          	   0x0000,
						          	   0xE091,
						          	   0x46FC,
						          	   0x2200,
						          	   0x41F8,
						          	   0x0100,
						          	   0x5290,
						          	   0x60F4,
						          	   0x0000,
						          	   0x0000,
						          	   0x0000,
						          	   0x0000,
						          	   0x0000,
						          	   0x0000,
						          	   0x0000,
						          	   0x0000,
						          	   0x43F8,
						          	   0x0104,
						          	   0x5291,
						          	   0x43F9,
						          	   0x00FC,
						          	   0xE002,
						          	   0x12B8,
						          	   0x0000,
						          	   0x4E73,
						          	   0x0000 };
/*********************************************************************/
/*********************************************************************/
/*ROUTINE TO TEST FSLA INTERUPT TO SAILBOAT !!                       */
/*********************************************************************/
static unsigned short rt_intr_data1[RT_DATA1_LEN] = {
	     			0x0000,
	     			0x8000,
	     			0x0000,
	     			0x1000 };
	
static unsigned short rt_intr_data2[RT_DATA2_LEN] = {
	     			0x0000,
	     			0x000F,
	     			0x0000,
	     			0x0000 };
	
	
static unsigned short  rt_intr_routine[RT_DATA3_LEN] = {
	     			0x13FC,
	     			0x0000,
	     			0x00FC,
	     			0xE090,
	     			0x13FC,
	     			0x0000,
	     			0x0000,
	     			0xE090,
	     			0x13FC,
	     			0x0000,
	     			0x00FC,
	     			0xE091,
	     			0x41F9,
	     			0x00FC,
	     			0xE000,
	     			0x10B8,
	     			0x0000,
	     			0x303C,
	     			0x7FFF,
	     			0x56C8,
	     			0xFFFE,
	     			0x43F8,
	     			0x0100,
	     			0x5391,
	     			0x66E6,
	     			0x60FE,
	     			0x0000,
	     			0x0000 };
