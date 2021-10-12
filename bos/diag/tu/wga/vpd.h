/* @(#)81       1.1  src/bos/diag/tu/wga/vpd.h, tu_wga, bos411, 9428A410j 2/9/93 09:56:33 */
/*
 *   COMPONENT_NAME: TU_WGA
 *
 *   FUNCTIONS: includes needed in WGA Test Units
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



/*****************************************************************************/
/*******************       VPD & ROS  Registers        ***********************/
/*****************************************************************************/

#define EC_LEVEL_MASKS              0x07         /* EC level mask, bits 29-31*/
#define WGA_EC_LEVEL                0x00         /* EC level - 1nd release   */

#define  MACHINE_MODEL_220          220          /* SGA machine model        */
#define  MACHINE_MODEL_230          230          /* WGA machine model        */
#define  DEFAULT_MACHINE_MODEL      MACHINE_MODEL_230


/* ------------------------------------------------------------------------- */
/* Function prototypes                                                       */
/* ------------------------------------------------------------------------- */

extern int get_machine_model (void);
extern void set_machine_model (int);
