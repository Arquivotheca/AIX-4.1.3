#static char sccsid[] = "@(#)32  1.2  src/bos/diag/tu/mouse/Makefile, tu_mouse, bos411, 9428A410j 2/2/94 09:37:29";
#
# COMPONENT_NAME: tu_mouse
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
# Makefile HTX/Mfg. Mouse Test Unit library
# 
################################################################################

LIBRARIES       = libmousetu.a

EXPLIB_TARGETS  = export_libmousetu.a
EXPDIR          = /usr/lib/
#EXPDIR          = /${HTXLIBDIR}

LIBS            =

#CFLAGS          = -Dnodiag

OFILES          = \
	exectu.o \
	mktu_rc.o \
	mou_io.o \
	sem.o\
	tu10.o\
	tu20.o\
        tu30.o\
	tu40.o\
	tu50.o


ILIST           =
IDIR            =

.include <${RULES_MK}>

