# @(#)55        1.3  src/bos/usr/bin/setsenv/Makefile, cmdsuser, bos411, 9428A410j 10/7/93 13:03:24
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

VPATH		= ${MAKETOP}/bos/usr/bin/newgrp

PROGRAMS	= setsenv
MSGHDRS		= tcbauth_msg.h
CATFILES	= tcbauth.cat
HFILES		= ${MSGHDRS}

ILIST		= setsenv
IDIR		= /usr/bin/

INCFLAGS	= -I${MAKETOP}/bos/usr/bin/newgrp

OFILES		= setsenv.o util.o env_util.o

LIBS		= -ls

.include <${RULES_MK}>
