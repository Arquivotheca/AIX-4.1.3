# @(#)28        1.8  src/bos/usr/bin/errlg/libras/Makefile, cmderrlg, bos41J, 9516B_all 4/17/95 13:22:56
# COMPONENT_NAME:
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

LIBRARIES	= libras.a

EXPLIB_TARGETS	= export_libras.a

INCLUDES	= SWservAt.h

BUILD_LIST	= SWservAt SWservAt.vc

ILIST		= ${BUILD_LIST} SWservAt.add

IDIR		= /usr/lib/ras/
SWservAt_IDIR	= /etc/objrepos/
SWservAt.vc_IDIR	= /etc/objrepos/
SWservAt.h_EXPDIR	= /usr/include/errlg/

OFILES		= vset.o dircheck.o shell.o \
		  streq_cn.o streq_c.o streq.o jalloc.o jdump.o \
		  dateutil.o stracpy.o cat.o dbg.o er.o \
		  odmperror.o basename.o genexit.o jsignal.o \
		  mainreturn.o range.o savebase.o getattr.o \
		  SWservAt.o num_chk.o

INCFLAGS	=-I../msg
INCFLAGS	=-I..

EXPDIR		= /usr/ccs/lib/

NOUPDATE_LIST = \
    /etc/objrepos/SWservAt \
    /etc/objrepos/SWservAt.vc \
    /usr/lib/ras/SWservAt.add \
    /usr/lib/ras/SWservAt.cre



.include <${RULES_MK}>

${BUILD_LIST}:	SWservAt.crex
	${RM} -f SWservAt
	ODMDIR=. ${ODMCREATE} -p SWservAt.crex
	ODMDIR=. ${ODMADD} ${SWservAt.add:P} > /dev/null || { ${RM} -f SWservAt; false ; }

SWservAt.h :    SWservAt.crex
	 ${RM} -f SWservAt.h ${.TARGET:.h=.c}
	 ODMDIR=. ${ODMCREATE} -h SWservAt.crex

SWservAt.crex : SWservAt.cre
	 ${CP} ${SWservAt.cre:P} ${.TARGET}
