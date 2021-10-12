#static char sccsid[] = "@(#)70  1.9  src/bos/diag/tu/tablet/Makefile, tu_tab, bos411, 9428A410j 5/25/94 14:29:51
#
# COMPONENT_NAME: tu_tab
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Makefile HTX/Mfg. Tablet Test Unit library
# 
################################################################################

VPATH           = ${MAKETOP}/bos/diag/tu/tablet

LIBRARIES       = libtabtu.a

EXPLIB_TARGETS  = export_libtabtu.a
#EXPDIR          = /${HTXLIBDIR}
EXPDIR          = /usr/lib/

OPT_LEVEL       = -g

#CFLAGS	 	=  -Dnodiag 
CFLAGS          = -DDIAGNOSTICS

INCFLAGS        = -I${MAKETOP}/bos/diag/tu/tablet
MSGHDRS         = dtablet_msg.h
CATFILES        = dtablet.cat
HFILES          = ${MSGHDRS}

LIBS            =

OFILES          = \
	exectu.o  \
	tab_msg.o \
	mktu_rc.o \
	tab_io.o  \
        tab_fns.o \
	sem.o  \
	tu10.o \
	tu20.o \
        tu30.o \
	tu40.o \
	tu50.o \
	tu60.o \
        tu70.o 

ILIST           = dtablet.cat
IDIR            = /usr/lpp/diagnostics/catalog/default/

.include <${RULES_MK}>

