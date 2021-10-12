# @(#)33	1.2  src/bos/diag/tu/pcitok/slih/Makefile, tu_pcitok, bos41J, 9514A_all 3/30/95 13:50:10
#
#   COMPONENT_NAME: tu_pcitok
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
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


VPATH       = ${MAKETOP}/bos/diag/tu/pcitok

PROGRAMS   = sky_intr

OFILES     = sky_intr.o

LIBFLAGS   =
INCFLAGS   = -I${MAKETOP}/bos/diag/tu/pcitok
LIBS       = -lcsys

CC_OPT_LEVEL =

OPT_LEVEL  =

CFLAGS     = -DKERNEL -O -D_KERNEL -g -D_AIX

sky_intr_ENTRYPOINT = sky_interrupt
sky_intr_IMPORTS    = -bI:${MAKETOP}/bos/kernext/exp/diagex.exp -bI:${MAKETOP}/bos/kernel/exp/kernex.exp -bI:${MAKETOP}/bos/kernext/exp/dtok32_kext.exp
sky_intr_EXPORTS    = -bE:sky_intr.exp


ILIST      = ${PROGRAMS}
IDIR       = /usr/lpp/diagnostics/slih/


.include <${RULES_MK}>
