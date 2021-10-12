# @(#)02        1.4  src/bos/usr/ccs/lib/libtli/Makefile, libtli, bos411, 9428A410j 12/20/93 17:01:51
# COMPONENT_NAME:  LIBTLI
#
# FUNCTIONS:
#
# ORIGINS: 27 63
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   Copyright (c) 1990  Mentat Inc.
#

VPATH		= ${MAKETOP}/bos/kernext/exp

SHARED_LIBRARIES= libtli.a
MSGHDRS         = libtli_msg.h
CATFILES        = libtli.cat
HFILES          = ${MSGHDRS}

EXPINC_TARGETS  = export_libtli_msg.h
EXPLIB_TARGETS  = export_libtli.a

INCLUDES        = libtli_msg.h

ILIST           = libtli.a
IDIR            = /usr/lib/

LIBS		= libc.a
IMPORTS		= -bI:pse.exp
EXPORTS		= -bE:libtli.exp

SHARED_OFILES = \
	iostate.o taccept.o talloc.o tbind.o tblock.o tclose.o \
	tconnect.o terrno.o terror.o tfixup.o tfree.o tgetinfo.o tgetst.o \
	tlisten.o tlook.o tnblock.o topen.o toptmgmt.o trcv.o \
	trcvcon.o trcvdis.o trcvrel.o trudata.o truderr.o tsnd.o \
	tsnddis.o tsndrel.o tsudata.o tsync.o tunbind.o tdbg.o

.include <${RULES_MK}>
