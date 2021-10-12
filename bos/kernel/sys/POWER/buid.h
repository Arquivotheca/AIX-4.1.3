/* @(#)44	1.3  src/bos/kernel/sys/POWER/buid.h, sysios, bos411, 9428A410j 3/12/93 18:38:26 */
#ifndef _H_BUID
#define _H_BUID
/*
 * COMPONENT_NAME: (SYSIOS) BUID definitions
 *
 * FUNCTIONS: Machine-dependent Bus Unit ID definitions
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define SCU_BID		0x80000000	/* for io access to scu bus	*/

/* defined buids
 */
#define SCU_BUID	0x00		/* scu buss			*/

#define IOCC0_BUID	0x20		/* reserved value for IOCCs	*/
#define IOCC1_BUID	0x21
#define IOCC2_BUID	0x22
#define IOCC3_BUID	0x23

#define SGA_BUID	0x40		/* salmon grafics adpater	*/

#define MSLA0_BUID	0x80		/* sla adapters			*/
#define MSLA1_BUID	0x81
#define MSLA2_BUID	0x82
#define MSLA3_BUID	0x83
#define MSLA4_BUID	0x84
#define MSLA5_BUID	0x85
#define MSLA6_BUID	0x86
#define MSLA7_BUID	0x87

/* BID_TO_BUID(int bid)
 * Convert a BID to BUID
 */
#define BID_TO_BUID(bid) (((bid) & 0x1FF00000) >> 20)


#endif /* _H_BUID */







