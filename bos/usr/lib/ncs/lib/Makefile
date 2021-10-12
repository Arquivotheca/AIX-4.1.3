# @(#)27	1.7  src/bos/usr/lib/ncs/lib/Makefile, libncs, bos411, 9428A410j 4/15/94 17:36:02
#
#   COMPONENT_NAME: libncs
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

SUBDIRS		= En_US compat1.1

EXPLIB_TARGETS	= export_libcps.a \
                  export_libnck.a \
                  export_libnck_task.a \
                  export_libshrnck.a

EXPDIR		= /usr/lib/

ILIST		= uuidname.txt \
		  libshrnck.a
IDIR		= /usr/lib/ncs/lib/


.include <${RULES_MK}>

libcps.a:	libcps.aB
	${CP} ${libcps.aB:P} ${.TARGET}

libnck.a:	libnck.aB
	${CP} ${libnck.aB:P} ${.TARGET}

libnck_task.a:	libnck_task.aB
	${CP} ${libnck_task.aB:P} ${.TARGET}

libshrnck.a:	libshrnck.aB
	${CP} ${libshrnck.aB:P} ${.TARGET}
