# @(#)71        1.6  src/bldenv/yacc/Makefile, ade_build, bos412, GOLDA411a 1/20/94 08:17:49
#
#   COMPONENT_NAME: BOSBUILD
#
#   FUNCTIONS:
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

VPATH			= ${MAKETOP}bos/usr/ccs/bin/yacc

yaccpar_EXECUTABLE	= yes

PROGRAMS		= yacc
MSGHDRS			= yacc_msg.h
CATFILES		= yacc.cat yacc_user.cat

ILIST			= yacc yaccpar
IDIR			= /usr/bin/
yaccpar_IDIR		= /usr/lib/

INCFLAGS		= -I${MAKETOP}bos/usr/ccs/bin/yacc
CC_OPT_LEVEL		=-g
HFILES			= ${MSGHDRS} 

OFILES			= y1.o y2.o y3.o y4.o

.include <${RULES_MK}>
