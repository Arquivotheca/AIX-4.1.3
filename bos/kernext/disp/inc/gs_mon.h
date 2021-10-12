/* @(#)50	1.2  src/bos/kernext/disp/inc/gs_mon.h, dispcfg, bos411, 9428A410j 7/5/94 11:32:39 */

/*
 *
 * COMPONENT_NAME: (dispcfg) Display Configuration
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_GS_MON
#define _H_GS_MON

#define GS_MON_ID_DIP_SWITCHES_SHIFT		16
#define GS_MON_ID_DIP_SWITCHES_MASK		\
		(0xF << GS_MON_ID_DIP_SWITCHES_SHIFT)

#define GS_MON_ID_CABLE_ID_0			0x0
#define GS_MON_ID_CABLE_ID_H			0x1
#define GS_MON_ID_CABLE_ID_V			0x2
#define GS_MON_ID_CABLE_ID_1			0x3

#define GS_MON_ID_CABLE_BIT_0_SHIFT		6
#define GS_MON_ID_CABLE_BIT_0_MASK		\
		(0x3 << GS_MON_ID_CABLE_BIT_0_SHIFT)
#define GS_MON_ID_CABLE_BIT_1_SHIFT		4
#define GS_MON_ID_CABLE_BIT_1_MASK		\
		(0x3 << GS_MON_ID_CABLE_BIT_1_SHIFT)
#define GS_MON_ID_CABLE_BIT_2_SHIFT		2
#define GS_MON_ID_CABLE_BIT_2_MASK		\
		(0x3 << GS_MON_ID_CABLE_BIT_2_SHIFT)
#define GS_MON_ID_CABLE_BIT_3_SHIFT		0
#define GS_MON_ID_CABLE_BIT_3_MASK		\
		(0x3 << GS_MON_ID_CABLE_BIT_3_SHIFT)

#endif /* ! _H_GS_MON */

