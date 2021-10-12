# @(#)72        1.3 8/15/94 src/bos/kernext/disp/trace/gs_io_trcfmt.s, sysxdisp, bos411, 9433B411a 15:00:26
#
# COMPONENT_NAME: (sysxdisp) Display Sub-System
#
# FUNCTIONS: None
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#
###############################################################################
#
#
#	I/O Trace Macro System Trace Formatter File
#
#
#	This formatter was developed strictly for use with the
#	Low-level I/O hooks.
#
#	In order for the system trace mechanism to be useful, anytime
#	any function is added or major changes are made that require
#	additional trace hooks to be added to source files(s), this file
#	should be updated so that there is always an up-to-date formatter
#	file that is ready to be used whenever needed.
#
#
###############################################################################

700 1.0 L=APPL "GS I/O " \
	{{$format = $HD & 0x0000ffff}} O2 \
	$format, \
		0001 { "AP RAST   REG  "X4" "X4 O12 }, \
		0002 { "DD RAST   REG  "X4" "X4 O12 }, \
		0003 { "AP RAMDAC ADDR     "O2 X2 O16 }, \
		0004 { "DD RAMDAC ADDR     "O2 X2 O16 }, \
		0005 { "AP RAMDAC REG                 "O7 X1 O12 }, \
		0006 { "DD RAMDAC REG                 "O7 X1 O12 }, \
		0007 { "AP RAMDAC LUT                 "O7 X1 O12 }, \
		0008 { "DD RAMDAC LUT                 "O7 X1 O12 }, \
		0009 { "AP DFA    4    "X4" "X4 O12 }, \
		000A { "AP DFA    2    "X4"     "O2 X2 O12 }, \
		000B { "AP DFA    1    "X4"       "O3 X1 O12 }, \
		000C { "AP BD     ADDR     "O2 X2 O16 }, \
		000D { "AP BD     REG                 "O7 X1 O12 }, \
		000E { "DD BD     ADDR     "O2 X2 O16 }, \
		000F { "DD BD     REG                 "O7 X1 O12 }, \
		0010 { "DD CFG    REG  "X4" "X4 O12 } \
