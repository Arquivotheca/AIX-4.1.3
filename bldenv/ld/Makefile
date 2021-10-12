# @(#)18        1.8  src/bldenv/ld/Makefile, ade_build, bos412, GOLDA411a 1/10/94 13:49:42
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

VPATH		= ${MAKETOP}bos/usr/ccs/bin/ld:${MAKETOP}bos/usr/include:${MAKETOP}bos/kernel/sys/${TARGET_MACHINE}

SUBDIRS		= bind

PROGRAMS	= ld

CCTYPE		= ansi
CFLAGS		= -D_ALL_SOURCE -U__STR31__ -Uaiws -DAIX_REL3

MSGHDRS         = ld_msg.h
CATFILES        = ld.cat
HFILES          = ${MSGHDRS}

INCLUDES	= typchk.h filehdr.h debug.h syms.h reloc.h \
		  storclass.h scnhdr.h
EXPDIR		= /usr/include/
debug.h_EXPDIR	= /usr/include/sys/

ILIST		= ld
IDIR		= /usr/bin/

.include <${RULES_MK}>

ld.o:  ld_msg.c

ld_msg.c: ld.msg
	${RM} ${_RMFLAGS_} ${.TARGET}
	${SED} -e '/^$$/d;/^\$$/d;s/^[A-Z_a-z][A-Z_a-z0-9]*[ 	]*"/,"/' \
		${ld.msg:P} >${.TARGET}
