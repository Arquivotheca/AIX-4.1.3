# @(#)04        1.3  src/bos/usr/ccs/lib/libtermcap/Makefile, libtermcap, bos411, 9428A410j 3/1/93 15:37:10
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


SHARED_LIBRARIES	=libtermcap.a
MSGHDRS         = termc_msg.h
CATFILES        = termc.cat
HFILES          = ${MSGHDRS}
SUBDIRS		= termcap.db

EXPLIB_TARGETS  = export_libtermcap.a

ILIST           = libtermcap.a termcap.src reorder
IDIR            = /usr/ccs/lib/
termcap.src_IDIR = /usr/lib/libtermcap/
reorder_IDIR	= /usr/share/lib/tabset/

LIBS=libc.a
EXPORTS= -bE:libtermcap.exp

SHARED_OFILES	= termcap.o tgoto.o tputs.o

.include <${RULES_MK}>

${SHARED_OFILES}: ${HFILES}
