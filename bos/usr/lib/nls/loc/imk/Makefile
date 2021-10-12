# @(#)68	1.1  src/bos/usr/lib/nls/loc/imk/Makefile, libkr, bos411, 9428A410j 1/13/93 15:18:37
#
# COMPONENT_NAME:  LIBKR
#
# FUNCTIONS: Korean Input Method
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

PROGRAMS = KR.im
SUBDIRS = hgcomp kdict ked kfep khhc kpro

OFILES = \
	hgcomp/libhgcomp.a \
	kdict/libkdict.a \
	ked/libked.a \
	kfep/libkfep.a \
	khhc/libkhhc.a \
	kpro/libkpro.a

LDFLAGS = -eKIMInitialize
LIBS = -lIM -liconv

ILIST = $(PROGRAMS)
IDIR = /usr/lib/nls/loc/

.include <${RULES_MK}>
