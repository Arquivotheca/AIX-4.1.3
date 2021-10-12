# @(#)04        1.6  src/bos/usr/bin/src/cmds/Makefile, cmdsrc, bos411, 9428A410j 1/19/94 19:37:23
#
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
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

VPATH		= ${MAKETOP}bos/objclass

PROGRAMS	= srcmstr startsrc lssrc stopsrc mknotify mkserver \
		  mkssys chserver chssys rmnotify rmserver rmssys \
		  refresh traceson tracesoff
MSGHDRS		=
CATFILES	= src.cat
HFILES		= ${MSGHDRS}
OTHERS		= ${CATFILES}

ILIST		= ${PROGRAMS}
IDIR		= /usr/bin/

srcmstr_IDIR	= /usr/sbin/

INCFLAGS	= -I../include 

srcmstr_OFILES	= srcalrm.o srcaddr.o srcdchd.o srcdreq.o srcdsrt.o \
		  srcdstp.o srcdsvr.o srchevn.o srcmstr.o srcnotify.o \
		  srcselect.o srcsvrs.o srcterm.o

startsrc_OFILES	= start.o
lssrc_OFILES	= status.o
stopsrc_OFILES	= stop.o
mknotify_OFILES	= addnotify.o
mkserver_OFILES	= addserver.o
mkssys_OFILES	= addssys.o
rmnotify_OFILES	= delnotify.o
rmserver_OFILES	= delserver.o
rmssys_OFILES	= delssys.o

refresh.o_CFLAGS	= -DCMD_REFRESH
traceson.o_CFLAGS	= -DCMD_TRACESON
tracesoff.o_CFLAGS	= -DCMD_TRACESOFF
srcselect.o_CFLAGS	= -D_BSD

LIBS		= -lsrc

mknotify_LIBS	= -lsrc -lodm
mkserver_LIBS	= -lsrc -lodm
mkssys_LIBS	= -lsrc -lodm
chserver_LIBS	= -lsrc -lodm
chssys_LIBS	= -lsrc -lodm
rmnotify_LIBS	= -lsrc -lodm
rmserver_LIBS	= -lsrc -lodm
rmssys_LIBS	= -lsrc -lodm
srcmstr_LIBS	= -lsrc -lodm -ls
refresh_LIBS	= -lsrc -lodm
traceson_LIBS	= -lsrc -lodm
tracesoff_LIBS	= -lsrc -lodm

.include <${RULES_MK}>

refresh.c traceson.c tracesoff.c : tracfresh.c
	@${RM} -f ${.TARGET}
	@${CP} ${tracfresh.c:P} ${.TARGET}

addnotify.o addserver.o addssys.o chserver.o \
chssys.o delnotify.o delserver.o delssys.o \
traceson.o tracesoff.o refresh.o \
srcnotify.o:  srcobj.h

srcobj.h: srcobj.cre
