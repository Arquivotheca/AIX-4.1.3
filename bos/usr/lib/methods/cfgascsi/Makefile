# @(#)71        1.1  src/bos/usr/lib/methods/cfgascsi/Makefile, cfgmethods, bos411, 9428A410j 9/27/93 11:21:14
#
#   COMPONENT_NAME: CFGMETHODS
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#CFLAGS		= -DCFGDEBUG

VPATH		=../common

PROGRAMS	= cfgascsi

OFILES		= cfgcommo.o cfgtoolsx.o chkslot.o cfgascsi.o  \
		  put_scsi_id.o location.o pvidtools.o

INCFLAGS	= -I../common

ILIST		= cfgascsi
IDIR		= /usr/lib/methods/

LIBS		= -lodm -lcfg

.include <${RULES_MK}>
