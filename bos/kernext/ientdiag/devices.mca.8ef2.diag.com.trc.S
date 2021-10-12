# @(#)33        1.1  src/bos/kernext/ientdiag/devices.mca.8ef2.diag.com.trc.S, diagddient, bos411, 9428A410j 11/8/93 10:29:58
#
# COMPONENT_NAME: DIAGDDIENT
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
+1CB 1.0 ""                                                    \
	{{ $num = X2 }}                                         \
	A4                                                      \
	$num,                                                   \
	00  {  "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 } ,       \
	01  {                                         } ,       \
	02  {  "d1=" X4                               } ,       \
	03  {  "d1=" X4 " d2=" X4                     } ,       \
	04  {  "d1=" X4 " d2=" X4 " d3=" X4           } ,       \
	05  {  "d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 }

+1CC 1.0 "PC2 ETHERNET" \
	$1CB
