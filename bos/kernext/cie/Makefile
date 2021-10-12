#  @(#)76   1.7  src/bos/kernext/cie/Makefile, sysxcie, bos411, 9428A410j 6/20/94 19:05:39
#
#
#  COMPONENT_NAME: (SYSXCIE) COMIO Emulator
#
#  DESCRIPTION:
#     COMIO Emulator Makefile
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1988, 1994
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


.if defined(DEBUG)
NSDEBUG = 1
DMALLOC = 1
INLINE	= 0
.else
NSDEBUG = 0
DMALLOC = 0
INLINE	= 1
.endif

.include <${MAKETOP}/bos/kernext/Kernext.mk>

ILIST			= ciedd
IDIR			= /usr/lib/drivers/
LIBS			= ${KERNEXT_LIBS}

KERNEL_EXT		= ciedd

ciedd_ENTRYPOINT	= cieconfig
ciedd_IMPORTS		= ciedd.imp
ciedd_EXPORTS		= ciedd.exp
ciedd_OFILES		= addrtbl.o    \
			  chn.o        \
			  chnlist.o    \
			  ciedata.o    \
			  ciedd.o      \
			  cierecv.o    \
			  ciestat.o    \
			  cieutil.o    \
			  devtable.o   \
			  dev.o        \
			  devmgr.o     \
			  dmalloc.o    \
			  filter.o     \
			  mapaddr.o    \
			  statistics.o \
			  status.o     \
			  trccap.o     \
			  nsdmx.o      \
			  nsdebug.o    \
			  queue.o      \
			  ses.o


CFLAGS		  += -qcpluscmt -qlist -qsource

.if ${NSDEBUG}==1
CFLAGS		  += -DNSDEBUG
.endif

.if ${DMALLOC}==1
CFLAGS		  += -DDMALLOC
.endif

.if ${INLINE}==1

OPT_LEVEL	   = -O -U__STR31__
CFLAGS		  += -DINLINE -Q+cieTrace

cierecv.o_CFLAGS   = ${CFLAGS} -Q+cieTrace:mbqQueue:mbqDeQueue \
			       -Q+isMulticast:isBroadcast:activeMultiCastAddr
chn.o_CFLAGS	   = ${CFLAGS} -Q+cieTrace:mbqQueue:mbqDeQueue
ciedd.o_CFLAGS	   = ${CFLAGS} -Q+cieTrace:mbqQueue:mbqDeQueue

.else

OPT_LEVEL	   = -g

.endif

dev.o_CFLAGS       = ${CFLAGS} -B${ODE_TOOLS}/usr/lib/ -tp

.include <${RULES_MK}>
