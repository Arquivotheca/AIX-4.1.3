# @(#)04        1.4  src/bldenv/errlg/liberrlg/Makefile, cmderrlg, bos412, GOLDA411a 7/7/93 12:34:19
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

VPATH           += ${MAKETOP}/bos/usr/bin/errlg/liberrlg:${MAKETOP}/bos/kernel/sys

INCLUDES        = err_rec.h erec.h

err_rec.h_EXPDIR = /usr/include/sys/

erec.h_EXPDIR	= /usr/include/sys/

LIBRARIES	= liberrlg.a

EXPLIB_TARGETS	= export_liberrlg.a

OFILES		= errstop.o notify.o child.o log.o threshold.o \
		  codeptcat.o log_ptr_mgmt.o tmplt.o dyn_ent_util.o \
		  nls.o udbutil.o lstchk.o compress.o codeptutil.o

LIBS		=  -lodm -lrts -lcfg

INCFLAGS	=-I../odm -I../msg -I../errinstall -I..

EXPDIR		= /usr/ccs/lib/

.include <${RULES_MK}>
