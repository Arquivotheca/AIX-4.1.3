# @(#)98        1.2  src/bos/usr/bin/bootlist/Makefile, Makefiles, bos41J, 9522A_b 5/31/95 11:34:58
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

PROGRAMS	= bootlist
OFILES		= bootlist.o cfgboot_tools.o
VPATH		= ${MAKETOP}/bos/usr/lib/methods/common
MSGHDRS		= 
CATFILES	= bootlist.cat
OTHERS		= ${CATFILES}

ILIST		= bootlist
IDIR		= /usr/bin/

LIBS		= -ls -lodm -lcfg

.include <${RULES_MK}>
