# @(#)94        1.8  src/bos/usr/ccs/lib/libasl/Makefile, libasl, bos411, 9428A410j 5/13/94 14:07:03
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


CFLAGS=-D_ASL41
SHARED_LIBRARIES	= libasl.a
MSGHDRS         = asl_msg.h
CATFILES        = asl.cat
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libasl.a

CFLAGS          += ${_AIX3CURFLAGS} ${_AIX3CURSESFLAGS}
LIBFLAGS        += ${_AIX3CURLIBFLAG} ${_AIX3CURSESLIBFLAG}

ILIST           = libasl.a
IDIR            = /usr/lib/

EXPORTS= -bE:libasl.exp
LIBS=libcur.a libcurses.a libc.a

INCLUDES=asl_msg.h asl_cat.h
asl_msg.h_EXPDIR=/usr/include/
asl_cat.h_EXPDIR=/usr/include/

SHARED_OFILES= asl.o search.o

.include <${RULES_MK}>

asl.o:	asl_msg.h asl_cat.h

asl_cat.h: asl.prolog asl.msg asl_no_MSG.sh
	${CP} ${asl_no_MSG.sh:P} .
	${CHMOD} +x asl_no_MSG.sh
	./asl_no_MSG.sh ${asl.prolog:P} ${asl.msg:P}
	${RM} -f asl_no_MSG.sh

