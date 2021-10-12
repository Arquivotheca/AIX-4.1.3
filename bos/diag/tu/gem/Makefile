# @(#)54        1.3  src/bos/diag/tu/gem/Makefile, tu_gem, bos411, 9428A410j 3/23/94 08:42:26
#
#   COMPONENT_NAME: TU_GEM
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
LIBRARIES = libtu_gem.a

INCFLAGS        =   -I${MAKETOP}/bos/kernext/disp/gem/com
EXPLIB_TARGETS  = export_libtu_gem.a
EXPDIR          = /usr/lib/

OFILES         =        bustest.o chk.o dgsmem.o dgsmon1.o dgsreg.o dgsmbc.o \
			dgsrgb.o dgsvpd.o dmatest.o  fifotest.o fifo.o \
			gemexectu.o gemd_subs.o pattern.o ldgcpbat.o loaduc.o \
			interaction.o wrfifo.o


.include <${RULES_MK}>
