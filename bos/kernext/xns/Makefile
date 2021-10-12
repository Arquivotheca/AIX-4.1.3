# @(#)34	1.4  src/bos/kernext/xns/Makefile, Makefiles, bos411, 9428A410j 6/9/94 08:13:39
#
#   COMPONENT_NAME: SYSXXNS
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

KERNEL_EXT	= netns
IDIR		= /usr/lib/drivers/
ILIST		= netns

LIBS		= ${KERNEXT_LIBS}

KERNEXT_IMPORTS	= sysoverlay.exp kernex.exp syscalls.exp sockets.exp statcmd.exp ../inet/netinet.exp

netns_ENTRYPOINT	= config_ns
netns_EXPORTS		= -bE:netns.exp

netns_OFILES =		\
	ns_init.o idp_usrreq.o ns.o ns_cksum.o	\
	ns_error.o ns_input.o ns_ip.o ns_output.o	\
	ns_pcb.o ns_proto.o spp_debug.o	\
	spp_usrreq.o


.include <${RULES_MK}>
