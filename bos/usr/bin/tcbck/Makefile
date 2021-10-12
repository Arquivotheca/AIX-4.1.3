# @(#)06        1.5  src/bos/usr/bin/tcbck/Makefile, Makefiles, bos411, 9438C411e 9/25/94 13:35:49
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		=${MAKETOP}/bos/usr/sbin/grpck

PROGRAMS	= tcbck
MSGHDRS		= tcbck_msg.h
CATFILES	= tcbck.cat
HFILES		= ${MSGHDRS}

ILIST		= tcbck
IDIR		= /usr/bin/

OFILES		= comfix.o tcbcomhash.o tcbck.o tcbacl.o tcbadd.o \
		  tcball.o tcbattr.o tcbaudit.o tcbdbio.o tcbdel.o \
		  tcbentry.o tcbinit.o tcblink.o tcbmode.o tcbmsg.o \
		  tcbsrc.o systcbck.o tcbtree.o tcbutil.o

#CFLAGS          = -D_BSD
LIBS		= -ls -lodm -lIN -lfs

.include <${RULES_MK}>
