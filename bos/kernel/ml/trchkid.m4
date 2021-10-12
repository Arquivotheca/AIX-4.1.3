# @(#)87      1.19  src/bos/kernel/ml/trchkid.m4, sysml, bos411, 9428A410j 8/31/93 07:39:10
#
#   COMPONENT_NAME: SYSML
#
#   FUNCTIONS: none
#
#   ORIGINS: 27 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************
#
#   LEVEL 1,  5 Years Bull Confidential Information
#
#****************************************************************************

	.set HKID_MASK, 0xFFF00000
	.set HKTY_XMASK,0xFFF0FFFF
	.set HKTY_S,    0x00010000  # just hookword
	.set HKTY_ST,   0x00090000  # hookword & timestamp
	.set HKTY_L,    0x00020000  # hookword, dataword
	.set HKTY_LT,   0x000A0000  # hookword, dataword & timestamp
	.set HKTY_GT,   0x000E0000  # hookword, 5 datawords & timestamp

	.set	HKWD_TRACE               , 0x00000000
	.set	HKWD_TRACE_SYNC          , 0x00000000
	.set	HKWD_TRACE_TRCON         , 0x00100000
	.set	HKWD_TRACE_TRCOFF        , 0x00200000
	.set	HKWD_TRACE_HEADER        , 0x00300000
	.set	HKWD_TRACE_NULL          , 0x00400000
	.set	HKWD_TRACE_LWRAP         , 0x00500000
	.set	HKWD_TRACE_TWRAP         , 0x00600000
	.set	HKWD_TRACE_UNDEFINED     , 0x00700000
	.set	HKWD_TRACE_DEFAULT       , 0x00800000
	.set	HKWD_TRACE_CALIB         , 0x00900000

	.set	HKWD_USER1               , 0x01000000
	.set	HKWD_USER2               , 0x02000000
	.set	HKWD_USER3               , 0x03000000
	.set	HKWD_USER4               , 0x04000000
	.set	HKWD_USER5               , 0x05000000
	.set	HKWD_USER6               , 0x06000000
	.set	HKWD_USER7               , 0x07000000
	.set	HKWD_USER8               , 0x08000000
	.set	HKWD_USER9               , 0x09000000
	.set	HKWD_USERA               , 0x0a000000
	.set	HKWD_USERB               , 0x0b000000
	.set	HKWD_USERC               , 0x0c000000
	.set	HKWD_USERD               , 0x0d000000
	.set	HKWD_USERE               , 0x0e000000
	.set	HKWD_USERAIX             , 0x0f000000

	.set	HKWD_KERN                , 0x10000000
	.set	HKWD_KERN_FLIH           , 0x10000000
	.set	HKWD_KERN_SVC            , 0x10100000
	.set	HKWD_KERN_SYSCRET        , 0x10400000
	.set	HKWD_KERN_LOCK           , 0x11200000
	.set	HKWD_KERN_UNLOCK         , 0x11300000
	.set	HKWD_KERN_LOCKALLOC      , 0x11400000
	.set	HKWD_KERN_SETRECURSIVE   , 0x11500000
	.set	HKWD_KERN_RESUME         , 0x20000000
	.set	HKWD_KERN_LOCKL          , 0x20E00000
	.set	HKWD_KERN_UNLOCKL        , 0x20F00000

# "sub-hookids"
        .set    LOCK_TAKEN               , 1
        .set    LOCK_MISS                , 2
        .set    LOCK_RECURSIVE           , 3
        .set    LOCK_BUSY                , 4
        .set    SETRECURSIVE             , 1
        .set    CLEARRECURSIVE           , 2

