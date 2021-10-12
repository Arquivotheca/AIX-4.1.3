# @(#)97        1.1  src/bos/usr/lib/nls/loc/jim/Makefile, libKJI, bos411, 9428A410j 1/20/93 10:59:32
#
# COMPONENT_NAME: libKJI
#
# FUNCTIONS: Japanese Input Method
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS		= jkkc jmnt jexm jdict jpro jed jfep

PROGRAMS	= JP.im
MSGHDRS		=
CATFILES	=
HFILES		= ${MSGHDRS}

${PROGRAMS}_ENTRYPOINT = JIMInitialize

OFILES		= jfep/libjfep.a \
			jed/libjed.a \
			jpro/libjpro.a \
			jdict/libjdict.a \
			jexm/libjexm.a \
			jmnt/libjmnt.a \
			jkkc/libjkkc.a 

ILIST		= ${PROGRAMS}
IDIR		= /usr/lib/nls/loc/

INCFLAGS	= -Iusr/ccs/lib/libim
LIBS		= -lIM -liconv

.include <${RULES_MK}>
