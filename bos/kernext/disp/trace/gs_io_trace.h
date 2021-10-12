/* @(#)71	1.4  src/bos/kernext/disp/trace/gs_io_trace.h, sysxdisp, bos411, 9433B411a 8/15/94 15:00:23 */

/*
 * COMPONENT_NAME: (sysxdisp) Display Sub-System
 *
 * FUNCTIONS:  None
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************
*******************************************************************************
**                                                                           **
** Contains the low-level I/O system trace definitions.			     **
**                                                                           **
*******************************************************************************
******************************************************************************/

/* This file houses the low-level I/O system trace interface */

#ifndef _H_GS_IO_TRACE
#define _H_GS_IO_TRACE

#ifndef GS_IO_TRACE

#define GS_IO_TRC(format, address, data)

#else /* GS_IO_TRACE */

#include <graphics/gs_trace.h>

#define hkwd_GS_GS_IO_TRC_AP_LEVEL_RASTERIZER_REG	(0x001)
#define hkwd_GS_GS_IO_TRC_DD_LEVEL_RASTERIZER_REG	(0x002)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_RAMDAC_ADDR		(0x003)
#define hkwd_GS_GS_IO_TRC_DD_LEVEL_RAMDAC_ADDR		(0x004)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_RAMDAC_REG		(0x005)
#define hkwd_GS_GS_IO_TRC_DD_LEVEL_RAMDAC_REG		(0x006)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_RAMDAC_LUT		(0x007)
#define hkwd_GS_GS_IO_TRC_DD_LEVEL_RAMDAC_LUT		(0x008)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_DFA_4		(0x009)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_DFA_2		(0x00A)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_DFA_1		(0x00B)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_BD_ADDR		(0x00C)
#define hkwd_GS_GS_IO_TRC_AP_LEVEL_BD_REG		(0x00D)
#define hkwd_GS_GS_IO_TRC_DD_LEVEL_BD_ADDR		(0x00E)
#define hkwd_GS_GS_IO_TRC_DD_LEVEL_BD_REG		(0x00F)
#define hkwd_GS_GS_IO_TRC_DD_LEVEL_CFG_REG		(0x010)

#define GS_IO_TRC(format, address, data)			\
	TRCHKGT (((HKWD_GS_GC_IO << 20) | hkwd_GS_ ## format),	\
		(address), (data), 0, 0, 0);

#endif /* GS_IO_TRACE */

#endif /* _H_GS_IO_TRACE */

