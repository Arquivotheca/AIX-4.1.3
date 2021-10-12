/* @(#)14       1.8  src/bos/kernext/disp/wga/inc/wga_regval.h, wgadd, bos411, 9428A410j 4/30/93 13:06:10 */

/*
 *   COMPONENT_NAME: WGADD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
* COMPONENT NAME:    White Oak Device Driver include file
*
* FUNCTIONS:         KSR device driver include file 
*
* ORGINS:            27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with aggregated modules for this product)
*                  MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1991
* All Rights Reserved
*
* US Government Users  Restricted Rights - Use, duplication or 
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/


/****START OF SPECIFICATIONS *******************************************
 *    DESCRIPTIVE NAME:  White Oak Graphics Adaptor Register Values    *
 *                                                                     *
 *    FUNCTION:  Define register offsets and values                    *
 *                                                                     *
 *    ATTRIBUTES:                                                      *
 *    REFERENCES:  White Oak Graphics Adaptor Specification            *
 ***********************************************************************
 *                                                                     *
 ****END   OF SPECIFICATIONS ******************************************/




/***************************************************************************/
/********************** BUID 40 Register VALUES  ***************************/
/***************************************************************************/






/***************************************************************************/
/******************* VPD Register Values ***********************************/
/***************************************************************************/

/*--------------  Vital Product Data Registers are  -----------------------*/
/*--------------------------- READ ONLY -----------------------------------*/


/***************************************************************************/
/******************* XYADDR Register Values ********************************/
/***************************************************************************/
#define YSHIFT          0x2000     /* shifts y value for XYADDR register*/
#define XSHIFT          0x4        /* shifts y value for XYADDR register*/
#define PART_W          0x04000000 /* write partial word mode in XYADDR reg*/
#define FULL_W          0x05000000 /* write fullword mode in XYADDR reg*/
#define PART_R          0x06000000 /* read partial in XYADDR register*/
#define FULL_R          0x05000000 /* read fullword mode in XYADDR register*/
#define PEL1_R          0x04000000 /* read 1 pel in XYADDR register*/
#define FULL            0x05000000 /* read/write fullword  in XYADDR reg*/
/* NOTE: for PART_R, length should be put in the RDLEN register */



/* WEITEK setup values */

#define SYS_CONFIG_DATA  0x700000
#define INTR_ENABLE_DATA 0x80
#define RF_PERIOD_DATA   0x186
#define RL_MAX_DATA   	 0xFA
#define MEM_CONFIG_3     0x02

/* Brooktree indirect values */

#define IND_DATA_1   	 	0x02
#define IND_DATA_2   	 	0x01
#define IND_DATA_3   	 	0x03

/* Brooktree setup values */

#define REG_0_DATA   	 	0x40
#define REG_1_DATA   	 	0x0
#define REG_2_DATA   	 	0x00
#define PXL_READ_MSK_DATA	0xFF
#define DUMMY_DATA		0x0
#define PXL_BLINK_MSK_DATA	0x0
#define OVRLY_READ_MSK_DATA	0x0
#define OVRLY_BLINK_MSK_DATA	0x0
#define INTERLEAVE_DATA	 	0x0
#define CRSR_CMMND_DATA   	0x0


/***************************************************************************/
/************** DISPLAY CONTROLLER REGISTER VALUES  ************************/
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*--------------  CRTC (CTR Controller Regs) (see define suffixes)---------*/
/*-------------------------------------------------------------------------*/
#define SRTCTL_MTYPE_ALL  0x1F5

/* 
 * WGA Adapter Control Register
 */

/*  Bits 8   -----------------------------------------------------------*/
#define HSYNC_MINUS    0x00000000   /* Horizontal sync is MINUS active     */
#define HSYNC_PLUS     0x00800000   /* Horizontal sync is POSITIVE active  */
/*  Bits 9   --------------------------------------------------------------*/
#define VSYNC_MINUS    0x00000000   /* Vertical sync is MINUS active       */
#define VSYNC_PLUS     0x00400000   /* Vertical sync is POSITIVE active    */
/*  Bits 24-28 ------------------------------------------------------------*/




/*-------------------------------------------------------------------------*/
/*---------  Adapter Status Register Values  (ADSTAT/adstat)  -------------*/
/*-------------------------------------------------------------------------*/
/*  Bits 0-3 --------------------------------------------------------------*/
#define STAT_ERR_MASK  0xF0000000 /*Use to Mask off non-err portion of Status */
#define NO_ERR         0x00000000 /* No Error Reported by Status Register    */
#define PARITY_ERR     0x10000000 /* Parity Error on RSC addr/data bus       */
#define INVAL_CMD      0x20000000 /* Invalid Command Error on RSC bus        */
#define RESV_ACCESS    0x40000000 /* Attempt to Access Reserved Register Error*/
#define BAD_STRING_OP  0x60000000 /* Illegal String Operation Error          */
#define INVAL_VALUE    0x70000000 /* Invalid Value put in Control Register   */
/*  Bits 4   --------------------------------------------------------------*/
#define V_INTR_SET     0x08000000 /* Vertical Interrupt was Reported         */
/*  Bits 5   --------------------------------------------------------------*/
#define V_BLANK_SET    0x04000000 /* Vertical Blanking is now in progress    */
/*  Bits 6-9   ---------------RESERVED-------------------------------------*/


/*
 * Define the Monitor id values for each supported monitor
 */


#define MT_0		0xf000f		/*  No Monitor Attached - use default values     	*/

#define MT_1		0x5000f		/* 1152x900, 76Hz, 105.5MHz	*/
					/* Sun				*/
#define MT_2		0x4000f		/* 1152x900, 66Hz, 93MHz.  	*/

#define MT_3		0xf010a		/* 1024x 768, 70Hz, 78 MHz.	*/
					/* IBM - 8517			*/

#define MT_4     	0xd0004		/* 1280x1024, 77Hz, 148MHz	*/
					/* Spruce - 19"			*/
					/* IBM 6091 - 16"		*/


#define MT_5		0xf0004		/* 1280x1024, 60Hz, 112MHz 	*/
					/* IBM 6091 - 16"		*/

#define MT_6		0xf0007		/* 1280x1024, 67Hz, 128MHz 	*/
					/* IBM 8508 - Mono		*/

#define MT_7		0xe0004		/* 1280x1024, 67Hz, 120MHz 	*/
					/* IBM 6091 - 19"		*/

#define MT_8		0x70004		/* 1280x1024, 60Hz, 108MHz 	*/
					/* Mitsubishi HL/FL-6615	*/

#define MT_9		0x60004		/* 1280x1024, 74Hz, 135MHz 	*/
					/* NEC				*/

#define MT_10		0x30004		/* 1024x768, 60Hz, 64MHz 	*/
					/* Idek MF5117 			*/

#define MT_11		0x20004		/* 1024x768, 70Hz, 75MHz 	*/
					/* NEC 4D and 5D		*/

#define MT_12		0x10004		/* 1024x768, 76Hz, 86MHz 	*/
					/* Vendor			*/

#define MT_13		0xf020f		/* 1280x1024, 72Hz, 128MHz 	*/
					/* Sugarloaf			*/

#define MT_14		0xb0004		/* 1024x768, 76Hz, 86MHz 	*/
                                        /* IBM 6091-16"                 */
 
#define MT_15           0x2000a         /* 1024x768, 70Hz, 78 MHz.      */
					/* IBM 6314                     */

#define MT_16           0xb000a         /* 1024x768, 76Hz, 86MHz        */
					/* IBM 6319                     */

#define MT_17           0xa000a         /* 1280x1024, 60Hz, 112MHz      */
					/* IBM 6317                     */

#define MT_18           0xd000a         /* 1280x1024, 77Hz, 148MHz      */
					/* Galaxy                       */

#define MT_19           0xf000a         /* 1024x768, 60Hz, 64MHz        */
					/* Galaxy X                     */

#define MT_END 0

/*
 * Setup values for "magic" cursor position from wga spec.
 */

#define X_MAGIC_FACTOR_1	52
#define X_MAGIC_FACTOR_2	4
#define Y_MAGIC_FACTOR_1	32
