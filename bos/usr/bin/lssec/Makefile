# @(#)97	1.1  src/bos/usr/bin/lssec/Makefile, cmdsuser, bos411, 9428A410j 10/8/93 15:06:33
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

VPATH           = ../chsec:../newgrp

PROGRAMS	= lssec
MSGHDRS		= tcbauth_msg.h
CATFILES	= tcbauth.cat
HFILES		= ${MSGHDRS}

ILIST		= lssec
IDIR		= /usr/bin/

INCFLAGS	= -I../chsec -I../newgrp

OFILES		= lssec.o chsecatab.o chk_util.o util.o

LIBS		= -ls

.include <${RULES_MK}>
