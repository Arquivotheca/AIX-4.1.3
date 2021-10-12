# @(#)96        1.1  src/bos/usr/ccs/lib/libdbm/Makefile, libdbm, bos411, 9428A410j 8/5/92 16:20:27
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



SHARED_LIBRARIES	= libdbm.a
MSGHDRS         = libdbm_msg.h
CATFILES        = libdbm.cat
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libdbm.a

ILIST           = libdbm.a
IDIR            = /usr/ccs/lib/

LIBS=libc.a
EXPORTS= -bE:libdbm.exp

SHARED_OFILES=dbm.o

.include <${RULES_MK}>

dbm.o : libdbm_msg.h
