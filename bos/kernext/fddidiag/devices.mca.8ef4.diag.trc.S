# @(#)40        1.1  src/bos/kernext/fddidiag/devices.mca.8ef4.diag.trc.S, diagddfddi, bos411, 9428A410j 11/4/93 14:42:04
#
# COMPONENT_NAME: DIAGDDFDDI
#
# FUNCTIONS: Template file for trcrpt
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
* /etc/trcfmt
+263 1.0 "@FDDI" \
	"FDDI DD : " \
	O1.0 {{$fp1 = S1}} R4.0 {{$num = 1}}\
	$D1, \
	0x4d707842 { "Multiplex Entry (" $fp1 ")\n" \
			"\tdevno = " $D2 "\n" \
			"\tchan ptr = " $D3 "\n" \
			"\tname ptr = \'" O15.0 $num%S1 "\'"}, \
	0x4d707845 { "Multiplex Exit (" $fp1 ")\n" \
			"\topen ptr = " $D2 "\n" \
			"\terrno = " $D3 }, \
	0x4f706e42 { "Open Entry (" $fp1 ")\n" \
			"\tdevno = " $D2 "\n" \
			"\tchannel = " $D3 "\n" \
			"\text ptr = " $D4}, \
	0x4f706e45 { "Open Exit (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\topen ptr = " $D3 "\n" \
			"\terrno = " $D4 }, \
	0x436c7342 { "Close entry (" $fp1 ")\n" \
			"\tdevno = " $D2 "\n" \
			"\tchannel = " $D3 }, \
	0x436c7345 { "Close exit (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\topen ptr = " $D3 }, \
	0x63666742 { "Config entry (" $fp1 ")\n" \
			"\tdevno = " $D2 "\n" \
			"\tcmd = " $D3 "\n" \
			"\tuio ptr = " $D4 }, \
	0x63666745 { "Config exit (" $fp1 ")\n" \
			"\tadap = " $D2 "\n" \
			"\terrno = " $D3 }, \
	0x4e656c42 { "Enter Network Recovery Mode entry (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\tstate= " $D3 "\n" \
			"\trc1 = " $D4 }, \
	0x4e656c43 { "(" $fp1 ")\n" \
			"\tac = " $D2 "\n" \
			"\tcarry over= " $D3 "\n" \
			"\tPIO wall = " $D4 }, \
	0x4e656c45 { "Enter Network Recovery Mode exit (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\tstate= " $D3 "\n" \
			"\trc1 = " $D4 }, \
	0x4e626f42 { "Fatal Error Termination entry (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\tstate= " $D3 "\n" \
			"\trc1 = " $D4 }, \
	0x4e626f43 { "(" $fp1 ")\n" \
			"\trc2 = " $D2 "\n" \
			"\tac= " $D3 }, \
	0x4e626f45 { "Fatal Error Termination exit (" $fp1 ")\n" \
			"\tstate = " $D2 }, \
	0x57727442 { "Write entry (" $fp1 ")\n" \
			"\tdevno = " $D2 "\n" \
			"\tuio ptr = " $D3 "\n" \
			"\text ptr = " $D4 },\
	0x54727445 { "Write exit (" $fp1 ")\n"\
			"\tACS ptr = " $D2 "\n" \
			"\tOpen ptr = " $D3 "\n" \
			"\trc = " $D4}, \
	0x54667242 { "FastWrite entry (" $fp1 ")\n" \
			"\tdevno = " $D2 "\n" \
			"\tmbuf = "$D3}, \
	0x54667245 { "FastWrite exit (" $fp1 ")\n" }, \
	0x54636d42 { "Transmit Offlevel entry (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\tTX in use = " $D3 "\n" \
			"\tNext cmplt = " $D4}, \
	0x54636d45 { "Transmit Offlevel exit (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\tTX in use = " $D3 "\n" \
			"\tNext cmplt = " $D4}, \
	0x54666442 { "Transmit Frame Done (" $fp1 ")\n" \
			"\tACS ptr = " $D2 "\n" \
			"\terror = " $D3 "\n" \
			"\tstatus = " $D4}, \
	0x52726642 { "Receive Notify entry (" $fp1 ")\n" \
			"\theader = " $D2 }, \
	0x52726645 { "Receive Notify exit (" $fp1 ")\n" \
			"\theader = " $D2 }, \
	0x52726842 { "Receive Offlevel entry (" $fp1 ")\n" \
			"\tACS prt = " $D2 "\n" \
			"\tbus = " $D3 "\n" \
			"\trcv descriptor = " $D4}, \
	0x52726845 { "Receive Offlevel exit (" $fp1 ")\n" \
			"\tACS prt = " $D2 "\n" \
			"\tbus = " $D3 "\n" \
			"\trcv descriptor = " $D4}, \
	\*              { "Trace Point : " $fp1 "\n"\
			"\targ 1 = " $D2 "\n" \
			"\targ 2 = " $D3 "\n" \
			"\targ 3 = " $D4 }
