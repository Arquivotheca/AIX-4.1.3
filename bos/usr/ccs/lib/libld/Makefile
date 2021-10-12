# @(#)98        1.1  src/bos/usr/ccs/lib/libld/Makefile, libld, bos411, 9428A410j 8/5/92 16:20:45
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#



LIBRARIES	= libld.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libld.a

ILIST           = libld.a
IDIR            = /usr/ccs/lib/

CFLAGS= -DAIXV3AR -DAR32W -DFLEXNAMES

OFILES= \
        allocldptr.o freeldptr.o ldaclose.o ldahread.o \
        ldaopen.o lddopen.o ldclose.o ldfhread.o ldgetname.o \
        ldlread.o ldlseek.o ldnlseek.o ldnrseek.o ldnshread.o \
        ldnsseek.o ldohseek.o ldopen.o ldrseek.o ldshread.o \
        ldsseek.o ldtbindex.o ldtbread.o ldtbseek.o sgetl.o \
        sputl.o vldldptr.o


.include <${RULES_MK}>
