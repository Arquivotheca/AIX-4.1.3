# @(#)58	1.6  src/bos/kernel/uipc/Makefile, sysuipc, bos411, 9428A410j 6/21/94 16:18:48
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
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

.include <${MAKETOP}bos/kernel/Kernel.mk>
.include <${MAKETOP}bos/kernel/net/Net.mk>

VPATH		=${TARGET_MACHINE}
CFLAGS		+= -Q+sosblock -Q+sbunlock

PINNED_OFILES	=	\
	syscalls.o	\
	sys_sock.o	\
	socket.o	\
	socket2.o	\
	domain.o	\
	proto.o		\
	usrreq.o	\
	log.o		\
	getcaller.o	\
	mbuf.o

.include <${RULES_MK}>
