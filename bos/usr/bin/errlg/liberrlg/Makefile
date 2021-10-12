# @(#)04        1.6  src/bos/usr/bin/errlg/liberrlg/Makefile, cmderrlg, bos411, 9428A410j 12/14/93 09:38:29
# COMPONENT_NAME: Makefile
#
# FUNCTIONS: Makefile for liberrlg
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

LIBRARIES	= liberrlg.a

EXPLIB_TARGETS	= export_liberrlg.a

OFILES		= errstop.o notify.o child.o log.o threshold.o \
			  codeptcat.o log_ptr_mgmt.o tmplt.o dyn_ent_util.o \
			  nls.o udbutil.o lstchk.o compress.o codeptutil.o \
			  buffer.o symptom.o validate.o

LIBS		=  -lodm -lrts -lcfg

INCFLAGS	=-I../odm -I../msg -I../errinstall -I..

EXPDIR		= /usr/ccs/lib/

.include <${RULES_MK}>
