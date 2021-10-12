# @(#)31        1.5  src/bos/kernel/include/pse/Makefile, sysxpse, bos411, 9428A410j 10/28/93 11:39:11
#
# COMPONENT_NAME: SYSXPSE - STREAMS framework
# 
# FUNCTIONS:
# 
# ORIGINS: 27, 63, 71, 83
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
# LEVEL 1, 5 Years Bull Confidential Information
#

VPATH=${MAKETOP}bos/usr/include/pse

EXPDIR		= /kernel/include/pse/

PORTFILES	= \
		str_config.h \
		str_system.h \
		str_lock.h \
		str_ioctl.h \
		str_select.h \
		str_funnel.h 

DATAFILES	= \
		str_debug.h  \
		str_stream.h \
		str_proto.h

COMMFILES	=\
		mi.h \
		nd.h

MENTATFILES	=\
		common.h \
		led.h \
		echo.h \
		errgen.h \
		perftest.h \
		tmod.h

INCLUDES	= ${DATAFILES} ${PORTFILES} ${COMMFILES} ${MENTATFILES}

.include <${RULES_MK}>
