# @(#)60	1.11  src/bos/kernext/dlc/Makefile, sysxdlcg, bos411, 9428A410j 3/31/94 17:35:15
#
# COMPONENT_NAME: (SYSXDLCG) Data Link Controls
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#-----------------------------------------------------------------------#
# Data Link Control kernel extension subdirectory
#
#	This makefile resides in the DLC  directory of the src tree.
#	It visits each subdirectory and invokes make recursively to 
#	produce each product.
#
#-----------------------------------------------------------------------#
.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPINC_SUBDIRS = include errlg fdl trl sdl qlc smit

EXPLIB_SUBDIRS = head

SUBDIRS =   include edl e3l errlg fdl trl sdl qlc trace smit


.include <${RULES_MK}>

