# @(#)57        1.3  src/bos/usr/lib/methods/cfgsga/Makefile, sgadd, bos411, 9428A410j 11/22/93 14:38:37
#
# COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
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

VPATH		= ../common
PROGRAMS	= cfgsga
ILIST		= ${PROGRAMS}
IDIR		= /usr/lib/methods/
cfgsga_OFILES	= cfgsga.o cfg_sgadevice.o cfgtools.o ddstools.o
LIBS            = -ls -lodm -lcfg -lrts
INCFLAGS	= -I../common

.include <${RULES_MK}>
