/* @(#)83	1.3  src/bos/usr/lib/methods/common/artic.h, artic, bos411, 9428A410j 8/31/93 13:51:17 */
/*---------------------------------------------------------------------------
 *
 * COMPONENT_NAME: ARTIC
 *
 * FUNCTIONS: artic.h - ARTIC device driver internal header file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *---------------------------------------------------------------------------
 */
/*
 *      Include file for ARTIC Adapter Support
 *
 */

/*
 *	ARTIC POWER ON SELF TEST DEFINITION 
 */

#define ROSREADY	0x40		/* ROS Ready Bit, INITREG1 */

/*
 *	I/O Register Offsets from start of I/O Memory or DREG Value  
 */

#define PTRREG		0x02		/* Pointer Register     (PTRREG) */
#define DREG		0x03		/* Data Register          (DREG) */
#define INITREG1	0x10		/* Init. Register 1   (INITREG1) */

/*
 *	adapter offset for EIB value
 */

#define EIB_LOCATION (0x416)
#define MP2_PORT_LOCATION (0x456)
#define MP2_PORT_MASK (0x07)

/*
 *	EIB values for ARTIC daughter boards
 */

#define NOBOARD     0xff        

/*
 *	X.25 CoProcessor/2 adapter (C2X)
 */

#define X25_C2X               0xc9
#define X25_C2X_MSG            "1"

/*
 *	Multiport/2 Adapter and Message Numbers from devices.msg
 */

#define MP_GENERIC_MSG       "180"
#define MP_4O8PORT_R232       0xc7
#define MP_6PORT_SYNC         0xcf
#define MP_6PORT_SYNC_MSG    "183"
#define MP_8PORT_R422         0xbe
#define MP_8PORT_R422_MSG    "185" 
#define MP_8PORT_R232_R422    0xc8
#define MP_8PORT_R232_R422_MSG "186"

/*
 *	Multiport/2 Adapter port values and Message Numbers from devices.msg
 */

#define MP2_4PORT_R232          0x01
#define MP2_4PORT_R232_MSG     "182"
#define MP2_6PORT_R232          0x03
#define MP2_8PORT_R232          0x07
#define MP2_8PORT_R232_MSG     "184"

/*
 *	Multiprotocol Controller (Portmaster) Adapter and Message Numbers
 *	from devices.msg
 */

#define PM_4PORT_MPQP         0xbf
#define PM_4PORT_MPQP_MSG      "1"
#define PM_GENERIC_MSG        "10"
#define PM_6PORT_V35          0x18
#define PM_6PORT_V35_MSG      "11"
#define PM_6PORT_X21          0x28
#define PM_6PORT_X21_MSG      "12"
#define PM_8PORT_R232         0x10
#define PM_8PORT_R232_MSG     "13"
#define PM_8PORT_R422         0x21
#define PM_8PORT_R422_MSG     "14"
