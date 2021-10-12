# @(#)92        1.1  src/bos/kernext/inputdd/mousedd/Makefile, inputdd, bos411, 9428A410j 10/24/93 14:43:44
# COMPONENT_NAME: INPUTDD
#
# FUNCTIONS: none
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

.if defined(GS_DEBUG)
CFLAGS +=-DGS_DEBUG_TRACE -DGS_DEBUG
.endif

VPATH              +=:../common
INCFLAGS           =-I../inc
STRIP_FLAG         ?=-s
LDFLAGS_KERNEXT    +=${STRIP_FLAG}
LIBS               = ${KERNEXT_LIBS}

KERNEL_EXT         = mousedd
IDIR               = /usr/lib/drivers/
ILIST              = mousedd

mousedd_ENTRYPOINT = mseconfig

OFILES = \
    msefnspin.o mseintr.o mseconfig.o msefns.o ktsmtrace.o \
    ktsmfnspin.o ktsmfns.o

.include <${RULES_MK}>
