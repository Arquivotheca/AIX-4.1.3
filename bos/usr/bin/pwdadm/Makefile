# @(#)30        1.2  src/bos/usr/bin/pwdadm/Makefile, cmdsuser, bos411, 9428A410j 10/7/93 12:57:14
#
#   COMPONENT_NAME: CMDSUSER
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1988,1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH		= ../newgrp

PROGRAMS	= pwdadm
MSGHDRS		= tcbauth_msg.h
CATFILES	= tcbauth.cat
HFILES		= ${MSGHDRS}

ILIST		= pwdadm
IDIR		= /usr/bin/

INCFLAGS	= -I../newgrp

OFILES		= pwdadm.o util.o

LIBS		= -ls

.include <${RULES_MK}>
