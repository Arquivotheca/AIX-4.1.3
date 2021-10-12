# @(#)48	1.4  src/bos/kernel/net/Makefile, sysnet, bos411, 9428A410j 5/16/94 16:02:55
#
#   COMPONENT_NAME: SYSNET
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


.include <${MAKETOP}bos/kernel/Kernel.mk>
.include <${MAKETOP}bos/kernel/net/Net.mk>

PINNED_OFILES	=	\
        if.o            \
        if_loop.o       \
        nddsock.o       \
        cdli.o          \
        nd_lan.o        \
        raw_cb.o        \
        raw_usrreq.o    \
        net_if.o        \
	rtsock.o	\
        route.o         \
        netisr.o        \
        kern_malloc.o   \
        aix_misc.o


.include <${RULES_MK}>
