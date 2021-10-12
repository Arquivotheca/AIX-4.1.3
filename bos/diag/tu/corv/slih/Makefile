# @(#)80        1.2  src/bos/diag/tu/corv/slih/Makefile, tu_corv, bos411, 9428A410j 12/10/93 08:07:58
#
# COMPONENT_NAME: (tu_corv) Corvette Adapter Test Units
#
# FUNCTIONS: Makefile for second level interrupt handler
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
#
 
 
VPATH	= ${MAKETOP}/bos/kernext/exp
 
CFLAGS = -D CORVETTE -D DIAGNOSTICS
 
INCFLAGS 	= -I${MAKETOP}/bos/diag/tu/corv
LDFLAGS		= -bM:SRE -D512 -H512 -bloadmap:corv_slih.map
PROGRAMS	= corv_slih
LIBS		= -lodm -lcfg -lc
 
ILIST	= ${PROGRAMS}
IDIR	= /usr/lpp/diagnostics/slih/
 
corv_slih_ENTRYPOINT = corvette_interrupt
 
corv_slih_IMPORTS	= -bI:diagex.exp
 
corv_slih_EXPORTS	= -bE:corvette_intr.exp
 
corv_slih_OFILES	= corvette_intr.o
 
.include <${RULES_MK}>
