# @(#)13        1.17  src/bos/usr/lib/methods/common/Makefile, cfgmethods, bos41J, 9510A_all 2/28/95 19:53:05
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27, 83
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#
LIBRARIES	= libcfgtcp.a
EXPLIB_TARGETS	= export_libcfgtcp.a
MSGHDRS		= tcpcfglb_msg.h console_msg.h
CATFILES	= tcpcfglb.cat devices.cat devices_p.cat console.cat \
		cfgchk.cat devices_ent.cat devices_tok.cat scdisk.cat \
		ide.cat graphdev.cat
HFILES		= ${MSGHDRS}

INCFLAGS	= -I../cfgif

OFILES		= tcpcfglb.o

ILIST		= devices.cat devices_p.cat console.cat cfgchk.cat \
		devices_ent.cat devices_tok.cat scdisk.cat scarray.cat \
		ide.cat graphdev.cat
IDIR		= /usr/lib/methods/

INCLUDES	= cfgdebug.h artic.h pparms.h

EXPDIR		= /usr/include/sys/

libcfgtcp.a_EXPDIR = /usr/ccs/lib/

OTHERS		= scarray.cat
scarray.cat: scarray.cat.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>

tcpcfglb.o :   tcpcfglb_msg.h


