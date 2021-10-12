# @(#)93        1.3  src/bos/usr/lib/nls/loc/jim/jfep/Makefile, libKJI, bos411, 9428A410j 2/2/94 21:49:31
#
# COMPONENT_NAME: libKJI
#
# FUNCTIONS: Japanese Input Method
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LIBRARIES	= libjfep.a

INCFLAGS	= -I../jed -I../jkkc -I${MAKETOP}/bos/usr/include
DEBUG		=

DATAFILES	= imjim.h \
			jimerrno.h

EXPDIR		= /usr/include/

INCLUDES	= ${DATAFILES}
ILIST		= ${DATAFILES}
IDIR		= /usr/include/

OFILES		= JIMClose.o \
			JIMDestroy.o \
			JIMIoctl.o \
			JIMProcess.o \
			JIMsupp.o \
			JIMCreate.o \
			JIMInit.o \
			JIMProcAux.o \
			JIMctlsub.o \
			JIMRegister.o \
			JIMCheckString.o \
			eucstr.o

.include <${RULES_MK}>
