# @(#)44	1.3  src/bos/diag/tu/mps/slih/Makefile, tu_mps, bos411, 9438A411a 9/18/94 14:50:34
#
#   COMPONENT_NAME: tu_mps
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#-----------------------------------------------------------------------#
#                                                                       #
#       Make file for the token ring adapter                            #
#                                                                       #
#-----------------------------------------------------------------------#


VPATH       = ${MAKETOP}/bos/diag/tu/mps

PROGRAMS   = tok32_intr

OFILES     = tok32_intr.o



LIBFLAGS   =
INCFLAGS   = -I${MAKETOP}/bos/diag/tu/mps
LIBS       = -lcsys

CC_OPT_LEVEL =

OPT_LEVEL  =

CFLAGS     = -DKERNEL -O -D_KERNEL -g -D_AIX

tok32_intr_ENTRYPOINT = mps_interrupt
tok32_intr_IMPORTS    = -bI:${MAKETOP}/bos/kernext/exp/diagex.exp -bI:${MAKETOP}/bos/kernel/exp/kernex.exp -bI:${MAKETOP}/bos/kernext/exp/dtok32_kext.exp
tok32_intr_EXPORTS    = -bE:tok32_intr.exp


ILIST      = ${PROGRAMS}
IDIR       = /usr/lpp/diagnostics/slih/


.include <${RULES_MK}>
