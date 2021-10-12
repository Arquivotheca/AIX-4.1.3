# @(#)55	1.1  src/bos/usr/lib/nls/loc/imt/Makefile, libtw, bos411, 9428A410j 1/13/93 14:53:10
#
# COMPONENT_NAME:  LIBTW
#
# FUNCTIONS: Traditional Chinese Input Method
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

PROGRAMS = TW.im
SUBDIRS = tfep

OFILES = tfep/libtimfep.a

LDFLAGS = -eTIMInitialize
LIBS = -lIM -liconv

ILIST = $(PROGRAMS)
IDIR = /usr/lib/nls/loc/

.include <${RULES_MK}>
