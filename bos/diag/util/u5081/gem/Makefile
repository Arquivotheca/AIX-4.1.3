# @(#)78	1.4  src/bos/diag/util/u5081/gem/Makefile, dagem, bos41B, 9505A 1/23/95 17:16:07
#
#   COMPONENT_NAME: DAGEM
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


PROGRAMS	= ugem

LIBS		= -lasl -ldiag -lodm -lcfg -lc -ltu_gem
IMPORTS		= -bI:${MAKETOP}/bos/kernext/exp/rcm.exp

INCFLAGS	= -I${MAKETOP}/bos/diag/tu/gem

ILIST		= ${PROGRAMS}
IDIR		= /usr/lpp/diagnostics/bin/

#TEST_UNITS	= gemexectu.o gemd_subs.o e0mom.o bustest.o dgsvpd.o dgsmem.o \
#		dgsreg.o dgsmbc.o ldgcpbat.o pattern.o dgsrgb.o interaction.o \
#		dgsmon1.o loaduc.o fifotest.o dmatest.o wrfifo.o chk.o \
#		fifo.o

ugem_OFILES	= ugem.o

.include <${RULES_MK}>
