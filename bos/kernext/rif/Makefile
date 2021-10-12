# @(#)26        1.1  src/bos/kernext/rif/Makefile, Makefiles, bos411, 9428A410j 8/5/92 18:12:39
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

.include <${MAKETOP}bos/kernext/Kernext.mk>

KERNEL_EXT	= netrif
IDIR		= /usr/lib/drivers/
ILIST		= netrif


#we don't like the NET_DEF0 (from Make.net) here ...

NET_DEF1 = -DINET -DISO -DNETHER=4 -DN802_5=4 -DN802_3=4 -DNX25=4 -DNSLIP=8 \
	   -DNFDDI=4

NET_DEF2 = -DNS -DNSIP -DGATEWAY -D_BSD=44 -D_SUN -DCOMPAT_43

NET_DEFS = ${NET_DEF1} ${NET_DEF2}

LOCALCFLAGS = ${NET_DEFS}

netrif_ENTRYPOINT	= config_rif
netrif_EXPORTS		= -bE:netrif.exp
LIBS			= ${KERNEXT_LIBS}

netrif_OFILES	=	\
	rif_init.o rif.o rif_input.o	\
	rif_proto.o rif_usrreq.o	\
	raw_rif.o

.include <${RULES_MK}>
