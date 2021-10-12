# @(#)78	1.5  src/bos/kernel/net/Net.mk, sysnet, bos411, 9428A410j 4/5/94 16:14:22
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
NET_DEFS = 	-DINET -DNS -DNSIP -DGATEWAY=1 -D_BSD=44 -D_SUN -DCOMPAT_43 \
		-DIP_MULTICAST -DDIRECTED_BROADCAST=1 -DUIPC=1 \
		-D_NO_BITFIELDS -DIPFORWARDING=0 \
		-DNONLOCSRCROUTE=0 -DIPSENDREDIRECTS=1 \
		-DRFC1122ADDRCHK=0 -DPANIC2ASSERT

CFLAGS += ${NET_DEFS}
