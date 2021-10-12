#  @(#)13	1.5  src/bos/usr/ccs/bin/make/Makefile, cmdmake, bos412, 9445A165578  10/25/94  10:21:18
#
#   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,85
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1985,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
# ALL RIGHTS RESERVED 
#
#
# OSF/1 1.2

VPATH			= lst.lib

PROGRAMS		= make
MSGHDRS			= make_msg.h
CATFILES		= make.cat
DATAFILES		= aix.mk posix.mk

ILIST			= ${PROGRAMS} ${DATAFILES}
IDIR			= /usr/ccs/bin/
aix.mk_IDIR		= /usr/ccs/lib/
posix.mk_IDIR		= /usr/ccs/lib/

CFLAGS			= -DDEBUG_FLAG
INCFLAGS		= -I. -Ilst.lib
HFILES			= lst.h sprite.h ${MSGHDRS)
IMPL_OBJS		= arch.o compat.o dir.o getopt.o job.o main.o make.o\
			  mbyte.o parse.o str.o suff.o targ.o var.o
UTIL_OBJS		= buf.o hash.o
LIST_OBJS		= lstAppend.o \
			  lstClose.o lstConcat.o \
			  lstDeQueue.o lstDestroy.o \
			  lstFindFrom.o\
			  lstForEachFrom.o\
			  lstInit.o lstInsert.o \
			  lstMember.o lstNext.o\
			  lstOpen.o lstRemove.o lstReplace.o
OFILES			= ${IMPL_OBJS} ${UTIL_OBJS} ${LIST_OBJS}

.include <${RULES_MK}>

${IMPL_OBJS}: make.h nonints.h make_msg.h
arch.o dir.o getopt.o hash.o mbyte.o targ.o: hash.h
buf.o parse.o var.o: buf.h
job.o: job.h
main.o: pathnames.h
suff.o: bit.h
