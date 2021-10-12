# @(#)83	1.1  src/bos/diag/tu/ktat/Makefile, tu_ktat, bos41J 3/24/95 08:19:35
#
# COMPONENT_NAME: TU_KTAT
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Makefile Klickitat Ethernet Test Unit library
#
###########################################################################
# Uncomment the following two lines to save symbolic info for dbx
CFLAGS = -DHAVESLIH -DDIAGPATH
STRIP_FLAG =

SUBDIRS		 = slih

EXPINC_SUBDIRS	= slih
EXPLIB_SUBDIRS	= slih

LIBRARIES	= libktattu.a

INCLUDES  =\
	   kent_defs.h \
	   kent_tu_type.h

EXPLIB_TARGETS	=	export_libktattu.a
EXPDIR		=	/usr/lib/

LIBS            =

OFILES          = \
	tu_open.o \
	tu_close.o \
	dev_access.o \
	kent_utilities.o \
	tu_wrap.o \
	exectu.o \
	tu001.o \
	tu002.o \
	tu003.o \
	tu004.o \
	tu005.o \
	tu006.o \
	tu008.o

ILIST           =
IDIR            =

.include <${RULES_MK}>
