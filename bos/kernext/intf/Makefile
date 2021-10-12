# @(#)14        1.4  src/bos/kernext/intf/Makefile, Makefiles, bos411, 9428A410j 6/14/94 16:51:22
# COMPONENT_NAME: (SYSXINTF) raw interface services
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1984
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= netintf

IDIR		= /usr/lib/drivers/
ILIST		= netintf

LIBS		= ${KERNEXT_LIBS}

netintf_ENTRYPOINT	= config_intf
netintf_EXPORTS		= -bE:netintf.exp

#we don't like the NET_DEF0 (from Make.net) here ...

NET_DEF1	= -DINET -DISO -DNETHER=4 -DN802_5=4 -DN802_3=4 -DNX25=4 \
		  -DNSLIP=8 -DNFDDI=4
NET_DEF2	= -DIP_MULTICAST -DNS -DNSIP -DGATEWAY -D_BSD=44 -D_SUN \
		  -DCOMPAT_43

NET_DEFS = ${NET_DEF1} ${NET_DEF2}

LOCALCFLAGS	= ${NET_DEFS}

KERNEXT_IMPORTS = sysoverlay.exp kernex.exp syscalls.exp statcmd.exp

netintf_OFILES=		\
	intf_init.o	\
	intf.o		\
	intf_input.o	\
	intf_proto.o	\
	intf_usrreq.o

.include <${RULES_MK}>
