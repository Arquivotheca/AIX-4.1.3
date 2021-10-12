# @(#)63	1.1  src/bos/usr/lib/methods/cfgkma/Makefile, inputdd, bos41J, 9509A_all 2/14/95 12:56:42
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
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=../cfgktsm:../common:../cfgcommon

PROGRAMS	= cfgkma

OFILES		= cfgcommon.o  cfg_runtime_br_rs6k.o  chkslot.o cfg_dvdr.o \
			cfg_get_dvdr.o  cfgtools.o  cfgkma.o util.o 

INCFLAGS	= -I../common -I../cfgcommon -I../cfgktsm

ILIST		= cfgkma
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

.include <${RULES_MK}>
