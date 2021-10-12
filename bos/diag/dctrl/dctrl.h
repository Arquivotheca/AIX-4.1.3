/* @(#)52	1.1.1.3  src/bos/diag/dctrl/dctrl.h, dctrl, bos411, 9438C411a 9/23/94 11:13:29 */
/*
 *   COMPONENT_NAME: DCTRL
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _h_dctrl
#define _h_dctrl

/* Source code and reason codes for controller */
/* .......diagnostics did not detect installed resource....... */
#define DC_SOURCE_NEW	0x801
#define DC_SOURCE_MISS	0x802
/* .......software error occurred while running diagnostics... */
#define DC_SOURCE_SOFT	0x803
/* .......adapter not found................................... */
#define DC_RCODE_801_M	0x101
/* .......adapter not found after being reconfigured.......... */
#define DC_RCODE_801_D	0x102
/* .......MORPS generated sn for scsi devices and disk arrays..*/
#define DC_SOURCE_MORPS1 0x950
#define DC_SOURCE_MORPS2 0x844

/* Menu number definitions */
#define DIAG_MODE_MENU	0x801003	/* diagnostic mode selection menu */
#define TEST_METHOD   	0x801004	/* test method selection menu     */
#define DIAG_SEL_ADV  	0x801005	/* diagnostic selection menu (adv)*/
#define DIAG_SEL_CUST 	0x801006	/* diagnostic selection menu (cus)*/
#define SYS_CHECKOUT  	0x801007	/* system checkout                */
#define PROMPT_DISK	0x801008	/* prompt for next diskette	  */

#define TEST_COMPLETE 	0x801010	/* testing complete NTF  menu     */
#define ADD_RESOURCE_1	0x801011	/* additional resources needed #1 */
#define ADD_RESOURCE_2	0x801012	/* additional resources needed #2 */
#define ADD_RESOURCE_3	0x801013	/* additional resources needed #3 */
#define PROB_REPORT   	0x801014	/* problem report menu            */

#define MISS_RESOURCE 	0x801020	/* missing resource menu          */
#define MISS_REPLACED 	0x801021	/* missing resource replaced      */

#define NEW_RESOURCE_1 	0x801030	/* new resource menu #1           */
#define NEW_RESOURCE_2	0x801031	/* new resource menu #2           */

/* NVRAM address and value definitions */
#define LED_ADDR	0x00A00320	/* address to insert LED values	   */
#define OCS_ADDR	0x00A0037C	/* address to activate LED's       */
#define OCS_VALUE	0x01000000	/* value to activate LED's         */
#define OCS_LEN		0x04		/* # of bytes in OCS value	   */
#define OCS_ADDR_SAL	0x00A00315	/* similar address, value and	   */
#define OCS_VALUE_SAL	0xFF032200	/* length for SALMON machine 	   */
#define OCS_LEN_SAL	0x03		/* type.			   */ 
#define OCS_RESET	0x00000000	/* value to de-activate LED's      */

#define RSC_MODEL	0x02000000	/* Flag for SALMON machine type	   */

#endif
