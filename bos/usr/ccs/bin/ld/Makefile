# @(#)33        1.4  src/bos/usr/ccs/bin/ld/Makefile, cmdld, bos411, 9428A410j 6/7/93 13:24:19
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 10,27
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

SUBDIRS		= bind

PROGRAMS	= ld

CCTYPE		= ansi
CFLAGS		= -D_ALL_SOURCE -U__STR31__ -Uaiws

MSGHDRS         = ld_msg.h
CATFILES        = ld.cat
HFILES          = ${MSGHDRS}

ILIST           = ld
IDIR            = /usr/ccs/bin/

.include <${RULES_MK}>

ld.o:  ld_msg.c

ld_msg.c: ld.msg
	${SED} -e '/^$$/d;/^\$$/d;s/^[A-Z_a-z][A-Z_a-z0-9]*[ 	]*"/,"/' \
		${ld.msg:P} > ${.TARGET}
