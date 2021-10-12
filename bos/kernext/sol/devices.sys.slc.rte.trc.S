# @(#)90  1.1  src/bos/kernext/sol/devices.sys.slc.rte.trc.S, sysxsol, bos411, 9428A410j 10/26/93 10:24:43
#
# COMPONENT_NAME: SYSXSOL
#
# FUNCTIONS: SOCC template file for trcrpt
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
* /etc/trcfmt
+248 1.0 "@SERIAL OPTICAL LINK" \
	"Serial Optical Link: " \
	$D1, \
	0x54777242 { "Write entry\n" \
			"uio = "$D2 "base = "$D3 "extension = " $D4 },\
	0x54777245 { "Write exit " }, \
	0x54667742 { "FastWrite entry\n" \
			"mbuf = " $D2 }, \
	0x54667745 { "FastWrite exit " }, \
	0x54637742 { "Common write entry\n" \
			"mbuf = " $D2 "header = " $D3 "cluster = " $D4 },\
	0x54637745 { "Common write exit\n" \
			"mbuf = " $D2 }, \
	0x546e6f42 { "Transmit notify entry\n"  \
			"header = " $D2 }, \
	0x546e6f45 { "Transmit notify exit\n"  \
			"header = " $D2 }, \
	0x546f6c42 { "Transmit Offlevel entry\n" \
			"header = " $D2 }, \
	0x546f6c45 { "Transmit Offlevel exit\n" \
			"header = " $D2 }, \
	0x526e6f42 { "Receive Notify entry\n" \
			"header = " $D2 }, \
	0x526e7f45 { "Receive Notify exit\n" \
			"header = " $D2 }, \
	0x526f6c42 { "Receive Offlevel entry\n" \
			"header = " $D2 }, \
	0x526f6c45 { "Receive Offlevel exit\n" \
			"header = " $D2 }, \
	0x50636c42 { "Close entry" \
			"devno = " $D2 "channel = " $D3 }, \
	0x50636c45 { "Close exit " \
			"devno = " $D2 "channel = " $D3 }, \
	0x50696f42 { "IOctl entry " \
			"devno = " $D2 "channel = " $D3 "cmd = " $D4 }, \
	0x50696f45 { "IOctl exit " \
			"return code = " $D2 "channel = " $D3 "cmd = " $D4 }, \
	0x50737442 { "Start Entry " \
			"arg = " $D2 "devflg = " $D3 "channel = " $D4 }, \
	0x50737445 { "Start Exit " \
			"arg = " $D2 "devflg = " $D3 "channel = " $D4 }, \
	0x50687442 { " Halt entry " \
			"arg = " $D2 "devflg = " $D3 "channel = " $D4 }, \
	0x50687445 { "Halt Exit " \
			"arg = " $D2 "devflg = " $D3 "channel = " $D4 }, \
	0x50687431 { "Halt trace 1 " \
			"arg = " $D2 "devflg = " $D3 "channel = " $D4 }, \
	0x4D727342 { "Report Status Entry  " \
			"code = " $D2 "option 0 = " $D3 "open ptr = " $D4 }, \
	0x4D727371 { "Report Status Trace 1 " \
			"code = " $D2 "option 0 = " $D3 "open ptr = " $D4 }, \
	0x4D72736E { "Report Status Trace 2 " \
			"code = " $D2 "option 0 = " $D3 "open ptr = " $D4 }, \
	0x50736c42 { "Select Entry " \
			"event = " $D2 "select = " $D3 "open ptr = " $D4 }, \
	0x50736c45 { "Select Exit " \
			"event = " $D2 "select = " $D3 "open ptr = " $D4 }, \
	0x506D7842 { "Multiplex Entry " \
			"devno = " $D2 "chan ptr = " $D3 "name ptr = " $D4 }, \
	0x506D7845 { "Multiplex Exit " \
			"devno = " $D2 "chan ptr = " $D3 "name ptr = " $D4 }, \
	0x506F7042 { "Open Entry " \
			"errno = " $D2 "channel = " $D3 }, \
	0x506F7045 { "Open Exit " \
			"errno = " $D2 "channel = " $D3 }, \
	0x50726442 { "Read Entry " \
			"uiop = " $D2 "channel = " $D3 "arg = " $D4 }, \
	0x50726445 { "Read Exit " \
			"return code = " $D2 "channel = " $D3 }, \
	0x50726431 { "Read Trace 1 " \
			"mptr  = " $D2 "total bytes = " $D3 }

