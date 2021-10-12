# @(#)19	1.3  src/bos/kernext/dmx/Makefile, sysxdmx, bos411, 9428A410j 4/8/94 11:41:51
#
#   COMPONENT_NAME: SYSXDMX
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
.include <${MAKETOP}bos/kernext/Kernext.mk>
.include <${MAKETOP}bos/kernel/net/Net.mk>

KERNEL_EXT	= eth_demux tok_demux fddi_demux
IDIR		= /usr/lib/drivers/
ILIST		= eth_demux tok_demux fddi_demux

LIBS		= ${KERNEXT_LIBS}

eth_demux_ENTRYPOINT	= eth_config
tok_demux_ENTRYPOINT	= tok_config
fddi_demux_ENTRYPOINT	= fddi_dmx_config

tok_demux_EXPORTS	= -bE:tok_demux.exp
fddi_demux_EXPORTS	= -bE:fddi_demux.exp

INCFLAGS	=  -I..

.include <${RULES_MK}>
