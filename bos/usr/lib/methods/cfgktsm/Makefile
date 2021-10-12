# @(#)15	1.4  src/bos/usr/lib/methods/cfgktsm/Makefile, inputdd, bos41J, 9519A_all 5/9/95 07:29:02
#
#   COMPONENT_NAME: INPUTDD
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=../common:../cfgcommon

PROGRAMS	= cfgktsm

OFILES		= cfgcommon.o  cfg_runtime_br_rs6k.o  chkslot.o \
		  cfg_dvdr.o  cfg_get_dvdr.o  cfgtools.o  cfgktsm.o \
		  util.o  ddstools.o

INCFLAGS	= -I../common -I../cfgcommon

ILIST		= cfgktsm
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
