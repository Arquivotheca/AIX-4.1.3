# @(#)83	1.5  src/bos/usr/bin/que/aix/Makefile, cmdque, bos411, 9428A410j 1/3/94 10:38:10
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
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

VPATH		= ${MAKETOP}/bos/usr/bin/que:${MAKETOP}/bos/usr/bin/que/qadm:${MAKETOP}/bos/usr/lib/pios

PROGRAMS	= qprt qadm qchk qcan qpri qhld qmov
MSGHDRS		= qprt_msg.h qadm_msg.h qchk_msg.h qcan_msg.h qpri_msg.h \
		  qhld_msg.h qmov_msg.h piobe_msg.h qcadm_msg.h
CATFILES	= qprt.cat qadm.cat qchk.cat qcan.cat qpri.cat qhld.cat \
		  qmov.cat
HFILES		= ${MSGHDRS} common.h frontend.h qcadm.h \
		  piostruct.h pioformat.h piolimits.h

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

INCFLAGS	= -I../

LIBFLAGS	= -L../libque

LIBS		= -lque -ls -lqb

qprt_LIBS		= ${LIBS} -lPW
qprt_OFILES		= qprt.o qccom.o piogetvals.o piogetopt.o \
			  pioparm.o piovalfgs.o piosubr3.o piosubr2.o \
			  piosubr1.o piomsgipc.o
qprt.o_INCFLAGS		= ${INCFLAGS} -I${MAKETOP}/bos/usr/bin/que/qadm \
			  -I${MAKETOP}/bos/usr/lib/pios
qccom.o_INCFLAGS	= ${INCFLAGS} -I${MAKETOP}/bos/usr/bin/que/qadm

.include <${RULES_MK}>

.SUFFIXES: .msg .def
.def.msg:
	${CP} ${.IMPSRC} ${.TARGET}
