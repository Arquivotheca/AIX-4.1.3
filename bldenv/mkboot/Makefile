# @(#)06	1.7  src/bldenv/mkboot/Makefile, bldprocess, bos412, GOLDA411a 3/19/94 15:27:09
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
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

VPATH		= ${MAKETOP}bos/usr/sbin/mkboot

PROGRAMS	= mkboot
CATFILES	= mkboot.cat

INCFLAGS 	= -I${MAKETOP}bos/usr/sbin/mkboot \
		-I${MAKETOP}/bos/kernel

ILIST		= ${PROGRAMS}
IDIR		= /usr/sbin/
LIBS		= -lodm -lcfg

.include <${RULES_MK}>
