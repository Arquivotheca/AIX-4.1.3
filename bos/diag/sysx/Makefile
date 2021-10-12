# @(#)22	1.4  src/bos/diag/sysx/Makefile, dsysx, bos411, 9433A411a 8/10/94 13:33:04
#
#   COMPONENT_NAME: DSYSX
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1992
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


VPATH		= ${MAKETOP}/bos/diag/dctrl
PROGRAMS	= sysx

MSGHDRS		= sysx_msg.h
HFILES		= ${MSGHDRS}
CATFILES 	= sysx.cat

LIBS		= -ldiag -lasl -lodm -lcfg -lc

ILIST		= ${PROGRAMS} sysx.cat
IDIR		= /usr/lpp/diagnostics/bin/
sysx.cat_IDIR	= /usr/lpp/diagnostics/catalog/default/

sysx_OFILES	= sysx.o sysxtest.o convert.o

CFLAGS		= -D_KERNSYS

.include <${RULES_MK}>
