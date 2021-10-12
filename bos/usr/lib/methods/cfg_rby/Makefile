# @(#)73	1.3  src/bos/usr/lib/methods/cfg_rby/Makefile, rcmrby, bos411, 9428A410j 3/21/94 18:09:55
#
#   COMPONENT_NAME: RCMRBY
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ../common

PROGRAMS	= cfgrby

OFILES		= cfgtools.o chkslot.o rby_cfg_load.o cfgrby.o ddstools.o

INCFLAGS	= -I../common 					\
		  -I../graphics					\
		  -I${MAKETOP}bos/kernext/disp/rby/inc

ILIST		= cfgrby
IDIR		= /usr/lib/methods/

LIBS		= -ls -lodm -lcfg -lrts

cfgrby_ENTRYPOINT = dev_cfg_entry

.include <${RULES_MK}>
