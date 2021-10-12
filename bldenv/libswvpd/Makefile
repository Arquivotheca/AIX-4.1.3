# @(#)22        1.1  src/bldenv/libswvpd/Makefile, ade_build, bos412, GOLDA411a 2/17/93 08:21:52
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ${MAKETOP}/bos/usr/ccs/lib/libswvpd

LIBRARIES	= libswvpd.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libswvpd.a

ILIST           = libswvpd.a
IDIR            = /usr/ccs/lib/

OFILES		= \
	vpdclass.o vpdadd.o vpdchg.o vpddel.o vpdget.o \
	vpdinit.o vpdremote.o vpdreslpp.o vpdsql.o


.include <${RULES_MK}>
