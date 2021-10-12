# @(#)48	1.5  src/bos/usr/bin/localedef/Makefile, cmdnls, bos41J, 9516A_all 4/17/95 09:52:47
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 85
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 1.2
#

PROGRAMS	= localedef
MSGHDRS		= localedef_msg.h
CATFILES	= localedef.cat
HFILES		= ${MSGHDRS}
YFLAGS		= -d -v`${RM} -f y.output`

ILIST		= localedef
IDIR		= /usr/bin/

OFILES		= err.o gen.o scan.o gram.o list.o localedef.o method.o \
		  sem_chr.o sem_ctype.o sem_xlat.o semstack.o symtab.o \
		  sem_coll.o init.o check.o sem_method.o copy.o fwdrefstack.o

.include <${RULES_MK}>

scan.o	: gram.h

