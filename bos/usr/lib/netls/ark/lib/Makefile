# @(#)87        1.2  src/bos/usr/ccs/lib/libC/Makefile, bos, bos410 2/18/93 12:51:49
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
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

SUBDIRS		= En_US

#EXPLIB_TARGETS	= export_libX11.a export_libXt.a export_libXm.a

ILIST		= libX11.a libXt.a libXm.a 
IDIR		= /usr/lib/netls/ark/lib/


.include <${RULES_MK}>

libX11.a:	libX11.aB
	${CP} ${libX11.aB:P} ${.TARGET}

libXt.a:	libXt.aB
	${CP} ${libXt.aB:P} ${.TARGET}

libXm.a:	libXm.aB
	${CP} ${libXm.aB:P} ${.TARGET}
