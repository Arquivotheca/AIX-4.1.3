# @(#)02        1.1  src/bos/kernext/entdiag/devices.mca.8ef5.diag.trc.S, diagddent, bos411, 9428A410j 10/28/93 15:58:52
#
# COMPONENT_NAME: DIAGDDENT
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
+1CD 1.0 "@R2 ETHERNET" \
	"Ethernet: " \
	$D1, \
	0x724E4F54 { "enque kernel data  " }, \
	0x72515545 { "enque user   data  " }, \
	0x724F5652 { "receive overflow   " }, \
	0x77454E44 { "transmit done      " }, \
	0x72454E44 { "return from read   " }, \
	0x77515545 { "write              " }, \
	0x77424547 { "transmit interrupt " }, \
	0x72444154 { "receive  interrupt " devtoname($D2) count=$D5 $BREAK }, \
	\*         { `op=$D1 G4 A4` $NOPRINT $BREAK } \
	devtoname($D2) `mbuf=$D4 count=$D5 channel=$D3`
