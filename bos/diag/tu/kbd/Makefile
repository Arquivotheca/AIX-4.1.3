# static char sccsid[] = "@(#)27  1.6  src/bos/diag/tu/kbd/Makefile, tu_kbd, bos411, 9428A410j 5/24/94 14:29:56";
#
# COMPONENT_NAME: tu_kbd
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
# Makefile HTX/Mfg. Keyboard Test Unit library
# 
################################################################################

LIBRARIES       = libkbdtu.a

EXPLIB_TARGETS  = export_libkbdtu.a
EXPDIR          = /usr/lib/

VPATH           = ${MAKETOP}/bos/diag/tu/kbd

CFLAGS          = -DDIAGNOSTICS ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}

INCFLAGS        = -I${MAKETOP}/bos/diag/tu/kbd
MSGHDRS         = dkbd_msg.h
CATFILES        = dkbd.cat
HFILES          = ${MSGHDRS}

LIBS            = ${_AIX3CURLIB} ${_AIX3CURSESLIB}

OFILES          = \
	exectu.o \
	mktu_rc.o \
	kbd_io.o \
	kbd_fns.o \
	sem.o \
	tu10.o \
	tu20.o \
	tu30.o \
	tu40.o \
	tu50.o \
	tu60.o \
	kbd_msg.o 

ILIST           = dkbd.cat
IDIR            = /usr/lpp/diagnostics/catalog/default/

.include <${RULES_MK}>


