# @(#)91        1.2  src/bos/kernext/tokdiag/devices.mca.8fc8.diag.trc.S, diagddtok, bos411, 9428A410j 10/28/93 11:16:33
#
# COMPONENT_NAME: DIAGDDTOK
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
#
# FILE FORMAT: All lines prior to the "* /etc/trcfmt" line must be comments
#       starting with the "#" character with no blank lines.  These lines
#       will be striped during build.  This file must be shipped with the
#       "* /etc/trcfmt" line as the top line.
#
#
# TRC_RNOT (0x724E4F54) /* proc_recv -- kernel process passed data     */
# TRC_RQUE (0x72515545) /* proc_recv -- read data queued for user      */
# TRC_ROVR (0x724F5652) /* proc_recv -- user proc read queue overflow  */
# TRC_WEND (0x77454E44) /* tok_xmit_done -- write is complete          */
# TRC_REND (0x72454E44) /* tokread -- data delivered to user proc      */
# TRC_WQUE (0x77515545) /* tokwrite -- write data has just been queued */
# TRC_RDAT (0x72444154) /* read_recv_chain -- a packet was received    */
#
# proc_recv:
#    trchkgt(TRC_RNOT,devno,chan,mbuf,count); /* kernel process */
#    trchkgt(TRC_RQUE,devno,chan,mbuf,count); /* user   process */
#    trchkgt(TRC_ROVR,devno,chan,mbuf,count); /* receive overflow */
# tok_xmit_done:
#    trchkgt(TRC_WEND,0,0,mbuf,count);        /* transmit done */
# tokread:
#    trchkgt(TRC_REND,devno,chan,mbuf,count); /* read completion */
# tokwrite:
#    trchkgt(TRC_WQUE,devno,chan,mbuf,count); /* write has been queued */
# read_recv_chain:
#    trchkgt(TRC_RDAT,CIO.devno,0,0,count);   /* receive data interrupt */
#
#
* /etc/trcfmt
+1CE 1.0 "tokdd" \
	"ID:" O2 A4.8 "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4  \
	R0 \
	$D1, \
	0x724E4F54 { \n "enque kernel data  " devtoname($D2) \
			`mbuf=$D4 count=$D5 channel=$D3` }, \
	0x72515545 { \n "enque user   data  " devtoname($D2) \
			`mbuf=$D4 count=$D5 channel=$D3` }, \
	0x724F5652 { \n "receive overflow   " devtoname($D2) \
			`mbuf=$D4 count=$D5 channel=$D3` }, \
	0x77454E44 { \n "transmit done      " devtoname($D2) \
			`mbuf=$D4 count=$D5 channel=$D3` }, \
	0x72454E44 { \n "return from read   " devtoname($D2) \
			`mbuf=$D4 count=$D5 channel=$D3` }, \
	0x77515545 { \n "write              " devtoname($D2) \
			`mbuf=$D4 count=$D5 channel=$D3` }, \
	0x72444154 { \n "receive  interrupt " devtoname($D2) count=$D5 }
