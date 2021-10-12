# @(#)93	1.1  src/bos/usr/bin/chsec/Makefile, cmdsuser, bos411, 9428A410j 10/8/93 15:05:31
#
# COMPONENT_NAME:
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

VPATH           = ../newgrp

PROGRAMS	= chsec
MSGHDRS		= tcbauth_msg.h
CATFILES	= tcbauth.cat
HFILES		= ${MSGHDRS}

ILIST		= chsec
IDIR		= /usr/bin/

INCFLAGS	= -I../newgrp

OFILES		= chsec.o chsecatab.o chk_util.o util.o

LIBS		= -ls

.include <${RULES_MK}>
