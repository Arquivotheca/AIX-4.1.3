# @(#)13        1.1  src/bos/usr/ccs/lib/libori2/Makefile, libori, bos411, 9428A410j 1/10/94 14:28:36
#
#   COMPONENT_NAME: INFO
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988, 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

VPATH		= ${MAKETOP}/info/usr/lpp/info/bin/info_gr/api2/src

LIBRARIES       = libori2.a

EXPLIB_TARGETS  = export_libori2.a

OFILES          = chartran.o o2dict.o o2mgmt.o o2pqry.o o2prse.o o2read.o \
                o2refc.o o2shar.o o2srch.o o2stre.o o2titl.o o2xtrn.o

INCFLAGS        = -I${MAKETOP}/info/usr/lpp/info/bin/info_gr/api2/inc -I${MAKETOP}info/usr/lpp/info/bin/info_gr/genrinc -I${MAKETOP}/info/usr/lpp/info/bin/info_gr/platinc

EXPDIR          = /usr/ccs/lib/

.include <${RULES_MK}>

