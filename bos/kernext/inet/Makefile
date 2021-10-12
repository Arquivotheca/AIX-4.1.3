# @(#)13	1.8  src/bos/kernext/inet/Makefile, Makefiles, bos411, 9428A410j 6/9/94 08:10:02
#
#   COMPONENT_NAME: SYSXINET
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>
.include <${MAKETOP}bos/kernel/net/Net.mk>

VPATH		+=:${TARGET_MACHINE}

KERNEL_EXT	= netinet

IDIR		= /usr/lib/drivers/
ILIST		= netinet netinet.exp

netinet.exp_IDIR = /usr/lib/

LIBS		= ${KERNEXT_LIBS}

netinet_ENTRYPOINT	= config_inet
netinet_EXPORTS		= -bE:netinet.exp
KERNEXT_IMPORTS		= sysoverlay.exp kernex.exp syscalls.exp sockets.exp \
			  statcmd.exp

netinet_OFILES=		\
	igmp.o if_mcast_aux.o \
	in_init.o if_arp.o in.o	in_cksum.o in_pcb.o	\
	in_proto.o ip_icmp.o ip_input.o	ip_output.o	\
	raw_ip.o tcp_debug.o tcp_input.o tcp_output.o	\
	tcp_subr.o tcp_timer.o tcp_usrreq.o udp_usrreq.o

.include <${RULES_MK}>
