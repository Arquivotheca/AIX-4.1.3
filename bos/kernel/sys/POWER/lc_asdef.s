# @(#)31	1.2  src/bos/kernel/sys/POWER/lc_asdef.s, sysml, bos41J, 9513B_all 3/22/95 14:39:35
#
# COMPONENT_NAME: sysml
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


# S_PROLOG(name) - prolog for a simple routine which
#                   does not call or modify r13-r31
#
# This version causes the csect to be aligned on a
# 2^5 byte boundary.
#
# This might properly belong in asdef.s, but as of now
# I don't want to force everything that uses asdef.s
# to rebuild.
		
define( S_PROLOG_5,
        `.csect ENTRY($1[PR]),5
	 .globl ENTRY($1[PR])')
