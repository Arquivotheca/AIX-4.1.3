# @(#)33	1.3  src/bos/kernel/ml/POWER/svc_tab.m4, sysml, bos41J, 9511A_all 3/10/95 06:36:12
#*******************************************************************************
#
# COMPONENT_NAME: (SYSML)
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#******************************************************************************


#*******************************************************************************
#
# This table defines default values for all Power SVC vectors.  Vectors
# that are used are overwritten durring hardinit().
#
#*******************************************************************************

ifdef(`_POWER_RS',`
	.org	real0+0x1000
	b	bad_sc
	.org	real0+0x1020
ENTRY(svc1):				# for mtrace tool
	b	bad_sc
	.org	real0+0x1040
	b	bad_sc
	.org	real0+0x1060
	b	bad_sc
	.org	real0+0x1080		# This is not a documented function
	b	bad_sc
	b	fmap_svc
	.org	real0+0x10a0
	b	bad_sc
	.org	real0+0x10c0
	b	bad_sc
	.org	real0+0x10e0
	b	bad_sc
	.org	real0+0x1100
	b	bad_sc
	.org	real0+0x1120
	b	bad_sc
	.org	real0+0x1140
	b	bad_sc
	.org	real0+0x1160
	b	bad_sc
	.org	real0+0x1180
	b	bad_sc
	.org	real0+0x11a0
	b	bad_sc
	.org	real0+0x11c0
	b	bad_sc
	.org	real0+0x11e0
	b	bad_sc
	.org	real0+0x1200
	b	bad_sc
	.org	real0+0x1220
	b	bad_sc
	.org	real0+0x1240
	b	bad_sc
	.org	real0+0x1260
	b	bad_sc
	.org	real0+0x1280
	b	bad_sc
	.org	real0+0x12a0
	b	bad_sc
	.org	real0+0x12c0
	b	bad_sc
	.org	real0+0x12e0
	b	bad_sc
	.org	real0+0x1300
	b	bad_sc
	.org	real0+0x1320
	b	bad_sc
	.org	real0+0x1340
	b	bad_sc
	.org	real0+0x1360
	b	bad_sc
	.org	real0+0x1380
	b	bad_sc
	.org	real0+0x13a0
	b	bad_sc
	.org	real0+0x13c0
	b	bad_sc
	.org	real0+0x13e0
	b	bad_sc
	.org	real0+0x1400
	b	bad_sc
	.org	real0+0x1420
	b	bad_sc
	.org	real0+0x1440
	b	bad_sc
	.org	real0+0x1460
	b	bad_sc
	.org	real0+0x1480
	b	bad_sc
	.org	real0+0x14a0
	b	bad_sc
	.org	real0+0x14c0
	b	bad_sc
	.org	real0+0x14e0
	b	bad_sc
	.org	real0+0x1500
	b	bad_sc
	.org	real0+0x1520
	b	bad_sc
	.org	real0+0x1540
	b	bad_sc
	.org	real0+0x1560
	b	bad_sc
	.org	real0+0x1580
	b	bad_sc
	.org	real0+0x15a0
	b	bad_sc
	.org	real0+0x15c0
	b	bad_sc
	.org	real0+0x15e0
	b	bad_sc
	.org	real0+0x1600
	b	bad_sc
	.org	real0+0x1620
	b	bad_sc
	.org	real0+0x1640
	b	bad_sc
	.org	real0+0x1660
	b	bad_sc
	.org	real0+0x1680
	b	bad_sc
	.org	real0+0x16a0
	b	bad_sc
	.org	real0+0x16c0
	b	bad_sc
	.org	real0+0x16e0
	b	bad_sc
	.org	real0+0x1700
	b	bad_sc
	.org	real0+0x1720
	b	bad_sc
	.org	real0+0x1740
	b	bad_sc
	.org	real0+0x1760
	b	bad_sc
	.org	real0+0x1780
	b	bad_sc
	.org	real0+0x17a0
	b	bad_sc
	.org	real0+0x17c0
	b	bad_sc
	.org	real0+0x17e0
	b	bad_sc
	.org	real0+0x1800
	b	bad_sc
	.org	real0+0x1820
	b	bad_sc
	.org	real0+0x1840
	b	bad_sc
	.org	real0+0x1860
	b	bad_sc
	.org	real0+0x1880
	b	bad_sc
	.org	real0+0x18a0
	b	bad_sc
	.org	real0+0x18c0
	b	bad_sc
	.org	real0+0x18e0
	b	bad_sc
	.org	real0+0x1900
	b	bad_sc
	.org	real0+0x1920
	b	bad_sc
	.org	real0+0x1940
	b	bad_sc
	.org	real0+0x1960
	b	bad_sc
	.org	real0+0x1980
	b	bad_sc
	.org	real0+0x19a0
	b	bad_sc
	.org	real0+0x19c0
	b	bad_sc
	.org	real0+0x19e0
	b	bad_sc
	.org	real0+0x1a00
	b	bad_sc
	.org	real0+0x1a20
	b	bad_sc
	.org	real0+0x1a40
	b	bad_sc
	.org	real0+0x1a60
	b	bad_sc
	.org	real0+0x1a80
	b	bad_sc
	.org	real0+0x1aa0
	b	bad_sc
	.org	real0+0x1ac0
	b	bad_sc
	.org	real0+0x1ae0
	b	bad_sc
	.org	real0+0x1b00
	b	bad_sc
	.org	real0+0x1b20
	b	bad_sc
	.org	real0+0x1b40
	b	bad_sc
	.org	real0+0x1b60
	b	bad_sc
	.org	real0+0x1b80
	b	bad_sc
	.org	real0+0x1ba0
	b	bad_sc
	.org	real0+0x1bc0
	b	bad_sc
	.org	real0+0x1be0
	b	bad_sc
	.org	real0+0x1c00
	b	bad_sc
	.org	real0+0x1c20
	b	bad_sc
	.org	real0+0x1c40
	b	bad_sc
	.org	real0+0x1c60
	b	bad_sc
	.org	real0+0x1c80
	b	bad_sc
	.org	real0+0x1ca0
	b	bad_sc
	.org	real0+0x1cc0
	b	bad_sc
	.org	real0+0x1ce0
	b	bad_sc
	.org	real0+0x1d00
	b	bad_sc
	.org	real0+0x1d20
	b	bad_sc
	.org	real0+0x1d40
	b	bad_sc
	.org	real0+0x1d60
	b	bad_sc
	.org	real0+0x1d80
	b	bad_sc
	.org	real0+0x1da0
	b	bad_sc
	.org	real0+0x1dc0
	b	bad_sc
	.org	real0+0x1de0
	b	bad_sc
	.org	real0+0x1e00
	b	bad_sc
	.org	real0+0x1e20
	b	bad_sc
	.org	real0+0x1e40
	b	bad_sc
	.org	real0+0x1e60
	b	bad_sc
	.org	real0+0x1e80
	b	bad_sc
	.org	real0+0x1ea0
	b	bad_sc
	.org	real0+0x1ec0
	b	bad_sc
	.org	real0+0x1ee0
	b	bad_sc
	.org	real0+0x1f00
	b	bad_sc
	.org	real0+0x1f20
	b	bad_sc
	.org	real0+0x1f40
	b	bad_sc
	.org	real0+0x1f60
	b	bad_sc
	.org	real0+0x1f80
	b	bad_sc
	.org	real0+0x1fa0
	b	bad_sc
	.org	real0+0x1fc0
	b	bad_sc
	.org	real0+0x1fe0
	b	bad_sc

	.globl ENTRY(svc1)
',)
