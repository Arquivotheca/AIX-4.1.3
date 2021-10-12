# @(#)79	1.7  src/bos/kernext/dlc/qlc/Makefile, sysxdlcq, bos411, 9428A410j 4/1/94 13:53:14
#
# COMPONENT_NAME: (SYSXDLCQ) QLLC Data Link Controls
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
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
#----------------------------------------------------------------------
#  Makefile for dlcqllc
#-----------------------------------------------------------------------

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT = QLLC.ext
IDIR = /usr/lib/drivers/
ILIST = QLLC.ext qlcextcb.h

qlcextcb.h_IDIR = /usr/include/sys/

LIBS=  ${KERNEXT_LIBS}

X25=${MAKETOP}bos/kernext/x25

VPATH := ${VPATH}:${X25}

INCLUDES= qlcextcb.h

EXPDIR = /kernel/include/sys/

CFLAGS =  -DJS_KERNEL -DQLLC -D_KERNEL -D_AIXV3_POSTWAIT

QLLC.ext_ENTRYPOINT = qlcconfig

QLLC.ext_OFILES =  cnfg_init.o qlch.o qlcb.o qlcc.o qlcctrc.o qlclasyn.o \
	qlclsync.o qlcltime.o qlclutil.o qlcp.o qlcpcons.o qlcqfsm1.o \
	qlcqfsm2.o qlcqmisc.o qlcs.o qlcvbuf.o qlcvfac.o qlcvpkt.o qlcvsess.o \
	qlcerrlog.o qlcoutputf.o  jsmbuf.o jsfac.o

	
INCFLAGS = -I${MAKETOP}/bos/kernext

.include <${RULES_MK}>

