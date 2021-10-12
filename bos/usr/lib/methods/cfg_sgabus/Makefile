#@(#)66        1.3  src/bos/usr/lib/methods/cfg_sgabus/Makefile, dispcfg, bos411, 9428A410j 11/22/93 14:38:24
#
# COMPONENT_NAME: SYSXDISPCCM 
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
VPATH			= ../common:../graphics
PROGRAMS		= cfg_sgabus
LIBS			= -ls -lcfg -lodm -lrts
ILIST			= ${PROGRAMS}
IDIR			= /usr/lib/methods/
CONFBASE		= cfgdevice.o cfgtools.o chkslot.o
cfg_sgabus_OFILES 	= $(CONFBASE) cfg_sgabus.o cfg_graphics_frs_tools.o \
                          cfg_graphics_60x_frs_tools.o
INCFLAGS		= -I../common				\
			  -I../graphics
.include	<${RULES_MK}>
