# @(#)21        1.1  src/bos/usr/bin/copydump/Makefile, cmddump, bos411, 9428A410j 8/26/93 17:10:00
# COMPONENT_NAME: cmddump
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

VPATH           += ${MAKETOP}/bos/usr/lpp/bosinst/BosMenus
PROGRAMS        = copydump copydumpmenu


copydump_OFILES	= copydump.o

copydumpmenu_OFILES = copydumpmenu.o MenuHand.o readfunc.o Help.o parse.o

ILIST           = copydump copydumpmenu

IDIR            = /usr/bin/

LIBS            = -lodm -lcfg

.include <${RULES_MK}>
