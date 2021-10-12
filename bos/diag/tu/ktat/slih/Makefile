# @(#)33	1.2  src/bos/diag/tu/pcitok/slih/Makefile, tu_pcitok, bos41J, 9514A_all 3/30/95 13:50:10
#
#   COMPONENT_NAME: tu_ktat
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
#       Make file for the Klickitat adapter                             #
#                                                                       #
#-----------------------------------------------------------------------#


VPATH       = ${MAKETOP}/bos/diag/tu/ktat

PROGRAMS   = ktat_intr

OFILES     = ktat_intr.o

LIBFLAGS   =
INCFLAGS   = -I${MAKETOP}/bos/diag/tu/ktat
LIBS       = -lcsys

CC_OPT_LEVEL =

OPT_LEVEL  =

CFLAGS     = -DKERNEL -O -D_KERNEL -g -D_AIX

ktat_intr_ENTRYPOINT = ktat_interrupt
ktat_intr_IMPORTS    = -bI:${MAKETOP}/bos/kernext/exp/diagex.exp -bI:${MAKETOP}/bos/kernel/exp/kernex.exp 
ktat_intr_EXPORTS    = -bE:ktat_intr.exp


ILIST      = ${PROGRAMS}
IDIR       = /usr/lpp/diagnostics/slih/


.include <${RULES_MK}>
