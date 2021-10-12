# @(#)07        1.4  src/bos/usr/ccs/lib/libsrc/Makefile, libsrc, bos411, 9428A410j 9/8/93 13:40:45
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

VPATH		= ${MAKETOP}/bos/objclass:${MAKETOP}/bos/usr/bin/src/cmds:${MAKETOP}/bos/kernel/exp

SHARED_LIBRARIES	= libsrc.a
MSGHDRS         =
CATFILES        =
HFILES          = ${MSGHDRS}

EXPLIB_TARGETS  = export_libsrc.a

ILIST           = libsrc.a
IDIR            = /usr/lib/

LIBS		= libc.a libodm.a librts.a libs.a
IMPORTS		= -bI:syscalls.exp
EXPORTS		= -bE:libsrc.exp

INCFLAGS	=-I${MAKETOP}/bos/objclass -I${MAKETOP}/bos/usr/bin/src/include

SHARED_OFILES	= \
	srcprint.o srcinet.o whatsz.o getssys.o delssys.o \
	defssys.o addssys.o chssys.o bldview.o checkssys.o \
	cmdargs.o srcobj.o parseopt.o readsubsys.o recvpkt.o \
	sendpkt.o setupsocket.o srcaudit.o srcclose.o srcelog.o \
	srcerr.o srcmsg.o def_msg.o def_stat.o srclock.o srcodm.o srcrrqs.o \
	srcsbuf.o srcsockaddr.o srcsockset.o srcsrpy.o srcsrqt.o \
	srcstat.o srcstathdr.o srcstattxt.o srcstop.o srcstrt.o \
	tellsrc.o

.include <${RULES_MK}>

srcprint.o getssys.o delssys.o defssys.o \
addssys.o chssys.o checkssys.o cmdargs.o \
readsubsys.o:	srcobj.h

def_msg.c: src.msg
	${CP} ${src.msg:P} .
	${ECHO} '/0513\n-1p\n1,.d\n1\ng/^\\$$ NO_MSG/s/.*/(char *)0,\n1\ng/^\\$$/d\n1\ng/^[ 	]$$/s//g\n1\ng/^$$/d\n1\ng/0513/s/^[01][0-9][0-9] \"/\"/\n1\ng/\"$$/s/$$/,/\n$$\ns/\,$$/};/\n0a\nchar *src_def_msg[]={\n.\nw def_msg.c\nQ\n' | ${ED} ./src.msg
	${RM} ./src.msg

def_stat.c: src.msg
	${CP} ${src.msg:P} .
	${ECHO} '/^\\$$set\n1,.d\n/^\\$$set\n.,$$d\n1\ng/^\\$$/d\n1\ng/^[ 	]$$/s///g\n1\ng/^$$/d\n1\ng/^.*[ 	]"/s//"/\n1\ng/\"$$/s/$$/,/\n$$\ns/\,$$/};/\n0a\nchar *src_def_stat[]={\n.\nw def_stat.c\nQ\n' | ${ED} ./src.msg
	${RM} ./src.msg
