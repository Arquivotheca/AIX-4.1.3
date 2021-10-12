# @(#)67	1.1  src/bos/kernext/diagex/bos.diag.rte.trc.S, sysxdiag, bos411, 9431A411a 7/21/94 16:56:03
#
#   COMPONENT_NAME: sysxdiag
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Add the system trace formatter for DIAGEX
#
#########################################################################
#                                                                       #
#       This file is the system trace                                   # 
#       formatter for the DIAGEX kernel extension.                      #
#                                                                       #
#########################################################################
#
* /etc/trcfmt
+ 355 1.0 "DIAGEX"                                             \
	{{ $dummy = X2 }}                                         \
	A4 "arg1="X4 "arg2=" X4 "arg3=" X4 
