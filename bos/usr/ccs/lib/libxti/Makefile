# @(#)03        1.4  src/bos/usr/ccs/lib/libxti/Makefile, libxti, bos411, 9428A410j 2/20/94 17:29:45
# COMPONENT_NAME: LIBXTI
#
# FUNCTIONS:
#
# ORIGINS: 27 63
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   Copyright (c) 1990  Mentat Inc.
#


VPATH		= ${MAKETOP}/bos/kernext/exp:${MAKETOP}/bos/usr/ccs/lib/libtli

SHARED_LIBRARIES= libxti.a
MSGHDRS         = libtli_msg.h
CATFILES        = libtli.cat
HFILES          = ${MSGHDRS}

EXPINC_TARGETS  = export_libtli_msg.h
EXPLIB_TARGETS  = export_libxti.a

INCLUDES        = libtli_msg.h

ILIST           = libxti.a
IDIR            = /usr/lib/

CFLAGS		= -DXTI

INCFLAGS        = -I${MAKETOP}/bos/usr/ccs/lib/libtli

LIBS		= libc.a
IMPORTS		= -bI:pse.exp 
EXPORTS		= -bE:libxti.exp

SHARED_OFILES = \
	iostate.o taccept.o talloc.o tbind.o tblock.o tclose.o \
	tconnect.o terrno.o terror.o tfixup.o tfree.o tgetinfo.o \
	tgetpr.o tgetst.o tlisten.o tlook.o tnblock.o topen.o \
	toptmgmt.o trcv.o trcvcon.o trcvdis.o trcvrel.o trudata.o \
	truderr.o tsnd.o tsnddis.o tsndrel.o tstrerr.o tsudata.o \
	tsync.o tunbind.o tdbg.o

.include <${RULES_MK}>
