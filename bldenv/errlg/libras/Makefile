# @(#)55        1.4  src/bldenv/errlg/libras/Makefile, cmderrlg, bos410, errlg.9327d57s 8/10/93 17:30:41
# COMPONENT_NAME: BOSBUILD
#
# FUNCTIONS:
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

VPATH           += ${MAKETOP}bos/usr/bin/errlg/libras:${MAKETOP}bos/kernel/sys

OTHERS		= SWservAt.h

INCLUDES        = err_rec.h erec.h ${OTHERS}

err_rec.h_EXPDIR = /usr/include/sys/

erec.h_EXPDIR	= /usr/include/sys/
SWservAt.h_EXPDIR = /usr/include/errlg/

LIBRARIES	= libras.a

EXPLIB_TARGETS	= export_libras.a

OFILES          = SWservAt.o vset.o dircheck.o shell.o \
                  streq_cn.o streq_c.o streq.o jalloc.o jdump.o \
                  dateutil.o stracpy.o cat.o dbg.o er.o \
                  odmperror.o basename.o genexit.o jsignal.o \
                  mainreturn.o range.o savebase.o getattr.o


INCFLAGS	= -I../msg -I.. -I${MAKETOP}bos/usr/bin/errlg/libras

EXPDIR		= /usr/ccs/lib/

.include <${RULES_MK}>

SWservAt.h : SWservAt.crex
	${RM} ${_RMFLAGS_} ${.TARGET}
	ODMDIR=. ${ODMCREATE} -h SWservAt.crex

SWservAt.crex : SWservAt.cre
	${RM} ${_RMFLAGS_} ${.TARGET}
	${CP} ${SWservAt.cre:P} ${.TARGET}
